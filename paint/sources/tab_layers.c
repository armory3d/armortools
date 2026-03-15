
#include "global.h"

void tab_layers_draw(ui_handle_t *htab) {
	bool mini = config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] <= ui_sidebar_w_mini;
	mini ? tab_layers_draw_mini(htab) : tab_layers_draw_full(htab);
}

void tab_layers_draw_mini(ui_handle_t *htab) {
	ui_set_hovered_tab_name(tr("Layers"));

	i32 _ELEMENT_H            = ui->ops->theme->ELEMENT_H;
	ui->ops->theme->ELEMENT_H = math_floor(ui_sidebar_w_mini / (float)2 / (float)UI_SCALE());

	ui_begin_sticky();
	ui_separator(5, true);

	tab_layers_combo_filter();
	tab_layers_button_2d_view();
	tab_layers_button_new("");

	ui_end_sticky();
	ui->_y += 2;

	tab_layers_highlight_odd_lines();
	tab_layers_draw_slots(true);

	ui->ops->theme->ELEMENT_H = _ELEMENT_H;
}

void tab_layers_draw_full(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Layers"), false, -1, false)) {
		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -70,
		        -70,
		        -140,
		    },
		    3);
		ui_row(row);

		tab_layers_button_new(tr("New"));
		tab_layers_button_2d_view();
		tab_layers_combo_filter();

		ui_end_sticky();
		ui->_y += 2;

		tab_layers_highlight_odd_lines();
		tab_layers_draw_slots(false);
	}
}

void tab_layers_button_2d_view() {
	if (ui_button(tr("2D View"), UI_ALIGN_CENTER, "")) {
		ui_base_show_2d_view(VIEW_2D_TYPE_LAYER);
	}
	else if (ui->is_hovered) {
		ui_tooltip(string("%s (%s)", tr("Show 2D View"), (char *)any_map_get(config_keymap, "toggle_2d_view")));
	}
}

void tab_layers_draw_slots(bool mini) {
	for (i32 i = 0; i < project_layers->length; ++i) {
		if (i >= project_layers->length) {
			break; // Layer was deleted
		}
		i32           j = project_layers->length - 1 - i;
		slot_layer_t *l = project_layers->buffer[j];
		tab_layers_draw_layer_slot(l, j, mini);
	}
}

void tab_layers_highlight_odd_lines() {
	i32 step   = ui->ops->theme->ELEMENT_H * 2;
	i32 full_h = ui->_window_h - ui_base_hwnds->buffer[0]->scroll_offset;
	for (i32 i = 0; i < math_floor(full_h / (float)step); ++i) {
		if (i % 2 == 0) {
			ui_fill(0, i * step, (ui->_w / (float)UI_SCALE() - 2), step, ui->ops->theme->WINDOW_BG_COL - 0x00040404);
		}
	}
}

void tab_layers_button_new_to_fill_layer(slot_layer_t *m) {
	slot_layer_to_fill_layer(m);
}

void tab_layers_button_new_layer_clear(slot_layer_t *m) {
	slot_layer_clear(m, 0xffffffff, NULL, 1.0, layers_default_rough, 0.0);
}

void tab_layers_button_new_update_fill_layers(void *_) {
	layers_update_fill_layers();
}

