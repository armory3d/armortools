package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.object.MeshObject;
import iron.RenderPath;

typedef TPreferences = {
	public var w:Int;
	public var h:Int;
	public var save_location:String;
	public var load_location:String;
}

// typedef TProject = {
	// public var brushes:Array<>;
	// public var materials:Array<>;
// }

class MaterialSlot {
	public var nodes = new Nodes();
	public function new() {}
}

class BrushSlot {
	public var nodes = new Nodes();
	public function new() {}
}

class LayerSlot {
	static var counter = 0;
	public var id = 0;
	public var visible = true;

	public var texpaint:kha.Image;
	public var texpaint_nor:kha.Image;
	public var texpaint_pack:kha.Image;
	
	public var texpaint_opt:kha.Image;

	public function new() {
		id = counter++;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint" + id;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			t.depth_buffer = "paintdb";
			texpaint = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor" + id;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			texpaint_nor = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack" + id;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			texpaint_pack = RenderPath.active.createRenderTarget(t).image;
		}

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_opt" + id;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			texpaint_opt = RenderPath.active.createRenderTarget(t).image;
		}
	}

	public function unload() {
		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();
		
		texpaint_opt.unload();
	}
}

@:access(zui.Zui)
@:access(iron.data.Data)
class UITrait extends iron.Trait {

	public var assets:Array<TAsset> = [];
	public var assetNames:Array<String> = [];
	public var assetId = 0;

	public static var inst:UITrait;
	public static var defaultWindowW = 280;

	public static var penPressure = true;
	public static var drawWorld = true;
	public static var worldColor = 0xffffffff;

	public var isScrolling = false;

	public var colorIdPicked = false;

	public var show = true;
	public var dirty = true;

	var message = "";
	var messageTimer = 0.0;

	public var bundled:Map<String, kha.Image> = new Map();
	var ui:Zui;

	public var windowW = 280; // Panel width

	var colorIdHandle = Id.handle();

