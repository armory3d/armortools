
#include "global.h"

void tab_textures_draw_set_as_envmap(void *_) {
	import_envmap_run(_tab_textures_draw_asset->file, _tab_textures_draw_img);
}

void tab_textures_draw_to_mask(void *_) {
	layers_create_image_mask(_tab_textures_draw_asset);
}

void tab_textures_draw_export_on_next_frame2(gpu_texture_t *target) {
	char *path = _tab_textures_draw_path;
	char *f    = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled"));
	}
	if (!ends_with(f, ".png")) {
		f = string("%s.png", f);
	}
	buffer_t *buf = gpu_get_texture_pixels(target);
	iron_write_png(string("%s%s%s", path, PATH_SEP, f), buf, target->width, target->height, 0);
	gpu_delete_texture(target);
}

void tab_textures_draw_export_on_next_frame(void *_) {
	gpu_texture_t *img    = _tab_textures_draw_img;
	gpu_texture_t *target = gpu_create_render_target(img->width, img->height, GPU_TEXTURE_FORMAT_RGBA32);
	draw_begin(target, false, 0);
	draw_set_pipeline(pipes_copy);
	draw_scaled_image(img, 0, 0, target->width, target->height);
	draw_set_pipeline(NULL);
	draw_end();
	sys_notify_on_next_frame(&tab_textures_draw_export_on_next_frame2, target);
}

void tab_textures_draw_export(char *path) {
	gc_unroot(_tab_textures_draw_path);
	_tab_textures_draw_path = string_copy(path);
	gc_root(_tab_textures_draw_path);
	sys_notify_on_next_frame(&tab_textures_draw_export_on_next_frame, NULL);
}

void tab_textures_draw_context_menu() {
	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		ui_files_show("png", true, false, &tab_textures_draw_export);
	}
	if (ui_menu_button(tr("Reimport"), "", ICON_SYNC)) {
		project_reimport_texture(_tab_textures_draw_asset);
	}
	if (ui_menu_button(tr("To Mask"), "", ICON_MASK)) {
		sys_notify_on_next_frame(&tab_textures_draw_to_mask, NULL);
	}
	if (ui_menu_button(tr("Set as Envmap"), "", ICON_LANDSCAPE)) {
		sys_notify_on_next_frame(&tab_textures_draw_set_as_envmap, NULL);
	}
	if (ui_menu_button(tr("Set as Color ID Map"), "", ICON_COLOR_ID)) {
		context_raw->colorid_handle->i = _tab_textures_draw_i;
		context_raw->colorid_picked    = false;
		ui_toolbar_handle->redraws     = 1;
		if (context_raw->tool == TOOL_TYPE_COLORID) {
			ui_header_handle->redraws = 2;
			context_raw->ddirty       = 2;
		}
	}
	if (ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		tab_textures_delete_texture(_tab_textures_draw_asset);
	}
	if (!_tab_textures_draw_is_packed && ui_menu_button(tr("Open Containing Directory..."), "", ICON_FOLDER_OPEN)) {
		file_start(substring(_tab_textures_draw_asset->file, 0, string_last_index_of(_tab_textures_draw_asset->file, PATH_SEP)));
	}
	if (!_tab_textures_draw_is_packed && ui_menu_button(tr("Open in Browser"), "", ICON_NONE)) {
		tab_browser_show_directory(substring(_tab_textures_draw_asset->file, 0, string_last_index_of(_tab_textures_draw_asset->file, PATH_SEP)));
	}
}

void tab_textures_draw_import(char *path) {
	import_asset_run(path, -1.0, -1.0, true, false, NULL);
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
}

