
class ImportTheme {

	static run = (path: string) => {
		if (!Path.isJson(path)) {
			Console.error(Strings.error1());
			return;
		}

		let filename = path.substr(path.lastIndexOf(Path.sep) + 1);
		let dstPath = Path.data() + Path.sep + "themes" + Path.sep + filename;
		File.copy(path, dstPath); // Copy to preset folder
		BoxPreferences.fetchThemes(); // Refresh file list
		Config.raw.theme = filename;
		BoxPreferences.themeHandle.position = BoxPreferences.getThemeIndex();
		Config.loadTheme(Config.raw.theme);
		Console.info(tr("Theme imported:") + " " + filename);
	}
}
