
#include "global.h"

void tab_brushes_draw_make_brush_preview(void * _) {
	i32           i      = _tab_brushes_draw_i;
	slot_brush_t *_brush = context_raw->brush;
	context_raw->brush   = project_brushes->buffer[i];
	make_material_parse_brush();
	util_render_make_brush_preview();
	context_raw->brush = _brush;
}

void tab_brushes_draw_duplicate(void * _) {
	i32 i              = _tab_brushes_draw_i;
	context_raw->brush = slot_brush_create(NULL);
	any_array_push(project_brushes, context_raw->brush);
	void * cloned                 = util_clone_canvas(project_brushes->buffer[i]->canvas);
	context_raw->brush->canvas = cloned;
	context_set_brush(context_raw->brush);
	util_render_make_brush_preview();
}

void tab_brushes_draw_context_menu() {
	i32 i = _tab_brushes_draw_i;
	// let b: slot_brush_t = brushes[i];

	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		context_select_brush(i);
		box_export_show_brush();
	}

	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		sys_notify_on_next_frame(&tab_brushes_draw_duplicate, NULL);
	}

	if (project_brushes->length > 1 && ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		tab_brushes_delete_brush(project_brushes->buffer[i]);
	}
}

void tab_brushes_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Brushes"), false, -1, false)) {
		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -70,
		        -70,
		        -70,
		    },
		    3);
		ui_row(row);
		if (ui_icon_button(tr("New"), ICON_PLUS, UI_ALIGN_CENTER)) {
			context_raw->brush = slot_brush_create(NULL);
			any_array_push(project_brushes, context_raw->brush);
			make_material_parse_brush();
			ui_nodes_hwnd->redraws = 2;
		}
		if (ui_icon_button(tr("Import"), ICON_IMPORT, UI_ALIGN_CENTER)) {
			project_import_brush();
		}
		if (ui_button(tr("Nodes"), UI_ALIGN_CENTER, "")) {
			ui_base_show_brush_nodes();
		}
		ui_end_sticky();
		ui_separator(3, false);

		i32 slotw = math_floor(51 * UI_SCALE());
		i32 num   = math_floor(ui->_window_w / (float)slotw);
		if (num == 0) {
			return;
		}

		for (i32 row = 0; row < math_floor(math_ceil(project_brushes->length / (float)num)); ++row) {
			i32          mult = config_raw->show_asset_names ? 2 : 1;
			f32_array_t *ar   = f32_array_create_from_raw((f32[]){}, 0);
			for (i32 i = 0; i < num * mult; ++i) {
				f32_array_push(ar, 1 / (float)num);
			}
			ui_row(ar);

			ui->_x += 2;
			f32 off = config_raw->show_asset_names ? UI_ELEMENT_OFFSET() * 10.0 : 6;
			if (row > 0) {
				ui->_y += off;
			}

			for (i32 j = 0; j < num; ++j) {
				i32 imgw = math_floor(50 * UI_SCALE());
				i32 i    = j + row * num;
				if (i >= project_brushes->length) {
					ui_end_element_of_size(imgw);
					if (config_raw->show_asset_names) {
						ui_end_element_of_size(0);
					}
					continue;
				}
				gpu_texture_t *img      = UI_SCALE() > 1 ? project_brushes->buffer[i]->image : project_brushes->buffer[i]->image_icon;
				gpu_texture_t *img_full = project_brushes->buffer[i]->image;

				if (context_raw->brush == project_brushes->buffer[i]) {
					// ui_fill(1, -2, img.width + 3, img.height + 3, ui.ops.theme.HIGHLIGHT_COL); // TODO
					i32 off = row % 2 == 1 ? 1 : 0;
					i32 w   = 50;
					if (config_raw->window_scale > 1) {
						w += math_floor(config_raw->window_scale * 2);
					}
					ui_fill(-1, -2, w + 3, 2, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(-1, w - off, w + 3, 2 + off, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(-1, -2, 2, w + 3, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(w + 1, -2, 2, w + 4, ui->ops->theme->HIGHLIGHT_COL);
				}

				f32        uix   = ui->_x;
				// let uiy: f32 = ui._y;
				i32        tile  = UI_SCALE() > 1 ? 100 : 50;
				ui_state_t state = project_brushes->buffer[i]->preview_ready ? ui_image(img, 0xffffffff, -1.0)
				                                                             : ui_sub_image(resource_get("icons.k"), -1, -1.0, tile * 5, tile, tile, tile);
				if (state == UI_STATE_STARTED) {
					if (context_raw->brush != project_brushes->buffer[i]) {
						context_select_brush(i);
					}
					if (sys_time() - context_raw->select_time < 0.2) {
						ui_base_show_brush_nodes();
					}
					context_raw->select_time = sys_time();
					// app_drag_off_x = -(mouse_x - uix - ui._windowX - 3);
					// app_drag_off_y = -(mouse_y - uiy - ui._windowY + 1);
					// app_drag_brush = raw.brush;
				}
				if (ui->is_hovered && ui->input_released_r) {
					context_select_brush(i);

					_tab_brushes_draw_i = i;

					ui_menu_draw(&tab_brushes_draw_context_menu, -1, -1);
				}

				if (ui->is_hovered) {
					if (img_full == NULL) {
						_tab_brushes_draw_i = i;
						sys_notify_on_next_frame(&tab_brushes_draw_make_brush_preview, NULL);
					}
					else {
						ui_tooltip_image(img_full, 0);
						ui_tooltip(project_brushes->buffer[i]->canvas->name);
					}
				}

				if (config_raw->show_asset_names) {
					ui->_x = uix;
					ui->_y += slotw * 0.9;
					ui_text(project_brushes->buffer[i]->canvas->name, UI_ALIGN_CENTER, 0x00000000);
					if (ui->is_hovered) {
						ui_tooltip(project_brushes->buffer[i]->canvas->name);
					}
					ui->_y -= slotw * 0.9;
					if (i == project_brushes->length - 1) {
						ui->_y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
					}
				}
			}

			ui->_y += 6;
		}

		bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
		                ui->input_y < ui->_window_y + ui->_window_h;
		if (in_focus && ui->is_delete_down && project_brushes->length > 1) {
			ui->is_delete_down = false;
			tab_brushes_delete_brush(context_raw->brush);
		}
	}
}

void tab_brushes_delete_brush(slot_brush_t *b) {
	i32 i = array_index_of(project_brushes, b);
	context_select_brush(i == project_brushes->length - 1 ? i - 1 : i + 1);
	array_splice(project_brushes, i, 1);
	ui_base_hwnds->buffer[1]->redraws = 2;
}