void tab_layers_button_new_black_mask(slot_layer_t *m) {
	slot_layer_clear(m, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
}

void tab_layers_button_new_sculpt_layer(void *_) {
	sculpt_layers_create_sculpt_layer();
}

void tab_layers_button_new_menu() {
	slot_layer_t *l = context_raw->layer;
	if (config_raw->workspace == WORKSPACE_SCULPT) {
		if (ui_menu_button(tr("Sculpt Layer"), "", ICON_PAINT)) {
			sys_notify_on_next_frame(&tab_layers_button_new_sculpt_layer, NULL);
		}
	}
	else {
		if (ui_menu_button(tr("Paint Layer"), "", ICON_PAINT)) {
			layers_new_layer(true, -1);
			history_new_layer();
		}
	}
	if (ui_menu_button(tr("Fill Layer"), "", ICON_SPHERE)) {
		layers_create_fill_layer(UV_TYPE_UVMAP, mat4nan, -1);
	}
	if (ui_menu_button(tr("Decal Layer"), "", ICON_DECAL)) {
		layers_create_fill_layer(UV_TYPE_PROJECT, mat4nan, -1);
	}
	if (ui_menu_button(tr("Black Mask"), "", ICON_MASK)) {
		if (slot_layer_is_mask(l)) {
			context_set_layer(l->parent);
		}
		l = context_raw->layer;

		slot_layer_t *m = layers_new_mask(false, l, -1);
		sys_notify_on_next_frame(&tab_layers_button_new_black_mask, m);
		context_raw->layer_preview_dirty = true;
		history_new_black_mask();
		sys_notify_on_next_frame(&tab_layers_button_new_update_fill_layers, NULL);
	}
	if (ui_menu_button(tr("White Mask"), "", ICON_MASK_WHITE)) {
		if (slot_layer_is_mask(l)) {
			context_set_layer(l->parent);
		}
		l = context_raw->layer;

		slot_layer_t *m = layers_new_mask(false, l, -1);
		sys_notify_on_next_frame(&tab_layers_button_new_layer_clear, m);
		context_raw->layer_preview_dirty = true;
		history_new_white_mask();
		sys_notify_on_next_frame(&tab_layers_button_new_update_fill_layers, NULL);
	}
	if (ui_menu_button(tr("Fill Mask"), "", ICON_MASK_FILL)) {
		if (slot_layer_is_mask(l)) {
			context_set_layer(l->parent);
		}
		l = context_raw->layer;

		slot_layer_t *m = layers_new_mask(false, l, -1);
		sys_notify_on_next_frame(&tab_layers_button_new_to_fill_layer, m);
		context_raw->layer_preview_dirty = true;
		history_new_fill_mask();
		sys_notify_on_next_frame(&tab_layers_button_new_to_fill_layer, NULL);
	}
	ui->enabled = !slot_layer_is_group(context_raw->layer) && !slot_layer_is_in_group(context_raw->layer);
	if (ui_menu_button(tr("Group"), "", ICON_FOLDER)) {
		if (slot_layer_is_group(l) || slot_layer_is_in_group(l)) {
			return;
		}

		if (slot_layer_is_layer_mask(l)) {
			l = l->parent;
		}

		i32_map_t    *pointers = tab_layers_init_layer_map();
		slot_layer_t *group    = layers_new_group();
		context_set_layer(l);
		array_remove(project_layers, group);
		array_insert(project_layers, array_index_of(project_layers, l) + 1, group);
		l->parent = group;
		for (i32 i = 0; i < project_materials->length; ++i) {
			slot_material_t *m = project_materials->buffer[i];
			tab_layers_remap_layer_pointers(m->canvas->nodes, tab_layers_fill_layer_map(pointers));
		}
		context_set_layer(group);
		history_new_group();
	}
	ui->enabled = true;
}

void tab_layers_button_new(char *text) {
	if (ui_icon_button(text, ICON_PLUS, UI_ALIGN_CENTER)) {
		ui_menu_draw(&tab_layers_button_new_menu, -1, -1);
	}
}

void tab_layers_combo_filter() {
	string_t_array_t *ar = any_array_create_from_raw(
	    (void *[]){
	        tr("All"),
	    },
	    1);
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *p = project_paint_objects->buffer[i];
		any_array_push(ar, p->base->name);
	}
	string_t_array_t *atlases = project_get_used_atlases();
	if (atlases != NULL) {
		for (i32 i = 0; i < atlases->length; ++i) {
			char *a = atlases->buffer[i];
			any_array_push(ar, a);
		}
	}
	ui_handle_t *filter_handle = ui_handle(__ID__);
	filter_handle->i           = context_raw->layer_filter;
	context_raw->layer_filter  = ui_combo(filter_handle, ar, tr("Filter"), false, UI_ALIGN_LEFT, true);
	if (filter_handle->changed) {
		for (i32 i = 0; i < project_paint_objects->length; ++i) {
			mesh_object_t *p           = project_paint_objects->buffer[i];
			char          *filter_name = ar->buffer[context_raw->layer_filter];
			p->base->visible           = context_raw->layer_filter == 0 || string_equals(p->base->name, filter_name) || project_is_atlas_object(p);
		}
		if (context_raw->layer_filter == 0 && context_raw->merged_object_is_atlas) { // All
			util_mesh_merge(NULL);
		}
		else if (context_raw->layer_filter > project_paint_objects->length) { // Atlas
			mesh_object_t_array_t *visibles = any_array_create_from_raw((void *[]){}, 0);
			for (i32 i = 0; i < project_paint_objects->length; ++i) {
				mesh_object_t *p = project_paint_objects->buffer[i];
				if (p->base->visible) {
					any_array_push(visibles, p);
				}
			}
			util_mesh_merge(visibles);
		}
		layers_set_object_mask();
		util_uv_uvmap_cached       = false;
		context_raw->ddirty        = 2;
		render_path_raytrace_ready = false;
	}
}

void tab_layers_remap_layer_pointers(ui_node_t_array_t *nodes, i32_imap_t *pointer_map) {
	for (i32 i = 0; i < nodes->length; ++i) {
		ui_node_t *n = nodes->buffer[i];
		if (string_equals(n->type, "LAYER") || string_equals(n->type, "LAYER_MASK")) {
			i32 i = n->buttons->buffer[0]->default_value->buffer[0];
			if (i32_imap_get(pointer_map, i) != -1) {
				n->buttons->buffer[0]->default_value->buffer[0] = i32_imap_get(pointer_map, i);
			}
		}
	}
}

i32_map_t *tab_layers_init_layer_map() {
	i32_map_t *res = any_map_create();
	for (i32 i = 0; i < project_layers->length; ++i) {
		i32_map_set(res, project_layers->buffer[i], i);
	}
	return res;
}

i32_imap_t *tab_layers_fill_layer_map(i32_map_t *map) {
	i32_imap_t       *res  = any_map_create();
	string_t_array_t *keys = map_keys(map);
	for (i32 i = 0; i < keys->length; ++i) {
		char *l = keys->buffer[i];
		i32_imap_set(res, i32_map_get(map, l), array_index_of(project_layers, l) > -1 ? array_index_of(project_layers, l) : 9999);
	}
	return res;
}

void tab_layers_set_drag_layer(slot_layer_t *layer, f32 off_x, f32 off_y) {
	base_drag_off_x = off_x;
	base_drag_off_y = off_y;
	gc_unroot(base_drag_layer);
	base_drag_layer = layer;
	gc_root(base_drag_layer);
	context_raw->drag_dest = array_index_of(project_layers, layer);
}

