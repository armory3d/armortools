
function ui_base_ext_init_hwnd_tabs(): tab_draw_array_t[] {

	let a0: tab_draw_array_t = [];
	let a1: tab_draw_array_t = [];
	let a2: tab_draw_array_t = [
		_draw_callback_create(tab_browser_draw),
		_draw_callback_create(tab_meshes_draw),
		_draw_callback_create(tab_textures_draw),
		_draw_callback_create(tab_swatches_draw),
		_draw_callback_create(tab_plugins_draw),
		_draw_callback_create(tab_scripts_draw),
		_draw_callback_create(tab_console_draw),
		_draw_callback_create(ui_status_draw_version_tab)
	];

	let r: tab_draw_array_t[] = [];
	array_push(r, a0);
	array_push(r, a1);
	array_push(r, a2);
	return r;
}
