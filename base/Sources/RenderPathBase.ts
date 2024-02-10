
class RenderPathBase {

	static taaFrame = 0;
	static superSample = 1.0;
	static lastX = -1.0;
	static lastY = -1.0;
	static bloomMipmaps: render_target_t[];
	static bloomCurrentMip = 0;
	static bloomSampleScale: f32;
	///if arm_voxels
	static voxelsRes = 256;
	static voxelsCreated = false;
	///end

	static init = () => {
		RenderPathBase.superSample = Config.raw.rp_supersample;
	}

	///if arm_voxels
	static initVoxels = (targetName = "voxels") => {
		if (Config.raw.rp_gi != true || RenderPathBase.voxelsCreated) return;
		RenderPathBase.voxelsCreated = true;

		{
			let t = render_target_create();
			t.name = targetName;
			t.format = "R8";
			t.width = RenderPathBase.voxelsRes;
			t.height = RenderPathBase.voxelsRes;
			t.depth = RenderPathBase.voxelsRes;
			t.is_image = true;
			t.mipmaps = true;
			render_path_create_render_target(t);
		}
	}
	///end

	static applyConfig = () => {
		if (RenderPathBase.superSample != Config.raw.rp_supersample) {
			RenderPathBase.superSample = Config.raw.rp_supersample;
			for (let rt of render_path_render_targets.values()) {
				if (rt.width == 0 && rt.scale != null) {
					rt.scale = RenderPathBase.superSample;
				}
			}
			render_path_resize();
		}
		///if arm_voxels
		if (!RenderPathBase.voxelsCreated) RenderPathBase.initVoxels();
		///end
	}

	static getSuperSampling = (): f32 => {
		return RenderPathBase.superSample;
	}

	static drawCompass = () => {
		if (Context.raw.showCompass) {
			let cam = scene_camera;
			let compass: mesh_object_t = scene_get_child(".Compass").ext;

			let _visible = compass.base.visible;
			let _parent = compass.base.parent;
			let _loc = compass.base.transform.loc;
			let _rot = compass.base.transform.rot;
			let crot = cam.base.transform.rot;
			let ratio = app_w() / app_h();
			let _P = cam.p;
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
		if (Context.raw.splitView && !Context.raw.paint2dView) {
			if (Context.raw.viewIndexLast == -1 && Context.raw.viewIndex == -1) {
				// Begin split, draw right viewport first
				Context.raw.viewIndex = 1;
			}
			else {
				// Set current viewport
				Context.raw.viewIndex = mouse_view_x() > Base.w() / 2 ? 1 : 0;
			}

			let cam = scene_camera;
			if (Context.raw.viewIndexLast > -1) {
				// Save current viewport camera
				mat4_set_from(Camera.views[Context.raw.viewIndexLast], cam.base.transform.local);
			}

			let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;

			if (Context.raw.viewIndexLast != Context.raw.viewIndex || decal || !Config.raw.brush_3d) {
				// Redraw on current viewport change
				Context.raw.ddirty = 1;
			}

			transform_set_matrix(cam.base.transform, Camera.views[Context.raw.viewIndex]);
			camera_object_build_mat(cam);
			camera_object_build_proj(cam);
		}

		// Match projection matrix jitter
		let skipTaa = Context.raw.splitView || ((Context.raw.tool == WorkspaceTool.ToolClone || Context.raw.tool == WorkspaceTool.ToolBlur || Context.raw.tool == WorkspaceTool.ToolSmudge) && Context.raw.pdirty > 0);
		scene_camera.frame = skipTaa ? 0 : RenderPathBase.taaFrame;
		camera_object_proj_jitter(scene_camera);
		camera_object_build_mat(scene_camera);
	}

	static end = () => {
		// End split
		Context.raw.viewIndexLast = Context.raw.viewIndex;
		Context.raw.viewIndex = -1;

		if (Context.raw.foregroundEvent && !mouse_down()) {
			Context.raw.foregroundEvent = false;
			Context.raw.pdirty = 0;
		}

		RenderPathBase.taaFrame++;
	}

	static ssaa4 = (): bool => {
		return Config.raw.rp_supersample == 4;
	}

	static isCached = (): bool => {
		if (sys_width() == 0 || sys_height() == 0) return true;

		let mx = RenderPathBase.lastX;
		let my = RenderPathBase.lastY;
		RenderPathBase.lastX = mouse_view_x();
		RenderPathBase.lastY = mouse_view_y();

		if (Context.raw.ddirty <= 0 && Context.raw.rdirty <= 0 && Context.raw.pdirty <= 0) {
			if (mx != RenderPathBase.lastX || my != RenderPathBase.lastY || mouse_locked) Context.raw.ddirty = 0;
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
				RenderPathPaint.commandsCursor();
				if (Context.raw.ddirty <= 0) Context.raw.ddirty--;
			}
			RenderPathBase.end();
			return true;
		}
		return false;
	}

