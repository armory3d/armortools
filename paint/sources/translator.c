
// Mark strings as localizable in order to be parsed by the extract_locale script
// The string will not be translated to the currently selected locale though
string_t *_tr(string_t *s) {
	return s;
}

// Localizes a string with the given placeholders replaced (format is "{placeholder_name}")
// If the string isn't available in the translation, this method will return the source English string
string_t *tr(string_t *id, any_map_t *vars) {
	string_t *translation = string_copy(id);

	// English is the source language
	if (!string_equals(config_raw->locale, "en")) {
		if (string_index_of(id, "\n") > -1) {
			id = string_copy(string_replace_all(id, "\n", "\\n"));
		}
		string_t *s = any_map_get(translator_translations, id);
		if (s != null) {
			if (string_index_of(s, "\\n") > -1) {
				s = string_copy(string_replace_all(s, "\\n", "\n"));
			}
			translation = string_copy(s);
		}
	}

	if (vars != null) {
		string_t_array_t *keys = map_keys(vars);
		for (i32 i = 0; i < keys->length; ++i) {
			string_t *search = string_join(string_join("{", keys->buffer[i]), "}");
			translation      = string_copy(string_replace_all(translation, search, any_map_get(vars, keys->buffer[i])));
		}
	}

	return translation;
}

// (Re)loads translations for the specified locale
void translator_load_translations(string_t *new_locale) {

	if (translator_cjk_font_indices == null) {
		gc_unroot(translator_cjk_font_indices);
		translator_cjk_font_indices = i32_map_create();
		gc_root(translator_cjk_font_indices);
		i32_map_set(translator_cjk_font_indices, "ja", 0);
		i32_map_set(translator_cjk_font_indices, "ko", 1);
		i32_map_set(translator_cjk_font_indices, "zh_cn", 2);
		i32_map_set(translator_cjk_font_indices, "zh_tw", 3);
		i32_map_set(translator_cjk_font_indices, "zh_tw.big5", 4);
	}

	if (string_equals(new_locale, "system")) {
		config_raw->locale = string_copy(iron_language());
	}

	// Check whether the requested or detected locale is available
	if (!string_equals(config_raw->locale, "en") && char_ptr_array_index_of(translator_get_supported_locales(), config_raw->locale) == -1) {
		// Fall back to English
		config_raw->locale = "en";
	}

	// No translations to load, as source strings are in English
	// Clear existing translations if switching languages at runtime
	gc_unroot(translator_translations);
	translator_translations = any_map_create();
	gc_root(translator_translations);

	if (string_equals(config_raw->locale, "en") && string_equals(translator_last_locale, "en")) {
		// No need to generate extended font atlas for English locale
		return;
	}

	gc_unroot(translator_last_locale);
	translator_last_locale = string_copy(config_raw->locale);
	gc_root(translator_last_locale);

	if (!string_equals(config_raw->locale, "en")) {
		// Load the translation file
		string_t *translation_json = sys_buffer_to_string(iron_load_blob(string_join(string_join("data/locale/", config_raw->locale), ".json")));
		gc_unroot(translator_translations);
		translator_translations = json_parse_to_map(translation_json);
		gc_root(translator_translations);
	}

	// Generate extended font atlas
	translator_extended_glyphs();

	// Push additional char codes contained in translation file
	bool              cjk  = false;
	string_t_array_t *keys = map_keys(translator_translations);
	for (i32 i = 0; i < keys->length; ++i) {
		string_t *s = any_map_get(translator_translations, keys->buffer[i]);

		for (i32 i = 0; char_code_at(s, i) != 0;) {
			i32 l         = 0;
			i32 codepoint = string_utf8_decode((s) + i, ADDRESS(l));
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
			gc_unroot(_translator_load_translations_cjk_font_path);
			_translator_load_translations_cjk_font_path = string_copy(iron_internal_save_path());
			gc_root(_translator_load_translations_cjk_font_path);
			gc_unroot(_translator_load_translations_cjk_font_disk_path);
			_translator_load_translations_cjk_font_disk_path = string_copy(iron_internal_save_path());
			gc_root(_translator_load_translations_cjk_font_disk_path);
		}
		else {
			gc_unroot(_translator_load_translations_cjk_font_path);
			_translator_load_translations_cjk_font_path = "";
			gc_root(_translator_load_translations_cjk_font_path);
			gc_unroot(_translator_load_translations_cjk_font_disk_path);
			_translator_load_translations_cjk_font_disk_path = string_join(path_data(), PATH_SEP);
			gc_root(_translator_load_translations_cjk_font_disk_path);
		}
		gc_unroot(_translator_load_translations_cjk_font_path);
		_translator_load_translations_cjk_font_path = string_join(_translator_load_translations_cjk_font_path, "font_cjk.ttc");
		gc_root(_translator_load_translations_cjk_font_path);
		gc_unroot(_translator_load_translations_cjk_font_disk_path);
		_translator_load_translations_cjk_font_disk_path = string_join(_translator_load_translations_cjk_font_disk_path, "font_cjk.ttc");
		gc_root(_translator_load_translations_cjk_font_disk_path);

		if (!iron_file_exists(_translator_load_translations_cjk_font_disk_path)) {
			file_download_to("https://github.com/armory3d/armorbase/raw/main/Assets/common/extra/font_cjk.ttc",
			                 _translator_load_translations_cjk_font_disk_path, &translator_load_translations_91848, 20332392);
		}
		else {
			translator_init_font(true, _translator_load_translations_cjk_font_path, 1.4);
		}
	}
	else {
		translator_init_font(false, "font.ttf", 1.0);
	}
}

