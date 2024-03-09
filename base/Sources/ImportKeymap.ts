
class ImportKeymap {

	static run = (path: string) => {
		if (!path_is_json(path)) {
			console_error(strings_error1());
			return;
		}

		let filename: string = path.substr(path.lastIndexOf(path_sep) + 1);
		let dst_path: string = path_data() + path_sep + "keymap_presets" + path_sep + filename;
		file_copy(path, dst_path); // Copy to preset folder
		BoxPreferences.fetch_keymaps(); // Refresh file list
		BoxPreferences.preset_handle.position = BoxPreferences.get_preset_index();
		console_info(tr("Keymap imported:") + " " + filename);
	}
}
