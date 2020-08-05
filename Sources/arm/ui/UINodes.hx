package arm.ui;

import haxe.Json;
import kha.Image;
import kha.System;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.system.Input;
import arm.node.NodesMaterial;
import arm.node.NodesBrush;
import arm.node.MaterialParser;
import arm.util.RenderUtil;
import arm.ui.UIHeader;
import arm.Enums;

@:access(zui.Zui)
class UINodes {

	public static var inst: UINodes;

	public var show = false;
	public var defaultWindowW = 0;
	public var defaultWindowH = 0;
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

	public var changed = false;
	var mdown = false;
	var mreleased = false;
	var mchanged = false;
	var mstartedlast = false;
	var recompileMat = false; // Mat preview
	var recompileMatFinal = false;
	var nodeSearchSpawn: TNode = null;
	var nodeSearchOffset = 0;
	var nodeSearchLast = "";
	var lastCanvas: TNodeCanvas;

	public var grid: Image = null;
	public var hwnd = Id.handle();

	public function new() {
		inst = this;

		Nodes.excludeRemove.push("OUTPUT_MATERIAL_PBR");
		Nodes.excludeRemove.push("BrushOutputNode");
		Nodes.onLinkDrag = onLinkDrag;

		var scale = Config.raw.window_scale;
		ui = new Zui({font: App.font, theme: App.theme, color_wheel: App.colorWheel, scaleFactor: scale});
		ui.scrollEnabled = false;
	}

	function onLinkDrag(linkDrag: TNodeLink, isNewLink: Bool) {
		if (isNewLink) {
			nodeSearch(-1, -1, function() {
				var n = getNodes().nodesSelected[0];
				if (linkDrag.to_id == -1 && n.inputs.length > 0) {
					linkDrag.to_id = n.id;
					linkDrag.to_socket = 0;
					getCanvas().links.push(linkDrag);
				}
				else if (linkDrag.from_id == -1 && n.outputs.length > 0) {
					linkDrag.from_id = n.id;
					linkDrag.from_socket = 0;
					getCanvas().links.push(linkDrag);
				}
			});
		}
	}

	public function getCanvas(): TNodeCanvas {
		if (canvasType == CanvasMaterial) return getCanvasMaterial();
		else return Context.brush.canvas;
	}

	public function getCanvasMaterial(): TNodeCanvas {
		var isScene = UIHeader.inst.worktab.position == SpaceRender;
		return isScene ? Context.materialScene.canvas : Context.material.canvas;
	}

	public function getNodes(): Nodes {
		var isScene = UIHeader.inst.worktab.position == SpaceRender;
		if (canvasType == CanvasMaterial) return isScene ? Context.materialScene.nodes : Context.material.nodes;
		else return Context.brush.nodes;
	}

