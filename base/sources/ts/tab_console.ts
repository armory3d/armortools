
function tab_console_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;

	let title: string = console_message_timer > 0 ? console_message + "        " : tr("Console");
	let color: i32 = console_message_timer > 0 ? console_message_color : -1;

	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_tab(htab, title, false, color) && statush > ui_status_default_status_h * ui_SCALE(ui)) {

		ui_begin_sticky();
		///if (arm_windows || arm_linux || arm_macos) // Copy
		if (config_raw.touch_ui) {
			let row: f32[] = [1 / 4, 1 / 4, 1 / 4];
			ui_row(row);
		}
		else {
			let row: f32[] = [1 / 14, 1 / 14, 1 / 14];
			ui_row(row);
		}
		///else
		if (config_raw.touch_ui) {
			let row: f32[] = [1 / 4, 1 / 4];
			ui_row(row);
		}
		else {
			let row: f32[] = [1 / 14, 1 / 14];
			ui_row(row);
		}
		///end

		if (ui_button(tr("Clear"))) {
			console_last_traces = [];
		}
		if (ui_button(tr("Export"))) {
			ui_files_show("txt", true, false, function (path: string) {
				let str: string = string_array_join(console_last_traces, "\n");
				let f: string = ui_files_filename;
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
		///if (arm_windows || arm_linux || arm_macos)
		if (ui_button(tr("Copy"))) {
			let str: string = string_array_join(console_last_traces, "\n");
			iron_copy_to_clipboard(str);
		}
		///end

		ui_end_sticky();

		let _font: g2_font_t = ui.ops.font;
		let _font_size: i32 = ui.font_size;
		let f: g2_font_t = data_get_font("font_mono.ttf");
		ui_set_font(ui, f);
		ui.font_size = math_floor(15 * ui_SCALE(ui));
		for (let i: i32 = 0; i < console_last_traces.length; ++i) {
			let t: string = console_last_traces[i];
			ui_text(t);
		}
		ui_set_font(ui, _font);
		ui.font_size = _font_size;
	}
}
