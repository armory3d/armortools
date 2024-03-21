
function tab_console_draw(htab: zui_handle_t) {
	let ui: zui_t = ui_base_ui;

	let title: string = console_message_timer > 0 ? console_message + "        " : tr("Console");
	let color: i32 = console_message_timer > 0 ? console_message_color : -1;

	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (zui_tab(htab, title, false, color) && statush > ui_status_default_status_h * zui_SCALE(ui)) {

		zui_begin_sticky();
		///if (krom_windows || krom_linux || krom_darwin) // Copy
		if (config_raw.touch_ui) {
			zui_row([1 / 4, 1 / 4, 1 / 4]);
		}
		else {
			zui_row([1 / 14, 1 / 14, 1 / 14]);
		}
		///else
		if (config_raw.touch_ui) {
			zui_row([1 / 4, 1 / 4]);
		}
		else {
			zui_row([1 / 14, 1 / 14]);
		}
		///end

		if (zui_button(tr("Clear"))) {
			console_last_traces = [];
		}
		if (zui_button(tr("Export"))) {
			let str: string = console_last_traces.join("\n");
			ui_files_show("txt", true, false, function (path: string) {
				let f: string = ui_files_filename;
				if (f == "") {
					f = tr("untitled");
				}
				path = path + path_sep + f;
				if (!ends_with(path, ".txt")) {
					path += ".txt";
				}
				krom_file_save_bytes(path, sys_string_to_buffer(str));
			});
		}
		///if (krom_windows || krom_linux || krom_darwin)
		if (zui_button(tr("Copy"))) {
			let str: string = console_last_traces.join("\n");
			krom_copy_to_clipboard(str);
		}
		///end

		zui_end_sticky();

		let _font: g2_font_t = ui.font;
		let _fontSize: i32 = ui.font_size;
		let f: g2_font_t = data_get_font("font_mono.ttf");
		zui_set_font(ui, f);
		ui.font_size = math_floor(15 * zui_SCALE(ui));
		for (let i: i32 = 0; i < console_last_traces.length; ++i) {
			let t: string = console_last_traces[i];
			zui_text(t);
		}
		zui_set_font(ui, _font);
		ui.font_size = _fontSize;
	}
}
