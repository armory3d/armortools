
class TabScript {

	static hscript = zui_handle_create();
	static textColoring: zui_text_coloring_t = null;

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, tr("Script")) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

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
					data_get_blob(path, (b: ArrayBuffer) => {
						TabScript.hscript.text = sys_buffer_to_string(b);
						data_delete_blob(path);
					});
				});
			}
			if (zui_button(tr("Export"))) {
				let str = TabScript.hscript.text;
				UIFiles.show("js", true, false, (path: string) => {
					let f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".js")) path += ".js";
					krom_file_save_bytes(path, sys_string_to_buffer(str));
				});
			}
			zui_end_sticky();

			let _font = ui.font;
			let _fontSize = ui.font_size;
			data_get_font("font_mono.ttf", (f: g2_font_t) => { zui_set_font(ui, f); }); // Sync
			ui.font_size = Math.floor(15 * zui_SCALE(ui));
			zui_set_text_area_line_numbers(true);
			zui_set_text_area_scroll_past_end(true);
			zui_set_text_area_coloring(TabScript.getTextColoring());
			zui_text_area(TabScript.hscript);
			zui_set_text_area_line_numbers(false);
			zui_set_text_area_scroll_past_end(false);
			zui_set_text_area_coloring(null);
			zui_set_font(ui, _font);
			ui.font_size = _fontSize;
		}
	}

	static getTextColoring = (): zui_text_coloring_t => {
		if (TabScript.textColoring == null) {
			data_get_blob("text_coloring.json", (blob: ArrayBuffer) => {
				TabScript.textColoring = JSON.parse(sys_buffer_to_string(blob));
				TabScript.textColoring.default_color = Math.floor(TabScript.textColoring.default_color);
				for (let coloring of TabScript.textColoring.colorings) coloring.color = Math.floor(coloring.color);
			});
		}
		return TabScript.textColoring;
	}
}