	function loadBundled(names:Array<String>, done:Void->Void) {
		var loaded = 0;
		for (s in names) {
			kha.Assets.loadImageFromPath(s, false, function(image:kha.Image) {
				bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	public function depthDirty():Bool {
		return dirty || first < 10;
	}

	var first = 0;
	public function redraw():Bool {
		if (first < 10) {
			first++;
			return true;
		}
		var m = iron.system.Input.getMouse();
		return m.down() || m.down("right") || m.released() || m.released("right") || depthDirty();
	}

	var _onBrush:Array<Void->Void> = [];

	public function notifyOnBrush(f:Void->Void) {
		_onBrush.push(f);
	}

	public var paint = false;
	public var paintVec = new iron.math.Vec4();
	public var lastPaintX = 0.0;
	public var lastPaintY = 0.0;
	var painted = 0;
	public var brushTime = 0.0;
	public function paintDirty():Bool {
		// Paint bounds
		if (paintVec.x > 1) return false;

		var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();

		if (brushType == 4 && UITrait.inst.assets.length > 0 && down) {
			colorIdPicked = true;
		}

		// Prevent painting the same spot - save perf & reduce projection paint jittering caused by _sub offset
		if (down && paintVec.x == lastPaintX && paintVec.y == lastPaintY) painted++;
		else painted = 0;

		if (painted > 8) return false;
		lastPaintX = paintVec.x;
		lastPaintY = paintVec.y;

		return paint;
	}

	public var brushNodesRadius = 1.0;
	public var brushNodesOpacity = 1.0;
	public var brushNodesScale = 1.0;
	public var brushNodesStrength = 1.0;

	public var brushRadius = 0.5;
	public var brushOpacity = 1.0;
	public var brushScale = 0.5;
	public var brushStrength = 1.0;
	public var brushBias = 1.0;
	public var brushPaint = 0;
	public var brushType = 0;

	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	
	public var paintVisible = true;

	function linkFloat(link:String):Null<Float> {

		if (link == '_brushRadius') {
			var r = (brushRadius * brushNodesRadius) / 15.0;
			var p = iron.system.Input.getPen().pressure;
			if (p != 0.0 && penPressure) r *= p;
			return r;
		}
		else if (link == '_brushOpacity') {
			return brushOpacity * brushNodesOpacity;
		}
		else if (link == '_brushScale') {
			return (brushScale * brushNodesScale) * 2.0;
		}
		else if (link == '_brushStrength') {
			var f = brushStrength * brushNodesStrength;
			return f * f * 100;
		}
		else if (link == '_paintDepthBias') {
			return paintVisible ? 0.0001 : 1.0;
		}

		return null;
	}

	var sub = 0;
	var vec2 = new iron.math.Vec4();
	function linkVec2(link:String):iron.math.Vec4 {

		if (link == '_sub') {
			var seps = brushBias * 0.0004;
			sub = (sub + 1) % 9;
			if (sub == 0) vec2.set(0.0 + seps, 0.0, 0.0);
			else if (sub == 1) vec2.set(0.0 - seps, 0.0, 0.0);
			else if (sub == 2) vec2.set(0.0, 0.0 + seps, 0.0);
			else if (sub == 3) vec2.set(0.0, 0.0 - seps, 0.0);
			else if (sub == 4) vec2.set(0.0 + seps, 0.0 + seps, 0.0);
			else if (sub == 5) vec2.set(0.0 - seps, 0.0 - seps, 0.0);
			else if (sub == 6) vec2.set(0.0 + seps, 0.0 - seps, 0.0);
			else if (sub == 7) vec2.set(0.0 - seps, 0.0 + seps, 0.0);
			else if (sub == 8) vec2.set(0.0, 0.0, 0.0);
			return vec2;
		}
		else if (link == '_texcoloridSize') {
			vec2.set(0, 0, 0);
			if (UITrait.inst.assets.length == 0) return vec2;
			var img = UITrait.inst.getImage(UITrait.inst.assets[colorIdHandle.position]);
			vec2.set(img.width, img.height, 0);
			return vec2;
		}

		return null;
	}

	public var lastPaintVecX = -1.0;
	public var lastPaintVecY = -1.0;
	function linkVec4(link:String):iron.math.Vec4 {
		if (link == '_inputBrush') {
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			vec2.set(paintVec.x, paintVec.y, down ? 1.0 : 0.0, 0.0);
			return vec2;
		}
		else if (link == '_inputBrushLast') {
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			vec2.set(lastPaintVecX, lastPaintVecY, down ? 1.0 : 0.0, 0.0);
			return vec2;
		}
		return null;
	}

	function linkTex(link:String):kha.Image {
		if (link == "_texcolorid") {
			if (UITrait.inst.assets.length == 0) return bundled.get("mat_empty.jpg");
			else return UITrait.inst.getImage(UITrait.inst.assets[colorIdHandle.position]);
		}
		return null;
	}

	public function new() {
		super();

		inst = this;

		haxeTrace = haxe.Log.trace;
		haxe.Log.trace = consoleTrace;

		windowW = Std.int(defaultWindowW * armory.data.Config.raw.window_scale);

		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalTextureLinks.push(linkTex);

		if (materials == null) {
			materials = [];
			materials.push(new MaterialSlot());
			selectedMaterial = materials[0];
		}

		if (brushes == null) {
			brushes = [];
			brushes.push(new BrushSlot());
			selectedBrush = brushes[0];
		}

		if (layers == null) {
			layers = [];
			layers.push(new LayerSlot());
			selectedLayer = layers[0];
		}

		var scale = armory.data.Config.raw.window_scale;
		ui = new Zui( { theme: arm.App.theme, font: arm.App.font, scaleFactor: scale, color_wheel: arm.App.color_wheel } );
		loadBundled(['cursor.png', 'mat.jpg', 'mat_empty.jpg', 'brush_draw.png', 'brush_erase.png', 'brush_fill.png', 'brush_bake.png', 'brush_colorid.png', 'cay_thumb.jpg'], done);
	}

	var haxeTrace:Dynamic->haxe.PosInfos->Void;
	var lastTrace = '';
	function consoleTrace(v:Dynamic, ?inf:haxe.PosInfos) {
		lastTrace = Std.string(v);
		haxeTrace(v, inf);
    }

	function showMessage(s:String) {
		messageTimer = 3.0;
		message = s;
		hwnd.redraws = 2;
	}

	function checkImageFormat(path:String):Bool {
		var p = path.toLowerCase();
		if (!StringTools.endsWith(p, ".jpg") &&
			!StringTools.endsWith(p, ".png") &&
			!StringTools.endsWith(p, ".tga") &&
			!StringTools.endsWith(p, ".hdr")) {
			return false;
		}
		return true;
	}

	public function importAsset(path:String) {
		if (!checkImageFormat(path)) {
			showMessage("Error: Unknown asset format");
			return;
		}
		
		iron.data.Data.getImage(path, function(image:kha.Image) {
			var ar = path.split("/");
			var name = ar[ar.length - 1];
			var asset:TAsset = {name: name, file: path, id: UITrait.inst.assetId++};
			UITrait.inst.assets.push(asset);
			UITrait.inst.assetNames.push(name);
			Canvas.assetMap.set(asset.id, image);
			hwnd.redraws = 2;
		});
	}

	// var rt:kha.Image; ////
	// var uiWidth = 2048;
	// var uiHeight = 2048;

	var currentObject:MeshObject;

	function done() {

		notifyOnInit(function() {
		// iron.Scene.active.notifyOnInit(function() { ////

			// var pui = iron.Scene.active.getChild("PlaneUI"); ////
			// rt = kha.Image.createRenderTarget(uiWidth, uiHeight);
			// var mat:armory.data.MaterialData = cast(pui, armory.object.MeshObject).materials[0];
			// mat.contexts[0].textures[0] = rt; // Override diffuse texture

			currentObject = cast(iron.Scene.active.getChild("Cube"), MeshObject);

			iron.App.notifyOnUpdate(update);
			iron.App.notifyOnRender2D(render);
			iron.App.notifyOnRender(initLayers);

			// Save last pos for continuos paint
			iron.App.notifyOnRender(function(g:kha.graphics4.Graphics) { //
				var m = iron.system.Input.getMouse();
				if (m.down()) { //
					arm.UITrait.inst.lastPaintVecX = arm.UITrait.inst.paintVec.x; //
					arm.UITrait.inst.lastPaintVecY = arm.UITrait.inst.paintVec.y;//
				}//
				else {
					arm.UITrait.inst.lastPaintVecX = m.x / iron.App.w();
					arm.UITrait.inst.lastPaintVecY = m.y / iron.App.h();
				}
			});//

			// Init plugins
			Plugin.keep();
			var apconfig:TAPConfig = cast armory.data.Config.raw;
			if (apconfig.plugins != null) {
				for (plugin in apconfig.plugins) {
					iron.data.Data.getBlob(plugin, function(blob:kha.Blob) {
						untyped __js__("(1, eval)({0})", blob.toString());
					});
				}
			}
		});
	}

	// public static var pickColorId = false;

	function update() {
		isScrolling = ui.isScrolling;
		updateUI();

		var kb = iron.system.Input.getKeyboard();
		var shift = kb.down("shift");
		var alt = kb.down("alt");
		if (kb.started("tab")) {
			UIView2D.inst.show = false;
			UINodes.inst.show = !UINodes.inst.show;
			arm.App.resize();
		}
		else if (kb.started("1") && (shift || alt)) shift ? setBrushType(0) : selectMaterial(0);
		else if (kb.started("2") && (shift || alt)) shift ? setBrushType(1) : selectMaterial(1);
		else if (kb.started("3") && (shift || alt)) shift ? setBrushType(2) : selectMaterial(2);
		else if (kb.started("4") && (shift || alt)) shift ? setBrushType(3) : selectMaterial(3);
		else if (kb.started("5") && (shift || alt)) shift ? setBrushType(4) : selectMaterial(4);

		// pickColorId = kb.down("alt");

		for (p in Plugin.plugins) if (p.update != null) p.update();
	}

	function selectMaterial(i:Int) {
		if (materials.length <= i) return;
		selectedMaterial = materials[i];
		UINodes.inst.updateCanvasMap();
		UINodes.inst.parsePaintMaterial();
		hwnd.redraws = 2;
	}

	function selectBrush(i:Int) {
		if (brushes.length <= i) return;
		selectedBrush = brushes[i];
		UINodes.inst.updateCanvasBrushMap();
		UINodes.inst.parseBrush();
		hwnd.redraws = 2;
	}

	function updateUI() {

		messageTimer -= iron.system.Time.delta;

		paint = false;
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if (!show) return;
		if (!arm.App.uienabled) return;

		var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
		if (down && !kb.down("ctrl")) {
			brushTime += iron.system.Time.delta;
			for (f in _onBrush) f();
		}
		else brushTime = 0;
	}

	function initLayers(g:kha.graphics4.Graphics) {
		g.end();

		layers[0].texpaint.g4.begin();
		layers[0].texpaint.g4.clear(kha.Color.fromFloats(0.5, 0.5, 0.5, 1.0)); // Base
		layers[0].texpaint.g4.end();

		layers[0].texpaint_nor.g4.begin();
		layers[0].texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 1.0)); // Nor
		layers[0].texpaint_nor.g4.end();

		layers[0].texpaint_pack.g4.begin();
		layers[0].texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 1.0)); // Occ, rough, met
		layers[0].texpaint_pack.g4.end();

		layers[0].texpaint_opt.g4.begin();
		layers[0].texpaint_opt.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Opac, emis, height
		layers[0].texpaint_opt.g4.end();

		g.begin();
		iron.App.removeRender(initLayers);
	}

