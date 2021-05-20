package arm.ui;

import haxe.Json;
import kha.Image;
import kha.System;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.system.Input;
import arm.shader.NodesMaterial;
import arm.node.NodesBrush;
import arm.node.MakeMaterial;
import arm.util.RenderUtil;
import arm.ui.UIHeader;
import arm.Enums;
import arm.Project;

@:access(zui.Zui)
@:access(zui.Nodes)
class UINodes {

	public static var inst: UINodes;

	public var show = false;
	public var wx: Int;
	public var wy: Int;
	public var ww: Int;
	public var wh: Int;

	public var ui: Zui;
	public var canvasType = CanvasMaterial;
	var drawMenu = false;
	var showMenu = false;
	var hideMenu = false;
	var menuCategory = 0;
	var addNodeButton = false;
	var popupX = 0.0;
	var popupY = 0.0;

	var uichangedLast = false;
	var recompileMat = false; // Mat preview
	var recompileMatFinal = false;
	var nodeSearchSpawn: TNode = null;
	var nodeSearchOffset = 0;
	var nodeSearchLast = "";
	var lastCanvas: TNodeCanvas = null;
	var lastNodeSelected: TNode = null;
	var releaseLink = false;

	public var grid: Image = null;
	public var hwnd = Id.handle();
	public var groupStack: Array<TNodeGroup> = [];

	public function new() {
		inst = this;

		Nodes.excludeRemove.push("OUTPUT_MATERIAL_PBR");
		Nodes.excludeRemove.push("GROUP_OUTPUT");
		Nodes.excludeRemove.push("GROUP_INPUT");
		Nodes.excludeRemove.push("BrushOutputNode");
		Nodes.onLinkDrag = onLinkDrag;
		Nodes.onSocketReleased = onSocketReleased;
		Nodes.onCanvasReleased = onCanvasReleased;
		Nodes.onNodeRemove = onNodeRemove;
		Nodes.onCanvasControl = onCanvasControl;

		var scale = Config.raw.window_scale;
		ui = new Zui({theme: App.theme, font: App.font, color_wheel: App.colorWheel, scaleFactor: scale});
		ui.scrollEnabled = false;
	}

	function onLinkDrag(linkDrag: TNodeLink, isNewLink: Bool) {
		if (isNewLink) {
			var nodes = getNodes();
			var node = nodes.getNode(getCanvas(true).nodes, linkDrag.from_id > -1 ? linkDrag.from_id : linkDrag.to_id);
			var linkX = ui._windowX + nodes.NODE_X(node);
			var linkY = ui._windowY + nodes.NODE_Y(node);
			if (linkDrag.from_id > -1) {
				linkX += nodes.NODE_W();
				linkY += nodes.OUTPUT_Y(node.outputs, linkDrag.from_socket);
			}
			else {
				linkY += nodes.INPUT_Y(getCanvas(true), node.inputs, linkDrag.to_socket) + nodes.OUTPUTS_H(node.outputs) + nodes.BUTTONS_H(node);
			}
			var mouse = Input.getMouse();
			if (Math.abs(mouse.x - linkX) > 5 || Math.abs(mouse.y - linkY) > 5) { // Link length
				nodeSearch(-1, -1, function() {
					var n = nodes.nodesSelected[0];
					if (linkDrag.to_id == -1 && n.inputs.length > 0) {
						linkDrag.to_id = n.id;
						linkDrag.to_socket = 0;
						getCanvas(true).links.push(linkDrag);
					}
					else if (linkDrag.from_id == -1 && n.outputs.length > 0) {
						linkDrag.from_id = n.id;
						linkDrag.from_socket = 0;
						getCanvas(true).links.push(linkDrag);
					}
				});
			}
			// Selecting which node socket to preview
			else if (node == nodes.nodesSelected[0]) {
				Context.nodePreviewSocket = linkDrag.from_id > -1 ? linkDrag.from_socket : 0;
				Context.nodePreviewDirty = true;
			}
		}
	}

