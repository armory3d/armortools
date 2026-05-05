
#include "global.h"

i32 tab_debug_line_counter = 0;

void tab_debug_draw_list(ui_handle_t *list_handle, object_t *current_object) {
	if (string_equals(char_at(current_object->name, 0), ".")) {
		return; // Hidden
	}

	bool b = false;

	// Highlight every other line
	if (tab_debug_line_counter % 2 == 0) {
		draw_set_color(ui->ops->theme->SEPARATOR_COL);
		draw_filled_rect(0, ui->_y, ui->_window_w, UI_ELEMENT_H());
		draw_set_color(0xffffffff);
	}

	if (current_object->children->length > 0) {
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        1 / 13.0,
		        12 / 13.0,
		    },
		    2);
		ui_row(row);
		ui_handle_t *h = ui_nest(list_handle, tab_debug_line_counter);
		if (h->init) {
			h->b = true;
		}
		b = ui_panel(h, "", true, false, false);
		ui_text(current_object->name, UI_ALIGN_LEFT, 0x00000000);
	}
	else {
		ui->_x += 18; // Sign offset

		// Draw line that shows parent relations
		draw_set_color(ui->ops->theme->BUTTON_COL);
		draw_line(ui->_x - 10, ui->_y + UI_ELEMENT_H() / 2.0, ui->_x, ui->_y + UI_ELEMENT_H() / 2.0, 1.0);
		draw_set_color(0xffffffff);

		ui_text(current_object->name, UI_ALIGN_LEFT, 0x00000000);
		ui->_x -= 18;
	}

	tab_debug_line_counter++;

	// Undo applied offset for row drawing caused by end_element()
	ui->_y -= UI_ELEMENT_OFFSET();

	if (b) {
		i32 current_y = ui->_y;
		for (i32 i = 0; i < current_object->children->length; ++i) {
			object_t *child = current_object->children->buffer[i];
			ui->_x += 8;
			tab_debug_draw_list(list_handle, child);
			ui->_x -= 8;
		}

		// Draw line that shows parent relations
		draw_set_color(ui->ops->theme->BUTTON_COL);
		draw_line(ui->_x + 14, current_y, ui->_x + 14, ui->_y - UI_ELEMENT_H() / 2.0, 1.0);
		draw_set_color(0xffffffff);
	}
}

void tab_debug_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Debug"), false, -1, false)) {

		ui_handle_t *h0 = ui_handle(__ID__);
		if (ui_panel(h0, "Render Targets", false, false, false)) {
			string_array_t *rt_keys = map_keys(render_path_render_targets);
			array_sort(rt_keys, NULL);
			for (i32 i = 0; i < rt_keys->length; ++i) {
				render_target_t *rt = any_map_get(render_path_render_targets, rt_keys->buffer[i]);
				ui_text(rt_keys->buffer[i], UI_ALIGN_LEFT, 0x00000000);
				ui_image(rt->_image, 0xffffffff, -1.0);
			}
		}

		ui_handle_t *h1 = ui_handle(__ID__);
		if (ui_panel(h1, "Scene", false, false, false)) {
			tab_debug_line_counter = 0;

			object_t *scene = _scene_root->children->buffer[0];
			for (i32 i = 0; i < scene->children->length; ++i) {
				object_t *c = scene->children->buffer[i];
				tab_debug_draw_list(ui_handle(__ID__), c);
			}
		}
	}
}
