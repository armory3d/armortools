package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.object.MeshObject;

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

@:access(zui.Zui)
@:access(iron.data.Data)
class UITrait extends iron.Trait {

	public static var inst:UITrait;
	public static var defaultWindowW = 280;

	public var isScrolling = false;

	public var show = true;
	public var dirty = true;

	var message = "";
	var messageTimer = 0.0;

	public var bundled:Map<String, kha.Image> = new Map();
	var ui:Zui;

	public var windowW = 280; // Panel width

	function loadBundled(names:Array<String>, done:Void->Void) {
		var loaded = 0;
		for (s in names) {
			kha.Assets.loadImage(s, function(image:kha.Image) {
				bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	public function depthDirty():Bool {
		if (!dirty && !firstPaint) return false;
		dirty = false;
		return true;
	}

	var _onBrush:Array<Void->Void> = [];

	public function notifyOnBrush(f:Void->Void) {
		_onBrush.push(f);
	}

	public var firstPaint = true;
	public var paint = false;
	public var paintVec = new iron.math.Vec4();
	public var lastPaintX = 0.0;
	public var lastPaintY = 0.0;
	var painted = 0;
	public var brushTime = 0.0;
	public function paintDirty():Bool {
		// Paint bounds
		if (paintVec.x > 1) return false;
		// if (UINodes.inst.show && paintVec.y > UINodes.inst.wy) return false;

		// Prevent painting the same spot - save perf & reduce projection paint jittering caused by _sub offset
		var mouse = iron.system.Input.getMouse();
		if (mouse.down() && paintVec.x == lastPaintX && paintVec.y == lastPaintY) painted++;
		else painted = 0;

		if (painted > 8) return false;
		lastPaintX = paintVec.x;
		lastPaintY = paintVec.y;

		if (paint) firstPaint = false;
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
			return (brushRadius * brushNodesRadius) / 15.0;
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
			return paintVisible ? 0.0 : 1.0;
		}

		return null;
	}

	var sub = 0;
	var vec2 = new iron.math.Vec4();
	function linkVec2(link:String):iron.math.Vec4 {

		if (link == '_sub') {
			var seps = brushBias * 0.0001;
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

		return null;
	}

	function linkVec4(link:String):iron.math.Vec4 {
		if (link == '_inputBrush') {
			vec2.set(paintVec.x, paintVec.y, iron.system.Input.getMouse().down() ? 1.0 : 0.0, 0.0);
			return vec2;
		}
		return null;
	}

	public function new() {
		super();

		inst = this;

		windowW = Std.int(windowW * armory.data.Config.raw.window_scale);

		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec4Links = [linkVec4];

		var scale = armory.data.Config.raw.window_scale;
		ui = new Zui( { font: arm.App.font, scaleFactor: scale } );
		loadBundled(['cursor', 'mat', 'mat_empty', 'brush', 'cay_thumb'], done);
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
			var asset:TAsset = {name: name, file: path, id: UILibrary.inst.assetId++};
			UILibrary.inst.assets.push(asset);
			UILibrary.inst.assetNames.push(name);
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
			iron.App.notifyOnRender(clearTargetsHandler);

			// Init plugins
			// iron.data.Data.getBlob('my_plugin.js', function(blob:kha.Blob) {
				// Plugin.keep();
				// untyped __js__("(1, eval)({0})", blob.toString());
			// });
		});
	}

	function update() {
		isScrolling = ui.isScrolling;
		updateUI();

		var kb = iron.system.Input.getKeyboard();
		if (kb.started("tab")) {
			UINodes.inst.show = !UINodes.inst.show;
			arm.App.resize();
		}
	}

	function updateUI() {

		messageTimer -= iron.system.Time.delta;

		paint = false;
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if (!show) return;
		if (!arm.App.uienabled) return;

		if (mouse.down() && !kb.down("ctrl")) {
			brushTime += iron.system.Time.delta;
			for (f in _onBrush) f();
		}
		else brushTime = 0;
	}

	public var texpaint:kha.Image;
	public var texpaint_nor:kha.Image;
	public var texpaint_pack:kha.Image;
	function clearTargetsHandler(g:kha.graphics4.Graphics) {
		var pd = iron.RenderPath.active;
		texpaint = pd.renderTargets.get("texpaint").image;
		texpaint_nor = pd.renderTargets.get("texpaint_nor").image;
		texpaint_pack = pd.renderTargets.get("texpaint_pack").image;
		g.end();

		texpaint.g4.begin();
		texpaint.g4.clear(kha.Color.fromFloats(0.2, 0.2, 0.2, 0.0)); // Base
		texpaint.g4.end();

		texpaint_nor.g4.begin();
		texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor.g4.end();

		texpaint_pack.g4.begin();
		texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 1.0, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack.g4.end();

		g.begin();
		iron.App.removeRender(clearTargetsHandler);
	}

	function resizeTargetsHandler(g:kha.graphics4.Graphics) {
		var res = getTextureRes();
		var pd = iron.RenderPath.active;
		var texpaint = pd.renderTargets.get("texpaint");
		var texpaint_nor = pd.renderTargets.get("texpaint_nor");
		var texpaint_pack = pd.renderTargets.get("texpaint_pack");

		var texpaint_oldimg = texpaint.image;
		var texpaint_nor_oldimg = texpaint_nor.image;
		var texpaint_pack_oldimg = texpaint_pack.image;

		texpaint.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.Depth16);
		texpaint_nor.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		texpaint_pack.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);

		g.end();
		texpaint.image.g2.begin();
		texpaint.image.g2.drawScaledImage(texpaint_oldimg, 0, 0, res, res);
		texpaint.image.g2.end();

		texpaint_nor.image.g2.begin();
		texpaint_nor.image.g2.drawScaledImage(texpaint_nor_oldimg, 0, 0, res, res);
		texpaint_nor.image.g2.end();

		texpaint_pack.image.g2.begin();
		texpaint_pack.image.g2.drawScaledImage(texpaint_pack_oldimg, 0, 0, res, res);
		texpaint_pack.image.g2.end();
		g.begin();

		texpaint_oldimg.unload();
		texpaint_nor_oldimg.unload();
		texpaint_pack_oldimg.unload();

		dirty = true;

		iron.App.removeRender(resizeTargetsHandler);
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
		if (UINodes.inst.show && !UINodes.inst.isBrush) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.isBrush = false; }
		arm.App.resize();
	}

	function showBrushNodes() {
		if (UINodes.inst.show && UINodes.inst.isBrush) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.isBrush = true; }
		arm.App.resize();
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
	public var selected:MaterialSlot;
	var brushes:Array<BrushSlot> = null;
	public var selectedBrush:BrushSlot;
	var selectTime = 0.0;
	function renderUI(g:kha.graphics2.Graphics) {
		if (!show) return;

		if (materials == null) {
			materials = [];
			materials.push(new MaterialSlot());
			selected = materials[0];
		}

		if (brushes == null) {
			brushes = [];
			brushes.push(new BrushSlot());
			selectedBrush = brushes[0];
		}

		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();

		var brushImg = bundled.get('brush');
		var envThumbCay = bundled.get('cay_thumb');
		var cursorImg = bundled.get('cursor');
		var mouse = iron.system.Input.getMouse();
		g.color = 0xffffffff;

		// Brush
		if (arm.App.uienabled) {
			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));
			// g.imageScaleQuality = kha.graphics2.ImageScaleQuality.High;
			g.drawScaledImage(cursorImg, mouse.x - psize / 2, mouse.y - psize / 2, psize, psize);
		}

		g.end();
		ui.begin(g);
		// ui.begin(rt.g2); ////
		
		if (ui.window(hwnd, arm.App.realw() - windowW, 0, windowW, arm.App.realh())) {

			ui._y += 6;

			// ui.row([1/2, 1/2]);
			// ui.button("Open");
			// ui.button("Save");

			if (messageTimer > 0) {
				ui.text(message);
			}

			ui.row([1/2,1/2]);
			if (ui.button("Import Mesh")) {
				arm.App.showFiles = true;
				arm.App.foldersOnly = false;
				arm.App.filesDone = function(path:String) {
					importMesh(path);
				}
			}
			else if (ui.isHovered) ui.tooltip("Drop .obj mesh here"); 
			if (ui.button("Export Textures")) {
				var textureSize = getTextureRes();

				arm.App.showFiles = true;
				arm.App.foldersOnly = true;
				// var path = 'C:\\Users\\lubos\\Documents\\';
				arm.App.filesDone = function(path:String) {
					var bo = new haxe.io.BytesOutput();
					var pixels = texpaint.getPixels();
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

					pixels = texpaint_nor.getPixels();
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

					pixels = texpaint_pack.getPixels(); // occ, rough, met

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
				}
			}

			if (ui.panel(Id.handle({selected: false}), "Export")) {

				var hres = Id.handle({position: textureRes});
				textureRes = ui.combo(hres, ["1K", "2K", "4K", "8K", "16K", "20K"], "Res", true);
				if (hres.changed) {
					iron.App.notifyOnRender(resizeTargetsHandler);
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

			if (ui.panel(Id.handle({selected: false}), "Camera")) {
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
							break;
						}
					}
				}
			}


			// if (ui.panel(Id.handle({selected: true}), "TREE")) {
			// 	ui.row([1/3, 1/3, 1/3]);
			// 	if (ui.button("Mesh")) {
			// 	}
			// 	if (ui.button("Lamp")) {
			// 	}
			// 	if (ui.button("Camera")) {
			// 	}
			// 	var h = Id.handle();
			// 	ui.radio(h, 0, "Mesh");
			// 	ui.radio(h, 1, "Lamp");
			// 	ui.radio(h, 2, "Camera");
			// 	ui.radio(h, 3, "World");
			// }
			// ui.separator();

			// if (ui.panel(Id.handle({selected: true}), "PROPERTIES")) {
			// 	ui.textInput(Id.handle({text: "Mesh"}), "Name", Right);
			// 	ui.row([1/3, 1/3, 1/3]);
			// 	var strx = ui.textInput(Id.handle().nest(0, {text: "0"}), "X", Right);
			// 	var stry = ui.textInput(Id.handle().nest(1, {text: "0"}), "Y", Right);
			// 	var strz = ui.textInput(Id.handle().nest(2, {text: "0"}), "X", Right);
			// }
			// ui.separator();

			if (ui.panel(Id.handle({selected: true}), "Lighting")) {
				ui.image(envThumbCay);
				var p = iron.Scene.active.world.getGlobalProbe();
				ui.row([1/2, 1/2]);
				var envType = ui.combo(Id.handle({position: 0}), ["Indoor"], "Map");
				p.raw.strength = ui.slider(Id.handle({value: p.raw.strength}), "Strength", 0.0, 5.0, true);
				
				ui.row([1/2, 1/2]);
				var showType = ui.combo(Id.handle({position: 0}), ["Render", "Base Color", "Normal", "Occlusion", "Roughness", "Metallic"], "Show");
				if (iron.Scene.active.lamps.length > 0) {
					var lamp = iron.Scene.active.lamps[0];
					lamp.data.raw.strength = ui.slider(Id.handle({value: lamp.data.raw.strength / 10}), "Light", 0.0, 5.0, true) * 10;
				}
			}

			if (ui.panel(Id.handle({selected: true}), "Brushes")) {

				ui.row([1/2,1/2]);
				if (ui.button("New")) {
					brushes.push(new BrushSlot());
				}
				if (ui.button("Nodes")) {
					showBrushNodes();
				}

				var img = bundled.get("brush");
				var img2 = bundled.get("mat_empty");
				ui.row([1/5,1/5,1/5,1/5,1/5]);
				for (i in 0...5) {
					var im = img;
					if (brushes.length <= i) im = img2;

					if (im == img && selectedBrush == brushes[i]) {
						ui.g.color = 0xff205d9c;
						ui.g.fillRect(ui._x + 1, ui._y - 2, im.width + 3, im.height + 3);
						ui.g.color = 0xffffffff;
					}

					if (ui.image(im) == State.Started && im == img) {
						if (selectedBrush != brushes[i]) {
							selectedBrush = brushes[i];
							UINodes.inst.updateCanvasBrushMap();
							UINodes.inst.parseBrush();
						}
						if (iron.system.Time.time() - selectTime < 0.3) {
							showBrushNodes();
						}
						selectTime = iron.system.Time.time();
					}
				}

				ui.row([1/2, 1/2]);
				var typeHandle = Id.handle();
				brushType = ui.combo(typeHandle, ["Draw", "Fill", "Bake AO"], "Type");
				if (typeHandle.changed) {
					UINodes.inst.parseMaterial();
				}
				ui.combo(Id.handle(), ["Add"], "Blending");
				ui.row([1/2, 1/2]);
				var paintHandle = Id.handle();
				brushPaint = ui.combo(paintHandle, ["UV", "Project"], "Paint");
				if (paintHandle.changed) {
					UINodes.inst.parseMaterial();
				}
				brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 4.0, true);
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
					UINodes.inst.parseMaterial();
				}

				var norHandle = Id.handle({selected: paintNor});
				paintNor = ui.check(norHandle, "Normal Map");
				if (norHandle.changed) {
					UINodes.inst.updateCanvasMap();
					UINodes.inst.parseMaterial();
				}

				var roughHandle = Id.handle({selected: paintRough});
				// TODO: Use glColorMaski() to disable specific channel
				paintRough = ui.check(roughHandle, "ORM");
				if (roughHandle.changed) {
					UINodes.inst.updateCanvasMap();
					UINodes.inst.parseMaterial();
				}

				paintVisible = ui.check(Id.handle({selected: paintVisible}), "Visible Only");
			}

			if (ui.panel(Id.handle({selected: true}), "Materials")) {
					
				ui.row([1/2,1/2]);
				if (ui.button("New")) {
					materials.push(new MaterialSlot());
				}
				if (ui.button("Nodes")) {
					showMaterialNodes();
				}

				var img = bundled.get("mat");
				var img2 = bundled.get("mat_empty");
				ui.row([1/5,1/5,1/5,1/5,1/5]);
				for (i in 0...5) {
					var im = img;
					if (materials.length <= i) im = img2;

					if (im == img && selected == materials[i]) {
						ui.g.color = 0xff205d9c;
						ui.g.fillRect(ui._x + 1, ui._y - 2, im.width + 3, im.height + 3);
						ui.g.color = 0xffffffff;
					}

					if (ui.image(im) == State.Started && im == img) {
						if (selected != materials[i]) {
							selected = materials[i];
							UINodes.inst.updateCanvasMap();
							UINodes.inst.parseMaterial();
						}
						if (iron.system.Time.time() - selectTime < 0.3) {
							showMaterialNodes();
						}
						selectTime = iron.system.Time.time();
					}
				}
			}

			if (ui.panel(Id.handle({selected: true}), "Layers")) {
				if (ui.button("2D View")) {

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
			iron.App.notifyOnRender(clearTargetsHandler);
			
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