	function onSocketReleased(socket: TNodeSocket) {
		var nodes = getNodes();
		var node = nodes.getNode(getCanvas(true).nodes, socket.node_id);
		if (ui.inputReleasedR) {
			if (node.type == "GROUP_INPUT" || node.type == "GROUP_OUTPUT") {
				App.notifyOnNextFrame(function() {
					arm.ui.UIMenu.draw(function(ui: Zui) {
						ui.text(tr("Socket"), Right, ui.t.HIGHLIGHT_COL);
						if (ui.button(tr("Edit"), Left)) {
							var htype = Id.handle();
							var hname = Id.handle();
							var hmin = Id.handle();
							var hmax = Id.handle();
							var hval0 = Id.handle();
							var hval1 = Id.handle();
							var hval2 = Id.handle();
							var hval3 = Id.handle();
							htype.position = socket.type == "RGBA" ? 0 : socket.type == "VECTOR" ? 1 : 2;
							hname.text = socket.name;
							hmin.value = socket.min;
							hmax.value = socket.max;
							if (socket.type == "RGBA" || socket.type == "VECTOR") {
								hval0.value = socket.default_value[0];
								hval1.value = socket.default_value[1];
								hval2.value = socket.default_value[2];
								if (socket.type == "RGBA") {
									hval3.value = socket.default_value[3];
								}
							}
							else hval0.value = socket.default_value;
							App.notifyOnNextFrame(function() {
								App.uiBox.endInput();
								UIBox.showCustom(function(ui: Zui) {
									if (ui.tab(Id.handle(), tr("Socket"))) {
										var type = ui.combo(htype, [tr("Color"), tr("Vector"), tr("Value")], tr("Type"), true);
										if (htype.changed) hname.text = type == 0 ? tr("Color") : type == 1 ? tr("Vector") : tr("Value");
										var name = ui.textInput(hname, tr("Name"));
										var min = zui.Ext.floatInput(ui, hmin, tr("Min"));
										var max = zui.Ext.floatInput(ui, hmax, tr("Max"));
										var default_value: Dynamic = null;
										if (type == 0) {
											ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
											zui.Ext.floatInput(ui, hval0, tr("R"));
											zui.Ext.floatInput(ui, hval1, tr("G"));
											zui.Ext.floatInput(ui, hval2, tr("B"));
											zui.Ext.floatInput(ui, hval3, tr("A"));
											default_value = [hval0.value, hval1.value, hval2.value, hval3.value];
										}
										else if (type == 1) {
											ui.row([1 / 3, 1 / 3, 1 / 3]);
											hval0.value = zui.Ext.floatInput(ui, hval0, tr("X"));
											hval1.value = zui.Ext.floatInput(ui, hval1, tr("Y"));
											hval2.value = zui.Ext.floatInput(ui, hval2, tr("Z"));
											default_value = [hval0.value, hval1.value, hval2.value];
										}
										else {
											default_value = zui.Ext.floatInput(ui, hval0, tr("default_value"));
										}
										if (ui.button(tr("OK"))) { // || ui.isReturnDown
											socket.name = name;
											socket.type = type == 0 ? "RGBA" : type == 1 ? "VECTOR" : "VALUE";
											socket.color = NodesMaterial.get_socket_color(socket.type);
											socket.min = min;
											socket.max = max;
											socket.default_value = default_value;
											UIBox.show = false;
											NodesMaterial.syncSockets(node);
											hwnd.redraws = 2;
										}
									}
								}, 400, 250);
							});
						}
						if (ui.button(tr("Delete"), Left)) {
							node.inputs.remove(socket);
							node.outputs.remove(socket);
							NodesMaterial.syncSockets(node);
						}
					}, 3);
				});
			}
		}
		// Selecting which node socket to preview
		else if (node == nodes.nodesSelected[0]) {
			var i = node.outputs.indexOf(socket);
			if (i > -1) {
				Context.nodePreviewSocket = i;
				Context.nodePreviewDirty = true;
			}
		}
	}

