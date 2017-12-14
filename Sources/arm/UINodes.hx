package arm;

import armory.object.Object;
import armory.system.Cycles;
import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MaterialData;

class UINodes extends armory.Trait {

	public static var inst:UINodes;
	public static var show = false;

	public static var wx:Int;
	public static var wy:Int;

	var ui:Zui;
	var drawMenu = false;
	var showMenu = false;
	var hideMenu = false;
	var menuCategory = 0;
	var addNodeButton = false;
	var popupX = 0.0;
	var popupY = 0.0;

	var sc:iron.data.ShaderData.ShaderContext = null;
	public var _matcon:TMaterialContext = null;
	var _materialcontext:MaterialContext = null;

	static var font:kha.Font;

	public function new() {
		super();
		inst = this;

		// Load font for UI labels
		iron.data.Data.getFont('droid_sans.ttf', function(f:kha.Font) {

			iron.data.Data.getBlob('default_material.json', function(b:kha.Blob) {

				kha.Assets.loadImage('color_wheel', function(image:kha.Image) {

					canvas = haxe.Json.parse(b.toString());

					font = f;
					var t = Reflect.copy(zui.Themes.dark);
					t.FILL_WINDOW_BG = true;
					t.ELEMENT_H = 18;
					t.BUTTON_H = 16;
					// ui = new Zui({font: f, theme: t, scaleFactor: 2.5}); ////
					ui = new Zui({font: f, theme: t, color_wheel: image});
					ui.scrollEnabled = false;
					armory.Scene.active.notifyOnInit(sceneInit);
				});
			});
		});
	}

	// var rt:kha.Image; ////
	// var uiWidth = 2048;
	// var uiHeight = 1024;

	function sceneInit() {

		// var pui = iron.Scene.active.getChild("PlaneNodeUI"); ////
		// rt = kha.Image.createRenderTarget(uiWidth, uiHeight);
		// var mat:armory.data.MaterialData = cast(pui, armory.object.MeshObject).materials[0];
		// mat.contexts[0].textures[0] = rt; // Override diffuse texture

		// Store references to cube and plane objects
		notifyOnRender2D(render2D);
		notifyOnUpdate(update);
	}

	var mx = 0.0;
	var my = 0.0;
	// var wh = 300;
	static var frame = 0;
	var mdown = false;
	var mreleased = false;
	var mchanged = false;
	public static var changed = false;
	function update() {
		if (frame == 8) parseMaterial(); // Temp cpp fix
		frame++;


		//
		var mouse = iron.system.Input.getMouse();
		mreleased = mouse.released();
		mdown = mouse.down();

		if (ui.changed) {
			mchanged = true;
			if (!mdown) {
				changed = true;
			}
		}
		if ((mreleased && mchanged) || changed) {
			mchanged = changed = false;
			parseMaterial();
		}
		//


		if (!show) return;
		if (!UITrait.uienabled) return;
		var keyboard = iron.system.Input.getKeyboard();

		wx = Std.int(iron.App.w());//200;
		wy = 0;//iron.App.h() - wh;
		if (mouse.x < wx || mouse.y < wy) return;

		if (ui.isTyping) return;

		if (mouse.started("right")) {
			mx = mouse.x;
			my = mouse.y;
		}
		else if (addNodeButton) {
			showMenu = true;
			addNodeButton = false;
		}
		else if (mouse.released()) {
			hideMenu = true;
		}

		if (keyboard.started("x")) {
			nodes.removeNode(nodes.nodeSelected, canvas);
			changed = true;
		}

		if (keyboard.started("p")) {
			trace(haxe.Json.stringify(canvas));
		}
	}

	static var nodes = new Nodes();

	static var canvas:TNodeCanvas = null;

	static var bg:kha.Image = null;

	function getNodeX():Int {
		var mouse = iron.system.Input.getMouse();
		return Std.int((mouse.x - wx - nodes.PAN_X()) / nodes.SCALE);
	}

	function getNodeY():Int {
		var mouse = iron.system.Input.getMouse();
		return Std.int((mouse.y - wy - nodes.PAN_Y()) / nodes.SCALE);
	}