void tab_layers_draw_layer_slot(slot_layer_t *l, i32 i, bool mini) {
	if (context_raw->layer_filter > 0 && slot_layer_get_object_mask(l) > 0 && slot_layer_get_object_mask(l) != context_raw->layer_filter) {
		return;
	}

	if (l->parent != NULL && !l->parent->show_panel) { // Group closed
		return;
	}
	if (l->parent != NULL && l->parent->parent != NULL && !l->parent->parent->show_panel) {
		return;
	}

	i32 step   = ui->ops->theme->ELEMENT_H;
	f32 checkw = (ui->_window_w / (float)100 * 8) / (float)UI_SCALE();

	// Highlight drag destination
	f32 absy = ui->_window_y + ui->_y;
	if (base_is_dragging && base_drag_layer != NULL && context_in_layers()) {
		if (mouse_y > absy + step && mouse_y < absy + step * 3) {
			bool down                          = array_index_of(project_layers, base_drag_layer) >= i;
			context_raw->drag_dest             = down ? i : i - 1;
			slot_layer_t_array_t *ls           = project_layers;
			i32                   dest         = context_raw->drag_dest;
			bool                  to_group     = down ? dest > 0 && ls->buffer[dest - 1]->parent != NULL && ls->buffer[dest - 1]->parent->show_panel
			                                          : dest < ls->length && ls->buffer[dest]->parent != NULL && ls->buffer[dest]->parent->show_panel;
			bool                  nested_group = slot_layer_is_group(base_drag_layer) && to_group;
			if (!nested_group) {
				if (slot_layer_can_move(context_raw->layer, context_raw->drag_dest)) {
					ui_fill(checkw, step * 2, (ui->_window_w / (float)UI_SCALE() - 2) - checkw, 2 * UI_SCALE(), ui->ops->theme->HIGHLIGHT_COL);
				}
			}
		}
		else if (i == project_layers->length - 1 && mouse_y < absy + step) {
			context_raw->drag_dest = project_layers->length - 1;
			if (slot_layer_can_move(context_raw->layer, context_raw->drag_dest)) {
				ui_fill(checkw, 0, (ui->_window_w / (float)UI_SCALE() - 2) - checkw, 2 * UI_SCALE(), ui->ops->theme->HIGHLIGHT_COL);
			}
		}
	}
	if (base_is_dragging && (base_drag_material != NULL || base_drag_swatch != NULL) && context_in_layers()) {
		if (mouse_y > absy + step && mouse_y < absy + step * 3) {
			context_raw->drag_dest = i;
			if (tab_layers_can_drop_new_layer(i)) {
				ui_fill(checkw, 2 * step, (ui->_window_w / (float)UI_SCALE() - 2) - checkw, 2 * UI_SCALE(), ui->ops->theme->HIGHLIGHT_COL);
			}
		}
		else if (i == project_layers->length - 1 && mouse_y < absy + step) {
			context_raw->drag_dest = project_layers->length;
			if (tab_layers_can_drop_new_layer(project_layers->length)) {
				ui_fill(checkw, 0, (ui->_window_w / (float)UI_SCALE() - 2) - checkw, 2 * UI_SCALE(), ui->ops->theme->HIGHLIGHT_COL);
			}
		}
	}
	mini ? tab_layers_draw_layer_slot_mini(l, i) : tab_layers_draw_layer_slot_full(l, i);

	tab_layers_draw_layer_highlight(l, mini);

	if (tab_layers_show_context_menu) {
		tab_layers_draw_layer_context_menu(l, mini);
	}
}

void tab_layers_draw_layer_slot_mini(slot_layer_t *l, i32 i) {
	f32        uix   = ui->_x;
	f32        uiy   = ui->_y;
	ui_state_t state = tab_layers_draw_layer_icon(l, i, uix, uiy, true);
	tab_layers_handle_layer_icon_state(l, i, state, uix, uiy);
	ui->_x = uix;
	ui->_y = uiy + ui->ops->theme->ELEMENT_H * 2 * UI_SCALE();
}

void tab_layers_draw_layer_slot_full_delete_layer(void *_) {
	tab_layers_delete_layer(context_raw->layer);
}

void tab_layers_draw_layer_slot_full(slot_layer_t *l, i32 i) {
	i32 step   = ui->ops->theme->ELEMENT_H;
	f32 center = (step / (float)2) * UI_SCALE();
	f32 uiw    = ui->_w;
	f32 uix    = ui->_x;
	f32 uiy    = ui->_y;

	bool has_children = slot_layer_is_group(l) || (slot_layer_is_layer(l) && slot_layer_get_masks(l, false) != NULL);

	// Draw eye icon
	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        0.08,
	    },
	    1);
	ui_row(row);
	gpu_texture_t *icons = resource_get("icons.k");
	rect_t        *r     = resource_tile18(icons, l->visible ? ICON18_EYE_ON : ICON18_EYE_OFF);
	ui->_x               = uix + 4;
	ui->_y               = uiy + 3 + center;
	i32  col             = ui->ops->theme->HOVER_COL + 0x00282828;
	bool parent_hidden   = l->parent != NULL && (!l->parent->visible || (l->parent->parent != NULL && !l->parent->parent->visible));
	if (parent_hidden) {
		col -= 0x99000000;
	}
	if (ui_sub_image(icons, col, -1.0, r->x, r->y, r->w, r->h) == UI_STATE_RELEASED) {
		tab_layers_layer_toggle_visible(l);
	}

	// Nested offset
	f32 offx = 0.0;
	if (l->parent != NULL) {
		offx = 14 * UI_SCALE();
		if (l->parent->parent != NULL) {
			offx += 14 * UI_SCALE();
		}
	}

	// Layer icon
	ui->_x           = uix + uiw * 0.08 + offx;
	ui->_y           = uiy + 3;
	ui->_w           = uiw * 0.16;
	ui_state_t state = tab_layers_draw_layer_icon(l, i, uix, uiy, false);
	tab_layers_handle_layer_icon_state(l, i, state, uix, uiy);

	// Draw layer name
	ui->_x = uix + uiw * 0.25 + 2 * UI_SCALE() + offx;
	ui->_y = uiy + center;
	ui->_w = uiw * 0.36;
	if (config_raw->touch_ui) {
		ui->_x += 12 * UI_SCALE();
	}
	if (tab_layers_layer_name_edit == l->id) {
		tab_layers_layer_name_handle->text = string_copy(l->name);
		l->name                            = string_copy(ui_text_input(tab_layers_layer_name_handle, "", UI_ALIGN_LEFT, true, false));
		if (ui->text_selected_handle != tab_layers_layer_name_handle) {
			tab_layers_layer_name_edit = -1;
		}
	}
	else {
		if (ui->enabled && ui->input_enabled && ui->combo_selected_handle == NULL && ui->input_x > ui->_window_x + ui->_x &&
		    ui->input_x < ui->_window_x + uiw && ui->input_y > ui->_window_y + ui->_y - center &&
		    ui->input_y < ui->_window_y + ui->_y - center + (step * UI_SCALE()) * 2) {
			if (ui->input_started) {
				context_set_layer(l);
				tab_layers_set_drag_layer(context_raw->layer, -(mouse_x - uix - ui->_window_x - 3), -(mouse_y - uiy - ui->_window_y + 1));
			}
			else if (ui->input_released_r) {
				context_set_layer(l);
				tab_layers_show_context_menu = true;
			}
		}

		ui_state_t state = ui_text(l->name, UI_ALIGN_LEFT, 0x00000000);
		if (state == UI_STATE_RELEASED) {
			if (sys_time() - context_raw->select_time < 0.2) {
				tab_layers_layer_name_edit         = l->id;
				tab_layers_layer_name_handle->text = string_copy(l->name);
				ui_start_text_edit(tab_layers_layer_name_handle, UI_ALIGN_LEFT);
			}
			context_raw->select_time = sys_time();
		}

		bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
		                ui->input_y < ui->_window_y + ui->_window_h;
		if (in_focus && ui->is_delete_down && tab_layers_can_delete(context_raw->layer)) {
			ui->is_delete_down = false;
			sys_notify_on_next_frame(&tab_layers_draw_layer_slot_full_delete_layer, NULL);
		}
	}

	// Blending combo
	if (!slot_layer_is_group(l)) {
		ui->_x = uix + uiw * 0.60;
		ui->_y = uiy;
		ui->_w = uiw * 0.30;
		if (slot_layer_is_mask(l)) {
			ui->_y += center;
		}
		tab_layers_combo_blending(l, false);
	}

	// Object combo
	if (!slot_layer_is_group(l) && !slot_layer_is_mask(l)) {
		ui->_x = uix + uiw * 0.60;
		ui->_y = uiy + center * 2;
		ui->_w = uiw * 0.30;
		tab_layers_combo_object(l, false);
	}

	// Panel
	if (has_children) {
		ui->_x                   = uix + uiw * 0.90;
		ui->_y                   = uiy + center;
		ui->_w                   = uiw * 0.15;
		ui_handle_t *layer_panel = ui_nest(ui_handle(__ID__), l->id);
		layer_panel->b           = l->show_panel;
		l->show_panel            = ui_panel(layer_panel, "", false, false, true);
	}

	ui->_x = uix;
	ui->_y = uiy + step * 2 * UI_SCALE();
	ui->_w = uiw;
}

