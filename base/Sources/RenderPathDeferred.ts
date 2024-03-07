
class RenderPathDeferred {

	static init = () => {
		render_path_create_depth_buffer("main", "DEPTH24");

		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.get_super_sampling();
			t.depth_buffer = "main";
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.get_super_sampling();
			///if krom_opengl
			t.depth_buffer = "main";
			///end
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			///if (krom_direct3d12 || krom_vulkan)// || krom_metal)
			// Match raytrace_target format
			// Will cause "The render target format in slot 0 does not match that specified by the current pipeline state"
			t.format = "RGBA64";
			///else
			t.format = "RGBA32";
			///end
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "buf2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "empty_white";
			t.width = 1;
			t.height = 1;
			t.format = "R8";
			let b: ArrayBuffer = new ArrayBuffer(1);
			let v: DataView = new DataView(b);
			v.setUint8(0, 255);
			t._image = image_from_bytes(b, t.width, t.height, tex_format_t.R8);
			render_path_render_targets.set(t.name, t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "empty_black";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			let b: ArrayBuffer = new ArrayBuffer(4);
			let v: DataView = new DataView(b);
			v.setUint8(0, 0);
			v.setUint8(1, 0);
			v.setUint8(2, 0);
			v.setUint8(3, 0);
			t._image = image_from_bytes(b, t.width, t.height, tex_format_t.RGBA32);
			render_path_render_targets.set(t.name, t);
		}

		///if is_sculpt
		{
			let t: render_target_t = render_target_create();
			t.name = "gbuffer0_undo";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "gbufferD_undo";
			t.width = 0;
			t.height = 0;
			t.format = "R32";
			t.scale = RenderPathBase.get_super_sampling();
			render_path_create_render_target(t);
		}
		///end

		if (Config.raw.rp_ssao) {
			RenderPathBase.init_ssao();
		}

		render_path_load_shader("shader_datas/world_pass/world_pass");
		render_path_load_shader("shader_datas/deferred_light/deferred_light");
		render_path_load_shader("shader_datas/compositor_pass/compositor_pass");
		render_path_load_shader("shader_datas/copy_pass/copy_pass");
		render_path_load_shader("shader_datas/copy_pass/copyR8_pass");
		render_path_load_shader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		render_path_load_shader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		render_path_load_shader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		render_path_load_shader("shader_datas/taa_pass/taa_pass");
		render_path_load_shader("shader_datas/supersample_resolve/supersample_resolve");
		// render_path_load_shader("shader_datas/motion_blur_pass/motion_blur_pass");
		// render_path_load_shader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
		///if arm_voxels
		{
			RenderPathBase.init_voxels();
			render_path_load_shader("shader_datas/deferred_light/deferred_light_voxel");
		}
		///end

		RenderPathPaint.init();

		///if (is_paint || is_sculpt)
		RenderPathPreview.init();
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.init();
		///end
	}

	static commands = () => {
		///if is_paint
		RenderPathPaint.live_brush_dirty();
		///end
		RenderPathBase.commands(RenderPathDeferred.draw_deferred);
	}

	static draw_deferred = () => {
		RenderPathBase.draw_ssao();
		///if arm_voxels
		RenderPathBase.draw_voxels();
		///end
		RenderPathBase.draw_deferred_light();
		RenderPathBase.draw_bloom();
		RenderPathBase.draw_ssr();
		// RenderPathBase.drawMotionBlur();
		// RenderPathBase.drawHistogram();

		render_path_set_target("buf");
		render_path_bind_target("tex", "tex");
		// render_path_bind_target("histogram", "histogram");
		render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

		render_path_set_target("buf");
		RenderPathBase.draw_compass();
		render_path_draw_meshes("overlay");

		RenderPathBase.draw_taa();
	}
}
