
#include "global.h"

i32 _tab_materials_draw_slots;

void tab_materials_button_nodes() {
	if (ui_button(tr("Nodes"), UI_ALIGN_CENTER, "")) {
		ui_base_show_material_nodes();
	}
	else if (ui->is_hovered) {
		ui_tooltip(string("%s (%s)", tr("Show Node Editor"), (char *)any_map_get(config_keymap, "toggle_node_editor")));
	}
}

void tab_materials_draw_slots_update_fill_layers(void *_) {
	layers_update_fill_layers();
}

void tab_materials_update_material() {
	ui_header_handle->redraws = 2;
	ui_nodes_hwnd->redraws    = 2;
	gc_unroot(ui_nodes_group_stack);
	ui_nodes_group_stack = any_array_create_from_raw((void *[]){}, 0);
	gc_root(ui_nodes_group_stack);
	make_material_parse_paint_material(true);
	util_render_make_material_preview();
	bool decal = context_is_decal();
	if (decal) {
		util_render_make_decal_preview();
	}
}

void tab_materials_draw_slots_duplicate(void *_) {
	i32 i                 = _tab_materials_draw_slots;
	g_context->material = slot_material_create(project_materials->buffer[0]->data, NULL);
	any_array_push(project_materials, g_context->material);
	ui_node_canvas_t *cloned      = util_clone_canvas(project_materials->buffer[i]->canvas);
	g_context->material->canvas = cloned;
	tab_materials_update_material();
	history_duplicate_material();
}

void tab_materials_update_material_pointers(ui_node_t_array_t *nodes, i32 i) {
	for (i32 i = 0; i < nodes->length; ++i) {
		ui_node_t *n = nodes->buffer[i];
		if (string_equals(n->type, "MATERIAL")) {
			if (n->buttons->buffer[0]->default_value->buffer[0] == i) {
				n->buttons->buffer[0]->default_value->buffer[0] = 9999; // Material deleted
			}
			else if (n->buttons->buffer[0]->default_value->buffer[0] > i) {
				n->buttons->buffer[0]->default_value->buffer[0]--; // Offset by deleted material
			}
		}
	}
}

void tab_materials_delete_material(slot_material_t *m) {
	i32 i = array_index_of(project_materials, m);
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->fill_layer == m) {
			l->fill_layer = NULL;
		}
	}
	history_delete_material();
	context_select_material(i == project_materials->length - 1 ? i - 1 : i + 1);
	array_splice(project_materials, i, 1);
	ui_base_hwnds->buffer[1]->redraws = 2;
	for (i32 i = 0; i < project_materials->length; ++i) {
		slot_material_t *m = project_materials->buffer[i];
		tab_materials_update_material_pointers(m->canvas->nodes, i);
	}
	for (i32 i = 0; i < m->canvas->nodes->length; ++i) {
		ui_node_t *n = m->canvas->nodes->buffer[i];
		ui_viewnodes_on_node_remove(n);
	}
}

