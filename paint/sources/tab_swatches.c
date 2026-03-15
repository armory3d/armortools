
#include "global.h"

void tab_swatches_empty_set(gpu_texture_t *image) {
	gc_unroot(_tab_swatches_empty);
	_tab_swatches_empty = image;
	gc_root(_tab_swatches_empty);
}

gpu_texture_t *tab_swatches_empty_get() {
	if (_tab_swatches_empty == NULL) {
		u8_array_t *b = u8_array_create(4);
		b->buffer[0]  = 255;
		b->buffer[1]  = 255;
		b->buffer[2]  = 255;
		b->buffer[3]  = 255;
		gc_unroot(_tab_swatches_empty);
		_tab_swatches_empty = gpu_create_texture_from_bytes(b, 1, 1, GPU_TEXTURE_FORMAT_RGBA32);
		gc_root(_tab_swatches_empty);
	}
	return _tab_swatches_empty;
}

void tab_swatches_draw_16719() {
	i32 i = _tab_swatches_draw_i;

	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		context_set_swatch(project_clone_swatch(context_raw->swatch));
		any_array_push(project_raw->swatches, context_raw->swatch);
	}
#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	else if (ui_menu_button(tr("Copy Hex Code"), "", ICON_HASH)) {
		i32 color = context_raw->swatch->base;
		color     = color_set_ab(color, context_raw->swatch->opacity * 255);
		u32 val   = color;
		iron_copy_to_clipboard(i32_to_string(val));
	}
#endif
	else if (project_raw->swatches->length > 1 && ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		tab_swatches_delete_swatch(project_raw->swatches->buffer[i]);
	}
	else if (ui_menu_button(tr("Create Material"), "", ICON_SPHERE)) {
		tab_materials_accept_swatch_drop(project_raw->swatches->buffer[i]);
	}
	else if (ui_menu_button(tr("Create Color Layer"), "", ICON_LAYERS)) {
		i32 color = project_raw->swatches->buffer[i]->base;
		color     = color_set_ab(color, project_raw->swatches->buffer[i]->opacity * 255);
		layers_create_color_layer(color, project_raw->swatches->buffer[i]->occlusion, project_raw->swatches->buffer[i]->roughness,
		                          project_raw->swatches->buffer[i]->metallic, -1);
	}
}

void tab_swatches_draw_color_picker_callback(swatch_color_t *color) {
	i32 i                            = _tab_swatches_draw_i;
	project_raw->swatches->buffer[i] = project_clone_swatch(color);
}

void tab_swatches_draw_color_picker() {
	context_raw->color_picker_previous_tool = context_raw->tool;
	context_select_tool(TOOL_TYPE_PICKER);
	context_raw->color_picker_callback = &tab_swatches_draw_color_picker_callback;
}

void tab_swatches_draw_edit_menu() {
	ui->changed    = false;
	ui_handle_t *h = ui_handle(__ID__);
	h->color       = context_raw->swatch->base;

	context_raw->swatch->base = ui_color_wheel(h, false, -1, 11 * ui->ops->theme->ELEMENT_H * UI_SCALE(), true, &tab_swatches_draw_color_picker, NULL);

	ui_handle_t *hopacity          = ui_handle(__ID__);
	hopacity->f                    = context_raw->swatch->opacity;
	context_raw->swatch->opacity   = ui_slider(hopacity, "Opacity", 0, 1, true, 100.0, true, UI_ALIGN_RIGHT, true);

	if (config_raw->workflow == WORKFLOW_PBR) {
		ui_handle_t *hocclusion        = ui_handle(__ID__);
		hocclusion->f                  = context_raw->swatch->occlusion;
		context_raw->swatch->occlusion = ui_slider(hocclusion, "Occlusion", 0, 1, true, 100.0, true, UI_ALIGN_RIGHT, true);
		ui_handle_t *hroughness        = ui_handle(__ID__);
		hroughness->f                  = context_raw->swatch->roughness;
		context_raw->swatch->roughness = ui_slider(hroughness, "Roughness", 0, 1, true, 100.0, true, UI_ALIGN_RIGHT, true);
		ui_handle_t *hmetallic         = ui_handle(__ID__);
		hmetallic->f                   = context_raw->swatch->metallic;
		context_raw->swatch->metallic  = ui_slider(hmetallic, "Metallic", 0, 1, true, 100.0, true, UI_ALIGN_RIGHT, true);
		ui_handle_t *hheight           = ui_handle(__ID__);
		hheight->f                     = context_raw->swatch->height;
		context_raw->swatch->height    = ui_slider(hheight, "Height", 0, 1, true, 100.0, true, UI_ALIGN_RIGHT, true);
	}

	if (ui->changed || ui->is_typing) {
		ui_menu_keep_open = true;
	}
	if (ui->input_released) {
		context_set_swatch(context_raw->swatch); // Trigger material preview update
		context_raw->picked_color = util_clone_swatch_color(context_raw->swatch);
		ui_header_handle->redraws = 2;
	}
}