	function onCanvasReleased() {
		if (ui.inputReleasedR && Math.abs(ui.inputX - ui.inputStartedX) < 2 && Math.abs(ui.inputY - ui.inputStartedY) < 2) {
			UIMenu.draw(function(uiMenu: Zui) {
				var nodes = getNodes();
				uiMenu._y += 1;
				uiMenu.enabled = nodes.nodesSelected.length > 0;
				if (menuButton(uiMenu, tr("Cut"), "ctrl+x")) {
					Zui.isCopy = true;
					Zui.isCut = true;
				}
				if (menuButton(uiMenu, tr("Copy"), "ctrl+c")) {
					Zui.isCopy = true;
				}
				uiMenu.enabled = Nodes.clipboard != "";
				if (menuButton(uiMenu, tr("Paste"), "ctrl+v")) {
					Zui.isPaste = true;
				}
				uiMenu.enabled = nodes.nodesSelected.length > 0;
				if (menuButton(uiMenu, tr("Delete"), "delete")) {
					ui.isDeleteDown = true;
					App.notifyOnNextFrame(function() { ui.isDeleteDown = false; });
				}
				if (menuButton(uiMenu, tr("Duplicate"))) {
					Zui.isCopy = true;
					Zui.isPaste = true;
				}
				uiMenu.enabled = true;
			}, 5);
		}
	}

	public static function onNodeRemove(node: TNode) {
		if (node.type == "GROUP") { // Remove unused groups
			var found = false;
			var canvases: Array<TNodeCanvas> = [];
			for (m in Project.materials) canvases.push(m.canvas);
			for (m in Project.materialGroups) canvases.push(m.canvas);
			for (canvas in canvases) {
				for (n in canvas.nodes) {
					if (n.type == "GROUP" && n.name == node.name) {
						found = true;
						break;
					}
				}
			}
			if (!found) {
				for (g in Project.materialGroups) {
					if (g.canvas.name == node.name) {
						Project.materialGroups.remove(g);
						break;
					}
				}
			}
		}
	}

	function onCanvasControl(): zui.Nodes.CanvasControl {
		return getCanvasControl(ui);
	}

	public static function getCanvasControl(ui: Zui): zui.Nodes.CanvasControl {
		var pan = ui.inputDownR || Operator.shortcut(Config.keymap.action_pan, ShortcutDown);
		var zoomDelta = Operator.shortcut(Config.keymap.action_zoom, ShortcutDown) ? getZoomDelta(ui) / 100.0 : 0.0;
		return {
			panX: pan ? ui.inputDX : 0.0,
			panY: pan ? ui.inputDY : 0.0,
			zoom: ui.inputWheelDelta != 0.0 ? -ui.inputWheelDelta / 10 : zoomDelta
		}
	}

	static function getZoomDelta(ui: Zui): Float {
		return Config.raw.zoom_direction == ZoomVertical ? -ui.inputDY :
			   Config.raw.zoom_direction == ZoomVerticalInverted ? -ui.inputDY :
			   Config.raw.zoom_direction == ZoomHorizontal ? ui.inputDX :
			   Config.raw.zoom_direction == ZoomHorizontalInverted ? ui.inputDX :
			   -(ui.inputDY - ui.inputDX);
	}

	public function getCanvas(groups = false): TNodeCanvas {
		if (canvasType == CanvasMaterial) {
			if (groups && groupStack.length > 0) return groupStack[groupStack.length - 1].canvas;
			else return getCanvasMaterial();
		}
		else return Context.brush.canvas;
	}

	public function getCanvasMaterial(): TNodeCanvas {
		return Context.material.canvas;
	}

	public function getNodes(): Nodes {
		if (canvasType == CanvasMaterial) {
			if (groupStack.length > 0) return groupStack[groupStack.length - 1].nodes;
			else return Context.material.nodes;
		}
		else return Context.brush.nodes;
	}

