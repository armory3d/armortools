package arm.io;

import iron.data.Data;
import arm.util.Path;
using StringTools;

class ImportPlugin {

	public static function run(path:String) {
		if (!Path.isPlugin(path)) {
			Log.showError(Strings.error1);
			return;
		}

		#if krom_windows
		var sep = "\\";
		var copy = "copy";
		var dataPath = Data.dataPath.replace("/", "\\");
		#else
		var sep = "/";
		var copy = "cp";
		var dataPath = Data.dataPath;
		#end

		var filename = path.substr(path.lastIndexOf(sep) + 1);
		var dest = Krom.getFilesLocation() + sep + dataPath + sep + "plugins" + sep + filename;

		Krom.sysCommand(copy + ' ' + path + ' ' + dest); // Copy to plugin folder
		arm.ui.BoxPreferences.files = null; // Refresh file list

		Log.showMessage("Plugin '" + filename + "' installed.");
	}
}
