
class ImportKeymap {

	static run = (path: string) => {
		if (!Path.isJson(path)) {
			Console.error(Strings.error1());
			return;
		}

		let filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		let dstPath = Path.data() + Path.sep + "keymap_presets" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		BoxPreferences.fetchKeymaps(); // Refresh file list
		BoxPreferences.presetHandle.position = BoxPreferences.getPresetIndex();
		Console.info(tr("Keymap imported:") + " " + filename);
	}
}
