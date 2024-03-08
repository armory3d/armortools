
class ImportKeymap {

	static run = (path: string) => {
		if (!Path.is_json(path)) {
			Console.error(Strings.error1());
			return;
		}

		let filename: string = path.substr(path.lastIndexOf(Path.sep) + 1);
		let dst_path: string = Path.data() + Path.sep + "keymap_presets" + Path.sep + filename;
		File.copy(path, dst_path); // Copy to preset folder
		BoxPreferences.fetch_keymaps(); // Refresh file list
		BoxPreferences.preset_handle.position = BoxPreferences.get_preset_index();
		Console.info(tr("Keymap imported:") + " " + filename);
	}
}
