package arm.ui;

import haxe.Json;
import kha.Image;
import kha.System;
import kha.Color;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.system.Input;
import arm.shader.NodesMaterial;
import arm.shader.MaterialParser;
import arm.node.NodesBrush;
import arm.node.MakeMaterial;
import arm.ui.UIHeader;
import arm.Enums;
import arm.Project;
import arm.ProjectFormat.TSwatchColor;

@:access(zui.Zui)
@:access(zui.Nodes)
class UINodes {

	public static var inst: UINodes;

	public var show = true;
	public var wx: Int;
	public var wy: Int;
	public var ww: Int;
	public var wh: Int;

	public var ui: Zui;
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

		Nodes.excludeRemove.push("GROUP_OUTPUT");
		Nodes.excludeRemove.push("GROUP_INPUT");
		Nodes.excludeRemove.push("BrushOutputNode");
		Nodes.onLinkDrag = onLinkDrag;
		Nodes.onSocketReleased = onSocketReleased;
		Nodes.onCanvasReleased = onCanvasReleased;
		Nodes.onNodeRemove = onNodeRemove;
		Nodes.onCanvasControl = onCanvasControl;

		var scale = Config.raw.window_scale;
		ui = new Zui({ theme: App.theme, font: App.font, color_wheel: App.colorWheel, black_white_gradient: App.blackWhiteGradient, scaleFactor: scale });
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
						// 1. step: Connect to the first socket.
						linkDrag.to_socket = 0;
						// 2. step: Try to find the first type-matching socket and use it if present.
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
				});
			}
			// Selecting which node socket to preview
			else if (node == nodes.nodesSelected[0]) {
				Context.nodePreviewSocket = linkDrag.from_id > -1 ? linkDrag.from_socket : 0;
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
											UIBox.hide();
											NodesMaterial.syncSockets(node);
											hwnd.redraws = 2;
										}
									}
								}, 400, 250);
							});
						}
						if (ui.button(tr("Delete"), Left)) {
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
					}, 3);
				});
			}
			else onCanvasReleased();
		}
		// Selecting which node socket to preview
		else if (node == nodes.nodesSelected[0]) {
			var i = node.outputs.indexOf(socket);
			if (i > -1) {
				Context.nodePreviewSocket = i;
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
				if (selected != null && selected.type == "RGB") ++numberOfEntries;

				UIMenu.draw(function(uiMenu: Zui) {
					uiMenu._y += 1;
					var protected = selected == null ||
									selected.type == "GROUP_INPUT" ||
									selected.type == "GROUP_OUTPUT" ||
									selected.type == "BrushOutputNode";
					uiMenu.enabled = !protected;
					if (menuButton(uiMenu, tr("Cut"), "ctrl+x")) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							Zui.isCopy = true;
							Zui.isCut = true;
							isNodeMenuOperation = true;
						});
					}
					if (menuButton(uiMenu, tr("Copy"), "ctrl+c")) {
						App.notifyOnNextFrame(function() {
							Zui.isCopy = true;
							isNodeMenuOperation = true;
						});
					}
					uiMenu.enabled = Nodes.clipboard != "";
					if (menuButton(uiMenu, tr("Paste"), "ctrl+v")) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							Zui.isPaste = true;
							isNodeMenuOperation = true;
						});
					}
					uiMenu.enabled = !protected;
					if (menuButton(uiMenu, tr("Delete"), "delete")) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							ui.isDeleteDown = true;
							isNodeMenuOperation = true;
						});
					}
					if (menuButton(uiMenu, tr("Duplicate"))) {
						App.notifyOnNextFrame(function() {
							hwnd.redraws = 2;
							Zui.isCopy = true;
							Zui.isPaste = true;
							isNodeMenuOperation = true;
						});
					}
					if (selected != null && selected.type == "RGB") {
						if (menuButton(uiMenu, tr("Add Swatch"))) {
							var color = selected.outputs[0].default_value;
							var newSwatch = Project.makeSwatch(Color.fromFloats(color[0], color[1], color[2], color[3]));
							Context.setSwatch(newSwatch);
							Project.raw.swatches.push(newSwatch);
							UIStatus.inst.statusHandle.redraws = 1;
						}
					}
					uiMenu.enabled = true;
				}, numberOfEntries);
			}
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
		return Project.canvas;
	}

	public function getNodes(): Nodes {
		if (groupStack.length > 0) return groupStack[groupStack.length - 1].nodes;
		else return Project.nodes;
	}

	public function update() {
		if (!show || !App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		wx = Std.int(iron.App.w());
		wy = UIHeader.inst.headerh * 2;
		var ww = Config.raw.layout[LayoutNodesW];
		if (!UISidebar.inst.show) {
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
			ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 8, ui.t.SEPARATOR_COL);
			var search = ui.textInput(searchHandle, "", Left, true, true);
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
			var nodeList = NodesBrush.list;
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
							arm.node.Brush.parse(canvas, false);
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
		var wh = iron.App.h();
		var w = ww + 100 * 2;
		var h = wh + 100 * 2;
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		grid = Image.createRenderTarget(w, h);
		grid.g2.begin(true, ui.t.SEPARATOR_COL);

		grid.g2.color = ui.t.SEPARATOR_COL - 0x00050505;
		var step = 20; // * ui.SCALE();
		for (i in 0...Std.int(h / step) + 1) {
			grid.g2.drawLine(0, i * step, w, i * step);
		}
		for (i in 0...Std.int(w / step) + 1) {
			grid.g2.drawLine(i * step, 0, i * step, h);
		}

		grid.g2.color = ui.t.SEPARATOR_COL - 0x00090909;
		var step = 100; // * ui.SCALE();
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
			arm.node.Brush.parse(Project.canvas, false);
			recompileMat = false;
		}
		else if (recompileMatFinal) {
			recompileMatFinal = false;
		}

		var nodes = getNodes();
		if (nodes.nodesSelected.length > 0 && nodes.nodesSelected[0] != lastNodeSelected) {
			lastNodeSelected = nodes.nodesSelected[0];
			Context.nodePreviewSocket = 0;
			Context.ddirty = 2; // Show selected node texture in viewport
			UIHeader.inst.headerHandle.redraws = 2;
		}
		else if (lastNodeSelected != null) {
			lastNodeSelected = null;
			Context.ddirty = 2;
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

		// Start with UI
		ui.begin(g);

		// Make window
		ww = Config.raw.layout[LayoutNodesW];
		wx = Std.int(iron.App.w());
		wy = UIHeader.inst.headerh * 2;
		if (!UISidebar.inst.show) {
			wy = 0;
		}
		var ew = Std.int(ui.ELEMENT_W() * 0.7);
		wh = iron.App.h();
		if (ui.window(hwnd, wx, wy, ww, wh)) {

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE()) % 100 - 100, (nodes.panY * nodes.SCALE()) % 100 - 100);

			// Undo
			if (ui.inputStarted || ui.isKeyPressed) {
				lastCanvas = Json.parse(Json.stringify(getCanvas(true)));
			}

			// Nodes
			var _inputEnabled = ui.inputEnabled;
			ui.inputEnabled = _inputEnabled && !showMenu;
			ui.windowBorderTop = UIHeader.inst.headerh * 2;
			ui.windowBorderBottom = Config.raw.layout[LayoutStatusH];
			nodes.nodeCanvas(ui, c);
			ui.inputEnabled = _inputEnabled;
			if (isNodeMenuOperation) {
				Zui.isCopy = Zui.isCut = Zui.isPaste = ui.isDeleteDown = false;
			}

			if (nodes.colorPickerCallback != null) {
				Context.colorPickerPreviousTool = Context.tool;
				Context.selectTool(ToolPicker);
				var tmp = nodes.colorPickerCallback;
				Context.colorPickerCallback = function(color: TSwatchColor) {
					tmp(color.base);
					UINodes.inst.hwnd.redraws = 2;
					UINodes.inst.canvasChanged();
				};
				nodes.colorPickerCallback = null;
			}

			// Remove nodes with unknown id for this canvas type
			if (Zui.isPaste) {
				var nodeList = NodesBrush.list;
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

			// Recompile material on change
			if (ui.changed) {
				recompileMat = (ui.inputDX != 0 || ui.inputDY != 0 || !uichangedLast); // Instant preview
			}
			else if (uichangedLast) {
				canvasChanged();
				pushUndo(lastCanvas);
			}
			uichangedLast = ui.changed;

			// Close node group
			if (groupStack.length > 0) {
				ui._x = 5;
				ui._y = wh - ui.ELEMENT_H() * 1.2;
				ui._w = Std.int(ui.ELEMENT_W() * 1.4);
				if (ui.button(tr("Close"))) groupStack.pop();
			}

			// Menu
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, 0, ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET());
			ui.g.color = 0xffffffff;

			ui._x = 0;
			ui._y = 0;
			ui._w = ew;

			if (ui.button(tr("Run"))) {
				Console.toast(tr("Processing"));
				iron.App.notifyOnInit(function() {
					#if arm_debug
					var timer = iron.system.Time.realTime();
					#end

					arm.node.Brush.parse(Project.canvas, false);

					arm.node.brush.PhotoToPBRNode.cachedSource = null;
					@:privateAccess arm.node.brush.BrushOutputNode.inst.get(ChannelBaseColor, function(texbase) {
					@:privateAccess arm.node.brush.BrushOutputNode.inst.get(ChannelOcclusion, function(texocc) {
					@:privateAccess arm.node.brush.BrushOutputNode.inst.get(ChannelRoughness, function(texrough) {
					@:privateAccess arm.node.brush.BrushOutputNode.inst.get(ChannelNormalMap, function(texnor) {
					@:privateAccess arm.node.brush.BrushOutputNode.inst.get(ChannelHeight, function(texheight) {

						if (texbase != null) {
							var texpaint = iron.RenderPath.active.renderTargets.get("texpaint").image;
							texpaint.g2.begin(false);
							texpaint.g2.drawScaledImage(texbase, 0, 0, Config.getTextureResX(), Config.getTextureResY());
							texpaint.g2.end();
						}

						if (texnor != null) {
							var texpaint_nor = iron.RenderPath.active.renderTargets.get("texpaint_nor").image;
							texpaint_nor.g2.begin(false);
							texpaint_nor.g2.drawScaledImage(texnor, 0, 0, Config.getTextureResX(), Config.getTextureResY());
							texpaint_nor.g2.end();
						}

						if (Layers.pipeCopy == null) Layers.makePipe();
						if (Layers.pipeCopyA == null) Layers.makePipeCopyA();
						if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

						var texpaint_pack = iron.RenderPath.active.renderTargets.get("texpaint_pack").image;

						if (texocc != null) {
							texpaint_pack.g2.begin(false);
							texpaint_pack.g2.pipeline = Layers.pipeCopyR;
							texpaint_pack.g2.drawScaledImage(texocc, 0, 0, Config.getTextureResX(), Config.getTextureResY());
							texpaint_pack.g2.pipeline = null;
							texpaint_pack.g2.end();
						}

						if (texrough != null) {
							texpaint_pack.g2.begin(false);
							texpaint_pack.g2.pipeline = Layers.pipeCopyG;
							texpaint_pack.g2.drawScaledImage(texrough, 0, 0, Config.getTextureResX(), Config.getTextureResY());
							texpaint_pack.g2.pipeline = null;
							texpaint_pack.g2.end();
						}

						if (texheight != null) {
							texpaint_pack.g4.begin();
							texpaint_pack.g4.setPipeline(Layers.pipeCopyA);
							texpaint_pack.g4.setTexture(Layers.pipeCopyATex, texheight);
							texpaint_pack.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
							texpaint_pack.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
							texpaint_pack.g4.drawIndexedVertices();
							texpaint_pack.g4.end();

							// arm.util.MeshUtil.applyDisplacement();
							// arm.util.MeshUtil.calcNormals();
						}

						Context.ddirty = 2;

						#if (kha_direct3d12 || kha_vulkan)
						arm.render.RenderPathRaytrace.ready = false;
						#end

						#if arm_debug
						trace("Model run in " + (iron.system.Time.realTime() - timer));
						#end
					});
					});
					});
					});
					});
				});
			}
			ui._x += ew + 3;
			ui._y = 0;

			#if (krom_android || krom_ios)
			ui.combo(App.resHandle, ["2K", "4K"], tr("Resolution"));
			#else
			ui.combo(App.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
			#end
			if (App.resHandle.changed) {
				Layers.onLayersResized();
			}
			ui._x += ew + 3;
			ui._y = 0;

			var _BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

			var cats = NodesBrush.categories;
			for (i in 0...cats.length) {
				if ((ui.button(tr(cats[i]), Left)) || (ui.isHovered && showMenu)) {
					showMenu = true;
					menuCategory = i;
					popupX = wx + ui._x;
					popupY = wy + ui._y;
					if (Config.raw.touch_ui) {
						showMenuFirst = true;
					}
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

		ui.end(!showMenu);

		g.begin(false);

		if (showMenu) {
			var list = NodesBrush.list;
			var numNodes = list[menuCategory].length;

			var isGroupCategory = NodesMaterial.categories[menuCategory] == "Group";
			if (isGroupCategory) numNodes += Project.materialGroups.length;

			var py = popupY;
			var menuw = Std.int(ew * 2.0);
			ui.beginRegion(g, Std.int(popupX), Std.int(py), menuw);
			var _BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;
			var _BUTTON_H = ui.t.BUTTON_H;
			ui.t.BUTTON_H = ui.t.ELEMENT_H;
			var _ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;

			for (n in list[menuCategory]) {
				ui.fill(0, 1, ui._w / ui.SCALE(), ui.t.BUTTON_H + 2, ui.t.ACCENT_SELECT_COL);
				if (ui.button(Config.buttonSpacing + tr(n.name), Config.buttonAlign)) {
					pushUndo();
					var canvas = getCanvas(true);
					var nodes = getNodes();
					var node = makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					nodes.nodesSelected = [node];
					nodes.nodesDrag = true;
					arm.node.Brush.parse(canvas, false);
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
					ui.enabled = canPlaceGroup(g.canvas.name);
					if (ui.button("      " + g.canvas.name, Left)) {
						pushUndo();
						var canvas = getCanvas(true);
						var nodes = getNodes();
						var node = makeGroupNode(g.canvas, nodes, canvas);
						canvas.nodes.push(node);
						nodes.nodesSelected = [node];
						nodes.nodesDrag = true;
					}
					ui.enabled = true;
				}
			}

			hideMenu = ui.comboSelectedHandle == null && !showMenuFirst && (ui.changed || ui.inputReleased || ui.inputReleasedR || ui.isEscapeDown);
			showMenuFirst = false;

			ui.t.BUTTON_COL = _BUTTON_COL;
			ui.t.BUTTON_H = _BUTTON_H;
			ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
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
		History.editNodes(lastCanvas, canvasGroup);
	}

	public function acceptAssetDrag(index: Int) {
		pushUndo();
		var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		// var n = NodesBrush.createNode("ImageTextureNode", g);
		var n = NodesBrush.createNode("ImageTextureNode");
		n.buttons[0].default_value = index;
		getNodes().nodesSelected = [n];
		arm.node.Brush.parse(Project.canvas, false);
	}

	public function acceptSwatchDrag(swatch: TSwatchColor) {
		// pushUndo();
		// var g = groupStack.length > 0 ? groupStack[groupStack.length - 1] : null;
		// var n = NodesMaterial.createNode("RGB", g);
		// n.outputs[0].default_value = [swatch.base.R, swatch.base.G, swatch.base.B, swatch.base.A];
		// getNodes().nodesSelected = [n];
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
		if (Config.raw.touch_ui) {
			label = "";
		}
		return ui.button(Config.buttonSpacing + text, Config.buttonAlign, label);
	}
}
