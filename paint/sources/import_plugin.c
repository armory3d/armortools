void import_plugin_run(char *path) {
	if (!path_is_plugin(path)) {
		console_error(strings_unknown_asset_format());
		return;
	}

	char *filename = substring(path, string_last_index_of(path, PATH_SEP) + 1, string_length(path));
	char *dst_path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "plugins"), PATH_SEP), filename);
	file_copy(path, dst_path);           // Copy to plugin folder
	gc_unroot(box_preferences_files_plugin);
	box_preferences_files_plugin = NULL; // Refresh file list
	console_info(string_join(string_join(tr("Plugin imported:", NULL), " "), filename));
}
