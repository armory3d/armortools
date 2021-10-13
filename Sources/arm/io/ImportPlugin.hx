package arm.io;

import arm.sys.Path;
import arm.sys.File;

class ImportPlugin {

	public static function run(path: String) {
		if (!Path.isPlugin(path)) {
			Console.error(Strings.error1());
			return;
		}

		var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		var dstPath = Path.data() + Path.sep + "plugins" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to plugin folder
		arm.ui.BoxPreferences.filesPlugin = null; // Refresh file list
		Console.info(tr("Plugin imported:") + " " + filename);
	}
}