void tab_textures_draw(ui_handle_t *htab) {

	if (ui_tab(htab, tr("Textures"), false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();

		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		    },
		    2);
		ui_row(row);

		if (ui_icon_button(tr("Import"), ICON_IMPORT, UI_ALIGN_CENTER)) {
			ui_files_show(string_array_join(path_texture_formats(), ","), false, true, &tab_textures_draw_import);
		}
		if (ui->is_hovered) {
			ui_tooltip(string("%s (%s)", tr("Import texture file"), (char *)any_map_get(config_keymap, "file_import_assets")));
		}
		if (ui_icon_button(tr("2D View"), ICON_WINDOW, UI_ALIGN_CENTER)) {
			ui_base_show_2d_view(VIEW_2D_TYPE_ASSET);
		}

		ui_end_sticky();

		if (project_assets->length > 0) {

			i32 slotw = math_floor(52 * UI_SCALE());
			i32 num   = math_floor(ui->_window_w / (float)slotw);
			if (num == 0) {
				return;
			}

			for (i32 row = 0; row < math_floor(math_ceil(project_assets->length / (float)num)); ++row) {
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
					if (i >= project_assets->length) {
						ui_end_element_of_size(imgw);
						if (config_raw->show_asset_names) {
							ui_end_element_of_size(0);
						}
						continue;
					}

					asset_t       *asset = project_assets->buffer[i];
					gpu_texture_t *img   = project_get_image(asset);
					if (img == NULL) {
						render_target_t *empty_rt = any_map_get(render_path_render_targets, "empty_black");
						img                       = empty_rt->_image;
					}
					f32 uix = ui->_x;
					f32 uiy = ui->_y;
					i32 sw  = img->height < img->width ? img->height : 0;
					if (ui_sub_image(img, 0xffffffff, slotw, 0, 0, sw, sw) == UI_STATE_STARTED && ui->input_y > ui->_window_y) {
						base_drag_off_x = -(mouse_x - uix - ui->_window_x - 3);
						base_drag_off_y = -(mouse_y - uiy - ui->_window_y + 1);
						gc_unroot(base_drag_asset);
						base_drag_asset = asset;
						gc_root(base_drag_asset);
						context_raw->texture = asset;
						if (sys_time() - context_raw->select_time < 0.2) {
							ui_base_show_2d_view(VIEW_2D_TYPE_ASSET);
						}
						context_raw->select_time = sys_time();
						ui_view2d_hwnd->redraws  = 2;
					}

					if (asset == context_raw->texture) {
						f32 _uix = ui->_x;
						f32 _uiy = ui->_y;
						ui->_x   = uix;
						ui->_y   = uiy;
						i32 off  = i % 2 == 1 ? 1 : 0;
						i32 w    = 50;
						ui_fill(0, 0, w + 3, 2, ui->ops->theme->HIGHLIGHT_COL);
						ui_fill(0, w - off + 2, w + 3, 2 + off, ui->ops->theme->HIGHLIGHT_COL);
						ui_fill(0, 0, 2, w + 3, ui->ops->theme->HIGHLIGHT_COL);
						ui_fill(w + 2, 0, 2, w + 4, ui->ops->theme->HIGHLIGHT_COL);
						ui->_x = _uix;
						ui->_y = _uiy;
					}

					bool is_packed = project_raw->packed_assets != NULL && project_packed_asset_exists(project_raw->packed_assets, asset->file);

					if (ui->is_hovered) {
						ui_tooltip_image(img, 256);
						if (is_packed) {
							ui_tooltip(string("%s %s", asset->name, tr("(packed)")));
						}
						else {
							ui_tooltip(asset->name);
						}
					}

					if (ui->is_hovered && ui->input_released_r) {
						context_raw->texture = asset;

						gc_unroot(_tab_textures_draw_img);
						_tab_textures_draw_img = img;
						gc_root(_tab_textures_draw_img);
						gc_unroot(_tab_textures_draw_asset);
						_tab_textures_draw_asset = asset;
						gc_root(_tab_textures_draw_asset);
						_tab_textures_draw_i         = i;
						_tab_textures_draw_is_packed = is_packed;
						ui_menu_draw(&tab_textures_draw_context_menu, -1, -1);
					}

					if (config_raw->show_asset_names) {
						ui->_x = uix;
						ui->_y += slotw * 0.9;
						ui_text(project_assets->buffer[i]->name, UI_ALIGN_CENTER, 0x00000000);
						if (ui->is_hovered) {
							ui_tooltip(project_assets->buffer[i]->name);
						}
						ui->_y -= slotw * 0.9;
						if (i == project_assets->length - 1) {
							ui->_y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
						}
					}
				}
			}
		}
		else {
			gpu_texture_t *img = resource_get("icons.k");
			rect_t        *r   = resource_tile50(img, ICON_DROP);
			ui_sub_image(img, ui->ops->theme->BUTTON_COL, r->h, r->x, r->y, r->w, r->h);
			if (ui->is_hovered) {
				ui_tooltip(tr("Drag and drop files here"));
			}
		}

		bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
		                ui->input_y < ui->_window_y + ui->_window_h;
		if (in_focus && ui->is_delete_down && project_assets->length > 0 && array_index_of(project_assets, context_raw->texture) >= 0) {
			ui->is_delete_down = false;
			tab_textures_delete_texture(context_raw->texture);
		}
	}
}

void tab_textures_update_texture_pointers(ui_node_t_array_t *nodes, i32 index) {
	for (i32 i = 0; i < nodes->length; ++i) {
		ui_node_t *n = nodes->buffer[i];
		if (string_equals(n->type, "TEX_IMAGE")) {
			if (n->buttons->buffer[0]->default_value->buffer[0] == index) {
				n->buttons->buffer[0]->default_value->buffer[0] = 9999; // Texture deleted, use pink now
			}
			else if (n->buttons->buffer[0]->default_value->buffer[0] > index) {
				n->buttons->buffer[0]->default_value->buffer[0]--; // Offset by deleted texture
			}
		}
	}
}

void tab_textures_delete_texture_on_next_frame(void *_) {
	make_material_parse_paint_material(true);
	util_render_make_material_preview();
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
}

void tab_textures_delete_texture(asset_t *asset) {
	i32 index = array_index_of(project_assets, asset);
	if (project_assets->length > 1) {
		context_raw->texture = project_assets->buffer[index == project_assets->length - 1 ? index - 1 : index + 1];
	}
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;

	if (context_raw->tool == TOOL_TYPE_COLORID && index == context_raw->colorid_handle->i) {
		ui_header_handle->redraws   = 2;
		context_raw->ddirty         = 2;
		context_raw->colorid_picked = false;
		ui_toolbar_handle->redraws  = 1;
	}

	if (data_get_image(asset->file) == scene_world->_->envmap) {
		project_set_default_envmap();
	}

	if (project_raw->packed_assets != NULL) {
		for (i32 i = 0; i < project_raw->packed_assets->length; ++i) {
			packed_asset_t *pa = project_raw->packed_assets->buffer[i];
			if (string_equals(pa->name, asset->file)) {
				array_splice(project_raw->packed_assets, i, 1);
				break;
			}
		}
	}

	data_delete_image(asset->file);
	imap_delete(project_asset_map, asset->id);
	array_splice(project_assets, index, 1);
	array_splice(project_asset_names, index, 1);
	sys_notify_on_next_frame(&tab_textures_delete_texture_on_next_frame, NULL);

	for (i32 i = 0; i < project_materials->length; ++i) {
		slot_material_t *m = project_materials->buffer[i];
		tab_textures_update_texture_pointers(m->canvas->nodes, index);
	}

	for (i32 i = 0; i < project_brushes->length; ++i) {
		slot_brush_t *b = project_brushes->buffer[i];
		tab_textures_update_texture_pointers(b->canvas->nodes, index);
	}
}
