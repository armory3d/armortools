
let tab_layers_layer_name_edit: i32 = -1;
let tab_layers_layer_name_handle: ui_handle_t = ui_handle_create();
let tab_layers_show_context_menu: bool = false;
let tab_layers_l: slot_layer_t;
let tab_layers_mini: bool;

function tab_layers_draw(htab: ui_handle_t) {
	let mini: bool = config_raw.layout[layout_size_t.SIDEBAR_W] <= ui_base_sidebar_mini_w;
	mini ? tab_layers_draw_mini(htab) : tab_layers_draw_full(htab);
}

function tab_layers_draw_mini(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	ui_set_hovered_tab_name(tr("Layers"));

	let _ELEMENT_H: i32 = ui.ops.theme.ELEMENT_H;
	ui.ops.theme.ELEMENT_H = math_floor(ui_base_sidebar_mini_w / 2 / ui_SCALE(ui));

	ui_begin_sticky();
	ui_separator(5);

	tab_layers_combo_filter();
	tab_layers_button_2d_view();
	tab_layers_button_new("+");

	ui_end_sticky();
	ui._y += 2;

	tab_layers_highlight_odd_lines();
	tab_layers_draw_slots(true);

	ui.ops.theme.ELEMENT_H = _ELEMENT_H;
}

function tab_layers_draw_full(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("Layers"))) {
		ui_begin_sticky();
		let row: f32[] = [1 / 4, 1 / 4, 1 / 2];
		ui_row(row);

		tab_layers_button_new(tr("New"));
		tab_layers_button_2d_view();
		tab_layers_combo_filter();

		ui_end_sticky();
		ui._y += 2;

		tab_layers_highlight_odd_lines();
		tab_layers_draw_slots(false);
	}
}

function tab_layers_button_2d_view() {
	let ui: ui_t = ui_base_ui;
	if (ui_button(tr("2D View"))) {
		ui_base_show_2d_view(view_2d_type_t.LAYER);
	}
	else if (ui.is_hovered) {
		ui_tooltip(tr("Show 2D View") + " (" + map_get(config_keymap, "toggle_2d_view") + ")");
	}
}

function tab_layers_draw_slots(mini: bool) {
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		if (i >= project_layers.length) {
			break; // Layer was deleted
		}
		let j: i32 = project_layers.length - 1 - i;
		let l: slot_layer_t = project_layers[j];
		tab_layers_draw_layer_slot(l, j, mini);
	}
}

function tab_layers_highlight_odd_lines() {
	let ui: ui_t = ui_base_ui;
	let step: i32 = ui.ops.theme.ELEMENT_H * 2;
	let full_h: i32 = ui._window_h - ui_base_hwnds[0].scroll_offset;
	for (let i: i32 = 0; i < math_floor(full_h / step); ++i) {
		if (i % 2 == 0) {
			ui_fill(0, i * step, (ui._w / ui_SCALE(ui) - 2), step, ui.ops.theme.WINDOW_BG_COL - 0x00040404);
		}
	}
}

function tab_layers_button_new(text: string) {
	if (ui_button(text)) {
		ui_menu_draw(function (ui: ui_t) {
			let l: slot_layer_t = context_raw.layer;
			if (ui_menu_button(ui, tr("Paint Layer"))) {
				layers_new_layer();
				history_new_layer();
			}
			if (ui_menu_button(ui, tr("Fill Layer"))) {
				layers_create_fill_layer(uv_type_t.UVMAP);
			}
			if (ui_menu_button(ui, tr("Decal Layer"))) {
				layers_create_fill_layer(uv_type_t.PROJECT);
			}
			if (ui_menu_button(ui, tr("Black Mask"))) {
				if (slot_layer_is_mask(l)) {
					context_set_layer(l.parent);
				}
				l = context_raw.layer;

				let m: slot_layer_t = layers_new_mask(false, l);
				app_notify_on_next_frame(function (m: slot_layer_t) {
					slot_layer_clear(m, 0x00000000);
				}, m);
				context_raw.layer_preview_dirty = true;
				history_new_black_mask();
				layers_update_fill_layers();
			}
			if (ui_menu_button(ui, tr("White Mask"))) {
				if (slot_layer_is_mask(l)) {
					context_set_layer(l.parent);
				}
				l = context_raw.layer;

				let m: slot_layer_t = layers_new_mask(false, l);
				app_notify_on_next_frame(function (m: slot_layer_t) {
					slot_layer_clear(m, 0xffffffff);
				}, m);
				context_raw.layer_preview_dirty = true;
				history_new_white_mask();
				layers_update_fill_layers();
			}
			if (ui_menu_button(ui, tr("Fill Mask"))) {
				if (slot_layer_is_mask(l)) {
					context_set_layer(l.parent);
				}
				l = context_raw.layer;

				let m: slot_layer_t = layers_new_mask(false, l);
				app_notify_on_init(function (m: slot_layer_t) {
					slot_layer_to_fill_layer(m);
				}, m);
				context_raw.layer_preview_dirty = true;
				history_new_fill_mask();
				layers_update_fill_layers();
			}
			ui.enabled = !slot_layer_is_group(context_raw.layer) && !slot_layer_is_in_group(context_raw.layer);
			if (ui_menu_button(ui, tr("Group"))) {
				if (slot_layer_is_group(l) || slot_layer_is_in_group(l)) {
					return;
				}

				if (slot_layer_is_layer_mask(l)) {
					l = l.parent;
				}

				let pointers: map_t<slot_layer_t, i32> = tab_layers_init_layer_map();
				let group: slot_layer_t = layers_new_group();
				context_set_layer(l);
				array_remove(project_layers, group);
				array_insert(project_layers, array_index_of(project_layers, l) + 1, group);
				l.parent = group;
				for (let i: i32 = 0; i < project_materials.length; ++i) {
					let m: slot_material_t = project_materials[i];
					tab_layers_remap_layer_pointers(m.canvas.nodes, tab_layers_fill_layer_map(pointers));
				}
				context_set_layer(group);
				history_new_group();
			}
			ui.enabled = true;
		});
	}
}

