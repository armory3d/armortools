package arm;

class Translator {

	// Localizes a string with the given placeholders replaced (format is `{placeholderName}`).
	// If the string isn't available in the translation, this method will return the source English string.
	public static function tr(id: String, ?vars: Map<String, Dynamic>): String {
		var translation = id;

		// English is the source language
		if (App.locale != "en" && App.translations != null && App.translations.exists(id)) {
			translation = App.translations[id];
		}

		if (vars != null) {
			for (key => value in vars) {
				translation = translation.replace('{$key}', Std.string(value));
			}
		}

		return translation;
	}
}