	public function update() {
		if (!show || !App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		wy = UIHeader.inst.headerh * 2;
		if (UIView2D.inst.show) {
			wy += iron.App.h() - Config.raw.layout[LayoutNodesH];
		}
		var ww = Config.raw.layout[LayoutNodesW];
		if (!UISidebar.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
			wx -= UIToolbar.inst.toolbarw;
			wy = 0;
		}
		var mx = mouse.x;
		var my = mouse.y;
		if (mx < wx || mx > wx + ww || my < wy) return;
		if (ui.isTyping || !ui.inputEnabled) return;

		if (addNodeButton) {
			showMenu = true;
			addNodeButton = false;
		}
		else if (mouse.released()) {
			hideMenu = true;
		}

		var nodes = getNodes();
		if (nodes.nodesSelected.length > 0 && ui.isKeyPressed) {
			if (ui.key == kha.input.KeyCode.Left) for (n in nodes.nodesSelected) n.x -= 1;
			else if (ui.key == kha.input.KeyCode.Right) for (n in nodes.nodesSelected) n.x += 1;
			if (ui.key == kha.input.KeyCode.Up) for (n in nodes.nodesSelected) n.y -= 1;
			else if (ui.key == kha.input.KeyCode.Down) for (n in nodes.nodesSelected) n.y += 1;
		}

		// Node search popup
		if (kb.started(Config.keymap.node_search)) nodeSearch();
		if (nodeSearchSpawn != null) {
			ui.inputX = mouse.x; // Fix inputDX after popup removal
			ui.inputY = mouse.y;
			nodeSearchSpawn = null;
		}

		if (Operator.shortcut(Config.keymap.view_reset)) {
			nodes.panX = 0.0;
			nodes.panY = 0.0;
			nodes.zoom = 1.0;
		}
	}

	public function canvasChanged() {
		recompileMat = true;
		recompileMatFinal = true;
	}

	function nodeSearch(x = -1, y = -1, done: Void->Void = null) {
		var kb = Input.getKeyboard();
		var searchHandle = Id.handle();
		var first = true;
		UIMenu.draw(function(ui: Zui) {
			ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 8, ui.t.WINDOW_BG_COL);
			ui.textInput(searchHandle, "");
			ui.changed = false;
			if (first) {
				first = false;
				ui.startTextEdit(searchHandle); // Focus search bar
				ui.textSelected = searchHandle.text;
				searchHandle.text = "";
				nodeSearchLast = "";
			}
			var search = searchHandle.text;
			if (ui.textSelected != "") search = ui.textSelected;

			if (search != nodeSearchLast) {
				nodeSearchOffset = 0;
				nodeSearchLast = search;
			}
			if (ui.isKeyPressed) { // Move selection
				if (ui.key == kha.input.KeyCode.Down && nodeSearchOffset < 6) nodeSearchOffset++;
				if (ui.key == kha.input.KeyCode.Up && nodeSearchOffset > 0) nodeSearchOffset--;
			}
			var enter = kb.down("enter");
			var count = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;
			var nodeList = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			for (list in nodeList) {
				for (n in list) {
					if (tr(n.name).toLowerCase().indexOf(search) >= 0) {
						ui.t.BUTTON_COL = count == nodeSearchOffset ? ui.t.HIGHLIGHT_COL : ui.t.WINDOW_BG_COL;
						if (ui.button(tr(n.name), Left) || (enter && count == nodeSearchOffset)) {
							pushUndo();
							var nodes = getNodes();
							var canvas = getCanvas(true);
							nodeSearchSpawn = makeNode(n, nodes, canvas); // Spawn selected node
							canvas.nodes.push(nodeSearchSpawn);
							nodes.nodesSelected = [nodeSearchSpawn];
							nodes.nodesDrag = true;
							hwnd.redraws = 2;
							if (enter) {
								ui.changed = true;
								count = 6; // Trigger break
							}
							if (done != null) done();
						}
						if (++count > 6) break;
					}
				}
				if (count > 6) break;
			}
			if (enter && count == 0) { // Hide popup on enter when node is not found
				ui.changed = true;
				searchHandle.text = "";
			}
			ui.t.BUTTON_COL = BUTTON_COL;
		}, 0, x, y);
	}

	public function getNodeX(): Int {
		var mouse = Input.getMouse();
		return Std.int((mouse.x - wx - getNodes().PAN_X()) / getNodes().SCALE());
	}

	public function getNodeY(): Int {
		var mouse = Input.getMouse();
		return Std.int((mouse.y - wy - getNodes().PAN_Y()) / getNodes().SCALE());
	}