function tab_layers_combo_filter() {
	let ar: string[] = [tr("All")];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		array_push(ar, p.base.name);
	}
	let atlases: string[] = project_get_used_atlases();
	if (atlases != null) {
		for (let i: i32 = 0; i < atlases.length; ++i) {
			let a: string = atlases[i];
			array_push(ar, a);
		}
	}
	let filter_handle: ui_handle_t = ui_handle(__ID__);
	filter_handle.position = context_raw.layer_filter;
	context_raw.layer_filter = ui_combo(filter_handle, ar, tr("Filter"), false, ui_align_t.LEFT);
	if (filter_handle.changed) {
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let p: mesh_object_t = project_paint_objects[i];
			let filter_name: string = ar[context_raw.layer_filter];
			p.base.visible = context_raw.layer_filter == 0 || p.base.name == filter_name || project_is_atlas_object(p);
		}
		if (context_raw.layer_filter == 0 && context_raw.merged_object_is_atlas) { // All
			util_mesh_merge();
		}
		else if (context_raw.layer_filter > project_paint_objects.length) { // Atlas
			let visibles: mesh_object_t[] = [];
			for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
				let p: mesh_object_t = project_paint_objects[i];
				if (p.base.visible) {
					array_push(visibles, p);
				}
			}
			util_mesh_merge(visibles);
		}
		layers_set_object_mask();
		util_uv_uvmap_cached = false;
		context_raw.ddirty = 2;
		render_path_raytrace_ready = false;
	}
}

