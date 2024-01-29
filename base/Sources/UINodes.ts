
class UINodes {

	///if (is_paint || is_sculpt)
	static show = false;
	///end
	///if is_lab
	static show = true;
	///end

	static wx: i32;
	static wy: i32;
	static ww: i32;
	static wh: i32;

	static ui: Zui;
	static canvasType = CanvasType.CanvasMaterial;
	static showMenu = false;
	static showMenuFirst = true;
	static hideMenu = false;
	static menuCategory = 0;
	static popupX = 0.0;
	static popupY = 0.0;

	static uichangedLast = false;
	static recompileMat = false; // Mat preview
	static recompileMatFinal = false;
	static nodeSearchSpawn: TNode = null;
	static nodeSearchOffset = 0;
	static lastCanvas: TNodeCanvas = null;
	static lastNodeSelectedId = -1;
	static releaseLink = false;
	static isNodeMenuOperation = false;

	static grid: Image = null;
	static hwnd = new Handle();
	static groupStack: TNodeGroup[] = [];
	static controlsDown = false;

	constructor() {
		Nodes.onLinkDrag = UINodes.onLinkDrag;
		Nodes.onSocketReleased = UINodes.onSocketReleased;
		Nodes.onCanvasReleased = UINodes.onCanvasReleased;
		// Nodes.onNodeRemove = UINodes.onNodeRemove;
		Nodes.onCanvasControl = UINodes.onCanvasControl;

		let scale = Config.raw.window_scale;
		UINodes.ui = new Zui({ theme: Base.theme, font: Base.font, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient, scaleFactor: scale });
		UINodes.ui.scrollEnabled = false;
	}

	static onLinkDrag = (linkDragId: i32, isNewLink: bool) => {
		if (isNewLink) {
			let nodes = UINodes.getNodes();
			let linkDrag = nodes.getLink(UINodes.getCanvas(true).links, linkDragId);
			let node = nodes.getNode(UINodes.getCanvas(true).nodes, linkDrag.from_id > -1 ? linkDrag.from_id : linkDrag.to_id);
			let linkX = UINodes.ui._windowX + nodes.NODE_X(node);
			let linkY = UINodes.ui._windowY + nodes.NODE_Y(node);
			if (linkDrag.from_id > -1) {
				linkX += nodes.NODE_W(node);
				linkY += nodes.OUTPUT_Y(node.outputs, linkDrag.from_socket);
			}
			else {
				linkY += nodes.INPUT_Y(UINodes.getCanvas(true), node.inputs, linkDrag.to_socket) + nodes.OUTPUTS_H(node.outputs) + nodes.BUTTONS_H(node);
			}
			if (Math.abs(Mouse.x - linkX) > 5 || Math.abs(Mouse.y - linkY) > 5) { // Link length
				UINodes.nodeSearch(-1, -1, () => {
					let n = nodes.getNode(UINodes.getCanvas(true).nodes, nodes.nodesSelectedId[0]);
					if (linkDrag.to_id == -1 && n.inputs.length > 0) {
						linkDrag.to_id = n.id;
						let fromType = node.outputs[linkDrag.from_socket].type;
						// Connect to the first socket
						linkDrag.to_socket = 0;
						// Try to find the first type-matching socket and use it if present
						for (let socket of n.inputs) {
							if (socket.type == fromType) {
								linkDrag.to_socket = n.inputs.indexOf(socket);
								break;
							}
						}
						UINodes.getCanvas(true).links.push(linkDrag);
					}
					else if (linkDrag.from_id == -1 && n.outputs.length > 0) {
						linkDrag.from_id = n.id;
						linkDrag.from_socket = 0;
						UINodes.getCanvas(true).links.push(linkDrag);
					}
					///if is_lab
					ParserLogic.parse(UINodes.getCanvas(true));
					Context.raw.rdirty = 5;
					///end
				});
			}
			// Selecting which node socket to preview
			else if (node.id == nodes.nodesSelectedId[0]) {
				Context.raw.nodePreviewSocket = linkDrag.from_id > -1 ? linkDrag.from_socket : 0;
				///if (is_paint || is_sculpt)
				Context.raw.nodePreviewDirty = true;
				///end
			}
		}
	}

	static onSocketReleased = (socket_id: i32) => {
		let nodes = UINodes.getNodes();
		let canvas = UINodes.getCanvas(true);
		let socket = nodes.getSocket(canvas.nodes, socket_id);
		let node = nodes.getNode(canvas.nodes, socket.node_id);
		if (UINodes.ui.inputReleasedR) {
			if (node.type == "GROUP_INPUT" || node.type == "GROUP_OUTPUT") {
				Base.notifyOnNextFrame(() => {
					UIMenu.draw((ui: Zui) => {
						if (UIMenu.menuButton(ui, tr("Edit"))) {
							let htype = Zui.handle("uinodes_0");
							let hname = Zui.handle("uinodes_1");
							let hmin = Zui.handle("uinodes_2");
							let hmax = Zui.handle("uinodes_3");
							let hval0 = Zui.handle("uinodes_4");
							let hval1 = Zui.handle("uinodes_5");
							let hval2 = Zui.handle("uinodes_6");
							let hval3 = Zui.handle("uinodes_7");
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
							Base.notifyOnNextFrame(() => {
								Base.uiBox.endInput();
								UIBox.showCustom((ui: Zui) => {
									if (ui.tab(Zui.handle("uinodes_8"), tr("Socket"))) {
										let type = ui.combo(htype, [tr("Color"), tr("Vector"), tr("Value")], tr("Type"), true);
										if (htype.changed) hname.text = type == 0 ? tr("Color") : type == 1 ? tr("Vector") : tr("Value");
										let name = ui.textInput(hname, tr("Name"));
										let min = ui.floatInput(hmin, tr("Min"));
										let max = ui.floatInput(hmax, tr("Max"));
										let default_value: any = null;
										if (type == 0) {
											ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
											ui.floatInput(hval0, tr("R"));
											ui.floatInput(hval1, tr("G"));
											ui.floatInput(hval2, tr("B"));
											ui.floatInput(hval3, tr("A"));
											default_value = new Float32Array([hval0.value, hval1.value, hval2.value, hval3.value]);
										}
										else if (type == 1) {
											ui.row([1 / 3, 1 / 3, 1 / 3]);
											hval0.value = ui.floatInput(hval0, tr("X"));
											hval1.value = ui.floatInput(hval1, tr("Y"));
											hval2.value = ui.floatInput(hval2, tr("Z"));
											default_value = new Float32Array([hval0.value, hval1.value, hval2.value]);
										}
										else {
											default_value = ui.floatInput(hval0, tr("default_value"));
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
											UINodes.hwnd.redraws = 2;
										}
									}
								}, 400, 250);
							});
						}
						if (UIMenu.menuButton(ui, tr("Delete"))) {
							let i = 0;
							// Remove links connected to the socket
							while (i < canvas.links.length) {
								let l = canvas.links[i];
								if ((l.from_id == node.id && l.from_socket == node.outputs.indexOf(socket)) ||
									(l.to_id == node.id && l.to_socket == node.inputs.indexOf(socket))) {
									canvas.links.splice(i, 1);
								}
								else i++;
							}
							// Remove socket
							array_remove(node.inputs, socket);
							array_remove(node.outputs, socket);
							NodesMaterial.syncSockets(node);
						}
					}, 2);
				});
			}
			else UINodes.onCanvasReleased();
		}
		// Selecting which node socket to preview
		else if (node.id == nodes.nodesSelectedId[0]) {
			let i = node.outputs.indexOf(socket);
			if (i > -1) {
				Context.raw.nodePreviewSocket = i;
				///if (is_paint || is_sculpt)
				Context.raw.nodePreviewDirty = true;
				///end
			}
		}
	}

