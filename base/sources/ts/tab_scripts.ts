
let tab_scripts_hscript: ui_handle_t = ui_handle_create();
let tab_scripts_text_coloring: ui_text_coloring_t = null;

function tab_scripts_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("Scripts"))) {

		ui_begin_sticky();
		let row: f32[] = [1 / 4, 1 / 4, 1 / 2];
		ui_row(row);

		if (ui_button(tr("Run"))) {
			js_eval(tab_scripts_hscript.text);
		}

		if (ui_button(tr("Edit"))) {

			ui_menu_draw(function (ui: ui_t) {
				if (ui_menu_button(tr("Clear"))) {
					tab_scripts_hscript.text = "";
				}
				if (ui_menu_button(tr("Import"))) {
					ui_files_show("js", false, false, function (path: string) {
						let b: buffer_t = data_get_blob(path);
						tab_scripts_hscript.text = sys_buffer_to_string(b);
						data_delete_blob(path);
					});
				}
				if (ui_menu_button(tr("Export"))) {
					ui_files_show("js", true, false, function (path: string) {
						let str: string = tab_scripts_hscript.text;
						let f: string = ui_files_filename;
						if (f == "") {
							f = tr("untitled");
						}
						path = path + path_sep + f;
						if (!ends_with(path, ".js")) {
							path += ".js";
						}
						iron_file_save_bytes(path, sys_string_to_buffer(str), 0);
					});
				}
			});

		}

		let ar: string[] = ["script.js"];
		let file_handle: ui_handle_t = ui_handle(__ID__);
		ui_combo(file_handle, ar, tr("File"), false, ui_align_t.LEFT);

		ui_end_sticky();

		let _font: draw_font_t = ui.ops.font;
		let _font_size: i32 = ui.font_size;
		let f: draw_font_t = data_get_font("font_mono.ttf");
		ui_set_font(ui, f);
		ui.font_size = math_floor(15 * UI_SCALE());
		ui_text_area_line_numbers = true;
		ui_text_area_scroll_past_end = true;
		ui_text_area_coloring = tab_scripts_get_text_coloring();
		ui_text_area(tab_scripts_hscript);
		ui_text_area_line_numbers = false;
		ui_text_area_scroll_past_end = false;
		ui_text_area_coloring = null;
		ui_set_font(ui, _font);
		ui.font_size = _font_size;
	}
}

function tab_scripts_get_text_coloring(): ui_text_coloring_t {
	if (tab_scripts_text_coloring == null) {
		let blob: buffer_t = data_get_blob("text_coloring.json");
		tab_scripts_text_coloring = json_parse(sys_buffer_to_string(blob));
	}
	return tab_scripts_text_coloring;
}
