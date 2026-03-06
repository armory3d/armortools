void tab_console_draw(ui_handle_t *htab) {
	string_t *title = console_message_timer > 0 ? string_join(console_message, "        ") : tr("Console", null);
	i32       color = console_message_timer > 0 ? console_message_color : -1;

	if (ui_tab(htab, title, false, color, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS) // Copy
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		        -100,
		    },
		    3);
		#else
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		    },
		    2);
		#endif
		ui_row(row);

		if (ui_icon_button(tr("Clear", null), ICON_ERASE, UI_ALIGN_CENTER)) {
			gc_unroot(console_last_traces);
			console_last_traces = any_array_create_from_raw((any[]){}, 0);
			gc_root(console_last_traces);
		}
		if (ui_icon_button(tr("Export", null), ICON_EXPORT, UI_ALIGN_CENTER)) {
			ui_files_show("txt", true, false, &tab_console_draw_37642);
		}
		#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
		if (ui_icon_button(tr("Copy", null), ICON_COPY, UI_ALIGN_CENTER)) {
			string_t *str = string_array_join(console_last_traces, "\n");
			iron_copy_to_clipboard(str);
		}
		#endif

		ui_end_sticky();

		draw_font_t *_font      = ui->ops->font;
		i32          _font_size = ui->font_size;
		draw_font_t *f          = data_get_font("font_mono.ttf");
		ui_set_font(ui, f);
		ui->font_size = math_floor(15 * UI_SCALE());
		for (i32 i = 0; i < console_last_traces->length; ++i) {
			string_t *t = console_last_traces->buffer[i];
			ui_text(t, UI_ALIGN_LEFT, 0x00000000);
		}
		ui_set_font(ui, _font);
		ui->font_size = _font_size;
	}
}

void tab_console_draw_37642(string_t *path) {
	string_t *str = string_array_join(console_last_traces, "\n");
	string_t *f   = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled", null));
	}
	path = string_join(string_join(path, PATH_SEP), f);
	if (!ends_with(path, ".txt")) {
		path = string_join(path, ".txt");
	}
	iron_file_save_bytes(path, sys_string_to_buffer(str), 0);
}
