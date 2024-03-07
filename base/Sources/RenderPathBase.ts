
class RenderPathBase {

	static taa_frame: i32 = 0;
	static super_sample: f32 = 1.0;
	static last_x: f32 = -1.0;
	static last_y: f32 = -1.0;
	static bloom_mipmaps: render_target_t[];
	static bloom_current_mip: i32 = 0;
	static bloom_sample_scale: f32;
	///if arm_voxels
	static voxels_res: i32 = 256;
	static voxels_created: bool = false;
	///end

	static init = () => {
		RenderPathBase.super_sample = Config.raw.rp_supersample;
	}

	///if arm_voxels
	static init_voxels = (targetName: string = "voxels") => {
		if (Config.raw.rp_gi != true || RenderPathBase.voxels_created) return;
		RenderPathBase.voxels_created = true;

		{
			let t: render_target_t = render_target_create();
			t.name = targetName;
			t.format = "R8";
			t.width = RenderPathBase.voxels_res;
			t.height = RenderPathBase.voxels_res;
			t.depth = RenderPathBase.voxels_res;
			t.is_image = true;
			t.mipmaps = true;
			render_path_create_render_target(t);
		}
	}
	///end

	static apply_config = () => {
		if (RenderPathBase.super_sample != Config.raw.rp_supersample) {
			RenderPathBase.super_sample = Config.raw.rp_supersample;
			for (let rt of render_path_render_targets.values()) {
				if (rt.width == 0 && rt.scale != null) {
					rt.scale = RenderPathBase.super_sample;
				}
			}
			render_path_resize();
		}
		///if arm_voxels
		if (!RenderPathBase.voxels_created) RenderPathBase.init_voxels();
		///end
	}

	static get_super_sampling = (): f32 => {
		return RenderPathBase.super_sample;
	}

	static draw_compass = () => {
		if (Context.raw.show_compass) {
			let cam: camera_object_t = scene_camera;
			let compass: mesh_object_t = scene_get_child(".Compass").ext;

			let _visible: bool = compass.base.visible;
			let _parent: object = compass.base.parent;
			let _loc: vec4_t = compass.base.transform.loc;
			let _rot: quat_t = compass.base.transform.rot;
			let crot: quat_t = cam.base.transform.rot;
			let ratio: f32 = app_w() / app_h();
			let _P: mat4_t = cam.p;
			cam.p = mat4_ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
			compass.base.visible = true;
			compass.base.parent = cam.base;
			compass.base.transform.loc = vec4_create(7.4 * ratio, 7.0, -1);
			compass.base.transform.rot = quat_create(-crot.x, -crot.y, -crot.z, crot.w);
			vec4_set(compass.base.transform.scale, 0.4, 0.4, 0.4);
			transform_build_matrix(compass.base.transform);
			compass.frustum_culling = false;
			mesh_object_render(compass, "overlay", []);

			cam.p = _P;
			compass.base.visible = _visible;
			compass.base.parent = _parent;
			compass.base.transform.loc = _loc;
			compass.base.transform.rot = _rot;
			transform_build_matrix(compass.base.transform);
		}
	}

	static begin = () => {
		// Begin split
		if (Context.raw.split_view && !Context.raw.paint2d_view) {
			if (Context.raw.view_index_last == -1 && Context.raw.view_index == -1) {
				// Begin split, draw right viewport first
				Context.raw.view_index = 1;
			}
			else {
				// Set current viewport
				Context.raw.view_index = mouse_view_x() > Base.w() / 2 ? 1 : 0;
			}

			let cam: camera_object_t = scene_camera;
			if (Context.raw.view_index_last > -1) {
				// Save current viewport camera
				mat4_set_from(Camera.views[Context.raw.view_index_last], cam.base.transform.local);
			}

			let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;

			if (Context.raw.view_index_last != Context.raw.view_index || decal || !Config.raw.brush_3d) {
				// Redraw on current viewport change
				Context.raw.ddirty = 1;
			}

			transform_set_matrix(cam.base.transform, Camera.views[Context.raw.view_index]);
			camera_object_build_mat(cam);
			camera_object_build_proj(cam);
		}

		// Match projection matrix jitter
		let skipTaa: bool = Context.raw.split_view || ((Context.raw.tool == workspace_tool_t.CLONE || Context.raw.tool == workspace_tool_t.BLUR || Context.raw.tool == workspace_tool_t.SMUDGE) && Context.raw.pdirty > 0);
		scene_camera.frame = skipTaa ? 0 : RenderPathBase.taa_frame;
		camera_object_proj_jitter(scene_camera);
		camera_object_build_mat(scene_camera);
	}