	static commands = (drawCommands: ()=>void) => {
		if (RenderPathBase.isCached()) return;
		RenderPathBase.begin();

		RenderPathPaint.begin();
		RenderPathBase.drawSplit(drawCommands);
		RenderPathBase.drawGbuffer();
		RenderPathPaint.draw();

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		if (Context.raw.viewportMode ==  ViewportMode.ViewPathTrace) {
			///if is_paint
			let useLiveLayer = Context.raw.tool == WorkspaceTool.ToolMaterial;
			///else
			let useLiveLayer = false;
			///end
			RenderPathRaytrace.draw(useLiveLayer);
			return;
		}
		///end

		drawCommands();
		RenderPathPaint.end();
		RenderPathBase.end();
	}

	static drawBloom = (tex = "tex") => {
		if (Config.raw.rp_bloom != false) {
			if (RenderPathBase.bloomMipmaps == null) {
				RenderPathBase.bloomMipmaps = [];

				let prevScale = 1.0;
				for (let i = 0; i < 10; ++i) {
					let t = render_target_create();
					t.name = "bloom_mip_" + i;
					t.width = 0;
					t.height = 0;
					t.scale = (prevScale *= 0.5);
					t.format = "RGBA64";
					RenderPathBase.bloomMipmaps.push(render_path_create_render_target(t));
				}

				render_path_load_shader("shader_datas/bloom_pass/bloom_downsample_pass");
				render_path_load_shader("shader_datas/bloom_pass/bloom_upsample_pass");
			}

			let bloomRadius = 6.5;
			let minDim = Math.min(render_path_current_w,render_path_current_h);
			let logMinDim = Math.max(1.0, Math.log2(minDim) + (bloomRadius - 8.0));
			let numMips = Math.floor(logMinDim);
			RenderPathBase.bloomSampleScale = 0.5 + logMinDim - numMips;

			for (let i = 0; i < numMips; ++i) {
				RenderPathBase.bloomCurrentMip = i;
				render_path_set_target(RenderPathBase.bloomMipmaps[i].name);
				render_path_clear_target();
				render_path_bind_target(i == 0 ? tex : RenderPathBase.bloomMipmaps[i - 1].name, "tex");
				render_path_draw_shader("shader_datas/bloom_pass/bloom_downsample_pass");
			}
			for (let i = 0; i < numMips; ++i) {
				let mipLevel = numMips - 1 - i;
				RenderPathBase.bloomCurrentMip = mipLevel;
				render_path_set_target(mipLevel == 0 ? tex : RenderPathBase.bloomMipmaps[mipLevel - 1].name);
				render_path_bind_target(RenderPathBase.bloomMipmaps[mipLevel].name, "tex");
				render_path_draw_shader("shader_datas/bloom_pass/bloom_upsample_pass");
			}
		}
	}

	static drawSplit = (drawCommands: ()=>void) => {
		if (Context.raw.splitView && !Context.raw.paint2dView) {
			Context.raw.ddirty = 2;
			let cam = scene_camera;

			Context.raw.viewIndex = Context.raw.viewIndex == 0 ? 1 : 0;
			transform_set_matrix(cam.base.transform, Camera.views[Context.raw.viewIndex]);
			camera_object_build_mat(cam);
			camera_object_build_proj(cam);

			RenderPathBase.drawGbuffer();

			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			///if is_paint
			let useLiveLayer = Context.raw.tool == WorkspaceTool.ToolMaterial;
			///else
			let useLiveLayer = false;
			///end
			Context.raw.viewportMode == ViewportMode.ViewPathTrace ? RenderPathRaytrace.draw(useLiveLayer) : drawCommands();
			///else
			drawCommands();
			///end

			Context.raw.viewIndex = Context.raw.viewIndex == 0 ? 1 : 0;
			transform_set_matrix(cam.base.transform, Camera.views[Context.raw.viewIndex]);
			camera_object_build_mat(cam);
			camera_object_build_proj(cam);
		}
	}

	///if arm_voxels
	static drawVoxels = () => {
		if (Config.raw.rp_gi != false) {
			let voxelize = Context.raw.ddirty > 0 && RenderPathBase.taaFrame > 0;
			if (voxelize) {
				render_path_clear_image("voxels", 0x00000000);
				render_path_set_target("");
				render_path_set_viewport(RenderPathBase.voxelsRes, RenderPathBase.voxelsRes);
				render_path_bind_target("voxels", "voxels");
				if (MakeMaterial.heightUsed) {
					let tid = 0; // Project.layers[0].id;
					render_path_bind_target("texpaint_pack" + tid, "texpaint_pack");
				}
				render_path_draw_meshes("voxel");
				render_path_gen_mipmaps("voxels");
			}
		}
	}
	///end

	static initSSAO = () => {
		{
			let t = render_target_create();
			t.name = "singlea";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = RenderPathBase.getSuperSampling();
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "singleb";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = RenderPathBase.getSuperSampling();
			render_path_create_render_target(t);
		}
		render_path_load_shader("shader_datas/ssao_pass/ssao_pass");
		render_path_load_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");
		render_path_load_shader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
	}

