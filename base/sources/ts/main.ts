
function main() {

	// Used to locate external application data folder
	iron_set_app_name(manifest_title);
	config_load();

	sys_on_resize = base_on_resize;
	sys_on_w = base_w;
	sys_on_h = base_h;
	sys_on_x = base_x;
	sys_on_y = base_y;

	config_init();
	context_init();
	sys_start(config_get_options());
	if (config_raw.layout == null) {
		base_init_layout();
	}
	iron_set_app_name(manifest_title);
	scene_set_active("Scene");
	uniforms_ext_init();
	render_path_base_init();
	render_path_deferred_init(); // Allocate gbuffer

	if (context_raw.render_mode == render_mode_t.FORWARD) {
		render_path_forward_init();
		render_path_commands = render_path_forward_commands;
	}
	else {
		render_path_commands = render_path_deferred_commands;
	}

	base_init();
}

main();
