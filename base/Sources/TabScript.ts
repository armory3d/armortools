
class TabScript {

	static tab_script_hscript: zui_handle_t = zui_handle_create();
	static tab_script_text_coloring: zui_text_coloring_t = null;

	static tab_script_draw = (htab: zui_handle_t) => {
		let ui: zui_t = ui_base_ui;
		let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Script")) && statush > ui_status_default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (config_raw.touch_ui) {
				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}
			if (zui_button(tr("Run"))) {
				try {
					eval(TabScript.tab_script_hscript.text);
				}
				catch(e: any) {
					console_log(e);
				}
			}
			if (zui_button(tr("Clear"))) {
				TabScript.tab_script_hscript.text = "";
			}
			if (zui_button(tr("Import"))) {
				ui_files_show("js", false, false, (path: string) => {
					let b: ArrayBuffer = data_get_blob(path);
					TabScript.tab_script_hscript.text = sys_buffer_to_string(b);
					data_delete_blob(path);
				});
			}
			if (zui_button(tr("Export"))) {
				let str: string = TabScript.tab_script_hscript.text;
				ui_files_show("js", true, false, (path: string) => {
					let f: string = ui_files_filename;
					if (f == "") f = tr("untitled");
					path = path + path_sep + f;
					if (!path.endsWith(".js")) path += ".js";
					krom_file_save_bytes(path, sys_string_to_buffer(str));
				});
			}
			zui_end_sticky();

			let _font: g2_font_t = ui.font;
			let _font_size: i32 = ui.font_size;
			let f: g2_font_t = data_get_font("font_mono.ttf");
			zui_set_font(ui, f);
			ui.font_size = math_floor(15 * zui_SCALE(ui));
			zui_set_text_area_line_numbers(true);
			zui_set_text_area_scroll_past_end(true);
			zui_set_text_area_coloring(TabScript.tab_script_get_text_coloring());
			zui_text_area(TabScript.tab_script_hscript);
			zui_set_text_area_line_numbers(false);
			zui_set_text_area_scroll_past_end(false);
			zui_set_text_area_coloring(null);
			zui_set_font(ui, _font);
			ui.font_size = _font_size;
		}
	}

	static tab_script_get_text_coloring = (): zui_text_coloring_t => {
		if (TabScript.tab_script_text_coloring == null) {
			let blob: ArrayBuffer = data_get_blob("text_coloring.json");
			TabScript.tab_script_text_coloring = json_parse(sys_buffer_to_string(blob));
			TabScript.tab_script_text_coloring.default_color = math_floor(TabScript.tab_script_text_coloring.default_color);
			for (let coloring of TabScript.tab_script_text_coloring.colorings) {
				coloring.color = math_floor(coloring.color);
			}
		}
		return TabScript.tab_script_text_coloring;
	}
}