	function clearLastLayer(g:kha.graphics4.Graphics) {
		g.end();

		var i = layers.length - 1;
		layers[i].texpaint.g4.begin();
		layers[i].texpaint.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Base
		layers[i].texpaint.g4.end();

		layers[i].texpaint_nor.g4.begin();
		layers[i].texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		layers[i].texpaint_nor.g4.end();

		layers[i].texpaint_pack.g4.begin();
		layers[i].texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Occ, rough, met
		layers[i].texpaint_pack.g4.end();

		layers[i].texpaint_opt.g4.begin();
		layers[i].texpaint_opt.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Opac, emis, height
		layers[i].texpaint_opt.g4.end();

		g.begin();
		iron.App.removeRender(clearLastLayer);
	}

	function resizeLayers(g:kha.graphics4.Graphics) {
		var res = getTextureRes();
		var rts = RenderPath.active.renderTargets;

		for (l in layers) {

			var rttexpaint = rts.get("texpaint" + l.id);
			var rttexpaint_nor = rts.get("texpaint_nor" + l.id);
			var rttexpaint_pack = rts.get("texpaint_pack" + l.id);
			var rttexpaint_opt = rts.get("texpaint_opt" + l.id);

			rttexpaint.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.Depth16);
			rttexpaint_nor.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
			rttexpaint_pack.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
			rttexpaint_opt.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);

			g.end();
			rttexpaint.image.g2.begin();
			rttexpaint.image.g2.drawScaledImage(l.texpaint, 0, 0, res, res);
			rttexpaint.image.g2.end();

			rttexpaint_nor.image.g2.begin();
			rttexpaint_nor.image.g2.drawScaledImage(l.texpaint_nor, 0, 0, res, res);
			rttexpaint_nor.image.g2.end();

			rttexpaint_pack.image.g2.begin();
			rttexpaint_pack.image.g2.drawScaledImage(l.texpaint_pack, 0, 0, res, res);
			rttexpaint_pack.image.g2.end();

			rttexpaint_opt.image.g2.begin();
			rttexpaint_opt.image.g2.drawScaledImage(l.texpaint_opt, 0, 0, res, res);
			rttexpaint_opt.image.g2.end();
			g.begin();

			l.unload();

