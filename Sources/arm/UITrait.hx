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

class UITrait extends armory.Trait {

	public static var uienabled = true;
	public static var isScrolling = false;
	public static var isDragging = false;
	public static var dragAsset:TAsset = null;

	public static var showFiles = false;
	public static var filesDone:String->Void;
	public static var showSplash = false;
	public static var show = true;
	public static var dirty = true;

	var dropPath = "";
	var bundled:Map<String, kha.Image> = new Map();
	var ui:Zui;
	var uimodal:Zui;

	public static var windowW = 240; // Panel width

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

	public static function depthDirty():Bool {
		if (!dirty && !firstPaint) return false;
		dirty = false;
		return true;
	}

	static var _onBrush:Array<Void->Void> = [];

	public static function notifyOnBrush(f:Void->Void) {
		_onBrush.push(f);
	}

	public static var firstPaint = true;
	public static var paint = false;
	public static var paintX = 0.0;
	public static var paintY = 0.0;
	public static var lastPaintX = 0.0;
	public static var lastPaintY = 0.0;
	static var painted = 0;
	public static function paintDirty():Bool {
		// Paint bounds
		if (paintX > iron.App.w()) return false;
		// if (UINodes.show && paintY > UINodes.wy) return false;
		// if (UINodesBrush.show && paintY > UINodesBrush.wy) return false;

		// Prevent painting the same spot - save perf & reduce projection paint jittering caused by _sub offset
		if (paintX == lastPaintX && paintY == lastPaintY) painted++;
		else painted = 0;
		if (painted > 8) return false;
		lastPaintX = paintX;
		lastPaintY = paintY;

		if (paint) firstPaint = false;
		return paint;
	}

	// TODO: Recompile mat instead of uniforms?
	public static var brushRadius = 0.5;
	public static var brushOpacity = 1.0;
	public static var brushScale = 0.5;
	public static var brushStrength = 1.0;
	public static var brushBias = 1.0;
	public static var brushPaint = 0;
	function linkFloat(link:String):Null<Float> {

		if (link == '_brushRadius') {
			return brushRadius / 15.0;
		}
		else if (link == '_brushOpacity') {
			return brushOpacity;
		}
		else if (link == '_brushScale') {
			return brushScale * 2.0;
		}
		else if (link == '_brushStrength') {
			return brushStrength * brushStrength * 100;
		}
		// else if (link == '_brushPaint') {
			// return brushPaint;
		// }

		return null;
	}

	var sub = 0;
	var vsub = new iron.math.Vec4();
	function linkVec2(link:String):iron.math.Vec4 {

		if (link == '_sub') {
			var seps = brushBias * 0.0001;
			sub = (sub + 1) % 9;
			if (sub == 0) vsub.set(0.0 + seps, 0.0, 0.0);
			else if (sub == 1) vsub.set(0.0 - seps, 0.0, 0.0);
			else if (sub == 2) vsub.set(0.0, 0.0 + seps, 0.0);
			else if (sub == 3) vsub.set(0.0, 0.0 - seps, 0.0);
			else if (sub == 4) vsub.set(0.0 + seps, 0.0 + seps, 0.0);
			else if (sub == 5) vsub.set(0.0 - seps, 0.0 - seps, 0.0);
			else if (sub == 6) vsub.set(0.0 + seps, 0.0 - seps, 0.0);
			else if (sub == 7) vsub.set(0.0 - seps, 0.0 + seps, 0.0);
			else if (sub == 8) vsub.set(0.0, 0.0, 0.0);
			return vsub;
		}

		return null;
	}

	var font:kha.Font;
	public function new() {
		super();

		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];

		iron.data.Data.getFont('droid_sans.ttf', function(f:kha.Font) {
			font = f;
			zui.Themes.dark.FILL_WINDOW_BG = true;
			zui.Nodes.getEnumTexts = getEnumTexts;
			zui.Nodes.mapEnum = mapEnum;
			ui = new Zui( { font: f } );
			uimodal = new Zui( { font: f } );
			// ui = new Zui( { font: f, scaleFactor: 8, theme: zui.Themes.light } ); ////
			loadBundled(['brush', 'mat', 'matbrush', 'cay_thumb'], done);
		});

