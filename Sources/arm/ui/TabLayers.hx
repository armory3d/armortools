package arm.ui;

import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.system.Time;
import iron.system.Input;
import iron.object.MeshObject;
import arm.data.LayerSlot;
import arm.node.MakeMaterial;
import arm.util.UVUtil;
import arm.util.MeshUtil;
import arm.util.RenderUtil;
import arm.sys.Path;
import arm.Enums;

@:access(zui.Zui)
class TabLayers {

	static var layerNameEdit = -1;
	static var layerNameHandle = Id.handle();

	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab0, tr("Layers"))) {

			ui.beginSticky();

			ui.row([1 / 4, 1 / 4, 1 / 2]);
			if (ui.button(tr("New"))) {
				UIMenu.draw(function(ui: Zui) {
					var l = Context.layer;
					ui.text("New", Right, ui.t.HIGHLIGHT_COL);
					if (ui.button(tr("Paint Layer"), Left)) {
						Layers.newLayer();
						History.newLayer();
					}
					if (ui.button(tr("Fill Layer"), Left)) {
						Layers.createFillLayer(UVMap);
					}
					if (ui.button(tr("Decal Layer"), Left)) {
						Layers.createFillLayer(UVProject);
					}
					if (ui.button(tr("Black Mask"), Left)) {
						if (l.isMask()) Context.setLayer(l.parent);
						var l = Context.layer;

						var m = Layers.newMask(false, l);
						function _next() {
							m.clear(0x00000000);
						}
						App.notifyOnNextFrame(_next);
						Context.layerPreviewDirty = true;
						History.newBlackMask();
					}
					if (ui.button(tr("White Mask"), Left)) {
						if (l.isMask()) Context.setLayer(l.parent);
						var l = Context.layer;

						var m = Layers.newMask(false, l);
						function _next() {
							m.clear(0xffffffff);
						}
						App.notifyOnNextFrame(_next);
						Context.layerPreviewDirty = true;
						History.newWhiteMask();
					}
					if (ui.button(tr("Fill Mask"), Left)) {
						if (l.isMask()) Context.setLayer(l.parent);
						var l = Context.layer;

						var m = Layers.newMask(false, l);
						function _init() {
							m.toFillLayer();
						}
						iron.App.notifyOnInit(_init);
						Context.layerPreviewDirty = true;
						History.newFillMask();
					}
					ui.enabled = !Context.layer.isGroup() && !Context.layer.isInGroup();
					if (ui.button(tr("Group"), Left)) {
						if (l.isGroup() || l.isInGroup()) return;

						if (l.isLayerMask()) l = l.parent;

						var pointers = initLayerMap();
						var group = Layers.newGroup();
						Context.setLayer(l);
						Project.layers.remove(group);
						Project.layers.insert(Project.layers.indexOf(l) + 1, group);
						l.parent = group;
						for (m in Project.materials) remapLayerPointers(m.canvas.nodes, fillLayerMap(pointers));
						Context.setLayer(group);
						History.newGroup();
					}
					ui.enabled = true;
				}, 8);
			}
			if (ui.button(tr("2D View"))) UISidebar.inst.show2DView(View2DLayer);
			else if (ui.isHovered) ui.tooltip(tr("Show 2D View") + ' (${Config.keymap.toggle_2d_view})');

			var ar = [tr("All")];
			for (p in Project.paintObjects) ar.push(p.name);
			var atlases = Project.getUsedAtlases();
			if (atlases != null) for (a in atlases) ar.push(a);
			var filterHandle = Id.handle();
			filterHandle.position = Context.layerFilter;
			Context.layerFilter = ui.combo(filterHandle, ar, tr("Filter"), false, Left);
			if (filterHandle.changed) {
				for (p in Project.paintObjects) {
					p.visible = Context.layerFilter == 0 || p.name == ar[Context.layerFilter] || Project.isAtlasObject(p);
				}
				if (Context.layerFilter == 0 && Context.mergedObjectIsAtlas) { // All
					MeshUtil.mergeMesh();
				}
				else if (Context.layerFilter > Project.paintObjects.length) { // Atlas
					var visibles: Array<MeshObject> = [];
					for (p in Project.paintObjects) if (p.visible) visibles.push(p);
					MeshUtil.mergeMesh(visibles);
				}
				Layers.setObjectMask();
				UVUtil.uvmapCached = false;
				Context.ddirty = 2;
				#if (kha_direct3d12 || kha_vulkan)
				arm.render.RenderPathRaytrace.ready = false;
				#end
			}

			ui.endSticky();
			ui._y += 2;

			var step = ui.t.ELEMENT_H * 2;
			var fullH = ui._windowH - UISidebar.inst.hwnd0.scrollOffset;
			for (i in 0...Std.int(fullH / step)) {
				if (i % 2 == 0) {
					ui.fill(0, i * step, (ui._w / ui.SCALE() - 2), step, ui.t.WINDOW_BG_COL - 0x00040404);
				}
			}

			for (i in 0...Project.layers.length) {
				if (i >= Project.layers.length) break; // Layer was deleted
				var j = Project.layers.length - 1 - i;
				var l = Project.layers[j];
				drawLayerSlot(l, j);
			}
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
		Context.dragDestination = Project.layers.indexOf(layer);
	}

	static function drawLayerSlot(l: LayerSlot, i: Int) {
		var ui = UISidebar.inst.ui;

		if (Context.layerFilter > 0 &&
			l.getObjectMask() > 0 &&
			l.getObjectMask() != Context.layerFilter) {
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
		var mx = mouse.x;
		var my = mouse.y;
		var inLayers = mx > UISidebar.inst.tabx && my < Config.raw.layout[LayoutSidebarH0];
		if (App.isDragging && App.dragLayer != null && inLayers) {
			if (my > ui._y + step && my < ui._y + step * 3) {
				var down = Project.layers.indexOf(App.dragLayer) >= i;
				Context.dragDestination = down ? i : i - 1;

				var ls = Project.layers;
				var dest = Context.dragDestination;
				var toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				var nestedGroup = App.dragLayer.isGroup() && toGroup;
				if (!nestedGroup) {
					if (Context.layer.canMove(Context.dragDestination)) {
						ui.fill(checkw, step * 2, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
					}
				}
			}
			else if (i == Project.layers.length - 1 && my < ui._y + step) {
				Context.dragDestination = Project.layers.length - 1;
				if (Context.layer.canMove(Context.dragDestination)) {
					ui.fill(checkw, 0, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
				}
			}
		}

		var hasPanel = l.isGroup() || (l.isLayer() && l.getMasks(false) != null);
		if (hasPanel) {
			ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
		}
		else {
			ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100]);
		}

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
			l.visible = !l.visible;
			UIView2D.inst.hwnd.redraws = 2;
			MakeMaterial.parseMeshMaterial();
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		var contextMenu = false;

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

		var state = State.Idle;
		var iconH = (ui.ELEMENT_H() - 3) * 2;

		if (!l.isGroup()) {
			var icon = l.fill_layer == null ? l.texpaint_preview : l.fill_layer.imageIcon;
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

			state = ui.image(icon, 0xffffffff, iconH);

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

			if (l.fill_layer == null && l.isMask()) {
				ui.g.pipeline = null;
			}
		}
		else { // Group
			var folderClosed = Res.tile50(icons, 2, 1);
			var folderOpen = Res.tile50(icons, 8, 1);
			var folder = l.show_panel ? folderOpen : folderClosed;
			state = ui.image(icons, ui.t.LABEL_COL - 0x00202020, iconH, folder.x, folder.y, folder.w, folder.h);
		}

		ui._x -= 2;
		ui._y -= 3;

		#if kha_opengl
		ui.imageInvertY = false;
		#end

		if (ui.isHovered && l.texpaint_preview != null) {
			if (l.isMask()) {
				makeMaskPreviewRgba32(l);
				ui.tooltipImage(Context.maskPreviewRgba32);
			}
			else {
				ui.tooltipImage(l.texpaint_preview);
			}
			if (i < 9) ui.tooltip(l.name + " - (" + Config.keymap.select_layer + " " + (i + 1) + ")");
			else ui.tooltip(l.name);
		}
		if (ui.isHovered && ui.inputReleasedR) {
			Context.setLayer(l);
			contextMenu = true;
		}
		if (state == State.Started) {
			Context.setLayer(l);
			var mouse = Input.getMouse();
			setDragLayer(Context.layer, -(mouse.x - uix - ui._windowX - 3), -(mouse.y - uiy - ui._windowY + 1));
		}
		else if (state == State.Released) {
			if (Time.time() - Context.selectTime < 0.2) {
				UISidebar.inst.show2DView(View2DLayer);
			}
			if (Time.time() - Context.selectTime > 0.2) {
				Context.selectTime = Time.time();
			}
			if (l.fill_layer != null) Context.setMaterial(l.fill_layer);
		}

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
					setDragLayer(Context.layer, -(mouse.x - uix - ui._windowX - 3), -(mouse.y - uiy - ui._windowY + 1));
				}
				else if (ui.inputReleased) {
					if (Time.time() - Context.selectTime > 0.2) {
						Context.selectTime = Time.time();
					}
				}
				else if (ui.inputReleasedR) {
					Context.setLayer(l);
					contextMenu = true;
				}
			}

			var state = ui.text(l.name);
			if (state == State.Released) {
				var td = Time.time() - Context.selectTime;
				if (td < 0.2 && td > 0.0) {
					layerNameEdit = l.id;
					layerNameHandle.text = l.name;
					ui.startTextEdit(layerNameHandle);
				}
			}

			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && canDelete(Context.layer)) {
				ui.isDeleteDown = false;
				function _init() {
					deleteLayer(Context.layer);
				}
				iron.App.notifyOnInit(_init);
			}
		}
		ui._y -= center;

		if (l.parent != null) {
			ui._x -= 10 * ui.SCALE();
			if (l.parent.parent != null) ui._x -= 10 * ui.SCALE();
		}

		if (contextMenu) {
			drawLayerContextMenu(l);
		}

		if (l.isGroup()) {
			@:privateAccess ui.endElement();
		}
		else {
			if (l.isMask()) {
				ui._y += center;
			}

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
			], tr("Blending"));
			if (blendingHandle.changed) {
				Context.setLayer(l);
				History.layerBlending();
				l.blending = blendingHandle.position;
				MakeMaterial.parseMeshMaterial();
			}

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

			var ar = [tr("Shared")];
			for (p in Project.paintObjects) ar.push(p.name);
			var atlases = Project.getUsedAtlases();
			if (atlases != null) for (a in atlases) ar.push(a);
			var objectHandle = Id.handle().nest(l.id);
			objectHandle.position = l.objectMask;
			l.objectMask = ui.combo(objectHandle, ar, tr("Object"), false, Left);
			if (objectHandle.changed) {
				Context.setLayer(l);
				MakeMaterial.parseMeshMaterial();
				if (l.fill_layer != null) { // Fill layer
					function _init() {
						Context.material = l.fill_layer;
						l.clear();
						Layers.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
				}
				else {
					Layers.setObjectMask();
				}
			}
			@:privateAccess ui.endElement();
		}

		ui._y -= ui.ELEMENT_OFFSET();

		ui.fill(0, 0, (ui._w / ui.SCALE() - 2), 1 * ui.SCALE(), ui.t.SEPARATOR_COL);

		if (Context.layer == l) {
			ui.rect(1, -step * 2 - 1, (ui._w / ui.SCALE() - 2), step * 2 + 1, ui.t.HIGHLIGHT_COL, 2);
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

	static function drawLayerContextMenu(l: LayerSlot) {
		var add = 0;

		if (l.fill_layer == null) add += 1; // Clear
		if (l.fill_layer != null && !l.isMask()) add += 3;
		if (l.fill_layer != null && l.isMask()) add += 2;
		if (l.isMask()) add += 2;
		var menuElements = l.isGroup() ? 8 : (20 + add);

		UIMenu.draw(function(ui: Zui) {
			ui.text(l.name, Right, ui.t.HIGHLIGHT_COL);

			if (ui.button(tr("Export"), Left)) {
				if (l.isMask()) {
					UIFiles.show("png", true, false, function(path: String) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						if (!f.endsWith(".png")) f += ".png";
						Krom.writePng(path + Path.sep + f, l.texpaint.getPixels().getData(), l.texpaint.width, l.texpaint.height, 3); // RRR1
					});
				}
				else {
					Context.layersExport = ExportSelected;
					BoxExport.showTextures();
				}
			}

			if (!l.isGroup()) {
				var toFillString = l.isLayer() ? tr("To Fill Layer") : tr("To Fill Mask");
				var toPaintString = l.isLayer() ? tr("To Paint Layer") : tr("To Paint Mask");

				if (l.fill_layer == null && ui.button(toFillString, Left)) {
					function _init() {
						l.isLayer() ? History.toFillLayer() : History.toFillMask();
						l.toFillLayer();
					}
					iron.App.notifyOnInit(_init);
				}
				if (l.fill_layer != null && ui.button(toPaintString, Left)) {
					function _init() {
						l.isLayer() ? History.toPaintLayer() : History.toPaintMask();
						l.toPaintLayer();
					}
					iron.App.notifyOnInit(_init);
				}
			}

			ui.enabled = canDelete(l);
			if (ui.button(tr("Delete"), Left, "delete")) {
				function _init() {
					deleteLayer(Context.layer);
				}
				iron.App.notifyOnInit(_init);
			}
			ui.enabled = true;

			if (l.fill_layer == null && ui.button(tr("Clear"), Left)) {
				Context.setLayer(l);
				function _init() {
					if (!l.isGroup()) {
						History.clearLayer();
						l.clear();
					}
					else {
						for (c in l.getChildren()) {
							Context.layer = c;
							History.clearLayer();
							c.clear();
						}
						Context.layersPreviewDirty = true;
						Context.layer = l;
					}
				}
				iron.App.notifyOnInit(_init);
			}
			if (l.isMask() && l.fill_layer == null && ui.button(tr("Invert"), Left)) {
				function _init() {
					Context.setLayer(l);
					History.invertMask();
					l.invertMask();
				}
				iron.App.notifyOnInit(_init);
			}
			if (l.isMask() && ui.button(tr("Apply"), Left)) {
				function _init() {
					Context.layer = l;
					History.applyMask();
					l.applyMask();
					Context.setLayer(l.parent);
					MakeMaterial.parseMeshMaterial();
					Context.layersPreviewDirty = true;
				}
				iron.App.notifyOnInit(_init);
			}
			if (l.isGroup() && ui.button(tr("Merge Group"), Left)) {
				function _init() {
					Layers.mergeGroup(l);
				}
				iron.App.notifyOnInit(_init);
			}
			ui.enabled = canMergeDown(l);
			if (ui.button(tr("Merge Down"), Left)) {
				function _init() {
					Context.setLayer(l);
					History.mergeLayers();
					Layers.mergeDown();
					if (Context.layer.fill_layer != null) Context.layer.toPaintLayer();
				}
				iron.App.notifyOnInit(_init);
			}
			ui.enabled = true;
			if (ui.button(tr("Duplicate"), Left)) {
				function _init() {
					Context.setLayer(l);
					History.duplicateLayer();
					Layers.duplicateLayer(l);
				}
				iron.App.notifyOnInit(_init);
			}

			ui.row([7 / 10, 3 / 10]);
			var layerOpacHandle = Id.handle().nest(l.id);
			layerOpacHandle.value = l.maskOpacity;
			ui.slider(layerOpacHandle, "", 0.0, 1.0, true);
			if (layerOpacHandle.changed) {
				if (ui.inputStarted) History.layerOpacity();
				l.maskOpacity = layerOpacHandle.value;
				MakeMaterial.parseMeshMaterial();
				UIMenu.keepOpen = true;
			}
			ui.text(tr("Opacity"));

			if (!l.isGroup()) {
				ui.row([7 / 10, 3 / 10]);
				var resHandleChangedLast = App.resHandle.changed;
				#if (krom_android || krom_ios)
				var ar = ["128", "256", "512", "1K", "2K", "4K"];
				#else
				var ar = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
				#end
				App.resHandle.value = App.resHandle.position;
				App.resHandle.position = Std.int(ui.slider(App.resHandle, ar[App.resHandle.position], 0, ar.length - 1, false, 1, false, Left, false));
				if (App.resHandle.changed) {
					UIMenu.keepOpen = true;
				}
				if (resHandleChangedLast && !App.resHandle.changed) {
					Layers.onLayersResized();
				}
				ui.text(tr("Res"));

				ui.row([7 / 10, 3 / 10]);
				#if (krom_android || krom_ios)
				zui.Ext.inlineRadio(ui, App.bitsHandle, ["8bit"]);
				#else
				zui.Ext.inlineRadio(ui, App.bitsHandle, ["8bit", "16bit", "32bit"]);
				#end
				if (App.bitsHandle.changed) {
					iron.App.notifyOnInit(Layers.setLayerBits);
					UIMenu.keepOpen = true;
				}
				ui.text(tr("Color"));
			}

			if (l.fill_layer != null) {
				ui.row([7 / 10, 3 / 10]);
				var scaleHandle = Id.handle().nest(l.id);
				scaleHandle.value = l.scale;
				l.scale = ui.slider(scaleHandle, "", 0.0, 5.0, true);
				if (scaleHandle.changed) {
					Context.setMaterial(l.fill_layer);
					Context.setLayer(l);
					function _init() {
						Layers.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
					UIMenu.keepOpen = true;
				}
				ui.text(tr("UV Scale"));

				ui.row([7 / 10, 3 / 10]);
				var angleHandle = Id.handle().nest(l.id);
				angleHandle.value = l.angle;
				l.angle = ui.slider(angleHandle, "", 0.0, 360, true, 1);
				if (angleHandle.changed) {
					Context.setMaterial(l.fill_layer);
					Context.setLayer(l);
					MakeMaterial.parsePaintMaterial();
					function _init() {
						Layers.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
					UIMenu.keepOpen = true;
				}
				ui.text(tr("Angle"));

				ui.row([7 / 10, 3 / 10]);
				var uvTypeHandle = Id.handle().nest(l.id);
				uvTypeHandle.position = l.uvType;
				l.uvType = zui.Ext.inlineRadio(ui, uvTypeHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], Left);
				if (uvTypeHandle.changed) {
					Context.setMaterial(l.fill_layer);
					Context.setLayer(l);
					MakeMaterial.parsePaintMaterial();
					function _init() {
						Layers.updateFillLayers();
					}
					iron.App.notifyOnInit(_init);
					UIMenu.keepOpen = true;
				}
				ui.text(tr("TexCoord"));
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
				l.paintBase = ui.check(baseHandle, tr("Base Color"));
				l.paintOpac = ui.check(opacHandle, tr("Opacity"));
				l.paintNor = ui.check(norHandle, tr("Normal"));
				l.paintNorBlend = ui.check(norBlendHandle, tr("Normal Blending"));
				l.paintOcc = ui.check(occHandle, tr("Occlusion"));
				l.paintRough = ui.check(roughHandle, tr("Roughness"));
				l.paintMet = ui.check(metHandle, tr("Metallic"));
				l.paintHeight = ui.check(heightHandle, tr("Height"));
				l.paintHeightBlend = ui.check(heightBlendHandle, tr("Height Blending"));
				l.paintEmis = ui.check(emisHandle, tr("Emission"));
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
		if (Context.maskPreviewRgba32 == null) {
			Context.maskPreviewRgba32 = kha.Image.createRenderTarget(RenderUtil.layerPreviewSize, RenderUtil.layerPreviewSize);
		}
		// Convert from R8 to RGBA32 for tooltip display
		if (Context.maskPreviewLast != l) {
			Context.maskPreviewLast = l;
			iron.App.notifyOnInit(function() {
				Context.maskPreviewRgba32.g2.begin();
				Context.maskPreviewRgba32.g2.pipeline = UIView2D.pipe;
				Context.maskPreviewRgba32.g4.setInt(UIView2D.channelLocation, 1);
				Context.maskPreviewRgba32.g2.drawImage(l.texpaint_preview, 0, 0);
				Context.maskPreviewRgba32.g2.end();
				Context.maskPreviewRgba32.g2.pipeline = null;
			});
		}
	}

	static function deleteLayer(l: LayerSlot) {
		var pointers = initLayerMap();
		
		if (l.isLayer() && l.hasMasks(false)) {
			for (m in l.getMasks(false)) {
				Context.layer = m;
				History.deleteLayer();
				m.delete();
			}
		}
		if (l.isGroup()) {
			for (c in l.getChildren()) {
				if (c.hasMasks(false)) {
					for (m in c.getMasks(false)) {
						Context.layer = m;
						History.deleteLayer();
						m.delete();
					}
				}
				Context.layer = c;
				History.deleteLayer();
				c.delete();
			}
			if (l.hasMasks()) {
				for (m in l.getMasks()) {
					Context.layer = m;
					History.deleteLayer();
					m.delete();
				}
			}
		}
		
		Context.layer = l;
		History.deleteLayer();
		l.delete();

		// Remove empty group
		if (l.isInGroup() && l.getContainingGroup().getChildren() == null) {
			var g = l.getContainingGroup();
			// Maybe some group masks are left
			if (g.hasMasks()) {
				for (m in g.getMasks()) {
					Context.layer = m;
					History.deleteLayer();
					m.delete();
				}
			}
			Context.layer = l.parent;
			History.deleteLayer();
			l.parent.delete();
		}
		Context.ddirty = 2;
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
}
