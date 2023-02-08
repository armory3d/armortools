package arm.io;

import arm.sys.Path;
import arm.sys.File;

class ImportKeymap {

	public static function run(path: String) {
		if (!Path.isJson(path)) {
			Console.error(Strings.error1());
			return;
		}

		var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		var dstPath = Path.data() + Path.sep + "keymap_presets" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		arm.ui.BoxPreferences.fetchKeymaps(); // Refresh file list
		arm.ui.BoxPreferences.presetHandle.position = arm.ui.BoxPreferences.getPresetIndex();
		Console.info(tr("Keymap imported:") + " " + filename);
	}
}