	static onCanvasReleased = () => {
		if (UINodes.ui.inputReleasedR && Math.abs(UINodes.ui.inputX - UINodes.ui.inputStartedX) < 2 && Math.abs(UINodes.ui.inputY - UINodes.ui.inputStartedY) < 2) {
			// Node selection
			let nodes = UINodes.getNodes();
			let canvas = UINodes.getCanvas(true);
			let selected: TNode = null;
			for (let node of canvas.nodes) {
				if (UINodes.ui.getInputInRect(UINodes.ui._windowX + nodes.NODE_X(node), UINodes.ui._windowY + nodes.NODE_Y(node), nodes.NODE_W(node), nodes.NODE_H(canvas, node))) {
					selected = node;
					break;
				}
			}
			if (selected == null) nodes.nodesSelectedId = [];
			else if (nodes.nodesSelectedId.indexOf(selected.id) == -1) nodes.nodesSelectedId = [selected.id];

			// Node context menu
			if (!Nodes.socketReleased) {
				let numberOfEntries = 5;
				if (UINodes.canvasType == CanvasType.CanvasMaterial) ++numberOfEntries;
				if (selected != null && selected.type == "RGB") ++numberOfEntries;

				UIMenu.draw((uiMenu: Zui) => {
					uiMenu._y += 1;
					let isProtected = selected == null ||
									///if (is_paint || is_sculpt)
									selected.type == "OUTPUT_MATERIAL_PBR" ||
									///end
									selected.type == "GROUP_INPUT" ||
									selected.type == "GROUP_OUTPUT" ||
									selected.type == "BrushOutputNode";
					uiMenu.enabled = !isProtected;
					if (UIMenu.menuButton(uiMenu, tr("Cut"), "ctrl+x")) {
						Base.notifyOnNextFrame(() => {
							UINodes.hwnd.redraws = 2;
							Zui.isCopy = true;
							Zui.isCut = true;
							UINodes.isNodeMenuOperation = true;
						});
					}
					if (UIMenu.menuButton(uiMenu, tr("Copy"), "ctrl+c")) {
						Base.notifyOnNextFrame(() => {
							Zui.isCopy = true;
							UINodes.isNodeMenuOperation = true;
						});
					}
					uiMenu.enabled = Nodes.clipboard != "";
					if (UIMenu.menuButton(uiMenu, tr("Paste"), "ctrl+v")) {
						Base.notifyOnNextFrame(() => {
							UINodes.hwnd.redraws = 2;
							Zui.isPaste = true;
							UINodes.isNodeMenuOperation = true;
						});
					}
					uiMenu.enabled = !isProtected;
					if (UIMenu.menuButton(uiMenu, tr("Delete"), "delete")) {
						Base.notifyOnNextFrame(() => {
							UINodes.hwnd.redraws = 2;
							UINodes.ui.isDeleteDown = true;
							UINodes.isNodeMenuOperation = true;
						});
					}
					if (UIMenu.menuButton(uiMenu, tr("Duplicate"))) {
						Base.notifyOnNextFrame(() => {
							UINodes.hwnd.redraws = 2;
							Zui.isCopy = true;
							Zui.isPaste = true;
							UINodes.isNodeMenuOperation = true;
						});
					}
					if (selected != null && selected.type == "RGB") {
						if (UIMenu.menuButton(uiMenu, tr("Add Swatch"))) {
							let color = selected.outputs[0].default_value;
							let newSwatch = Project.makeSwatch(color_from_floats(color[0], color[1], color[2], color[3]));
							Context.setSwatch(newSwatch);
							Project.raw.swatches.push(newSwatch);
							UIBase.hwnds[TabArea.TabStatus].redraws = 1;
						}
					}

					if (UINodes.canvasType == CanvasType.CanvasMaterial) {
						UIMenu.menuSeparator(uiMenu);
						if (UIMenu.menuButton(uiMenu, tr("2D View"))) {
							UIBase.show2DView(View2DType.View2DNode);
						}
					}

					uiMenu.enabled = true;
				}, numberOfEntries);
			}
		}

		if (UINodes.ui.inputReleased) {
			let nodes = UINodes.getNodes();
			let canvas = UINodes.getCanvas(true);
			for (let node of canvas.nodes) {
				if (UINodes.ui.getInputInRect(UINodes.ui._windowX + nodes.NODE_X(node), UINodes.ui._windowY + nodes.NODE_Y(node), nodes.NODE_W(node), nodes.NODE_H(canvas, node))) {
					if (node.id == nodes.nodesSelectedId[0]) {
						UIView2D.hwnd.redraws = 2;
						if (Time.time() - Context.raw.selectTime < 0.25) UIBase.show2DView(View2DType.View2DNode);
						Context.raw.selectTime = Time.time();
					}
					break;
				}
			}
		}
	}

