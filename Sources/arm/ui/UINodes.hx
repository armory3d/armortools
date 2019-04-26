package arm.ui;

import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.ShaderData.ShaderContext;
import iron.data.MaterialData;
import armory.system.Cycles;
import armory.system.CyclesFormat;
import armory.system.CyclesShader;
import armory.system.CyclesShader.CyclesShaderContext;
import arm.creator.*;
import arm.util.*;
import arm.ui.UITrait;

@:access(zui.Zui)
class UINodes extends iron.Trait {

	public static var inst:UINodes;
	static var materialCounter = 0;

	public var show = false;
	public var wx:Int;
	public var wy:Int;
	public var ww:Int;

	public var ui:Zui;
	var drawMenu = false;
	var showMenu = false;
	var hideMenu = false;
	var menuCategory = 0;
	var addNodeButton = false;
	var popupX = 0.0;
	var popupY = 0.0;

	var sc:ShaderContext = null;
	public var _matcon:TMaterialContext = null;
	var _materialcontext:MaterialContext = null;

	public var nodes:Nodes;
	public var canvas:TNodeCanvas = null;
	public var canvasMap:Map<MaterialSlot, TNodeCanvas> = null;
	public var canvasMap2:Map<MaterialSlot, TNodeCanvas> = null;
	var canvasBlob:String;

	public var canvasBrush:TNodeCanvas = null;
	public var canvasBrushMap:Map<BrushSlot, TNodeCanvas> = null;
	var canvasBrushBlob:String;

	public var canvasLogic:TNodeCanvas = null;
	public var canvasLogicMap:Map<BrushSlot, TNodeCanvas> = null;
	var canvasLogicBlob:String;

	public var canvasType = 0; // material, brush, logic

	var mdown = false;
	var mreleased = false;
	var mchanged = false;
	var mstartedlast = false;
	public var changed = false;
	var recompileMat = false; // Mat preview

	public var grid:kha.Image = null;

	public var hwnd = Id.handle();
	var lastT:iron.Trait = null;

	public function new() {
		super();
		inst = this;

		Nodes.excludeRemove.push("OUTPUT_MATERIAL_PBR");

		// Cycles.arm_export_tangents = false;

		iron.data.Data.getBlob('default_material.json', function(b1:kha.Blob) {
			iron.data.Data.getBlob('default_brush.json', function(b2:kha.Blob) {
				iron.data.Data.getBlob('default_logic.json', function(b3:kha.Blob) {
					iron.data.Data.getBlob('logic_nodes.json', function(bnodes:kha.Blob) {

						canvasBlob = b1.toString();
						canvasBrushBlob = b2.toString();
						canvas = haxe.Json.parse(canvasBlob);
						canvasBrush = haxe.Json.parse(canvasBrushBlob);
						parseBrush();
						canvasLogicBlob = b3.toString();
						canvasLogic = haxe.Json.parse(canvasLogicBlob);

						NodeCreatorLogic.list = haxe.Json.parse(bnodes.toString());

						var t = Reflect.copy(arm.App.theme);
						t.ELEMENT_H = 18;
						t.BUTTON_H = 16;
						var scale = armory.data.Config.raw.window_scale;
						ui = new Zui({font: arm.App.font, theme: t, color_wheel: arm.App.color_wheel, scaleFactor: scale});
						ui.scrollEnabled = false;
					});
				});
			});
		});
	}

	public function updateCanvasMap() {

		if (UITrait.inst.worktab.position == 1) {
			if (UITrait.inst.selectedMaterial2 != null) {
				if (canvasMap2 == null) canvasMap2 = new Map();
				var c = canvasMap2.get(UITrait.inst.selectedMaterial2);
				if (c == null) {
					c = haxe.Json.parse(canvasBlob);
					canvasMap2.set(UITrait.inst.selectedMaterial2, c);
					canvas = c;
				}
				else canvas = c;

				if (canvasType == 0) nodes = UITrait.inst.selectedMaterial2.nodes;
			}
			return;
		}

		if (UITrait.inst.selectedMaterial != null) {
			if (canvasMap == null) canvasMap = new Map();
			var c = canvasMap.get(UITrait.inst.selectedMaterial);
			if (c == null) {
				c = haxe.Json.parse(canvasBlob);
				canvasMap.set(UITrait.inst.selectedMaterial, c);
				canvas = c;
				c.name = "Material " + (++materialCounter);
			}
			else canvas = c;

			if (canvasType == 0) nodes = UITrait.inst.selectedMaterial.nodes;
		}
	}

	public function updateCanvasBrushMap() {
		if (UITrait.inst.selectedBrush != null) {
			if (canvasBrushMap == null) canvasBrushMap = new Map();
			var c = canvasBrushMap.get(UITrait.inst.selectedBrush);
			if (c == null) {
				c = haxe.Json.parse(canvasBrushBlob);
				canvasBrushMap.set(UITrait.inst.selectedBrush, c);
				canvasBrush = c;
			}
			else canvasBrush = c;

			if (canvasType == 1) nodes = UITrait.inst.selectedBrush.nodes;
		}
	}

