
class Translator {

	static translations: Map<string, string> = new Map();
	// The font index is a value specific to font_cjk.ttc.
	static cjk_font_indices: Map<string, i32> = new Map([
		["ja", 0],
		["ko", 1],
		["zh_cn", 2],
		["zh_tw", 3],
		["zh_tw.big5", 4]
	]);

	static last_locale: string = "en";

	// Mark strings as localizable in order to be parsed by the extract_locale script.
	// The string will not be translated to the currently selected locale though.
	static _tr = (s: string) => {
		return s;
	}

	// Localizes a string with the given placeholders replaced (format is `{placeholderName}`).
	// If the string isn't available in the translation, this method will return the source English string.
	static tr = (id: string, vars: Map<string, string> = null): string => {
		let translation: string = id;

		// English is the source language
		if (Config.raw.locale != "en" && Translator.translations.has(id)) {
			let atranslations: any = Translator.translations as any;
			translation = atranslations[id];
		}

		if (vars != null) {
			for (let [key, value] of vars) {
				translation = string_replace_all(translation, `{${key}}`, String(value));
			}
		}

		return translation;
	}

	// (Re)loads translations for the specified locale
	static load_translations = (new_locale: string) => {
		if (new_locale == "system") {
			Config.raw.locale = krom_language();
		}

		// Check whether the requested or detected locale is available
		if (Config.raw.locale != "en" && Translator.get_supported_locales().indexOf(Config.raw.locale) == -1) {
			// Fall back to English
			Config.raw.locale = "en";
		}

		// No translations to load, as source strings are in English
		// Clear existing translations if switching languages at runtime
		Translator.translations.clear();

		if (Config.raw.locale == "en" && Translator.last_locale == "en") {
			// No need to generate extended font atlas for English locale
			return;
		}
		Translator.last_locale = Config.raw.locale;

		if (Config.raw.locale != "en") {
			// Load the translation file
			let translation_json: string = sys_buffer_to_string(krom_load_blob(`data/locale/${Config.raw.locale}.json`));

			let data: any = json_parse(translation_json);
			for (let field in data) {
				let atranslations: any = Translator.translations as any;
				atranslations[field] = data[field];
			}
		}

		// Generate extended font atlas
		Translator.extended_glyphs();

		// Push additional char codes contained in translation file
		let cjk: bool = false;
		for (let s of Translator.translations.values()) {
			for (let i: i32 = 0; i < s.length; ++i) {
				// Assume cjk in the > 1119 range for now
				if (s.charCodeAt(i) > 1119 && _g2_font_glyphs.indexOf(s.charCodeAt(i)) == -1) {
					if (!cjk) {
						_g2_font_glyphs = _g2_make_glyphs(32, 127);
						cjk = true;
					}
					_g2_font_glyphs.push(s.charCodeAt(i));
				}
			}
		}

		if (cjk) {
			let cjk_font_path: string = (Path.is_protected() ? krom_save_path() : "") + "font_cjk.ttc";
			let cjk_font_disk_path: string = (Path.is_protected() ? krom_save_path() : Path.data() + Path.sep) + "font_cjk.ttc";
			if (!File.exists(cjk_font_disk_path)) {
				File.download("https://github.com/armory3d/armorbase/raw/main/Assets/common/extra/font_cjk.ttc", cjk_font_disk_path, () => {
					if (!File.exists(cjk_font_disk_path)) {
						// Fall back to English
						Config.raw.locale = "en";
						Translator.extended_glyphs();
						Translator.translations.clear();
						Translator.init_font(false, "font.ttf", 1.0);
					}
					else Translator.init_font(true, cjk_font_path, 1.4);
				}, 20332392);
			}
			else Translator.init_font(true, cjk_font_path, 1.4);
		}
		else Translator.init_font(false, "font.ttf", 1.0);
	}

	static init_font = (cjk: bool, fontPath: string, fontScale: f32) => {
		_g2_font_glyphs.sort((a: i32, b: i32) => { return a - b; });
		// Load and assign font with cjk characters
		app_notify_on_init(() => {
			let f: g2_font_t = data_get_font(fontPath);
			if (cjk) {
				let acjk_font_indices: any = Translator.cjk_font_indices as any;
				let font_index: i32 = Translator.cjk_font_indices.has(Config.raw.locale) ? acjk_font_indices[Config.raw.locale] : 0;
				g2_font_set_font_index(f, font_index);
			}
			base_font = f;
			// Scale up the font size and elements width a bit
			base_theme.FONT_SIZE = Math.floor(base_default_font_size * fontScale);
			base_theme.ELEMENT_W = Math.floor(base_default_element_w * (Config.raw.locale != "en" ? 1.4 : 1.0));
			let uis: zui_t[] = base_get_uis();
			for (let ui of uis) {
				zui_set_font(ui, f);
				zui_set_scale(ui, zui_SCALE(ui));
			}
		});
	}

	static extended_glyphs = () => {
		// Basic Latin + Latin-1 Supplement + Latin Extended-A
		_g2_font_glyphs = _g2_make_glyphs(32, 383);
		// + Greek
		for (let i: i32 = 880; i < 1023; ++i) _g2_font_glyphs.push(i);
		// + Cyrillic
		for (let i: i32 = 1024; i < 1119; ++i) _g2_font_glyphs.push(i);
	}

	// Returns a list of supported locales (plus English and the automatically detected system locale)
	static get_supported_locales = (): string[] => {
		let locales: string[] = ["system", "en"];
		for (let locale_filename of File.read_directory(Path.data() + Path.sep + "locale")) {
			// Trim the `.json` file extension from file names
			locales.push(locale_filename.substr(0, -5));
		}
		return locales;
	}
}
