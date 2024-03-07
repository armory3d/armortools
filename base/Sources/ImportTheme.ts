
class ImportTheme {

	static run = (path: string) => {
		if (!Path.is_json(path)) {
			Console.error(Strings.error1());
			return;
		}

		let filename: string = path.substr(path.lastIndexOf(Path.sep) + 1);
		let dstPath: string = Path.data() + Path.sep + "themes" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		BoxPreferences.fetch_themes(); // Refresh file list
		Config.raw.theme = filename;
		BoxPreferences.theme_handle.position = BoxPreferences.get_theme_index();
		Config.load_theme(Config.raw.theme);
		Console.info(tr("Theme imported:") + " " + filename);
	}
}
