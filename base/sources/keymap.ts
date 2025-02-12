
function keymap_load() {
	config_keymap = keymap_get_default();
	if (config_raw.keymap != "default.json") {
		let blob: buffer_t = data_get_blob("keymap_presets/" + config_raw.keymap);
		let new_keymap: map_t<string, string> = json_parse_to_map(sys_buffer_to_string(blob));
		let keys: string[] = map_keys(new_keymap);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let key: string = keys[i];
			map_set(config_keymap, key, map_get(new_keymap, key));
		}
	}
}

function keymap_save() {
	if (config_raw.keymap == "default.json") {
		return;
	}
	let path: string = data_path() + "keymap_presets/" + config_raw.keymap;
	let buffer: buffer_t = sys_string_to_buffer(keymap_to_json(config_keymap));
	iron_file_save_bytes(path, buffer, 0);
}

function keymap_to_json(keymap: map_t<string, string>): string {
	json_encode_begin();
	json_encode_map(keymap);
	return json_encode_end();
}

function keymap_get_default(): map_t<string, string> {
	let keymap: map_t<string, string> = map_create();
	map_set(keymap, "action_paint", "left");
	map_set(keymap, "action_rotate", "alt+left");
	map_set(keymap, "action_pan", "alt+middle");
	map_set(keymap, "action_zoom", "alt+right");
	map_set(keymap, "rotate_envmap", "ctrl+middle");
	map_set(keymap, "set_clone_source", "alt");
	map_set(keymap, "stencil_transform", "ctrl");
	map_set(keymap, "stencil_hide", "z");
	map_set(keymap, "brush_radius", "f");
	map_set(keymap, "brush_radius_decrease", "[");
	map_set(keymap, "brush_radius_increase", "]");
	map_set(keymap, "brush_ruler", "shift");
	map_set(keymap, "file_new", "ctrl+n");
	map_set(keymap, "file_open", "ctrl+o");
	map_set(keymap, "file_open_recent", "ctrl+shift+o");
	map_set(keymap, "file_save", "ctrl+s");
	map_set(keymap, "file_save_as", "ctrl+shift+s");
	map_set(keymap, "file_reimport_mesh", "ctrl+r");
	map_set(keymap, "file_reimport_textures", "ctrl+shift+r");
	map_set(keymap, "file_import_assets", "ctrl+i");
	map_set(keymap, "file_export_textures", "ctrl+e");
	map_set(keymap, "file_export_textures_as", "ctrl+shift+e");
	map_set(keymap, "edit_undo", "ctrl+z");
	map_set(keymap, "edit_redo", "ctrl+shift+z");
	map_set(keymap, "edit_prefs", "ctrl+k");
	map_set(keymap, "view_reset", "0");
	map_set(keymap, "view_front", "1");
	map_set(keymap, "view_back", "ctrl+1");
	map_set(keymap, "view_right", "3");
	map_set(keymap, "view_left", "ctrl+3");
	map_set(keymap, "view_top", "7");
	map_set(keymap, "view_bottom", "ctrl+7");
	map_set(keymap, "view_camera_type", "5");
	map_set(keymap, "view_orbit_left", "4");
	map_set(keymap, "view_orbit_right", "6");
	map_set(keymap, "view_orbit_up", "8");
	map_set(keymap, "view_orbit_down", "2");
	map_set(keymap, "view_orbit_opposite", "9");
	map_set(keymap, "view_zoom_in", "");
	map_set(keymap, "view_zoom_out", "");
	map_set(keymap, "view_distract_free", "f11");
	map_set(keymap, "viewport_mode", "ctrl+m");
	map_set(keymap, "toggle_node_editor", "tab");
	map_set(keymap, "toggle_2d_view", "shift+tab");
	map_set(keymap, "toggle_browser", "`");
	map_set(keymap, "node_search", "space");
	map_set(keymap, "operator_search", "space");
	map_set(keymap, "decal_mask", "ctrl");
	map_set(keymap, "select_material", "shift+number");
	map_set(keymap, "select_layer", "alt+number");
	map_set(keymap, "brush_opacity", "shift+f");
	map_set(keymap, "brush_angle", "alt+f");
	map_set(keymap, "tool_brush", "b");
	map_set(keymap, "tool_eraser", "e");
	map_set(keymap, "tool_fill", "g");
	map_set(keymap, "tool_decal", "d");
	map_set(keymap, "tool_text", "t");
	map_set(keymap, "tool_clone", "l");
	map_set(keymap, "tool_blur", "u");
	map_set(keymap, "tool_smudge", "m");
	map_set(keymap, "tool_particle", "p");
	map_set(keymap, "tool_colorid", "c");
	map_set(keymap, "tool_picker", "v");
	map_set(keymap, "tool_bake", "k");
	map_set(keymap, "tool_gizmo", "");
	map_set(keymap, "tool_material", "");
	map_set(keymap, "swap_brush_eraser", "");
	return keymap;
}
