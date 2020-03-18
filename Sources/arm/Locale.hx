package arm;

class Locale {

	// Localize a string with the given placeholders replaced (format is `{placeholderName}`).
	// TODO: Implement localization support.
	public static function tr(id: String, ?vars: Map<String, Dynamic>): String {
		if (vars != null) {
			for (key => value in vars) {
				id = id.replace('{$key}', Std.string(value));
			}
		}

		return id;
	}
}
