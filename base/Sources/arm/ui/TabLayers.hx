package arm.ui;

#if (is_paint || is_sculpt)

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
	static var layerNameHandle = Id.handle();
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
		button2dView();
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
			ui.row([1 / 4, 1 / 4, 1 / 2]);

			buttonNew(tr("New"));
			button2dView();
			comboFilter();

			ui.endSticky();
			ui._y += 2;

			highlightOddLines();
			drawSlots(false);
		}
	}

	static function button2dView() {
		var ui = UIBase.inst.ui;
		if (ui.button(tr("2D View"))) {
			UIBase.inst.show2DView(View2DLayer);
		}
		else if (ui.isHovered) ui.tooltip(tr("Show 2D View") + ' (${Config.keymap.toggle_2d_view})');
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
				if (UIMenu.menuButton(ui, tr("Fill Layer"))) {
					App.createFillLayer(UVMap);
				}
				if (UIMenu.menuButton(ui, tr("Decal Layer"))) {
					App.createFillLayer(UVProject);
				}
				if (UIMenu.menuButton(ui, tr("Black Mask"))) {
					if (l.isMask()) Context.setLayer(l.parent);
					var l = Context.raw.layer;

					var m = App.newMask(false, l);
					function _next() {
						m.clear(0x00000000);
					}
					App.notifyOnNextFrame(_next);
					Context.raw.layerPreviewDirty = true;
					History.newBlackMask();
					App.updateFillLayers();
				}
				if (UIMenu.menuButton(ui, tr("White Mask"))) {
					if (l.isMask()) Context.setLayer(l.parent);
					var l = Context.raw.layer;

					var m = App.newMask(false, l);
					function _next() {
						m.clear(0xffffffff);
					}
					App.notifyOnNextFrame(_next);
					Context.raw.layerPreviewDirty = true;
					History.newWhiteMask();
					App.updateFillLayers();
				}
				if (UIMenu.menuButton(ui, tr("Fill Mask"))) {
					if (l.isMask()) Context.setLayer(l.parent);
					var l = Context.raw.layer;

					var m = App.newMask(false, l);
					function _init() {
						m.toFillLayer();
					}
					iron.App.notifyOnInit(_init);
					Context.raw.layerPreviewDirty = true;
					History.newFillMask();
					App.updateFillLayers();
				}
				ui.enabled = !Context.raw.layer.isGroup() && !Context.raw.layer.isInGroup();
				if (UIMenu.menuButton(ui, tr("Group"))) {
					if (l.isGroup() || l.isInGroup()) return;

					if (l.isLayerMask()) l = l.parent;

					var pointers = initLayerMap();
					var group = App.newGroup();
					Context.setLayer(l);
					Project.layers.remove(group);
					Project.layers.insert(Project.layers.indexOf(l) + 1, group);
					l.parent = group;
					for (m in Project.materials) remapLayerPointers(m.canvas.nodes, fillLayerMap(pointers));
					Context.setLayer(group);
					History.newGroup();
				}
				ui.enabled = true;
			}, 7);
		}
	}

	static function comboFilter() {
		var ui = UIBase.inst.ui;
		var ar = [tr("All")];
		for (p in Project.paintObjects) ar.push(p.name);
		var atlases = Project.getUsedAtlases();
		if (atlases != null) for (a in atlases) ar.push(a);
		var filterHandle = Id.handle();
		filterHandle.position = Context.raw.layerFilter;
		Context.raw.layerFilter = ui.combo(filterHandle, ar, tr("Filter"), false, Left);
		if (filterHandle.changed) {
			for (p in Project.paintObjects) {
				p.visible = Context.raw.layerFilter == 0 || p.name == ar[Context.raw.layerFilter] || Project.isAtlasObject(p);
			}
			if (Context.raw.layerFilter == 0 && Context.raw.mergedObjectIsAtlas) { // All
				MeshUtil.mergeMesh();
			}
			else if (Context.raw.layerFilter > Project.paintObjects.length) { // Atlas
				var visibles: Array<MeshObject> = [];
				for (p in Project.paintObjects) if (p.visible) visibles.push(p);
				MeshUtil.mergeMesh(visibles);
			}
			App.setObjectMask();
			UVUtil.uvmapCached = false;
			Context.raw.ddirty = 2;
			#if (kha_direct3d12 || kha_vulkan || kha_metal)
			arm.render.RenderPathRaytrace.ready = false;
			#end
		}
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
		var state = drawLayerIcon(l, i, uix, uiy, true);
		handleLayerIconState(l, i, state, uix, uiy);
		@:privateAccess ui.endElement();

		ui._y += ui.ELEMENT_H();
		ui._y -= ui.ELEMENT_OFFSET();
	}

	static function drawLayerSlotFull(l: LayerSlot, i: Int) {
		var ui = UIBase.inst.ui;

		var step = ui.t.ELEMENT_H;

		var hasPanel = l.isGroup() || (l.isLayer() && l.getMasks(false) != null);
		if (hasPanel) {
			ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
		}
		else {
			ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100]);
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

		#if kha_opengl
		ui.imageInvertY = l.fill_layer != null;
		#end

		var uix = ui._x;
		var uiy = ui._y;
		ui._x += 2;
		ui._y += 3;
		if (l.parent != null) {
			ui._x += 10 * ui.SCALE();
			if (l.parent.parent != null) ui._x += 10 * ui.SCALE();
		}

		var state = drawLayerIcon(l, i, uix, uiy, false);

		ui._x -= 2;
		ui._y -= 3;

		if (Config.raw.touch_ui) {
			ui._x += 12 * ui.SCALE();
		}

		#if kha_opengl
		ui.imageInvertY = false;
		#end

		handleLayerIconState(l, i, state, uix, uiy);

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

			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && canDelete(Context.raw.layer)) {
				ui.isDeleteDown = false;
				function _init() {
					deleteLayer(Context.raw.layer);
				}
				iron.App.notifyOnInit(_init);
			}
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

			comboBlending(ui, l);

			if (l.isMask()) {
				ui._y -= center;
			}
		}

		if (hasPanel) {
			ui._y += center;
			var layerPanel = Id.handle().nest(l.id);
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

			comboObject(ui, l);
			@:privateAccess ui.endElement();
		}

		ui._y -= ui.ELEMENT_OFFSET();
	}

	static function comboObject(ui: Zui, l: LayerSlot, label = false): Handle {
		var ar = [tr("Shared")];
		for (p in Project.paintObjects) ar.push(p.name);
		var atlases = Project.getUsedAtlases();
		if (atlases != null) for (a in atlases) ar.push(a);
		var objectHandle = Id.handle().nest(l.id);
		objectHandle.position = l.objectMask;
		l.objectMask = ui.combo(objectHandle, ar, tr("Object"), label, Left);
		if (objectHandle.changed) {
			Context.setLayer(l);
			MakeMaterial.parseMeshMaterial();
			if (l.fill_layer != null) { // Fill layer
				function _init() {
					Context.raw.material = l.fill_layer;
					l.clear();
					App.updateFillLayers();
				}
				iron.App.notifyOnInit(_init);
			}
			else {
				App.setObjectMask();
			}
		}
		return objectHandle;
	}

	static function comboBlending(ui: Zui, l: LayerSlot, label = false): Handle {
		var blendingHandle = Id.handle().nest(l.id);
		blendingHandle.position = l.blending;
		ui.combo(blendingHandle, [
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
		], tr("Blending"), label);
		if (blendingHandle.changed) {
			Context.setLayer(l);
			History.layerBlending();
			l.blending = blendingHandle.position;
			MakeMaterial.parseMeshMaterial();
		}
		return blendingHandle;
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

	static function handleLayerIconState(l: LayerSlot, i: Int, state: State, uix: Float, uiy: Float) {
		var ui = UIBase.inst.ui;

		#if is_paint
		var texpaint_preview = l.texpaint_preview;
		#end
		#if is_sculpt
		var texpaint_preview = l.texpaint;
		#end

		showContextMenu = false;

		// Layer preview tooltip
		if (ui.isHovered && texpaint_preview != null) {
			if (l.isMask()) {
				makeMaskPreviewRgba32(l);
				ui.tooltipImage(Context.raw.maskPreviewRgba32);
			}
			else {
				ui.tooltipImage(texpaint_preview);
			}
			if (i < 9) ui.tooltip(l.name + " - (" + Config.keymap.select_layer + " " + (i + 1) + ")");
			else ui.tooltip(l.name);
		}

		// Show context menu
		if (ui.isHovered && ui.inputReleasedR) {
			Context.setLayer(l);
			showContextMenu = true;
		}

		if (state == State.Started) {
			Context.setLayer(l);
			var mouse = Input.getMouse();
			setDragLayer(Context.raw.layer, -(mouse.x - uix - ui._windowX - 3), -(mouse.y - uiy - ui._windowY + 1));
		}
		else if (state == State.Released) {
			if (Time.time() - Context.raw.selectTime < 0.2) {
				UIBase.inst.show2DView(View2DLayer);
			}
			if (Time.time() - Context.raw.selectTime > 0.2) {
				Context.raw.selectTime = Time.time();
			}
			if (l.fill_layer != null) Context.setMaterial(l.fill_layer);
		}
	}

	static function drawLayerIcon(l: LayerSlot, i: Int, uix: Float, uiy: Float, mini: Bool) {
		var ui = UIBase.inst.ui;
		var icons = Res.get("icons.k");
		var iconH = (ui.ELEMENT_H() - (mini ? 2 : 3)) * 2;

		if (mini && ui.SCALE() > 1) {
			ui._x -= 1 * ui.SCALE();
		}

		if (l.parent != null) {
			ui._x += (iconH - iconH * 0.9) / 2;
			iconH *= 0.9;
			if (l.parent.parent != null) {
				ui._x += (iconH - iconH * 0.9) / 2;
				iconH *= 0.9;
			}
		}

		if (!l.isGroup()) {
			#if is_paint
			var texpaint_preview = l.texpaint_preview;
			#end
			#if is_sculpt
			var texpaint_preview = l.texpaint;
			#end

			var icon = l.fill_layer == null ? texpaint_preview : l.fill_layer.imageIcon;
			if (l.fill_layer == null) {
				// Checker
				var r = Res.tile50(icons, 4, 1);
				var _x = ui._x;
				var _y = ui._y;
				var _w = ui._w;
				ui.image(icons, 0xffffffff, iconH, r.x, r.y, r.w, r.h);
				ui.curRatio--;
				ui._x = _x;
				ui._y = _y;
				ui._w = _w;
			}
			if (l.fill_layer == null && l.isMask()) {
				ui.g.pipeline = UIView2D.pipe;
				#if kha_opengl
				ui.currentWindow.texture.g4.setPipeline(UIView2D.pipe);
				#end
				ui.currentWindow.texture.g4.setInt(UIView2D.channelLocation, 1);
			}

			var state = ui.image(icon, 0xffffffff, iconH);

			if (l.fill_layer == null && l.isMask()) {
				ui.g.pipeline = null;
			}

			// Draw layer numbers when selecting a layer via keyboard shortcut
			var isTyping = ui.isTyping || UIView2D.inst.ui.isTyping || UINodes.inst.ui.isTyping;
			if (!isTyping) {
				if (i < 9 && Operator.shortcut(Config.keymap.select_layer, ShortcutDown)) {
					var number = Std.string(i + 1) ;
					var width = ui.ops.font.width(ui.fontSize, number) + 10;
					var height = ui.ops.font.height(ui.fontSize);
					ui.g.color = ui.t.TEXT_COL;
					ui.g.fillRect(uix, uiy, width, height);
					ui.g.color = ui.t.ACCENT_COL;
					ui.g.drawString(number, uix + 5, uiy);
				}
			}

			return state;
		}
		else { // Group
			var folderClosed = Res.tile50(icons, 2, 1);
			var folderOpen = Res.tile50(icons, 8, 1);
			var folder = l.show_panel ? folderOpen : folderClosed;
			return ui.image(icons, ui.t.LABEL_COL - 0x00202020, iconH, folder.x, folder.y, folder.w, folder.h);
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
		var add = 0;

		if (l.fill_layer == null) add += 1; // Clear
		if (l.fill_layer != null && !l.isMask()) add += 3;
		if (l.fill_layer != null && l.isMask()) add += 2;
		if (l.isMask()) add += 2;
		if (mini) {
			add += 1;
			if (!l.isGroup()) add += 1;
			if (l.isLayer()) add += 1;
		}
		var menuElements = l.isGroup() ? 7 : (19 + add);

		UIMenu.draw(function(ui: Zui) {

			if (mini) {
				var visibleHandle = Id.handle();
				visibleHandle.selected = l.visible;
				UIMenu.menuFill(ui);
				ui.check(visibleHandle, tr("Visible"));
				if (visibleHandle.changed) {
					layerToggleVisible(l);
					UIMenu.keepOpen = true;
				}

				if (!l.isGroup()) {
					UIMenu.menuFill(ui);
					if (comboBlending(ui, l, true).changed) {
						UIMenu.keepOpen = true;
					}
				}
				if (l.isLayer()) {
					UIMenu.menuFill(ui);
					if (comboObject(ui, l, true).changed) {
						UIMenu.keepOpen = true;
					}
				}
			}

			if (UIMenu.menuButton(ui, tr("Export"))) {
				if (l.isMask()) {
					UIFiles.show("png", true, false, function(path: String) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						if (!f.endsWith(".png")) f += ".png";
						Krom.writePng(path + Path.sep + f, l.texpaint.getPixels().getData(), l.texpaint.width, l.texpaint.height, 3); // RRR1
					});
				}
				else {
					#if is_paint
					Context.raw.layersExport = ExportSelected;
					BoxExport.showTextures();
					#end
				}
			}

			if (!l.isGroup()) {
				var toFillString = l.isLayer() ? tr("To Fill Layer") : tr("To Fill Mask");
				var toPaintString = l.isLayer() ? tr("To Paint Layer") : tr("To Paint Mask");

				if (l.fill_layer == null && UIMenu.menuButton(ui, toFillString)) {
					function _init() {
						l.isLayer() ? History.toFillLayer() : History.toFillMask();
						l.toFillLayer();
					}
					iron.App.notifyOnInit(_init);
				}
				if (l.fill_layer != null && UIMenu.menuButton(ui, toPaintString)) {
					function _init() {
						l.isLayer() ? History.toPaintLayer() : History.toPaintMask();
						l.toPaintLayer();
					}
					iron.App.notifyOnInit(_init);
				}
			}

			ui.enabled = canDelete(l);
			if (UIMenu.menuButton(ui, tr("Delete"), "delete")) {
				function _init() {
					deleteLayer(Context.raw.layer);
				}
				iron.App.notifyOnInit(_init);
			}
			ui.enabled = true;

			if (l.fill_layer == null && UIMenu.menuButton(ui, tr("Clear"))) {
				Context.setLayer(l);
				function _init() {
					if (!l.isGroup()) {
						History.clearLayer();
						l.clear();
					}
					else {
						for (c in l.getChildren()) {
							Context.raw.layer = c;
							History.clearLayer();
							c.clear();
						}
						Context.raw.layersPreviewDirty = true;
						Context.raw.layer = l;
					}
				}
				iron.App.notifyOnInit(_init);
			}
			if (l.isMask() && l.fill_layer == null && UIMenu.menuButton(ui, tr("Invert"))) {
				function _init() {
					Context.setLayer(l);
					History.invertMask();
					l.invertMask();
				}
				iron.App.notifyOnInit(_init);
			}
			if (l.isMask() && UIMenu.menuButton(ui, tr("Apply"))) {
				function _init() {
					Context.raw.layer = l;
					History.applyMask();
					l.applyMask();
					Context.setLayer(l.parent);
					MakeMaterial.parseMeshMaterial();
					Context.raw.layersPreviewDirty = true;
				}
				iron.App.notifyOnInit(_init);
			}
			if (l.isGroup() && UIMenu.menuButton(ui, tr("Merge Group"))) {
				function _init() {
					App.mergeGroup(l);
				}
				iron.App.notifyOnInit(_init);
			}
			ui.enabled = canMergeDown(l);
			if (UIMenu.menuButton(ui, tr("Merge Down"))) {
				function _init() {
					Context.setLayer(l);
					History.mergeLayers();
					App.mergeDown();
					if (Context.raw.layer.fill_layer != null) Context.raw.layer.toPaintLayer();
				}
				iron.App.notifyOnInit(_init);
			}
			ui.enabled = true;
			if (UIMenu.menuButton(ui, tr("Duplicate"))) {
				function _init() {
					Context.setLayer(l);
					History.duplicateLayer();
					App.duplicateLayer(l);
				}
				iron.App.notifyOnInit(_init);
			}

			UIMenu.menuFill(ui);
			UIMenu.menuAlign(ui);
			var layerOpacHandle = Id.handle().nest(l.id);
			layerOpacHandle.value = l.maskOpacity;
			ui.slider(layerOpacHandle, tr("Opacity"), 0.0, 1.0, true);
			if (layerOpacHandle.changed) {
				if (ui.inputStarted) History.layerOpacity();
				l.maskOpacity = layerOpacHandle.value;
				MakeMaterial.parseMeshMaterial();
				UIMenu.keepOpen = true;
			}

			if (!l.isGroup()) {
				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				var resHandleChangedLast = App.resHandle.changed;
				#if (krom_android || krom_ios)
				var ar = ["128", "256", "512", "1K", "2K", "4K"];
				#else
				var ar = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
				#end
				var _y = ui._y;
				App.resHandle.value = App.resHandle.position;
				App.resHandle.position = Std.int(ui.slider(App.resHandle, ar[App.resHandle.position], 0, ar.length - 1, false, 1, false, Left, false));
				if (App.resHandle.changed) {
					UIMenu.keepOpen = true;
				}
				if (resHandleChangedLast && !App.resHandle.changed) {
					App.onLayersResized();
				}
				ui._y = _y;
				ui.drawString(ui.g, tr("Res"), null, 0, Right);
				ui.endElement();

				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				#if (krom_android || krom_ios)
				zui.Ext.inlineRadio(ui, App.bitsHandle, ["8bit"]);
				#else
				zui.Ext.inlineRadio(ui, App.bitsHandle, ["8bit", "16bit", "32bit"]);
				#end
				if (App.bitsHandle.changed) {
					iron.App.notifyOnInit(App.setLayerBits);
					UIMenu.keepOpen = true;
				}
			}

			if (l.fill_layer != null) {
				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				var scaleHandle = Id.handle().nest(l.id);
				scaleHandle.value = l.scale;
				l.scale = ui.slider(scaleHandle, tr("UV Scale"), 0.0, 5.0, true);
				if (scaleHandle.changed) {
					Context.setMaterial(l.fill_layer);
					Context.setLayer(l);
					function _init() {
						App.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
					UIMenu.keepOpen = true;
				}

				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				var angleHandle = Id.handle().nest(l.id);
				angleHandle.value = l.angle;
				l.angle = ui.slider(angleHandle, tr("Angle"), 0.0, 360, true, 1);
				if (angleHandle.changed) {
					Context.setMaterial(l.fill_layer);
					Context.setLayer(l);
					MakeMaterial.parsePaintMaterial();
					function _init() {
						App.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
					UIMenu.keepOpen = true;
				}

				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				var uvTypeHandle = Id.handle().nest(l.id);
				uvTypeHandle.position = l.uvType;
				l.uvType = zui.Ext.inlineRadio(ui, uvTypeHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], Left);
				if (uvTypeHandle.changed) {
					Context.setMaterial(l.fill_layer);
					Context.setLayer(l);
					MakeMaterial.parsePaintMaterial();
					function _init() {
						App.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
					UIMenu.keepOpen = true;
				}
			}

			if (!l.isGroup()) {
				var baseHandle = Id.handle().nest(l.id);
				var opacHandle = Id.handle().nest(l.id);
				var norHandle = Id.handle().nest(l.id);
				var norBlendHandle = Id.handle().nest(l.id);
				var occHandle = Id.handle().nest(l.id);
				var roughHandle = Id.handle().nest(l.id);
				var metHandle = Id.handle().nest(l.id);
				var heightHandle = Id.handle().nest(l.id);
				var heightBlendHandle = Id.handle().nest(l.id);
				var emisHandle = Id.handle().nest(l.id);
				var subsHandle = Id.handle().nest(l.id);
				baseHandle.selected = l.paintBase;
				opacHandle.selected = l.paintOpac;
				norHandle.selected = l.paintNor;
				norBlendHandle.selected = l.paintNorBlend;
				occHandle.selected = l.paintOcc;
				roughHandle.selected = l.paintRough;
				metHandle.selected = l.paintMet;
				heightHandle.selected = l.paintHeight;
				heightBlendHandle.selected = l.paintHeightBlend;
				emisHandle.selected = l.paintEmis;
				subsHandle.selected = l.paintSubs;
				UIMenu.menuFill(ui);
				l.paintBase = ui.check(baseHandle, tr("Base Color"));
				UIMenu.menuFill(ui);
				l.paintOpac = ui.check(opacHandle, tr("Opacity"));
				UIMenu.menuFill(ui);
				l.paintNor = ui.check(norHandle, tr("Normal"));
				UIMenu.menuFill(ui);
				l.paintNorBlend = ui.check(norBlendHandle, tr("Normal Blending"));
				UIMenu.menuFill(ui);
				l.paintOcc = ui.check(occHandle, tr("Occlusion"));
				UIMenu.menuFill(ui);
				l.paintRough = ui.check(roughHandle, tr("Roughness"));
				UIMenu.menuFill(ui);
				l.paintMet = ui.check(metHandle, tr("Metallic"));
				UIMenu.menuFill(ui);
				l.paintHeight = ui.check(heightHandle, tr("Height"));
				UIMenu.menuFill(ui);
				l.paintHeightBlend = ui.check(heightBlendHandle, tr("Height Blending"));
				UIMenu.menuFill(ui);
				l.paintEmis = ui.check(emisHandle, tr("Emission"));
				UIMenu.menuFill(ui);
				l.paintSubs = ui.check(subsHandle, tr("Subsurface"));
				if (baseHandle.changed ||
					opacHandle.changed ||
					norHandle.changed ||
					norBlendHandle.changed ||
					occHandle.changed ||
					roughHandle.changed ||
					metHandle.changed ||
					heightHandle.changed ||
					heightBlendHandle.changed ||
					emisHandle.changed ||
					subsHandle.changed) {
					MakeMaterial.parseMeshMaterial();
					UIMenu.keepOpen = true;
				}
			}
		}, menuElements);
	}

	public static function makeMaskPreviewRgba32(l: LayerSlot) {
		#if is_paint
		if (Context.raw.maskPreviewRgba32 == null) {
			Context.raw.maskPreviewRgba32 = kha.Image.createRenderTarget(RenderUtil.layerPreviewSize, RenderUtil.layerPreviewSize);
		}
		// Convert from R8 to RGBA32 for tooltip display
		if (Context.raw.maskPreviewLast != l) {
			Context.raw.maskPreviewLast = l;
			iron.App.notifyOnInit(function() {
				Context.raw.maskPreviewRgba32.g2.begin();
				Context.raw.maskPreviewRgba32.g2.pipeline = UIView2D.pipe;
				Context.raw.maskPreviewRgba32.g4.setInt(UIView2D.channelLocation, 1);
				Context.raw.maskPreviewRgba32.g2.drawImage(l.texpaint_preview, 0, 0);
				Context.raw.maskPreviewRgba32.g2.end();
				Context.raw.maskPreviewRgba32.g2.pipeline = null;
			});
		}
		#end
	}

	static function deleteLayer(l: LayerSlot) {
		var pointers = initLayerMap();
		
		if (l.isLayer() && l.hasMasks(false)) {
			for (m in l.getMasks(false)) {
				Context.raw.layer = m;
				History.deleteLayer();
				m.delete();
			}
		}
		if (l.isGroup()) {
			for (c in l.getChildren()) {
				if (c.hasMasks(false)) {
					for (m in c.getMasks(false)) {
						Context.raw.layer = m;
						History.deleteLayer();
						m.delete();
					}
				}
				Context.raw.layer = c;
				History.deleteLayer();
				c.delete();
			}
			if (l.hasMasks()) {
				for (m in l.getMasks()) {
					Context.raw.layer = m;
					History.deleteLayer();
					m.delete();
				}
			}
		}

		Context.raw.layer = l;
		History.deleteLayer();
		l.delete();

		if (l.isMask()) {
			Context.raw.layer = l.parent;
			App.updateFillLayers();
		}

		// Remove empty group
		if (l.isInGroup() && l.getContainingGroup().getChildren() == null) {
			var g = l.getContainingGroup();
			// Maybe some group masks are left
			if (g.hasMasks()) {
				for (m in g.getMasks()) {
					Context.raw.layer = m;
					History.deleteLayer();
					m.delete();
				}
			}
			Context.raw.layer = l.parent;
			History.deleteLayer();
			l.parent.delete();
		}
		Context.raw.ddirty = 2;
		for (m in Project.materials) remapLayerPointers(m.canvas.nodes, fillLayerMap(pointers));
	}

	static function canDelete(l: LayerSlot) {
		var numLayers = 0;

		if (l.isMask()) return true;
		
		for (slot in Project.layers) {
			if (slot.isLayer()) ++numLayers;
		}

		// All layers are in one group
		if (l.isGroup() && l.getChildren().length == numLayers) return false;

		// Do not delete last layer
		return numLayers > 1;
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

#end