	public static var grid:kha.Image = null;
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

	@:access(zui.Zui)
	function render2D(g:kha.graphics2.Graphics) {

		if (!show) return;
		
		if (!UITrait.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (UITrait.uienabled && !ui.inputRegistered) ui.registerInput();
		
		g.end();

		if (grid == null) drawGrid();

		// Start with UI
		ui.begin(g);
		// ui.begin(rt.g2); ////
		
		// Make window
		var ww = Std.int(iron.App.w());
		wx = Std.int(iron.App.w());
		wy = 0;
		var hwin = Id.handle();
		if (ui.window(hwin, wx, wy, ww, iron.App.h())) {
			
			ui.g.color = 0xffffffff;
			ui.g.drawImage(grid, (nodes.panX * nodes.SCALE) % 40 - 40, (nodes.panY * nodes.SCALE) % 40 - 40);

			ui.g.font = font;
			ui.g.fontSize = 22;
			var title = "Material";
			var titlew = ui.g.font.width(22, title);
			var titleh = ui.g.font.height(22);
			ui.g.drawString(title, ww - titlew - 20, iron.App.h() - titleh - 10);
			
			// Recompile material on change
			ui.changed = false;
			nodes.nodeCanvas(ui, canvas);

			ui.g.color = ui.t.WINDOW_BG_COL;
			ui.g.fillRect(0, 0, ww, 24);
			ui.g.color = 0xffffffff;

			ui._x = 3;
			ui._y = 3;
			ui._w = 105;
			if (ui.button("Input")) { addNodeButton = true; menuCategory = 0; popupX = wx + ui._x; popupY = wy + ui._y; }
			ui._x += 105 + 3;
			ui._y = 3;
			if (ui.button("Output")) { addNodeButton = true; menuCategory = 1; popupX = wx + ui._x; popupY = wy + ui._y; }
			ui._x += 105 + 3;
			ui._y = 3;
			if (ui.button("Texture")) { addNodeButton = true; menuCategory = 2; popupX = wx + ui._x; popupY = wy + ui._y; }
			ui._x += 105 + 3;
			ui._y = 3;
			if (ui.button("Color")) { addNodeButton = true; menuCategory = 3; popupX = wx + ui._x; popupY = wy + ui._y; }
			ui._x += 105 + 3;
			ui._y = 3;
			if (ui.button("Converter")) { addNodeButton = true; menuCategory = 4; popupX = wx + ui._x; popupY = wy + ui._y; }
		}

		ui.endWindow();

		if (drawMenu) {
			
			var ph = NodeCreator.numNodes[menuCategory] * 20;
			var py = popupY;
			g.color = 0xff222222;
			g.fillRect(popupX, py, 105, ph);

			ui.beginLayout(g, Std.int(popupX), Std.int(py), 105);
			
			NodeCreator.draw(this, menuCategory);

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

	function make_paint(data:ShaderData, matcon:TMaterialContext):armory.system.ShaderContext {
		var context_id = 'paint';
		var con_paint:armory.system.ShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: 'always',
			cull_mode: 'counter_clockwise',
			blend_source: 'source_alpha', //blend_one
			blend_destination: 'inverse_source_alpha', //blend_zero
			blend_operation: 'add',
			alpha_blend_source: 'blend_one',
			alpha_blend_destination: 'blend_zero',
			alpha_blend_operation: 'add',
			vertex_structure: [{"name": "pos", "size": 3},{"name": "nor", "size": 3},{"name": "tex", "size": 2}] });

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();

		vert.add_out('vec3 sp');
		// vert.add_out('vec3 wnormal');
		frag.ins = vert.outs;
		// vert.add_uniform('mat4 N', '_normalMatrix');
		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');

		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, tex.y * 2.0 - 1.0);');

		// TODO: Fix seams at uv borders
		vert.add_uniform('vec2 sub', '_sub');
		vert.write('tpos += sub;');
		
		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');
		
		vert.write('vec4 ndc = WVP * vec4(pos, 1.0);');
		vert.write('ndc.xyz = ndc.xyz / ndc.w;');
		vert.write('sp.xyz = ndc.xyz * 0.5 + 0.5;');
		vert.write('sp.y = 1.0 - sp.y;');
		vert.write('sp.z -= 0.0001;'); // Bias

		vert.add_out('vec3 mposition');
		if (UITrait.brushPaint == 0 && con_paint.is_elem('tex')) {
        	vert.write('mposition = pos.xyz;');
        }
        else {
        	vert.write('mposition = ndc.xyz;');
        }

		frag.add_out('vec4 fragColor[3]');
		frag.add_uniform('vec4 inp', '_input');

		// frag.add_uniform('vec3 v', '_cameraLook');
		// frag.write('vec3 n = normalize(wnormal);');
		// frag.write('if (dot(n, v) > 0.0) discard;');

		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D paintdb');
		frag.write('if (sp.z > texture(paintdb, vec2(sp.x, 1.0 - bsp.y)).r) discard;');

		frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
		frag.write('binp.x *= aspectRatio;');
		frag.write('binp = binp * 0.5 + 0.5;');
		
		frag.write('float dist = distance(bsp.xy, binp.xy);');
		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushStrength', '_brushStrength');

		frag.write('if (dist > brushRadius) discard;');
		
		// Texture projection - texcoords
		if (UITrait.brushPaint == 0 && con_paint.is_elem('tex')) {
			vert.add_uniform('float brushScale', '_brushScale');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = fract(tex * brushScale);'); // TODO: fract(tex) - somehow clamp is set after first paint
		}
		else {
			frag.add_uniform('float brushScale', '_brushScale');
			// TODO: use prescaled value from VS
			Cycles.texCoordName = 'fract(sp * brushScale)'; // Texture projection - project
		}
		var sout = Cycles.parse(canvas, con_paint, vert, frag, null, null, null, matcon);
		Cycles.texCoordName = 'texCoord';
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var nortan = Cycles.out_normaltan;
		frag.write('vec3 basecol = $base;');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('vec3 nortan = $nortan;');

		frag.write('    float str = brushOpacity * clamp((brushRadius - dist) * brushStrength, 0.0, 1.0);');
		frag.write('    str = clamp(str, 0.0, 1.0);');
		
		frag.write('    fragColor[0] = vec4(basecol, str);');
		frag.write('    fragColor[1] = vec4(nortan, 1.0);'); // Encode normal, height, opacity, matid,..
		frag.write('    fragColor[2] = vec4(occlusion, roughness, metallic, str);');

		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}

