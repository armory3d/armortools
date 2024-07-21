
function import_theme_run(path: string) {
	if (!path_is_json(path)) {
		console_error(strings_error1());
		return;
	}

	let filename: string = substring(path, string_last_index_of(path, path_sep) + 1, path.length);
	let dst_path: string = path_data() + path_sep + "themes" + path_sep + filename;
	file_copy(path, dst_path); // Copy to preset folder
	box_preferences_fetch_themes(); // Refresh file list
	config_raw.theme = filename;
	box_preferences_theme_handle.position = box_preferences_get_theme_index();
	config_load_theme(config_raw.theme);
	console_info(tr("Theme imported:") + " " + filename);
}
