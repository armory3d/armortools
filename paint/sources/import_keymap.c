void import_keymap_run(char *path) {
	if (!path_is_json(path)) {
		console_error(strings_unknown_asset_format());
		return;
	}

	char *filename = substring(path, string_last_index_of(path, PATH_SEP) + 1, string_length(path));
	char *dst_path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "keymap_presets"), PATH_SEP), filename);
	file_copy(path, dst_path);       // Copy to preset folder
	box_preferences_fetch_keymaps(); // Refresh file list
	box_preferences_h_preset->i = box_preferences_get_preset_index();
	console_info(string_join(string_join(tr("Keymap imported:", NULL), " "), filename));
}