void tab_layers_combo_object_layer_clear(slot_layer_t *l) {
	context_raw->material = l->fill_layer;
	slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
	layers_update_fill_layers();
}

ui_handle_t *tab_layers_combo_object(slot_layer_t *l, bool label) {
	string_t_array_t *ar = any_array_create_from_raw(
	    (void *[]){
	        tr("Shared"),
	    },
	    1);
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *p = project_paint_objects->buffer[i];
		any_array_push(ar, p->base->name);
	}
	string_t_array_t *atlases = project_get_used_atlases();
	if (atlases != NULL) {
		for (i32 i = 0; i < atlases->length; ++i) {
			char *a = atlases->buffer[i];
			any_array_push(ar, a);
		}
	}
	ui_handle_t *object_handle = ui_nest(ui_handle(__ID__), l->id);
	object_handle->i           = l->object_mask;
	l->object_mask             = ui_combo(object_handle, ar, tr("Object"), label, UI_ALIGN_LEFT, true);
	if (object_handle->changed) {
		context_set_layer(l);
		make_material_parse_mesh_material();
		if (l->fill_layer != NULL) { // Fill layer
			sys_notify_on_next_frame(&tab_layers_combo_object_layer_clear, l);
		}
		else {
			layers_set_object_mask();
		}
	}
	return object_handle;
}

ui_handle_t *tab_layers_combo_blending(slot_layer_t *l, bool label) {
	ui_handle_t *blending_handle     = ui_nest(ui_handle(__ID__), l->id);
	blending_handle->i               = l->blending;
	string_t_array_t *blending_combo = any_array_create_from_raw(
	    (void *[]){
	        tr("Mix"),
	        tr("Darken"),
	        tr("Multiply"),
	        tr("Burn"),
	        tr("Lighten"),
	        tr("Screen"),
	        tr("Dodge"),
	        tr("Add"),
	        tr("Overlay"),
	        tr("Soft Light"),
	        tr("Linear Light"),
	        tr("Difference"),
	        tr("Subtract"),
	        tr("Divide"),
	        tr("Hue"),
	        tr("Saturation"),
	        tr("Color"),
	        tr("Value"),
	    },
	    18);
	ui_combo(blending_handle, blending_combo, tr("Blending"), label, UI_ALIGN_LEFT, true);
	if (blending_handle->changed) {
		context_set_layer(l);
		history_layer_blending();
		l->blending = blending_handle->i;
		make_material_parse_mesh_material();
	}
	return blending_handle;
}

void tab_layers_layer_toggle_visible(slot_layer_t *l) {
	l->visible              = !l->visible;
	ui_view2d_hwnd->redraws = 2;
	make_material_parse_mesh_material();
}

void tab_layers_draw_layer_highlight(slot_layer_t *l, bool mini) {
	i32 step = ui->ops->theme->ELEMENT_H;

	// Separator line
	ui_fill(0, 0, (ui->_w / (float)UI_SCALE() - 2), 1 * UI_SCALE(), ui->ops->theme->SEPARATOR_COL);

	// Highlight selected
	if (context_raw->layer == l) {
		if (mini) {
			ui_rect(1, -step * 2, ui->_w / (float)UI_SCALE() - 1, step * 2 + (mini ? -1 : 1), ui->ops->theme->HIGHLIGHT_COL, 3);
		}
		else {
			ui_rect(1, -step * 2 - 1, ui->_w / (float)UI_SCALE() - 2, step * 2 + (mini ? -2 : 1), ui->ops->theme->HIGHLIGHT_COL, 2);
		}
	}
}