	public function drawGrid() {
		var ww = Config.raw.layout[LayoutNodesW];
		if (!UISidebar.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
		}
		var wh = iron.App.h();
		var w = ww + 100 * 2;
		var h = wh + 100 * 2;
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		grid = Image.createRenderTarget(w, h);
		grid.g2.begin(true, ui.t.SEPARATOR_COL);

		grid.g2.color = ui.t.SEPARATOR_COL - 0x00050505;
		for (i in 0...Std.int(h / 20) + 1) {
			grid.g2.drawLine(0, i * 20, w, i * 20);
		}
		for (i in 0...Std.int(w / 20) + 1) {
			grid.g2.drawLine(i * 20, 0, i * 20, h);
		}

		grid.g2.color = ui.t.SEPARATOR_COL - 0x00090909;
		for (i in 0...Std.int(h / 100) + 1) {
			grid.g2.drawLine(0, i * 100, w, i * 100);
		}
		for (i in 0...Std.int(w / 100) + 1) {
			grid.g2.drawLine(i * 100, 0, i * 100, h);
		}

		grid.g2.end();
	}

	public function render(g: kha.graphics2.Graphics) {
		if (recompileMat) {
			if (canvasType == CanvasBrush) {
				MakeMaterial.parseBrush();
				RenderUtil.makeBrushPreview();
				UISidebar.inst.hwnd1.redraws = 2;
			}
			else {
				Layers.isFillMaterial() ? Layers.updateFillLayers() : RenderUtil.makeMaterialPreview();
			}

			UISidebar.inst.hwnd1.redraws = 2;
			recompileMat = false;
		}
		else if (recompileMatFinal) {
			MakeMaterial.parsePaintMaterial();

			if (canvasType == CanvasMaterial && Layers.isFillMaterial()) {
				Layers.updateFillLayers();
				RenderUtil.makeMaterialPreview();
			}

			var decal = Context.tool == ToolDecal || Context.tool == ToolText;
			if (decal) RenderUtil.makeDecalPreview();

			UISidebar.inst.hwnd0.redraws = 2;
			recompileMatFinal = false;
			Context.nodePreviewDirty = true;
		}

		var nodes = getNodes();
		if (nodes.nodesSelected.length > 0 && nodes.nodesSelected[0] != lastNodeSelected) {
			lastNodeSelected = nodes.nodesSelected[0];
			Context.nodePreviewDirty = true;
			Context.nodePreviewSocket = 0;
		}

		// Remove dragged link when mouse is released out of the node viewport
		var c = getCanvas(true);
		if (releaseLink && nodes.linkDrag != null) {
			c.links.remove(nodes.linkDrag);
			nodes.linkDrag = null;
		}
		releaseLink = ui.inputReleased;

		if (!show || System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (!App.uiEnabled && ui.inputRegistered) ui.unregisterInput();
		if (App.uiEnabled && !ui.inputRegistered) ui.registerInput();

		g.end();

		if (grid == null) drawGrid();

		if (Config.raw.node_preview && Context.nodePreviewDirty) makeNodePreview();

		// Start with UI
		ui.begin(g);

		// Make window
		ww = Config.raw.layout[LayoutNodesW];
		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		wy = UIHeader.inst.headerh * 2;
		if (!UISidebar.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
			wx -= UIToolbar.inst.toolbarw;
			wy = 0;
		}
		var ew = Std.int(ui.ELEMENT_W() * 0.7);
		wh = iron.App.h();
		if (UIView2D.inst.show) {
			wh = Config.raw.layout[LayoutNodesH];
			wy = iron.App.h() - Config.raw.layout[LayoutNodesH] + UIHeader.inst.headerh * 2;
			if (!UISidebar.inst.show) {
				wy -= UIHeader.inst.headerh * 2;
			}
		}
		if (ui.window(hwnd, wx, wy, ww, wh)) {

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE()) % 100 - 100, (nodes.panY * nodes.SCALE()) % 100 - 100);

			// Undo
			if (ui.inputStarted || ui.isKeyPressed) {
				lastCanvas = Json.parse(Json.stringify(getCanvas(true)));
			}

			// Nodes
			ui.inputEnabled = !drawMenu;
			ui.windowBorderRight = Config.raw.layout[LayoutSidebarW];
			ui.windowBorderTop = UIHeader.inst.headerh * 2;
			ui.windowBorderBottom = Config.raw.layout[LayoutStatusH];
			nodes.nodeCanvas(ui, c);
			ui.inputEnabled = true;

			// Remove nodes with unknown id for this canvas type
			if (Zui.isPaste) {
				var nodeList = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
				var i = 0;
				while (i++ < c.nodes.length) {
					var canvasNode = c.nodes[i - 1];
					if (Nodes.excludeRemove.indexOf(canvasNode.type) >= 0) {
						continue;
					}
					var found = false;
					for (list in nodeList) {
						for (listNode in list) {
							if (canvasNode.type == listNode.type) {
								found = true;
								break;
							}
						}
						if (found) break;
					}
					if (!found) {
						nodes.removeNode(canvasNode, c);
						nodes.nodesSelected.remove(canvasNode);
						i--;
					}
				}
			}

			// Recompile material on change
			if (ui.changed) {
				recompileMat = (ui.inputDX != 0 || ui.inputDY != 0 || !uichangedLast) && Config.raw.material_live; // Instant preview
			}
			else if (uichangedLast) {
				canvasChanged();
				pushUndo(lastCanvas);
			}
			uichangedLast = ui.changed;

			// Node previews
			if (Config.raw.node_preview && nodes.nodesSelected.length > 0) {
				var img: kha.Image = null;
				var sel = nodes.nodesSelected[0];
				if (sel.type == "LAYER") {
					var id = sel.buttons[0].default_value;
					if (id < Project.layers.length) {
						img = Project.layers[id].texpaint_preview;
					}
				}
				else if (sel.type == "LAYER_MASK") {
					var id = sel.buttons[0].default_value;
					if (id < Project.layers.length) {
						img = Project.layers[id].texpaint_mask_preview;
					}
				}
				else if (sel.type == "MATERIAL") {
					var id = sel.buttons[0].default_value;
					if (id < Project.materials.length) {
						img = Project.materials[id].image;
					}
				}
				else if (sel.type == "OUTPUT_MATERIAL_PBR") {
					img = Context.material.image;
				}
				else if (sel.type == "BrushOutputNode") {
					img = Context.brush.image;
				}
				else if (canvasType == CanvasMaterial) {
					img = Context.nodePreview;
				}
				if (img != null) {
					var tw = 80 * ui.SCALE();
					var th = tw * (img.height / img.width);
					var tx = ww - tw - 8 * ui.SCALE();
					var ty = wh - th - 40 * ui.SCALE();

					#if kha_opengl
					var invertY = sel.type == "MATERIAL";
					#else
					var invertY = false;
					#end

					ui.g.color = 0xffffffff;
					invertY ?
						ui.g.drawScaledImage(img, tx, ty + th, tw, -th) :
						ui.g.drawScaledImage(img, tx, ty, tw, th);
				}
			}

			// Editable canvas name
			var _ACCENT_COL = ui.t.ACCENT_COL;
			var _BUTTON_H = ui.t.BUTTON_H;
			var _ELEMENT_H = ui.t.ELEMENT_H;
			var _FONT_SIZE = ui.fontSize;
			ui.t.ACCENT_COL = 0x00000000;
			ui.t.BUTTON_H = 30;
			ui.t.ELEMENT_H = 30;
			ui.fontSize = Std.int(22 * ui.SCALE());
			ui._x = ww - ui.ELEMENT_W() * 1.4;
			ui._y = wh - ui.ELEMENT_H() * 1.2;
			ui._w = Std.int(ui.ELEMENT_W() * 1.4);
			var h = Id.handle();
			h.text = c.name;
			var oldName = c.name;
			c.name = ui.textInput(h, "", Right);
			if (h.changed && groupStack.length > 0) { // Update group links
				var canvases: Array<TNodeCanvas> = [];
				for (m in Project.materials) canvases.push(m.canvas);
				for (m in Project.materialGroups) canvases.push(m.canvas);
				for (canvas in canvases) {
					for (n in canvas.nodes) {
						if (n.type == "GROUP" && n.name == oldName) {
							n.name = c.name;
						}
					}
				}
			}
			ui.t.ACCENT_COL = _ACCENT_COL;
			ui.t.BUTTON_H = _BUTTON_H;
			ui.t.ELEMENT_H = _ELEMENT_H;
			ui.fontSize = _FONT_SIZE;

			// Close node group
			if (groupStack.length > 0) {
				ui._x = 5;
				ui._y = wh - ui.ELEMENT_H() * 1.2;
				ui._w = Std.int(ui.ELEMENT_W() * 1.4);
				if (ui.button(tr("Close"))) groupStack.pop();
			}

			// Menu
			ui.g.color = ui.t.WINDOW_BG_COL;
			ui.g.fillRect(0, 0, ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET());
			ui.g.color = 0xffffffff;

			ui._x = 0;
			ui._y = 0;
			ui._w = ew;

			var _BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;

			var cats = canvasType == CanvasMaterial ? NodesMaterial.categories : NodesBrush.categories;
			for (i in 0...cats.length) {
				if ((ui.button(tr(cats[i]), Left) && UISidebar.inst.ui.comboSelectedHandle == null) || (ui.isHovered && drawMenu)) {
					addNodeButton = true;
					menuCategory = i;
					popupX = wx + ui._x;
					popupY = wy + ui._y;
				}
				if (i < cats.length - 1) {
					ui._x += ew + 3;
					ui._y = 0;
				}
			}
			ui._x += ew + 3;
			ui._y = 0;

			if (ui.button(tr("Search"), Left)) nodeSearch(Std.int(ui._windowX + ui._x), Std.int(ui._windowY + ui._y));
			if (ui.isHovered) ui.tooltip(tr("Search for nodes") + ' (${Config.keymap.node_search})');

			ui.t.BUTTON_COL = _BUTTON_COL;
		}