			l.texpaint = rts.get("texpaint" + l.id).image;
			l.texpaint_nor = rts.get("texpaint_nor" + l.id).image;
			l.texpaint_pack = rts.get("texpaint_pack" + l.id).image;
			l.texpaint_opt = rts.get("texpaint_opt" + l.id).image;
		}

		dirty = true;
		iron.App.removeRender(resizeLayers);
	}

	function deleteSelectedLayer() {
		selectedLayer.unload();
		layers.remove(selectedLayer);
		selectedLayer = layers[0];
		UINodes.inst.parseMeshMaterial();
		UINodes.inst.parsePaintMaterial();
		dirty = true;
	}

	var pipe:kha.graphics4.PipelineState = null;
	function makePipe() {
		pipe = new kha.graphics4.PipelineState();
		pipe.fragmentShader = kha.Shaders.view2d_frag;
		pipe.vertexShader = kha.Shaders.view2d_vert;
		// pipe.fragmentShader = kha.Shaders.painter_image_frag;
		// pipe.vertexShader = kha.Shaders.painter_image_vert;
		var vs = new kha.graphics4.VertexStructure();
		vs.add("vertexPosition", kha.graphics4.VertexData.Float3);
		vs.add("texPosition", kha.graphics4.VertexData.Float2);
		vs.add("vertexColor", kha.graphics4.VertexData.Float4);
		pipe.inputLayout = [vs];
		pipe.blendSource = kha.graphics4.BlendingFactor.SourceAlpha;
		pipe.blendDestination = kha.graphics4.BlendingFactor.InverseSourceAlpha;
		// pipe.alphaBlendSource = kha.graphics4.BlendingFactor.BlendZero;
		// pipe.alphaBlendDestination = kha.graphics4.BlendingFactor.BlendOne;
		pipe.compile();
	}

	function applySelectedLayer(g:kha.graphics4.Graphics) {

		if (pipe == null) makePipe();

		var l0 = layers[0];
		var l1 = selectedLayer;

		g.end();

		l0.texpaint.g2.begin(false);
		l0.texpaint.g2.pipeline = pipe;
		l0.texpaint.g2.drawImage(l1.texpaint, 0, 0);
		l0.texpaint.g2.end();

		l0.texpaint_nor.g2.begin(false);
		l0.texpaint_nor.g2.pipeline = pipe;
		l0.texpaint_nor.g2.drawImage(l1.texpaint_nor, 0, 0);
		l0.texpaint_nor.g2.end();

		l0.texpaint_pack.g2.begin(false);
		l0.texpaint_pack.g2.pipeline = pipe;
		l0.texpaint_pack.g2.drawImage(l1.texpaint_pack, 0, 0);
		l0.texpaint_pack.g2.end();

		l0.texpaint_opt.g2.begin(false);
		l0.texpaint_opt.g2.drawImage(l1.texpaint_opt, 0, 0);
		l0.texpaint_opt.g2.end();
		
		g.begin();

		deleteSelectedLayer();
		iron.App.removeRender(applySelectedLayer);
	}

	function render(g:kha.graphics2.Graphics) {
		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		renderUI(g);

		// var ready = showFiles || dirty;
		// TODO: Texture params get overwritten
		// if (ready) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
		// if (UINodes.inst._matcon != null) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;

		// iron.Scene.active.camera.renderPath.ready = ready;
		// dirty = false;
	}

	public var cameraType = 0;
	var textureRes = 2;
	function getTextureRes():Int {
		if (textureRes == 0) return 1024;
		if (textureRes == 1) return 2048;
		if (textureRes == 2) return 4096;
		if (textureRes == 3) return 8192;
		if (textureRes == 4) return 16384;
		if (textureRes == 5) return 20480;
		return 0;
	}

	function showMaterialNodes() {
		UIView2D.inst.show = false;
		if (UINodes.inst.show && !UINodes.inst.isBrush) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.isBrush = false; }
		arm.App.resize();
	}

	function showBrushNodes() {
		UIView2D.inst.show = false;
		if (UINodes.inst.show && UINodes.inst.isBrush) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.isBrush = true; }
		arm.App.resize();
	}

	function show2DView() {
		UINodes.inst.show = false;
		UIView2D.inst.show = !UIView2D.inst.show;
		arm.App.resize();
	}

	function setBrushType(i:Int) {
		brushType = i;
		UINodes.inst.parsePaintMaterial();
		UINodes.inst.parseMeshMaterial();
		hwnd.redraws = 2;
	}

	var outputType = 0;
	var isBase = true;
	var isOpac = true;
	var isOcc = true;
	var isRough = true;
	var isMet = true;
	var isNor = true;
	var hwnd = Id.handle();
	var materials:Array<MaterialSlot> = null;
	public var selectedMaterial:MaterialSlot;
	var brushes:Array<BrushSlot> = null;
	public var selectedBrush:BrushSlot;
	public var layers:Array<LayerSlot> = null;
	public var selectedLayer:LayerSlot;
	var selectTime = 0.0;
	public var displaceStrength = 1.0;
	function renderUI(g:kha.graphics2.Graphics) {
		if (!show) return;

		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();

		var brushImg = bundled.get('brush.jpg');
		var envThumbCay = bundled.get('cay_thumb.jpg');
		var cursorImg = bundled.get('cursor.png');
		var mouse = iron.system.Input.getMouse();
		g.color = 0xffffffff;

		// Brush
		if (arm.App.uienabled) {
			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));
			// g.imageScaleQuality = kha.graphics2.ImageScaleQuality.High;
			var mx = mouse.x;
			var my = mouse.y;
			var pen = iron.system.Input.getPen();
			if (pen.down()) {
				mx = pen.x;
				my = pen.y;
			}
			g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
		}

		g.end();
		ui.begin(g);
		// ui.begin(rt.g2); ////
		
		if (ui.window(hwnd, arm.App.realw() - windowW, 0, windowW, arm.App.realh())) {

			var htab = Id.handle({position: 0});

			if (ui.tab(htab, "Tools")) {

				if (messageTimer > 0) {
					ui.text(message);
				}

				ui._y += 6;

				var img1 = bundled.get("brush_draw.png");
				var img2 = bundled.get("brush_erase.png");
				var img3 = bundled.get("brush_fill.png");
				var img4 = bundled.get("brush_bake.png");
				var img5 = bundled.get("brush_colorid.png");
				ui.row([1/5,1/5,1/5,1/5,1/5]);
				var tool = "";
				if (brushType == 0) { tool = "Draw"; ui.fill(1, -2, img1.width + 3, img1.height + 3, 0xff205d9c); }
				if (ui.image(img1) == State.Started) setBrushType(0);
				if (brushType == 1) { tool = "Erase"; ui.fill(1, -2, img1.width + 3, img1.height + 3, 0xff205d9c); }
				if (ui.image(img2) == State.Started) setBrushType(1);
				if (brushType == 2) { tool = "Fill"; ui.fill(1, -2, img1.width + 3, img1.height + 3, 0xff205d9c); }
				if (ui.image(img3) == State.Started) setBrushType(2);
				if (brushType == 3) { tool = "Bake"; ui.fill(1, -2, img1.width + 3, img1.height + 3, 0xff205d9c); }
				if (ui.image(img4) == State.Started) setBrushType(3);
				if (brushType == 4) { tool = "Color ID"; ui.fill(1, -2, img1.width + 3, img1.height + 3, 0xff205d9c); }
				if (ui.image(img5) == State.Started) setBrushType(4);

				ui._y += 6;

				if (ui.panel(Id.handle({selected: true}), tool)) {
					// Color ID
					if (brushType == 4) {
						// Picked color
						ui.row([1/2, 1/2]);
						ui.text("Picked Color");
						if (ui.button("Clear")) colorIdPicked = false;
						if (colorIdPicked) {
							ui.image(iron.RenderPath.active.renderTargets.get("texpaint_colorid0").image, 0xffffffff, 64);
						}
						// Set color map
						ui.text("Color ID Map");
						var cid = ui.combo(colorIdHandle, App.getEnumTexts(), "Color ID");
						if (UITrait.inst.assets.length > 0) ui.image(UITrait.inst.getImage(UITrait.inst.assets[cid]));
					}
					else if (brushType == 3) { // Bake AO
						ui.radio(Id.handle(), 0, "Ambient Occlusion");
					}
					else {
						ui.row([1/2, 1/2]);
						ui.combo(Id.handle(), ["Add"], "Blending");
						if (ui.button("Nodes")) showBrushNodes();
						ui.row([1/2, 1/2]);
						var paintHandle = Id.handle();
						brushPaint = ui.combo(paintHandle, ["UV", "Project"], "Paint");
						if (paintHandle.changed) {
							UINodes.inst.parsePaintMaterial();
						}
						brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 1.0, true);
						ui.row([1/2, 1/2]);
						brushRadius = ui.slider(Id.handle({value: brushRadius}), "Radius", 0.0, 2.0, true);
						brushOpacity = ui.slider(Id.handle({value: brushOpacity}), "Opacity", 0.0, 1.0, true);
						ui.row([1/2, 1/2]);
						brushScale = ui.slider(Id.handle({value: brushScale}), "UV Scale", 0.0, 2.0, true);
						brushStrength = ui.slider(Id.handle({value: brushStrength}), "Strength", 0.0, 1.0, true);

						ui.row([1/3,1/3,1/3]);

						var baseHandle = Id.handle({selected: paintBase});
						paintBase = ui.check(baseHandle, "Base Color");
						if (baseHandle.changed) {
							UINodes.inst.updateCanvasMap();
							UINodes.inst.parsePaintMaterial();
						}

						var norHandle = Id.handle({selected: paintNor});
						paintNor = ui.check(norHandle, "Normal Map");
						if (norHandle.changed) {
							UINodes.inst.updateCanvasMap();
							UINodes.inst.parsePaintMaterial();
						}

						var roughHandle = Id.handle({selected: paintRough});
						// TODO: Use glColorMaski() to disable specific channel
						paintRough = ui.check(roughHandle, "ORM");
						if (roughHandle.changed) {
							UINodes.inst.updateCanvasMap();
							UINodes.inst.parsePaintMaterial();
						}

						paintVisible = ui.check(Id.handle({selected: paintVisible}), "Visible Only");
					}
				}

				if (ui.panel(Id.handle({selected: true}), "Material")) {

					var img = bundled.get("mat.jpg");
					var img2 = bundled.get("mat_empty.jpg");

					for (row in 0...Std.int(Math.ceil(materials.length / 5))) { 
						ui.row([1/5,1/5,1/5,1/5,1/5]);

						for (j in 0...5) {
							var i = j + row * 5;
							var im = img;
							if (materials.length <= i) im = img2;

							if (im == img && selectedMaterial == materials[i]) {
								ui.fill(1, -2, im.width + 3, im.height + 3, 0xff205d9c);
							}

							if (ui.image(im) == State.Started && im == img) {
								if (selectedMaterial != materials[i]) {
									selectedMaterial = materials[i];
									UINodes.inst.updateCanvasMap();
									UINodes.inst.parsePaintMaterial();
								}
								if (iron.system.Time.time() - selectTime < 0.3) {
									showMaterialNodes();
								}
								selectTime = iron.system.Time.time();
							}
						}
					}

					ui.row([1/2,1/2]);
					if (ui.button("New")) {
						selectedMaterial = new MaterialSlot();
						materials.push(selectedMaterial);
					}
					if (ui.button("Nodes")) {
						showMaterialNodes();
					}
				}

				if (ui.panel(Id.handle({selected: true}), "Layers")) {

					function drawList(h:zui.Zui.Handle, l:LayerSlot, i:Int) {
						if (selectedLayer == l) {
							ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, 0xff205d9c);
						}
						if (i > 0) ui.row([1/10, 5/10, 2/10, 2/10]);
						else ui.row([1/10, 9/10]);
						l.visible = ui.check(Id.handle().nest(l.id, {selected: l.visible}), "");
						if (ui.changed) {
							UINodes.inst.parseMeshMaterial();
							dirty = true;
						}
						ui.text("Layer " + (i + 1));
						if (ui.isReleased) {
							selectedLayer = l;
							UINodes.inst.parsePaintMaterial(); // Different blending for layer on top
							dirty = true;
						}
						if (i > 0) {
							if (ui.button("Apply")) {
								if (layers.length > 1 && selectedLayer == layers[1]) {
									iron.App.notifyOnRender(applySelectedLayer);
								}
							}
							if (ui.button("Delete")) {
								if (layers.length > 1 && selectedLayer == layers[1]) {
									deleteSelectedLayer();
								}
							}
						}
					}
					for (i in 0...layers.length) {
						var j = layers.length - 1 - i;
						var l = layers[j];
						drawList(Id.handle(), l, j);
					}

					if (layers.length == 1) {
						ui.row([1/2, 1/2]);
						if (ui.button("New")) {
							if (layers.length < 2) {
								selectedLayer = new LayerSlot();
								layers.push(selectedLayer);
								UINodes.inst.parseMeshMaterial();
								UINodes.inst.parsePaintMaterial();
								dirty = true;
								iron.App.notifyOnRender(clearLastLayer);
							}
						}
					}
					if (ui.button("2D View")) show2DView();
				}

				if (ui.panel(Id.handle({selected: false}), "Viewport")) {
					var scene = iron.Scene.active;
					var cam = scene.cameras[0];
					ui.row([1/2,1/2]);
					cameraType = ui.combo(Id.handle({position: cameraType}), ["Orbit", "Fly"], "Camera");
					var fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
					cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
					if (ui.changed) {
						cam.buildProjection();
					}
					if (ui.button("Reset")) {
						for (o in scene.raw.objects) {
							if (o.type == 'camera_object') {
								cam.transform.local.setF32(o.transform.values);
								cam.transform.decompose();
								fovHandle.value = 0.92;
								cam.buildProjection();
								currentObject.transform.reset();
								dirty = true;
								break;
							}
						}
					}

					ui.text("Lighting");
					ui.image(envThumbCay);
					var p = iron.Scene.active.world.getGlobalProbe();
					ui.row([1/2, 1/2]);
					var envType = ui.combo(Id.handle({position: 0}), ["Indoor"], "Map");
					p.raw.strength = ui.slider(Id.handle({value: p.raw.strength}), "Environment", 0.0, 5.0, true);
					
					ui.row([1/2, 1/2]);
					var showType = ui.combo(Id.handle({position: 0}), ["Render", "Base Color", "Normal", "Occlusion", "Roughness", "Metallic"], "Show");
					if (iron.Scene.active.lamps.length > 0) {
						var lamp = iron.Scene.active.lamps[0];
						lamp.data.raw.strength = ui.slider(Id.handle({value: lamp.data.raw.strength / 10}), "Light", 0.0, 5.0, true) * 10;
					}

					displaceStrength = ui.slider(Id.handle({value: displaceStrength}), "Displace", 0.0, 2.0, true);
					if (ui.changed) {
						UINodes.inst.parseMeshMaterial();
					}
				}

				// Draw plugins
				for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
			}
			if (ui.tab(htab, "Import")) {

				ui.row([1/2, 1/2]);
				if (ui.button("Import Mesh")) {
					arm.App.showFiles = true;
					arm.App.foldersOnly = false;
					arm.App.filesDone = function(path:String) {
						importMesh(path);
					}
				}
				if (ui.button("Import Texture")) {
					arm.App.showFiles = true;
					arm.App.foldersOnly = false;
					arm.App.filesDone = function(path:String) {
						UITrait.inst.importAsset(path);
					}
				}

				if (assets.length > 0) {
					var i = assets.length - 1;
					while (i >= 0) {
						var asset = assets[i];
						if (ui.image(UITrait.inst.getImage(asset)) == State.Started) {
							arm.App.dragAsset = asset;
						}
						ui.row([1/8, 7/8]);
						var b = ui.button("X");
						asset.name = ui.textInput(Id.handle().nest(asset.id, {text: asset.name}), "", Right);
						assetNames[i] = asset.name;
						if (b) {
							UITrait.inst.getImage(asset).unload();
							assets.splice(i, 1);
							assetNames.splice(i, 1);
						}
						i--;
					}
				}
				else {
					ui.text("(Drag & drop assets here)");
				}
			}

			if (ui.tab(htab, "Export")) {
				if (ui.button("Export Textures")) {
					var textureSize = getTextureRes();

					arm.App.showFiles = true;
					arm.App.foldersOnly = true;
					// var path = 'C:\\Users\\lubos\\Documents\\';
					arm.App.filesDone = function(path:String) {
						var bo = new haxe.io.BytesOutput();
						var pixels = selectedLayer.texpaint.getPixels();
						var rgb = haxe.io.Bytes.alloc(textureSize * textureSize * 3);
						// BGRA to RGB
						for (i in 0...textureSize * textureSize) {
							rgb.set(i * 3 + 0, pixels.get(i * 4 + 2));
							rgb.set(i * 3 + 1, pixels.get(i * 4 + 1));
							rgb.set(i * 3 + 2, pixels.get(i * 4 + 0));
						}
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
						// var jpgdata:iron.format.jpg.Data.Data = {
						// 	width: textureSize,
						// 	height: textureSize,
						// 	quality: 80,
						// 	pixels: rgb
						// };
						// var jpgwriter = new iron.format.jpg.Writer(bo);
						// jpgwriter.write(jpgdata);
						#if kha_krom
						if (isBase) Krom.fileSaveBytes(path + "/tex_basecol.png", bo.getBytes().getData());
						#end

						pixels = selectedLayer.texpaint_nor.getPixels();
						for (i in 0...textureSize * textureSize) {
							rgb.set(i * 3 + 0, pixels.get(i * 4 + 2));
							rgb.set(i * 3 + 1, pixels.get(i * 4 + 1));
							rgb.set(i * 3 + 2, pixels.get(i * 4 + 0));
						}
						bo = new haxe.io.BytesOutput();
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
						#if kha_krom
						if (isNor) Krom.fileSaveBytes(path + "/tex_nor.png", bo.getBytes().getData());
						#end

						// for (i in 0...textureSize * textureSize) {
						// 	rgb.set(i * 3 + 0, pixels.get(i * 4 + 3));
						// 	rgb.set(i * 3 + 1, pixels.get(i * 4 + 3));
						// 	rgb.set(i * 3 + 2, pixels.get(i * 4 + 3));
						// }
						// bo = new haxe.io.BytesOutput();
						// var pngwriter = new iron.format.png.Writer(bo);
						// pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
						// #if kha_krom
						// Krom.fileSaveBytes(path + "/tex_height.png", bo.getBytes().getData());
						// #end

						pixels = selectedLayer.texpaint_pack.getPixels(); // occ, rough, met

						if (outputType == 0) {
							for (i in 0...textureSize * textureSize) {
								rgb.set(i * 3 + 0, pixels.get(i * 4));
								rgb.set(i * 3 + 1, pixels.get(i * 4));
								rgb.set(i * 3 + 2, pixels.get(i * 4));
							}
							bo = new haxe.io.BytesOutput();
							var pngwriter = new iron.format.png.Writer(bo);
							pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
							#if kha_krom
							if (isOcc) Krom.fileSaveBytes(path + "/tex_occ.png", bo.getBytes().getData());
							#end

							for (i in 0...textureSize * textureSize) {
								rgb.set(i * 3 + 0, pixels.get(i * 4 + 1));
								rgb.set(i * 3 + 1, pixels.get(i * 4 + 1));
								rgb.set(i * 3 + 2, pixels.get(i * 4 + 1));
							}
							bo = new haxe.io.BytesOutput();
							var pngwriter = new iron.format.png.Writer(bo);
							pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
							#if kha_krom
							if (isRough) Krom.fileSaveBytes(path + "/tex_rough.png", bo.getBytes().getData());
							#end

							for (i in 0...textureSize * textureSize) {
								rgb.set(i * 3 + 0, pixels.get(i * 4 + 2));
								rgb.set(i * 3 + 1, pixels.get(i * 4 + 2));
								rgb.set(i * 3 + 2, pixels.get(i * 4 + 2));
							}
							bo = new haxe.io.BytesOutput();
							var pngwriter = new iron.format.png.Writer(bo);
							pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
							#if kha_krom
							if (isMet) Krom.fileSaveBytes(path + "/tex_met.png", bo.getBytes().getData());
							#end
						}
						else { // UE4
							for (i in 0...textureSize * textureSize) {
								rgb.set(i * 3 + 0, pixels.get(i * 4));
								rgb.set(i * 3 + 1, pixels.get(i * 4 + 1));
								rgb.set(i * 3 + 2, pixels.get(i * 4 + 2));
							}
							bo = new haxe.io.BytesOutput();
							var pngwriter = new iron.format.png.Writer(bo);
							pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
							#if kha_krom
							if (isOcc) Krom.fileSaveBytes(path + "/tex_orm.png", bo.getBytes().getData());
							#end
						}

						pixels = selectedLayer.texpaint_opt.getPixels();
						for (i in 0...textureSize * textureSize) {
							rgb.set(i * 3 + 0, pixels.get(i * 4 + 2));
							rgb.set(i * 3 + 1, pixels.get(i * 4 + 2));
							rgb.set(i * 3 + 2, pixels.get(i * 4 + 2));
						}
						bo = new haxe.io.BytesOutput();
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
						#if kha_krom
						if (isMet) Krom.fileSaveBytes(path + "/tex_oeh.png", bo.getBytes().getData());
						#end
					}
				}

				var hres = Id.handle({position: textureRes});
				textureRes = ui.combo(hres, ["1K", "2K", "4K", "8K", "16K", "20K"], "Res", true);
				if (hres.changed) {
					iron.App.notifyOnRender(resizeLayers);
				}
				ui.combo(Id.handle(), ["8bit"], "Color", true);
				ui.combo(Id.handle(), ["png"], "Format", true);
				outputType = ui.combo(Id.handle(), ["Generic", "UE4 (ORM)"], "Output", true);
				ui.text("Channels");
				ui.row([1/2, 1/2]);
				isBase = ui.check(Id.handle({selected: isBase}), "Base Color");
				isOpac = ui.check(Id.handle({selected: isOpac}), "Opacity");
				ui.row([1/2, 1/2]);
				isOcc = ui.check(Id.handle({selected: isOcc}), "Occlusion");
				isRough = ui.check(Id.handle({selected: isRough}), "Roughness");
				ui.row([1/2, 1/2]);
				isMet = ui.check(Id.handle({selected: isMet}), "Metallic");
				isNor = ui.check(Id.handle({selected: isNor}), "Normal Map");
			}

			if (ui.tab(htab, "Preferences")) {
				var hscale = Id.handle({value: armory.data.Config.raw.window_scale});
				ui.slider(hscale, "UI Scale", 0.5, 4.0, true);
				if (ui.changed && !iron.system.Input.getMouse().down()) {
					armory.data.Config.raw.window_scale = hscale.value;
					ui.setScale(hscale.value);
					windowW = Std.int(defaultWindowW * armory.data.Config.raw.window_scale);
					arm.App.resize();
				}
				penPressure = ui.check(Id.handle({selected: penPressure}), "Pen Pressure");
				drawWorld = ui.check(Id.handle({selected: drawWorld}), "Envmap");
				if (ui.changed) {
					dirty = true;
				}
				if (!drawWorld) {
					var hwheel = Id.handle();
					worldColor = Ext.colorWheel(ui, hwheel);
				}
				armory.data.Config.raw.window_vsync = ui.check(Id.handle({selected: armory.data.Config.raw.window_vsync}), "VSync");
				if (ui.button("Save")) {
					#if kha_krom
					Krom.fileSaveBytes("config.arm", haxe.io.Bytes.ofString(haxe.Json.stringify(armory.data.Config.raw)).getData());
					#end
				}
				ui.text("v0.3 armorpaint.org");

				if (ui.panel(Id.handle({selected: true}), "Console")) {
					ui.text(lastTrace);
				}
			}
		}
		ui.end();
		g.begin(false);

		if (arm.App.dragAsset != null) {
			dirty = true;
			var mouse = iron.system.Input.getMouse();
			var ratio = 128 / getImage(arm.App.dragAsset).width;
			var h = getImage(arm.App.dragAsset).height * ratio;
			g.drawScaledImage(getImage(arm.App.dragAsset), mouse.x, mouse.y, 128, h);
		}
	}

	public function getImage(asset:TAsset):kha.Image {
		return Canvas.assetMap.get(asset.id);
	}

	public function importMesh(path:String) {
		var p = path.toLowerCase();
		if (StringTools.endsWith(p, ".obj")) importObj(path);
		else if (StringTools.endsWith(p, ".gltf")) importGltf(path);
		else if (StringTools.endsWith(p, ".fbx")) importFbx(path);
		// else if (StringTools.endsWith(p, ".blend")) importBlend(path);
		else showMessage("Error: Unknown mesh format");
	}

	function importObj(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.obj.ObjParser(b);
			if (obj.texa == null) {
				showMessage("Error: Invalid mesh - no UVs found");
				return;
			}
			makeMesh(obj);
		});
	}

	function importGltf(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.gltf.GltfParser(b);
			if (obj.texa == null) {
				showMessage("Error: Invalid mesh - no UVs found");
				return;
			}
			makeMesh(obj);
		});
	}

	function importFbx(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.fbx.FbxParser(b);
			if (obj.texa == null) {
				showMessage("Error: Invalid mesh - no UVs found");
				return;
			}
			makeMesh(obj);
		});
	}

	function importBlend(path:String) {
		// iron.data.Data.getBlob(path, function(b:kha.Blob) {
			// var obj = new blend.Loader(b);
			// if (obj.texa == null) {
				// showMessage("Error: Invalid mesh - no UVs found");
				// return;
			// }
			// makeMesh(obj);
		// });
	}

	function makeMesh(mesh:Dynamic) {
		var raw:TMeshData = {
			name: "Mesh",
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos" },
				{ values: mesh.nora, attrib: "nor" },
				{ values: mesh.texa, attrib: "tex" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			]
		};

		new MeshData(raw, function(md:MeshData) {
			currentObject.data.delete();
			iron.App.notifyOnRender(initLayers);
			
			currentObject.setData(md);
			
			// Scale to bounds
			md.geom.calculateAABB();
			var r = Math.sqrt(md.geom.aabb.x * md.geom.aabb.x + md.geom.aabb.y * md.geom.aabb.y + md.geom.aabb.z * md.geom.aabb.z);
			currentObject.transform.scale.set(3 / r, 3 / r, 3 / r);
			currentObject.transform.buildMatrix();

			// Face camera
			// currentObject.transform.setRotation(Math.PI / 2, 0, 0);
			
			dirty = true;
		});
	}
}

typedef TAPConfig = {
	@:optional var debug_console:Null<Bool>;
	@:optional var window_mode:Null<Int>; // window, borderless, fullscreen
	@:optional var window_w:Null<Int>;
	@:optional var window_h:Null<Int>;
	@:optional var window_resizable:Null<Bool>;
	@:optional var window_maximizable:Null<Bool>;
	@:optional var window_minimizable:Null<Bool>;
	@:optional var window_vsync:Null<Bool>;
	@:optional var window_msaa:Null<Int>;
	@:optional var window_scale:Null<Float>;
	@:optional var rp_supersample:Null<Float>;
	@:optional var rp_shadowmap:Null<Int>;
	@:optional var rp_voxelgi:Null<Int>; // off, ao, ao_revox, gi, gi_revox
	@:optional var rp_ssgi:Null<Bool>;
	@:optional var rp_ssr:Null<Bool>;
	@:optional var rp_bloom:Null<Bool>;
	@:optional var rp_motionblur:Null<Bool>;
	@:optional var plugins:Array<String>;
}