void translator_load_translations_91848(string_t *url) {
	if (!iron_file_exists(_translator_load_translations_cjk_font_disk_path)) {
		// Fall back to English
		config_raw->locale = "en";
		translator_extended_glyphs();
		gc_unroot(translator_translations);
		translator_translations = any_map_create();
		gc_root(translator_translations);
		translator_init_font(false, "font.ttf", 1.0);
	}
	else {
		translator_init_font(true, _translator_load_translations_cjk_font_path, 1.4);
	}
}

void translator_init_font(bool cjk, string_t *font_path, f32 font_scale) {
	_translator_init_font_cjk = cjk;
	gc_unroot(_translator_init_font_font_path);
	_translator_init_font_font_path = string_copy(font_path);
	gc_root(_translator_init_font_font_path);
	_translator_init_font_font_scale = font_scale;

	// Load and assign font with cjk characters
	sys_notify_on_next_frame(&translator_init_font_91962, null);
}

void translator_init_font_91962(any _) {
	bool         cjk        = _translator_init_font_cjk;
	string_t    *font_path  = _translator_init_font_font_path;
	f32          font_scale = _translator_init_font_font_scale;

	draw_font_t *f          = data_get_font(font_path);
	if (cjk) {
		i32 font_index = i32_map_get(translator_cjk_font_indices, config_raw->locale) != -1 ? i32_map_get(translator_cjk_font_indices, config_raw->locale) : 0;
		f->index       = font_index;
		f->glyphs_version = 0;
		draw_font_init(f);
	}
	gc_unroot(base_font);
	base_font = f;

	// Scale up the font size and elements width a bit
	gc_root(base_font);
	base_theme->FONT_SIZE = math_floor(base_default_font_size * font_scale);
	base_theme->ELEMENT_W = math_floor(base_default_element_w * (!string_equals(config_raw->locale, "en") ? 1.4 : 1.0));

	ui_set_font(ui, f);
	ui_set_scale(UI_SCALE());
}

void translator_extended_glyphs() {
	// Basic Latin + Latin-1 Supplement + Latin Extended-A
	draw_font_init_glyphs(32, 383);
	// + Greek
	for (i32 i = 880; i < 1023; ++i) {
		draw_font_add_glyph(i);
	}
	// + Cyrillic
	for (i32 i = 1024; i < 1119; ++i) {
		draw_font_add_glyph(i);
	}
}

// Returns a list of supported locales (plus English and the automatically detected system locale)
string_t_array_t *translator_get_supported_locales() {
	string_t_array_t *locales = any_array_create_from_raw(
	    (any[]){
	        "system",
	        "en",
	    },
	    2);
	string_t_array_t *files = file_read_directory(string_join(string_join(path_data(), PATH_SEP), "locale"));
	for (i32 i = 0; i < files->length; ++i) {
		string_t *locale_filename = files->buffer[i];
		// Trim the ".json" file extension from file names
		any_array_push(locales, substring(locale_filename, 0, string_length(locale_filename) - 5));
	}
	return locales;
}
