
let translator_translations: map_t<string, string> = map_create();
// The font index is a value specific to font_cjk.ttc
let translator_cjk_font_indices: map_t<string, i32> = null;

let translator_last_locale: string = "en";

// Mark strings as localizable in order to be parsed by the extract_locale script
// The string will not be translated to the currently selected locale though
function _tr(s: string): string {
	return s;
}

// Localizes a string with the given placeholders replaced (format is "{placeholder_name}")
// If the string isn't available in the translation, this method will return the source English string
function tr(id: string, vars: map_t<string, string> = null): string {
	let translation: string = id;

	// English is the source language
	if (config_raw.locale != "en" && map_get(translator_translations, id) != null) {
		translation = map_get(translator_translations, id);
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

let _translator_load_translations_cjk_font_path: string;
let _translator_load_translations_cjk_font_disk_path: string;

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

		let data: map_t<string, string> = json_parse_to_map(translation_json);
		let keys: string[] = map_keys(data);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let field: string = keys[i];
			map_set(translator_translations, field, map_get(data, field));
		}
	}

	// Generate extended font atlas
	translator_extended_glyphs();

	// Push additional char codes contained in translation file
	let cjk: bool = false;
	let keys: string[] = map_keys(translator_translations);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let s: string = map_get(translator_translations, keys[i]);
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
		if (path_is_protected()) {
			_translator_load_translations_cjk_font_path = iron_save_path();
			_translator_load_translations_cjk_font_disk_path = iron_save_path();
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

let _translator_init_font_cjk: bool;
let _translator_init_font_font_path: string;
let _translator_init_font_font_scale: f32;

function translator_init_font(cjk: bool, font_path: string, font_scale: f32) {
	array_sort(_g2_font_glyphs, function (pa: u32_ptr, pb: u32_ptr): i32 {
		let a: i32 = (i32)DEREFERENCE(pa);
		let b: i32 = (i32)DEREFERENCE(pb);
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
			let font_index: i32 = map_get(translator_cjk_font_indices, config_raw.locale) != -1 ? map_get(translator_cjk_font_indices, config_raw.locale) : 0;
			g2_font_set_font_index(f, font_index);
		}
		base_font = f;
		// Scale up the font size and elements width a bit
		base_theme.FONT_SIZE = math_floor(base_default_font_size * font_scale);
		base_theme.ELEMENT_W = math_floor(base_default_element_w * (config_raw.locale != "en" ? 1.4 : 1.0));
		let uis: ui_t[] = base_get_uis();
		for (let i: i32 = 0; i < uis.length; ++i) {
			let ui: ui_t = uis[i];
			ui_set_font(ui, f);
			_ui_set_scale(ui, ui_SCALE(ui));
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
	let files: string[] = file_read_directory(path_data() + path_sep + "locale");
	for (let i: i32 = 0; i < files.length; ++i) {
		let locale_filename: string = files[i];
		// Trim the ".json" file extension from file names
		array_push(locales, substring(locale_filename, 0, locale_filename.length - 5));
	}
	return locales;
}
