
class TabLayers {

	static layerNameEdit = -1;
	static layerNameHandle = new Handle();
	static showContextMenu = false;

	static draw = (htab: Handle) => {
		let mini = Config.raw.layout[LayoutSidebarW] <= UIBase.sidebarMiniW;
		mini ? drawMini(htab) : drawFull(htab);
	}

	static drawMini = (htab: Handle) => {
		let ui = UIBase.inst.ui;
		ui.setHoveredTabName(tr("Layers"));

		let _ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Math.floor(UIBase.sidebarMiniW / 2 / ui.SCALE());

		ui.beginSticky();
		ui.separator(5);

		comboFilter();
		buttonNew("+");

		ui.endSticky();
		ui._y += 2;

		highlightOddLines();
		drawSlots(true);

		ui.t.ELEMENT_H = _ELEMENT_H;
	}

	static drawFull = (htab: Handle) => {
		let ui = UIBase.inst.ui;
		if (ui.tab(htab, tr("Layers"))) {
			ui.beginSticky();
			ui.row([1 / 4, 3 / 4]);

			buttonNew(tr("New"));
			comboFilter();

			ui.endSticky();
			ui._y += 2;

			highlightOddLines();
			drawSlots(false);
		}
	}

	static drawSlots = (mini: bool) => {
		for (let i = 0; i < Project.layers.length; ++i) {
			if (i >= Project.layers.length) break; // Layer was deleted
			let j = Project.layers.length - 1 - i;
			let l = Project.layers[j];
			drawLayerSlot(l, j, mini);
		}
	}

	static highlightOddLines = () => {
		let ui = UIBase.inst.ui;
		let step = ui.t.ELEMENT_H * 2;
		let fullH = ui._windowH - UIBase.inst.hwnds[0].scrollOffset;
		for (let i = 0; i < Math.floor(fullH / step); ++i) {
			if (i % 2 == 0) {
				ui.fill(0, i * step, (ui._w / ui.SCALE() - 2), step, ui.t.WINDOW_BG_COL - 0x00040404);
			}
		}
	}

	static buttonNew = (text: string) => {
		let ui = UIBase.inst.ui;
		if (ui.button(text)) {
			UIMenu.draw((ui: Zui) => {
				let l = Context.raw.layer;
				if (UIMenu.menuButton(ui, tr("Paint Layer"))) {
					Base.newLayer();
					History.newLayer();
				}
			}, 1);
		}
	}

	static comboFilter = () => {
		let ui = UIBase.inst.ui;
		let ar = [tr("All")];
		let filterHandle = Zui.handle("tablayers_0");
		filterHandle.position = Context.raw.layerFilter;
		Context.raw.layerFilter = ui.combo(filterHandle, ar, tr("Filter"), false, Left);
	}

	static remapLayerPointers = (nodes: TNode[], pointerMap: Map<i32, i32>) => {
		for (let n of nodes) {
			if (n.type == "LAYER" || n.type == "LAYER_MASK") {
				let i = n.buttons[0].default_value;
				if (pointerMap.exists(i)) {
					n.buttons[0].default_value = pointerMap.get(i);
				}
			}
		}
	}

	static initLayerMap = (): Map<SlotLayer, i32> => {
		let res: Map<SlotLayer, i32> = [];
		for (let i = 0; i < Project.layers.length; ++i) res.set(Project.layers[i], i);
		return res;
	}

	static fillLayerMap = (map: Map<SlotLayer, i32>): Map<i32, i32> => {
		let res: Map<i32, i32> = new Map();
		for (let l of map.keys()) res.set(map.get(l), Project.layers.indexOf(l) > -1 ? Project.layers.indexOf(l) : 9999);
		return res;
	}

	static setDragLayer = (layer: SlotLayer, offX: f32, offY: f32) => {
		Base.dragOffX = offX;
		Base.dragOffY = offY;
		Base.dragLayer = layer;
		Context.raw.dragDestination = Project.layers.indexOf(layer);
	}

