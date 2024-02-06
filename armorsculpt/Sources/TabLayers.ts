
class TabLayers {

	static layerNameEdit = -1;
	static layerNameHandle = zui_handle_create();
	static showContextMenu = false;

	static draw = (htab: zui_handle_t) => {
		let mini = Config.raw.layout[LayoutSize.LayoutSidebarW] <= UIBase.sidebarMiniW;
		mini ? TabLayers.drawMini(htab) : TabLayers.drawFull(htab);
	}

	static drawMini = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		zui_set_hovered_tab_name(tr("Layers"));

		let _ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Math.floor(UIBase.sidebarMiniW / 2 / zui_SCALE(ui));

		zui_begin_sticky();
		zui_separator(5);

		TabLayers.comboFilter();
		TabLayers.buttonNew("+");

		zui_end_sticky();
		ui._y += 2;

		TabLayers.highlightOddLines();
		TabLayers.drawSlots(true);

		ui.t.ELEMENT_H = _ELEMENT_H;
	}

	static drawFull = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Layers"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 3 / 4]);

			TabLayers.buttonNew(tr("New"));
			TabLayers.comboFilter();

			zui_end_sticky();
			ui._y += 2;

			TabLayers.highlightOddLines();
			TabLayers.drawSlots(false);
		}
	}

	static drawSlots = (mini: bool) => {
		for (let i = 0; i < Project.layers.length; ++i) {
			if (i >= Project.layers.length) break; // Layer was deleted
			let j = Project.layers.length - 1 - i;
			let l = Project.layers[j];
			TabLayers.drawLayerSlot(l, j, mini);
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
				if (UIMenu.menuButton(ui, tr("Paint Layer"))) {
					Base.newLayer();
					History.newLayer();
				}
			}, 1);
		}
	}

	static comboFilter = () => {
		let ui = UIBase.ui;
		let ar = [tr("All")];
		let filterHandle = zui_handle("tablayers_0");
		filterHandle.position = Context.raw.layerFilter;
		Context.raw.layerFilter = zui_combo(filterHandle, ar, tr("Filter"), false, Align.Left);
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
		Base.dragOffX = offX;
		Base.dragOffY = offY;
		Base.dragLayer = layer;
		Context.raw.dragDestination = Project.layers.indexOf(layer);
	}

	static drawLayerSlot = (l: SlotLayerRaw, i: i32, mini: bool) => {
		let ui = UIBase.ui;

		if (Context.raw.layerFilter > 0 &&
			SlotLayer.getObjectMask(l) > 0 &&
			SlotLayer.getObjectMask(l) != Context.raw.layerFilter) {
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
		if (Base.isDragging && Base.dragLayer != null && Context.inLayers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				let down = Project.layers.indexOf(Base.dragLayer) >= i;
				Context.raw.dragDestination = down ? i : i - 1;

				let ls = Project.layers;
				let dest = Context.raw.dragDestination;
				let toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				let nestedGroup = SlotLayer.isGroup(Base.dragLayer) && toGroup;
				if (!nestedGroup) {
					if (SlotLayer.canMove(Context.raw.layer, Context.raw.dragDestination)) {
						zui_fill(checkw, step * 2, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
					}
				}
			}
			else if (i == Project.layers.length - 1 && mouse_y < absy + step) {
				Context.raw.dragDestination = Project.layers.length - 1;
				if (SlotLayer.canMove(Context.raw.layer, Context.raw.dragDestination)) {
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
				}
			}
		}
		if (Base.isDragging && (Base.dragMaterial != null || Base.dragSwatch != null) && Context.inLayers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				Context.raw.dragDestination = i;
				if (TabLayers.canDropNewLayer(i))
					zui_fill(checkw, 2 * step, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
			else if (i == Project.layers.length - 1 && mouse_y < absy + step) {
				Context.raw.dragDestination = Project.layers.length;
				if (TabLayers.canDropNewLayer(Project.layers.length))
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
		}

		mini ? TabLayers.drawLayerSlotMini(l, i) : TabLayers.drawLayerSlotFull(l, i);

		TabLayers.drawLayerHighlight(l, mini);

		if (TabLayers.showContextMenu) {
			TabLayers.drawLayerContextMenu(l, mini);
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

		let hasPanel = SlotLayer.isGroup(l) || (SlotLayer.isLayer(l) && SlotLayer.getMasks(l, false) != null);
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

		if (zui_image(icons, col, null, r.x, r.y, r.w, r.h) == State.Released) {
			TabLayers.layerToggleVisible(l);
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		let uix = ui._x;
		let uiy = ui._y;

		// Draw layer name
		ui._y += center;
		if (TabLayers.layerNameEdit == l.id) {
			TabLayers.layerNameHandle.text = l.name;
			l.name = zui_text_input(TabLayers.layerNameHandle);
			if (ui.text_selected_handle_ptr != TabLayers.layerNameHandle.ptr) TabLayers.layerNameEdit = -1;
		}
		else {
			if (ui.enabled && ui.input_enabled && ui.combo_selected_handle_ptr == null &&
				ui.input_x > ui._window_x + ui._x && ui.input_x < ui._window_x + ui._window_w &&
				ui.input_y > ui._window_y + ui._y - center && ui.input_y < ui._window_y + ui._y - center + (step * zui_SCALE(ui)) * 2) {
				if (ui.input_started) {
					Context.setLayer(l);
					TabLayers.setDragLayer(Context.raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
				}
				else if (ui.input_released) {
					if (time_time() - Context.raw.selectTime > 0.2) {
						Context.raw.selectTime = time_time();
					}
				}
				else if (ui.input_released_r) {
					Context.setLayer(l);
					TabLayers.showContextMenu = true;
				}
			}

			let state = zui_text(l.name);
			if (state == State.Released) {
				let td = time_time() - Context.raw.selectTime;
				if (td < 0.2 && td > 0.0) {
					TabLayers.layerNameEdit = l.id;
					TabLayers.layerNameHandle.text = l.name;
					zui_start_text_edit(TabLayers.layerNameHandle);
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

		if (SlotLayer.isGroup(l)) {
			zui_end_element();
		}
		else {
			if (SlotLayer.isMask(l)) {
				ui._y += center;
			}

			// comboBlending(ui, l);
			zui_end_element();

			if (SlotLayer.isMask(l)) {
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

		if (SlotLayer.isGroup(l) || SlotLayer.isMask(l)) {
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
			TabLayers.comboObject(ui, l);
			ui._y += center;

			zui_end_element();
		}

		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static comboObject = (ui: zui_t, l: SlotLayerRaw, label = false): zui_handle_t => {
		let ar = [tr("Shared")];
		let objectHandle = zui_nest(zui_handle("tablayers_2"), l.id);
		objectHandle.position = l.objectMask;
		l.objectMask = zui_combo(objectHandle, ar, tr("Object"), label, Align.Left);
		return objectHandle;
	}

	static layerToggleVisible = (l: SlotLayerRaw) => {
		l.visible = !l.visible;
		UIView2D.hwnd.redraws = 2;
		MakeMaterial.parseMeshMaterial();
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
		if (SlotLayer.isLayer(l) && SlotLayer.isMask(Project.layers[0]) && Project.layers[0].parent == l) return false;
		// The lowest toplevel layer is a group
		if (SlotLayer.isGroup(l) && SlotLayer.isInGroup(Project.layers[0]) && SlotLayer.getContainingGroup(Project.layers[0]) == l) return false;
		// Masks must be merged down to masks
		if (SlotLayer.isMask(l) && !SlotLayer.isMask(Project.layers[index - 1])) return false;
		return true;
	}

	static drawLayerContextMenu = (l: SlotLayerRaw, mini: bool) => {

	}

	static canDropNewLayer = (position: i32) => {
		if (position > 0 && position < Project.layers.length && SlotLayer.isMask(Project.layers[position - 1])) {
			// 1. The layer to insert is inserted in the middle
			// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
			return false;
		}
		return true;
	}
}