void tab_layers_handle_layer_icon_state(slot_layer_t *l, i32 i, ui_state_t state, f32 uix, f32 uiy) {
	gpu_texture_t *texpaint_preview = l->texpaint_preview;
	tab_layers_show_context_menu    = false;

	// Layer preview tooltip
	if (ui->is_hovered && texpaint_preview != NULL) {
		if (slot_layer_is_mask(l)) {
			tab_layers_make_mask_preview_rgba32(l);
			ui_tooltip_image(context_raw->mask_preview_rgba32, 0);
		}
		else {
			ui_tooltip_image(texpaint_preview, 0);
		}
		if (i < 9) {
			i32 i1 = (i + 1);
			ui_tooltip(string("%s - (%s %d)", l->name, (char *)any_map_get(config_keymap, "select_layer"), i1));
		}
		else {
			ui_tooltip(l->name);
		}
	}

	// Show context menu
	if (ui->is_hovered && ui->input_released_r) {
		context_set_layer(l);
		tab_layers_show_context_menu = true;
	}

	if (state == UI_STATE_STARTED) {
		context_set_layer(l);
		tab_layers_set_drag_layer(context_raw->layer, -(mouse_x - uix - ui->_window_x - 3), -(mouse_y - uiy - ui->_window_y + 1));
	}
	else if (state == UI_STATE_RELEASED) {
		if (sys_time() - context_raw->select_time < 0.2) {
			ui_base_show_2d_view(VIEW_2D_TYPE_LAYER);
		}
		if (sys_time() - context_raw->select_time > 0.2) {
			context_raw->select_time = sys_time();
		}
		if (l->fill_layer != NULL) {
			context_set_material(l->fill_layer);
		}
	}
}

ui_state_t tab_layers_draw_layer_icon(slot_layer_t *l, i32 i, f32 uix, f32 uiy, bool mini) {
	gpu_texture_t *icons  = resource_get("icons.k");
	i32            icon_h = (UI_ELEMENT_H() - (mini ? 2 : 3)) * 2;

	if (mini && UI_SCALE() > 1) {
		ui->_x -= 1 * UI_SCALE();
	}

	if (l->parent != NULL) {
		ui->_x += (icon_h - icon_h * 0.9) / (float)2;
		icon_h *= 0.9;
		if (l->parent->parent != NULL) {
			ui->_x += (icon_h - icon_h * 0.9) / (float)2;
			icon_h *= 0.9;
		}
	}

	if (!slot_layer_is_group(l)) {
		gpu_texture_t *texpaint_preview = l->texpaint_preview;

		gpu_texture_t *icon;
		if (l->fill_layer == NULL) {
			icon = texpaint_preview;
		}
		else if (config_raw->window_scale > 1) {
			icon = l->fill_layer->image;
		}
		else {
			icon = l->fill_layer->image_icon;
		}

		if (l->fill_layer == NULL) {
			// Checker
			rect_t *r  = resource_tile50(icons, ICON_CHECKER);
			f32     _x = ui->_x;
			f32     _y = ui->_y;
			f32     _w = ui->_w;
			ui_sub_image(icons, 0xffffffff, icon_h, r->x, r->y, r->w, r->h);
			ui->_x = _x;
			ui->_y = _y;
			ui->_w = _w;
		}
		if (l->fill_layer == NULL && slot_layer_is_mask(l)) {
			draw_set_pipeline(ui_view2d_pipe);
			gpu_set_int(ui_view2d_channel_loc, 1);
		}

		ui_state_t state = ui_image(icon, 0xffffffff, icon_h);

		if (l->fill_layer == NULL && slot_layer_is_mask(l)) {
			draw_set_pipeline(NULL);
		}

		// Draw layer numbers when selecting a layer via keyboard shortcut
		bool is_typing = ui->is_typing;
		if (!is_typing) {
			if (i < 9 && operator_shortcut(any_map_get(config_keymap, "select_layer"), SHORTCUT_TYPE_DOWN)) {
				char *number = i32_to_string(i + 1);
				i32   width  = draw_string_width(ui->ops->font, ui->font_size, number) + 10;
				i32   height = draw_font_height(ui->ops->font, ui->font_size);
				draw_set_color(ui->ops->theme->TEXT_COL);
				draw_filled_rect(uix, uiy, width, height);
				draw_set_color(ui->ops->theme->BUTTON_COL);
				draw_string(number, uix + 5, uiy);
			}
		}

		return state;
	}
	else { // Group
		rect_t *folder_closed = resource_tile50(icons, ICON_FOLDER_FULL);
		rect_t *folder_open   = resource_tile50(icons, ICON_FOLDER_OPEN);
		rect_t *folder        = l->show_panel ? folder_open : folder_closed;
		return ui_sub_image(icons, ui->ops->theme->LABEL_COL - 0x00202020, icon_h, folder->x, folder->y, folder->w, folder->h);
	}
}

bool tab_layers_can_merge_down(slot_layer_t *l) {
	i32 index = array_index_of(project_layers, l);
	// Lowest layer
	if (index == 0) {
		return false;
	}
	// Lowest layer that has masks
	if (slot_layer_is_layer(l) && slot_layer_is_mask(project_layers->buffer[0]) && project_layers->buffer[0]->parent == l) {
		return false;
	}
	// The lowest toplevel layer is a group
	if (slot_layer_is_group(l) && slot_layer_is_in_group(project_layers->buffer[0]) && slot_layer_get_containing_group(project_layers->buffer[0]) == l) {
		return false;
	}
	// Masks must be merged down to masks
	if (slot_layer_is_mask(l) && !slot_layer_is_mask(project_layers->buffer[index - 1])) {
		return false;
	}
	return true;
}

void tab_layers_draw_layer_context_menu_update_fill_layers(void *_) {
	layers_update_fill_layers();
}

void tab_layers_draw_layer_context_menu_set_bits(void *_) {
	layers_set_bits();
}