void tab_materials_draw_slots_menu() {
	i32              i = _tab_materials_draw_slots;
	slot_material_t *m = project_materials->buffer[i];

	if (ui_menu_button(tr("To Fill Layer"), "", ICON_SPHERE)) {
		context_select_material(i);
		layers_create_fill_layer(UV_TYPE_UVMAP, mat4nan, -1);
	}

	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		context_select_material(i);
		box_export_show_material();
	}

	if (ui_menu_button(tr("Bake"), "", ICON_BAKE)) {
		context_select_material(i);
		box_export_show_bake_material();
	}

	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		sys_notify_on_next_frame(&tab_materials_draw_slots_duplicate, NULL);
	}

	if (project_materials->length > 1 && ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		tab_materials_delete_material(m);
	}

	ui_handle_t *base_handle = ui_nest(ui_handle(__ID__), m->id);
	if (base_handle->init) {
		base_handle->b = m->paint_base;
	}

	ui_handle_t *opac_handle = ui_nest(ui_handle(__ID__), m->id);
	if (opac_handle->init) {
		opac_handle->b = m->paint_opac;
	}

	ui_handle_t *nor_handle = ui_nest(ui_handle(__ID__), m->id);
	if (nor_handle->init) {
		nor_handle->b = m->paint_nor;
	}

	ui_handle_t *occ_handle = ui_nest(ui_handle(__ID__), m->id);
	if (occ_handle->init) {
		occ_handle->b = m->paint_occ;
	}

	ui_handle_t *rough_handle = ui_nest(ui_handle(__ID__), m->id);
	if (rough_handle->init) {
		rough_handle->b = m->paint_rough;
	}

	ui_handle_t *met_handle = ui_nest(ui_handle(__ID__), m->id);
	if (met_handle->init) {
		met_handle->b = m->paint_met;
	}

	ui_handle_t *height_handle = ui_nest(ui_handle(__ID__), m->id);
	if (height_handle->init) {
		height_handle->b = m->paint_height;
	}

	ui_handle_t *emis_handle = ui_nest(ui_handle(__ID__), m->id);
	if (emis_handle->init) {
		emis_handle->b = m->paint_emis;
	}

	ui_handle_t *subs_handle = ui_nest(ui_handle(__ID__), m->id);
	if (subs_handle->init) {
		subs_handle->b = m->paint_subs;
	}

	ui_menu_separator();
	ui_menu_align();
	ui_menu_label(tr("Channels"), NULL);
	ui_menu_align();
	ui_row2();
	m->paint_base = ui_check(base_handle, tr("Base Color"), "");
	m->paint_opac = ui_check(opac_handle, tr("Opacity"), "");

	if (g_config->workflow == WORKFLOW_PBR) {
		ui_row2();
		m->paint_nor    = ui_check(nor_handle, tr("Normal"), "");
		m->paint_height = ui_check(height_handle, tr("Height"), "");
		ui_row2();
		m->paint_rough = ui_check(rough_handle, tr("Roughness"), "");
		m->paint_met   = ui_check(met_handle, tr("Metallic"), "");
		ui_row2();
		m->paint_emis = ui_check(emis_handle, tr("Emission"), "");
		m->paint_subs = ui_check(subs_handle, tr("Subsurface"), "");
		m->paint_occ  = ui_check(occ_handle, tr("Occlusion"), "");
	}

	if (base_handle->changed || opac_handle->changed || nor_handle->changed || occ_handle->changed || rough_handle->changed || met_handle->changed ||
	    height_handle->changed || emis_handle->changed || subs_handle->changed) {
		make_material_parse_paint_material(true);
		ui_menu_keep_open = true;
	}
}

