
let tab_layers_layer_name_edit: i32 = -1;
let tab_layers_layer_name_handle: zui_handle_t = zui_handle_create();
let tab_layers_show_context_menu: bool = false;

function tab_layers_draw(htab: zui_handle_t) {
	let mini = config_raw.layout[layout_size_t.SIDEBAR_W] <= ui_base_sidebar_mini_w;
	mini ? tab_layers_draw_mini(htab) : tab_layers_draw_full(htab);
}

function tab_layers_draw_mini(htab: zui_handle_t) {
	let ui = ui_base_ui;
	zui_set_hovered_tab_name(tr("Layers"));

	let _ELEMENT_H = ui.ops.theme.ELEMENT_H;
	ui.ops.theme.ELEMENT_H = math_floor(ui_base_sidebar_mini_w / 2 / zui_SCALE(ui));

	zui_begin_sticky();
	zui_separator(5);

	tab_layers_combo_filter();
	tab_layers_button_new("+");

	zui_end_sticky();
	ui._y += 2;

	tab_layers_highlight_odd_lines();
	tab_layers_draw_slots(true);

	ui.ops.theme.ELEMENT_H = _ELEMENT_H;
}

function tab_layers_draw_full(htab: zui_handle_t) {
	let ui = ui_base_ui;
	if (zui_tab(htab, tr("Layers"))) {
		zui_begin_sticky();
		let row: f32[] = [1 / 4, 3 / 4];
		zui_row(row);

		tab_layers_button_new(tr("New"));
		tab_layers_combo_filter();

		zui_end_sticky();
		ui._y += 2;

		tab_layers_highlight_odd_lines();
		tab_layers_draw_slots(false);
	}
}

function tab_layers_draw_slots(mini: bool) {
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		if (i >= project_layers.length) {
			break; // Layer was deleted
		}
		let j = project_layers.length - 1 - i;
		let l = project_layers[j];
		tab_layers_draw_layer_slot(l, j, mini);
	}
}

function tab_layers_highlight_odd_lines() {
	let ui = ui_base_ui;
	let step = ui.ops.theme.ELEMENT_H * 2;
	let full_h = ui._window_h - ui_base_hwnds[0].scroll_offset;
	for (let i: i32 = 0; i < math_floor(full_h / step); ++i) {
		if (i % 2 == 0) {
			zui_fill(0, i * step, (ui._w / zui_SCALE(ui) - 2), step, ui.ops.theme.WINDOW_BG_COL - 0x00040404);
		}
	}
}

function tab_layers_button_new(text: string) {
	let ui = ui_base_ui;
	if (zui_button(text)) {
		ui_menu_draw(function (ui: zui_t) {
			let l = context_raw.layer;
			if (ui_menu_button(ui, tr("Paint Layer"))) {
				base_new_layer();
				history_new_layer();
			}
		}, 1);
	}
}

function tab_layers_combo_filter() {
	let ui = ui_base_ui;
	let ar = [tr("All")];
	let filter_handle = zui_handle(__ID__);
	filter_handle.position = context_raw.layer_filter;
	context_raw.layer_filter = zui_combo(filter_handle, ar, tr("Filter"), false, zui_align_t.LEFT);
}

function tab_layers_remap_layer_pointers(nodes: zui_node_t[], pointer_map: map_t<i32, i32>) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n = nodes[i];
		if (n.type == "LAYER" || n.type == "LAYER_MASK") {
			let i = n.buttons[0].default_value;
			if (map_get(pointer_map, i) != null) {
				n.buttons[0].default_value = map_get(pointer_map, i);
			}
		}
	}
}

function tab_layers_init_layer_map(): map_t<slot_layer_t, i32> {
	let res: map_t<slot_layer_t, i32> = map_create();
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		map_set(res, project_layers[i], i);
	}
	return res;
}

function tab_layers_fill_layer_map(map: map_t<slot_layer_t, i32>): map_t<i32, i32> {
	let res: map_t<i32, i32> = map_create();
	let keys: string[] = map_keys(map);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let l = keys[i];
		map_set(res, map_get(map, l), array_index_of(project_layers, l) > -1 ? array_index_of(project_layers, l) : 9999);
	}
	return res;
}

function tab_layers_set_drag_layer(layer: slot_layer_t, off_x: f32, off_y: f32) {
	base_drag_off_x = off_x;
	base_drag_off_y = off_y;
	base_drag_layer = layer;
	context_raw.drag_dest = array_index_of(project_layers, layer);
}

