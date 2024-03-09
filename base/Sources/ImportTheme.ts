
class ImportTheme {

	static run = (path: string) => {
		if (!path_is_json(path)) {
			console_error(strings_error1());
			return;
		}

		let filename: string = path.substr(path.lastIndexOf(path_sep) + 1);
		let dst_path: string = path_data() + path_sep + "themes" + path_sep + filename;
		file_copy(path, dst_path); // Copy to preset folder
		BoxPreferences.fetch_themes(); // Refresh file list
		config_raw.theme = filename;
		BoxPreferences.theme_handle.position = BoxPreferences.get_theme_index();
		config_load_theme(config_raw.theme);
		console_info(tr("Theme imported:") + " " + filename);
	}
}