	static drawLayerSlot = (l: SlotLayer, i: i32, mini: bool) => {
		let ui = UIBase.inst.ui;

		if (Context.raw.layerFilter > 0 &&
			l.getObjectMask() > 0 &&
			l.getObjectMask() != Context.raw.layerFilter) {
			return;
		}

		if (l.parent != null && !l.parent.show_panel) { // Group closed
			return;
		}
		if (l.parent != null && l.parent.parent != null && !l.parent.parent.show_panel) {
			return;
		}

		let step = ui.t.ELEMENT_H;
		let checkw = (ui._windowW / 100 * 8) / ui.SCALE();

		// Highlight drag destination
		let mouse = Input.getMouse();
		let absy = ui._windowY + ui._y;
		if (Base.isDragging && Base.dragLayer != null && Context.inLayers()) {
			if (mouse.y > absy + step && mouse.y < absy + step * 3) {
				let down = Project.layers.indexOf(Base.dragLayer) >= i;
				Context.raw.dragDestination = down ? i : i - 1;

				let ls = Project.layers;
				let dest = Context.raw.dragDestination;
				let toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				let nestedGroup = Base.dragLayer.isGroup() && toGroup;
				if (!nestedGroup) {
					if (Context.raw.layer.canMove(Context.raw.dragDestination)) {
						ui.fill(checkw, step * 2, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
					}
				}
			}
			else if (i == Project.layers.length - 1 && mouse.y < absy + step) {
				Context.raw.dragDestination = Project.layers.length - 1;
				if (Context.raw.layer.canMove(Context.raw.dragDestination)) {
					ui.fill(checkw, 0, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
				}
			}
		}
		if (Base.isDragging && (Base.dragMaterial != null || Base.dragSwatch != null) && Context.inLayers()) {
			if (mouse.y > absy + step && mouse.y < absy + step * 3) {
				Context.raw.dragDestination = i;
				if (canDropNewLayer(i))
					ui.fill(checkw, 2 * step, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
			}
			else if (i == Project.layers.length - 1 && mouse.y < absy + step) {
				Context.raw.dragDestination = Project.layers.length;
				if (canDropNewLayer(Project.layers.length))
					ui.fill(checkw, 0, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
			}
		}

		mini ? drawLayerSlotMini(l, i) : drawLayerSlotFull(l, i);

		drawLayerHighlight(l, mini);

		if (showContextMenu) {
			drawLayerContextMenu(l, mini);
		}
	}

	static drawLayerSlotMini = (l: SlotLayer, i: i32) => {
		let ui = UIBase.inst.ui;

		ui.row([1, 1]);
		let uix = ui._x;
		let uiy = ui._y;
		ui.endElement();
		ui.endElement();

		ui._y += ui.ELEMENT_H();
		ui._y -= ui.ELEMENT_OFFSET();
	}

	static drawLayerSlotFull = (l: SlotLayer, i: i32) => {
		let ui = UIBase.inst.ui;

		let step = ui.t.ELEMENT_H;

		let hasPanel = l.isGroup() || (l.isLayer() && l.getMasks(false) != null);
		if (hasPanel) {
			ui.row([8 / 100, 52 / 100, 30 / 100, 10 / 100]);
		}
		else {
			ui.row([8 / 100, 52 / 100, 30 / 100]);
		}

		// Draw eye icon
		let icons = Res.get("icons.k");
		let r = Res.tile18(icons, l.visible ? 0 : 1, 0);
		let center = (step / 2) * ui.SCALE();
		ui._x += 2;
		ui._y += 3;
		ui._y += center;
		let col = ui.t.ACCENT_SELECT_COL;
		let parentHidden = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
		if (parentHidden) col -= 0x99000000;

		if (ui.image(icons, col, null, r.x, r.y, r.w, r.h) == Released) {
			layerToggleVisible(l);
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		let uix = ui._x;
		let uiy = ui._y;

		// Draw layer name
		ui._y += center;
		if (layerNameEdit == l.id) {
			layerNameHandle.text = l.name;
			l.name = ui.textInput(layerNameHandle);
			if (ui.textSelectedHandle_ptr != layerNameHandle.ptr) layerNameEdit = -1;
		}
		else {
			if (ui.enabled && ui.inputEnabled && ui.comboSelectedHandle_ptr == null &&
				ui.inputX > ui._windowX + ui._x && ui.inputX < ui._windowX + ui._windowW &&
				ui.inputY > ui._windowY + ui._y - center && ui.inputY < ui._windowY + ui._y - center + (step * ui.SCALE()) * 2) {
				if (ui.inputStarted) {
					Context.setLayer(l);
					let mouse = Input.getMouse();
					setDragLayer(Context.raw.layer, -(mouse.x - uix - ui._windowX - 3), -(mouse.y - uiy - ui._windowY + 1));
				}
				else if (ui.inputReleased) {
					if (Time.time() - Context.raw.selectTime > 0.2) {
						Context.raw.selectTime = Time.time();
					}
				}
				else if (ui.inputReleasedR) {
					Context.setLayer(l);
					showContextMenu = true;
				}
			}

			let state = ui.text(l.name);
			if (state == State.Released) {
				let td = Time.time() - Context.raw.selectTime;
				if (td < 0.2 && td > 0.0) {
					layerNameEdit = l.id;
					layerNameHandle.text = l.name;
					ui.startTextEdit(layerNameHandle);
				}
			}

			// let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
			// 			  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			// if (inFocus && ui.isDeleteDown && canDelete(Context.raw.layer)) {
			// 	ui.isDeleteDown = false;
			// 	let _init() = () => {
			// 		deleteLayer(Context.raw.layer);
			// 	}
			// 	App.notifyOnInit(_init);
			// }
		}
		ui._y -= center;

		if (l.parent != null) {
			ui._x -= 10 * ui.SCALE();
			if (l.parent.parent != null) ui._x -= 10 * ui.SCALE();
		}

		if (l.isGroup()) {
			ui.endElement();
		}
		else {
			if (l.isMask()) {
				ui._y += center;
			}

			// comboBlending(ui, l);
			ui.endElement();

			if (l.isMask()) {
				ui._y -= center;
			}
		}

		if (hasPanel) {
			ui._y += center;
			let layerPanel = Zui.handle("tablayers_1").nest(l.id);
			layerPanel.selected = l.show_panel;
			l.show_panel = ui.panel(layerPanel, "", true, false, false);
			ui._y -= center;
		}

		if (l.isGroup() || l.isMask()) {
			ui._y -= ui.ELEMENT_OFFSET();
			ui.endElement();
		}
		else {
			ui._y -= ui.ELEMENT_OFFSET();

			ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
			ui.endElement();
			ui.endElement();
			ui.endElement();

			if (Config.raw.touch_ui) {
				ui._x += 12 * ui.SCALE();
			}

			ui._y -= center;
			comboObject(ui, l);
			ui._y += center;

			ui.endElement();
		}

		ui._y -= ui.ELEMENT_OFFSET();
	}

	static comboObject = (ui: Zui, l: SlotLayer, label = false): Handle => {
		let ar = [tr("Shared")];
		let objectHandle = Zui.handle("tablayers_2").nest(l.id);
		objectHandle.position = l.objectMask;
		l.objectMask = ui.combo(objectHandle, ar, tr("Object"), label, Left);
		return objectHandle;
	}

	static layerToggleVisible = (l: SlotLayer) => {
		l.visible = !l.visible;
		UIView2D.inst.hwnd.redraws = 2;
		MakeMaterial.parseMeshMaterial();
	}

	static drawLayerHighlight = (l: SlotLayer, mini: bool) => {
		let ui = UIBase.inst.ui;
		let step = ui.t.ELEMENT_H;

		// Separator line
		ui.fill(0, 0, (ui._w / ui.SCALE() - 2), 1 * ui.SCALE(), ui.t.SEPARATOR_COL);

		// Highlight selected
		if (Context.raw.layer == l) {
			if (mini) {
				ui.rect(1, -step * 2, ui._w / ui.SCALE() - 1, step * 2 + (mini ? -1 : 1), ui.t.HIGHLIGHT_COL, 3);
			}
			else {
				ui.rect(1, -step * 2 - 1, ui._w / ui.SCALE() - 2, step * 2 + (mini ? -2 : 1), ui.t.HIGHLIGHT_COL, 2);
			}
		}
	}

	static canMergeDown = (l: SlotLayer) : bool => {
		let index = Project.layers.indexOf(l);
		// Lowest layer
		if (index == 0) return false;
		// Lowest layer that has masks
		if (l.isLayer() && Project.layers[0].isMask() && Project.layers[0].parent == l) return false;
		// The lowest toplevel layer is a group
		if (l.isGroup() && Project.layers[0].isInGroup() && Project.layers[0].getContainingGroup() == l) return false;
		// Masks must be merged down to masks
		if (l.isMask() && !Project.layers[index - 1].isMask()) return false;
		return true;
	}

	static drawLayerContextMenu = (l: SlotLayer, mini: bool) => {

	}

	static canDropNewLayer = (position: i32) => {
		if (position > 0 && position < Project.layers.length && Project.layers[position - 1].isMask()) {
			// 1. The layer to insert is inserted in the middle
			// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
			return false;
		}
		return true;
	}
}