	// static onNodeRemove = (node: TNode) => {
		// if (node.type == "GROUP") { // Remove unused groups
		// 	let found = false;
		// 	let canvases: TNodeCanvas[] = [];
		// 	for (let m of Project.materials) canvases.push(m.canvas);
		// 	for (let m of Project.materialGroups) canvases.push(m.canvas);
		// 	for (let canvas of canvases) {
		// 		for (let n of canvas.nodes) {
		// 			if (n.type == "GROUP" && n.name == node.name) {
		// 				found = true;
		// 				break;
		// 			}
		// 		}
		// 	}
		// 	if (!found) {
		// 		for (let g of Project.materialGroups) {
		// 			if (g.canvas.name == node.name) {
		// 				Project.materialGroups.remove(g);
		// 				break;
		// 			}
		// 		}
		// 	}
		// }
	// }

	static onCanvasControl = (): CanvasControl => {
		return UINodes.getCanvasControl(UINodes.ui, UINodes);
	}

	static getCanvasControl = (ui: Zui, parent: any): CanvasControl => {
		if (Config.raw.wrap_mouse && parent.controlsDown) {
			if (ui.inputX < ui._windowX) {
				ui.inputX = ui._windowX + ui._windowW;
				Krom.setMousePosition(Math.floor(ui.inputX), Math.floor(ui.inputY));
			}
			else if (ui.inputX > ui._windowX + ui._windowW) {
				ui.inputX = ui._windowX;
				Krom.setMousePosition(Math.floor(ui.inputX), Math.floor(ui.inputY));
			}
			else if (ui.inputY < ui._windowY) {
				ui.inputY = ui._windowY + ui._windowH;
				Krom.setMousePosition(Math.floor(ui.inputX), Math.floor(ui.inputY));
			}
			else if (ui.inputY > ui._windowY + ui._windowH) {
				ui.inputY = ui._windowY;
				Krom.setMousePosition(Math.floor(ui.inputX), Math.floor(ui.inputY));
			}
		}

		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutStarted) ||
			ui.inputStartedR ||
			ui.inputWheelDelta != 0.0) {
			parent.controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) &&
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

		let pan = ui.inputDownR || Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown);
		let zoomDelta = Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) ? UINodes.getZoomDelta(ui) / 100.0 : 0.0;
		let control = {
			panX: pan ? ui.inputDX : 0.0,
			panY: pan ? ui.inputDY : 0.0,
			zoom: ui.inputWheelDelta != 0.0 ? -ui.inputWheelDelta / 10 : zoomDelta
		};
		if (Base.isComboSelected()) control.zoom = 0.0;
		return control;
	}

	static getZoomDelta = (ui: Zui): f32 => {
		return Config.raw.zoom_direction == ZoomDirection.ZoomVertical ? -ui.inputDY :
			   Config.raw.zoom_direction == ZoomDirection.ZoomVerticalInverted ? -ui.inputDY :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontal ? ui.inputDX :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontalInverted ? ui.inputDX :
			   -(ui.inputDY - ui.inputDX);
	}

	static getCanvas = (groups = false): TNodeCanvas => {
		///if (is_paint || is_sculpt)
		if (UINodes.canvasType == CanvasType.CanvasMaterial) {
			if (groups && UINodes.groupStack.length > 0) return UINodes.groupStack[UINodes.groupStack.length - 1].canvas;
			else return UINodes.getCanvasMaterial();
		}
		else return Context.raw.brush.canvas;
		///end

		///if is_lab
		return Project.canvas;
		///end
	}

	///if (is_paint || is_sculpt)
	static getCanvasMaterial = (): TNodeCanvas => {
		return Context.raw.material.canvas;
	}
	///end

	static getNodes = (): Nodes => {
		///if (is_paint || is_sculpt)
		if (UINodes.canvasType == CanvasType.CanvasMaterial) {
			if (UINodes.groupStack.length > 0) return UINodes.groupStack[UINodes.groupStack.length - 1].nodes;
			else return Context.raw.material.nodes;
		}
		else return Context.raw.brush.nodes;
		///end

		///if is_lab
		if (UINodes.groupStack.length > 0) return UINodes.groupStack[UINodes.groupStack.length - 1].nodes;
		else return Project.nodes;
		///end
	}

	static update = () => {
		if (!UINodes.show || !Base.uiEnabled) return;

		///if (is_paint || is_sculpt)
		UINodes.wx = Math.floor(App.w()) + UIToolbar.toolbarw;
		///end
		///if is_lab
		UINodes.wx = Math.floor(App.w());
		///end
		UINodes.wy = UIHeader.headerh * 2;

		if (UIView2D.show) {
			UINodes.wy += App.h() - Config.raw.layout[LayoutSize.LayoutNodesH];
		}

		let ww = Config.raw.layout[LayoutSize.LayoutNodesW];
		if (!UIBase.show) {
			///if (is_paint || is_sculpt)
			ww += Config.raw.layout[LayoutSize.LayoutSidebarW] + UIToolbar.toolbarw;
			UINodes.wx -= UIToolbar.toolbarw;
			///end
			UINodes.wy = 0;
		}

		let mx = Mouse.x;
		let my = Mouse.y;
		if (mx < UINodes.wx || mx > UINodes.wx + ww || my < UINodes.wy) return;
		if (UINodes.ui.isTyping || !UINodes.ui.inputEnabled) return;

		let nodes = UINodes.getNodes();
		if (nodes.nodesSelectedId.length > 0 && UINodes.ui.isKeyPressed) {
			if (UINodes.ui.key == KeyCode.Left) for (let n of nodes.nodesSelectedId) nodes.getNode(UINodes.getCanvas(true).nodes, n).x -= 1;
			else if (UINodes.ui.key == KeyCode.Right) for (let n of nodes.nodesSelectedId) nodes.getNode(UINodes.getCanvas(true).nodes, n).x += 1;
			if (UINodes.ui.key == KeyCode.Up) for (let n of nodes.nodesSelectedId) nodes.getNode(UINodes.getCanvas(true).nodes, n).y -= 1;
			else if (UINodes.ui.key == KeyCode.Down) for (let n of nodes.nodesSelectedId) nodes.getNode(UINodes.getCanvas(true).nodes, n).y += 1;
		}

		// Node search popup
		if (Operator.shortcut(Config.keymap.node_search)) UINodes.nodeSearch();
		if (UINodes.nodeSearchSpawn != null) {
			UINodes.ui.inputX = Mouse.x; // Fix inputDX after popup removal
			UINodes.ui.inputY = Mouse.y;
			UINodes.nodeSearchSpawn = null;
		}

		if (Operator.shortcut(Config.keymap.view_reset)) {
			nodes.panX = 0.0;
			nodes.panY = 0.0;
			nodes.zoom = 1.0;
		}
	}

	static canvasChanged = () => {
		UINodes.recompileMat = true;
		UINodes.recompileMatFinal = true;
	}

	static nodeSearch = (x = -1, y = -1, done: ()=>void = null) => {
		let searchHandle = Zui.handle("uinodes_9");
		let first = true;
		UIMenu.draw((ui: Zui) => {
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.drawRect(ui.g, true, ui._x, ui._y, ui._w, ui.ELEMENT_H() * 8);
			ui.g.color = 0xffffffff;

			let search = ui.textInput(searchHandle, "", Align.Left, true, true).toLowerCase();
			ui.changed = false;
			if (first) {
				first = false;
				searchHandle.text = "";
				ui.startTextEdit(searchHandle); // Focus search bar
			}

			if (searchHandle.changed) UINodes.nodeSearchOffset = 0;

			if (ui.isKeyPressed) { // Move selection
				if (ui.key == KeyCode.Down && UINodes.nodeSearchOffset < 6) UINodes.nodeSearchOffset++;
				if (ui.key == KeyCode.Up && UINodes.nodeSearchOffset > 0) UINodes.nodeSearchOffset--;
			}
			let enter = Keyboard.down("enter");
			let count = 0;
			let BUTTON_COL = ui.t.BUTTON_COL;

			///if (is_paint || is_sculpt)
			let nodeList = UINodes.canvasType == CanvasType.CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			///end
			///if is_lab
			let nodeList = NodesBrush.list;
			///end

			for (let list of nodeList) {
				for (let n of list) {
					if (tr(n.name).toLowerCase().indexOf(search) >= 0) {
						ui.t.BUTTON_COL = count == UINodes.nodeSearchOffset ? ui.t.HIGHLIGHT_COL : ui.t.SEPARATOR_COL;
						if (ui.button(tr(n.name), Align.Left) || (enter && count == UINodes.nodeSearchOffset)) {
							UINodes.pushUndo();
							let nodes = UINodes.getNodes();
							let canvas = UINodes.getCanvas(true);
							UINodes.nodeSearchSpawn = UINodes.makeNode(n, nodes, canvas); // Spawn selected node
							canvas.nodes.push(UINodes.nodeSearchSpawn);
							nodes.nodesSelectedId = [UINodes.nodeSearchSpawn.id];
							nodes.nodesDrag = true;

							///if is_lab
							ParserLogic.parse(canvas);
							///end

							UINodes.hwnd.redraws = 2;
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

	static getNodeX = (): i32 => {
		return Math.floor((Mouse.x - UINodes.wx - UINodes.getNodes().PAN_X()) / UINodes.getNodes().SCALE());
	}

	static getNodeY = (): i32 => {
		return Math.floor((Mouse.y - UINodes.wy - UINodes.getNodes().PAN_Y()) / UINodes.getNodes().SCALE());
	}

	static drawGrid = () => {
		let ww = Config.raw.layout[LayoutSize.LayoutNodesW];

		///if (is_paint || is_sculpt)
		if (!UIBase.show) {
			ww += Config.raw.layout[LayoutSize.LayoutSidebarW] + UIToolbar.toolbarw;
		}
		///end

		let wh = App.h();
		let step = 100 * UINodes.ui.SCALE();
		let w = Math.floor(ww + step * 3);
		let h = Math.floor(wh + step * 3);
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		UINodes.grid = Image.createRenderTarget(w, h);
		UINodes.grid.g2.begin(true, UINodes.ui.t.SEPARATOR_COL);

		UINodes.grid.g2.color = UINodes.ui.t.SEPARATOR_COL - 0x00050505;
		step = 20 * UINodes.ui.SCALE();
		for (let i = 0; i < Math.floor(h / step) + 1; ++i) {
			UINodes.grid.g2.drawLine(0, i * step, w, i * step);
		}
		for (let i = 0; i < Math.floor(w / step) + 1; ++i) {
			UINodes.grid.g2.drawLine(i * step, 0, i * step, h);
		}

		UINodes.grid.g2.color = UINodes.ui.t.SEPARATOR_COL - 0x00090909;
		step = 100 * UINodes.ui.SCALE();
		for (let i = 0; i < Math.floor(h / step) + 1; ++i) {
			UINodes.grid.g2.drawLine(0, i * step, w, i * step);
		}
		for (let i = 0; i < Math.floor(w / step) + 1; ++i) {
			UINodes.grid.g2.drawLine(i * step, 0, i * step, h);
		}

		UINodes.grid.g2.end();
	}

	static render = (g: Graphics2) => {
		if (UINodes.recompileMat) {
			///if (is_paint || is_sculpt)
			if (UINodes.canvasType == CanvasType.CanvasBrush) {
				MakeMaterial.parseBrush();
				UtilRender.makeBrushPreview();
				UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			}
			else {
				Base.isFillMaterial() ? Base.updateFillLayers() : UtilRender.makeMaterialPreview();
				if (UIView2D.show && UIView2D.type == View2DType.View2DNode) {
					UIView2D.hwnd.redraws = 2;
				}
			}

			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			if (Context.raw.splitView) Context.raw.ddirty = 2;
			///end

			///if is_lab
			ParserLogic.parse(Project.canvas);
			///end

			UINodes.recompileMat = false;
		}
		else if (UINodes.recompileMatFinal) {
			///if (is_paint || is_sculpt)
			MakeMaterial.parsePaintMaterial();

			if (UINodes.canvasType == CanvasType.CanvasMaterial && Base.isFillMaterial()) {
				Base.updateFillLayers();
				UtilRender.makeMaterialPreview();
			}

			let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
			if (decal) UtilRender.makeDecalPreview();

			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			Context.raw.nodePreviewDirty = true;
			///end

			UINodes.recompileMatFinal = false;
		}

		let nodes = UINodes.getNodes();
		if (nodes.nodesSelectedId.length > 0 && nodes.nodesSelectedId[0] != UINodes.lastNodeSelectedId) {
			UINodes.lastNodeSelectedId = nodes.nodesSelectedId[0];
			///if (is_paint || is_sculpt)
			Context.raw.nodePreviewDirty = true;
			///end

			///if is_lab
			Context.raw.ddirty = 2; // Show selected node texture in viewport
			UIHeader.headerHandle.redraws = 2;
			///end

			Context.raw.nodePreviewSocket = 0;
		}

		// Remove dragged link when mouse is released out of the node viewport
		let c = UINodes.getCanvas(true);
		if (UINodes.releaseLink && nodes.linkDragId != -1) {
			array_remove(c.links, nodes.getLink(c.links, nodes.linkDragId));
			nodes.linkDragId = -1;
		}
		UINodes.releaseLink = UINodes.ui.inputReleased;

		if (!UINodes.show || System.width == 0 || System.height == 0) return;

		UINodes.ui.inputEnabled = Base.uiEnabled;

		g.end();

		if (UINodes.grid == null) UINodes.drawGrid();

		///if (is_paint || is_sculpt)
		if (Config.raw.node_preview && Context.raw.nodePreviewDirty) {
			UINodes.makeNodePreview();
		}
		///end

		// Start with UI
		UINodes.ui.begin(g);

		// Make window
		UINodes.ww = Config.raw.layout[LayoutSize.LayoutNodesW];

		///if (is_paint || is_sculpt)
		UINodes.wx = Math.floor(App.w()) + UIToolbar.toolbarw;
		///end
		///if is_lab
		UINodes.wx = Math.floor(App.w());
		///end

		UINodes.wy = 0;

		///if (is_paint || is_sculpt)
		if (!UIBase.show) {
			UINodes.ww += Config.raw.layout[LayoutSize.LayoutSidebarW] + UIToolbar.toolbarw;
			UINodes.wx -= UIToolbar.toolbarw;
		}
		///end

		let ew = Math.floor(UINodes.ui.ELEMENT_W() * 0.7);
		UINodes.wh = App.h() + UIHeader.headerh;
		if (Config.raw.layout[LayoutSize.LayoutHeader] == 1) UINodes.wh += UIHeader.headerh;

		if (UIView2D.show) {
			UINodes.wh = Config.raw.layout[LayoutSize.LayoutNodesH];
			UINodes.wy = App.h() - Config.raw.layout[LayoutSize.LayoutNodesH] + UIHeader.headerh;
			if (Config.raw.layout[LayoutSize.LayoutHeader] == 1) UINodes.wy += UIHeader.headerh;
			if (!UIBase.show) {
				UINodes.wy -= UIHeader.headerh * 2;
			}
		}

		if (UINodes.ui.window(UINodes.hwnd, UINodes.wx, UINodes.wy, UINodes.ww, UINodes.wh)) {

			UINodes.ui.tab(Zui.handle("uinodes_10"), tr("Nodes"));

			// Grid
			UINodes.ui.g.color = 0xffffffff;
			let step = 100 * UINodes.ui.SCALE();
			UINodes.ui.g.drawImage(UINodes.grid, (nodes.panX * nodes.SCALE()) % step - step, (nodes.panY * nodes.SCALE()) % step - step);

			// Undo
			if (UINodes.ui.inputStarted || UINodes.ui.isKeyPressed) {
				UINodes.lastCanvas = JSON.parse(JSON.stringify(UINodes.getCanvas(true)));
			}

			// Nodes
			let _inputEnabled = UINodes.ui.inputEnabled;
			UINodes.ui.inputEnabled = _inputEnabled && !UINodes.showMenu;
			///if (is_paint || is_sculpt)
			UINodes.ui.windowBorderRight = Config.raw.layout[LayoutSize.LayoutSidebarW];
			///end
			UINodes.ui.windowBorderTop = UIHeader.headerh * 2;
			UINodes.ui.windowBorderBottom = Config.raw.layout[LayoutSize.LayoutStatusH];
			nodes.nodeCanvas(UINodes.ui, c);
			UINodes.ui.inputEnabled = _inputEnabled;

			if (nodes.colorPickerCallback != null) {
				Context.raw.colorPickerPreviousTool = Context.raw.tool;
				Context.selectTool(WorkspaceTool.ToolPicker);
				let tmp = nodes.colorPickerCallback;
				Context.raw.colorPickerCallback = (color: TSwatchColor) => {
					tmp(color.base);
					UINodes.hwnd.redraws = 2;

					///if (is_paint || is_sculpt)
					let material_live = Config.raw.material_live;
					///end
					///if is_lab
					let material_live = true;
					///end

					if (material_live) {
						UINodes.canvasChanged();
					}
				};
				nodes.colorPickerCallback = null;
			}

			// Remove nodes with unknown id for this canvas type
			if (Zui.isPaste) {
				///if (is_paint || is_sculpt)
				let nodeList = UINodes.canvasType == CanvasType.CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
				///end
				///if is_lab
				let nodeList = NodesBrush.list;
				///end

				let i = 0;
				while (i++ < c.nodes.length) {
					let canvasNode = c.nodes[i - 1];
					if (Nodes.excludeRemove.indexOf(canvasNode.type) >= 0) {
						continue;
					}
					let found = false;
					for (let list of nodeList) {
						for (let listNode of list) {
							if (canvasNode.type == listNode.type) {
								found = true;
								break;
							}
						}
						if (found) break;
					}
					if (canvasNode.type == "GROUP" && !UINodes.canPlaceGroup(canvasNode.name)) {
						found = false;
					}
					if (!found) {
						nodes.removeNode(canvasNode, c);
						array_remove(nodes.nodesSelectedId, canvasNode.id);
						i--;
					}
				}
			}

			if (UINodes.isNodeMenuOperation) {
				Zui.isCopy = Zui.isCut = Zui.isPaste = UINodes.ui.isDeleteDown = false;
			}

			// Recompile material on change
			if (UINodes.ui.changed) {
				///if (is_paint || is_sculpt)
				UINodes.recompileMat = (UINodes.ui.inputDX != 0 || UINodes.ui.inputDY != 0 || !UINodes.uichangedLast) && Config.raw.material_live; // Instant preview
				///end
				///if is_lab
				UINodes.recompileMat = (UINodes.ui.inputDX != 0 || UINodes.ui.inputDY != 0 || !UINodes.uichangedLast); // Instant preview
				///end
			}
			else if (UINodes.uichangedLast) {
				UINodes.canvasChanged();
				UINodes.pushUndo(UINodes.lastCanvas);
			}
			UINodes.uichangedLast = UINodes.ui.changed;

			// Node previews
			if (Config.raw.node_preview && nodes.nodesSelectedId.length > 0) {
				let img: Image = null;
				let sel = nodes.getNode(c.nodes, nodes.nodesSelectedId[0]);

				///if (is_paint || is_sculpt)

				let singleChannel = sel.type == "LAYER_MASK";
				if (sel.type == "LAYER" || sel.type == "LAYER_MASK") {
					let id = sel.buttons[0].default_value;
					if (id < Project.layers.length) {
						///if is_paint
						img = Project.layers[id].texpaint_preview;
						///end
					}
				}
				else if (sel.type == "MATERIAL") {
					let id = sel.buttons[0].default_value;
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
				else if (UINodes.canvasType == CanvasType.CanvasMaterial) {
					img = Context.raw.nodePreview;
				}

				///else

				let brushNode = ParserLogic.getLogicNode(sel);
				if (brushNode != null) {
					img = brushNode.getCachedImage();
				}

				///end

				if (img != null) {
					let tw = 128 * UINodes.ui.SCALE();
					let th = tw * (img.height / img.width);
					let tx = UINodes.ww - tw - 8 * UINodes.ui.SCALE();
					let ty = UINodes.wh - th - 8 * UINodes.ui.SCALE();

					///if krom_opengl
					let invertY = sel.type == "MATERIAL";
					///else
					let invertY = false;
					///end

					///if (is_paint || is_sculpt)
					if (singleChannel) {
						UINodes.ui.g.pipeline = UIView2D.pipe;
						///if krom_opengl
						Krom.setPipeline(UIView2D.pipe.pipeline_);
						///end
						Krom.setInt(UIView2D.channelLocation, 1);
					}
					///end

					UINodes.ui.g.color = 0xffffffff;
					invertY ?
						UINodes.ui.g.drawScaledImage(img, tx, ty + th, tw, -th) :
						UINodes.ui.g.drawScaledImage(img, tx, ty, tw, th);

					///if (is_paint || is_sculpt)
					if  (singleChannel) {
						UINodes.ui.g.pipeline = null;
					}
					///end
				}
			}

			// Menu
			UINodes.ui.g.color = UINodes.ui.t.SEPARATOR_COL;
			UINodes.ui.g.fillRect(0, UINodes.ui.ELEMENT_H(), UINodes.ww, UINodes.ui.ELEMENT_H() + UINodes.ui.ELEMENT_OFFSET() * 2);
			UINodes.ui.g.color = 0xffffffff;

			let startY = UINodes.ui.ELEMENT_H() + UINodes.ui.ELEMENT_OFFSET();
			UINodes.ui._x = 0;
			UINodes.ui._y = 2 + startY;
			UINodes.ui._w = ew;

			///if (is_paint || is_sculpt)
			// Editable canvas name
			let h = Zui.handle("uinodes_11");
			h.text = c.name;
			UINodes.ui._w = Math.floor(Math.min(UINodes.ui.font.width(UINodes.ui.fontSize, h.text) + 15 * UINodes.ui.SCALE(), 100 * UINodes.ui.SCALE()));
			let newName = UINodes.ui.textInput(h, "");
			UINodes.ui._x += UINodes.ui._w + 3;
			UINodes.ui._y = 2 + startY;
			UINodes.ui._w = ew;

			if (h.changed) { // Check whether renaming is possible and update group links
				if (UINodes.groupStack.length > 0) {
					let canRename = true;
					for (let m of Project.materialGroups) {
						if (m.canvas.name == newName) canRename = false; // Name already used
					}

					if (canRename) {
						let oldName = c.name;
						c.name = newName;
						let canvases: TNodeCanvas[] = [];
						for (let m of Project.materials) canvases.push(m.canvas);
						for (let m of Project.materialGroups) canvases.push(m.canvas);
						for (let canvas of canvases) {
							for (let n of canvas.nodes) {
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
			///end

			///if is_lab
			UINodes.ui.windowBorderTop = 0;
			UINodesExt.drawButtons(ew, startY);
			///end

			let _BUTTON_COL = UINodes.ui.t.BUTTON_COL;
			UINodes.ui.t.BUTTON_COL = UINodes.ui.t.SEPARATOR_COL;

			///if (is_paint || is_sculpt)
			let cats = UINodes.canvasType == CanvasType.CanvasMaterial ? NodesMaterial.categories : NodesBrush.categories;
			///end
			///if is_lab
			let cats = NodesBrush.categories;
			///end

			for (let i = 0; i < cats.length; ++i) {
				if ((UINodes.ui.menuButton(tr(cats[i]))) || (UINodes.ui.isHovered && UINodes.showMenu)) {
					UINodes.showMenu = true;
					UINodes.menuCategory = i;
					UINodes.popupX = UINodes.wx + UINodes.ui._x;
					UINodes.popupY = UINodes.wy + UINodes.ui._y;
					if (Config.raw.touch_ui) {
						UINodes.showMenuFirst = true;
						let menuw = Math.floor(ew * 2.3);
						UINodes.popupX -= menuw / 2;
						UINodes.popupX += UINodes.ui._w / 2;
					}
					UIMenu.menuCategoryW = UINodes.ui._w;
					UIMenu.menuCategoryH = Math.floor(UINodes.ui.MENUBAR_H());
				}
				UINodes.ui._x += UINodes.ui._w + 3;
				UINodes.ui._y = 2 + startY;
			}

			if (Config.raw.touch_ui) {
				let _w = UINodes.ui._w;
				UINodes.ui._w = Math.floor(36 * UINodes.ui.SCALE());
				UINodes.ui._y = 4 * UINodes.ui.SCALE() + startY;
				if (UIMenubar.iconButton(UINodes.ui, 2, 3)) {
					UINodes.nodeSearch(Math.floor(UINodes.ui._windowX + UINodes.ui._x), Math.floor(UINodes.ui._windowY + UINodes.ui._y));
				}
				UINodes.ui._w = _w;
			}
			else {
				if (UINodes.ui.menuButton(tr("Search"))) {
					UINodes.nodeSearch(Math.floor(UINodes.ui._windowX + UINodes.ui._x), Math.floor(UINodes.ui._windowY + UINodes.ui._y));
				}
			}
			if (UINodes.ui.isHovered) {
				UINodes.ui.tooltip(tr("Search for nodes") + ` (${Config.keymap.node_search})`);
			}
			UINodes.ui._x += UINodes.ui._w + 3;
			UINodes.ui._y = 2 + startY;

			UINodes.ui.t.BUTTON_COL = _BUTTON_COL;

			// Close node group
			if (UINodes.groupStack.length > 0 && UINodes.ui.menuButton(tr("Close"))) {
				UINodes.groupStack.pop();
			}
		}

		UINodes.ui.end(!UINodes.showMenu);

		g.begin(false);

		if (UINodes.showMenu) {
			///if (is_paint || is_sculpt)
			let list = UINodes.canvasType == CanvasType.CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			///end
			///if is_lab
			let list = NodesBrush.list;
			///end

			let numNodes = list[UINodes.menuCategory].length;

			///if (is_paint || is_sculpt)
			let isGroupCategory = UINodes.canvasType == CanvasType.CanvasMaterial && NodesMaterial.categories[UINodes.menuCategory] == "Group";
			///end
			///if is_lab
			let isGroupCategory = NodesMaterial.categories[UINodes.menuCategory] == "Group";
			///end

			if (isGroupCategory) numNodes += Project.materialGroups.length;

			let py = UINodes.popupY;
			let menuw = Math.floor(ew * 2.3);
			UINodes.ui.beginRegion(g, Math.floor(UINodes.popupX), Math.floor(py), menuw);
			let _BUTTON_COL = UINodes.ui.t.BUTTON_COL;
			UINodes.ui.t.BUTTON_COL = UINodes.ui.t.SEPARATOR_COL;
			let _ELEMENT_OFFSET = UINodes.ui.t.ELEMENT_OFFSET;
			UINodes.ui.t.ELEMENT_OFFSET = 0;
			let _ELEMENT_H = UINodes.ui.t.ELEMENT_H;
			UINodes.ui.t.ELEMENT_H = Config.raw.touch_ui ? (28 + 2) : 28;

			UIMenu.menuStart(UINodes.ui);

			for (let n of list[UINodes.menuCategory]) {
				if (UIMenu.menuButton(UINodes.ui, tr(n.name))) {
					UINodes.pushUndo();
					let canvas = UINodes.getCanvas(true);
					let nodes = UINodes.getNodes();
					let node = UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					nodes.nodesSelectedId = [node.id];
					nodes.nodesDrag = true;
					///if is_lab
					ParserLogic.parse(canvas);
					///end
				}
				// Next column
				if (UINodes.ui._y - UINodes.wy + UINodes.ui.ELEMENT_H() / 2 > UINodes.wh) {
					UINodes.ui._x += menuw;
					UINodes.ui._y = py;
				}
			}
			if (isGroupCategory) {
				for (let g of Project.materialGroups) {
					UINodes.ui.fill(0, 1, UINodes.ui._w / UINodes.ui.SCALE(), UINodes.ui.t.BUTTON_H + 2, UINodes.ui.t.ACCENT_SELECT_COL);
					UINodes.ui.fill(1, 1, UINodes.ui._w / UINodes.ui.SCALE() - 2, UINodes.ui.t.BUTTON_H + 1, UINodes.ui.t.SEPARATOR_COL);
					UINodes.ui.enabled = UINodes.canPlaceGroup(g.canvas.name);
					UIMenu.menuFill(UINodes.ui);
					UINodes.ui.row([5 / 6, 1 / 6]);
					if (UINodes.ui.button(Config.buttonSpacing + g.canvas.name, Align.Left)) {
						UINodes.pushUndo();
						let canvas = UINodes.getCanvas(true);
						let nodes = UINodes.getNodes();
						let node = UINodes.makeGroupNode(g.canvas, nodes, canvas);
						canvas.nodes.push(node);
						nodes.nodesSelectedId = [node.id];
						nodes.nodesDrag = true;
					}

					///if (is_paint || is_sculpt)
					UINodes.ui.enabled = !Project.isMaterialGroupInUse(g);
					if (UINodes.ui.button("x", Align.Center)) {
						History.deleteMaterialGroup(g);
						array_remove(Project.materialGroups, g);
					}
					///end

					UINodes.ui.enabled = true;
				}
			}

			UINodes.hideMenu = UINodes.ui.comboSelectedHandle_ptr == null && !UINodes.showMenuFirst && (UINodes.ui.changed || UINodes.ui.inputReleased || UINodes.ui.inputReleasedR || UINodes.ui.isEscapeDown);
			UINodes.showMenuFirst = false;

			UINodes.ui.t.BUTTON_COL = _BUTTON_COL;
			UINodes.ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
			UINodes.ui.t.ELEMENT_H = _ELEMENT_H;
			UINodes.ui.endRegion();
		}

		if (UINodes.hideMenu) {
			UINodes.showMenu = false;
			UINodes.showMenuFirst = true;
		}
	}

	static containsNodeGroupRecursive = (group: TNodeGroup, groupName: string): bool => {
		if (group.canvas.name == groupName) {
			return true;
		}
		for (let n of group.canvas.nodes) {
			if (n.type == "GROUP") {
				let g = Project.getMaterialGroupByName(n.name);
				if (g != null && UINodes.containsNodeGroupRecursive(g, groupName)) {
					return true;
				}
			}
		}
		return false;
	}

	static canPlaceGroup = (groupName: string): bool => {
		// Prevent Recursive node groups
		// The group to place must not contain the current group or a group that contains the current group
		if (UINodes.groupStack.length > 0) {
			for (let g of UINodes.groupStack) {
				if (UINodes.containsNodeGroupRecursive(Project.getMaterialGroupByName(groupName), g.canvas.name)) return false;
			}
		}
		// Group was deleted / renamed
		let groupExists = false;
		for (let group of Project.materialGroups) {
			if (groupName == group.canvas.name) {
				groupExists = true;
			}
		}
		if (!groupExists) return false;
		return true;
	}

	static pushUndo = (lastCanvas: TNodeCanvas = null) => {
		if (lastCanvas == null) lastCanvas = UINodes.getCanvas(true);
		let canvasGroup = UINodes.groupStack.length > 0 ? Project.materialGroups.indexOf(UINodes.groupStack[UINodes.groupStack.length - 1]) : null;

		///if (is_paint || is_sculpt)
		UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		History.editNodes(lastCanvas, UINodes.canvasType, canvasGroup);
		///end
		///if is_lab
		History.editNodes(lastCanvas, canvasGroup);
		///end
	}

	static acceptAssetDrag = (index: i32) => {
		UINodes.pushUndo();
		let g = UINodes.groupStack.length > 0 ? UINodes.groupStack[UINodes.groupStack.length - 1] : null;
		///if (is_paint || is_sculpt)
		let n = UINodes.canvasType == CanvasType.CanvasMaterial ? NodesMaterial.createNode("TEX_IMAGE", g) : NodesBrush.createNode("TEX_IMAGE");
		///end
		///if is_lab
		let n = NodesBrush.createNode("ImageTextureNode");
		///end

		n.buttons[0].default_value = index;
		UINodes.getNodes().nodesSelectedId = [n.id];

		///if is_lab
		ParserLogic.parse(Project.canvas);
		///end
	}

	///if (is_paint || is_sculpt)
	static acceptLayerDrag = (index: i32) => {
		UINodes.pushUndo();
		if (SlotLayer.isGroup(Project.layers[index])) return;
		let g = UINodes.groupStack.length > 0 ? UINodes.groupStack[UINodes.groupStack.length - 1] : null;
		let n = NodesMaterial.createNode(SlotLayer.isMask(Context.raw.layer) ? "LAYER_MASK" : "LAYER", g);
		n.buttons[0].default_value = index;
		UINodes.getNodes().nodesSelectedId = [n.id];
	}

	static acceptMaterialDrag = (index: i32) => {
		UINodes.pushUndo();
		let g = UINodes.groupStack.length > 0 ? UINodes.groupStack[UINodes.groupStack.length - 1] : null;
		let n = NodesMaterial.createNode("MATERIAL", g);
		n.buttons[0].default_value = index;
		UINodes.getNodes().nodesSelectedId = [n.id];
	}
	///end

	static acceptSwatchDrag = (swatch: TSwatchColor) => {
		///if (is_paint || is_sculpt)
		UINodes.pushUndo();
		let g = UINodes.groupStack.length > 0 ? UINodes.groupStack[UINodes.groupStack.length - 1] : null;
		let n = NodesMaterial.createNode("RGB", g);
		n.outputs[0].default_value = [
			color_get_rb(swatch.base) / 255,
			color_get_gb(swatch.base) / 255,
			color_get_bb(swatch.base) / 255,
			color_get_ab(swatch.base) / 255
		];
		UINodes.getNodes().nodesSelectedId = [n.id];
		///end
	}

	static makeNode = (n: TNode, nodes: Nodes, canvas: TNodeCanvas): TNode => {
		let node: TNode = JSON.parse(JSON.stringify(n));
		node.id = nodes.getNodeId(canvas.nodes);
		node.x = UINodes.getNodeX();
		node.y = UINodes.getNodeY();
		let count = 0;
		for (let soc of node.inputs) {
			soc.id = nodes.getSocketId(canvas.nodes) + count;
			soc.node_id = node.id;
			count++;
		}
		for (let soc of node.outputs) {
			soc.id = nodes.getSocketId(canvas.nodes) + count;
			soc.node_id = node.id;
			count++;
		}
		return node;
	}

	static makeGroupNode = (groupCanvas: TNodeCanvas, nodes: Nodes, canvas: TNodeCanvas): TNode => {
		let n = NodesMaterial.list[5][0];
		let node: TNode = JSON.parse(JSON.stringify(n));
		node.name = groupCanvas.name;
		node.id = nodes.getNodeId(canvas.nodes);
		node.x = UINodes.getNodeX();
		node.y = UINodes.getNodeY();
		let groupInput: TNode = null;
		let groupOutput: TNode = null;
		for (let g of Project.materialGroups) {
			if (g.canvas.name == node.name) {
				for (let n of g.canvas.nodes) {
					if (n.type == "GROUP_INPUT") groupInput = n;
					else if (n.type == "GROUP_OUTPUT") groupOutput = n;
				}
				break;
			}
		}
		if (groupInput != null && groupOutput != null) {
			for (let soc of groupInput.outputs) {
				node.inputs.push(NodesMaterial.createSocket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
			}
			for (let soc of groupOutput.inputs) {
				node.outputs.push(NodesMaterial.createSocket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
			}
		}
		return node;
	}

	///if (is_paint || is_sculpt)
	static makeNodePreview = () => {
		let nodes = Context.raw.material.nodes;
		if (nodes.nodesSelectedId.length == 0) return;

		let node = nodes.getNode(Context.raw.material.canvas.nodes, nodes.nodesSelectedId[0]);
		// if (node == null) return;
		Context.raw.nodePreviewName = node.name;

		if (node.type == "LAYER" ||
			node.type == "LAYER_MASK" ||
			node.type == "MATERIAL" ||
			node.type == "OUTPUT_MATERIAL_PBR") return;

		if (Context.raw.material.canvas.nodes.indexOf(node) == -1) return;

		if (Context.raw.nodePreview == null) {
			Context.raw.nodePreview = Image.createRenderTarget(UtilRender.materialPreviewSize, UtilRender.materialPreviewSize);
		}

		Context.raw.nodePreviewDirty = false;
		UINodes.hwnd.redraws = 2;
		UtilRender.makeNodePreview(Context.raw.material.canvas, node, Context.raw.nodePreview);
	}
	///end

	static hasGroup = (c: TNodeCanvas): bool => {
		for (let n of c.nodes) if (n.type == "GROUP") return true;
		return false;
	}

	static traverseGroup = (mgroups: TNodeCanvas[], c: TNodeCanvas) => {
		for (let n of c.nodes) {
			if (n.type == "GROUP") {
				if (UINodes.getGroup(mgroups, n.name) == null) {
					let canvases: TNodeCanvas[] = [];
					for (let g of Project.materialGroups) canvases.push(g.canvas);
					let group = UINodes.getGroup(canvases, n.name);
					mgroups.push(JSON.parse(JSON.stringify(group)));
					UINodes.traverseGroup(mgroups, group);
				}
			}
		}
	}

	static getGroup = (canvases: TNodeCanvas[], name: string): TNodeCanvas => {
		for (let c of canvases) if (c.name == name) return c;
		return null;
	}
}
