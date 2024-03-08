
class TabLayers {

	static layerNameEdit = -1;
	static layerNameHandle = zui_handle_create();
	static showContextMenu = false;

	static draw = (htab: zui_handle_t) => {
		let mini = Config.raw.layout[layout_size_t.SIDEBAR_W] <= UIBase.sidebar_mini_w;
		mini ? TabLayers.draw_mini(htab) : TabLayers.draw_full(htab);
	}

	static drawMini = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		zui_set_hovered_tab_name(tr("Layers"));

		let _ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Math.floor(UIBase.sidebar_mini_w / 2 / zui_SCALE(ui));

		zui_begin_sticky();
		zui_separator(5);

		TabLayers.combo_filter();
		TabLayers.button_new("+");

		zui_end_sticky();
		ui._y += 2;

		TabLayers.highlight_odd_lines();
		TabLayers.draw_slots(true);

		ui.t.ELEMENT_H = _ELEMENT_H;
	}

	static drawFull = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Layers"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 3 / 4]);

			TabLayers.button_new(tr("New"));
			TabLayers.combo_filter();

			zui_end_sticky();
			ui._y += 2;

			TabLayers.highlight_odd_lines();
			TabLayers.draw_slots(false);
		}
	}

	static drawSlots = (mini: bool) => {
		for (let i = 0; i < Project.layers.length; ++i) {
			if (i >= Project.layers.length) break; // Layer was deleted
			let j = Project.layers.length - 1 - i;
			let l = Project.layers[j];
			TabLayers.draw_layer_slot(l, j, mini);
		}
	}

	static highlightOddLines = () => {
		let ui = UIBase.ui;
		let step = ui.t.ELEMENT_H * 2;
		let fullH = ui._window_h - UIBase.hwnds[0].scroll_offset;
		for (let i = 0; i < Math.floor(fullH / step); ++i) {
			if (i % 2 == 0) {
				zui_fill(0, i * step, (ui._w / zui_SCALE(ui) - 2), step, ui.t.WINDOW_BG_COL - 0x00040404);
			}
		}
	}

	static buttonNew = (text: string) => {
		let ui = UIBase.ui;
		if (zui_button(text)) {
			UIMenu.draw((ui: zui_t) => {
				let l = Context.raw.layer;
				if (UIMenu.menu_button(ui, tr("Paint Layer"))) {
					base_base_new_layer();
					History.new_layer();
				}
			}, 1);
		}
	}

	static comboFilter = () => {
		let ui = UIBase.ui;
		let ar = [tr("All")];
		let filterHandle = zui_handle("tablayers_0");
		filterHandle.position = Context.raw.layer_filter;
		Context.raw.layer_filter = zui_combo(filterHandle, ar, tr("Filter"), false, zui_align_t.LEFT);
	}

	static remapLayerPointers = (nodes: zui_node_t[], pointerMap: Map<i32, i32>) => {
		for (let n of nodes) {
			if (n.type == "LAYER" || n.type == "LAYER_MASK") {
				let i = n.buttons[0].default_value;
				if (pointerMap.has(i)) {
					n.buttons[0].default_value = pointerMap.get(i);
				}
			}
		}
	}

	static initLayerMap = (): Map<SlotLayerRaw, i32> => {
		let res: Map<SlotLayerRaw, i32> = new Map();
		for (let i = 0; i < Project.layers.length; ++i) res.set(Project.layers[i], i);
		return res;
	}

	static fillLayerMap = (map: Map<SlotLayerRaw, i32>): Map<i32, i32> => {
		let res: Map<i32, i32> = new Map();
		for (let l of map.keys()) res.set(map.get(l), Project.layers.indexOf(l) > -1 ? Project.layers.indexOf(l) : 9999);
		return res;
	}

	static setDragLayer = (layer: SlotLayerRaw, offX: f32, offY: f32) => {
		base_base_drag_off_x = offX;
		base_base_drag_off_y = offY;
		base_base_drag_layer = layer;
		Context.raw.drag_dest = Project.layers.indexOf(layer);
	}

	static drawLayerSlot = (l: SlotLayerRaw, i: i32, mini: bool) => {
		let ui = UIBase.ui;

		if (Context.raw.layer_filter > 0 &&
			SlotLayer.get_object_mask(l) > 0 &&
			SlotLayer.get_object_mask(l) != Context.raw.layer_filter) {
			return;
		}

		if (l.parent != null && !l.parent.show_panel) { // Group closed
			return;
		}
		if (l.parent != null && l.parent.parent != null && !l.parent.parent.show_panel) {
			return;
		}

		let step = ui.t.ELEMENT_H;
		let checkw = (ui._window_w / 100 * 8) / zui_SCALE(ui);

		// Highlight drag destination
		let absy = ui._window_y + ui._y;
		if (base_base_is_dragging && base_base_drag_layer != null && Context.in_layers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				let down = Project.layers.indexOf(base_base_drag_layer) >= i;
				Context.raw.drag_dest = down ? i : i - 1;

				let ls = Project.layers;
				let dest = Context.raw.drag_dest;
				let toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				let nestedGroup = SlotLayer.is_group(base_base_drag_layer) && toGroup;
				if (!nestedGroup) {
					if (SlotLayer.can_move(Context.raw.layer, Context.raw.drag_dest)) {
						zui_fill(checkw, step * 2, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
					}
				}
			}
			else if (i == Project.layers.length - 1 && mouse_y < absy + step) {
				Context.raw.drag_dest = Project.layers.length - 1;
				if (SlotLayer.can_move(Context.raw.layer, Context.raw.drag_dest)) {
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
				}
			}
		}
		if (base_base_is_dragging && (base_base_drag_material != null || base_base_drag_swatch != null) && Context.in_layers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				Context.raw.drag_dest = i;
				if (TabLayers.can_drop_new_layer(i))
					zui_fill(checkw, 2 * step, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
			else if (i == Project.layers.length - 1 && mouse_y < absy + step) {
				Context.raw.drag_dest = Project.layers.length;
				if (TabLayers.can_drop_new_layer(Project.layers.length))
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
		}

		mini ? TabLayers.draw_layer_slot_mini(l, i) : TabLayers.draw_layer_slot_full(l, i);

		TabLayers.draw_layer_highlight(l, mini);

		if (TabLayers.show_context_menu) {
			TabLayers.draw_layer_context_menu(l, mini);
		}
	}

	static drawLayerSlotMini = (l: SlotLayerRaw, i: i32) => {
		let ui = UIBase.ui;

		zui_row([1, 1]);
		let uix = ui._x;
		let uiy = ui._y;
		zui_end_element();
		zui_end_element();

		ui._y += zui_ELEMENT_H(ui);
		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static drawLayerSlotFull = (l: SlotLayerRaw, i: i32) => {
		let ui = UIBase.ui;

		let step = ui.t.ELEMENT_H;

		let hasPanel = SlotLayer.is_group(l) || (SlotLayer.is_layer(l) && SlotLayer.get_masks(l, false) != null);
		if (hasPanel) {
			zui_row([8 / 100, 52 / 100, 30 / 100, 10 / 100]);
		}
		else {
			zui_row([8 / 100, 52 / 100, 30 / 100]);
		}

		// Draw eye icon
		let icons = Res.get("icons.k");
		let r = Res.tile18(icons, l.visible ? 0 : 1, 0);
		let center = (step / 2) * zui_SCALE(ui);
		ui._x += 2;
		ui._y += 3;
		ui._y += center;
		let col = ui.t.ACCENT_SELECT_COL;
		let parentHidden = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
		if (parentHidden) col -= 0x99000000;

		if (zui_image(icons, col, -1.0, r.x, r.y, r.w, r.h) == zui_state_t.RELEASED) {
			TabLayers.layer_toggle_visible(l);
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		let uix = ui._x;
		let uiy = ui._y;

		// Draw layer name
		ui._y += center;
		if (TabLayers.layer_name_edit == l.id) {
			TabLayers.layer_name_handle.text = l.name;
			l.name = zui_text_input(TabLayers.layer_name_handle);
			if (ui.text_selected_handle_ptr != TabLayers.layer_name_handle.ptr) TabLayers.layer_name_edit = -1;
		}
		else {
			if (ui.enabled && ui.input_enabled && ui.combo_selected_handle_ptr == 0 &&
				ui.input_x > ui._window_x + ui._x && ui.input_x < ui._window_x + ui._window_w &&
				ui.input_y > ui._window_y + ui._y - center && ui.input_y < ui._window_y + ui._y - center + (step * zui_SCALE(ui)) * 2) {
				if (ui.input_started) {
					Context.set_layer(l);
					TabLayers.set_drag_layer(Context.raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
				}
				else if (ui.input_released) {
					if (time_time() - Context.raw.select_time > 0.2) {
						Context.raw.select_time = time_time();
					}
				}
				else if (ui.input_released_r) {
					Context.set_layer(l);
					TabLayers.show_context_menu = true;
				}
			}

			let state = zui_text(l.name);
			if (state == zui_state_t.RELEASED) {
				let td = time_time() - Context.raw.select_time;
				if (td < 0.2 && td > 0.0) {
					TabLayers.layer_name_edit = l.id;
					TabLayers.layer_name_handle.text = l.name;
					zui_start_text_edit(TabLayers.layer_name_handle);
				}
			}

			// let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
			// 			  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			// if (inFocus && ui.isDeleteDown && canDelete(Context.raw.layer)) {
			// 	ui.isDeleteDown = false;
			// 	let _init() = () => {
			// 		deleteLayer(Context.raw.layer);
			// 	}
			// 	app_notify_on_init(_init);
			// }
		}
		ui._y -= center;

		if (l.parent != null) {
			ui._x -= 10 * zui_SCALE(ui);
			if (l.parent.parent != null) ui._x -= 10 * zui_SCALE(ui);
		}

		if (SlotLayer.is_group(l)) {
			zui_end_element();
		}
		else {
			if (SlotLayer.is_mask(l)) {
				ui._y += center;
			}

			// comboBlending(ui, l);
			zui_end_element();

			if (SlotLayer.is_mask(l)) {
				ui._y -= center;
			}
		}

		if (hasPanel) {
			ui._y += center;
			let layerPanel = zui_nest(zui_handle("tablayers_1"), l.id);
			layerPanel.selected = l.show_panel;
			l.show_panel = zui_panel(layerPanel, "", true, false, false);
			ui._y -= center;
		}

		if (SlotLayer.is_group(l) || SlotLayer.is_mask(l)) {
			ui._y -= zui_ELEMENT_OFFSET(ui);
			zui_end_element();
		}
		else {
			ui._y -= zui_ELEMENT_OFFSET(ui);

			zui_row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
			zui_end_element();
			zui_end_element();
			zui_end_element();

			if (Config.raw.touch_ui) {
				ui._x += 12 * zui_SCALE(ui);
			}

			ui._y -= center;
			TabLayers.combo_object(ui, l);
			ui._y += center;

			zui_end_element();
		}

		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static comboObject = (ui: zui_t, l: SlotLayerRaw, label = false): zui_handle_t => {
		let ar = [tr("Shared")];
		let objectHandle = zui_nest(zui_handle("tablayers_2"), l.id);
		objectHandle.position = l.object_mask;
		l.object_mask = zui_combo(objectHandle, ar, tr("Object"), label, zui_align_t.LEFT);
		return objectHandle;
	}

	static layerToggleVisible = (l: SlotLayerRaw) => {
		l.visible = !l.visible;
		UIView2D.hwnd.redraws = 2;
		MakeMaterial.parse_mesh_material();
	}

	static drawLayerHighlight = (l: SlotLayerRaw, mini: bool) => {
		let ui = UIBase.ui;
		let step = ui.t.ELEMENT_H;

		// Separator line
		zui_fill(0, 0, (ui._w / zui_SCALE(ui) - 2), 1 * zui_SCALE(ui), ui.t.SEPARATOR_COL);

		// Highlight selected
		if (Context.raw.layer == l) {
			if (mini) {
				zui_rect(1, -step * 2, ui._w / zui_SCALE(ui) - 1, step * 2 + (mini ? -1 : 1), ui.t.HIGHLIGHT_COL, 3);
			}
			else {
				zui_rect(1, -step * 2 - 1, ui._w / zui_SCALE(ui) - 2, step * 2 + (mini ? -2 : 1), ui.t.HIGHLIGHT_COL, 2);
			}
		}
	}

	static canMergeDown = (l: SlotLayerRaw) : bool => {
		let index = Project.layers.indexOf(l);
		// Lowest layer
		if (index == 0) return false;
		// Lowest layer that has masks
		if (SlotLayer.is_layer(l) && SlotLayer.is_mask(Project.layers[0]) && Project.layers[0].parent == l) return false;
		// The lowest toplevel layer is a group
		if (SlotLayer.is_group(l) && SlotLayer.is_in_group(Project.layers[0]) && SlotLayer.get_containing_group(Project.layers[0]) == l) return false;
		// Masks must be merged down to masks
		if (SlotLayer.is_mask(l) && !SlotLayer.is_mask(Project.layers[index - 1])) return false;
		return true;
	}

	static drawLayerContextMenu = (l: SlotLayerRaw, mini: bool) => {

	}

	static canDropNewLayer = (position: i32) => {
		if (position > 0 && position < Project.layers.length && SlotLayer.is_mask(Project.layers[position - 1])) {
			// 1. The layer to insert is inserted in the middle
			// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
			return false;
		}
		return true;
	}
}
