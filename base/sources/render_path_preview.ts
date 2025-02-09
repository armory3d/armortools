
function render_path_preview_init() {

	{
		let t: render_target_t = render_target_create();
		t.name = "texpreview";
		t.width = 1;
		t.height = 1;
		t.format = "RGBA32";
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "texpreview_icon";
		t.width = 1;
		t.height = 1;
		t.format = "RGBA32";
		render_path_create_render_target(t);
	}

	render_path_create_depth_buffer("mmain", "DEPTH24");

	{
		let t: render_target_t = render_target_create();
		t.name = "mtex";
		t.width = math_floor(util_render_material_preview_size * 2.0);
		t.height = math_floor(util_render_material_preview_size * 2.0);
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}

	{
		let t: render_target_t = render_target_create();
		t.name = "mgbuffer0";
		t.width = math_floor(util_render_material_preview_size * 2.0);
		t.height = math_floor(util_render_material_preview_size * 2.0);
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		t.depth_buffer = "mmain";
		render_path_create_render_target(t);
	}

	{
		let t: render_target_t = render_target_create();
		t.name = "mgbuffer1";
		t.width = math_floor(util_render_material_preview_size * 2.0);
		t.height = math_floor(util_render_material_preview_size * 2.0);
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}

	{
		let t: render_target_t = render_target_create();
		t.name = "mgbuffer2";
		t.width = math_floor(util_render_material_preview_size * 2.0);
		t.height = math_floor(util_render_material_preview_size * 2.0);
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
}

function render_path_preview_commands_preview() {
	render_path_set_target("mgbuffer2");
	render_path_clear_target(0xff000000);

	render_path_set_target("mgbuffer0");
	render_path_clear_target(0xffffffff, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
	let additional: string[] = ["mgbuffer1", "mgbuffer2"];
	render_path_set_target("mgbuffer0", additional);
	render_path_draw_meshes("mesh");

	// Deferred light
	render_path_set_target("mtex");
	render_path_bind_target("_mmain", "gbufferD");
	render_path_bind_target("mgbuffer0", "gbuffer0");
	render_path_bind_target("mgbuffer1", "gbuffer1");
	{
		render_path_bind_target("empty_white", "ssaotex");
	}
	render_path_draw_shader("shader_datas/deferred_light/deferred_light");

	render_path_set_depth_from("mtex", "mgbuffer0"); // Bind depth for world pass

	render_path_set_target("mtex"); // Re-binds depth
	render_path_draw_skydome("shader_datas/world_pass/world_pass");

	render_path_set_depth_from("mtex", "mgbuffer1"); // Unbind depth

	let framebuffer: string = "texpreview";
	let selected_mat: slot_material_t = context_raw.material;
	let texpreview: render_target_t = map_get(render_path_render_targets, "texpreview");
	let texpreview_icon: render_target_t = map_get(render_path_render_targets, "texpreview_icon");
	texpreview._image = selected_mat.image;
	texpreview_icon._image = selected_mat.image_icon;

	render_path_set_target(framebuffer);
	render_path_bind_target("mtex", "tex");
	render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

	render_path_set_target("texpreview_icon");
	render_path_bind_target("texpreview", "tex");
	render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");
}

function render_path_preview_commands_decal() {
	render_path_set_target("gbuffer2");
	render_path_clear_target(0xff000000);

	render_path_set_target("gbuffer0");
	render_path_clear_target(0xffffffff, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
	let additional: string[] = ["gbuffer1", "gbuffer2"];
	render_path_set_target("gbuffer0", additional);
	render_path_draw_meshes("mesh");

	// Deferred light
	render_path_set_target("tex");
	render_path_bind_target("_main", "gbufferD");
	render_path_bind_target("gbuffer0", "gbuffer0");
	render_path_bind_target("gbuffer1", "gbuffer1");
	{
		render_path_bind_target("empty_white", "ssaotex");
	}
	render_path_draw_shader("shader_datas/deferred_light/deferred_light");

	render_path_set_depth_from("tex", "gbuffer0"); // Bind depth for world pass

	render_path_set_target("tex");
	render_path_draw_skydome("shader_datas/world_pass/world_pass");

	render_path_set_depth_from("tex", "gbuffer1"); // Unbind depth

	let framebuffer: string = "texpreview";
	let texpreview: render_target_t = map_get(render_path_render_targets, "texpreview");
	texpreview._image = context_raw.decal_image;

	render_path_set_target(framebuffer);

	render_path_bind_target("tex", "tex");
	render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");
}
