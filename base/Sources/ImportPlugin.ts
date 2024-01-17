
class ImportPlugin {

	static run = (path: string) => {
		if (!Path.isPlugin(path)) {
			Console.error(Strings.error1());
			return;
		}

		let filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		let dstPath = Path.data() + Path.sep + "plugins" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to plugin folder
		BoxPreferences.filesPlugin = null; // Refresh file list
		Console.info(tr("Plugin imported:") + " " + filename);
	}
}