void tab_layers_draw_layer_context_menu_duplicate(void *_) {
	slot_layer_t *l = tab_layers_l;
	context_set_layer(l);
	history_duplicate_layer();
	layers_duplicate_layer(l);
}

void tab_layers_draw_layer_context_menu_merge_down(void *_) {
	slot_layer_t *l = tab_layers_l;
	context_set_layer(l);
	history_merge_layers();
	layers_merge_down();
	if (context_raw->layer->fill_layer != NULL)
		slot_layer_to_paint_layer(context_raw->layer);
}

void tab_layers_draw_layer_context_menu_merge_group(void *_) {
	slot_layer_t *l = tab_layers_l;
	layers_merge_group(l);
}

void tab_layers_draw_layer_context_menu_apply(void *_) {
	slot_layer_t *l    = tab_layers_l;
	context_raw->layer = l;
	history_apply_mask();
	slot_layer_apply_mask(l);
	context_set_layer(l->parent);
	make_material_parse_mesh_material();
	context_raw->layers_preview_dirty = true;
}

void tab_layers_draw_layer_context_menu_invert(void *_) {
	slot_layer_t *l = tab_layers_l;
	context_set_layer(l);
	history_invert_mask();
	slot_layer_invert_mask(l);
}

void tab_layers_draw_layer_context_menu_clear(void *_) {
	slot_layer_t *l = tab_layers_l;
	if (!slot_layer_is_group(l)) {
		history_clear_layer();
		slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
	}
	else {
		for (i32 i = 0; i < slot_layer_get_children(l)->length; ++i) {
			slot_layer_t *c    = slot_layer_get_children(l)->buffer[i];
			context_raw->layer = c;
			history_clear_layer();
			slot_layer_clear(c, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
		}
		context_raw->layers_preview_dirty = true;
		context_raw->layer                = l;
	}
}

void tab_layers_draw_layer_context_menu_delete(void *_) {
	tab_layers_delete_layer(context_raw->layer);
}

void tab_layers_draw_layer_context_menu_to_paint_layer(void *_) {
	slot_layer_t *l = tab_layers_l;
	slot_layer_is_layer(l) ? history_to_paint_layer() : history_to_paint_mask();
	slot_layer_to_paint_layer(l);
}

void tab_layers_draw_layer_context_menu_to_fill_layer(void *_) {
	slot_layer_t *l = tab_layers_l;
	slot_layer_is_layer(l) ? history_to_fill_layer() : history_to_fill_mask();
	slot_layer_to_fill_layer(l);
}

void tab_layers_draw_layer_context_menu_export_on_file_picked(char *path) {
	slot_layer_t *l = tab_layers_l;
	char         *f = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled"));
	}
	if (!ends_with(f, ".png")) {
		f = string("%s.png", f);
	}
	iron_write_png(string("%s%s%s", path, PATH_SEP, f), gpu_get_texture_pixels(l->texpaint), l->texpaint->width, l->texpaint->height, 3); // RRR1
}

