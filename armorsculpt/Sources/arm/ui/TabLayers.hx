package arm.ui;

import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.system.Time;
import iron.system.Input;
import iron.object.MeshObject;
import arm.data.LayerSlot;
import arm.shader.MakeMaterial;
import arm.util.UVUtil;
import arm.util.MeshUtil;
import arm.util.RenderUtil;
import arm.sys.Path;

@:access(zui.Zui)
class TabLayers {

	static var layerNameEdit = -1;
	static var layerNameHandle = new Handle();
	static var showContextMenu = false;

	public static function draw(htab: Handle) {
		var mini = Config.raw.layout[LayoutSidebarW] <= UIBase.sidebarMiniW;
		mini ? drawMini(htab) : drawFull(htab);
	}

	static function drawMini(htab: Handle) {
		var ui = UIBase.inst.ui;
		@:privateAccess ui.setHoveredTabName(tr("Layers"));

		var _ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Std.int(UIBase.sidebarMiniW / 2 / ui.SCALE());

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

	static function drawFull(htab: Handle) {
		var ui = UIBase.inst.ui;
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

	static function drawSlots(mini: Bool) {
		for (i in 0...Project.layers.length) {
			if (i >= Project.layers.length) break; // Layer was deleted
			var j = Project.layers.length - 1 - i;
			var l = Project.layers[j];
			drawLayerSlot(l, j, mini);
		}
	}

	static function highlightOddLines() {
		var ui = UIBase.inst.ui;
		var step = ui.t.ELEMENT_H * 2;
		var fullH = ui._windowH - UIBase.inst.hwnds[0].scrollOffset;
		for (i in 0...Std.int(fullH / step)) {
			if (i % 2 == 0) {
				ui.fill(0, i * step, (ui._w / ui.SCALE() - 2), step, ui.t.WINDOW_BG_COL - 0x00040404);
			}
		}
	}

	static function buttonNew(text: String) {
		var ui = UIBase.inst.ui;
		if (ui.button(text)) {
			UIMenu.draw(function(ui: Zui) {
				var l = Context.raw.layer;
				if (UIMenu.menuButton(ui, tr("Paint Layer"))) {
					App.newLayer();
					History.newLayer();
				}
			}, 1);
		}
	}

	static function comboFilter() {
		var ui = UIBase.inst.ui;
		var ar = [tr("All")];
		var filterHandle = Id.handle("tablayers_0");
		filterHandle.position = Context.raw.layerFilter;
		Context.raw.layerFilter = ui.combo(filterHandle, ar, tr("Filter"), false, Left);
	}

	public static function remapLayerPointers(nodes: Array<TNode>, pointerMap: Map<Int, Int>) {
		for (n in nodes) {
			if (n.type == "LAYER" || n.type == "LAYER_MASK") {
				var i = n.buttons[0].default_value;
				if (pointerMap.exists(i)) {
					n.buttons[0].default_value = pointerMap.get(i);
				}
			}
		}
	}

	public static function initLayerMap(): Map<LayerSlot, Int> {
		var res: Map<LayerSlot, Int> = [];
		for (i in 0...Project.layers.length) res.set(Project.layers[i], i);
		return res;
	}

	public static function fillLayerMap(map: Map<LayerSlot, Int>): Map<Int, Int> {
		var res: Map<Int, Int> = [];
		for (l in map.keys()) res.set(map.get(l), Project.layers.indexOf(l) > -1 ? Project.layers.indexOf(l) : 9999);
		return res;
	}

	static function setDragLayer(layer: LayerSlot, offX: Float, offY: Float) {
		App.dragOffX = offX;
		App.dragOffY = offY;
		App.dragLayer = layer;
		Context.raw.dragDestination = Project.layers.indexOf(layer);
	}

	static function drawLayerSlot(l: LayerSlot, i: Int, mini: Bool) {
		var ui = UIBase.inst.ui;

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

		var step = ui.t.ELEMENT_H;
		var checkw = (ui._windowW / 100 * 8) / ui.SCALE();

		// Highlight drag destination
		var mouse = Input.getMouse();
		var absy = ui._windowY + ui._y;
		if (App.isDragging && App.dragLayer != null && Context.inLayers()) {
			if (mouse.y > absy + step && mouse.y < absy + step * 3) {
				var down = Project.layers.indexOf(App.dragLayer) >= i;
				Context.raw.dragDestination = down ? i : i - 1;

				var ls = Project.layers;
				var dest = Context.raw.dragDestination;
				var toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				var nestedGroup = App.dragLayer.isGroup() && toGroup;
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
		if (App.isDragging && (App.dragMaterial != null || App.dragSwatch != null) && Context.inLayers()) {
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

	static function drawLayerSlotMini(l: LayerSlot, i: Int) {
		var ui = UIBase.inst.ui;

		ui.row([1, 1]);
		var uix = ui._x;
		var uiy = ui._y;
		@:privateAccess ui.endElement();
		@:privateAccess ui.endElement();

		ui._y += ui.ELEMENT_H();
		ui._y -= ui.ELEMENT_OFFSET();
	}

	static function drawLayerSlotFull(l: LayerSlot, i: Int) {
		var ui = UIBase.inst.ui;

		var step = ui.t.ELEMENT_H;

		var hasPanel = l.isGroup() || (l.isLayer() && l.getMasks(false) != null);
		if (hasPanel) {
			ui.row([8 / 100, 52 / 100, 30 / 100, 10 / 100]);
		}
		else {
			ui.row([8 / 100, 52 / 100, 30 / 100]);
		}

		// Draw eye icon
		var icons = Res.get("icons.k");
		var r = Res.tile18(icons, l.visible ? 0 : 1, 0);
		var center = (step / 2) * ui.SCALE();
		ui._x += 2;
		ui._y += 3;
		ui._y += center;
		var col = ui.t.ACCENT_SELECT_COL;
		var parentHidden = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
		if (parentHidden) col -= 0x99000000;

		if (ui.image(icons, col, null, r.x, r.y, r.w, r.h) == Released) {
			layerToggleVisible(l);
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		var uix = ui._x;
		var uiy = ui._y;

		// Draw layer name
		ui._y += center;
		if (layerNameEdit == l.id) {
			layerNameHandle.text = l.name;
			l.name = ui.textInput(layerNameHandle);
			if (ui.textSelectedHandle != layerNameHandle) layerNameEdit = -1;
		}
		else {
			if (ui.enabled && ui.inputEnabled && ui.comboSelectedHandle == null &&
				ui.inputX > ui._windowX + ui._x && ui.inputX < ui._windowX + ui._windowW &&
				ui.inputY > ui._windowY + ui._y - center && ui.inputY < ui._windowY + ui._y - center + (step * ui.SCALE()) * 2) {
				if (ui.inputStarted) {
					Context.setLayer(l);
					var mouse = Input.getMouse();
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

			var state = ui.text(l.name);
			if (state == State.Released) {
				var td = Time.time() - Context.raw.selectTime;
				if (td < 0.2 && td > 0.0) {
					layerNameEdit = l.id;
					layerNameHandle.text = l.name;
					ui.startTextEdit(layerNameHandle);
				}
			}

			// var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
			// 			  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			// if (inFocus && ui.isDeleteDown && canDelete(Context.raw.layer)) {
			// 	ui.isDeleteDown = false;
			// 	function _init() {
			// 		deleteLayer(Context.raw.layer);
			// 	}
			// 	iron.App.notifyOnInit(_init);
			// }
		}
		ui._y -= center;

		if (l.parent != null) {
			ui._x -= 10 * ui.SCALE();
			if (l.parent.parent != null) ui._x -= 10 * ui.SCALE();
		}

		if (l.isGroup()) {
			@:privateAccess ui.endElement();
		}
		else {
			if (l.isMask()) {
				ui._y += center;
			}

			// comboBlending(ui, l);
			@:privateAccess ui.endElement();

			if (l.isMask()) {
				ui._y -= center;
			}
		}

		if (hasPanel) {
			ui._y += center;
			var layerPanel = Id.handle("tablayers_1").nest(l.id);
			layerPanel.selected = l.show_panel;
			l.show_panel = ui.panel(layerPanel, "", true, false, false);
			ui._y -= center;
		}

		if (l.isGroup() || l.isMask()) {
			ui._y -= ui.ELEMENT_OFFSET();
			@:privateAccess ui.endElement();
		}
		else {
			ui._y -= ui.ELEMENT_OFFSET();

			ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
			@:privateAccess ui.endElement();
			@:privateAccess ui.endElement();
			@:privateAccess ui.endElement();

			if (Config.raw.touch_ui) {
				ui._x += 12 * ui.SCALE();
			}

			ui._y -= center;
			comboObject(ui, l);
			ui._y += center;

			@:privateAccess ui.endElement();
		}

		ui._y -= ui.ELEMENT_OFFSET();
	}

	static function comboObject(ui: Zui, l: LayerSlot, label = false): Handle {
		var ar = [tr("Shared")];
		var objectHandle = Id.handle("tablayers_2").nest(l.id);
		objectHandle.position = l.objectMask;
		l.objectMask = ui.combo(objectHandle, ar, tr("Object"), label, Left);
		return objectHandle;
	}

	static function layerToggleVisible(l: LayerSlot) {
		l.visible = !l.visible;
		UIView2D.inst.hwnd.redraws = 2;
		MakeMaterial.parseMeshMaterial();
	}

	static function drawLayerHighlight(l: LayerSlot, mini: Bool) {
		var ui = UIBase.inst.ui;
		var step = ui.t.ELEMENT_H;

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

	static function canMergeDown(l: LayerSlot) : Bool {
		var index = Project.layers.indexOf(l);
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

	static function drawLayerContextMenu(l: LayerSlot, mini: Bool) {

	}

	public static function canDropNewLayer(position: Int) {
		if (position > 0 && position < Project.layers.length && Project.layers[position - 1].isMask()) {
			// 1. The layer to insert is inserted in the middle
			// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
			return false;
		}
		return true;
	}
}
