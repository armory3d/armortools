package arm;

import armory.system.Cycles;
import armory.system.CyclesFormat;
import armory.system.CyclesShader;
import armory.system.CyclesShader.CyclesShaderContext;
import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.ShaderData.ShaderContext;
import iron.data.MaterialData;

@:access(zui.Zui)
class UINodes extends iron.Trait {

	public static var inst:UINodes;

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

	public var nodes = new Nodes();
	public var canvas:TNodeCanvas = null;
	var canvasMap:Map<UITrait.MaterialSlot, TNodeCanvas> = null;
	#if arm_editor
	var canvasMap2:Map<UITrait.MaterialSlot, TNodeCanvas> = null;
	#end
	var canvasBlob:String;

	public var canvasBrush:TNodeCanvas = null;
	var canvasBrushMap:Map<UITrait.BrushSlot, TNodeCanvas> = null;
	var canvasBrushBlob:String;

	public var canvasLogic:TNodeCanvas = null;
	var canvasLogicMap:Map<UITrait.BrushSlot, TNodeCanvas> = null;
	var canvasLogicBlob:String;

	public var canvasType = 0; // material, brush, logic

	var mdown = false;
	var mreleased = false;
	var mchanged = false;
	var mstartedlast = false;
	public var changed = false;
	var recompileMat = false; // Mat preview

	public var grid:kha.Image = null;