	function make_mesh_preview(data:ShaderData, matcon:TMaterialContext):armory.system.ShaderContext {
		var context_id = 'mesh';
		var con_mesh:armory.system.ShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise' });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		
		frag.ins = vert.outs;
		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write('gl_Position = WVP * vec4(pos, 1.0);');


		var sout = Cycles.parse(canvas, con_mesh, vert, frag, null, null, null, matcon);
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		frag.write('vec3 basecol = $base;');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');

		frag.add_out('vec4[2] fragColor');
		frag.write('fragColor[0] = vec4(0.0, 0.0, 0.0, 1.0 - gl_FragCoord.z);');
		frag.write('fragColor[1] = vec4(basecol.rgb, 0.0);');

		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}

	function make_depth(data:ShaderData):armory.system.ShaderContext {
		var context_id = 'depth';
		var con_depth:armory.system.ShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			color_write_red: false,
			color_write_green: false,
			color_write_blue: false,
			color_write_alpha: false });

		var vert = con_depth.make_vert();
		var frag = con_depth.make_frag();

		
		frag.ins = vert.outs;
		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write('gl_Position = WVP * vec4(pos, 1.0);');

		con_depth.data.shader_from_source = true;
		con_depth.data.vertex_shader = vert.get();
		con_depth.data.fragment_shader = frag.get();

		return con_depth;
	}

	// function make_mesh_paint(data:ShaderData):armory.system.ShaderContext {
	// 	var context_id = 'mesh';
	// 	var con_mesh:armory.system.ShaderContext = data.add_context({
	// 		name: context_id,
	// 		depth_write: true,
	// 		compare_mode: 'less',
	// 		cull_mode: 'clockwise' });

	// 	var vert = con_mesh.make_vert();
	// 	var frag = con_mesh.make_frag();

	// 	frag.ins = vert.outs;

		



	// 	frag.write('vec3 basecol;');
	// 	frag.write('float roughness;');
	// 	frag.write('float metallic;');
	// 	frag.write('float occlusion;');
	// 	frag.write('float opacity;');

	// 	frag.add_uniform('sampler2D texpaint');
	// 	frag.write('basecol = pow(texture(texpaint, texCoord).rgb, vec3(2.2));');

	// 	frag.add_uniform('sampler2D texpaint_nor');
	// 	frag.write('vec3 n = texture(texpaint_nor, texCoord).rgb * 2.0 - 1.0;');
	// 	frag.write('n = normalize(TBN * normalize(n));');

	// 	frag.add_uniform('sampler2D texpaint_pack');
	// 	frag.write('vec4 pack = texture(texpaint_pack, texCoord);');
	// 	frag.write('occlusion = pack.r;');
	// 	frag.write('roughness = pack.g;');
	// 	frag.write('metallic = pack.b;');

	// 	// TODO: Sample disp at neightbour points to calc normal
	// 	// tese.add_uniform('sampler2D texpaint_pack')
	// 	// tese.write('vec4 pack = texture(texpaint_pack, texCoord);')
	// 	// tese.write('disp = pack.a * 0.05;')



	// 	con_mesh.data.shader_from_source = true;
	// 	con_mesh.data.vertex_shader = vert.get();
	// 	con_mesh.data.fragment_shader = frag.get();

	// 	return con_mesh;
	// }

	function parseMaterial() {
		UITrait.dirty = true;

		var mout = false;
		for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { mout = true; break; }

		if (mout) {

			iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
			
				var mat:TMaterial = {
					name: "Material",
					canvas: canvas
				};
				var _sd = new ShaderData(mat);
				
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
					sc = new iron.data.ShaderData.ShaderContext(cdata, null, function(sc:iron.data.ShaderData.ShaderContext){});
					m.shader.raw.contexts.push(sc.raw);
					m.shader.contexts.push(sc);
					m.raw.contexts.push(_matcon);

					new MaterialContext(_matcon, function(self:MaterialContext) {
						_materialcontext = self;
						m.contexts.push(self);
					});



					var dcon = make_depth(_sd);
					var dcdata = dcon.data;
					// from_source is synchronous..
					var dsc = new iron.data.ShaderData.ShaderContext(dcdata, null, function(sc:iron.data.ShaderData.ShaderContext){});
					m.shader.contexts.push(dsc);
					var dmatcon:TMaterialContext = {
						name: "depth"
					}
					m.raw.contexts.push(dmatcon);
					new MaterialContext(dmatcon, function(self:MaterialContext) {
						m.contexts.push(self);
					});
				// }
				// else {
				// 	sc.raw.vertex_shader = cdata.vertex_shader;
				// 	sc.raw.fragment_shader = cdata.fragment_shader;
				// 	sc.compile();
				// }


				// var sp = iron.Scene.active.getChild("SpherePreview");
				// var md = cast(sp, iron.object.MeshObject).materials[0];
				// var mc = md.shader.contexts[0]; // mesh

				// var pmat:TMaterial = {
				// 	name: "MaterialPreview",
				// 	canvas: canvas
				// };
				// var _psd = new ShaderData(pmat);
				// _psd.add_elem('pos', 3);
				// _psd.add_elem('nor', 3);
				// _psd.add_elem('tex', 2);
				// _psd.add_elem('tang', 3);
				// var pcon = make_mesh_preview(_psd, null);
				// var pcdata = pcon.data;

				// mc.raw.shader_from_source = true;
				// mc.raw.vertex_shader = pcdata.vertex_shader;
				// mc.raw.fragment_shader = pcdata.fragment_shader;
				// mc.compile();
			});
		}
	}

	public static function acceptDrag(assetIndex:Int) {
		NodeCreator.createImageTexture(inst);
		nodes.nodeSelected.buttons[0].default_value = assetIndex;
	}
}
