
#include "global.h"

void tab_scripts_draw_export(char *path) {
	char *str = tab_scripts_hscript->text;
	char *f   = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled"));
	}
	path = string("%s%s%s", path, PATH_SEP, f);
	if (!ends_with(path, ".js")) {
		path = string("%s.js", path);
	}
	iron_file_save_bytes(path, sys_string_to_buffer(str), 0);
}

void tab_scripts_draw_import(char *path) {
	buffer_t *b               = data_get_blob(path);
	tab_scripts_hscript->text = string_copy(sys_buffer_to_string(b));
	data_delete_blob(path);
}

void tab_scripts_draw_edit() {
	if (ui_menu_button(tr("Clear"), "", ICON_ERASE)) {
		tab_scripts_hscript->text = "";
	}
	if (ui_menu_button(tr("Import"), "", ICON_IMPORT)) {
		ui_files_show("js", false, false, &tab_scripts_draw_import);
	}
	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		ui_files_show("js", true, false, &tab_scripts_draw_export);
	}
}

void tab_scripts_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Scripts"), false, -1, false)) {

		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -70,
		        -70,
		        -140,
		    },
		    3);
		ui_row(row);

		if (ui_icon_button(tr("Run"), ICON_PLAY, UI_ALIGN_CENTER)) {
			js_eval(tab_scripts_hscript->text);
		}

		if (ui_icon_button(tr("Edit"), ICON_EDIT, UI_ALIGN_CENTER)) {
			ui_menu_draw(&tab_scripts_draw_edit, -1, -1);
		}

		string_t_array_t *ar = any_array_create_from_raw(
		    (void *[]){
		        "script.js",
		    },
		    1);
		ui_handle_t *file_handle = ui_handle(__ID__);
		ui_combo(file_handle, ar, tr("File"), false, UI_ALIGN_LEFT, true);

		ui_end_sticky();

		draw_font_t *_font      = ui->ops->font;
		i32          _font_size = ui->font_size;
		draw_font_t *f          = data_get_font("font_mono.ttf");
		ui_set_font(ui, f);
		ui->font_size                = math_floor(15 * UI_SCALE());
		ui_text_area_line_numbers    = true;
		ui_text_area_scroll_past_end = true;
		gc_unroot(ui_text_area_coloring);
		ui_text_area_coloring = tab_scripts_get_text_coloring();
		gc_root(ui_text_area_coloring);
		ui_text_area(tab_scripts_hscript, UI_ALIGN_LEFT, true, "", false);
		ui_text_area_line_numbers    = false;
		ui_text_area_scroll_past_end = false;
		gc_unroot(ui_text_area_coloring);
		ui_text_area_coloring = NULL;
		ui_set_font(ui, _font);
		ui->font_size = _font_size;
	}
}

ui_text_coloring_t *tab_scripts_get_text_coloring() {
	if (tab_scripts_text_coloring == NULL) {
		buffer_t *blob = data_get_blob("text_coloring.json");
		gc_unroot(tab_scripts_text_coloring);
		tab_scripts_text_coloring = json_parse(sys_buffer_to_string(blob));
		gc_root(tab_scripts_text_coloring);
	}
	return tab_scripts_text_coloring;
}