		ui.end(!drawMenu);

		g.begin(false);

		if (drawMenu) {
			var list = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			var numNodes = list[menuCategory].length;

			var isGroupCategory = canvasType == CanvasMaterial && NodesMaterial.categories[menuCategory] == tr("Group");
			if (isGroupCategory) numNodes += Project.materialGroups.length;

			var py = popupY;
			var menuw = Std.int(ew * 2.0);
			ui.beginRegion(g, Std.int(popupX), Std.int(py), menuw);
			var _BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
			var _BUTTON_H = ui.t.BUTTON_H;
			ui.t.BUTTON_H = ui.t.ELEMENT_H;
			var _ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;

			for (n in list[menuCategory]) {
				if (ui.button(Config.buttonSpacing + tr(n.name), Config.buttonAlign)) {
					pushUndo();
					var canvas = getCanvas(true);
					var nodes = getNodes();
					var node = makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					nodes.nodesSelected = [node];
					nodes.nodesDrag = true;
				}
				// Next column
				if (ui._y - wy + ui.ELEMENT_H() / 2 > wh) {
					ui._x += menuw;
					ui._y = py;
				}
			}
			if (isGroupCategory) {
				for (g in Project.materialGroups) {
					if (ui.button("      " + g.canvas.name, Left)) {
						pushUndo();
						var canvas = getCanvas(true);
						var nodes = getNodes();
						var node = makeGroupNode(g.canvas, nodes, canvas);
						canvas.nodes.push(node);
						nodes.nodesSelected = [node];
						nodes.nodesDrag = true;
					}
				}
			}

			ui.t.BUTTON_COL = _BUTTON_COL;
			ui.t.BUTTON_H = _BUTTON_H;
			ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
			ui.endRegion();
		}

