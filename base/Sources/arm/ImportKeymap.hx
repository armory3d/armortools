package arm;

class ImportKeymap {

	public static function run(path: String) {
		if (!Path.isJson(path)) {
			Console.error(Strings.error1());
			return;
		}

		var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		var dstPath = Path.data() + Path.sep + "keymap_presets" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		BoxPreferences.fetchKeymaps(); // Refresh file list
		BoxPreferences.presetHandle.position = BoxPreferences.getPresetIndex();
		Console.info(tr("Keymap imported:") + " " + filename);
	}
}
