
function render_path_deferred_init() {
	render_path_create_depth_buffer("main", "DEPTH24");

	{
		let t: render_target_t = render_target_create();
		t.name = "gbuffer0";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		t.depth_buffer = "main";
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "gbuffer1";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "gbuffer2";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "tex";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "buf";
		t.width = 0;
		t.height = 0;
		///if (arm_direct3d12 || arm_vulkan)
		// Match raytrace_target format
		// Will cause "The render target format in slot 0 does not match that specified by the current pipeline state"
		t.format = "RGBA64";
		///else
		t.format = "RGBA32";
		///end
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "buf2";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA32";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "taa";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA32";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "taa2";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA32";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "empty_white";
		t.width = 1;
		t.height = 1;
		t.format = "R8";
		let b: buffer_t = buffer_create(1);
		buffer_set_u8(b, 0, 255);
		t._image = image_from_bytes(b, t.width, t.height, tex_format_t.R8);
		map_set(render_path_render_targets, t.name, t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "empty_black";
		t.width = 1;
		t.height = 1;
		t.format = "RGBA32";
		let b: buffer_t = buffer_create(4);
		buffer_set_u8(b, 0, 0);
		buffer_set_u8(b, 1, 0);
		buffer_set_u8(b, 2, 0);
		buffer_set_u8(b, 3, 0);
		t._image = image_from_bytes(b, t.width, t.height, tex_format_t.RGBA32);
		map_set(render_path_render_targets, t.name, t);
	}

	///if is_sculpt
	{
		let t: render_target_t = render_target_create();
		t.name = "gbuffer0_undo";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "gbufferD_undo";
		t.width = 0;
		t.height = 0;
		t.format = "R32";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	///end

	if (config_raw.rp_ssao) {
		render_path_base_init_ssao();
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

	render_path_paint_init();

	///if (is_paint || is_sculpt)
	render_path_preview_init();
	///end

	render_path_raytrace_init();
}

function render_path_deferred_commands() {
	///if is_paint
	render_path_paint_live_brush_dirty();
	///end
	render_path_base_commands(render_path_deferred_draw_deferred);
}

function render_path_deferred_draw_deferred() {
	render_path_base_draw_ssao();
	render_path_base_draw_deferred_light();
	render_path_base_draw_bloom("tex");
	// draw_motion_blur();
	// draw_histogram();

	render_path_set_target("buf");
	render_path_bind_target("tex", "tex");
	// render_path_bind_target("histogram", "histogram");
	render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

	render_path_set_target("buf");
	render_path_base_draw_compass();
	render_path_draw_meshes("overlay");

	render_path_base_draw_taa();
}
