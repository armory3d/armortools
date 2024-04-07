
let translator_translations: map_t<string, string> = map_create();
// The font index is a value specific to font_cjk.ttc
let translator_cjk_font_indices: map_t<string, i32> = null;

let translator_last_locale: string = "en";

// Mark strings as localizable in order to be parsed by the extract_locale script
// The string will not be translated to the currently selected locale though
function _tr(s: string) {
	return s;
}

// Localizes a string with the given placeholders replaced (format is "{placeholder_name}")
// If the string isn't available in the translation, this method will return the source English string
function tr(id: string, vars: map_t<string, string> = null): string {
	let translation: string = id;

	// English is the source language
	if (config_raw.locale != "en" && map_get(translator_translations, id) != null) {
		let atranslations: any = translator_translations as any;
		translation = atranslations[id];
	}

	if (vars != null) {
		let keys: string[] = map_keys_to_array(vars);
		let values: string[] = map_to_array(vars);
		for (let i: i32 = 0; i < keys.length; ++i) {
			translation = string_replace_all(translation, "{" + keys[i] + "}", any_to_string(values[i]));
		}
	}

	return translation;
}

// (Re)loads translations for the specified locale
function translator_load_translations(new_locale: string) {

	if (translator_cjk_font_indices == null) {
		translator_cjk_font_indices = map_create();
		map_set(translator_cjk_font_indices, "ja", 0);
		map_set(translator_cjk_font_indices, "ko", 1);
		map_set(translator_cjk_font_indices, "zh_cn", 2);
		map_set(translator_cjk_font_indices, "zh_tw", 3);
		map_set(translator_cjk_font_indices, "zh_tw.big5", 4);
	}

	if (new_locale == "system") {
		config_raw.locale = krom_language();
	}

	// Check whether the requested or detected locale is available
	if (config_raw.locale != "en" && array_index_of(translator_get_supported_locales(), config_raw.locale) == -1) {
		// Fall back to English
		config_raw.locale = "en";
	}

	// No translations to load, as source strings are in English
	// Clear existing translations if switching languages at runtime
	translator_translations.clear();

	if (config_raw.locale == "en" && translator_last_locale == "en") {
		// No need to generate extended font atlas for English locale
		return;
	}
	translator_last_locale = config_raw.locale;

	if (config_raw.locale != "en") {
		// Load the translation file
		let translation_json: string = sys_buffer_to_string(krom_load_blob("data/locale/" + config_raw.locale + ".json"));

		let data: any = json_parse(translation_json);
		for (let field in data) {
			let atranslations: any = translator_translations as any;
			atranslations[field] = data[field];
		}
	}

	// Generate extended font atlas
	translator_extended_glyphs();

	// Push additional char codes contained in translation file
	let cjk: bool = false;
	let values: string[] = map_to_array(translator_translations);
	for (let i: i32 = 0; i < values.length; ++i) {
		let s: string = values[i];
		for (let i: i32 = 0; i < s.length; ++i) {
			// Assume cjk in the > 1119 range for now
			if (char_code_at(s, i) > 1119 && array_index_of(_g2_font_glyphs, char_code_at(s, i)) == -1) {
				if (!cjk) {
					_g2_font_glyphs = _g2_make_glyphs(32, 127);
					cjk = true;
				}
				array_push(_g2_font_glyphs, char_code_at(s, i));
			}
		}
	}

	if (cjk) {
		let cjk_font_path: string = (path_is_protected() ? krom_save_path() : "") + "font_cjk.ttc";
		let cjk_font_disk_path: string = (path_is_protected() ? krom_save_path() : path_data() + path_sep) + "font_cjk.ttc";
		if (!file_exists(cjk_font_disk_path)) {
			file_download("https://github.com/armory3d/armorbase/raw/main/Assets/common/extra/font_cjk.ttc", cjk_font_disk_path, function () {

				let cjk_font_path: string = (path_is_protected() ? krom_save_path() : "") + "font_cjk.ttc";
				let cjk_font_disk_path: string = (path_is_protected() ? krom_save_path() : path_data() + path_sep) + "font_cjk.ttc";

				if (!file_exists(cjk_font_disk_path)) {
					// Fall back to English
					config_raw.locale = "en";
					translator_extended_glyphs();
					translator_translations.clear();
					translator_init_font(false, "font.ttf", 1.0);
				}
				else {
					translator_init_font(true, cjk_font_path, 1.4);
				}
			}, 20332392);
		}
		else {
			translator_init_font(true, cjk_font_path, 1.4);
		}
	}
	else {
		translator_init_font(false, "font.ttf", 1.0);
	}
}

let _translator_init_font_cjk: bool;
let _translator_init_font_font_path: string;
let _translator_init_font_font_scale: f32;

function translator_init_font(cjk: bool, font_path: string, font_scale: f32) {
	array_sort(_g2_font_glyphs, function (a: i32, b: i32) {
		return a - b;
	});

	_translator_init_font_cjk = cjk;
	_translator_init_font_font_path = font_path;
	_translator_init_font_font_scale = font_scale;

	// Load and assign font with cjk characters
	app_notify_on_init(function () {

		let cjk: bool = _translator_init_font_cjk;
		let font_path: string = _translator_init_font_font_path;
		let font_scale: f32 = _translator_init_font_font_scale;

		let f: g2_font_t = data_get_font(font_path);
		if (cjk) {
			let acjk_font_indices: any = translator_cjk_font_indices as any;
			let font_index: i32 = map_get(translator_cjk_font_indices, config_raw.locale) != null ? acjk_font_indices[config_raw.locale] : 0;
			g2_font_set_font_index(f, font_index);
		}
		base_font = f;
		// Scale up the font size and elements width a bit
		base_theme.FONT_SIZE = math_floor(base_default_font_size * font_scale);
		base_theme.ELEMENT_W = math_floor(base_default_element_w * (config_raw.locale != "en" ? 1.4 : 1.0));
		let uis: zui_t[] = base_get_uis();
		for (let i: i32 = 0; i < uis.length; ++i) {
			let ui: zui_t = uis[i];
			zui_set_font(ui, f);
			zui_set_scale(ui, zui_SCALE(ui));
		}
	});
}

function translator_extended_glyphs() {
	// Basic Latin + Latin-1 Supplement + Latin Extended-A
	_g2_font_glyphs = _g2_make_glyphs(32, 383);
	// + Greek
	for (let i: i32 = 880; i < 1023; ++i) {
		array_push(_g2_font_glyphs, i);
	}
	// + Cyrillic
	for (let i: i32 = 1024; i < 1119; ++i) {
		array_push(_g2_font_glyphs, i);
	}
}

// Returns a list of supported locales (plus English and the automatically detected system locale)
function translator_get_supported_locales(): string[] {
	let locales: string[] = ["system", "en"];
	for (let locale_filename of file_read_directory(path_data() + path_sep + "locale")) {
		// Trim the ".json" file extension from file names
		array_push(locales, substring(locale_filename, 0, locale_filename.length - 5));
	}
	return locales;
}