void tab_layers_draw_layer_context_menu_draw() {
	slot_layer_t *l    = tab_layers_l;
	bool          mini = tab_layers_mini;

	if (mini) {
		ui_handle_t *visible_handle = ui_handle(__ID__);
		visible_handle->b           = l->visible;
		ui_check(visible_handle, tr("Visible"), "");
		if (visible_handle->changed) {
			tab_layers_layer_toggle_visible(l);
			ui_menu_keep_open = true;
		}

		if (!slot_layer_is_group(l)) {
			if (tab_layers_combo_blending(l, true)->changed) {
				ui_menu_keep_open = true;
			}
		}
		if (slot_layer_is_layer(l)) {
			if (tab_layers_combo_object(l, true)->changed) {
				ui_menu_keep_open = true;
			}
		}
	}

	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		if (slot_layer_is_mask(l)) {
			ui_files_show("png", true, false, &tab_layers_draw_layer_context_menu_export_on_file_picked);
		}
		else {
			context_raw->layers_export = EXPORT_MODE_SELECTED;
			box_export_show_textures();
		}
	}

	if (!slot_layer_is_group(l)) {
		char *to_fill_string  = slot_layer_is_layer(l) ? tr("To Fill Layer") : tr("To Fill Mask");
		char *to_paint_string = slot_layer_is_layer(l) ? tr("To Paint Layer") : tr("To Paint Mask");

		if (l->fill_layer == NULL && ui_menu_button(to_fill_string, "", ICON_SPHERE)) {
			sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_to_fill_layer, NULL);
		}
		if (l->fill_layer != NULL && ui_menu_button(to_paint_string, "", ICON_PAINT)) {
			sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_to_paint_layer, NULL);
		}
	}

	ui->enabled = tab_layers_can_delete(l);
	if (ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_delete, NULL);
	}
	ui->enabled = true;

	if (l->fill_layer == NULL && ui_menu_button(tr("Clear"), "", ICON_ERASE)) {
		context_set_layer(l);
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_clear, NULL);
	}
	if (slot_layer_is_mask(l) && l->fill_layer == NULL && ui_menu_button(tr("Invert"), "", ICON_INVERT)) {
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_invert, NULL);
	}
	if (slot_layer_is_mask(l) && ui_menu_button(tr("Apply"), "", ICON_CHECK)) {
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_apply, NULL);
	}
	if (slot_layer_is_group(l) && ui_menu_button(tr("Merge Group"), "", ICON_NONE)) {
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_merge_group, NULL);
	}

	ui->enabled = tab_layers_can_merge_down(l);
	if (ui_menu_button(tr("Merge Down"), "", ICON_ARROW_DOWN)) {
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_merge_down, NULL);
	}
	ui->enabled = true;

	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_duplicate, NULL);
	}

	ui_menu_align();
	ui_handle_t *layer_opac_handle = ui_nest(ui_handle(__ID__), l->id);
	layer_opac_handle->f           = l->mask_opacity;
	ui_slider(layer_opac_handle, tr("Opacity"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (layer_opac_handle->changed) {
		if (ui->input_started) {
			history_layer_opacity();
		}
		l->mask_opacity = layer_opac_handle->f;
		make_material_parse_mesh_material();
		ui_menu_keep_open = true;
	}

	if (!slot_layer_is_group(l)) {
		ui_menu_align();
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		string_t_array_t *ar = any_array_create_from_raw(
		    (void *[]){
		        "128",
		        "256",
		        "512",
		        "1024",
		        "2048",
		        "4096",
		    },
		    6);
#else
		string_t_array_t *ar = any_array_create_from_raw(
		    (void *[]){
		        "128",
		        "256",
		        "512",
		        "1024",
		        "2048",
		        "4096",
		        "8192",
		        "16384",
		    },
		    8);
#endif
		ui_handle_t *h            = ui_handle(__ID__);
		bool         changed_last = h->changed;
		h->i                      = base_res_handle->i;
		base_res_handle->i        = ui_combo(h, ar, tr("Resolution"), true, UI_ALIGN_LEFT, true);
		if (h->changed) {
			ui_menu_keep_open = true;
		}
		if (changed_last && !h->changed) {
			layers_on_resized();
		}

		ui_menu_align();
		ui_handle_t *huv       = ui_handle(__ID__);
		huv->i                 = l->uv_map;
		string_t_array_t *aruv = any_array_create_from_raw(
		    (void *[]){
		        "uv0",
		    },
		    1);
		if (mesh_data_get_vertex_array(context_raw->paint_object->data, "tex1") != NULL) {
			any_array_push(aruv, "uv1");
		}
		ui_combo(huv, aruv, tr("UV Map"), true, UI_ALIGN_LEFT, true);
		l->uv_map = huv->i;
		if (huv->changed) {
			make_material_parse_paint_material(true);
			make_material_parse_mesh_material();
			ui_menu_keep_open = true;
		}

#if defined(IRON_ANDROID) || defined(IRON_IOS)
// let bits_items: string[] = ["8"];
// ui_inline_radio(base_bits_handle, bits_items, ui_align_t.LEFT);
#else
		ui_menu_separator();
		ui_menu_align();
		ui_menu_label(tr("Bits"), NULL);
		ui_menu_align();
		string_t_array_t *bits_items = any_array_create_from_raw(
		    (void *[]){
		        "8",
		        "16",
		        "32",
		    },
		    3);
		ui_inline_radio(base_bits_handle, bits_items, UI_ALIGN_LEFT);
#endif
		if (base_bits_handle->changed) {
			sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_set_bits, NULL);
			make_material_parse_paint_material(true);
			ui_menu_keep_open = true;
		}
	}
	if (l->fill_layer != NULL) {
		ui_menu_align();
		ui_handle_t *scale_handle = ui_nest(ui_handle(__ID__), l->id);
		scale_handle->f           = l->scale;
		l->scale                  = ui_slider(scale_handle, tr("UV Scale"), 0.0, 5.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		if (scale_handle->changed) {
			context_set_material(l->fill_layer);
			context_set_layer(l);
			sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_update_fill_layers, NULL);
			ui_menu_keep_open = true;
		}

		ui_menu_align();
		ui_handle_t *angle_handle = ui_nest(ui_handle(__ID__), l->id);
		angle_handle->f           = l->angle;
		l->angle                  = ui_slider(angle_handle, tr("Angle"), 0.0, 360, true, 1, true, UI_ALIGN_RIGHT, true);
		if (angle_handle->changed) {
			context_set_material(l->fill_layer);
			context_set_layer(l);
			make_material_parse_paint_material(true);
			sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_update_fill_layers, NULL);
			ui_menu_keep_open = true;
		}

		ui_menu_align();
		ui_handle_t *uv_type_handle     = ui_nest(ui_handle(__ID__), l->id);
		uv_type_handle->i               = l->uv_type;
		string_t_array_t *uv_type_items = any_array_create_from_raw(
		    (void *[]){
		        tr("UV Map"),
		        tr("Triplanar"),
		        tr("Project"),
		    },
		    3);
		l->uv_type = ui_inline_radio(uv_type_handle, uv_type_items, UI_ALIGN_LEFT);
		if (uv_type_handle->changed) {
			context_set_material(l->fill_layer);
			context_set_layer(l);
			make_material_parse_paint_material(true);
			sys_notify_on_next_frame(&tab_layers_draw_layer_context_menu_update_fill_layers, NULL);
			ui_menu_keep_open = true;
		}
	}

	if (!slot_layer_is_group(l)) {
		ui_handle_t *base_handle         = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *opac_handle         = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *nor_handle          = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *nor_blend_handle    = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *occ_handle          = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *rough_handle        = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *met_handle          = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *height_handle       = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *height_blend_handle = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *emis_handle         = ui_nest(ui_handle(__ID__), l->id);
		ui_handle_t *subs_handle         = ui_nest(ui_handle(__ID__), l->id);
		base_handle->b                   = l->paint_base;
		opac_handle->b                   = l->paint_opac;
		nor_handle->b                    = l->paint_nor;
		nor_blend_handle->b              = l->paint_nor_blend;
		occ_handle->b                    = l->paint_occ;
		rough_handle->b                  = l->paint_rough;
		met_handle->b                    = l->paint_met;
		height_handle->b                 = l->paint_height;
		height_blend_handle->b           = l->paint_height_blend;
		emis_handle->b                   = l->paint_emis;
		subs_handle->b                   = l->paint_subs;

		ui_menu_separator();
		ui_menu_align();
		ui_menu_label(tr("Channels"), NULL);
		ui_menu_align();
		ui_row2();
		l->paint_base = ui_check(base_handle, tr("Base Color"), "");
		l->paint_opac = ui_check(opac_handle, tr("Opacity"), "");

		if (config_raw->workflow == WORKFLOW_PBR) {
			ui_row2();
			l->paint_nor       = ui_check(nor_handle, tr("Normal"), "");
			l->paint_nor_blend = ui_check(nor_blend_handle, tr("Normal Blend"), "");
			ui_row2();
			l->paint_rough = ui_check(rough_handle, tr("Roughness"), "");
			l->paint_met   = ui_check(met_handle, tr("Metallic"), "");
			ui_row2();
			l->paint_height       = ui_check(height_handle, tr("Height"), "");
			l->paint_height_blend = ui_check(height_blend_handle, tr("Height Blend"), "");
			ui_row2();
			l->paint_emis = ui_check(emis_handle, tr("Emission"), "");
			l->paint_subs = ui_check(subs_handle, tr("Subsurface"), "");
			l->paint_occ  = ui_check(occ_handle, tr("Occlusion"), "");
		}

		if (base_handle->changed || opac_handle->changed || nor_handle->changed || nor_blend_handle->changed || occ_handle->changed || rough_handle->changed ||
			met_handle->changed || height_handle->changed || height_blend_handle->changed || emis_handle->changed || subs_handle->changed) {
			make_material_parse_mesh_material();
			ui_menu_keep_open = true;
		}
	}
}

