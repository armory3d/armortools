
let translator_translations: map_t<string, string> = map_create();
// The font index is a value specific to font_cjk.ttc
let translator_cjk_font_indices: map_t<string, i32> = null;
let translator_last_locale: string = "en";
let _translator_load_translations_cjk_font_path: string;
let _translator_load_translations_cjk_font_disk_path: string;
let _translator_init_font_cjk: bool;
let _translator_init_font_font_path: string;
let _translator_init_font_font_scale: f32;

// Mark strings as localizable in order to be parsed by the extract_locale script
// The string will not be translated to the currently selected locale though
function _tr(s: string): string {
	return s;
}

// Localizes a string with the given placeholders replaced (format is "{placeholder_name}")
// If the string isn't available in the translation, this method will return the source English string
function tr(id: string, vars: map_t<string, string> = null): string {
	let translation: string = string_copy(id);

	// English is the source language
	if (config_raw.locale != "en") {
		if (string_index_of(id, "\n") > -1) {
			id = string_replace_all(id, "\n", "\\n");
		}
		let s: string = map_get(translator_translations, id);
		if (s != null) {
			if (string_index_of(s, "\\n") > -1) {
				s = string_replace_all(s, "\\n", "\n");
			}

			translation = s;
		}
	}

	if (vars != null) {
		let keys: string[] = map_keys(vars);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let search: string = "{" + keys[i] + "}";
			translation = string_replace_all(translation, search, map_get(vars, keys[i]));
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
		config_raw.locale = iron_language();
	}

	// Check whether the requested or detected locale is available
	if (config_raw.locale != "en" && array_index_of(translator_get_supported_locales(), config_raw.locale) == -1) {
		// Fall back to English
		config_raw.locale = "en";
	}

	// No translations to load, as source strings are in English
	// Clear existing translations if switching languages at runtime
	translator_translations = map_create();

	if (config_raw.locale == "en" && translator_last_locale == "en") {
		// No need to generate extended font atlas for English locale
		return;
	}
	translator_last_locale = config_raw.locale;

	if (config_raw.locale != "en") {
		// Load the translation file
		let translation_json: string = sys_buffer_to_string(iron_load_blob("data/locale/" + config_raw.locale + ".json"));
		translator_translations = json_parse_to_map(translation_json);
	}

	// Generate extended font atlas
	translator_extended_glyphs();

	// Push additional char codes contained in translation file
	let cjk: bool = false;
	let keys: string[] = map_keys(translator_translations);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let s: string = map_get(translator_translations, keys[i]);

		for (let i: i32 = 0; char_code_at(s, i) != 0; ) {
			let l: i32 = 0;
			let codepoint: i32 = string_utf8_decode((s) + i, ADDRESS(l));
			i += l;

			// Assume cjk in the > 1119 range
			if (codepoint > 1119 && !draw_font_has_glyph(codepoint)) {
				cjk = true;
				draw_font_add_glyph(codepoint);
			}
		}
	}

	if (cjk) {
		if (path_is_protected()) {
			_translator_load_translations_cjk_font_path = iron_internal_save_path();
			_translator_load_translations_cjk_font_disk_path = iron_internal_save_path();
		}
		else {
			_translator_load_translations_cjk_font_path = "";
			_translator_load_translations_cjk_font_disk_path = path_data() + path_sep;
		}
		_translator_load_translations_cjk_font_path += "font_cjk.ttc";
		_translator_load_translations_cjk_font_disk_path += "font_cjk.ttc";

		if (!file_exists(_translator_load_translations_cjk_font_disk_path)) {
			file_download("https://github.com/armory3d/armorbase/raw/main/Assets/common/extra/font_cjk.ttc", _translator_load_translations_cjk_font_disk_path, function () {

				if (!file_exists(_translator_load_translations_cjk_font_disk_path)) {
					// Fall back to English
					config_raw.locale = "en";
					translator_extended_glyphs();
					translator_translations = map_create();
					translator_init_font(false, "font.ttf", 1.0);
				}
				else {
					translator_init_font(true, _translator_load_translations_cjk_font_path, 1.4);
				}
			}, 20332392);
		}
		else {
			translator_init_font(true, _translator_load_translations_cjk_font_path, 1.4);
		}
	}
	else {
		translator_init_font(false, "font.ttf", 1.0);
	}
}

function translator_init_font(cjk: bool, font_path: string, font_scale: f32) {
	_translator_init_font_cjk = cjk;
	_translator_init_font_font_path = font_path;
	_translator_init_font_font_scale = font_scale;

	// Load and assign font with cjk characters
	sys_notify_on_next_frame(function () {

		let cjk: bool = _translator_init_font_cjk;
		let font_path: string = _translator_init_font_font_path;
		let font_scale: f32 = _translator_init_font_font_scale;

		let f: draw_font_t = data_get_font(font_path);
		if (cjk) {
			let font_index: i32 = map_get(translator_cjk_font_indices, config_raw.locale) != -1 ?
				map_get(translator_cjk_font_indices, config_raw.locale) : 0;
			f.index = font_index;
			f.glyphs_version = 0;
			draw_font_init(f);
		}
		base_font = f;
		// Scale up the font size and elements width a bit
		base_theme.FONT_SIZE = math_floor(base_default_font_size * font_scale);
		base_theme.ELEMENT_W = math_floor(base_default_element_w * (config_raw.locale != "en" ? 1.4 : 1.0));

		ui_set_font(ui, f);
		ui_set_scale(UI_SCALE());
	});
}

function translator_extended_glyphs() {
	// Basic Latin + Latin-1 Supplement + Latin Extended-A
	draw_font_init_glyphs(32, 383);
	// + Greek
	for (let i: i32 = 880; i < 1023; ++i) {
		draw_font_add_glyph(i);
	}
	// + Cyrillic
	for (let i: i32 = 1024; i < 1119; ++i) {
		draw_font_add_glyph(i);
	}
}

// Returns a list of supported locales (plus English and the automatically detected system locale)
function translator_get_supported_locales(): string[] {
	let locales: string[] = ["system", "en"];
	let files: string[] = file_read_directory(path_data() + path_sep + "locale");
	for (let i: i32 = 0; i < files.length; ++i) {
		let locale_filename: string = files[i];
		// Trim the ".json" file extension from file names
		array_push(locales, substring(locale_filename, 0, locale_filename.length - 5));
	}
	return locales;
}
