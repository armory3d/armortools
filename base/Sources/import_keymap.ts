
function import_keymap_run(path: string) {
	if (!path_is_json(path)) {
		console_error(strings_error1());
		return;
	}

	let filename: string = path.substr(path.lastIndexOf(path_sep) + 1);
	let dst_path: string = path_data() + path_sep + "keymap_presets" + path_sep + filename;
	file_copy(path, dst_path); // Copy to preset folder
	box_preferences_fetch_keymaps(); // Refresh file list
	box_preferences_preset_handle.position = box_preferences_get_preset_index();
	console_info(tr("Keymap imported:") + " " + filename);
}
