package arm.io;

import arm.sys.Path;
import arm.sys.File;

class ImportTheme {

	public static function run(path: String) {
		if (!Path.isJson(path)) {
			Log.error(Strings.error1);
			return;
		}

		var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		var dstPath = Path.data() + Path.sep + "themes" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		arm.ui.BoxPreferences.fetchThemes(); // Refresh file list
		Config.raw.theme = filename;
		arm.ui.BoxPreferences.themeHandle.position = arm.ui.BoxPreferences.getThemeIndex();
		arm.ui.BoxPreferences.loadTheme(Config.raw.theme);
		Log.info("Theme '" + filename + "' imported.");
	}
}
