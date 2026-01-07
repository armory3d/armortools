
function tab_console_draw(htab: ui_handle_t) {
	let title: string = console_message_timer > 0 ? console_message + "        " : tr("Console");
	let color: i32    = console_message_timer > 0 ? console_message_color : -1;

	if (ui_tab(htab, title, false, color) && ui._window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		/// if (arm_windows || arm_linux || arm_macos) // Copy
		let row: f32[] = [ -100, -100, -100 ];
		/// else
		let row: f32[] = [ -100, -100 ];
		/// end
		ui_row(row);

		if (ui_icon_button(tr("Clear"), icon_t.ERASE)) {
			console_last_traces = [];
		}
		if (ui_icon_button(tr("Export"), icon_t.EXPORT)) {
			ui_files_show("txt", true, false, function(path: string) {
				let str: string = string_array_join(console_last_traces, "\n");
				let f: string   = ui_files_filename;
				if (f == "") {
					f = tr("untitled");
				}
				path = path + path_sep + f;
				if (!ends_with(path, ".txt")) {
					path += ".txt";
				}
				iron_file_save_bytes(path, sys_string_to_buffer(str), 0);
			});
		}
		/// if (arm_windows || arm_linux || arm_macos)
		if (ui_icon_button(tr("Copy"), icon_t.COPY)) {
			let str: string = string_array_join(console_last_traces, "\n");
			iron_copy_to_clipboard(str);
		}
		/// end

		ui_end_sticky();

		let _font: draw_font_t = ui.ops.font;
		let _font_size: i32    = ui.font_size;
		let f: draw_font_t     = data_get_font("font_mono.ttf");
		ui_set_font(ui, f);
		ui.font_size = math_floor(15 * UI_SCALE());
		for (let i: i32 = 0; i < console_last_traces.length; ++i) {
			let t: string = console_last_traces[i];
			ui_text(t);
		}
		ui_set_font(ui, _font);
		ui.font_size = _font_size;
	}
}