void tab_materials_draw_slots(bool mini) {
	i32 slotw = math_floor(51 * UI_SCALE() + g_config->window_scale * 2);
	i32 num   = math_floor(ui->_window_w / (float)slotw);
	if (num == 0) {
		return;
	}

	for (i32 row = 0; row < math_floor(math_ceil(project_materials->length / (float)num)); ++row) {
		i32          mult = g_config->show_asset_names ? 2 : 1;
		f32_array_t *ar   = f32_array_create_from_raw((f32[]){}, 0);
		for (i32 i = 0; i < num * mult; ++i) {
			f32_array_push(ar, 1 / (float)num);
		}
		ui_row(ar);

		ui->_x += 2;
		f32 off = g_config->show_asset_names ? UI_ELEMENT_OFFSET() * 10.0 : 6;
		if (row > 0) {
			ui->_y += off;
		}

		for (i32 j = 0; j < num; ++j) {
			i32 imgw = math_floor(50 * UI_SCALE());
			i32 i    = j + row * num;
			if (i >= project_materials->length) {
				ui_end_element_of_size(imgw);
				if (g_config->show_asset_names) {
					ui_end_element_of_size(0);
				}
				continue;
			}
			gpu_texture_t *img      = UI_SCALE() > 1 ? project_materials->buffer[i]->image : project_materials->buffer[i]->image_icon;
			gpu_texture_t *img_full = project_materials->buffer[i]->image;

			// Highligh selected
			if (g_context->material == project_materials->buffer[i]) {
				if (mini) {
					f32 w = ui->_w / (float)UI_SCALE();
					ui_rect(0, -2, w - 2, w - 4, ui->ops->theme->HIGHLIGHT_COL, 3);
				}
				else {
					i32 off = row % 2 == 1 ? 1 : 0;
					i32 w   = 50 + math_floor(g_config->window_scale * 2);
					ui_fill(-1, -2, w + 3, 2, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(-1, w - off, w + 3, 2 + off, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(-1, -2, 2, w + 3, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(w + 1, -2, 2, w + 4, ui->ops->theme->HIGHLIGHT_COL);
				}
			}

			// Draw material icon
			f32        uix   = ui->_x;
			f32        uiy   = ui->_y;
			i32        tile  = UI_SCALE() > 1 ? 100 : 50;
			f32        imgh  = mini ? ui_sidebar_default_w_mini * 0.85 * UI_SCALE() : 50 * UI_SCALE();
			ui_state_t state = project_materials->buffer[i]->preview_ready ? ui_image(img, 0xffffffff, imgh)
			                                                               : ui_sub_image(resource_get("icons.k"), 0xffffffff, -1.0, tile, tile, tile, tile);

			// Draw material numbers when selecting a material via keyboard shortcut
			bool is_typing = ui->is_typing;
			if (!is_typing) {
				if (i < 9 && operator_shortcut(any_map_get(config_keymap, "select_material"), SHORTCUT_TYPE_DOWN)) {
					char *number = i32_to_string(i + 1);
					i32   width  = draw_string_width(ui->ops->font, ui->font_size, number) + 10;
					i32   height = draw_font_height(ui->ops->font, ui->font_size);
					draw_set_color(ui->ops->theme->TEXT_COL);
					draw_filled_rect(uix, uiy, width, height);
					draw_set_color(ui->ops->theme->BUTTON_COL);
					draw_string(number, uix + 5, uiy);
				}
			}

			// Select material
			if (state == UI_STATE_STARTED && ui->input_y > ui->_window_y) {
				if (g_context->material != project_materials->buffer[i]) {
					context_select_material(i);
					if (g_context->tool == TOOL_TYPE_MATERIAL) {
						sys_notify_on_next_frame(&tab_materials_draw_slots_update_fill_layers, NULL);
					}
				}
				base_drag_off_x = -(mouse_x - uix - ui->_window_x - 3);
				base_drag_off_y = -(mouse_y - uiy - ui->_window_y + 1);
				gc_unroot(base_drag_material);
				base_drag_material = g_context->material;
				gc_root(base_drag_material);
				// Double click to show nodes
				if (sys_time() - g_context->select_time < 0.2) {
					ui_base_show_material_nodes();
					gc_unroot(base_drag_material);
					base_drag_material = NULL;
					base_is_dragging   = false;
				}
				g_context->select_time = sys_time();
			}

			// Context menu
			if (ui->is_hovered && ui->input_released_r) {
				context_select_material(i);
				_tab_materials_draw_slots = i;
				ui_menu_draw(&tab_materials_draw_slots_menu, -1, -1);
			}
			if (ui->is_hovered) {
				ui_tooltip_image(img_full, 0);
				if (i < 9) {
					i32 i1 = i + 1;
					ui_tooltip(string("%s - (%s %d)", project_materials->buffer[i]->canvas->name, (char *)any_map_get(config_keymap, "select_material"), i1));
				}
				else {
					ui_tooltip(project_materials->buffer[i]->canvas->name);
				}
			}
			if (g_config->show_asset_names) {
				ui->_x = uix;
				ui->_y += slotw * 0.9;
				ui_text(project_materials->buffer[i]->canvas->name, UI_ALIGN_CENTER, 0x00000000);
				if (ui->is_hovered) {
					if (i < 9) {
						i32 i1 = i + 1;
						ui_tooltip(
						    string("%s - (%s %d)", project_materials->buffer[i]->canvas->name, (char *)any_map_get(config_keymap, "select_material"), i1));
					}
					else {
						ui_tooltip(project_materials->buffer[i]->canvas->name);
					}
				}
				ui->_y -= slotw * 0.9;
				if (i == project_materials->length - 1) {
					ui->_y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
				}
			}
		}

		ui->_y += mini ? 0 : 6;
	}

	bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
	                ui->input_y < ui->_window_y + ui->_window_h;
	if (in_focus && ui->is_delete_down && project_materials->length > 1) {
		ui->is_delete_down = false;
		tab_materials_delete_material(g_context->material);
	}
}

void tab_materials_button_new_on_next_frame(void *_) {
	g_context->material = slot_material_create(project_materials->buffer[0]->data, NULL);
	any_array_push(project_materials, g_context->material);
	tab_materials_update_material();
	history_new_material();
}

void tab_materials_button_new(char *text) {
	if (ui_icon_button(text, ICON_PLUS, UI_ALIGN_CENTER)) {
		sys_notify_on_next_frame(&tab_materials_button_new_on_next_frame, NULL);
	}
}

void tab_materials_draw_mini(ui_handle_t *htab) {
	ui_set_hovered_tab_name(tr("Materials"));

	ui_begin_sticky();
	ui_separator(5, true);

	tab_materials_button_nodes();
	tab_materials_button_new("");

	ui_end_sticky();
	ui_separator(3, false);
	tab_materials_draw_slots(true);
}

void tab_materials_draw_full(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Materials"), false, -1, false)) {
		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -70,
		        -70,
		        -70,
		    },
		    3);
		ui_row(row);

		tab_materials_button_new(tr("New"));
		if (ui_icon_button(tr("Import"), ICON_IMPORT, UI_ALIGN_CENTER)) {
			project_import_material();
		}
		tab_materials_button_nodes();

		ui_end_sticky();
		ui_separator(3, false);
		tab_materials_draw_slots(false);
	}
}