function tab_layers_draw_layer_slot(l: slot_layer_t, i: i32, mini: bool) {
	let ui = ui_base_ui;

	if (context_raw.layer_filter > 0 &&
		slot_layer_get_object_mask(l) > 0 &&
		slot_layer_get_object_mask(l) != context_raw.layer_filter) {
		return;
	}

	if (l.parent != null && !l.parent.show_panel) { // Group closed
		return;
	}
	if (l.parent != null && l.parent.parent != null && !l.parent.parent.show_panel) {
		return;
	}

	let step = ui.ops.theme.ELEMENT_H;
	let checkw = (ui._window_w / 100 * 8) / zui_SCALE(ui);

	// Highlight drag destination
	let absy = ui._window_y + ui._y;
	if (base_is_dragging && base_drag_layer != null && context_in_layers()) {
		if (mouse_y > absy + step && mouse_y < absy + step * 3) {
			let down = array_index_of(project_layers, base_drag_layer) >= i;
			context_raw.drag_dest = down ? i : i - 1;

			let ls = project_layers;
			let dest = context_raw.drag_dest;
			let toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
			let nestedGroup = slot_layer_is_group(base_drag_layer) && toGroup;
			if (!nestedGroup) {
				if (slot_layer_can_move(context_raw.layer, context_raw.drag_dest)) {
					zui_fill(checkw, step * 2, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
				}
			}
		}
		else if (i == project_layers.length - 1 && mouse_y < absy + step) {
			context_raw.drag_dest = project_layers.length - 1;
			if (slot_layer_can_move(context_raw.layer, context_raw.drag_dest)) {
				zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
			}
		}
	}
	if (base_is_dragging && (base_drag_material != null || base_drag_swatch != null) && context_in_layers()) {
		if (mouse_y > absy + step && mouse_y < absy + step * 3) {
			context_raw.drag_dest = i;
			if (tab_layers_can_drop_new_layer(i)) {
				zui_fill(checkw, 2 * step, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
			}
		}
		else if (i == project_layers.length - 1 && mouse_y < absy + step) {
			context_raw.drag_dest = project_layers.length;
			if (tab_layers_can_drop_new_layer(project_layers.length)) {
				zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
			}
		}
	}

	mini ? tab_layers_draw_layer_slot_mini(l, i) : tab_layers_draw_layer_slot_full(l, i);

	tab_layers_draw_layer_highlight(l, mini);

	if (tab_layers_show_context_menu) {
		tab_layers_draw_layer_context_menu(l, mini);
	}
}

function tab_layers_draw_layer_slot_mini(l: slot_layer_t, i: i32) {
	let ui = ui_base_ui;

	let row: f32[] = [1, 1];
	zui_row(row);
	let uix = ui._x;
	let uiy = ui._y;
	zui_end_element();
	zui_end_element();

	ui._y += zui_ELEMENT_H(ui);
	ui._y -= zui_ELEMENT_OFFSET(ui);
}

function tab_layers_draw_layer_slot_full(l: slot_layer_t, i: i32) {
	let ui = ui_base_ui;

	let step = ui.ops.theme.ELEMENT_H;

	let hasPanel = slot_layer_is_group(l) || (slot_layer_is_layer(l) && slot_layer_get_masks(l, false) != null);
	if (hasPanel) {
		let row: f32[] = [8 / 100, 52 / 100, 30 / 100, 10 / 100];
		zui_row(row);
	}
	else {
		let row: f32[] = [8 / 100, 52 / 100, 30 / 100];
		zui_row(row);
	}

	// Draw eye icon
	let icons = resource_get("icons.k");
	let r = resource_tile18(icons, l.visible ? 0 : 1, 0);
	let center = (step / 2) * zui_SCALE(ui);
	ui._x += 2;
	ui._y += 3;
	ui._y += center;
	let col = ui.ops.theme.ACCENT_SELECT_COL;
	let parentHidden = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
	if (parentHidden) {
		col -= 0x99000000;
	}

	if (zui_image(icons, col, -1.0, r.x, r.y, r.w, r.h) == zui_state_t.RELEASED) {
		tab_layers_layer_toggle_visible(l);
	}
	ui._x -= 2;
	ui._y -= 3;
	ui._y -= center;

	let uix = ui._x;
	let uiy = ui._y;

	// Draw layer name
	ui._y += center;
	if (tab_layers_layer_name_edit == l.id) {
		tab_layers_layer_name_handle.text = l.name;
		l.name = zui_text_input(tab_layers_layer_name_handle);
		if (ui.text_selected_handle_ptr != tab_layers_layer_name_handle.ptr) {
			tab_layers_layer_name_edit = -1;
		}
	}
	else {
		if (ui.enabled && ui.input_enabled && ui.combo_selected_handle_ptr == 0 &&
			ui.input_x > ui._window_x + ui._x && ui.input_x < ui._window_x + ui._window_w &&
			ui.input_y > ui._window_y + ui._y - center && ui.input_y < ui._window_y + ui._y - center + (step * zui_SCALE(ui)) * 2) {
			if (ui.input_started) {
				context_set_layer(l);
				tab_layers_set_drag_layer(context_raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
			}
			else if (ui.input_released) {
				if (time_time() - context_raw.select_time > 0.2) {
					context_raw.select_time = time_time();
				}
			}
			else if (ui.input_released_r) {
				context_set_layer(l);
				tab_layers_show_context_menu = true;
			}
		}

		let state = zui_text(l.name);
		if (state == zui_state_t.RELEASED) {
			let td = time_time() - context_raw.select_time;
			if (td < 0.2 && td > 0.0) {
				tab_layers_layer_name_edit = l.id;
				tab_layers_layer_name_handle.text = l.name;
				zui_start_text_edit(tab_layers_layer_name_handle);
			}
		}

		// let in_focus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
		// 			  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
		// if (in_focus && ui.is_delete_down && can_delete(context_raw.layer)) {
		// 	ui.is_delete_down = false;
		// 	let _init() = function () {
		// 		deleteLayer(context_raw.layer);
		// 	}
		// 	app_notify_on_init(_init);
		// }
	}
	ui._y -= center;

	if (l.parent != null) {
		ui._x -= 10 * zui_SCALE(ui);
		if (l.parent.parent != null) {
			ui._x -= 10 * zui_SCALE(ui);
		}
	}

	if (slot_layer_is_group(l)) {
		zui_end_element();
	}
	else {
		if (slot_layer_is_mask(l)) {
			ui._y += center;
		}

		// comboBlending(ui, l);
		zui_end_element();

		if (slot_layer_is_mask(l)) {
			ui._y -= center;
		}
	}

	if (hasPanel) {
		ui._y += center;
		let layer_panel = zui_nest(zui_handle(__ID__), l.id);
		layer_panel.selected = l.show_panel;
		l.show_panel = zui_panel(layer_panel, "", true, false, false);
		ui._y -= center;
	}

	if (slot_layer_is_group(l) || slot_layer_is_mask(l)) {
		ui._y -= zui_ELEMENT_OFFSET(ui);
		zui_end_element();
	}
	else {
		ui._y -= zui_ELEMENT_OFFSET(ui);

		let row: f32 = [8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100];
		zui_row(row);
		zui_end_element();
		zui_end_element();
		zui_end_element();

		if (config_raw.touch_ui) {
			ui._x += 12 * zui_SCALE(ui);
		}

		ui._y -= center;
		tab_layers_combo_object(ui, l);
		ui._y += center;

		zui_end_element();
	}

	ui._y -= zui_ELEMENT_OFFSET(ui);
}

function tab_layers_combo_object(ui: zui_t, l: slot_layer_t, label: bool = false): zui_handle_t {
	let ar = [tr("Shared")];
	let object_handle = zui_nest(zui_handle(__ID__), l.id);
	object_handle.position = l.object_mask;
	l.object_mask = zui_combo(object_handle, ar, tr("Object"), label, zui_align_t.LEFT);
	return object_handle;
}

function tab_layers_layer_toggle_visible(l: slot_layer_t) {
	l.visible = !l.visible;
	ui_view2d_hwnd.redraws = 2;
	make_material_parse_mesh_material();
}

function tab_layers_draw_layer_highlight(l: slot_layer_t, mini: bool) {
	let ui = ui_base_ui;
	let step = ui.ops.theme.ELEMENT_H;

	// Separator line
	zui_fill(0, 0, (ui._w / zui_SCALE(ui) - 2), 1 * zui_SCALE(ui), ui.ops.theme.SEPARATOR_COL);

	// Highlight selected
	if (context_raw.layer == l) {
		if (mini) {
			zui_rect(1, -step * 2, ui._w / zui_SCALE(ui) - 1, step * 2 + (mini ? -1 : 1), ui.ops.theme.HIGHLIGHT_COL, 3);
		}
		else {
			zui_rect(1, -step * 2 - 1, ui._w / zui_SCALE(ui) - 2, step * 2 + (mini ? -2 : 1), ui.ops.theme.HIGHLIGHT_COL, 2);
		}
	}
}

function tab_layers_can_merge_down(l: slot_layer_t) : bool {
	let index = array_index_of(project_layers, l);
	// Lowest layer
	if (index == 0) {
		return false;
	}
	// Lowest layer that has masks
	if (slot_layer_is_layer(l) && slot_layer_is_mask(project_layers[0]) && project_layers[0].parent == l) {
		return false;
	}
	// The lowest toplevel layer is a group
	if (slot_layer_is_group(l) && slot_layer_is_in_group(project_layers[0]) && slot_layer_get_containing_group(project_layers[0]) == l) {
		return false;
	}
	// Masks must be merged down to masks
	if (slot_layer_is_mask(l) && !slot_layer_is_mask(project_layers[index - 1])) {
		return false;
	}
	return true;
}

function tab_layers_draw_layer_context_menu(l: slot_layer_t, mini: bool) {

}

function tab_layers_can_drop_new_layer(position: i32) {
	if (position > 0 && position < project_layers.length && slot_layer_is_mask(project_layers[position - 1])) {
		// 1. The layer to insert is inserted in the middle
		// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
		return false;
	}
	return true;
}
