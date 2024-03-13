
function import_plugin_run(path: string) {
	if (!path_is_plugin(path)) {
		console_error(strings_error1());
		return;
	}

	let filename: string = path.substr(path.lastIndexOf(path_sep) + 1);
	let dst_path: string = path_data() + path_sep + "plugins" + path_sep + filename;
	file_copy(path, dst_path); // Copy to plugin folder
	box_preferences_files_plugin = null; // Refresh file list
	console_info(tr("Plugin imported:") + " " + filename);
}