	static drawSSAO = () => {
		let ssao = Config.raw.rp_ssao != false && Context.raw.cameraType == CameraType.CameraPerspective;
		if (ssao && Context.raw.ddirty > 0 && RenderPathBase.taaFrame > 0) {
			if (render_path_render_targets.get("singlea") == null) {
				RenderPathBase.initSSAO();
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

	static drawDeferredLight = () => {
		render_path_set_target("tex");
		render_path_bind_target("_main", "gbufferD");
		render_path_bind_target("gbuffer0", "gbuffer0");
		render_path_bind_target("gbuffer1", "gbuffer1");
		let ssao = Config.raw.rp_ssao != false && Context.raw.cameraType == CameraType.CameraPerspective;
		if (ssao && RenderPathBase.taaFrame > 0) {
			render_path_bind_target("singlea", "ssaotex");
		}
		else {
			render_path_bind_target("empty_white", "ssaotex");
		}

		let voxelao_pass = false;
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

	static drawSSR = () => {
		if (Config.raw.rp_ssr != false) {
			if (_render_path_cached_shader_contexts.get("shader_datas/ssr_pass/ssr_pass") == null) {
				{
					let t = render_target_create();
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
			let targeta = "bufb";
			let targetb = "gbuffer1";

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

	// static drawMotionBlur = () => {
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

	// static drawHistogram = () => {
	// 	{
	// 		let t = RenderTarget.create();
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

	static drawTAA = () => {
		let current = RenderPathBase.taaFrame % 2 == 0 ? "buf2" : "taa2";
		let last = RenderPathBase.taaFrame % 2 == 0 ? "taa2" : "buf2";

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

		let skipTaa = Context.raw.splitView;
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
			render_path_bind_target(RenderPathBase.taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
			render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");
		}
		else {
			render_path_set_target("");
			render_path_bind_target(RenderPathBase.taaFrame == 0 ? current : "taa", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");
		}
	}

	static drawGbuffer = () => {
		render_path_set_target("gbuffer0"); // Only clear gbuffer0
		///if krom_metal
		render_path_clear_target(0x00000000, 1.0);
		///else
		render_path_clear_target(null, 1.0);
		///end
		if (MakeMesh.layerPassCount == 1) {
			render_path_set_target("gbuffer2");
			render_path_clear_target(0xff000000);
		}
		render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
		RenderPathPaint.bindLayers();
		render_path_draw_meshes("mesh");
		RenderPathPaint.unbindLayers();
		if (MakeMesh.layerPassCount > 1) {
			RenderPathBase.makeGbufferCopyTextures();
			for (let i = 1; i < MakeMesh.layerPassCount; ++i) {
				let ping = i % 2 == 1 ? "_copy" : "";
				let pong = i % 2 == 1 ? "" : "_copy";
				if (i == MakeMesh.layerPassCount - 1) {
					render_path_set_target("gbuffer2" + ping);
					render_path_clear_target(0xff000000);
				}
				render_path_set_target("gbuffer0" + ping, ["gbuffer1" + ping, "gbuffer2" + ping]);
				render_path_bind_target("gbuffer0" + pong, "gbuffer0");
				render_path_bind_target("gbuffer1" + pong, "gbuffer1");
				render_path_bind_target("gbuffer2" + pong, "gbuffer2");
				RenderPathPaint.bindLayers();
				render_path_draw_meshes("mesh" + i);
				RenderPathPaint.unbindLayers();
			}
			if (MakeMesh.layerPassCount % 2 == 0) {
				RenderPathBase.copyToGbuffer();
			}
		}

		let hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown) || keyboard_down("control");
		let isDecal = Base.isDecalLayer();
		if (isDecal && !hide) LineDraw.render(Context.raw.layer.decalMat);
	}

	static makeGbufferCopyTextures = () => {
		let copy = render_path_render_targets.get("gbuffer0_copy");
		if (copy == null || copy.image.width != render_path_render_targets.get("gbuffer0").image.width || copy.image.height != render_path_render_targets.get("gbuffer0").image.height) {
			{
				let t = render_target_create();
				t.name = "gbuffer0_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.getSuperSampling();
				t.depth_buffer = "main";
				render_path_create_render_target(t);
			}
			{
				let t = render_target_create();
				t.name = "gbuffer1_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.getSuperSampling();
				render_path_create_render_target(t);
			}
			{
				let t = render_target_create();
				t.name = "gbuffer2_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.getSuperSampling();
				render_path_create_render_target(t);
			}

			///if krom_metal
			// TODO: Fix depth attach for gbuffer0_copy on metal
			// Use resize to re-create buffers from scratch for now
			render_path_resize();
			///end
		}
	}

	static copyToGbuffer = () => {
		render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
		render_path_bind_target("gbuffer0_copy", "tex0");
		render_path_bind_target("gbuffer1_copy", "tex1");
		render_path_bind_target("gbuffer2_copy", "tex2");
		render_path_draw_shader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
	}
}