	public function update() {
		var mouse = Input.getMouse();
		mreleased = mouse.released();
		mdown = mouse.down();

		// Recompile material on change
		if (ui.changed) {
			mchanged = true;
			if (!mdown) changed = true;
		}
		if ((mreleased && mchanged) || changed) {
			mchanged = changed = false;
			canvasChanged();
			if (mreleased) {
				UISidebar.inst.hwnd.redraws = 2;
				History.editNodes(lastCanvas, canvasType);
			}
		}
		else if (ui.changed && (mstartedlast || mouse.moved) && Config.raw.material_live) {
			recompileMat = true; // Instant preview
		}
		mstartedlast = mouse.started();

		if (!show) return;
		if (!App.uiEnabled) return;
		var kb = Input.getKeyboard();

		if (defaultWindowW == 0) defaultWindowW = Std.int(iron.App.w() / 2);
		if (defaultWindowH == 0) defaultWindowH = Std.int(iron.App.h() / 2);

		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		wy = UIHeader.inst.headerh * 2;
		if (UIView2D.inst.show) {
			wy += iron.App.h() - defaultWindowH;
		}
		var ww = defaultWindowW;
		var mx = mouse.x;
		var my = mouse.y;
		if (mx < wx || mx > wx + ww || my < wy) return;
		if (ui.isTyping) return;

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
					if (n.name.toLowerCase().indexOf(search) >= 0) {
						ui.t.BUTTON_COL = count == nodeSearchOffset ? ui.t.HIGHLIGHT_COL : ui.t.WINDOW_BG_COL;
						if (ui.button(n.name, Left) || (enter && count == nodeSearchOffset)) {
							var nodes = getNodes();
							var canvas = getCanvas();
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
		var ww = defaultWindowW;
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
				MaterialParser.parseBrush();
				Context.parseBrushInputs();
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
			MaterialParser.parsePaintMaterial();
			if (Layers.isFillMaterial()) {
				RenderUtil.makeMaterialPreview();
			}
			var decal = Context.tool == ToolDecal || Context.tool == ToolText;
			if (decal) RenderUtil.makeDecalPreview();

			UISidebar.inst.hwnd.redraws = 2;
			recompileMatFinal = false;
		}

		if (!show) return;
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (!App.uiEnabled && ui.inputRegistered) ui.unregisterInput();
		if (App.uiEnabled && !ui.inputRegistered) ui.registerInput();

		if (ui.inputStarted) {
			lastCanvas = Json.parse(Json.stringify(getCanvas()));
		}

		g.end();

		if (grid == null) drawGrid();

		// Start with UI
		ui.begin(g);

		// Make window
		ww = defaultWindowW;
		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		wy = UIHeader.inst.headerh * 2;
		var ew = Std.int(ui.ELEMENT_W() * 0.7);
		wh = iron.App.h();
		if (UIView2D.inst.show) {
			wh = defaultWindowH;
			wy = iron.App.h() - defaultWindowH + UIHeader.inst.headerh * 2;
		}
		if (ui.window(hwnd, wx, wy, ww, wh)) {

			// Grid
			var nodes = getNodes();
			ui.g.color = 0xffffffff;
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE()) % 100 - 100, (nodes.panY * nodes.SCALE()) % 100 - 100);

			// Nodes
			var c = getCanvas();
			nodes.nodeCanvas(ui, c);

			// Node previews
			if (nodes.nodesSelected.length > 0) {
				var img: kha.Image = null;
				var sel = nodes.nodesSelected[0];
				if (sel.type == "TEX_IMAGE") {
					var id = sel.buttons[0].default_value;
					if (id < Project.assets.length) {
						img = UISidebar.inst.getImage(Project.assets[id]);
					}
				}
				else if (sel.type == "LAYER") {
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
				if (img != null) {
					var tw = 64 * ui.SCALE();
					var th = tw * (img.height / img.width);
					var tx = ww - tw - 8 * ui.SCALE();
					var ty = wh - th - 40 * ui.SCALE();

					#if kha_opengl
					var invertY = sel.type == "MATERIAL";
					#else
					var invertY = false;
					#end

					invertY ?
						ui.g.drawScaledImage(img, tx, ty + th, tw, -th) :
						ui.g.drawScaledImage(img, tx, ty, tw, th);
				}
			}

			// Editable canvas name
			var ACCENT_COL = ui.t.ACCENT_COL;
			var BUTTON_H = ui.t.BUTTON_H;
			var ELEMENT_H = ui.t.ELEMENT_H;
			var FONT_SIZE = ui.fontSize;
			ui.t.ACCENT_COL = 0x00000000;
			ui.t.BUTTON_H = 30;
			ui.t.ELEMENT_H = 30;
			ui.fontSize = Std.int(22 * ui.SCALE());
			ui._x = ww - ui.ELEMENT_W() * 1.4;
			ui._y = wh - ui.ELEMENT_H() * 1.2;
			ui._w = Std.int(ui.ELEMENT_W() * 1.4);
			var h = Id.handle();
			h.text = c.name;
			c.name = ui.textInput(h, "", Right);
			ui.t.ACCENT_COL = ACCENT_COL;
			ui.t.BUTTON_H = BUTTON_H;
			ui.t.ELEMENT_H = ELEMENT_H;
			ui.fontSize = FONT_SIZE;

			// Menu
			ui.g.color = ui.t.WINDOW_BG_COL;
			ui.g.fillRect(0, 0, ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET());
			ui.g.color = 0xffffffff;

			ui._x = 0;
			ui._y = 0;
			ui._w = ew;

			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;

			var cats = canvasType == CanvasMaterial ? NodesMaterial.categories : NodesBrush.categories;
			for (i in 0...cats.length) {
				if ((ui.button(cats[i], Left) && UISidebar.inst.ui.comboSelectedHandle == null) || (ui.isHovered && drawMenu)) {
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

			ui.t.BUTTON_COL = BUTTON_COL;
		}

		ui.end(!drawMenu);

		g.begin(false);

		if (drawMenu) {
			var list = canvasType == CanvasMaterial ? NodesMaterial.list : NodesBrush.list;
			var numNodes = list[menuCategory].length;

			var ph = numNodes * ui.t.ELEMENT_H * ui.SCALE();
			var py = popupY;
			g.color = ui.t.WINDOW_BG_COL;
			var menuw = Std.int(ew * 2.0);
			g.fillRect(popupX, py, menuw, ph);

			ui.beginRegion(g, Std.int(popupX), Std.int(py), menuw);
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;

			for (n in list[menuCategory]) {
				if (ui.button("      " + n.name, Left)) {
					var canvas = getCanvas();
					var nodes = getNodes();
					var node = makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					nodes.nodesSelected = [node];
					nodes.nodesDrag = true;
				}
			}

			ui.t.BUTTON_COL = BUTTON_COL;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
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

	public function acceptAssetDrag(assetIndex: Int) {
		var n = canvasType == CanvasMaterial ? NodesMaterial.createNode("TEX_IMAGE") : NodesBrush.createNode("TEX_IMAGE");
		n.buttons[0].default_value = assetIndex;
		getNodes().nodesSelected = [n];
	}

	public function acceptLayerDrag(layerIndex: Int) {
		var n = NodesMaterial.createNode(Context.layerIsMask ? "LAYER_MASK" : "LAYER");
		n.buttons[0].default_value = layerIndex;
		getNodes().nodesSelected = [n];
	}

	public function acceptMaterialDrag(layerIndex: Int) {
		var n = NodesMaterial.createNode("MATERIAL");
		n.buttons[0].default_value = layerIndex;
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
}
