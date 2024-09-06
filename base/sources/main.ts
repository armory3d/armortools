
function main() {

	// Used to locate external application data folder
	iron_set_app_name(manifest_title);
	config_load();

	app_on_resize = base_on_resize;
	app_on_w = base_w;
	app_on_h = base_h;
	app_on_x = base_x;
	app_on_y = base_y;

	context_init();
	config_init();
	sys_start(config_get_options());
	if (config_raw.layout == null) {
		base_init_layout();
	}
	iron_set_app_name(manifest_title);
	app_init();
	scene_set_active("Scene");
	uniforms_ext_init();
	render_path_base_init();

	if (context_raw.render_mode == render_mode_t.FORWARD) {
		render_path_deferred_init(); // Allocate gbuffer
		render_path_forward_init();
		render_path_commands = render_path_forward_commands;
	}
	else {
		render_path_deferred_init();
		render_path_commands = render_path_deferred_commands;
	}

	base_init();
}

main();