	public function updateCanvasLogicMap() {
		if (UITrait.inst.selectedLogic != null) {
			if (canvasLogicMap == null) canvasLogicMap = new Map();
			var c = canvasLogicMap.get(UITrait.inst.selectedLogic);
			if (c == null) {
				c = haxe.Json.parse(canvasLogicBlob);
				canvasLogicMap.set(UITrait.inst.selectedLogic, c);
				canvasLogic = c;
			}
			else canvasLogic = c;

			if (canvasType == 2) nodes = UITrait.inst.selectedLogic.nodes;
		}
	}

	function getCanvas() {
		if (canvasType == 0) return canvas;
		else if (canvasType == 1) return canvasBrush;
		else return canvasLogic;
	}

	function update() {
		updateCanvasMap();
		updateCanvasBrushMap();
		updateCanvasLogicMap();

		var mouse = iron.system.Input.getMouse();
		mreleased = mouse.released();
		mdown = mouse.down();

		// Recompile material on change
		if (ui.changed) {
			mchanged = true;
			if (!mdown) changed = true;
			if (canvasType == 1) parseBrush();
			else if (canvasType == 2) parseLogic();
		}
		if ((mreleased && mchanged) || changed) {
			mchanged = changed = false;
			if (canvasType == 0) {
				if (Layers.isFillMaterial()) {
					Layers.updateFillLayers(1); // TODO: jitter
					UITrait.inst.hwnd.redraws = 2;
				}
				parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
				UITrait.inst.hwnd1.redraws = 2;
				var decal = UITrait.inst.selectedTool == ToolDecal || UITrait.inst.selectedTool == ToolText;
				if (decal) RenderUtil.makeDecalPreview();
			}
		}
		else if (ui.changed && (mstartedlast || mouse.moved)) {
			recompileMat = true; // Instant preview
		}
		mstartedlast = mouse.started();

		if (!show) return;
		if (!arm.App.uienabled) return;
		var keyboard = iron.system.Input.getKeyboard();

		var lay = UITrait.inst.C.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wx += UITrait.inst.toolbarw;
		wy = UITrait.inst.headerh * 2;
		var mx = mouse.x + App.x();
		var my = mouse.y + App.y();
		if (mx < wx || my < wy) return;
		if (ui.isTyping) return;

		if (addNodeButton) {
			showMenu = true;
			addNodeButton = false;
		}
		else if (mouse.released()) {
			hideMenu = true;
		}

		// if (keyboard.started("p")) {
		// 	var c = getCanvas();
		// 	var str = haxe.Json.stringify(c);
		// 	trace(str.substr(0, 1023));
		// 	trace(str.substr(1023, 2047));
		// }
	}

	public function getNodeX():Int {
		var mouse = iron.system.Input.getMouse();
		return Std.int((mouse.x + App.x() - wx - nodes.PAN_X()) / nodes.SCALE);
	}

	public function getNodeY():Int {
		var mouse = iron.system.Input.getMouse();
		return Std.int((mouse.y + App.y() - wy - nodes.PAN_Y()) / nodes.SCALE);
	}

	public function drawGrid() {
		var ww = iron.App.w() + UITrait.inst.toolbarw;
		var wh = iron.App.h();
		var w = ww + 40 * 2;
		var h = wh + 40 * 2;
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		grid = kha.Image.createRenderTarget(w, h);
		grid.g2.begin(true, ui.t.SEPARATOR_COL);
		for (i in 0...Std.int(h / 40) + 1) {
			grid.g2.color = ui.t.WINDOW_BG_COL;
			grid.g2.drawLine(0, i * 40 + 20, w, i * 40 + 20);
		}
		for (i in 0...Std.int(w / 40) + 1) {
			grid.g2.color = ui.t.WINDOW_BG_COL;
			grid.g2.drawLine(i * 40 + 20, 0, i * 40 + 20, h);
		}
		grid.g2.end();
	}

	function render2D(g:kha.graphics2.Graphics) {
		if (recompileMat) {
			recompileMat = false;
			if (Layers.isFillMaterial()) {
				Layers.updateFillLayers();
			}
			else {
				RenderUtil.makeMaterialPreview();
			}
			UITrait.inst.hwnd1.redraws = 2;
		}

		if (!show) return;
		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;
		
		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();
		
		g.end();

		if (grid == null) drawGrid();

		// Start with UI
		ui.begin(g);
		
		// Make window
		ww = Std.int(iron.App.w()) + UITrait.inst.toolbarw;
		var lay = UITrait.inst.C.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wx += UITrait.inst.toolbarw;
		wy = UITrait.inst.headerh * 2;
		var ew = Std.int(ui.ELEMENT_W() * 0.7);
		var wh = iron.App.h();
		if (UIView2D.inst.show) {
			wh = Std.int(iron.App.h() / 2);
			wy += Std.int(iron.App.h() / 2);
		}
		if (ui.window(hwnd, wx, wy, ww, wh)) {
			
			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE) % 40 - 40, (nodes.panY * nodes.SCALE) % 40 - 40);

