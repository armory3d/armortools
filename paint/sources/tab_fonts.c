
#include "global.h"

void tab_fonts_draw_make_font_preview(void * _) {
	i32          i     = _tab_fonts_draw_i;
	slot_font_t *_font = context_raw->font;
	context_raw->font  = project_fonts->buffer[i];
	util_render_make_font_preview();
	context_raw->font = _font;
}

void tab_fonts_draw_context_menu_draw() {
	i32 i = _tab_fonts_draw_i;
	if (project_fonts->length > 1 && ui_menu_button(tr("Delete"), "delete", ICON_DELETE) && !string_equals(project_fonts->buffer[i]->file, "")) {
		tab_fonts_delete_font(project_fonts->buffer[i]);
	}
}

void tab_fonts_draw_context_menu(void * _) {
	context_select_font(_tab_fonts_draw_i);
	ui_menu_draw(&tab_fonts_draw_context_menu_draw, -1, -1);
}

void tab_fonts_draw_select_font(void * _) {
	i32 i = _tab_fonts_draw_i;
	context_select_font(i);
}

void tab_fonts_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Fonts"), false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		    },
		    2);
		ui_row(row);

		if (ui_icon_button(tr("Import"), ICON_IMPORT, UI_ALIGN_CENTER)) {
			project_import_asset("ttf,ttc,otf", true);
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Import font file"));
		}

		if (ui_icon_button(tr("2D View"), ICON_WINDOW, UI_ALIGN_CENTER)) {
			ui_base_show_2d_view(VIEW_2D_TYPE_FONT);
		}
		ui_end_sticky();
		ui_separator(3, false);

		i32 slotw = math_floor(51 * UI_SCALE());
		i32 num   = math_floor(ui->_window_w / (float)slotw);
		if (num == 0) {
			return;
		}

		for (i32 row = 0; row < math_floor(math_ceil(project_fonts->length / (float)num)); ++row) {
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
				if (i >= project_fonts->length) {
					ui_end_element_of_size(imgw);
					if (config_raw->show_asset_names) {
						ui_end_element_of_size(0);
					}
					continue;
				}
				gpu_texture_t *img = project_fonts->buffer[i]->image;

				if (context_raw->font == project_fonts->buffer[i]) {
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
				i32        tile  = UI_SCALE() > 1 ? 100 : 50;
				ui_state_t state = UI_STATE_IDLE;
				if (project_fonts->buffer[i]->preview_ready) {
					// draw_set_pipeline(pipe); // L8
					// gpu_set_int(channel_location, 1);
					state = ui_image(img, 0xffffffff, -1.0);
					// draw_set_pipeline(NULL);
				}
				else {
					state = ui_sub_image(resource_get("icons.k"), -1, -1.0, tile * 6, tile, tile, tile);
				}

				if (state == UI_STATE_STARTED) {
					if (context_raw->font != project_fonts->buffer[i]) {
						_tab_fonts_draw_i = i;

						sys_notify_on_next_frame(&tab_fonts_draw_select_font, NULL);
					}
					if (sys_time() - context_raw->select_time < 0.2) {
						ui_base_show_2d_view(VIEW_2D_TYPE_FONT);
					}
					context_raw->select_time = sys_time();
				}
				if (ui->is_hovered && ui->input_released_r) {
					_tab_fonts_draw_i = i;
					sys_notify_on_next_frame(&tab_fonts_draw_context_menu, NULL);
				}
				if (ui->is_hovered) {
					if (img == NULL) {
						_tab_fonts_draw_i = i;
						sys_notify_on_next_frame(&tab_fonts_draw_make_font_preview, NULL);
					}
					else {
						ui_tooltip_image(img, 0);
						ui_tooltip(project_fonts->buffer[i]->name);
					}
				}

				if (config_raw->show_asset_names) {
					ui->_x = uix;
					ui->_y += slotw * 0.9;
					ui_text(project_fonts->buffer[i]->name, UI_ALIGN_CENTER, 0x00000000);
					if (ui->is_hovered) {
						ui_tooltip(project_fonts->buffer[i]->name);
					}
					ui->_y -= slotw * 0.9;
					if (i == project_fonts->length - 1) {
						ui->_y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
					}
				}
			}

			ui->_y += 6;
		}

		bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
		                ui->input_y < ui->_window_y + ui->_window_h;
		if (in_focus && ui->is_delete_down && project_fonts->length > 1 && !string_equals(context_raw->font->file, "")) {
			ui->is_delete_down = false;
			tab_fonts_delete_font(context_raw->font);
		}
	}
}

void tab_fonts_delete_font_on_next_frame(slot_font_t *font) {
	i32 i = array_index_of(project_fonts, font);
	context_select_font(i == project_fonts->length - 1 ? i - 1 : i + 1);
	data_delete_font(project_fonts->buffer[i]->file);
	array_splice(project_fonts, i, 1);
}

void tab_fonts_delete_font(slot_font_t *font) {
	sys_notify_on_next_frame(&tab_fonts_delete_font_on_next_frame, font);
	ui_base_hwnds->buffer[2]->redraws = 2;
}
