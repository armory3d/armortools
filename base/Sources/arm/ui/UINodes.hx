package arm.ui;

import haxe.Json;
import kha.Color;
import kha.Image;
import kha.System;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import zui.Ext;
import iron.system.Input;
import iron.system.Time;
import arm.shader.NodesMaterial;
import arm.logic.NodesBrush;
import arm.ui.UIHeader;
import arm.Project;
import arm.ProjectFormat;
#if (is_paint || is_sculpt)
import arm.util.RenderUtil;
import arm.shader.MakeMaterial;
#end

@:access(zui.Zui)
@:access(zui.Nodes)
class UINodes {

	public static var inst: UINodes;

	#if (is_paint || is_sculpt)
	public var show = false;
	#end
	#if is_lab
	public var show = true;
	#end

	public var wx: Int;
	public var wy: Int;
	public var ww: Int;
	public var wh: Int;

	public var ui: Zui;
	public var canvasType = CanvasMaterial;
	var showMenu = false;
	var showMenuFirst = true;
	var hideMenu = false;
	var menuCategory = 0;
	var popupX = 0.0;
	var popupY = 0.0;

	var uichangedLast = false;
	var recompileMat = false; // Mat preview
	var recompileMatFinal = false;
	var nodeSearchSpawn: TNode = null;
	var nodeSearchOffset = 0;
	var lastCanvas: TNodeCanvas = null;
	var lastNodeSelected: TNode = null;
	var releaseLink = false;
	var isNodeMenuOperation = false;

	public var grid: Image = null;
	public var hwnd = Id.handle();
	public var groupStack: Array<TNodeGroup> = [];
	public var controlsDown = false;

	public function new() {
		inst = this;

		#if (is_paint || is_sculpt)
		Nodes.excludeRemove.push("OUTPUT_MATERIAL_PBR");
		#end
		Nodes.excludeRemove.push("GROUP_OUTPUT");
		Nodes.excludeRemove.push("GROUP_INPUT");
		Nodes.excludeRemove.push("BrushOutputNode");
		Nodes.onLinkDrag = onLinkDrag;
		Nodes.onSocketReleased = onSocketReleased;
		Nodes.onCanvasReleased = onCanvasReleased;
		Nodes.onNodeRemove = onNodeRemove;
		Nodes.onCanvasControl = onCanvasControl;

		var scale = Config.raw.window_scale;
		ui = new Zui({ theme: App.theme, font: App.font, color_wheel: App.colorWheel, black_white_gradient: App.colorWheelGradient, scaleFactor: scale });
		ui.scrollEnabled = false;
	}