function tab_layers_remap_layer_pointers(nodes: ui_node_t[], pointer_map: map_t<i32, i32>) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = nodes[i];
		if (n.type == "LAYER" || n.type == "LAYER_MASK") {
			let i: i32 = n.buttons[0].default_value[0];
			if (map_get(pointer_map, i) != -1) {
				n.buttons[0].default_value[0] = map_get(pointer_map, i);
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
		let l: string = keys[i];
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
	let ui: ui_t = ui_base_ui;

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

	let step: i32 = ui.ops.theme.ELEMENT_H;
	let checkw: f32 = (ui._window_w / 100 * 8) / ui_SCALE(ui);

	// Highlight drag destination
	let absy: f32 = ui._window_y + ui._y;
	if (base_is_dragging && base_drag_layer != null && context_in_layers()) {
		if (mouse_y > absy + step && mouse_y < absy + step * 3) {
			let down: bool = array_index_of(project_layers, base_drag_layer) >= i;
			context_raw.drag_dest = down ? i : i - 1;

			let ls: slot_layer_t[] = project_layers;
			let dest: i32 = context_raw.drag_dest;
			let to_group: bool = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
			let nested_group: bool = slot_layer_is_group(base_drag_layer) && to_group;
			if (!nested_group) {
				if (slot_layer_can_move(context_raw.layer, context_raw.drag_dest)) {
					ui_fill(checkw, step * 2, (ui._window_w / ui_SCALE(ui) - 2) - checkw, 2 * ui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
				}
			}
		}
		else if (i == project_layers.length - 1 && mouse_y < absy + step) {
			context_raw.drag_dest = project_layers.length - 1;
			if (slot_layer_can_move(context_raw.layer, context_raw.drag_dest)) {
				ui_fill(checkw, 0, (ui._window_w / ui_SCALE(ui) - 2) - checkw, 2 * ui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
			}
		}
	}
	if (base_is_dragging && (base_drag_material != null || base_drag_swatch != null) && context_in_layers()) {
		if (mouse_y > absy + step && mouse_y < absy + step * 3) {
			context_raw.drag_dest = i;
			if (tab_layers_can_drop_new_layer(i)) {
				ui_fill(checkw, 2 * step, (ui._window_w / ui_SCALE(ui) - 2) - checkw, 2 * ui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
			}
		}
		else if (i == project_layers.length - 1 && mouse_y < absy + step) {
			context_raw.drag_dest = project_layers.length;
			if (tab_layers_can_drop_new_layer(project_layers.length)) {
				ui_fill(checkw, 0, (ui._window_w / ui_SCALE(ui) - 2) - checkw, 2 * ui_SCALE(ui), ui.ops.theme.HIGHLIGHT_COL);
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
	let ui: ui_t = ui_base_ui;

	let row: f32[] = [1, 1];
	ui_row(row);
	let uix: f32 = ui._x;
	let uiy: f32 = ui._y;
	let state: ui_state_t = tab_layers_draw_layer_icon(l, i, uix, uiy, true);
	tab_layers_handle_layer_icon_state(l, i, state, uix, uiy);
	_ui_end_element();

	ui._y += ui_ELEMENT_H(ui);
	ui._y -= ui_ELEMENT_OFFSET(ui);
}

function tab_layers_draw_layer_slot_full(l: slot_layer_t, i: i32) {
	let ui: ui_t = ui_base_ui;

	let step: i32 = ui.ops.theme.ELEMENT_H;

	let has_panel: bool = slot_layer_is_group(l) || (slot_layer_is_layer(l) && slot_layer_get_masks(l, false) != null);
	if (has_panel) {
		let row: f32[] = [8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100];
		ui_row(row);
	}
	else {
		let row: f32[] = [8 / 100, 16 / 100, 36 / 100, 30 / 100];
		ui_row(row);
	}

	// Draw eye icon
	let icons: image_t = resource_get("icons.k");
	let r: rect_t = resource_tile18(icons, l.visible ? 0 : 1, 0);
	let center: f32 = (step / 2) * ui_SCALE(ui);
	ui._x += 2;
	ui._y += 3;
	ui._y += center;
	let col: i32 = ui.ops.theme.ACCENT_COL;
	let parent_hidden: bool = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
	if (parent_hidden) {
		col -= 0x99000000;
	}

	if (_ui_image(icons, col, -1.0, r.x, r.y, r.w, r.h) == ui_state_t.RELEASED) {
		tab_layers_layer_toggle_visible(l);
	}
	ui._x -= 2;
	ui._y -= 3;
	ui._y -= center;

	let uix: f32 = ui._x;
	let uiy: f32 = ui._y;
	ui._x += 2;
	ui._y += 3;
	if (l.parent != null) {
		ui._x += 10 * ui_SCALE(ui);
		if (l.parent.parent != null) ui._x += 10 * ui_SCALE(ui);
	}

	let state: ui_state_t = tab_layers_draw_layer_icon(l, i, uix, uiy, false);

	ui._x -= 2;
	ui._y -= 3;

	if (config_raw.touch_ui) {
		ui._x += 12 * ui_SCALE(ui);
	}

	tab_layers_handle_layer_icon_state(l, i, state, uix, uiy);

	// Draw layer name
	ui._y += center;
	if (tab_layers_layer_name_edit == l.id) {
		tab_layers_layer_name_handle.text = l.name;
		l.name = ui_text_input(tab_layers_layer_name_handle);
		if (ui.text_selected_handle != tab_layers_layer_name_handle) {
			tab_layers_layer_name_edit = -1;
		}
	}
	else {
		if (ui.enabled && ui.input_enabled && ui.combo_selected_handle == null &&
			ui.input_x > ui._window_x + ui._x && ui.input_x < ui._window_x + ui._window_w &&
			ui.input_y > ui._window_y + ui._y - center && ui.input_y < ui._window_y + ui._y - center + (step * ui_SCALE(ui)) * 2) {
			if (ui.input_started) {
				context_set_layer(l);
				tab_layers_set_drag_layer(context_raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
			}
			else if (ui.input_released_r) {
				context_set_layer(l);
				tab_layers_show_context_menu = true;
			}
		}

		let state: ui_state_t = ui_text(l.name);
		if (state == ui_state_t.RELEASED) {
			if (time_time() - context_raw.select_time < 0.25) {
				tab_layers_layer_name_edit = l.id;
				tab_layers_layer_name_handle.text = l.name;
				ui_start_text_edit(tab_layers_layer_name_handle);
			}
			context_raw.select_time = time_time();
		}

		let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
							 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (in_focus && ui.is_delete_down && tab_layers_can_delete(context_raw.layer)) {
			ui.is_delete_down = false;
			app_notify_on_init(function () {
				tab_layers_delete_layer(context_raw.layer);
			});
		}
	}
	ui._y -= center;

	if (l.parent != null) {
		ui._x -= 10 * ui_SCALE(ui);
		if (l.parent.parent != null) ui._x -= 10 * ui_SCALE(ui);
	}

	if (slot_layer_is_group(l)) {
		_ui_end_element();
	}
	else {
		if (slot_layer_is_mask(l)) {
			ui._y += center;
		}

		tab_layers_combo_blending(ui, l);

		if (slot_layer_is_mask(l)) {
			ui._y -= center;
		}
	}

	if (has_panel) {
		ui._y += center;
		let layer_panel: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
		layer_panel.selected = l.show_panel;
		l.show_panel = ui_panel(layer_panel, "", true, false);
		ui._y -= center;
	}

	if (slot_layer_is_group(l) || slot_layer_is_mask(l)) {
		ui._y -= ui_ELEMENT_OFFSET(ui);
		_ui_end_element();
	}
	else {
		ui._y -= ui_ELEMENT_OFFSET(ui);

		let row: f32[] = [8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100];
		ui_row(row);
		_ui_end_element();
		_ui_end_element();
		_ui_end_element();

		if (config_raw.touch_ui) {
			ui._x += 12 * ui_SCALE(ui);
		}

		tab_layers_combo_object(ui, l);
		_ui_end_element();
	}

	ui._y -= ui_ELEMENT_OFFSET(ui);
}

function tab_layers_combo_object(ui: ui_t, l: slot_layer_t, label: bool = false): ui_handle_t {
	let ar: string[] = [tr("Shared")];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		array_push(ar, p.base.name);
	}
	let atlases: string[] = project_get_used_atlases();
	if (atlases != null) {
		for (let i: i32 = 0; i < atlases.length; ++i) {
			let a: string = atlases[i];
			array_push(ar, a);
		}
	}
	let object_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
	object_handle.position = l.object_mask;
	l.object_mask = ui_combo(object_handle, ar, tr("Object"), label, ui_align_t.LEFT);
	if (object_handle.changed) {
		context_set_layer(l);
		make_material_parse_mesh_material();
		if (l.fill_layer != null) { // Fill layer
			app_notify_on_init(function (l: slot_layer_t) {
				context_raw.material = l.fill_layer;
				slot_layer_clear(l);
				layers_update_fill_layers();
			}, l);
		}
		else {
			layers_set_object_mask();
		}
	}
	return object_handle;
}

function tab_layers_combo_blending(ui: ui_t, l: slot_layer_t, label: bool = false): ui_handle_t {
	let blending_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
	blending_handle.position = l.blending;
	let blending_combo: string[] = [
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
	];
	ui_combo(blending_handle, blending_combo, tr("Blending"), label);
	if (blending_handle.changed) {
		context_set_layer(l);
		history_layer_blending();
		l.blending = blending_handle.position;
		make_material_parse_mesh_material();
	}
	return blending_handle;
}

function tab_layers_layer_toggle_visible(l: slot_layer_t) {
	l.visible = !l.visible;
	ui_view2d_hwnd.redraws = 2;
	make_material_parse_mesh_material();
}

function tab_layers_draw_layer_highlight(l: slot_layer_t, mini: bool) {
	let ui: ui_t = ui_base_ui;
	let step: i32 = ui.ops.theme.ELEMENT_H;

	// Separator line
	ui_fill(0, 0, (ui._w / ui_SCALE(ui) - 2), 1 * ui_SCALE(ui), ui.ops.theme.SEPARATOR_COL);

	// Highlight selected
	if (context_raw.layer == l) {
		if (mini) {
			ui_rect(1, -step * 2, ui._w / ui_SCALE(ui) - 1, step * 2 + (mini ? -1 : 1), ui.ops.theme.HIGHLIGHT_COL, 3);
		}
		else {
			ui_rect(1, -step * 2 - 1, ui._w / ui_SCALE(ui) - 2, step * 2 + (mini ? -2 : 1), ui.ops.theme.HIGHLIGHT_COL, 2);
		}
	}
}

function tab_layers_handle_layer_icon_state(l: slot_layer_t, i: i32, state: ui_state_t, uix: f32, uiy: f32) {
	let ui: ui_t = ui_base_ui;

	let texpaint_preview: image_t = l.texpaint_preview;

	tab_layers_show_context_menu = false;

	// Layer preview tooltip
	if (ui.is_hovered && texpaint_preview != null) {
		if (slot_layer_is_mask(l)) {
			tab_layers_make_mask_preview_rgba32(l);
			_ui_tooltip_image(context_raw.mask_preview_rgba32);
		}
		else {
			_ui_tooltip_image(texpaint_preview);
		}
		if (i < 9) {
			let i1: i32 = (i + 1);
			ui_tooltip(l.name + " - (" + map_get(config_keymap, "select_layer") + " " + i1 + ")");
		}
		else {
			ui_tooltip(l.name);
		}
	}

	// Show context menu
	if (ui.is_hovered && ui.input_released_r) {
		context_set_layer(l);
		tab_layers_show_context_menu = true;
	}

	if (state == ui_state_t.STARTED) {
		context_set_layer(l);
		tab_layers_set_drag_layer(context_raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
	}
	else if (state == ui_state_t.RELEASED) {
		if (time_time() - context_raw.select_time < 0.2) {
			ui_base_show_2d_view(view_2d_type_t.LAYER);
		}
		if (time_time() - context_raw.select_time > 0.2) {
			context_raw.select_time = time_time();
		}
		if (l.fill_layer != null) {
			context_set_material(l.fill_layer);
		}
	}
}

function tab_layers_draw_layer_icon(l: slot_layer_t, i: i32, uix: f32, uiy: f32, mini: bool): ui_state_t {
	let ui: ui_t = ui_base_ui;
	let icons: image_t = resource_get("icons.k");
	let icon_h: i32 = (ui_ELEMENT_H(ui) - (mini ? 2 : 3)) * 2;

	if (mini && ui_SCALE(ui) > 1) {
		ui._x -= 1 * ui_SCALE(ui);
	}

	if (l.parent != null) {
		ui._x += (icon_h - icon_h * 0.9) / 2;
		icon_h *= 0.9;
		if (l.parent.parent != null) {
			ui._x += (icon_h - icon_h * 0.9) / 2;
			icon_h *= 0.9;
		}
	}

	if (!slot_layer_is_group(l)) {
		let texpaint_preview: image_t = l.texpaint_preview;

		let icon: image_t = l.fill_layer == null ? texpaint_preview : l.fill_layer.image_icon;
		if (l.fill_layer == null) {
			// Checker
			let r: rect_t = resource_tile50(icons, 4, 1);
			let _x: f32 = ui._x;
			let _y: f32 = ui._y;
			let _w: f32 = ui._w;
			_ui_image(icons, 0xffffffff, icon_h, r.x, r.y, r.w, r.h);
			ui.current_ratio--;
			ui._x = _x;
			ui._y = _y;
			ui._w = _w;
		}
		if (l.fill_layer == null && slot_layer_is_mask(l)) {
			g2_set_pipeline(ui_view2d_pipe);
			iron_g4_set_int(ui_view2d_channel_loc, 1);
		}

		let state: ui_state_t = _ui_image(icon, 0xffffffff, icon_h);

		if (l.fill_layer == null && slot_layer_is_mask(l)) {
			g2_set_pipeline(null);
		}

		// Draw layer numbers when selecting a layer via keyboard shortcut
		let is_typing: bool = ui.is_typing || ui_view2d_ui.is_typing || ui_nodes_ui.is_typing;
		if (!is_typing) {
			if (i < 9 && operator_shortcut(map_get(config_keymap, "select_layer"), shortcut_type_t.DOWN)) {
				let number: string = i32_to_string(i + 1) ;
				let width: i32 = g2_font_width(ui.ops.font, ui.font_size, number) + 10;
				let height: i32 = g2_font_height(ui.ops.font, ui.font_size);
				g2_set_color(ui.ops.theme.TEXT_COL);
				g2_fill_rect(uix, uiy, width, height);
				g2_set_color(ui.ops.theme.BUTTON_COL);
				g2_draw_string(number, uix + 5, uiy);
			}
		}

		return state;
	}
	else { // Group
		let folder_closed: rect_t = resource_tile50(icons, 2, 1);
		let folder_open: rect_t = resource_tile50(icons, 8, 1);
		let folder: rect_t = l.show_panel ? folder_open : folder_closed;
		return _ui_image(icons, ui.ops.theme.LABEL_COL - 0x00202020, icon_h, folder.x, folder.y, folder.w, folder.h);
	}
}

function tab_layers_can_merge_down(l: slot_layer_t): bool {
	let index: i32 = array_index_of(project_layers, l);
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

	tab_layers_l = l;
	tab_layers_mini = mini;

	ui_menu_draw(function (ui: ui_t) {

		let l: slot_layer_t = tab_layers_l;
		let mini: bool = tab_layers_mini;

		if (mini) {
			let visible_handle: ui_handle_t = ui_handle(__ID__);
			visible_handle.selected = l.visible;
			ui_check(visible_handle, tr("Visible"));
			if (visible_handle.changed) {
				tab_layers_layer_toggle_visible(l);
				ui_menu_keep_open = true;
			}

			if (!slot_layer_is_group(l)) {
				if (tab_layers_combo_blending(ui, l, true).changed) {
					ui_menu_keep_open = true;
				}
			}
			if (slot_layer_is_layer(l)) {
				if (tab_layers_combo_object(ui, l, true).changed) {
					ui_menu_keep_open = true;
				}
			}
		}

		if (ui_menu_button(ui, tr("Export"))) {
			if (slot_layer_is_mask(l)) {
				ui_files_show("png", true, false, function (path: string) {
					let l: slot_layer_t = tab_layers_l;
					let f: string = ui_files_filename;
					if (f == "") {
						f = tr("untitled");
					}
					if (!ends_with(f, ".png")) {
						f += ".png";
					}
					iron_write_png(path + path_sep + f, image_get_pixels(l.texpaint), l.texpaint.width, l.texpaint.height, 3); // RRR1
				});
			}
			else {
				context_raw.layers_export = export_mode_t.SELECTED;
				box_export_show_textures();
			}
		}

		if (!slot_layer_is_group(l)) {
			let to_fill_string: string = slot_layer_is_layer(l) ? tr("To Fill Layer") : tr("To Fill Mask");
			let to_paint_string: string = slot_layer_is_layer(l) ? tr("To Paint Layer") : tr("To Paint Mask");

			if (l.fill_layer == null && ui_menu_button(ui, to_fill_string)) {
				app_notify_on_init(function () {
					let l: slot_layer_t = tab_layers_l;
					slot_layer_is_layer(l) ? history_to_fill_layer() : history_to_fill_mask();
					slot_layer_to_fill_layer(l);
				});
			}
			if (l.fill_layer != null && ui_menu_button(ui, to_paint_string)) {
				app_notify_on_init(function () {
					let l: slot_layer_t = tab_layers_l;
					slot_layer_is_layer(l) ? history_to_paint_layer() : history_to_paint_mask();
					slot_layer_to_paint_layer(l);
				});
			}
		}

		ui.enabled = tab_layers_can_delete(l);
		if (ui_menu_button(ui, tr("Delete"), "delete")) {
			app_notify_on_init(function () {
				tab_layers_delete_layer(context_raw.layer);
			});
		}
		ui.enabled = true;

		if (l.fill_layer == null && ui_menu_button(ui, tr("Clear"))) {
			context_set_layer(l);
			app_notify_on_init(function () {
				let l: slot_layer_t = tab_layers_l;
				if (!slot_layer_is_group(l)) {
					history_clear_layer();
					slot_layer_clear(l);
				}
				else {
					for (let i: i32 = 0; i < slot_layer_get_children(l).length; ++i) {
						let c: slot_layer_t = slot_layer_get_children(l)[i];
						context_raw.layer = c;
						history_clear_layer();
						slot_layer_clear(c);
					}
					context_raw.layers_preview_dirty = true;
					context_raw.layer = l;
				}
			});
		}
		if (slot_layer_is_mask(l) && l.fill_layer == null && ui_menu_button(ui, tr("Invert"))) {
			app_notify_on_init(function () {
				let l: slot_layer_t = tab_layers_l;
				context_set_layer(l);
				history_invert_mask();
				slot_layer_invert_mask(l);
			});
		}
		if (slot_layer_is_mask(l) && ui_menu_button(ui, tr("Apply"))) {
			app_notify_on_init(function () {
				let l: slot_layer_t = tab_layers_l;
				context_raw.layer = l;
				history_apply_mask();
				slot_layer_apply_mask(l);
				context_set_layer(l.parent);
				make_material_parse_mesh_material();
				context_raw.layers_preview_dirty = true;
			});
		}
		if (slot_layer_is_group(l) && ui_menu_button(ui, tr("Merge Group"))) {
			app_notify_on_init(function () {
				let l: slot_layer_t = tab_layers_l;
				layers_merge_group(l);
			});
		}
		ui.enabled = tab_layers_can_merge_down(l);
		if (ui_menu_button(ui, tr("Merge Down"))) {
			app_notify_on_init(function () {
				let l: slot_layer_t = tab_layers_l;
				context_set_layer(l);
				history_merge_layers();
				layers_merge_down();
				if (context_raw.layer.fill_layer != null) slot_layer_to_paint_layer(context_raw.layer);
			});
		}
		ui.enabled = true;
		if (ui_menu_button(ui, tr("Duplicate"))) {
			app_notify_on_init(function () {
				let l: slot_layer_t = tab_layers_l;
				context_set_layer(l);
				history_duplicate_layer();
				layers_duplicate_layer(l);
			});
		}

		ui_menu_align(ui);
		let layer_opac_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
		layer_opac_handle.value = l.mask_opacity;
		ui_slider(layer_opac_handle, tr("Opacity"), 0.0, 1.0, true);
		if (layer_opac_handle.changed) {
			if (ui.input_started) {
				history_layer_opacity();
			}
			l.mask_opacity = layer_opac_handle.value;
			make_material_parse_mesh_material();
			ui_menu_keep_open = true;
		}

		if (!slot_layer_is_group(l)) {
			ui_menu_align(ui);
			let res_handle_changed_last: bool = base_res_handle.changed;
			///if (arm_android || arm_ios)
			let ar: string[] = ["128", "256", "512", "1K", "2K", "4K"];
			///else
			let ar: string[] = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
			///end
			let _y: i32 = ui._y;
			base_res_handle.value = base_res_handle.position;
			base_res_handle.position = math_floor(ui_slider(base_res_handle, ar[base_res_handle.position], 0, ar.length - 1, false, 1, false, ui_align_t.LEFT, false));
			if (base_res_handle.changed) {
				ui_menu_keep_open = true;
			}
			if (res_handle_changed_last && !base_res_handle.changed) {
				layers_on_resized();
			}
			ui._y = _y;
			ui_draw_string(tr("Res"), -1, 0, ui_align_t.RIGHT, true);
			_ui_end_element();

			ui_menu_align(ui);
			///if (arm_android || arm_ios)
			let bits_items: string[] = ["8bit"];
			ui_inline_radio(base_bits_handle, bits_items, ui_align_t.LEFT);
			///else
			let bits_items: string[] = ["8bit", "16bit", "32bit"];
			ui_inline_radio(base_bits_handle, bits_items, ui_align_t.LEFT);
			///end
			if (base_bits_handle.changed) {
				app_notify_on_init(layers_set_bits);
				ui_menu_keep_open = true;
			}
		}

		if (l.fill_layer != null) {
			ui_menu_align(ui);
			let scale_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			scale_handle.value = l.scale;
			l.scale = ui_slider(scale_handle, tr("UV Scale"), 0.0, 5.0, true);
			if (scale_handle.changed) {
				context_set_material(l.fill_layer);
				context_set_layer(l);
				app_notify_on_init(function () {
					layers_update_fill_layers();
				});
				ui_menu_keep_open = true;
			}

			ui_menu_align(ui);
			let angle_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			angle_handle.value = l.angle;
			l.angle = ui_slider(angle_handle, tr("Angle"), 0.0, 360, true, 1);
			if (angle_handle.changed) {
				context_set_material(l.fill_layer);
				context_set_layer(l);
				make_material_parse_paint_material();
				app_notify_on_init(function () {
					layers_update_fill_layers();
				});
				ui_menu_keep_open = true;
			}

			ui_menu_align(ui);
			let uv_type_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			uv_type_handle.position = l.uv_type;
			let uv_type_items: string[] = [tr("UV Map"), tr("Triplanar"), tr("Project")];
			l.uv_type = ui_inline_radio(uv_type_handle, uv_type_items, ui_align_t.LEFT);
			if (uv_type_handle.changed) {
				context_set_material(l.fill_layer);
				context_set_layer(l);
				make_material_parse_paint_material();
				app_notify_on_init(function () {
					layers_update_fill_layers();
				});
				ui_menu_keep_open = true;
			}
		}

		if (!slot_layer_is_group(l)) {
			let base_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let opac_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let nor_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let nor_blend_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let occ_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let rough_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let met_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let height_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let height_blend_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let emis_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			let subs_handle: ui_handle_t = ui_nest(ui_handle(__ID__), l.id);
			base_handle.selected = l.paint_base;
			opac_handle.selected = l.paint_opac;
			nor_handle.selected = l.paint_nor;
			nor_blend_handle.selected = l.paint_nor_blend;
			occ_handle.selected = l.paint_occ;
			rough_handle.selected = l.paint_rough;
			met_handle.selected = l.paint_met;
			height_handle.selected = l.paint_height;
			height_blend_handle.selected = l.paint_height_blend;
			emis_handle.selected = l.paint_emis;
			subs_handle.selected = l.paint_subs;
			l.paint_base = ui_check(base_handle, tr("Base Color"));
			l.paint_opac = ui_check(opac_handle, tr("Opacity"));
			l.paint_nor = ui_check(nor_handle, tr("Normal"));
			l.paint_nor_blend = ui_check(nor_blend_handle, tr("Normal Blending"));
			l.paint_occ = ui_check(occ_handle, tr("Occlusion"));
			l.paint_rough = ui_check(rough_handle, tr("Roughness"));
			l.paint_met = ui_check(met_handle, tr("Metallic"));
			l.paint_height = ui_check(height_handle, tr("Height"));
			l.paint_height_blend = ui_check(height_blend_handle, tr("Height Blending"));
			l.paint_emis = ui_check(emis_handle, tr("Emission"));
			l.paint_subs = ui_check(subs_handle, tr("Subsurface"));
			if (base_handle.changed ||
				opac_handle.changed ||
				nor_handle.changed ||
				nor_blend_handle.changed ||
				occ_handle.changed ||
				rough_handle.changed ||
				met_handle.changed ||
				height_handle.changed ||
				height_blend_handle.changed ||
				emis_handle.changed ||
				subs_handle.changed) {
				make_material_parse_mesh_material();
				ui_menu_keep_open = true;
			}
		}
	});
}

function tab_layers_make_mask_preview_rgba32(l: slot_layer_t) {
	if (context_raw.mask_preview_rgba32 == null) {
		context_raw.mask_preview_rgba32 = image_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size);
	}
	// Convert from R8 to RGBA32 for tooltip display
	if (context_raw.mask_preview_last != l) {
		context_raw.mask_preview_last = l;
		tab_layers_l = l;
		app_notify_on_init(function () {
			let l: slot_layer_t = tab_layers_l;
			g2_begin(context_raw.mask_preview_rgba32);
			g2_set_pipeline(ui_view2d_pipe);
			g4_set_int(ui_view2d_channel_loc, 1);
			g2_draw_image(l.texpaint_preview, 0, 0);
			g2_end();
			g2_set_pipeline(null);
		});
	}
}

function tab_layers_delete_layer(l: slot_layer_t) {
	let pointers: map_t<slot_layer_t, i32> = tab_layers_init_layer_map();

	if (slot_layer_is_layer(l) && slot_layer_has_masks(l, false)) {
		let masks: slot_layer_t[] = slot_layer_get_masks(l, false);
		for (let i: i32 = 0; i < masks.length; ++i) {
			let m: slot_layer_t = masks[i];
			context_raw.layer = m;
			history_delete_layer();
			slot_layer_delete(m);
		}
	}
	if (slot_layer_is_group(l)) {
		let children: slot_layer_t[] = slot_layer_get_children(l);
		for (let i: i32 = 0; i < children.length; ++i) {
			let c: slot_layer_t = children[i];
			if (slot_layer_has_masks(c, false)) {
				let masks: slot_layer_t[] = slot_layer_get_masks(c, false);
				for (let i: i32 = 0; i < masks.length; ++i) {
					let m: slot_layer_t = masks[i];
					context_raw.layer = m;
					history_delete_layer();
					slot_layer_delete(m);
				}
			}
			context_raw.layer = c;
			history_delete_layer();
			slot_layer_delete(c);
		}
		if (slot_layer_has_masks(l)) {
			for (let i: i32 = 0; i < slot_layer_get_masks(l).length; ++i) {
				let m: slot_layer_t = slot_layer_get_masks(l)[i];
				context_raw.layer = m;
				history_delete_layer();
				slot_layer_delete(m);
			}
		}
	}

	context_raw.layer = l;
	history_delete_layer();
	slot_layer_delete(l);

	if (slot_layer_is_mask(l)) {
		context_raw.layer = l.parent;
		layers_update_fill_layers();
	}

	// Remove empty group
	if (slot_layer_is_in_group(l) && slot_layer_get_children(slot_layer_get_containing_group(l)) == null) {
		let g: slot_layer_t = slot_layer_get_containing_group(l);
		// Maybe some group masks are left
		if (slot_layer_has_masks(g)) {
			for (let i: i32 = 0; i < slot_layer_get_masks(g).length; ++i) {
				let m: slot_layer_t = slot_layer_get_masks(g)[i];
				context_raw.layer = m;
				history_delete_layer();
				slot_layer_delete(m);
			}
		}
		context_raw.layer = l.parent;
		history_delete_layer();
		slot_layer_delete(l.parent);
	}
	context_raw.ddirty = 2;
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		tab_layers_remap_layer_pointers(m.canvas.nodes, tab_layers_fill_layer_map(pointers));
	}
}

function tab_layers_can_delete(l: slot_layer_t): bool {
	let num_layers: i32 = 0;

	if (slot_layer_is_mask(l)) {
		return true;
	}

	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let slot: slot_layer_t = project_layers[i];
		if (slot_layer_is_layer(slot)) {
			++num_layers;
		}
	}

	// All layers are in one group
	if (slot_layer_is_group(l) && slot_layer_get_children(l).length == num_layers) {
		return false;
	}

	// Do not delete last layer
	return num_layers > 1;
}

function tab_layers_can_drop_new_layer(position: i32): bool {
	if (position > 0 && position < project_layers.length && slot_layer_is_mask(project_layers[position - 1])) {
		// 1. The layer to insert is inserted in the middle
		// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
		return false;
	}
	return true;
}