	static end = () => {
		// End split
		Context.raw.view_index_last = Context.raw.view_index;
		Context.raw.view_index = -1;

		if (Context.raw.foreground_event && !mouse_down()) {
			Context.raw.foreground_event = false;
			Context.raw.pdirty = 0;
		}

		RenderPathBase.taa_frame++;
	}

	static ssaa4 = (): bool => {
		return Config.raw.rp_supersample == 4;
	}

	static is_cached = (): bool => {
		if (sys_width() == 0 || sys_height() == 0) return true;

		let mx: f32 = RenderPathBase.last_x;
		let my: f32 = RenderPathBase.last_y;
		RenderPathBase.last_x = mouse_view_x();
		RenderPathBase.last_y = mouse_view_y();

		if (Context.raw.ddirty <= 0 && Context.raw.rdirty <= 0 && Context.raw.pdirty <= 0) {
			if (mx != RenderPathBase.last_x || my != RenderPathBase.last_y || mouse_locked) Context.raw.ddirty = 0;
			///if (krom_metal || krom_android)
			if (Context.raw.ddirty > -6) {
			///else
			if (Context.raw.ddirty > -2) {
			///end
				render_path_set_target("");
				render_path_bind_target("taa", "tex");
				RenderPathBase.ssaa4() ?
					render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve") :
					render_path_draw_shader("shader_datas/copy_pass/copy_pass");
				RenderPathPaint.commands_cursor();
				if (Context.raw.ddirty <= 0) Context.raw.ddirty--;
			}
			RenderPathBase.end();
			return true;
		}
		return false;
	}

	static commands = (draw_commands: ()=>void) => {
		if (RenderPathBase.is_cached()) return;
		RenderPathBase.begin();

		RenderPathPaint.begin();
		RenderPathBase.draw_split(draw_commands);
		RenderPathBase.draw_gbuffer();
		RenderPathPaint.draw();

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		if (Context.raw.viewport_mode ==  viewport_mode_t.PATH_TRACE) {
			///if is_paint
			let useLiveLayer: bool = Context.raw.tool == workspace_tool_t.MATERIAL;
			///else
			let useLiveLayer: bool = false;
			///end
			RenderPathRaytrace.draw(useLiveLayer);
			return;
		}
		///end

		draw_commands();
		RenderPathPaint.end();
		RenderPathBase.end();
	}

	static draw_bloom = (tex: string = "tex") => {
		if (Config.raw.rp_bloom != false) {
			if (RenderPathBase.bloom_mipmaps == null) {
				RenderPathBase.bloom_mipmaps = [];

				let prevScale: f32 = 1.0;
				for (let i: i32 = 0; i < 10; ++i) {
					let t: render_target_t = render_target_create();
					t.name = "bloom_mip_" + i;
					t.width = 0;
					t.height = 0;
					t.scale = (prevScale *= 0.5);
					t.format = "RGBA64";
					RenderPathBase.bloom_mipmaps.push(render_path_create_render_target(t));
				}

				render_path_load_shader("shader_datas/bloom_pass/bloom_downsample_pass");
				render_path_load_shader("shader_datas/bloom_pass/bloom_upsample_pass");
			}

			let bloomRadius: f32 = 6.5;
			let minDim: f32 = Math.min(render_path_current_w,render_path_current_h);
			let logMinDim: f32 = Math.max(1.0, Math.log2(minDim) + (bloomRadius - 8.0));
			let numMips: i32 = Math.floor(logMinDim);
			RenderPathBase.bloom_sample_scale = 0.5 + logMinDim - numMips;

			for (let i: i32 = 0; i < numMips; ++i) {
				RenderPathBase.bloom_current_mip = i;
				render_path_set_target(RenderPathBase.bloom_mipmaps[i].name);
				render_path_clear_target();
				render_path_bind_target(i == 0 ? tex : RenderPathBase.bloom_mipmaps[i - 1].name, "tex");
				render_path_draw_shader("shader_datas/bloom_pass/bloom_downsample_pass");
			}
			for (let i: i32 = 0; i < numMips; ++i) {
				let mipLevel: i32 = numMips - 1 - i;
				RenderPathBase.bloom_current_mip = mipLevel;
				render_path_set_target(mipLevel == 0 ? tex : RenderPathBase.bloom_mipmaps[mipLevel - 1].name);
				render_path_bind_target(RenderPathBase.bloom_mipmaps[mipLevel].name, "tex");
				render_path_draw_shader("shader_datas/bloom_pass/bloom_upsample_pass");
			}
		}
	}

	static draw_split = (drawCommands: ()=>void) => {
		if (Context.raw.split_view && !Context.raw.paint2d_view) {
			Context.raw.ddirty = 2;
			let cam: camera_object_t = scene_camera;

			Context.raw.view_index = Context.raw.view_index == 0 ? 1 : 0;
			transform_set_matrix(cam.base.transform, Camera.views[Context.raw.view_index]);
			camera_object_build_mat(cam);
			camera_object_build_proj(cam);

			RenderPathBase.draw_gbuffer();

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			///if is_paint
			let useLiveLayer: bool = Context.raw.tool == workspace_tool_t.MATERIAL;
			///else
			let useLiveLayer: bool = false;
			///end
			Context.raw.viewport_mode == viewport_mode_t.PATH_TRACE ? RenderPathRaytrace.draw(useLiveLayer) : drawCommands();
			///else
			drawCommands();
			///end

			Context.raw.view_index = Context.raw.view_index == 0 ? 1 : 0;
			transform_set_matrix(cam.base.transform, Camera.views[Context.raw.view_index]);
			camera_object_build_mat(cam);
			camera_object_build_proj(cam);
		}
	}

	///if arm_voxels
	static draw_voxels = () => {
		if (Config.raw.rp_gi != false) {
			let voxelize: bool = Context.raw.ddirty > 0 && RenderPathBase.taa_frame > 0;
			if (voxelize) {
				render_path_clear_image("voxels", 0x00000000);
				render_path_set_target("");
				render_path_set_viewport(RenderPathBase.voxels_res, RenderPathBase.voxels_res);
				render_path_bind_target("voxels", "voxels");
				if (MakeMaterial.heightUsed) {
					let tid: i32 = 0; // Project.layers[0].id;
					render_path_bind_target("texpaint_pack" + tid, "texpaint_pack");
				}
				render_path_draw_meshes("voxel");
				render_path_gen_mipmaps("voxels");
			}
		}
	}
	///end

	static init_ssao = () => {
		{
			let t: render_target_t = render_target_create();
			t.name = "singlea";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "singleb";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		render_path_load_shader("shader_datas/ssao_pass/ssao_pass");
		render_path_load_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");
		render_path_load_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
	}

	static draw_ssao = () => {
		let ssao: bool = Config.raw.rp_ssao != false && Context.raw.camera_type == camera_type_t.PERSPECTIVE;
		if (ssao && Context.raw.ddirty > 0 && RenderPathBase.taa_frame > 0) {
			if (render_path_render_targets.get("singlea") == null) {
				RenderPathBase.init_ssao();
			}

			render_path_set_target("singlea");
			render_path_bind_target("_main", "gbufferD");
			render_path_bind_target("gbuffer0", "gbuffer0");
			render_path_draw_shader("shader_datas/ssao_pass/ssao_pass");

			render_path_set_target("singleb");
			render_path_bind_target("singlea", "tex");
			render_path_bind_target("gbuffer0", "gbuffer0");
			render_path_draw_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");

			render_path_set_target("singlea");
			render_path_bind_target("singleb", "tex");
			render_path_bind_target("gbuffer0", "gbuffer0");
			render_path_draw_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
		}
	}

	static draw_deferred_light = () => {
		render_path_set_target("tex");
		render_path_bind_target("_main", "gbufferD");
		render_path_bind_target("gbuffer0", "gbuffer0");
		render_path_bind_target("gbuffer1", "gbuffer1");
		let ssao: bool = Config.raw.rp_ssao != false && Context.raw.camera_type == camera_type_t.PERSPECTIVE;
		if (ssao && RenderPathBase.taa_frame > 0) {
			render_path_bind_target("singlea", "ssaotex");
		}
		else {
			render_path_bind_target("empty_white", "ssaotex");
		}

		let voxelao_pass: bool = false;
		///if arm_voxels
		if (Config.raw.rp_gi != false) {
			voxelao_pass = true;
			render_path_bind_target("voxels", "voxels");
		}
		///end

		voxelao_pass ?
			render_path_draw_shader("shader_datas/deferred_light/deferred_light_voxel") :
			render_path_draw_shader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		render_path_set_depth_from("tex", "gbuffer0"); // Bind depth for world pass
		///end

		render_path_set_target("tex");
		render_path_draw_skydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		render_path_set_depth_from("tex", "gbuffer1"); // Unbind depth
		///end
	}

	static draw_ssr = () => {
		if (Config.raw.rp_ssr != false) {
			if (_render_path_cached_shader_contexts.get("shader_datas/ssr_pass/ssr_pass") == null) {
				{
					let t: render_target_t = render_target_create();
					t.name = "bufb";
					t.width = 0;
					t.height = 0;
					t.format = "RGBA64";
					render_path_create_render_target(t);
				}
				render_path_load_shader("shader_datas/ssr_pass/ssr_pass");
				render_path_load_shader("shader_datas/ssr_blur_pass/ssr_blur_pass_x");
				render_path_load_shader("shader_datas/ssr_blur_pass/ssr_blur_pass_y3_blend");
			}
			let targeta: string = "bufb";
			let targetb: string = "gbuffer1";

			render_path_set_target(targeta);
			render_path_bind_target("tex", "tex");
			render_path_bind_target("_main", "gbufferD");
			render_path_bind_target("gbuffer0", "gbuffer0");
			render_path_bind_target("gbuffer1", "gbuffer1");
			render_path_draw_shader("shader_datas/ssr_pass/ssr_pass");

			render_path_set_target(targetb);
			render_path_bind_target(targeta, "tex");
			render_path_bind_target("gbuffer0", "gbuffer0");
			render_path_draw_shader("shader_datas/ssr_blur_pass/ssr_blur_pass_x");

			render_path_set_target("tex");
			render_path_bind_target(targetb, "tex");
			render_path_bind_target("gbuffer0", "gbuffer0");
			render_path_draw_shader("shader_datas/ssr_blur_pass/ssr_blur_pass_y3_blend");
		}
	}

	// static draw_motion_blur = () => {
	// 	if (Config.raw.rp_motionblur != false) {
	// 		render_path_set_target("buf");
	// 		render_path_bind_target("tex", "tex");
	// 		render_path_bind_target("gbuffer0", "gbuffer0");
	// 		///if (rp_motionblur == "Camera")
	// 		{
	// 			render_path_bind_target("_main", "gbufferD");
	// 			render_path_draw_shader("shader_datas/motion_blur_pass/motion_blur_pass");
	// 		}
	// 		///else
	// 		{
	// 			render_path_bind_target("gbuffer2", "sveloc");
	// 			render_path_draw_shader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
	// 		}
	// 		///end
	// 		render_path_set_target("tex");
	// 		render_path_bind_target("buf", "tex");
	// 		render_path_draw_shader("shader_datas/copy_pass/copy_pass");
	// 	}
	// }

	// static draw_histogram = () => {
	// 	{
	// 		let t: render_target_t = RenderTarget.create();
	// 		t.name = "histogram";
	// 		t.width = 1;
	// 		t.height = 1;
	// 		t.format = "RGBA64";
	// 		render_path_create_render_target(t);

	// 		render_path_load_shader("shader_datas/histogram_pass/histogram_pass");
	// 	}

	// 	render_path_set_target("histogram");
	// 	render_path_bind_target("taa", "tex");
	// 	render_path_draw_shader("shader_datas/histogram_pass/histogram_pass");
	// }

	static draw_taa = () => {
		let current: string = RenderPathBase.taa_frame % 2 == 0 ? "buf2" : "taa2";
		let last: string = RenderPathBase.taa_frame % 2 == 0 ? "taa2" : "buf2";

		render_path_set_target(current);
		render_path_clear_target(0x00000000);
		render_path_bind_target("buf", "colorTex");
		render_path_draw_shader("shader_datas/smaa_edge_detect/smaa_edge_detect");

		render_path_set_target("taa");
		render_path_clear_target(0x00000000);
		render_path_bind_target(current, "edgesTex");
		render_path_draw_shader("shader_datas/smaa_blend_weight/smaa_blend_weight");

		render_path_set_target(current);
		render_path_bind_target("buf", "colorTex");
		render_path_bind_target("taa", "blendTex");
		render_path_bind_target("gbuffer2", "sveloc");
		render_path_draw_shader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

		let skipTaa: bool = Context.raw.split_view;
		if (skipTaa) {
			render_path_set_target("taa");
			render_path_bind_target(current, "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");
		}
		else {
			render_path_set_target("taa");
			render_path_bind_target(current, "tex");
			render_path_bind_target(last, "tex2");
			render_path_bind_target("gbuffer2", "sveloc");
			render_path_draw_shader("shader_datas/taa_pass/taa_pass");
		}

		if (RenderPathBase.ssaa4()) {
			render_path_set_target("");
			render_path_bind_target(RenderPathBase.taa_frame % 2 == 0 ? "taa2" : "taa", "tex");
			render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");
		}
		else {
			render_path_set_target("");
			render_path_bind_target(RenderPathBase.taa_frame == 0 ? current : "taa", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");
		}
	}

	static draw_gbuffer = () => {
		render_path_set_target("gbuffer0"); // Only clear gbuffer0
		///if krom_metal
		render_path_clear_target(0x00000000, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
		///else
		render_path_clear_target(null, 1.0, clear_flag_t.DEPTH);
		///end
		if (MakeMesh.layer_pass_count == 1) {
			render_path_set_target("gbuffer2");
			render_path_clear_target(0xff000000);
		}
		render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
		RenderPathPaint.bind_layers();
		render_path_draw_meshes("mesh");
		RenderPathPaint.unbind_layers();
		if (MakeMesh.layer_pass_count > 1) {
			RenderPathBase.make_gbuffer_copy_textures();
			for (let i: i32 = 1; i < MakeMesh.layer_pass_count; ++i) {
				let ping: string = i % 2 == 1 ? "_copy" : "";
				let pong: string = i % 2 == 1 ? "" : "_copy";
				if (i == MakeMesh.layer_pass_count - 1) {
					render_path_set_target("gbuffer2" + ping);
					render_path_clear_target(0xff000000);
				}
				render_path_set_target("gbuffer0" + ping, ["gbuffer1" + ping, "gbuffer2" + ping]);
				render_path_bind_target("gbuffer0" + pong, "gbuffer0");
				render_path_bind_target("gbuffer1" + pong, "gbuffer1");
				render_path_bind_target("gbuffer2" + pong, "gbuffer2");
				RenderPathPaint.bind_layers();
				render_path_draw_meshes("mesh" + i);
				RenderPathPaint.unbind_layers();
			}
			if (MakeMesh.layer_pass_count % 2 == 0) {
				RenderPathBase.copy_to_gbuffer();
			}
		}

		let hide: bool = Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown) || keyboard_down("control");
		let isDecal: bool = Base.is_decal_layer();
		if (isDecal && !hide) LineDraw.render(Context.raw.layer.decalMat);
	}

	static make_gbuffer_copy_textures = () => {
		let copy: render_target_t = render_path_render_targets.get("gbuffer0_copy");
		if (copy == null || copy._image.width != render_path_render_targets.get("gbuffer0")._image.width || copy._image.height != render_path_render_targets.get("gbuffer0")._image.height) {
			{
				let t: render_target_t = render_target_create();
				t.name = "gbuffer0_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.get_super_sampling();
				t.depth_buffer = "main";
				render_path_create_render_target(t);
			}
			{
				let t: render_target_t = render_target_create();
				t.name = "gbuffer1_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.get_super_sampling();
				render_path_create_render_target(t);
			}
			{
				let t: render_target_t = render_target_create();
				t.name = "gbuffer2_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.get_super_sampling();
				render_path_create_render_target(t);
			}

			///if krom_metal
			// TODO: Fix depth attach for gbuffer0_copy on metal
			// Use resize to re-create buffers from scratch for now
			render_path_resize();
			///end
		}
	}

	static copy_to_gbuffer = () => {
		render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
		render_path_bind_target("gbuffer0_copy", "tex0");
		render_path_bind_target("gbuffer1_copy", "tex1");
		render_path_bind_target("gbuffer2_copy", "tex2");
		render_path_draw_shader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
	}
}