	function onLinkDrag(linkDrag: TNodeLink, isNewLink: Bool) {
		if (isNewLink) {
			var nodes = getNodes();
			var node = nodes.getNode(getCanvas(true).nodes, linkDrag.from_id > -1 ? linkDrag.from_id : linkDrag.to_id);
			var linkX = ui._windowX + nodes.NODE_X(node);
			var linkY = ui._windowY + nodes.NODE_Y(node);
			if (linkDrag.from_id > -1) {
				linkX += nodes.NODE_W(node);
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
						var fromType = node.outputs[linkDrag.from_socket].type;
						// Connect to the first socket
						linkDrag.to_socket = 0;
						// Try to find the first type-matching socket and use it if present
						for (socket in n.inputs) {
							if (socket.type == fromType) {
								linkDrag.to_socket = n.inputs.indexOf(socket);
								break;
							}
						}
						getCanvas(true).links.push(linkDrag);
					}
					else if (linkDrag.from_id == -1 && n.outputs.length > 0) {
						linkDrag.from_id = n.id;
						linkDrag.from_socket = 0;
						getCanvas(true).links.push(linkDrag);
					}
					#if is_lab
					arm.logic.LogicParser.parse(getCanvas(true), false);
					Context.raw.rdirty = 5;
					#end
				});
			}
			// Selecting which node socket to preview
			else if (node == nodes.nodesSelected[0]) {
				Context.raw.nodePreviewSocket = linkDrag.from_id > -1 ? linkDrag.from_socket : 0;
				#if (is_paint || is_sculpt)
				Context.raw.nodePreviewDirty = true;
				#end
			}
		}
	}

	function onSocketReleased(socket: TNodeSocket) {
		var nodes = getNodes();
		var canvas = getCanvas(true);
		var node = nodes.getNode(canvas.nodes, socket.node_id);
		if (ui.inputReleasedR) {
			if (node.type == "GROUP_INPUT" || node.type == "GROUP_OUTPUT") {
				App.notifyOnNextFrame(function() {
					UIMenu.draw(function(ui: Zui) {
						if (UIMenu.menuButton(ui, tr("Edit"))) {
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
										var min = Ext.floatInput(ui, hmin, tr("Min"));
										var max = Ext.floatInput(ui, hmax, tr("Max"));
										var default_value: Dynamic = null;
										if (type == 0) {
											ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
											Ext.floatInput(ui, hval0, tr("R"));
											Ext.floatInput(ui, hval1, tr("G"));
											Ext.floatInput(ui, hval2, tr("B"));
											Ext.floatInput(ui, hval3, tr("A"));
											default_value = [hval0.value, hval1.value, hval2.value, hval3.value];
										}
										else if (type == 1) {
											ui.row([1 / 3, 1 / 3, 1 / 3]);
											hval0.value = Ext.floatInput(ui, hval0, tr("X"));
											hval1.value = Ext.floatInput(ui, hval1, tr("Y"));
											hval2.value = Ext.floatInput(ui, hval2, tr("Z"));
											default_value = [hval0.value, hval1.value, hval2.value];
										}
										else {
											default_value = Ext.floatInput(ui, hval0, tr("default_value"));
										}
										if (ui.button(tr("OK"))) { // || ui.isReturnDown
											socket.name = name;
											socket.type = type == 0 ? "RGBA" : type == 1 ? "VECTOR" : "VALUE";
											socket.color = NodesMaterial.get_socket_color(socket.type);
											socket.min = min;
											socket.max = max;
											socket.default_value = default_value;
											UIBox.hide();
											NodesMaterial.syncSockets(node);
											hwnd.redraws = 2;
										}
									}
								}, 400, 250);
							});
						}
						if (UIMenu.menuButton(ui, tr("Delete"))) {
							var i = 0;
							// Remove links connected to the socket
							while (i < canvas.links.length) {
								var l = canvas.links[i];
								if ((l.from_id == node.id && l.from_socket == node.outputs.indexOf(socket)) ||
									(l.to_id == node.id && l.to_socket == node.inputs.indexOf(socket))) {
									canvas.links.splice(i, 1);
								}
								else i++;
							}
							// Remove socket
							node.inputs.remove(socket);
							node.outputs.remove(socket);
							NodesMaterial.syncSockets(node);
						}
					}, 2);
				});
			}
			else onCanvasReleased();
		}
		// Selecting which node socket to preview
		else if (node == nodes.nodesSelected[0]) {
			var i = node.outputs.indexOf(socket);
			if (i > -1) {
				Context.raw.nodePreviewSocket = i;
				#if (is_paint || is_sculpt)
				Context.raw.nodePreviewDirty = true;
				#end
			}
		}
	}

	function onCanvasReleased() {
		if (ui.inputReleasedR && Math.abs(ui.inputX - ui.inputStartedX) < 2 && Math.abs(ui.inputY - ui.inputStartedY) < 2) {
			// Node selection
			var nodes = getNodes();
			var canvas = getCanvas(true);
			var selected: TNode = null;
			for (node in canvas.nodes) {
				if (ui.getInputInRect(ui._windowX + nodes.NODE_X(node), ui._windowY + nodes.NODE_Y(node), nodes.NODE_W(node), nodes.NODE_H(canvas, node))) {
					selected = node;
					break;
				}
			}
			if (selected == null) nodes.nodesSelected = [];
			else if (nodes.nodesSelected.indexOf(selected) == -1) nodes.nodesSelected = [selected];

			// Node context menu
			if (!Nodes.socketReleased) {
				var numberOfEntries = 5;
				if (canvasType == CanvasMaterial) ++numberOfEntries;
				if (selected != null && selected.type == "RGB") ++numberOfEntries;
				
				UIMenu.draw(function(uiMenu: Zui) {
					uiMenu._y += 1;
					var protected = selected == null ||
									#if (is_paint || is_sculpt)
									selected.type == "OUTPUT_MATERIAL_PBR" ||
									#end
									selected.type == "GROUP_INPUT" ||
									selected.type == "GROUP_OUTPUT" ||
									selected.type == "BrushOutputNode";
					uiMenu.enabled = !protected;
					if (UIMenu.menuButton(uiMenu, tr("Cut"), "ctrl+x")) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							Zui.isCopy = true;
							Zui.isCut = true;
							isNodeMenuOperation = true;
						});
					}
					if (UIMenu.menuButton(uiMenu, tr("Copy"), "ctrl+c")) {
						App.notifyOnNextFrame(function() {
							Zui.isCopy = true;
							isNodeMenuOperation = true;
						});
					}
					uiMenu.enabled = Nodes.clipboard != "";
					if (UIMenu.menuButton(uiMenu, tr("Paste"), "ctrl+v")) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							Zui.isPaste = true;
							isNodeMenuOperation = true;
						});
					}
					uiMenu.enabled = !protected;
					if (UIMenu.menuButton(uiMenu, tr("Delete"), "delete")) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							ui.isDeleteDown = true;
							isNodeMenuOperation = true;
						});
					}
					if (UIMenu.menuButton(uiMenu, tr("Duplicate"))) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							Zui.isCopy = true;
							Zui.isPaste = true;
							isNodeMenuOperation = true;
						});
					}
					if (selected != null && selected.type == "RGB") {
						if (UIMenu.menuButton(uiMenu, tr("Add Swatch"))) {
							var color = selected.outputs[0].default_value;
							var newSwatch = Project.makeSwatch(Color.fromFloats(color[0], color[1], color[2], color[3]));
							Context.setSwatch(newSwatch);
							Project.raw.swatches.push(newSwatch);
							UIBase.inst.hwnds[TabStatus].redraws = 1;
						}
					}

					if (canvasType == CanvasMaterial) {
						UIMenu.menuSeparator(uiMenu);
						if (UIMenu.menuButton(uiMenu, tr("2D View"))) {
							UIBase.inst.show2DView(View2DNode);
						}
					}

					uiMenu.enabled = true;
				}, numberOfEntries);
			}
		}

		if (ui.inputReleased) {
			var nodes = getNodes();
			var canvas = getCanvas(true);
			for (node in canvas.nodes) {
				if (ui.getInputInRect(ui._windowX + nodes.NODE_X(node), ui._windowY + nodes.NODE_Y(node), nodes.NODE_W(node), nodes.NODE_H(canvas, node))) {
					if (node == nodes.nodesSelected[0]) {
						UIView2D.inst.hwnd.redraws = 2;
						if (Time.time() - Context.raw.selectTime < 0.25) UIBase.inst.show2DView(View2DNode);
						Context.raw.selectTime = Time.time();
					}
					break;
				}
			}
		}
	}

	public static function onNodeRemove(node: TNode) {
		// if (node.type == "GROUP") { // Remove unused groups
		// 	var found = false;
		// 	var canvases: Array<TNodeCanvas> = [];
		// 	for (m in Project.materials) canvases.push(m.canvas);
		// 	for (m in Project.materialGroups) canvases.push(m.canvas);
		// 	for (canvas in canvases) {
		// 		for (n in canvas.nodes) {
		// 			if (n.type == "GROUP" && n.name == node.name) {
		// 				found = true;
		// 				break;
		// 			}
		// 		}
		// 	}
		// 	if (!found) {
		// 		for (g in Project.materialGroups) {
		// 			if (g.canvas.name == node.name) {
		// 				Project.materialGroups.remove(g);
		// 				break;
		// 			}
		// 		}
		// 	}
		// }
	}

	function onCanvasControl(): zui.Nodes.CanvasControl {
		return getCanvasControl(ui, inst);
	}

	public static function getCanvasControl(ui: Zui, parent: Dynamic): zui.Nodes.CanvasControl {
		if (Config.raw.wrap_mouse && parent.controlsDown) {
			if (ui.inputX < ui._windowX) {
				@:privateAccess ui.inputX = ui._windowX + ui._windowW;
				Krom.setMousePosition(0, Std.int(ui.inputX), Std.int(ui.inputY));
			}
			else if (ui.inputX > ui._windowX + ui._windowW) {
				@:privateAccess ui.inputX = ui._windowX;
				Krom.setMousePosition(0, Std.int(ui.inputX), Std.int(ui.inputY));
			}
			else if (ui.inputY < ui._windowY) {
				@:privateAccess ui.inputY = ui._windowY + ui._windowH;
				Krom.setMousePosition(0, Std.int(ui.inputX), Std.int(ui.inputY));
			}
			else if (ui.inputY > ui._windowY + ui._windowH) {
				@:privateAccess ui.inputY = ui._windowY;
				Krom.setMousePosition(0, Std.int(ui.inputX), Std.int(ui.inputY));
			}
		}

		if (Operator.shortcut(Config.keymap.action_pan, ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutStarted) ||
			ui.inputStartedR ||
			ui.inputWheelDelta != 0.0) {
			parent.controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_pan, ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutDown) &&
			!ui.inputDownR &&
			ui.inputWheelDelta == 0.0) {
			parent.controlsDown = false;
		}
		if (!parent.controlsDown) {
			return {
				panX: 0,
				panY: 0,
				zoom: 0
			}
		}

		var pan = ui.inputDownR || Operator.shortcut(Config.keymap.action_pan, ShortcutDown);
		var zoomDelta = Operator.shortcut(Config.keymap.action_zoom, ShortcutDown) ? getZoomDelta(ui) / 100.0 : 0.0;
		var control = {
			panX: pan ? ui.inputDX : 0.0,
			panY: pan ? ui.inputDY : 0.0,
			zoom: ui.inputWheelDelta != 0.0 ? -ui.inputWheelDelta / 10 : zoomDelta
		};
		if (App.isComboSelected()) control.zoom = 0.0;
		return control;
	}

	static function getZoomDelta(ui: Zui): Float {
		return Config.raw.zoom_direction == ZoomVertical ? -ui.inputDY :
			   Config.raw.zoom_direction == ZoomVerticalInverted ? -ui.inputDY :
			   Config.raw.zoom_direction == ZoomHorizontal ? ui.inputDX :
			   Config.raw.zoom_direction == ZoomHorizontalInverted ? ui.inputDX :
			   -(ui.inputDY - ui.inputDX);
	}

	public function getCanvas(groups = false): TNodeCanvas {
		#if (is_paint || is_sculpt)
		if (canvasType == CanvasMaterial) {
			if (groups && groupStack.length > 0) return groupStack[groupStack.length - 1].canvas;
			else return getCanvasMaterial();
		}
		else return Context.raw.brush.canvas;
		#end

		#if is_lab
		return Project.canvas;
		#end
	}

	#if (is_paint || is_sculpt)
	public function getCanvasMaterial(): TNodeCanvas {
		return Context.raw.material.canvas;
	}
	#end

	public function getNodes(): Nodes {
		#if (is_paint || is_sculpt)
		if (canvasType == CanvasMaterial) {
			if (groupStack.length > 0) return groupStack[groupStack.length - 1].nodes;
			else return Context.raw.material.nodes;
		}
		else return Context.raw.brush.nodes;
		#end

		#if is_lab
		if (groupStack.length > 0) return groupStack[groupStack.length - 1].nodes;
		else return Project.nodes;
		#end
	}

	public function update() {
		if (!show || !App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		#if (is_paint || is_sculpt)
		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		#end
		#if is_lab
		wx = Std.int(iron.App.w());
		#end
		wy = UIHeader.headerh * 2;

		if (UIView2D.inst.show) {
			wy += iron.App.h() - Config.raw.layout[LayoutNodesH];
		}

		var ww = Config.raw.layout[LayoutNodesW];
		if (!UIBase.inst.show) {
			#if (is_paint || is_sculpt)
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
			wx -= UIToolbar.inst.toolbarw;
			#end
			wy = 0;
		}

		var mx = mouse.x;
		var my = mouse.y;
		if (mx < wx || mx > wx + ww || my < wy) return;
		if (ui.isTyping || !ui.inputEnabled) return;

		var nodes = getNodes();
		if (nodes.nodesSelected.length > 0 && ui.isKeyPressed) {
			if (ui.key == kha.input.KeyCode.Left) for (n in nodes.nodesSelected) n.x -= 1;
			else if (ui.key == kha.input.KeyCode.Right) for (n in nodes.nodesSelected) n.x += 1;
			if (ui.key == kha.input.KeyCode.Up) for (n in nodes.nodesSelected) n.y -= 1;
			else if (ui.key == kha.input.KeyCode.Down) for (n in nodes.nodesSelected) n.y += 1;
		}

		// Node search popup
		if (Operator.shortcut(Config.keymap.node_search)) nodeSearch();
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
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.drawRect(ui.g, true, ui._x, ui._y, ui._w, ui.ELEMENT_H() * 8);
			ui.g.color = 0xffffffff;

			var search = ui.textInput(searchHandle, "", Left, true, true).toLowerCase();
			ui.changed = false;
			if (first) {
				first = false;
				searchHandle.text = "";
				ui.startTextEdit(searchHandle); // Focus search bar
			}

			if (searchHandle.changed) nodeSearchOffset = 0;
			
			if (ui.isKeyPressed) { // Move selection
				if (ui.key == kha.input.KeyCode.Down && nodeSearchOffset < 6) nodeSearchOffset++;
				if (ui.key == kha.input.KeyCode.Up && nodeSearchOffset > 0) nodeSearchOffset--;
			}
			var enter = kb.down("enter");
			var count = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;

			#if (is_paint || is_sculpt)
			var nodeList = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			#end
			#if is_lab
			var nodeList = NodesBrush.list;
			#end

			for (list in nodeList) {
				for (n in list) {
					if (tr(n.name).toLowerCase().indexOf(search) >= 0) {
						ui.t.BUTTON_COL = count == nodeSearchOffset ? ui.t.HIGHLIGHT_COL : ui.t.SEPARATOR_COL;
						if (ui.button(tr(n.name), Left) || (enter && count == nodeSearchOffset)) {
							pushUndo();
							var nodes = getNodes();
							var canvas = getCanvas(true);
							nodeSearchSpawn = makeNode(n, nodes, canvas); // Spawn selected node
							canvas.nodes.push(nodeSearchSpawn);
							nodes.nodesSelected = [nodeSearchSpawn];
							nodes.nodesDrag = true;

							#if is_lab
							arm.logic.LogicParser.parse(canvas, false);
							#end

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
		}, 8, x, y);
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

		#if (is_paint || is_sculpt)
		if (!UIBase.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
		}
		#end

		var wh = iron.App.h();
		var step = 100 * ui.SCALE();
		var w = Std.int(ww + step * 3);
		var h = Std.int(wh + step * 3);
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		grid = Image.createRenderTarget(w, h);
		grid.g2.begin(true, ui.t.SEPARATOR_COL);

		grid.g2.color = ui.t.SEPARATOR_COL - 0x00050505;
		var step = 20 * ui.SCALE();
		for (i in 0...Std.int(h / step) + 1) {
			grid.g2.drawLine(0, i * step, w, i * step);
		}
		for (i in 0...Std.int(w / step) + 1) {
			grid.g2.drawLine(i * step, 0, i * step, h);
		}

		grid.g2.color = ui.t.SEPARATOR_COL - 0x00090909;
		var step = 100 * ui.SCALE();
		for (i in 0...Std.int(h / step) + 1) {
			grid.g2.drawLine(0, i * step, w, i * step);
		}
		for (i in 0...Std.int(w / step) + 1) {
			grid.g2.drawLine(i * step, 0, i * step, h);
		}

		grid.g2.end();
	}

	public function render(g: kha.graphics2.Graphics) {
		if (recompileMat) {
			#if (is_paint || is_sculpt)
			if (canvasType == CanvasBrush) {
				MakeMaterial.parseBrush();
				RenderUtil.makeBrushPreview();
				UIBase.inst.hwnds[TabSidebar1].redraws = 2;
			}
			else {
				App.isFillMaterial() ? App.updateFillLayers() : RenderUtil.makeMaterialPreview();
				if (UIView2D.inst.show && UIView2D.inst.type == View2DNode) {
					UIView2D.inst.hwnd.redraws = 2;
				}
			}

			UIBase.inst.hwnds[TabSidebar1].redraws = 2;
			if (Context.raw.splitView) Context.raw.ddirty = 2;
			#end

			#if is_lab
			arm.logic.LogicParser.parse(Project.canvas, false);
			#end

			recompileMat = false;
		}
		else if (recompileMatFinal) {
			#if (is_paint || is_sculpt)
			MakeMaterial.parsePaintMaterial();

			if (canvasType == CanvasMaterial && App.isFillMaterial()) {
				App.updateFillLayers();
				RenderUtil.makeMaterialPreview();
			}

			var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
			if (decal) RenderUtil.makeDecalPreview();

			UIBase.inst.hwnds[TabSidebar0].redraws = 2;
			Context.raw.nodePreviewDirty = true;
			#end

			recompileMatFinal = false;
		}

		var nodes = getNodes();
		if (nodes.nodesSelected.length > 0 && nodes.nodesSelected[0] != lastNodeSelected) {
			lastNodeSelected = nodes.nodesSelected[0];
			#if (is_paint || is_sculpt)
			Context.raw.nodePreviewDirty = true;
			#end

			#if is_lab
			Context.raw.ddirty = 2; // Show selected node texture in viewport
			UIHeader.inst.headerHandle.redraws = 2;
			#end

			Context.raw.nodePreviewSocket = 0;
		}

		// Remove dragged link when mouse is released out of the node viewport
		var c = getCanvas(true);
		if (releaseLink && nodes.linkDrag != null) {
			c.links.remove(nodes.linkDrag);
			nodes.linkDrag = null;
		}
		releaseLink = ui.inputReleased;

		if (!show || System.windowWidth() == 0 || System.windowHeight() == 0) return;

		ui.inputEnabled = App.uiEnabled;

		g.end();

		if (grid == null) drawGrid();

		#if (is_paint || is_sculpt)
		if (Config.raw.node_preview && Context.raw.nodePreviewDirty) {
			makeNodePreview();
		}
		#end

		// Start with UI
		ui.begin(g);

		// Make window
		ww = Config.raw.layout[LayoutNodesW];

		#if (is_paint || is_sculpt)
		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		#end
		#if is_lab
		wx = Std.int(iron.App.w());
		#end

		wy = 0;

		#if (is_paint || is_sculpt)
		if (!UIBase.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
			wx -= UIToolbar.inst.toolbarw;
		}
		#end

		var ew = Std.int(ui.ELEMENT_W() * 0.7);
		wh = iron.App.h() + UIHeader.headerh;
		if (Config.raw.layout[LayoutHeader] == 1) wh += UIHeader.headerh;

		if (UIView2D.inst.show) {
			wh = Config.raw.layout[LayoutNodesH];
			wy = iron.App.h() - Config.raw.layout[LayoutNodesH] + UIHeader.headerh;
			if (Config.raw.layout[LayoutHeader] == 1) wy += UIHeader.headerh;
			if (!UIBase.inst.show) {
				wy -= UIHeader.headerh * 2;
			}
		}

		if (ui.window(hwnd, wx, wy, ww, wh)) {

			ui.tab(Id.handle(), tr("Nodes"));

			// Grid
			ui.g.color = 0xffffffff;
			var step = 100 * ui.SCALE();
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE()) % step - step, (nodes.panY * nodes.SCALE()) % step - step);

			// Undo
			if (ui.inputStarted || ui.isKeyPressed) {
				lastCanvas = Json.parse(Json.stringify(getCanvas(true)));
			}

			// Nodes
			var _inputEnabled = ui.inputEnabled;
			ui.inputEnabled = _inputEnabled && !showMenu;
			#if (is_paint || is_sculpt)
			ui.windowBorderRight = Config.raw.layout[LayoutSidebarW];
			#end
			ui.windowBorderTop = UIHeader.headerh * 2;
			ui.windowBorderBottom = Config.raw.layout[LayoutStatusH];
			nodes.nodeCanvas(ui, c);
			ui.inputEnabled = _inputEnabled;

			if (nodes.colorPickerCallback != null) {
				Context.raw.colorPickerPreviousTool = Context.raw.tool;
				Context.selectTool(ToolPicker);
				var tmp = nodes.colorPickerCallback;
				Context.raw.colorPickerCallback = function(color: TSwatchColor) {
					tmp(color.base);
					UINodes.inst.hwnd.redraws = 2;

					#if (is_paint || is_sculpt)
					var material_live = Config.raw.material_live;
					#end
					#if is_lab
					var material_live = true;
					#end

					if (material_live) {
						UINodes.inst.canvasChanged();
					}
				};
				nodes.colorPickerCallback = null;
			}

			// Remove nodes with unknown id for this canvas type
			if (Zui.isPaste) {
				#if (is_paint || is_sculpt)
				var nodeList = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
				#end
				#if is_lab
				var nodeList = NodesBrush.list;
				#end

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
					if (canvasNode.type == "GROUP" && !canPlaceGroup(canvasNode.name)) {
						found = false;
					}
					if (!found) {
						nodes.removeNode(canvasNode, c);
						nodes.nodesSelected.remove(canvasNode);
						i--;
					}
				}
			}

			if (isNodeMenuOperation) {
				Zui.isCopy = Zui.isCut = Zui.isPaste = ui.isDeleteDown = false;
			}

			// Recompile material on change
			if (ui.changed) {
				#if (is_paint || is_sculpt)
				recompileMat = (ui.inputDX != 0 || ui.inputDY != 0 || !uichangedLast) && Config.raw.material_live; // Instant preview
				#end
				#if is_lab
				recompileMat = (ui.inputDX != 0 || ui.inputDY != 0 || !uichangedLast); // Instant preview
				#end
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

				#if (is_paint || is_sculpt)

				var singleChannel = sel.type == "LAYER_MASK";
				if (sel.type == "LAYER" || sel.type == "LAYER_MASK") {
					var id = sel.buttons[0].default_value;
					if (id < Project.layers.length) {
						#if is_paint
						img = Project.layers[id].texpaint_preview;
						#end
					}
				}
				else if (sel.type == "MATERIAL") {
					var id = sel.buttons[0].default_value;
					if (id < Project.materials.length) {
						img = Project.materials[id].image;
					}
				}
				else if (sel.type == "OUTPUT_MATERIAL_PBR") {
					img = Context.raw.material.image;
				}
				else if (sel.type == "BrushOutputNode") {
					img = Context.raw.brush.image;
				}
				else if (canvasType == CanvasMaterial) {
					img = Context.raw.nodePreview;
				}

				#else

				var brushNode = arm.logic.LogicParser.getLogicNode(sel);
				if (brushNode != null) {
					img = brushNode.getCachedImage();
				}

				#end

				if (img != null) {
					var tw = 128 * ui.SCALE();
					var th = tw * (img.height / img.width);
					var tx = ww - tw - 8 * ui.SCALE();
					var ty = wh - th - 8 * ui.SCALE();

					#if kha_opengl
					var invertY = sel.type == "MATERIAL";
					#else
					var invertY = false;
					#end

					#if (is_paint || is_sculpt)
					if (singleChannel) {
						ui.g.pipeline = UIView2D.pipe;
						#if kha_opengl
						ui.currentWindow.texture.g4.setPipeline(UIView2D.pipe);
						#end
						ui.currentWindow.texture.g4.setInt(UIView2D.channelLocation, 1);
					}
					#end

					ui.g.color = 0xffffffff;
					invertY ?
						ui.g.drawScaledImage(img, tx, ty + th, tw, -th) :
						ui.g.drawScaledImage(img, tx, ty, tw, th);

					#if (is_paint || is_sculpt)
					if  (singleChannel) {
						ui.g.pipeline = null;
					}
					#end
				}
			}

			// Menu
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, ui.ELEMENT_H(), ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET() * 2);
			ui.g.color = 0xffffffff;

			var startY = ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
			ui._x = 0;
			ui._y = 2 + startY;
			ui._w = ew;

			#if (is_paint || is_sculpt)
			// Editable canvas name
			var h = Id.handle();
			h.text = c.name;
			ui._w = Std.int(Math.min(ui.ops.font.width(ui.fontSize, h.text) + 15 * ui.SCALE(), 100 * ui.SCALE()));
			var newName = ui.textInput(h, "");
			ui._x += ui._w + 3;
			ui._y = 2 + startY;
			ui._w = ew;

			if (h.changed) { // Check whether renaming is possible and update group links
				if (groupStack.length > 0) {
					var canRename = true;
					for (m in Project.materialGroups) {
						if (m.canvas.name == newName) canRename = false; // Name already used
					}

					if (canRename) {
						var oldName = c.name;
						c.name = newName;
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
				}
				else {
					c.name = newName;
				}
			}
			#end

			#if is_lab
			ui.windowBorderTop = 0;
			UINodesExt.drawButtons(ew, startY);
			#end

			var _BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

			#if (is_paint || is_sculpt)
			var cats = canvasType == CanvasMaterial ? NodesMaterial.categories : NodesBrush.categories;
			#end
			#if is_lab
			var cats = NodesBrush.categories;
			#end

			for (i in 0...cats.length) {
				if ((Ext.menuButton(ui, tr(cats[i]))) || (ui.isHovered && showMenu)) {
					showMenu = true;
					menuCategory = i;
					popupX = wx + ui._x;
					popupY = wy + ui._y;
					if (Config.raw.touch_ui) {
						showMenuFirst = true;
						var menuw = Std.int(ew * 2.3);
						popupX -= menuw / 2;
						popupX += ui._w / 2;
					}
					UIMenu.menuCategoryW = ui._w;
					UIMenu.menuCategoryH = Std.int(Ext.MENUBAR_H(ui));
				}
				ui._x += ui._w + 3;
				ui._y = 2 + startY;
			}

			if (Config.raw.touch_ui) {
				var _w = ui._w;
				ui._w = Std.int(36 * ui.SCALE());
				ui._y = 4 * ui.SCALE() + startY;
				if (UIMenubar.iconButton(ui, 2, 3)) {
					nodeSearch(Std.int(ui._windowX + ui._x), Std.int(ui._windowY + ui._y));
				}
				ui._w = _w;
			}
			else {
				if (Ext.menuButton(ui, tr("Search"))) {
					nodeSearch(Std.int(ui._windowX + ui._x), Std.int(ui._windowY + ui._y));
				}
			}
			if (ui.isHovered) {
				ui.tooltip(tr("Search for nodes") + ' (${Config.keymap.node_search})');
			}
			ui._x += ui._w + 3;
			ui._y = 2 + startY;

			ui.t.BUTTON_COL = _BUTTON_COL;

			// Close node group
			if (groupStack.length > 0 && Ext.menuButton(ui, tr("Close"))) {
				groupStack.pop();
			}
		}

		ui.end(!showMenu);

		g.begin(false);

		if (showMenu) {
			#if (is_paint || is_sculpt)
			var list = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			#end
			#if is_lab
			var list = NodesBrush.list;
			#end

			var numNodes = list[menuCategory].length;

			#if (is_paint || is_sculpt)
			var isGroupCategory = canvasType == CanvasMaterial && NodesMaterial.categories[menuCategory] == "Group";
			#end
			#if is_lab
			var isGroupCategory = NodesMaterial.categories[menuCategory] == "Group";
			#end

			if (isGroupCategory) numNodes += Project.materialGroups.length;

			var py = popupY;
			var menuw = Std.int(ew * 2.3);
			ui.beginRegion(g, Std.int(popupX), Std.int(py), menuw);
			var _BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;
			var _ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var _ELEMENT_H = ui.t.ELEMENT_H;
			ui.t.ELEMENT_H = Config.raw.touch_ui ? (28 + 2) : 28;

			UIMenu.menuStart(ui);

			for (n in list[menuCategory]) {
				if (UIMenu.menuButton(ui, tr(n.name))) {
					pushUndo();
					var canvas = getCanvas(true);
					var nodes = getNodes();
					var node = makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					nodes.nodesSelected = [node];
					nodes.nodesDrag = true;
					#if is_lab
					arm.logic.LogicParser.parse(canvas, false);
					#end
				}
				// Next column
				if (ui._y - wy + ui.ELEMENT_H() / 2 > wh) {
					ui._x += menuw;
					ui._y = py;
				}
			}
			if (isGroupCategory) {
				for (g in Project.materialGroups) {
					ui.fill(0, 1, ui._w / ui.SCALE(), ui.t.BUTTON_H + 2, ui.t.ACCENT_SELECT_COL);
					ui.fill(1, 1, ui._w / ui.SCALE() - 2, ui.t.BUTTON_H + 1, ui.t.SEPARATOR_COL);
					ui.enabled = canPlaceGroup(g.canvas.name);
					UIMenu.menuFill(ui);
					ui.row([5 / 6, 1 / 6]);
					if (ui.button(Config.buttonSpacing + g.canvas.name, Left)) {
						pushUndo();
						var canvas = getCanvas(true);
						var nodes = getNodes();
						var node = makeGroupNode(g.canvas, nodes, canvas);
						canvas.nodes.push(node);
						nodes.nodesSelected = [node];
						nodes.nodesDrag = true;
					}

					#if (is_paint || is_sculpt)
					ui.enabled = !Project.isMaterialGroupInUse(g);
					if (ui.button("x", Center)) {
						History.deleteMaterialGroup(g);
						Project.materialGroups.remove(g);
					}
					#end
					
					ui.enabled = true;
				}
			}

			hideMenu = ui.comboSelectedHandle == null && !showMenuFirst && (ui.changed || ui.inputReleased || ui.inputReleasedR || ui.isEscapeDown);
			showMenuFirst = false;

			ui.t.BUTTON_COL = _BUTTON_COL;
			ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
			ui.t.ELEMENT_H = _ELEMENT_H;
			ui.endRegion();
		}

		if (hideMenu) {
			showMenu = false;
			showMenuFirst = true;
		}
	}

	function containsNodeGroupRecursive(group: TNodeGroup, groupName: String): Bool {
		if (group.canvas.name == groupName) {
			return true;
		}
		for (n in group.canvas.nodes) {
			if (n.type == "GROUP") {
				var g = Project.getMaterialGroupByName(n.name);
				if (g != null && containsNodeGroupRecursive(g, groupName)) {
					return true;
				}
			}
		}
		return false;
	}

	function canPlaceGroup(groupName: String): Bool {
		// Prevent Recursive node groups 
		// The group to place must not contain the current group or a group that contains the current group
		if (groupStack.length > 0) {
			for (g in groupStack) { 
				if (containsNodeGroupRecursive(Project.getMaterialGroupByName(groupName), g.canvas.name)) return false;
			}
		}
		// Group was deleted / renamed
		var groupExists = false;
		for (group in Project.materialGroups) {
			if (groupName == group.canvas.name) {
				groupExists = true;
			}
		}
		if (!groupExists) return false;
		return true;
	}

	function pushUndo(lastCanvas: TNodeCanvas = null) {
		if (lastCanvas == null) lastCanvas = getCanvas(true);
		var canvasGroup = groupStack.length > 0 ? Project.materialGroups.indexOf(groupStack[groupStack.length - 1]) : null;

		#if (is_paint || is_sculpt)
		UIBase.inst.hwnds[TabSidebar0].redraws = 2;
		History.editNodes(lastCanvas, canvasType, canvasGroup);
		#end
		#if is_lab
		History.editNodes(lastCanvas, canvasGroup);
		#end
	}

	public function acceptAssetDrag(index: Int) {
		pushUndo();
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		#if (is_paint || is_sculpt)
		var n = canvasType == CanvasMaterial ? NodesMaterial.createNode("TEX_IMAGE", g) : NodesBrush.createNode("TEX_IMAGE");
		#end
		#if is_lab
		var n = NodesBrush.createNode("ImageTextureNode");
		#end

		n.buttons[0].default_value = index;
		getNodes().nodesSelected = [n];

		#if is_lab
		arm.logic.LogicParser.parse(Project.canvas, false);
		#end
	}

	#if (is_paint || is_sculpt)
	public function acceptLayerDrag(index: Int) {
		pushUndo();
		if (Project.layers[index].isGroup()) return;
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		var n = NodesMaterial.createNode(Context.raw.layer.isMask() ? "LAYER_MASK" : "LAYER", g);
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
	#end

	public function acceptSwatchDrag(swatch: TSwatchColor) {
		#if (is_paint || is_sculpt)
		pushUndo();
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		var n = NodesMaterial.createNode("RGB", g);
		n.outputs[0].default_value = [swatch.base.R, swatch.base.G, swatch.base.B, swatch.base.A];
		getNodes().nodesSelected = [n];
		#end
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

	#if (is_paint || is_sculpt)
	function makeNodePreview() {
		var nodes = Context.raw.material.nodes;
		if (nodes.nodesSelected.length == 0) return;

		var node = nodes.nodesSelected[0];
		Context.raw.nodePreviewName = node.name;

		if (node.type == "LAYER" ||
			node.type == "LAYER_MASK" ||
			node.type == "MATERIAL" ||
			node.type == "OUTPUT_MATERIAL_PBR") return;

		if (Context.raw.material.canvas.nodes.indexOf(node) == -1) return;

		if (Context.raw.nodePreview == null) {
			Context.raw.nodePreview = kha.Image.createRenderTarget(RenderUtil.materialPreviewSize, RenderUtil.materialPreviewSize);
		}

		Context.raw.nodePreviewDirty = false;
		UINodes.inst.hwnd.redraws = 2;
		RenderUtil.makeNodePreview(Context.raw.material.canvas, node, Context.raw.nodePreview);
	}
	#end

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
}
