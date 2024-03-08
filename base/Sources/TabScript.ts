
class TabScript {

	static hscript: zui_handle_t = zui_handle_create();
	static text_coloring: zui_text_coloring_t = null;

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Script")) && statush > UIStatus.default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (Config.raw.touch_ui) {
				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}
			if (zui_button(tr("Run"))) {
				try {
					eval(TabScript.hscript.text);
				}
				catch(e: any) {
					Console.log(e);
				}
			}
			if (zui_button(tr("Clear"))) {
				TabScript.hscript.text = "";
			}
			if (zui_button(tr("Import"))) {
				UIFiles.show("js", false, false, (path: string) => {
					let b: ArrayBuffer = data_get_blob(path);
					TabScript.hscript.text = sys_buffer_to_string(b);
					data_delete_blob(path);
				});
			}
			if (zui_button(tr("Export"))) {
				let str: string = TabScript.hscript.text;
				UIFiles.show("js", true, false, (path: string) => {
					let f: string = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".js")) path += ".js";
					krom_file_save_bytes(path, sys_string_to_buffer(str));
				});
			}
			zui_end_sticky();

			let _font: g2_font_t = ui.font;
			let _font_size: i32 = ui.font_size;
			let f: g2_font_t = data_get_font("font_mono.ttf");
			zui_set_font(ui, f);
			ui.font_size = Math.floor(15 * zui_SCALE(ui));
			zui_set_text_area_line_numbers(true);
			zui_set_text_area_scroll_past_end(true);
			zui_set_text_area_coloring(TabScript.get_text_coloring());
			zui_text_area(TabScript.hscript);
			zui_set_text_area_line_numbers(false);
			zui_set_text_area_scroll_past_end(false);
			zui_set_text_area_coloring(null);
			zui_set_font(ui, _font);
			ui.font_size = _font_size;
		}
	}

	static get_text_coloring = (): zui_text_coloring_t => {
		if (TabScript.text_coloring == null) {
			let blob: ArrayBuffer = data_get_blob("text_coloring.json");
			TabScript.text_coloring = JSON.parse(sys_buffer_to_string(blob));
			TabScript.text_coloring.default_color = Math.floor(TabScript.text_coloring.default_color);
			for (let coloring of TabScript.text_coloring.colorings) {
				coloring.color = Math.floor(coloring.color);
			}
		}
		return TabScript.text_coloring;
	}
}
