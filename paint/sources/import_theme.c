void import_theme_run(string_t *path) {
	if (!path_is_json(path)) {
		console_error(strings_unknown_asset_format());
		return;
	}

	string_t *filename = substring(path, string_last_index_of(path, PATH_SEP) + 1, string_length(path));
	string_t *dst_path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "themes"), PATH_SEP), filename);
	file_copy(path, dst_path);      // Copy to preset folder
	box_preferences_fetch_themes(); // Refresh file list
	config_raw->theme          = string_copy(filename);
	box_preferences_h_theme->i = box_preferences_get_theme_index();
	config_load_theme(config_raw->theme, true);
	console_info(string_join(string_join(tr("Theme imported:", null), " "), filename));
}
