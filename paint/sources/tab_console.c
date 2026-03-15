
#include "global.h"

void tab_console_draw_export_on_file_picked(char *path) {
	char *str = string_array_join(console_last_traces, "\n");
	char *f   = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled"));
	}
	path = string("%s%s%s", path, PATH_SEP, f);
	if (!ends_with(path, ".txt")) {
		path = string("%s.txt", path);
	}
	iron_file_save_bytes(path, sys_string_to_buffer(str), 0);
}

void tab_console_draw(ui_handle_t *htab) {
	char *title = console_message_timer > 0 ? string("%s        ", console_message) : tr("Console");
	i32   color = console_message_timer > 0 ? console_message_color : -1;

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

		ui_handle_t *h_input = ui_handle(__ID__);

		if (ui_icon_button(tr("Clear"), ICON_ERASE, UI_ALIGN_CENTER)) {
			gc_unroot(console_last_traces);
			console_last_traces = any_array_create_from_raw((void *[]){}, 0);
			gc_root(console_last_traces);
			h_input->text = "";
		}
		if (ui_icon_button(tr("Export"), ICON_EXPORT, UI_ALIGN_CENTER)) {
			ui_files_show("txt", true, false, &tab_console_draw_export_on_file_picked);
		}
#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
		if (ui_icon_button(tr("Copy"), ICON_COPY, UI_ALIGN_CENTER)) {
			char *str = string_array_join(console_last_traces, "\n");
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
			char *t = console_last_traces->buffer[i];
			ui_text(t, UI_ALIGN_LEFT, 0x00000000);
		}

		row = f32_array_create_from_raw(
		    (f32[]){
		        0.9,
		        0.1,
		    },
		    2);
		ui_row(row);

		ui_text_input(h_input, "", UI_ALIGN_LEFT, true, false);

		ui_set_font(ui, _font);
		ui->font_size = _font_size;

		if (ui_icon_button(tr("Run"), ICON_PLAY, UI_ALIGN_CENTER)) {
			js_eval(h_input->text);
			h_input->text = "";
		}
	}
}