		if (showMenu) {
			showMenu = false;
			drawMenu = true;
		}
		if (hideMenu) {
			hideMenu = false;
			drawMenu = false;
		}
	}

	function pushUndo(lastCanvas: TNodeCanvas = null) {
		if (lastCanvas == null) lastCanvas = getCanvas(true);
		UISidebar.inst.hwnd0.redraws = 2;
		var canvasGroup = groupStack.length > 0 ? Project.materialGroups.indexOf(groupStack[groupStack.length - 1]) : null;
		History.editNodes(lastCanvas, canvasType, canvasGroup);
	}

	public function acceptAssetDrag(index: Int) {
		pushUndo();
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		var n = canvasType == CanvasMaterial ? NodesMaterial.createNode("TEX_IMAGE", g) : NodesBrush.createNode("TEX_IMAGE");
		n.buttons[0].default_value = index;
		getNodes().nodesSelected = [n];
	}

	public function acceptLayerDrag(index: Int) {
		pushUndo();
		if (Project.layers[index].getChildren() != null) return;
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		var n = NodesMaterial.createNode(Context.layerIsMask ? "LAYER_MASK" : "LAYER", g);
		n.buttons[0].default_value = index;
		getNodes().nodesSelected = [n];
	}

	public function acceptMaterialDrag(index: Int) {
		pushUndo();
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		var n = NodesMaterial.createNode("MATERIAL", g);
		n.buttons[0].default_value = index;
		getNodes().nodesSelected = [n];
	}

	public function acceptSwatchDrag(index: Int) {
		pushUndo();
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		var n = NodesMaterial.createNode("RGB", g);
		var color = Project.raw.swatches[index].base;
		n.outputs[0].default_value = [color.R, color.G, color.B, color.A];
		getNodes().nodesSelected = [n];
	}

	public static function makeNode(n: TNode, nodes: Nodes, canvas: TNodeCanvas): TNode {
		var node: TNode = Json.parse(Json.stringify(n));
		node.id = nodes.getNodeId(canvas.nodes);
		node.x = UINodes.inst.getNodeX();
		node.y = UINodes.inst.getNodeY();
		for (soc in node.inputs) {
			soc.id = nodes.getSocketId(canvas.nodes);
			soc.node_id = node.id;
		}
		for (soc in node.outputs) {
			soc.id = nodes.getSocketId(canvas.nodes);
			soc.node_id = node.id;
		}
		return node;
	}

	public static function makeGroupNode(groupCanvas: TNodeCanvas, nodes: Nodes, canvas: TNodeCanvas): TNode {
		var n = NodesMaterial.list[5][0];
		var node: TNode = Json.parse(Json.stringify(n));
		node.name = groupCanvas.name;
		node.id = nodes.getNodeId(canvas.nodes);
		node.x = UINodes.inst.getNodeX();
		node.y = UINodes.inst.getNodeY();
		var groupInput: TNode = null;
		var groupOutput: TNode = null;
		for (g in Project.materialGroups) {
			if (g.canvas.name == node.name) {
				for (n in g.canvas.nodes) {
					if (n.type == "GROUP_INPUT") groupInput = n;
					else if (n.type == "GROUP_OUTPUT") groupOutput = n;
				}
				break;
			}
		}
		if (groupInput != null && groupOutput != null) {
			for (soc in groupInput.outputs) {
				node.inputs.push(NodesMaterial.createSocket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
			}
			for (soc in groupOutput.inputs) {
				node.outputs.push(NodesMaterial.createSocket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
			}
		}
		return node;
	}

	function makeNodePreview() {
		var nodes = Context.material.nodes;
		if (nodes.nodesSelected.length == 0) return;

		var node = nodes.nodesSelected[0];
		if (node.type == "LAYER" ||
			node.type == "LAYER_MASK" ||
			node.type == "MATERIAL" ||
			node.type == "OUTPUT_MATERIAL_PBR") return;

		if (Context.material.canvas.nodes.indexOf(node) == -1) return;

		if (Context.nodePreview == null) {
			Context.nodePreview = kha.Image.createRenderTarget(RenderUtil.matPreviewSize, RenderUtil.matPreviewSize);
		}

		Context.nodePreviewDirty = false;
		UINodes.inst.hwnd.redraws = 2;
		RenderUtil.makeNodePreview(Context.material.canvas, node, Context.nodePreview);
	}

	public static function hasGroup(c: TNodeCanvas): Bool {
		for (n in c.nodes) if (n.type == "GROUP") return true;
		return false;
	}

	public static function traverseGroup(mgroups: Array<TNodeCanvas>, c: TNodeCanvas) {
		for (n in c.nodes) {
			if (n.type == "GROUP") {
				if (getGroup(mgroups, n.name) == null) {
					var canvases: Array<TNodeCanvas> = [];
					for (g in Project.materialGroups) canvases.push(g.canvas);
					var group = getGroup(canvases, n.name);
					mgroups.push(Json.parse(Json.stringify(group)));
					traverseGroup(mgroups, group);
				}
			}
		}
	}

	static function getGroup(canvases: Array<TNodeCanvas>, name: String): TNodeCanvas {
		for (c in canvases) if (c.name == name) return c;
		return null;
	}

	static function menuButton(ui: Zui, text: String, label = ""): Bool {
		#if (krom_android || krom_ios)
		label = "";
		#end
		return ui.button(Config.buttonSpacing + text, Config.buttonAlign, label);
	}
}