void tab_swatches_draw_import() {
	if (ui_menu_button(tr("Replace Existing"), "", ICON_NONE)) {
		project_import_swatches(true);
		context_set_swatch(project_raw->swatches->buffer[0]);
	}
	if (ui_menu_button(tr("Append"), "", ICON_NONE)) {
		project_import_swatches(false);
	}
}

void tab_swatches_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Swatches"), false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		        -100,
		        -100,
		        -100,
		    },
		    5);
		ui_row(row);

		if (ui_icon_button(tr("New"), ICON_PLUS, UI_ALIGN_CENTER)) {
			context_set_swatch(project_make_swatch(0xffffffff));
			any_array_push(project_raw->swatches, context_raw->swatch);
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Add new swatch"));
		}

		if (ui_icon_button(tr("Import"), ICON_IMPORT, UI_ALIGN_CENTER)) {
			ui_menu_draw(&tab_swatches_draw_import, -1, -1);
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Import swatches"));
		}

		if (ui_icon_button(tr("Export"), ICON_EXPORT, UI_ALIGN_CENTER)) {
			project_export_swatches();
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Export swatches"));
		}

		if (ui_icon_button(tr("Clear"), ICON_ERASE, UI_ALIGN_CENTER)) {
			context_set_swatch(project_make_swatch(0xffffffff));
			project_raw->swatches = any_array_create_from_raw(
			    (void *[]){
			        context_raw->swatch,
			    },
			    1);
		}

		if (ui_icon_button(tr("Restore"), ICON_REPLAY, UI_ALIGN_CENTER)) {
			project_set_default_swatches();
			context_set_swatch(project_raw->swatches->buffer[0]);
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Restore default swatches"));
		}

		ui_end_sticky();
		ui_separator(3, false);

		i32 slotw = math_floor(26 * UI_SCALE());
		i32 num   = math_floor(ui->_w / (float)(slotw + 3));
		if (num == 0) {
			return;
		}
		bool drag_pos_set = false;

		f32 uix = 0.0;
		f32 uiy = 0.0;
		for (i32 row = 0; row < math_floor(math_ceil(project_raw->swatches->length / (float)num)); ++row) {
			f32_array_t *ar = f32_array_create_from_raw((f32[]){}, 0);
			for (i32 i = 0; i < num; ++i) {
				f32_array_push(ar, 1 / (float)num);
			}
			ui_row(ar);

			ui->_x += 2;
			if (row > 0) {
				ui->_y += 6;
			}

			for (i32 j = 0; j < num; ++j) {
				i32 i = j + row * num;
				if (i >= project_raw->swatches->length) {
					ui_end_element_of_size(slotw);
					continue;
				}

				if (context_raw->swatch == project_raw->swatches->buffer[i]) {
					i32 w = 32;
					ui_fill(-2, -2, w, w, ui->ops->theme->HIGHLIGHT_COL);
				}

				uix = ui->_x;
				uiy = ui->_y;

				// Draw the drag position indicator
				if (base_drag_swatch != NULL && tab_swatches_drag_pos == i) {
					ui_fill(-1, -2, 2, 32, ui->ops->theme->HIGHLIGHT_COL);
				}

				ui_state_t state = ui_image(tab_swatches_empty_get(), project_raw->swatches->buffer[i]->base, slotw);

				if (state == UI_STATE_STARTED) {
					context_set_swatch(project_raw->swatches->buffer[i]);
					base_drag_off_x = -(mouse_x - uix - ui->_window_x - 2 * slotw);
					base_drag_off_y = -(mouse_y - uiy - ui->_window_y + 1);
					gc_unroot(base_drag_swatch);
					base_drag_swatch = context_raw->swatch;
					gc_root(base_drag_swatch);
					context_raw->picked_color = util_clone_swatch_color(context_raw->swatch);
					ui_header_handle->redraws = 2;
				}
				else if (state == UI_STATE_HOVERED) {
					tab_swatches_drag_pos = (mouse_x > uix + ui->_window_x + slotw / (float)2)
					                            ? i + 1
					                            : i; // Switch to the next position if the mouse crosses the swatch rectangle center
					drag_pos_set          = true;
				}
				else if (state == UI_STATE_RELEASED) {
					if (sys_time() - context_raw->select_time < 0.2) {
						_tab_swatches_draw_i = i;
						ui_menu_draw(&tab_swatches_draw_edit_menu, -1, -1);
					}

					context_raw->select_time = sys_time();
				}
				if (ui->is_hovered && ui->input_released_r) {
					context_set_swatch(project_raw->swatches->buffer[i]);

					_tab_swatches_draw_i = i;

					ui_menu_draw(&tab_swatches_draw_16719, -1, -1);
				}
				if (ui->is_hovered) {
					i32 color = project_raw->swatches->buffer[i]->base;
					color     = color_set_ab(color, project_raw->swatches->buffer[i]->opacity * 255);
					u32 val   = color;
					ui_tooltip(string("#%s", i32_to_string_hex(val)));
				}
			}
		}

		// Draw the rightmost line next to the last swatch
		if (base_drag_swatch != NULL && tab_swatches_drag_pos == project_raw->swatches->length) {
			ui->_x = uix; // Reset the position because otherwise it would start in the row below
			ui->_y = uiy;
			ui_fill(28, -2, 2, 32, ui->ops->theme->HIGHLIGHT_COL);
		}

		// Currently there is no valid drag_position so reset it
		if (!drag_pos_set) {
			tab_swatches_drag_pos = -1;
		}

		bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
		                ui->input_y < ui->_window_y + ui->_window_h;
		if (in_focus && ui->is_delete_down && project_raw->swatches->length > 1) {
			ui->is_delete_down = false;
			tab_swatches_delete_swatch(context_raw->swatch);
		}
	}
}

void tab_swatches_accept_swatch_drop(swatch_color_t *swatch) {
	// No valid position available
	if (tab_swatches_drag_pos == -1) {
		return;
	}

	i32 swatch_pos = array_index_of(project_raw->swatches, swatch);
	// A new swatch from color picker
	if (swatch_pos == -1) {
		array_insert(project_raw->swatches, tab_swatches_drag_pos, swatch);
	}
	else if (math_abs(swatch_pos - tab_swatches_drag_pos) > 0) { // Existing swatch is reordered
		array_remove(project_raw->swatches, swatch);
		// If the new position is after the old one, decrease by one because the swatch has been deleted
		i32 new_pos = tab_swatches_drag_pos - swatch_pos > 0 ? tab_swatches_drag_pos - 1 : tab_swatches_drag_pos;
		array_insert(project_raw->swatches, new_pos, swatch);
	}
}

void tab_swatches_delete_swatch(swatch_color_t *swatch) {
	i32 i = array_index_of(project_raw->swatches, swatch);
	context_set_swatch(project_raw->swatches->buffer[i == project_raw->swatches->length - 1 ? i - 1 : i + 1]);
	array_splice(project_raw->swatches, i, 1);
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
}
