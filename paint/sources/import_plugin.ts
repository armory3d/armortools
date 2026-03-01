
function import_plugin_run(path: string) {
	if (!path_is_plugin(path)) {
		console_error(strings_unknown_asset_format());
		return;
	}

	let filename: string = substring(path, string_last_index_of(path, PATH_SEP) + 1, path.length);
	let dst_path: string = path_data() + PATH_SEP + "plugins" + PATH_SEP + filename;
	file_copy(path, dst_path);           // Copy to plugin folder
	box_preferences_files_plugin = null; // Refresh file list
	console_info(tr("Plugin imported:") + " " + filename);
}
