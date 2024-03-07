
class ImportPlugin {

	static run = (path: string) => {
		if (!Path.is_plugin(path)) {
			Console.error(Strings.error1());
			return;
		}

		let filename: string = path.substr(path.lastIndexOf(Path.sep) + 1);
		let dstPath: string = Path.data() + Path.sep + "plugins" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to plugin folder
		BoxPreferences.files_plugin = null; // Refresh file list
		Console.info(tr("Plugin imported:") + " " + filename);
	}
}
