package arm.io;

import arm.sys.Path;
import arm.sys.File;

class ImportTheme {

	public static function run(path: String) {
		if (!Path.isJson(path)) {
			Console.error(Strings.error1());
			return;
		}

		var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		var dstPath = Path.data() + Path.sep + "themes" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		arm.ui.BoxPreferences.fetchThemes(); // Refresh file list
		Config.raw.theme = filename;
		arm.ui.BoxPreferences.themeHandle.position = arm.ui.BoxPreferences.getThemeIndex();
		Config.loadTheme(Config.raw.theme);
		Console.info(tr("Theme imported:") + " " + filename);
	}
}