		kha.System.notifyOnDropFiles(function(filePath:String) {
			dropPath = StringTools.rtrim(filePath);
		});
	}

	function importAsset(path:String) {
		if (!StringTools.endsWith(path, ".jpg") &&
			!StringTools.endsWith(path, ".png") &&
			!StringTools.endsWith(path, ".hdr")) return;
		
		iron.data.Data.getImage(path, function(image:kha.Image) {
			var ar = path.split("/");
			var name = ar[ar.length - 1];
			var asset:TAsset = {name: name, file: path, id: assetId++};
			assets.push(asset);
			assetNames.push(name);
			Canvas.assetMap.set(asset.id, image);
			hwnd.redraws = 2;
		});
	}

	function getEnumTexts():Array<String> {
		return assetNames.length > 0 ? assetNames : [""];
	}

	function mapEnum(s:String):String {
		for (a in assets) if (a.name == s) return a.file;
		return "";
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
		});
	}

	function resize() {
		iron.Scene.active.camera.buildProjection();
	}

	function update() {
		isScrolling = ui.isScrolling;
		updateUI();
		updateSplash();
		updateFiles();
	}

	function updateUI() {
		paint = false;
		var mouse = iron.system.Input.getMouse();
		// if (mouse.started() && mouse.x < 50 && mouse.y < 50) show = !show;

		isDragging = dragAsset != null;
		if (mouse.released() && isDragging) {
			if (UINodes.show && mouse.x > UINodes.wx && mouse.y > UINodes.wy) {
				var index = 0;
				for (i in 0...assets.length) if (assets[i] == dragAsset) { index = i; break; }
				UINodes.acceptDrag(index);
			}
			dragAsset = null;
		}

		if (!show) return;
		if (!UITrait.uienabled) return;

		if (mouse.down()) {
			for (f in _onBrush) f();
		}
	}

	var modalW = 625;
	var modalH = 545;
	var modalHeaderH = 66;
	var modalRectW = 625; // No shadow
	var modalRectH = 545;
	function updateSplash() {
		if (!showSplash) return;

		var mouse = iron.system.Input.getMouse();

		if (mouse.released()) {
			var left = iron.App.w() / 2 - modalRectW / 2;
			var right = iron.App.w() / 2 + modalRectW / 2;
			var top = iron.App.h() / 2 - modalRectH / 2;
			var bottom = iron.App.h() / 2 + modalRectH / 2;
			if (mouse.x < left || mouse.x > right || mouse.y < top + modalHeaderH || mouse.y > bottom) {
				showSplash = false;
			}
		}
	}

	function updateFiles() {
		if (!showFiles) return;

		var mouse = iron.system.Input.getMouse();

		if (mouse.released()) {
			var left = iron.App.w() / 2 - modalRectW / 2;
			var right = iron.App.w() / 2 + modalRectW / 2;
			var top = iron.App.h() / 2 - modalRectH / 2;
			var bottom = iron.App.h() / 2 + modalRectH / 2;
			if (mouse.x < left || mouse.x > right || mouse.y < top + modalHeaderH || mouse.y > bottom) {
				showFiles = false;
			}
		}
	}

	public static var texpaint:kha.Image;
	public static var texpaint_nor:kha.Image;
	public static var texpaint_pack:kha.Image;
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

		UITrait.dirty = true;

		iron.App.removeRender(resizeTargetsHandler);
	}

	var lastW = 0;
	var lastH = 0;
	function render(g:kha.graphics2.Graphics) {
		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		if (dropPath != "") {
			if (StringTools.endsWith(dropPath, ".obj")) importMesh(dropPath);
			else importAsset(dropPath);
			dropPath = "";
		}

		uienabled = !showSplash && !showFiles;
		renderUI(g);
		// if (showSplash) renderSplash(g);
		if (showFiles) renderFiles(g);

		if (lastW > 0 && (lastW != arm.App.realw() || lastH != arm.App.realh())) {
			resize();
		}
		lastW = arm.App.realw();
		lastH = arm.App.realh();

		// var ready = showFiles || dirty;
		// TODO: Texture params get overwritten
		// if (ready) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
		if (UINodes.inst._matcon != null) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;

		// iron.Scene.active.camera.renderPath.ready = ready;
		// dirty = false;
	}

	var assets:Array<TAsset> = [];
	var assetNames:Array<String> = [];
	var assetId = 0;

	public static var cameraType = 0;
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

	var isBase = true;
	var isOcc = true;
	var isRough = true;
	var isMet = true;
	var isNor = true;
	var hwnd = Id.handle();
	function renderUI(g:kha.graphics2.Graphics) {
		if (!show) return;

		if (!UITrait.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (UITrait.uienabled && !ui.inputRegistered) ui.registerInput();

		var matImg = bundled.get('mat');
		var matBrushImg = bundled.get('matbrush');
		var envThumbCay = bundled.get('cay_thumb');
		var brushImg = bundled.get('brush');
		var mouse = iron.system.Input.getMouse();
		g.color = 0xffffffff;

		// Brush
		if (UITrait.uienabled) {
			var psize = Std.int(brushImg.width * brushRadius);
			// g.imageScaleQuality = kha.graphics2.ImageScaleQuality.High;
			g.drawScaledImage(brushImg, mouse.x - psize / 2, mouse.y - psize / 2, psize, psize);
		}

		g.end();
		ui.begin(g);
		// ui.begin(rt.g2); ////
		
		if (ui.window(hwnd, arm.App.realw() - windowW, 0, windowW, iron.App.h())) {

			var htab = Id.handle({position: 0});

			if (ui.tab(htab, "Project")) {
				// ui.row([1/2, 1/2]);
				// ui.button("Open");
				// ui.button("Save");

				// if (ui.button("Help")) {
					// showSplash = true;
				// }

				if (ui.button("Import Mesh")) {
					showFiles = true;
					filesDone = function(path:String) {
						importMesh(path);
					}
				}
				else if (ui.isHovered) ui.tooltip("Drop .obj mesh here"); 

				if (ui.button("Export Textures")) {



					var textureSize = getTextureRes();

					showFiles = true;
					// var path = 'C:\\Users\\lubos\\Documents\\';
					filesDone = function(path:String) {
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
						Krom.fileSaveBytes(path + "/tex_basecol.png", bo.getBytes().getData());
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
						Krom.fileSaveBytes(path + "/tex_nor.png", bo.getBytes().getData());
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
						for (i in 0...textureSize * textureSize) {
							rgb.set(i * 3 + 0, pixels.get(i * 4));
							rgb.set(i * 3 + 1, pixels.get(i * 4));
							rgb.set(i * 3 + 2, pixels.get(i * 4));
						}
						bo = new haxe.io.BytesOutput();
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.buildRGB(textureSize, textureSize, rgb));
						#if kha_krom
						Krom.fileSaveBytes(path + "/tex_occ.png", bo.getBytes().getData());
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
						Krom.fileSaveBytes(path + "/tex_rough.png", bo.getBytes().getData());
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
						Krom.fileSaveBytes(path + "/tex_met.png", bo.getBytes().getData());
						#end
					}
				}

				if (ui.panel(Id.handle({selected: false}), "Properties")) {

					var hres = Id.handle({position: textureRes});
					textureRes = ui.combo(hres, ["1K", "2K", "4K", "8K", "16K", "20K"], "Res", true);
					if (hres.changed) {
						iron.App.notifyOnRender(resizeTargetsHandler);
					}
					ui.combo(Id.handle(), ["8bit"], "Color", true);
					ui.combo(Id.handle(), ["png"], "Format", true);
					// ui.text("Channels");
					ui.row([1/2, 1/2]);
					isBase = ui.check(Id.handle({selected: isBase}), "Base Color");
					isOcc = ui.check(Id.handle({selected: isOcc}), "Occlusion");
					ui.row([1/2, 1/2]);
					isRough = ui.check(Id.handle({selected: isRough}), "Roughness");
					isMet = ui.check(Id.handle({selected: isMet}), "Metallic");
					isNor = ui.check(Id.handle({selected: isNor}), "Normal Map");
				}

				if (ui.panel(Id.handle({selected: true}), "Camera")) {

					ui.row([1/2,1/2]);
					cameraType = ui.combo(Id.handle({position: cameraType}), ["Orbit", "Fly"], "Camera");
					if (ui.button("Reset")) {
						var scene = iron.Scene.active;
						var cam = scene.cameras[0];
						for (o in scene.raw.objects) {
							if (o.type == 'camera_object') {
								cam.transform.local.setF32(o.transform.values);
								cam.transform.decompose();
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

				if (ui.panel(Id.handle({selected: true}), "Environment")) {
					ui.image(envThumbCay);
					var p = iron.Scene.active.world.getGlobalProbe();
					ui.row([1/2, 1/2]);
					var envType = ui.combo(Id.handle({position: 0}), ["Indoor"], "Map");
					p.raw.strength = ui.slider(Id.handle({value: p.raw.strength}), "Strength", 0.0, 5.0, true);
				}

				if (ui.panel(Id.handle({selected: true}), "Brush")) {
					ui.image(matBrushImg);
					// if (ui.button("Nodes")) {
						// UINodesBrush.show = !UINodesBrush.show;
						// UINodes.show = false;
					// }
					ui.row([1/2, 1/2]);
					var paintHandle = Id.handle();
					brushPaint = ui.combo(paintHandle, ["UV", "Project"], "Paint");
					if (paintHandle.changed) {
						UINodes.changed = true;
					}
					brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 4.0, true);
					ui.row([1/2, 1/2]);
					// ui.combo(Id.handle(), ["Draw", "Fill"], "Type");
					ui.combo(Id.handle(), ["Draw"], "Type");
					ui.combo(Id.handle(), ["Add"], "Blending");
					ui.row([1/2, 1/2]);
					brushRadius = ui.slider(Id.handle({value: brushRadius}), "Radius", 0.0, 2.0, true);
					brushOpacity = ui.slider(Id.handle({value: brushOpacity}), "Opacity", 0.0, 1.0, true);
					ui.row([1/2, 1/2]);
					brushStrength = ui.slider(Id.handle({value: brushStrength}), "Strength", 0.0, 1.0, true);
					brushScale = ui.slider(Id.handle({value: brushScale}), "Scale", 0.0, 1.0, true);
				}
				ui.separator();

				if (ui.panel(Id.handle({selected: true}), "Material")) {
					ui.image(matImg);
					if (ui.button("Nodes")) {
						UINodes.show = !UINodes.show;
						UINodesBrush.show = false;
						resize();
					}
				}
			}

			if (ui.tab(htab, "Assets")) {

				if (ui.button("Import")) {
					showFiles = true;
					filesDone = function(path:String) {
						importAsset(path);
					}
				}

				if (assets.length > 0) {
					var i = assets.length - 1;
					while (i >= 0) {
						var asset = assets[i];
						if (ui.image(getImage(asset)) == State.Started) {
							dragAsset = asset;
						}
						ui.row([1/8, 7/8]);
						var b = ui.button("X");
						asset.name = ui.textInput(Id.handle().nest(asset.id, {text: asset.name}), "", Right);
						assetNames[i] = asset.name;
						if (b) {
							getImage(asset).unload();
							assets.splice(i, 1);
							assetNames.splice(i, 1);
						}
						i--;
					}
				}
				else {
					ui.text("(Drag & drop assets)");
				}

				// if (ui.panel(Id.handle({selected: false}), "LAYERS")) {}
				// ui.separator();

				// if (ui.panel(Id.handle({selected: false}), "HISTORY")) {}
				// ui.separator();
			}
		}
		ui.end();
		g.begin(false);

		if (dragAsset != null) {
			UITrait.dirty = true;
			var mouse = iron.system.Input.getMouse();
			var ratio = 128 / getImage(dragAsset).width;
			var h = getImage(dragAsset).height * ratio;
			g.drawScaledImage(getImage(dragAsset), mouse.x, mouse.y, 128, h);
		}
	}

	function renderSplash(g:kha.graphics2.Graphics) {
		var left = iron.App.w() / 2 - modalW / 2;
		var top = iron.App.h() / 2 - modalH / 2;
		var splashImg = bundled.get('splash');
		g.color = 0xffffffff;
		g.drawScaledImage(splashImg, left, top, modalW, modalH);

		uimodal.beginLayout(g, Std.int(left) + 200, Std.int(top) + 500, 120);
		uimodal.button("Manual");
		uimodal.endLayout();
	}

	function getImage(asset:TAsset):kha.Image {
		return Canvas.assetMap.get(asset.id);
	}

	var path = '/';
	function renderFiles(g:kha.graphics2.Graphics) {
		var left = iron.App.w() / 2 - modalW / 2;
		var top = iron.App.h() / 2 - modalH / 2;
		var filesImg = bundled.get('files');
		g.color = 0xff202020;
		// g.drawScaledImage(filesImg, left, top, modalW, modalH);
		g.fillRect(left, top, modalW, modalH);

		var leftRect = Std.int(iron.App.w() / 2 - modalRectW / 2);
		var rightRect = Std.int(iron.App.w() / 2 + modalRectW / 2);
		var topRect = Std.int(iron.App.h() / 2 - modalRectH / 2);
		var bottomRect = Std.int(iron.App.h() / 2 + modalRectH / 2);
		topRect += modalHeaderH;
		
		g.end();
		uimodal.begin(g);
		if (uimodal.window(Id.handle(), leftRect, topRect, modalRectW, modalRectH - 100)) {
			var pathHandle = Id.handle();
			pathHandle.text = uimodal.textInput(pathHandle);
			path = zui.Ext.fileBrowser(uimodal, pathHandle);
		}
		uimodal.end(false);
		g.begin(false);

		uimodal.beginLayout(g, rightRect - 100, bottomRect - 30, 100);
		if (uimodal.button("OK")) {
			showFiles = false;
			filesDone(path);
			UITrait.dirty = true;
		}
		uimodal.endLayout(false);

		uimodal.beginLayout(g, rightRect - 200, bottomRect - 30, 100);
		if (uimodal.button("Cancel")) {
			showFiles = false;
			UITrait.dirty = true;
		}
		uimodal.endLayout();
	}

	function importMesh(path:String) {

		iron.data.Data.getBlob(path, function(b:kha.Blob) {

			var obj = new iron.format.obj.Loader(b.toString());
			var pa = new TFloat32Array(obj.indexedVertices.length);
			for (i in 0...pa.length) pa[i] = obj.indexedVertices[i];
			var uva = new TFloat32Array(obj.indexedUVs.length);
			for (i in 0...uva.length) uva[i] = obj.indexedUVs[i];
			var na = new TFloat32Array(obj.indexedNormals.length);
			for (i in 0...na.length) na[i] = obj.indexedNormals[i];
			var ia = new TUint32Array(obj.indices.length);
			for (i in 0...ia.length) ia[i] = obj.indices[i];

			var raw:TMeshData = {
				name: "Mesh",
				vertex_arrays: [
					{
						values: pa,
						attrib: "pos"
					},
					{
						values: na,
						attrib: "nor"
					},
					{
						values: uva,
						attrib: "tex"
					}
				],
				index_arrays: [
					{
						values: ia,
						material: 0
					}
				]
			};

			new MeshData(raw, function(md:MeshData) {
				currentObject.data.delete();
				iron.App.notifyOnRender(clearTargetsHandler);
				
				currentObject.setData(md);
				
				// Scale to bounds
				md.geom.calculateAABB();
				var r = Math.sqrt(md.geom.aabb.x * md.geom.aabb.x + md.geom.aabb.y * md.geom.aabb.y + md.geom.aabb.z * md.geom.aabb.z);
				if (r > 3) {
					currentObject.transform.scale.set(3 / r, 3 / r, 3 / r);
					currentObject.transform.buildMatrix();
				}

				// Face camera
				currentObject.transform.setRotation(Math.PI / 2, 0, 0);
				
				UITrait.dirty = true;
			});
		});
	}
}
