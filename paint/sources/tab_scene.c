
#include "global.h"

i32 tab_scene_line_counter = 0;

void tab_scene_select_object(mesh_object_t *mo) {
	if (mo == NULL) {
		return;
	}

	g_context->selected_object = mo->base;

	if (!string_equals(mo->base->ext_type, "mesh_object_t")) {
		return;
	}

	g_context->paint_object = mo;
	if (g_context->merged_object != NULL) {
		g_context->merged_object->base->visible = false;
	}
	context_select_paint_object(mo);
}

i32 tab_scene_sort_compare(void **pa, void **pb) {
	object_t *a = *(pa);
	object_t *b = *(pb);
	return strcmp(a->name, b->name);
}

void tab_scene_sort() {
	object_t *scene = _scene_root->children->buffer[0];
	array_sort(scene->children, &tab_scene_sort_compare);
}

void tab_scene_import_mesh_done_on_next_frame(void *_) {
	util_mesh_merge(NULL);
	tab_scene_select_object(g_context->selected_object->ext);
	tab_scene_sort();
}

void tab_scene_import_mesh_done() {
	i32 count                      = project_paint_objects->length - _tab_scene_paint_object_length;
	_tab_scene_paint_object_length = project_paint_objects->length;

	for (i32 i = 0; i < count; ++i) {
		mesh_object_t *mo = project_paint_objects->buffer[project_paint_objects->length - 1 - i];
		object_set_parent(mo->base, NULL);
		tab_scene_select_object(mo);
	}

	sys_notify_on_next_frame(&tab_scene_import_mesh_done_on_next_frame, NULL);
}

void tab_scene_draw_list_context_menu() {
	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		sim_duplicate();
	}
	if (ui_menu_button(tr("Delete"), "", ICON_DELETE)) {
		sim_delete();
	}
}

void tab_scene_draw_list(ui_handle_t *list_handle, object_t *current_object) {
	if (string_equals(char_at(current_object->name, 0), ".")) {
		return; // Hidden
	}

	bool b = false;

	// Highlight every other line
	if (tab_scene_line_counter % 2 == 0) {
		draw_set_color(ui->ops->theme->SEPARATOR_COL);
		draw_filled_rect(0, ui->_y, ui->_window_w, UI_ELEMENT_H());
		draw_set_color(0xffffffff);
	}

	// Highlight selected line
	if (current_object == g_context->selected_object) {
		draw_set_color(0xff205d9c);
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
		ui_handle_t *h = ui_nest(list_handle, tab_scene_line_counter);
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

	tab_scene_line_counter++;
	// Undo applied offset for row drawing caused by end_element()
	ui->_y -= UI_ELEMENT_OFFSET();

	if (ui->is_released) {
		tab_scene_select_object(current_object->ext);
	}

	if (ui->is_hovered && ui->input_released_r) {
		tab_scene_select_object(current_object->ext);

		ui_menu_draw(&tab_scene_draw_list_context_menu, -1, -1);
	}

	if (b) {
		i32 current_y = ui->_y;
		for (i32 i = 0; i < current_object->children->length; ++i) {
			object_t *child = current_object->children->buffer[i];
			ui->_x += 8;
			tab_scene_draw_list(list_handle, child);
			ui->_x -= 8;
		}

		// Draw line that shows parent relations
		draw_set_color(ui->ops->theme->BUTTON_COL);
		draw_line(ui->_x + 14, current_y, ui->_x + 14, ui->_y - UI_ELEMENT_H() / 2.0, 1.0);
		draw_set_color(0xffffffff);
	}
}

void tab_scene_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Scene"), false, -1, false)) {

		tab_scene_line_counter = 0;

		object_t *scene = _scene_root->children->buffer[0];
		for (i32 i = 0; i < scene->children->length; ++i) {
			object_t *c = scene->children->buffer[i];
			tab_scene_draw_list(ui_handle(__ID__), c);
		}

		// Select object with arrow keys
		if (ui->is_key_pressed && ui->key_code == KEY_CODE_DOWN) {
			i32 i = array_index_of(project_paint_objects, g_context->selected_object->ext);
			if (i < project_paint_objects->length - 1) {
				tab_scene_select_object(project_paint_objects->buffer[i + 1]);
			}
		}
		if (ui->is_key_pressed && ui->key_code == KEY_CODE_UP) {
			i32 i = array_index_of(project_paint_objects, g_context->selected_object->ext);
			if (i > 1) {
				tab_scene_select_object(project_paint_objects->buffer[i - 1]);
			}
		}
	}
}