			// Nodes
			var c = getCanvas();
			nodes.nodeCanvas(ui, c);

			// Image node preview
			if (nodes.nodesSelected.length > 0 && nodes.nodesSelected[0].type == 'TEX_IMAGE') {
				var id = nodes.nodesSelected[0].buttons[0].default_value;
				if (id < UITrait.inst.assets.length) {
					var img = UITrait.inst.getImage(UITrait.inst.assets[id]);
					ui.g.drawScaledImage(img, ww - 64 - 20, wh - 64 - 40, 64, 64);
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
			ui.fontSize = Std.int(22 * ui.SCALE);
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
			ui.g.fillRect(0, 0, ww, 24 * ui.SCALE);
			ui.g.color = 0xffffffff;

			ui._x = 3;
			ui._y = 3;
			ui._w = ew;

			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;

			if (canvasType == 1) {
				if (ui.button("Nodes", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
			}
			else if (canvasType == 2) {
				if (ui.button("Action", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Animation", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 1; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Array", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 2; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Canvas", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 3; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Event", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 4; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Input", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 5; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x = 3;
				ui._y = 30;
				if (ui.button("Logic", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 6; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x = ew + 3;
				ui._y = 30;
				if (ui.button("Native", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 7; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Navmesh", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 8; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Physics", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 9; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Sound", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 10; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Value", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 11; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Variable", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 12; popupX = wx + ui._x; popupY = wy + ui._y; }
			}
			else {
				if (ui.button("Input", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				// if (ui.button("Output", Left)) { addNodeButton = true; menuCategory = 1; popupX = wx + ui._x; popupY = wy + ui._y; }
				// ui._x += ew + 3;
				// ui._y = 3;
				if (ui.button("Texture", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 2; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Color", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 3; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Vector", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 4; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Converter", Left) || (ui.isHovered && drawMenu)) { addNodeButton = true; menuCategory = 5; popupX = wx + ui._x; popupY = wy + ui._y; }
			}

			ui.t.BUTTON_COL = BUTTON_COL;
		}

		ui.endWindow();

		if (drawMenu) {
			var numNodes = 0;
			if (canvasType == 0) numNodes = NodeCreator.numNodes[menuCategory];
			else if (canvasType == 1) numNodes = NodeCreatorBrush.numNodes[menuCategory];
			else if (canvasType == 2) numNodes = NodeCreatorLogic.list.categories[menuCategory].nodes.length;
			var ph = numNodes * 20 * ui.SCALE;
			var py = popupY;
			g.color = ui.t.WINDOW_BG_COL;
			var menuw = Std.int(ew * 1.6);
			g.fillRect(popupX, py, menuw, ph);

			ui.beginLayout(g, Std.int(popupX), Std.int(py), menuw);
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var ELEMENT_H = ui.t.ELEMENT_H;
			ui.t.ELEMENT_H = 22;

			if (canvasType == 0) NodeCreator.draw(menuCategory);
			else if (canvasType == 1) NodeCreatorBrush.draw(menuCategory);
			else if (canvasType == 2) NodeCreatorLogic.draw(menuCategory);

			ui.t.BUTTON_COL = BUTTON_COL;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.ELEMENT_H = ELEMENT_H;
			ui.endLayout();
		}

		ui.end();

		g.begin(false);

		if (showMenu) {
			showMenu = false;
			drawMenu = true;
			
		}
		if (hideMenu) {
			hideMenu = false;
			drawMenu = false;
		}
	}

	function getMOut():Bool {
		for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	public function parseMeshMaterial() {
		if (UITrait.inst.worktab.position == 1 || UITrait.inst.worktab.position == 2) return;
		var m = UITrait.inst.materials[0].data;
		// iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
			var sc:ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
			if (sc != null) {
				m.shader.raw.contexts.remove(sc.raw);
				m.shader.contexts.remove(sc);
			}
			var con = MaterialBuilder.make_mesh(new CyclesShaderData({name: "Material", canvas: null}));
			if (sc != null) sc.delete();
			sc = new ShaderContext(con.data, function(sc:ShaderContext){});
			m.shader.raw.contexts.push(sc.raw);
			m.shader.contexts.push(sc);
			UITrait.inst.ddirty = 2;
		// });
	}

	public function parseParticleMaterial() {
		var m = UITrait.inst.particleMaterial;
		// iron.data.Data.getMaterial("Scene", "MaterialParticle", function(m:iron.data.MaterialData) {
			var sc:ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
			if (sc != null) {
				m.shader.raw.contexts.remove(sc.raw);
				m.shader.contexts.remove(sc);
			}
			var con = MaterialBuilder.make_particle(new CyclesShaderData({name: "MaterialParticle", canvas: null}));
			if (sc != null) sc.delete();
			sc = new ShaderContext(con.data, function(sc:ShaderContext){});
			m.shader.raw.contexts.push(sc.raw);
			m.shader.contexts.push(sc);
		// });
	}

	public function parseMeshPreviewMaterial() {
		if (!getMOut()) return;

		if (UITrait.inst.worktab.position == 2) { // Material
			UITrait.inst.rdirty = 1;
		}

		var m = UITrait.inst.worktab.position == 1 ? UITrait.inst.selectedMaterial2.data : UITrait.inst.materials[0].data;
		// iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {

			var sc:ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
			m.shader.raw.contexts.remove(sc.raw);
			m.shader.contexts.remove(sc);
			
			var matcon:TMaterialContext = { name: "mesh", bind_textures: [] };

			var sd = new CyclesShaderData({name: "Material", canvas: null});
			var con = MaterialBuilder.make_mesh_preview(sd, matcon);

			for (i in 0...m.contexts.length) {
				if (m.contexts[i].raw.name == "mesh") {
					m.contexts[i] = new MaterialContext(matcon, function(self:MaterialContext) {});
					break;
				}
			}
			
			if (sc != null) sc.delete();
			
			var compileError = false;
			sc = new ShaderContext(con.data, function(sc:ShaderContext) {
				if (sc == null) compileError = true;
			});
			if (compileError) return;

			m.shader.raw.contexts.push(sc.raw);
			m.shader.contexts.push(sc);

		// });
	}

	public function parsePaintMaterial() {
		if (!getMOut()) return;
		
		if (UITrait.inst.worktab.position == 1) {
			parseMeshPreviewMaterial();
			return;
		}

		//
		var m = UITrait.inst.materials[0].data;
		sc = null;
		_materialcontext = null;
		//
		// iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
		
			var mat:TMaterial = {
				name: "Material",
				canvas: canvas
			};
			var _sd = new CyclesShaderData(mat);

			if (sc == null) {
				for (c in m.shader.contexts) {
					if (c.raw.name == "paint") {
						sc = c;
						break;
					}
				}
			}
			if (_materialcontext == null) {
				for (c in m.contexts) {
					if (c.raw.name == "paint") {
						_materialcontext = c;
						_matcon = c.raw;
						break;
					}
				}
			}

			if (sc != null) {
				m.shader.raw.contexts.remove(sc.raw);
				m.shader.contexts.remove(sc);
			}
			if (_materialcontext != null) {
				m.raw.contexts.remove(_matcon);
				m.contexts.remove(_materialcontext);
			}

			_matcon = {
				name: "paint",
				bind_textures: []
			}

			var con = MaterialBuilder.make_paint(_sd, _matcon);
			var cdata = con.data;

				// from_source is synchronous..
				if (sc != null) sc.delete();
				
				var compileError = false;
				sc = new ShaderContext(cdata, function(sc:ShaderContext) {
					if (sc == null) compileError = true;
				});
				if (compileError) return;
				
				m.shader.raw.contexts.push(sc.raw);
				m.shader.contexts.push(sc);
				m.raw.contexts.push(_matcon);

				new MaterialContext(_matcon, function(self:MaterialContext) {
					_materialcontext = self;
					m.contexts.push(self);
				});

				var dcon = MaterialBuilder.make_depth(_sd, _matcon);
				var dcdata = dcon.data;
				// from_source is synchronous..
				var dsc = new ShaderContext(dcdata, function(sc:ShaderContext){});
				m.shader.contexts.push(dsc);
				var dmatcon:TMaterialContext = {
					name: "depth"
				}
				m.raw.contexts.push(dmatcon);
				new MaterialContext(dmatcon, function(self:MaterialContext) {
					m.contexts.push(self);
				});
		// });
	}

	public function acceptDrag(assetIndex:Int) {
		var n = NodeCreator.createImageTexture();
		n.buttons[0].default_value = assetIndex;
		nodes.nodesSelected = [n];
	}

	public function parseBrush() {
		armory.system.Logic.packageName = "arm.brushnode";
		var tree = armory.system.Logic.parse(canvasBrush, false);
	}

	public function parseLogic() {
		if (lastT != null) UITrait.inst.selectedObject.removeTrait(lastT);
		armory.system.Logic.packageName = "armory.logicnode";
		var t = armory.system.Logic.parse(canvasLogic);
		lastT = t;
		UITrait.inst.selectedObject.addTrait(t);
	}
}
