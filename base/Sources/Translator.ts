
class Translator {

	static translations: Map<string, string> = new Map();
	// The font index is a value specific to font_cjk.ttc.
	static cjkFontIndices: Map<string, i32> = new Map([
		["ja", 0],
		["ko", 1],
		["zh_cn", 2],
		["zh_tw", 3],
		["zh_tw.big5", 4]
	]);

	static lastLocale = "en";

	// Mark strings as localizable in order to be parsed by the extract_locale script.
	// The string will not be translated to the currently selected locale though.
	static _tr = (s: string) => {
		return s;
	}

	// Localizes a string with the given placeholders replaced (format is `{placeholderName}`).
	// If the string isn't available in the translation, this method will return the source English string.
	static tr = (id: string, vars: Map<string, string> = null): string => {
		let translation = id;

		// English is the source language
		if (Config.raw.locale != "en" && Translator.translations.has(id)) {
			let atranslations = Translator.translations as any;
			translation = atranslations[id];
		}

		if (vars != null) {
			for (let [key, value] of vars) {
				translation = translation.replaceAll(`{${key}}`, String(value));
			}
		}

		return translation;
	}

	// (Re)loads translations for the specified locale
	static loadTranslations = (newLocale: string) => {
		if (newLocale == "system") {
			Config.raw.locale = krom_language();
		}

		// Check whether the requested or detected locale is available
		if (Config.raw.locale != "en" && Translator.getSupportedLocales().indexOf(Config.raw.locale) == -1) {
			// Fall back to English
			Config.raw.locale = "en";
		}

		// No translations to load, as source strings are in English
		// Clear existing translations if switching languages at runtime
		Translator.translations.clear();

		if (Config.raw.locale == "en" && Translator.lastLocale == "en") {
			// No need to generate extended font atlas for English locale
			return;
		}
		Translator.lastLocale = Config.raw.locale;

		if (Config.raw.locale != "en") {
			// Load the translation file
			let translationJson = sys_buffer_to_string(krom_load_blob(`data/locale/${Config.raw.locale}.json`));

			let data = JSON.parse(translationJson);
			for (let field in data) {
				let atranslations = Translator.translations as any;
				atranslations[field] = data[field];
			}
		}

		// Generate extended font atlas
		Translator.extendedGlyphs();

		// Push additional char codes contained in translation file
		let cjk = false;
		for (let s of Translator.translations.values()) {
			for (let i = 0; i < s.length; ++i) {
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
			let cjkFontPath = (Path.isProtected() ? krom_save_path() : "") + "font_cjk.ttc";
			let cjkFontDiskPath = (Path.isProtected() ? krom_save_path() : Path.data() + Path.sep) + "font_cjk.ttc";
			if (!File.exists(cjkFontDiskPath)) {
				File.download("https://github.com/armory3d/armorbase/raw/main/Assets/common/extra/font_cjk.ttc", cjkFontDiskPath, () => {
					if (!File.exists(cjkFontDiskPath)) {
						// Fall back to English
						Config.raw.locale = "en";
						Translator.extendedGlyphs();
						Translator.translations.clear();
						Translator.initFont(false, "font.ttf", 1.0);
					}
					else Translator.initFont(true, cjkFontPath, 1.4);
				}, 20332392);
			}
			else Translator.initFont(true, cjkFontPath, 1.4);
		}
		else Translator.initFont(false, "font.ttf", 1.0);
	}

	static initFont = (cjk: bool, fontPath: string, fontScale: f32) => {
		_g2_font_glyphs.sort((a: i32, b: i32) => { return a - b; });
		// Load and assign font with cjk characters
		app_notify_on_init(() => {
			data_get_font(fontPath, (f: g2_font_t) => {
				if (cjk) {
					let acjkFontIndices = Translator.cjkFontIndices as any;
					let fontIndex = Translator.cjkFontIndices.has(Config.raw.locale) ? acjkFontIndices[Config.raw.locale] : 0;
					g2_font_set_font_index(f, fontIndex);
				}
				Base.font = f;
				// Scale up the font size and elements width a bit
				Base.theme.FONT_SIZE = Math.floor(Base.defaultFontSize * fontScale);
				Base.theme.ELEMENT_W = Math.floor(Base.defaultElementW * (Config.raw.locale != "en" ? 1.4 : 1.0));
				let uis = Base.getUIs();
				for (let ui of uis) {
					zui_set_font(ui, f);
					zui_set_scale(ui, zui_SCALE(ui));
				}
			});
		});
	}

	static extendedGlyphs = () => {
		// Basic Latin + Latin-1 Supplement + Latin Extended-A
		_g2_font_glyphs = _g2_make_glyphs(32, 383);
		// + Greek
		for (let i = 880; i < 1023; ++i) _g2_font_glyphs.push(i);
		// + Cyrillic
		for (let i = 1024; i < 1119; ++i) _g2_font_glyphs.push(i);
	}

	// Returns a list of supported locales (plus English and the automatically detected system locale)
	static getSupportedLocales = (): string[] => {
		let locales = ["system", "en"];
		for (let localeFilename of File.readDirectory(Path.data() + Path.sep + "locale")) {
			// Trim the `.json` file extension from file names
			locales.push(localeFilename.substr(0, -5));
		}
		return locales;
	}
}