void tab_layers_draw_layer_context_menu(slot_layer_t *l, bool mini) {
	gc_unroot(tab_layers_l);
	tab_layers_l = l;
	gc_root(tab_layers_l);
	tab_layers_mini = mini;

	ui_menu_draw(&tab_layers_draw_layer_context_menu_draw, -1, -1);
}

void tab_layers_make_mask_preview_rgba32_on_next_frame(void *_) {
	slot_layer_t *l = tab_layers_l;
	draw_begin(context_raw->mask_preview_rgba32, false, 0);
	draw_set_pipeline(ui_view2d_pipe);
	gpu_set_int(ui_view2d_channel_loc, 1);
	draw_image(l->texpaint_preview, 0, 0);
	draw_end();
	draw_set_pipeline(NULL);
}

void tab_layers_make_mask_preview_rgba32(slot_layer_t *l) {
	if (context_raw->mask_preview_rgba32 == NULL) {
		context_raw->mask_preview_rgba32 = gpu_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, GPU_TEXTURE_FORMAT_RGBA32);
	}
	// Convert from R8 to RGBA32 for tooltip display
	if (context_raw->mask_preview_last != l) {
		context_raw->mask_preview_last = l;
		gc_unroot(tab_layers_l);
		tab_layers_l = l;
		gc_root(tab_layers_l);
		sys_notify_on_next_frame(&tab_layers_make_mask_preview_rgba32_on_next_frame, NULL);
	}
}

void tab_layers_delete_layer(slot_layer_t *l) {
	i32_map_t *pointers = tab_layers_init_layer_map();

	if (slot_layer_is_layer(l) && slot_layer_has_masks(l, false)) {
		slot_layer_t_array_t *masks = slot_layer_get_masks(l, false);
		for (i32 i = 0; i < masks->length; ++i) {
			slot_layer_t *m    = masks->buffer[i];
			context_raw->layer = m;
			history_delete_layer();
			slot_layer_delete(m);
		}
	}
	if (slot_layer_is_group(l)) {
		slot_layer_t_array_t *children = slot_layer_get_children(l);
		for (i32 i = 0; i < children->length; ++i) {
			slot_layer_t *c = children->buffer[i];
			if (slot_layer_has_masks(c, false)) {
				slot_layer_t_array_t *masks = slot_layer_get_masks(c, false);
				for (i32 i = 0; i < masks->length; ++i) {
					slot_layer_t *m    = masks->buffer[i];
					context_raw->layer = m;
					history_delete_layer();
					slot_layer_delete(m);
				}
			}
			context_raw->layer = c;
			history_delete_layer();
			slot_layer_delete(c);
		}
		if (slot_layer_has_masks(l, true)) {
			for (i32 i = 0; i < slot_layer_get_masks(l, true)->length; ++i) {
				slot_layer_t *m    = slot_layer_get_masks(l, true)->buffer[i];
				context_raw->layer = m;
				history_delete_layer();
				slot_layer_delete(m);
			}
		}
	}

	context_raw->layer = l;
	history_delete_layer();
	slot_layer_delete(l);

	if (slot_layer_is_mask(l)) {
		context_raw->layer = l->parent;
		layers_update_fill_layers();
	}

	// Remove empty group
	if (slot_layer_is_in_group(l) && slot_layer_get_children(slot_layer_get_containing_group(l)) == NULL) {
		slot_layer_t *g = slot_layer_get_containing_group(l);
		// Maybe some group masks are left
		if (slot_layer_has_masks(g, true)) {
			for (i32 i = 0; i < slot_layer_get_masks(g, true)->length; ++i) {
				slot_layer_t *m    = slot_layer_get_masks(g, true)->buffer[i];
				context_raw->layer = m;
				history_delete_layer();
				slot_layer_delete(m);
			}
		}
		context_raw->layer = l->parent;
		history_delete_layer();
		slot_layer_delete(l->parent);
	}
	context_raw->ddirty = 2;
	for (i32 i = 0; i < project_materials->length; ++i) {
		slot_material_t *m = project_materials->buffer[i];
		tab_layers_remap_layer_pointers(m->canvas->nodes, tab_layers_fill_layer_map(pointers));
	}
}

bool tab_layers_can_delete(slot_layer_t *l) {
	i32 num_layers = 0;

	if (slot_layer_is_mask(l)) {
		return true;
	}

	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *slot = project_layers->buffer[i];
		if (slot_layer_is_layer(slot)) {
			++num_layers;
		}
	}

	// All layers are in one group
	if (slot_layer_is_group(l) && slot_layer_get_children(l)->length == num_layers) {
		return false;
	}

	// Do not delete last layer
	return num_layers > 1;
}

bool tab_layers_can_drop_new_layer(i32 position) {
	if (position > 0 && position < project_layers->length && slot_layer_is_mask(project_layers->buffer[position - 1])) {
		// 1. The layer to insert is inserted in the middle
		// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
		return false;
	}
	return true;
}