	public function new() {
		super();
		inst = this;

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
						
						notifyOnRender2D(render2D);
						notifyOnUpdate(update);
					});
				});
			});
		});
	}

	public function updateCanvasMap() {

		#if arm_editor
		if (UITrait.inst.htab.position == 0) {
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
		#end

		if (UITrait.inst.selectedMaterial != null) {
			if (canvasMap == null) canvasMap = new Map();
			var c = canvasMap.get(UITrait.inst.selectedMaterial);
			if (c == null) {
				c = haxe.Json.parse(canvasBlob);
				canvasMap.set(UITrait.inst.selectedMaterial, c);
				canvas = c;
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
				parsePaintMaterial();
				UITrait.inst.makeMaterialPreview();
				if (UITrait.inst.brushPaint == 2) UITrait.inst.makeStickerPreview();
			}
		}
		else if (ui.changed && (mstartedlast || mouse.moved) && UITrait.inst.instantMat) {
			recompileMat = true; // Instant preview
		}
		mstartedlast = mouse.started();

		if (!show) return;
		if (!arm.App.uienabled) return;
		var keyboard = iron.system.Input.getKeyboard();

		var lay = UITrait.inst.apconfig.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wy = 0;
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

		if (keyboard.started("x") || keyboard.started("backspace")) {
			if (nodes.nodeSelected != null && nodes.nodeSelected.type != "OUTPUT_MATERIAL_PBR") {
				var c = getCanvas();
				nodes.removeNode(nodes.nodeSelected, c);
				changed = true;
			}
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
		var ww = iron.App.w();
		var wh = iron.App.h();
		var w = ww + 40 * 2;
		var h = wh + 40 * 2;
		grid = kha.Image.createRenderTarget(w, h);
		grid.g2.begin(true, 0xff242424);
		for (i in 0...Std.int(h / 40) + 1) {
			grid.g2.color = 0xff282828;
			grid.g2.drawLine(0, i * 40, w, i * 40);
			grid.g2.color = 0xff323232;
			grid.g2.drawLine(0, i * 40 + 20, w, i * 40 + 20);
		}
		for (i in 0...Std.int(w / 40) + 1) {
			grid.g2.color = 0xff282828;
			grid.g2.drawLine(i * 40, 0, i * 40, h);
			grid.g2.color = 0xff323232;
			grid.g2.drawLine(i * 40 + 20, 0, i * 40 + 20, h);
		}
		grid.g2.end();
	}

	public var hwnd = Id.handle();

	function render2D(g:kha.graphics2.Graphics) {
		if (recompileMat) {
			recompileMat = false;
			if (UITrait.inst.autoFillHandle.selected) {
				parsePaintMaterial();
			}
			else {
				UITrait.inst.makeMaterialPreview();
				UITrait.inst.hwnd.redraws = 2;
			}
		}

		if (!show) return;
		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;
		
		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();
		
		g.end();

		if (grid == null) drawGrid();

		// Start with UI
		ui.begin(g);
		// ui.begin(rt.g2); ////
		
		// Make window
		ww = Std.int(iron.App.w());
		var lay = UITrait.inst.apconfig.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wy = 0;
		var ew = Std.int(ui.ELEMENT_W());
		if (ui.window(hwnd, wx, wy, ww, iron.App.h())) {
			
			ui.g.color = 0xffffffff;
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE) % 40 - 40, (nodes.panY * nodes.SCALE) % 40 - 40);

			ui.g.font = arm.App.font;
			ui.g.fontSize = 22;
			var title = canvasType == 1 ? "Brush" : "Material";
			var titlew = ui.g.font.width(22, title);
			var titleh = ui.g.font.height(22);
			ui.g.drawString(title, ww - titlew - 20, iron.App.h() - titleh - 10);
			
			var c = getCanvas();
			nodes.nodeCanvas(ui, c);

			ui.g.color = ui.t.WINDOW_BG_COL;
			ui.g.fillRect(0, 0, ww, 24);
			ui.g.color = 0xffffffff;

			ui._x = 3;
			ui._y = 3;
			ui._w = ew;

			if (canvasType == 1) {
				if (ui.button("Nodes")) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
			}
			else if (canvasType == 2) {
				if (ui.button("Action")) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Animation")) { addNodeButton = true; menuCategory = 1; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Array")) { addNodeButton = true; menuCategory = 2; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Canvas")) { addNodeButton = true; menuCategory = 3; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Event")) { addNodeButton = true; menuCategory = 4; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Input")) { addNodeButton = true; menuCategory = 5; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x = 3;
				ui._y = 30;
				if (ui.button("Logic")) { addNodeButton = true; menuCategory = 6; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x = ew + 3;
				ui._y = 30;
				if (ui.button("Native")) { addNodeButton = true; menuCategory = 7; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Navmesh")) { addNodeButton = true; menuCategory = 8; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Physics")) { addNodeButton = true; menuCategory = 9; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Sound")) { addNodeButton = true; menuCategory = 10; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Value")) { addNodeButton = true; menuCategory = 11; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 30;
				if (ui.button("Variable")) { addNodeButton = true; menuCategory = 12; popupX = wx + ui._x; popupY = wy + ui._y; }
			}
			else {
				if (ui.button("Input")) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				// if (ui.button("Output")) { addNodeButton = true; menuCategory = 1; popupX = wx + ui._x; popupY = wy + ui._y; }
				// ui._x += ew + 3;
				// ui._y = 3;
				if (ui.button("Texture")) { addNodeButton = true; menuCategory = 2; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Color")) { addNodeButton = true; menuCategory = 3; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Vector")) { addNodeButton = true; menuCategory = 4; popupX = wx + ui._x; popupY = wy + ui._y; }
				ui._x += ew + 3;
				ui._y = 3;
				if (ui.button("Converter")) { addNodeButton = true; menuCategory = 5; popupX = wx + ui._x; popupY = wy + ui._y; }
			}
		}

		ui.endWindow();

		if (drawMenu) {
			
			var numNodes = 0;
			if (canvasType == 0) numNodes = NodeCreator.numNodes[menuCategory];
			else if (canvasType == 1) numNodes = NodeCreatorBrush.numNodes[menuCategory];
			else if (canvasType == 2) numNodes = NodeCreatorLogic.list.categories[menuCategory].nodes.length;
			var ph = numNodes * 20;
			var py = popupY;
			g.color = 0xff222222;
			g.fillRect(popupX, py, ew, ph);

			ui.beginLayout(g, Std.int(popupX), Std.int(py), ew);
			
			if (canvasType == 0) NodeCreator.draw(menuCategory);
			else if (canvasType == 1) NodeCreatorBrush.draw(menuCategory);
			else if (canvasType == 2) NodeCreatorLogic.draw(menuCategory);

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

	function make_paint(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var layered = UITrait.inst.layers.length > 1 && UITrait.inst.selectedLayer == UITrait.inst.layers[1];
		var eraser = UITrait.inst.brushType == 1;
		var context_id = 'paint';
		var con_paint:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: 'always',
			cull_mode: 'counter_clockwise',
			blend_source: layered ? 'blend_one' : 'source_alpha',
			blend_destination: layered ? 'blend_zero' : 'inverse_source_alpha',
			blend_operation: 'add',
			alpha_blend_source: 'blend_one',
			alpha_blend_destination: eraser ? 'blend_zero' : 'blend_one',
			alpha_blend_operation: 'add',
			vertex_structure: [{name: "pos", size: 3},{name: "nor", size: 3},{name: "tex", size: 2}] });

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];

		if (UITrait.inst.brushType == 3) { // Bake AO
			con_paint.data.color_writes_green[2] = false; // No rough
			con_paint.data.color_writes_blue[2] = false; // No met
		}

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();

		vert.add_out('vec3 sp');
		frag.ins = vert.outs;
		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, tex.y * 2.0 - 1.0);');

		// TODO: Fix seams at uv borders
		vert.add_uniform('vec2 sub', '_sub');
		vert.add_uniform('float paintDepthBias', '_paintDepthBias');
		vert.write('tpos += sub;');
		
		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');
		
		vert.write_attrib('vec4 ndc = WVP * vec4(pos, 1.0);');
		vert.write_attrib('ndc.xyz = ndc.xyz / ndc.w;');
		vert.write('sp.xyz = ndc.xyz * 0.5 + 0.5;');
		vert.write('sp.y = 1.0 - sp.y;');
		vert.write('sp.z -= paintDepthBias;'); // small bias or !paintVisible

		if (UITrait.inst.brushPaint != 0) frag.ndcpos = true;

		if (UITrait.inst.brushType == 3) { // Bake ao
			frag.wposition = true;
			frag.n = true;
		}

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');
		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D paintdb');

		if (UITrait.inst.brushType == 4) { // Pick color id
			frag.add_out('vec4 fragColor');

			frag.write('if (sp.z > texture(paintdb, vec2(sp.x, 1.0 - bsp.y)).r) discard;');
			frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
			frag.write('binp.x *= aspectRatio;');
			frag.write('binp = binp * 0.5 + 0.5;');
			
			frag.write('float dist = distance(bsp.xy, binp.xy);');
			frag.write('if (dist > 0.025) discard;'); // Base this on camera zoom - more for zommed in camera, less for zoomed out

			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = fract(tex);'); // TODO: fract(tex) - somehow clamp is set after first paint
			frag.write('vec3 idcol = texture(texcolorid, texCoord).rgb;');
			frag.write('fragColor = vec4(idcol, 1.0);');

			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}

		var numTex = UITrait.inst.paintHeight ? 4 : 3;
		frag.add_out('vec4 fragColor[$numTex]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushStrength', '_brushStrength');

		if (UITrait.inst.brushType == 0 || UITrait.inst.brushType == 1) { // Draw / Erase
			
			frag.write('if (sp.z > texture(paintdb, vec2(sp.x, 1.0 - bsp.y)).r) { discard; return; }');
			
			if (UITrait.inst.brushPaint == 2) { // Sticker
				frag.write('float dist = 0.0;');
			}
			else {
				frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
				frag.write('binp.x *= aspectRatio;');
				frag.write('binp = binp * 0.5 + 0.5;');

				// Continuos paint
				frag.write('vec2 binplast = inplast.xy * 2.0 - 1.0;');
				frag.write('binplast.x *= aspectRatio;');
				frag.write('binplast = binplast * 0.5 + 0.5;');
				
				frag.write('vec2 pa = bsp.xy - binp.xy, ba = binplast.xy - binp.xy;');
				frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
				frag.write('float dist = length(pa - ba * h);');
				if (!UITrait.inst.mirrorX) {
					frag.write('if (dist > brushRadius) { discard; return; }');
				}
				else {
					frag.write('vec2 binp2 = vec2(0.5 - (binp.x - 0.5), binp.y), binplast2 = vec2(0.5 - (binplast.x - 0.5), binplast.y);');
					frag.write('vec2 pa2 = bsp.xy - binp2.xy, ba2 = binplast2.xy - binp2.xy;');
					frag.write('float h2 = clamp(dot(pa2, ba2) / dot(ba2, ba2), 0.0, 1.0);');
					frag.write('float dist2 = length(pa2 - ba2 * h2);');
					frag.write('if (dist > brushRadius && dist2 > brushRadius) { discard; return; }');
				}
				//

				// frag.write('float dist = distance(bsp.xy, binp.xy);');
				// frag.write('if (dist > brushRadius) { discard; return; }');
			}
		}
		else {
			frag.write('float dist = 0.0;');
		}

		if (UITrait.inst.colorIdPicked) {
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(tex);');
			frag.add_uniform('sampler2D texpaint_colorid0'); // 1x1 picker
			frag.add_uniform('sampler2D texcolorid', '_texcolorid'); // color map
			frag.add_uniform('vec2 texcoloridSize', '_texcoloridSize'); // color map
			frag.write('vec3 c1 = texelFetch(texpaint_colorid0, ivec2(0, 0), 0).rgb;');
			frag.write('vec3 c2 = texelFetch(texcolorid, ivec2(texCoordPick * texcoloridSize), 0).rgb;');
			frag.write('if (c1 != c2) { discard; return; }');
		}

		// Texture projection - texcoords
		if (UITrait.inst.brushPaint == 0) {
			vert.add_uniform('float brushScale', '_brushScale'); // TODO: Will throw uniform not found
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = tex * brushScale;');
		}
		// Texture projection - project
		else {
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_pre = true; // When tc is fetched by normalmap
			frag.write('vec2 uvsp = sp.xy;');

			// Sticker
			if (UITrait.inst.brushPaint == 2) {
				
				frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
				frag.write('binp = binp * 0.5 + 0.5;');

				frag.write('uvsp -= binp;');
				frag.write('uvsp.x *= aspectRatio;');

				// frag.write('uvsp *= brushScale;');
				frag.write('uvsp *= vec2(7.2);');
				frag.write('uvsp += vec2(0.5);');

				frag.write('if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) { discard; return; }');
			}
			else {
				frag.write('uvsp.x *= aspectRatio;');
			}

			frag.write_pre = false;
			Cycles.texCoordName = 'fract(uvsp * brushScale)'; // TODO: use prescaled value from VS
		}

		Cycles.parse_height_as_channel = true;
		var sout = Cycles.parse(canvas, con_paint, vert, frag, null, null, null, matcon);
		Cycles.parse_height_as_channel = false;
		Cycles.texCoordName = 'texCoord';
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var nortan = Cycles.out_normaltan;
		var height = sout.out_height;
		frag.write('vec3 basecol = $base;');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('vec3 nortan = $nortan;');
		frag.write('float height = $height;');

		if (eraser) frag.write('    float str = 1.0 - brushOpacity;');
		else frag.write('    float str = clamp(brushOpacity * (brushRadius - dist) * brushStrength, 0.0, 1.0);');

		if (UITrait.inst.mirrorX && UITrait.inst.brushType == 0) { // Draw
			frag.write('str += clamp(brushOpacity * (brushRadius - dist2) * brushStrength, 0.0, 1.0);');
			frag.write('str = clamp(str, 0.0, 1.0);');
		}

		frag.write('    fragColor[0] = vec4(basecol, str);');
		frag.write('    fragColor[1] = vec4(nortan, 1.0);');
		// frag.write('    fragColor[1] = vec4(nortan, str);');
		frag.write('    fragColor[2] = vec4(occlusion, roughness, metallic, str);');
		if (UITrait.inst.paintHeight) {
			frag.write('    fragColor[3] = vec4(1.0, 0.0, height, str);');
		}

		if (!UITrait.inst.paintBase) frag.write('fragColor[0].a = 0.0;');
		if (!UITrait.inst.paintNor) frag.write('fragColor[1].a = 0.0;');
		if (!UITrait.inst.paintOcc) con_paint.data.color_writes_red[2] = false;
		if (!UITrait.inst.paintRough) con_paint.data.color_writes_green[2] = false;
		if (!UITrait.inst.paintMet) con_paint.data.color_writes_blue[2] = false;

		if (UITrait.inst.brushType == 3) { // Bake AO
			frag.write('fragColor[0].a = 0.0;');
			frag.write('fragColor[1].a = 0.0;');

			// Apply normal channel
			frag.vVec = true;
			frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(TBN * n);');

			frag.write('const vec3 voxelgiHalfExtents = vec3(2.0);');
			frag.write('vec3 voxpos = wposition / voxelgiHalfExtents;');
			frag.add_uniform('sampler3D voxels');
			frag.add_function(armory.system.CyclesFunctions.str_traceAO);
			frag.n = true;
			var strength = UITrait.inst.bakeStrength;
			var radius = UITrait.inst.bakeRadius;
			var offset = UITrait.inst.bakeOffset;
			frag.write('fragColor[2].r = 1.0 - traceAO(voxpos, n, voxels, $radius, $offset) * $strength;');
		}

		Cycles.finalize(con_paint);
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}

	function make_mesh_preview(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var context_id = 'mesh';
		var con_mesh:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			// cull_mode: 'none',
			vertex_structure: [{name: "pos", size: 3},{name: "nor", size: 3},{name: "tex", size: 2}] });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		frag.ins = vert.outs;
		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.add_out('vec2 texCoord');
		vert.write('gl_Position = WVP * vec4(pos, 1.0);');
		vert.write('texCoord = tex;');

		var sout = Cycles.parse(canvas, con_mesh, vert, frag, null, null, null, matcon);
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var opac = sout.out_opacity;
		var nortan = Cycles.out_normaltan;
		frag.write('vec3 basecol = pow($base, vec3(2.2));');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('float opacity = $opac;');
		frag.write('vec3 nortan = $nortan;');

		//if discard_transparent:
			var opac = 0.2;//mat_state.material.discard_transparent_opacity
			frag.write('if (opacity < $opac) discard;');

		frag.add_out('vec4[3] fragColor');
		frag.n = true;

		frag.add_function(armory.system.CyclesFunctions.str_packFloat);
		frag.add_function(armory.system.CyclesFunctions.str_packFloat2);
		frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
		frag.add_function(armory.system.CyclesFunctions.str_octahedronWrap);

		// Apply normal channel
		frag.vVec = true;
		frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
		frag.write('n = nortan * 2.0 - 1.0;');
		frag.write('n.y = -n.y;');
		frag.write('n = normalize(TBN * n);');

		frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		frag.write('fragColor[0] = vec4(n.xy, packFloat(metallic, roughness), 1.0 - gl_FragCoord.z);');
		frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(occlusion, 1.0));'); // occ/spec
		frag.write('fragColor[2] = vec4(0.0);'); // veloc

		Cycles.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}

	function make_depth(data:CyclesShaderData, matcon:TMaterialContext, shadowmap = false):CyclesShaderContext {
		var context_id = shadowmap ? 'shadowmap' : 'depth';
		var con_depth:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			// cull_mode: 'none',
			color_write_red: false,
			color_write_green: false,
			color_write_blue: false,
			color_write_alpha: false,
			vertex_structure: [{name: "pos", size: 3}]
		});

		var vert = con_depth.make_vert();
		var frag = con_depth.make_frag();

		frag.ins = vert.outs;

		if (shadowmap) {
			if (UITrait.inst.paintHeight) {
				con_depth.add_elem('nor', 3);
				vert.wposition = true;
				vert.n = true;
				vert.add_uniform('sampler2D texpaint_opt');
				vert.write('vec4 opt = texture(texpaint_opt, tex);');
				vert.write('float height = opt.b;');
				var displaceStrength = UITrait.inst.displaceStrength * 0.1;
				vert.write('wposition += wnormal * height * $displaceStrength;');
				
				vert.add_uniform('mat4 LVP', '_lightViewProjectionMatrix');
				vert.write('gl_Position = LVP * vec4(wposition, 1.0);');
			}
			else {
				vert.add_uniform('mat4 LWVP', '_lightWorldViewProjectionMatrix');
				vert.write('gl_Position = LWVP * vec4(pos, 1.0);');
			}
		}
		else {
			vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
			vert.write('gl_Position = WVP * vec4(pos, 1.0);');
		}

		var parse_opacity = shadowmap; // arm_discard
		if (parse_opacity) {
			frag.write_attrib('vec3 n;'); // discard at compile time
			frag.write_attrib('float dotNV;'); // discard at compile time
			// frag.write('float opacity;');

			Cycles.parse_surface = false;
			var sout = Cycles.parse(canvas, con_depth, vert, frag, null, null, null, matcon);
			Cycles.parse_surface = true;
			// parse_surface=False, parse_opacity=True
			var opac = sout.out_opacity;
			frag.write('float opacity = $opac;');
			if (con_depth.is_elem('tex')) {
				vert.add_out('vec2 texCoord');
				vert.write('texCoord = tex;');
			}
			
			var opac = 0.2;//mat_state.material.discard_transparent_opacity_shadows
			frag.write('if (opacity < $opac) discard;');
		}

		Cycles.finalize(con_depth);
		con_depth.data.shader_from_source = true;
		con_depth.data.vertex_shader = vert.get();
		con_depth.data.fragment_shader = frag.get();
		return con_depth;
	}

	function make_mesh_paint(data:CyclesShaderData):CyclesShaderContext {
		var context_id = 'mesh';
		var con_mesh:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			vertex_structure: [{name: "pos", size: 3},{name: "nor", size: 3},{name: "tex", size: 2}] });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		vert.add_out('vec2 texCoord');
		frag.wvpposition = true;
		vert.add_out('vec4 prevwvpposition');
		vert.add_uniform('mat4 VP', '_viewProjectionMatrix');
		vert.add_uniform('mat4 prevWVP', '_prevWorldViewProjectionMatrix');
		vert.wposition = true;

		// Height
		// TODO: can cause TAA issues
		if (UITrait.inst.paintHeight) {
			vert.add_uniform('sampler2D texpaint_opt');
			vert.write('vec4 opt = texture(texpaint_opt, tex);');
			vert.write('float height = opt.b;');
			var displaceStrength = UITrait.inst.displaceStrength * 0.1;
			vert.n = true;
			vert.write('wposition += wnormal * height * $displaceStrength;');
		}
		//

		vert.write('gl_Position = VP * vec4(wposition, 1.0);');
		vert.write('texCoord = tex;');
		if (UITrait.inst.paintHeight) {
			vert.add_uniform('mat4 invW', '_inverseWorldMatrix');
			vert.write('prevwvpposition = prevWVP * (invW * vec4(wposition, 1.0));');
		}
		else {
			vert.write('prevwvpposition = prevWVP * vec4(pos, 1.0);');
		}

		frag.ins = vert.outs;

		frag.add_out('vec4[3] fragColor');
		frag.n = true;
		frag.add_function(armory.system.CyclesFunctions.str_packFloat);
		frag.add_function(armory.system.CyclesFunctions.str_packFloat2);

		if (arm.UITrait.inst.brushType == 4) { // Show color map
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, packFloat(1.0, 1.0), 1.0 - gl_FragCoord.z);');
			frag.write('vec3 idcol = pow(texture(texcolorid, texCoord).rgb, vec3(2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
		}
		else {
			frag.add_function(armory.system.CyclesFunctions.str_octahedronWrap);

			frag.add_uniform('sampler2D texpaint');
			frag.add_uniform('sampler2D texpaint_nor');
			frag.add_uniform('sampler2D texpaint_pack');

			frag.write('vec3 basecol;');
			frag.write('float roughness;');
			frag.write('float metallic;');
			frag.write('float occlusion;');
			frag.write('float opacity;');
			frag.write('float specular;');

			if (UITrait.inst.layers[0].visible) {
				frag.write('basecol = pow(texture(texpaint, texCoord).rgb, vec3(2.2));');

				frag.vVec = true;
				frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
				frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
				frag.write('vec3 ntex = texture(texpaint_nor, texCoord).rgb;');
				frag.write('n = ntex * 2.0 - 1.0;');
				frag.write('n.y = -n.y;');
				frag.write('n = normalize(TBN * n);');

				// Height
				if (UITrait.inst.paintHeight) {
					frag.add_uniform('sampler2D texpaint_opt');
					frag.write('vec4 vech;');
					frag.write('vech.x = textureOffset(texpaint_opt, texCoord, ivec2(-1, 0)).b;');
					frag.write('vech.y = textureOffset(texpaint_opt, texCoord, ivec2(1, 0)).b;');
					frag.write('vech.z = textureOffset(texpaint_opt, texCoord, ivec2(0, -1)).b;');
					frag.write('vech.w = textureOffset(texpaint_opt, texCoord, ivec2(0, 1)).b;');
					// Displace normal strength
					frag.write('vech *= 15 * 7; float h1 = vech.x - vech.y; float h2 = vech.z - vech.w;');
					frag.write('vec3 va = normalize(vec3(2.0, 0.0, h1));');
					frag.write('vec3 vb = normalize(vec3(0.0, 2.0, h2));');
					frag.write('vec3 vc = normalize(vec3(h1, h2, 2.0));');
					frag.write('n = normalize(mat3(va, vb, vc) * n);');
				}
				//

				frag.write('vec4 pack = texture(texpaint_pack, texCoord);');
				frag.write('occlusion = pack.r;');
				frag.write('roughness = pack.g;');
				frag.write('metallic = pack.b;');
			}
			else {
				frag.write('basecol = vec3(0.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 1.0;');
				frag.write('metallic = 0.0;');
				frag.write('specular = 1.0;');
			}

			if (UITrait.inst.layers.length > 1 && UITrait.inst.layers[1].visible) {
				frag.add_uniform('sampler2D texpaint1');
				frag.add_uniform('sampler2D texpaint_nor1');
				frag.add_uniform('sampler2D texpaint_pack1');
				// frag.add_uniform('sampler2D texpaint_opt1');

				frag.write('vec4 col_tex1 = texture(texpaint1, texCoord);');
				// frag.write('vec4 col_nor1 = texture(texpaint_nor1, texCoord);');

				frag.write('float factor = col_tex1.a;');
				frag.write('float factorinv = 1.0 - factor;');

				frag.write('basecol = basecol * factorinv + pow(col_tex1.rgb, vec3(2.2)) * factor;');
				
				// frag.write('vec3 n2 = texture(texpaint_nor1, texCoord).rgb * 2.0 - 1.0;');
				// frag.write('n2 = normalize(TBN * normalize(n2));');
				// frag.write('n *= n2;');
				frag.write('vec4 pack2 = texture(texpaint_pack1, texCoord);');
				frag.write('occlusion = occlusion * factorinv + pack2.r * factor;');
				frag.write('roughness = roughness * factorinv + pack2.g * factor;');
				frag.write('metallic = metallic * factorinv + pack2.b * factor;');
			}

			frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
			frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
			if (UITrait.inst.viewportMode == 0) { // Render
				frag.write('fragColor[0] = vec4(n.xy, packFloat(metallic, roughness), 1.0 - gl_FragCoord.z);');
				frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(occlusion, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 1) { // Basecol
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0 - gl_FragCoord.z);');
				frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 2) { // Normal
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0 - gl_FragCoord.z);');
				frag.write('fragColor[1] = vec4(ntex.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 3) { // Occ
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0 - gl_FragCoord.z);');
				frag.write('fragColor[1] = vec4(vec3(occlusion), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 4) { // Rough
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0 - gl_FragCoord.z);');
				frag.write('fragColor[1] = vec4(vec3(roughness), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 5) { // Met
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0 - gl_FragCoord.z);');
				frag.write('fragColor[1] = vec4(vec3(metallic), packFloat2(1.0, 1.0));'); // occ/spec
			}
		}

		frag.write('vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		frag.write('vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		frag.write('fragColor[2].rg = vec2(posa - posb);');

		Cycles.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}

	function getMOut():Bool {
		for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	public function parseMeshMaterial() {
		#if arm_editor
		if (UITrait.inst.htab.position == 0) return;
		var m = UITrait.inst.materials[0].data;
		#else
		iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
		#end
			var sc:ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
			if (sc != null) {
				m.shader.raw.contexts.remove(sc.raw);
				m.shader.contexts.remove(sc);
			}
			var con = make_mesh_paint(new CyclesShaderData({name: "Material", canvas: null}));
			if (sc != null) sc.delete();
			sc = new ShaderContext(con.data, function(sc:ShaderContext){});
			m.shader.raw.contexts.push(sc.raw);
			m.shader.contexts.push(sc);
		#if (!arm_editor)
		});
		#end
	}

	public function parseMeshPreviewMaterial() {
		if (!getMOut()) return;

		#if arm_editor
		var m = UITrait.inst.htab.position == 0 ? UITrait.inst.selectedMaterial2.data : UITrait.inst.materials[0].data;
		#else
		iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
		#end

			var sc:ShaderContext = null;
			for (c in m.shader.contexts) if (c.raw.name == "mesh") { sc = c; break; }
			m.shader.raw.contexts.remove(sc.raw);
			m.shader.contexts.remove(sc);
			
			var matcon:TMaterialContext = { name: "mesh", bind_textures: [] };

			var sd = new CyclesShaderData({name: "Material", canvas: null});
			var con = make_mesh_preview(sd, matcon);

			for (i in 0...m.contexts.length) {
				if (m.contexts[i].raw.name == "mesh") {
					m.contexts[i] = new MaterialContext(matcon, function(self:MaterialContext) {});
					break;
				}
			}
			
			if (sc != null) sc.delete();
			sc = new ShaderContext(con.data, function(sc:ShaderContext){});
			m.shader.raw.contexts.push(sc.raw);
			m.shader.contexts.push(sc);


			// var matcon:TMaterialContext = { name: "shadowmap", bind_textures: [] };
			// var smcon = make_depth(sd, matcon, true);
			// var smcdata = smcon.data;
			// // from_source is synchronous..
			// var smsc = new ShaderContext(smcdata, function(sc:ShaderContext){});
			// for (c in m.shader.contexts) if (c.raw.name == 'shadowmap') { m.shader.contexts.remove(c); break; }
			// m.shader.contexts.push(smsc);
			// for (i in 0...m.contexts.length) {
			// 	if (m.contexts[i].raw.name == "shadowmap") {
			// 		m.contexts[i] = new MaterialContext(matcon, function(self:MaterialContext) {});
			// 		break;
			// 	}
			// }

		#if (!arm_editor)
		});
		#end
	}

	public function parsePaintMaterial() {
		if (!getMOut()) return;
		
		#if arm_editor
		if (UITrait.inst.htab.position == 0) {
			parseMeshPreviewMaterial();
			return;
		}
		#end

		#if arm_editor
		var m = UITrait.inst.materials[0].data;
		sc = null;
		_materialcontext = null;
		#else
		iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
		#end
		
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

			var con = make_paint(_sd, _matcon);
			var cdata = con.data;

			// if (sc == null) {
				// from_source is synchronous..
				if (sc != null) sc.delete();
				sc = new ShaderContext(cdata, function(sc:ShaderContext){});
				m.shader.raw.contexts.push(sc.raw);
				m.shader.contexts.push(sc);
				m.raw.contexts.push(_matcon);

				new MaterialContext(_matcon, function(self:MaterialContext) {
					_materialcontext = self;
					m.contexts.push(self);
				});



				var dcon = make_depth(_sd, _matcon);
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



				var smcon = make_depth(_sd, _matcon, true);
				var smcdata = smcon.data;
				// from_source is synchronous..
				var smsc = new ShaderContext(smcdata, function(sc:ShaderContext){});
				for (c in m.shader.contexts) if (c.raw.name == 'shadowmap') { m.shader.contexts.remove(c); break; }
				m.shader.contexts.push(smsc);
				// var smmatcon:TMaterialContext = {
					// name: "shadowmap"
				// }
				// m.raw.contexts.push(smmatcon);
				// for (c in m.contexts) if (c.raw.name == 'shadowmap') { m.contexts.remove(c); break; }
				// new MaterialContext(smmatcon, function(self:MaterialContext) {
					// m.contexts.push(self);
				// });

				if ((UITrait.inst.brushType == 2 || UITrait.inst.brushType == 3) && UITrait.inst.autoFillHandle.selected) { // Fill
					UITrait.inst.pdirty = 8;
					UITrait.inst.ddirty = 8;
				}
			// }
			// else {
			// 	sc.raw.vertex_shader = cdata.vertex_shader;
			// 	sc.raw.fragment_shader = cdata.fragment_shader;
			// 	sc.compile();
			// }
		#if (!arm_editor)
		});
		#end
	}

	public function acceptDrag(assetIndex:Int) {
		var n = NodeCreator.createImageTexture();
		n.buttons[0].default_value = assetIndex;
		nodes.nodeSelected = n;
	}

	public function parseBrush() {
		armory.system.Logic.packageName = "arm.brushnode";
		var tree = armory.system.Logic.parse(canvasBrush, false);
	}

	var lastT:iron.Trait = null;
	public function parseLogic() {
		if (lastT != null) UITrait.inst.currentObject.removeTrait(lastT);
		armory.system.Logic.packageName = "armory.logicnode";
		var t = armory.system.Logic.parse(canvasLogic);
		lastT = t;
		UITrait.inst.currentObject.addTrait(t);
	}
}