void tab_materials_draw(ui_handle_t *htab) {
	bool mini = ui->_window_w <= ui_sidebar_w_mini;
	mini ? tab_materials_draw_mini(htab) : tab_materials_draw_full(htab);
}

void tab_materials_accept_swatch_drop(swatch_color_t *swatch) {
	g_context->material = slot_material_create(project_materials->buffer[0]->data, NULL);
	for (i32 i = 0; i < g_context->material->canvas->nodes->length; ++i) {
		ui_node_t *node = g_context->material->canvas->nodes->buffer[i];
		if (string_equals(node->type, "RGB")) {
			node->outputs->buffer[0]->default_value = f32_array_create_xyzw(color_get_rb(swatch->base) / 255.0, color_get_gb(swatch->base) / 255.0,
			                                                                color_get_bb(swatch->base) / 255.0, color_get_ab(swatch->base) / 255.0);
		}
		else if (string_equals(node->type, "OUTPUT_MATERIAL_PBR")) {
			node->inputs->buffer[1]->default_value->buffer[0] = swatch->opacity;
			node->inputs->buffer[2]->default_value->buffer[0] = swatch->occlusion;
			node->inputs->buffer[3]->default_value->buffer[0] = swatch->roughness;
			node->inputs->buffer[4]->default_value->buffer[0] = swatch->metallic;
			node->inputs->buffer[7]->default_value->buffer[0] = swatch->height;
		}
	}
	any_array_push(project_materials, g_context->material);
	tab_materials_update_material();
	history_new_material();
}
