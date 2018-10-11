package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.MaterialData;
import iron.object.Object;
import iron.object.MeshObject;
import iron.math.Mat4;
import iron.math.Math;
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
	public var image:kha.Image = null;
	#if arm_editor
	public var data:iron.data.MaterialData;
	#end
	public function new(m:iron.data.MaterialData = null) {
		image = kha.Image.createRenderTarget(50, 50); // TODO: Flickers
		#if arm_editor
		data = m;
		#end
	}
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
	public var rt:iron.RenderPath.RenderTarget;

	var ext:String;

	public function set_texpaint(img:kha.Image) {
		RenderPath.active.renderTargets.get("texpaint" + ext).image = img;
		texpaint = img;
	}

	public function set_texpaint_nor(img:kha.Image) {
		RenderPath.active.renderTargets.get("texpaint_nor" + ext).image = img;
		texpaint_nor = img;
	}

	public function set_texpaint_pack(img:kha.Image) {
		RenderPath.active.renderTargets.get("texpaint_pack" + ext).image = img;
		texpaint_pack = img;
	}

	public function set_texpaint_opt(img:kha.Image) {
		if (texpaint_opt == null) return;
		RenderPath.active.renderTargets.get("texpaint_opt" + ext).image = img;
		texpaint_opt = img;
	}

	public function new(ext = "") {
		id = counter++;
		if (ext == "") ext = id + "";
		this.ext = ext;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint" + ext;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			t.depth_buffer = "paintdb";
			rt = RenderPath.active.createRenderTarget(t);
			texpaint = rt.image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor" + ext;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			texpaint_nor = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack" + ext;
			t.width = 4096;
			t.height = 4096;
			t.format = 'RGBA32';
			texpaint_pack = RenderPath.active.createRenderTarget(t).image;
		}

		if (UITrait.inst.paintHeight) make_texpaint_opt();
	}

	public function make_texpaint_opt() {
		if (texpaint_opt != null) return;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_opt" + ext;
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
		if (texpaint_opt != null) texpaint_opt.unload();
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
	public static var undoEnabled = true;
	public static var worldColor = 0xffffffff;

	public var isScrolling = false;

	public var colorIdPicked = false;

	public var show = true;
	public var dirty = 2;
	public var materialPreview = false; // Drawing material previews
	var savedCamera = Mat4.identity();

	var message = "";
	var messageTimer = 0.0;

	public var bundled:Map<String, kha.Image> = new Map();
	var ui:Zui;

	public var windowW = 280; // Panel width
	public var toolbarw = 50;

	var colorIdHandle = Id.handle();

	var outputType = 0;
	var isBase = true;
	var isOpac = true;
	var isOcc = true;
	var isRough = true;
	var isMet = true;
	var isNor = true;
	var isHeight = true;
	var hwnd = Id.handle();
	public var materials:Array<MaterialSlot> = null;
	public var selectedMaterial:MaterialSlot;
	#if arm_editor
	var materials2:Array<MaterialSlot> = null;
	public var selectedMaterial2:MaterialSlot;
	#end
	var brushes:Array<BrushSlot> = null;
	public var selectedBrush:BrushSlot;
	public var selectedLogic:BrushSlot;
	public var layers:Array<LayerSlot> = null;
	public var undoLayers:Array<LayerSlot> = null;
	public var selectedLayer:LayerSlot;
	var selectTime = 0.0;
	public var displaceStrength = 1.0;
	public var stickerImage:kha.Image = null;
	public var stickerPreview = false;

	var first = 0;
	
	var _onBrush:Array<Void->Void> = [];

	public var paint = false;
	public var paintVec = new iron.math.Vec4();
	public var lastPaintX = 0.0;
	public var lastPaintY = 0.0;
	var painted = 0;
	public var brushTime = 0.0;
	public var pushUndo = false;


	public var selectedObject:iron.object.Object;
	public var paintObject:iron.object.Object;
	var gizmo:iron.object.Object = null;
	var gizmoX:iron.object.Object = null;
	var gizmoY:iron.object.Object = null;
	var gizmoZ:iron.object.Object = null;
	var grid:iron.object.Object = null;
	var selectedType = "";
	var axisX = false;
	var axisY = false;
	var axisZ = false;
	var axisStart = 0.0;
	var row4 = [1/4, 1/4, 1/4, 1/4];

	#if arm_editor
	public var htab = Id.handle({position: 1});
	#else
	public var htab = Id.handle({position: 0});
	#end

	function loadBundled(names:Array<String>, done:Void->Void) {
		var loaded = 0;
		for (s in names) {
			iron.data.Data.getImage(s, function(image:kha.Image) {
				bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	public function depthDirty():Bool {
		return dirty > 0 || first < 10;
	}

	public function redraw():Bool {
		if (first < 10) {
			first++;
			return true;
		}
		var m = iron.system.Input.getMouse();
		var b = m.x < iron.App.w() && (m.down() || m.down("right") || m.released() || m.released("right"));
		return b || depthDirty();
	}

	public function notifyOnBrush(f:Void->Void) {
		_onBrush.push(f);
	}

	public function paintDirty():Bool {
		#if arm_editor
		if (htab.position == 0) return false;
		#end

		// Paint bounds
		if (paintVec.x > 1 || paintVec.x < 0) return false;

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
	public var paintHeight = false;
	
	public var paintVisible = true;
	public var mirrorX = false;

	function linkFloat(object:Object, mat:MaterialData, link:String):Null<kha.FastFloat> {

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
	function linkVec2(object:Object, mat:MaterialData, link:String):iron.math.Vec4 {

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
	function linkVec4(object:Object, mat:MaterialData, link:String):iron.math.Vec4 {
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

	function linkTex(object:Object, mat:MaterialData, link:String):kha.Image {
		if (link == "_texcolorid") {
			if (UITrait.inst.assets.length == 0) return bundled.get("mat_slot.jpg");
			else return UITrait.inst.getImage(UITrait.inst.assets[colorIdHandle.position]);
		}
		return null;
	}

	public function new() {
		super();

		inst = this;

		// Init config
		apconfig = cast armory.data.Config.raw;
		if (apconfig.ui_layout == null) apconfig.ui_layout = 0;

		windowW = Std.int(defaultWindowW * apconfig.window_scale);

		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalTextureLinks.push(linkTex);

		if (materials == null) {
			materials = [];
			#if arm_editor
			iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
				materials.push(new MaterialSlot(m));
				selectedMaterial = materials[0];
			});
			#else
			materials.push(new MaterialSlot());
			selectedMaterial = materials[0];
			#end
		}
		#if arm_editor
		if (materials2 == null) {
			materials2 = [];
			iron.data.Data.getMaterial("Scene", "Material2", function(m:iron.data.MaterialData) {
				materials2.push(new MaterialSlot(m));
				selectedMaterial2 = materials2[0];
			});
		}
		#end

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
		if (undoLayers == null) {
			undoLayers = [];
			undoLayers.push(new LayerSlot("_undo"));
		}

		// Save last pos for continuos paint
		iron.App.notifyOnRender(function(g:kha.graphics4.Graphics) { //

			if (frame == 0) {
				UINodes.inst.parseMeshMaterial();
				UINodes.inst.parsePaintMaterial();
				UITrait.inst.makeMaterialPreview();
			}
			frame++;

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

		var scale = apconfig.window_scale;
		ui = new Zui( { theme: arm.App.theme, font: arm.App.font, scaleFactor: scale, color_wheel: arm.App.color_wheel } );
		loadBundled(['cursor.png', 'mat_slot.jpg', 'brush_draw.png', 'brush_erase.png', 'brush_fill.png', 'brush_bake.png', 'brush_colorid.png', 'env_thumb.jpg'], done);
	}

	function showMessage(s:String) {
		messageTimer = 3.0;
		message = s;
		hwnd.redraws = 2;
	}

	public static function checkImageFormat(path:String):Bool {
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

			// Set envmap, has to be 2K res for now
			if (StringTools.endsWith(path.toLowerCase(), ".hdr") && image.width == 2048) {
				#if kha_krom
				var sys = kha.System.systemId;
				var p = Krom.getFilesLocation() + '/' + iron.data.Data.dataPath;
				var cmft = p + "/cmft" + (sys == "Windows" ? ".exe" : sys == "Linux" ? "-linux64" : "-osx");

				var cmd = '';
				var tmp = Krom.getFilesLocation() + '/';

				// Irr
				cmd = cmft;
				cmd += ' --input "' + path + '"';
				cmd += ' --filter shcoeffs';
				cmd += ' --outputNum 1';
				cmd += ' --output0 "' + tmp + 'tmp_irr"';
				Krom.sysCommand(cmd);

				// Rad
				cmd = cmft;
				cmd += ' --input "' + path + '"';
				cmd += ' --filter radiance';
				cmd += ' --dstFaceSize 256';
				cmd += ' --srcFaceSize 256';
				cmd += ' --excludeBase false';
				cmd += ' --glossScale 8';
				cmd += ' --glossBias 3';
				cmd += ' --lightingModel blinnbrdf';
				cmd += ' --edgeFixup none';
				cmd += ' --numCpuProcessingThreads 4';
				cmd += ' --useOpenCL true';
				cmd += ' --clVendor anyGpuVendor';
				cmd += ' --deviceType gpu';
				cmd += ' --deviceIndex 0';
				cmd += ' --generateMipChain true';
				cmd += ' --inputGammaNumerator 1.0';
				cmd += ' --inputGammaDenominator 1.0';
				cmd += ' --outputGammaNumerator 1.0';
				cmd += ' --outputGammaDenominator 1.0';
				cmd += ' --outputNum 1';
				cmd += ' --output0 "' + tmp + 'tmp_rad"';
				cmd += ' --output0params hdr,rgbe,latlong';
				Krom.sysCommand(cmd);
				#end

				// Load irr
				iron.data.Data.getBlob(tmp + "tmp_irr.c", function(blob:kha.Blob) {
					var lines = blob.toString().split("\n");
					var band0 = lines[5];
					var band1 = lines[6];
					var band2 = lines[7];
					band0 = band0.substring(band0.indexOf("{"), band0.length);
					band1 = band1.substring(band1.indexOf("{"), band1.length);
					band2 = band2.substring(band2.indexOf("{"), band2.length);
					var band = band0 + band1 + band2;
					band = StringTools.replace(band, "{", "");
					band = StringTools.replace(band, "}", "");
					var ar = band.split(",");
					var buf = new kha.arrays.Float32Array(27);
					for (i in 0...ar.length) buf[i] = Std.parseFloat(ar[i]);
					iron.Scene.active.world.probe.irradiance = buf;
					dirty = 2;
				});

				// World envmap
				iron.Scene.active.world.envmap = image;

				// Load mips
				var mipsCount = 9;
				var mipsLoaded = 0;
				var mips:Array<kha.Image> = [];
				while (mips.length < mipsCount + 2) mips.push(null);
				var mw = 1024;
				var mh = 512;
				for (i in 0...mipsCount) {
					iron.data.Data.getImage(tmp + "tmp_rad_" + i + "_" + mw + "x" + mh + ".hdr", function(mip:kha.Image) {
						mips[i] = mip;
						mipsLoaded++;
						if (mipsLoaded == mipsCount) {
							// 2x1 and 1x1 mips
							mips[mipsCount] = kha.Image.create(2, 1, kha.graphics4.TextureFormat.RGBA128);
							mips[mipsCount + 1] = kha.Image.create(1, 1, kha.graphics4.TextureFormat.RGBA128);
							// Set radiance
							image.setMipmaps(mips);
							iron.Scene.active.world.probe.radiance = image;
							dirty = 2;
							// Update thumb
							bundled.set("env_thumb.jpg", mips[0]);
						}
					}, true); // Readable
					mw = Std.int(mw / 2);
					mh = Std.int(mh / 2);
				}
			}
		});
	}

	// var rt:kha.Image; ////
	// var uiWidth = 2048;
	// var uiHeight = 2048;

	public var currentObject:MeshObject;
	var frame = 0;

	public var apconfig:TAPConfig;

	function done() {

		notifyOnInit(function() {
		// iron.Scene.active.notifyOnInit(function() { ////

			// var pui = iron.Scene.active.getChild("PlaneUI"); ////
			// rt = kha.Image.createRenderTarget(uiWidth, uiHeight);
			// var mat:armory.data.MaterialData = cast(pui, iron.object.MeshObject).materials[0];
			// mat.contexts[0].textures[0] = rt; // Override diffuse texture

			#if arm_editor
			grid = iron.Scene.active.getChild(".Grid");
			gizmo = iron.Scene.active.getChild(".GizmoTranslate");
			gizmoX = iron.Scene.active.getChild("GizmoX");
			gizmoY = iron.Scene.active.getChild("GizmoY");
			gizmoZ = iron.Scene.active.getChild("GizmoZ");
			var light = iron.Scene.active.getChild("Lamp");
			light.addTrait(new armory.trait.physics.RigidBody(0.0));
			#end

			selectedObject = iron.Scene.active.getChild("Cube");
			paintObject = selectedObject;
			currentObject = cast(selectedObject, MeshObject);

			iron.App.notifyOnUpdate(update);
			iron.App.notifyOnRender2D(render);
			iron.App.notifyOnRender(initLayers);

			// Init plugins
			Plugin.keep();
			if (apconfig.plugins != null) {
				for (plugin in apconfig.plugins) {
					iron.data.Data.getBlob(plugin, function(blob:kha.Blob) {
						#if js
						untyped __js__("(1, eval)({0})", blob.toString());
						#end
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

		camBall();

		for (p in Plugin.plugins) if (p.update != null) p.update();
	}

	function camBall() {
		if (iron.system.Input.occupied) return;
		if (!arm.App.uienabled) return;
		if (UITrait.inst.isScrolling) return;
		if (arm.App.isDragging) return;
		if (UITrait.inst.cameraType != 0) return;
		if (!selectedObject.visible) return;

		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		// Paint bounds
		if (mouse.x > iron.App.w()) return;

		if (mouse.down("right") || (mouse.down("left") && kb.down("control"))) {
			UITrait.inst.dirty = 2;
			
			// Rotate X
			// if (!kb.down("alt")) {
				selectedObject.transform.rotate(new iron.math.Vec4(0, 0, 1), mouse.movementX / 100);
				selectedObject.transform.buildMatrix();
			// }
			
			// Rotate Y
			if (!kb.down("shift")) {
				var v = selectedObject.transform.right();
				v.normalize();
				selectedObject.transform.rotate(v, mouse.movementY / 100);
				selectedObject.transform.buildMatrix();
			}
		}
	}

	public function makeStickerPreview() {
		if (stickerImage == null) stickerImage = kha.Image.createRenderTarget(512, 512);

		var cube:MeshObject = cast iron.Scene.active.getChild("Cube");
		cube.visible = false;

		var plane:MeshObject = cast iron.Scene.active.getChild("Plane");
		plane.visible = true;
		currentObject = plane;

		savedCamera.setFrom(iron.Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		iron.Scene.active.camera.transform.setMatrix(m);
		iron.Scene.active.camera.buildMatrix();

		dirty = 2;
		stickerPreview = true;
		arm.App.resize();

		UINodes.inst.parseMeshPreviewMaterial();

		// TAA
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);

		plane.visible = false;
		cube.visible = true;
		currentObject = cube;

		dirty = 2;
		stickerPreview = false;

		iron.Scene.active.camera.transform.setMatrix(savedCamera);
		iron.Scene.active.camera.buildMatrix();
		arm.App.resize();
		iron.Scene.active.camera.buildMatrix();

		UINodes.inst.parseMeshMaterial();

		// Prevent flicker
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
	}

	public function makeMaterialPreview() {

		#if arm_editor
		var gizmo_vis = gizmo.visible;
		var grid_vis = grid.visible;
		gizmo.visible = false;
		grid.visible = false;
		#end

		var cube:MeshObject = cast iron.Scene.active.getChild("Cube");
		cube.visible = false;

		var sphere:MeshObject = cast iron.Scene.active.getChild("Sphere");
		sphere.visible = true;
		#if arm_editor
		sphere.materials[0] = htab.position == 0 ? selectedMaterial2.data : materials[0].data;
		#end
		currentObject = sphere;

		savedCamera.setFrom(iron.Scene.active.camera.transform.local);
		var m = new Mat4(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		iron.Scene.active.camera.transform.setMatrix(m);
		iron.Scene.active.camera.buildMatrix();

		dirty = 2;
		materialPreview = true;
		arm.App.resize();

		UINodes.inst.parseMeshPreviewMaterial();

		// TAA
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);

		sphere.visible = false;
		cube.visible = true;
		currentObject = cube;

		#if arm_editor
		gizmo.visible = gizmo_vis;
		grid.visible = grid_vis;
		#end

		dirty = 2;
		materialPreview = false;

		iron.Scene.active.camera.transform.setMatrix(savedCamera);
		iron.Scene.active.camera.buildMatrix();
		arm.App.resize();
		iron.Scene.active.camera.buildMatrix();

		UINodes.inst.parseMeshMaterial();

		// Prevent flicker
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
	}

	function selectMaterial(i:Int) {
		if (materials.length <= i) return;
		selectedMaterial = materials[i];

		// #if arm_editor
		// if (Std.is(selectedObject, iron.object.MeshObject)) {
		// 	cast(selectedObject, iron.object.MeshObject).materials[0] = selectedMaterial.data;
		// }
		// #end

		UINodes.inst.updateCanvasMap();
		UINodes.inst.parsePaintMaterial();

		// #if arm_editor
		// UINodes.inst.parseBrush();
		// #end

		hwnd.redraws = 2;
	}

	#if arm_editor
	function selectMaterial2(i:Int) {
		if (materials2.length <= i) return;
		selectedMaterial2 = materials2[i];

		#if arm_editor
		if (Std.is(selectedObject, iron.object.MeshObject)) {
			cast(selectedObject, iron.object.MeshObject).materials[0] = selectedMaterial2.data;
		}
		#end

		UINodes.inst.updateCanvasMap();
		UINodes.inst.parsePaintMaterial();

		hwnd.redraws = 2;
	}
	#end

	function selectBrush(i:Int) {
		if (brushes.length <= i) return;
		selectedBrush = brushes[i];
		UINodes.inst.updateCanvasBrushMap();
		UINodes.inst.parseBrush();
		hwnd.redraws = 2;
	}

	var isUndo = false;
	function updateUI() {

		messageTimer -= iron.system.Time.delta;

		paint = false;
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if (!show) return;
		if (!arm.App.uienabled) return;

		var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
		if (down && !kb.down("control")) {

			if (mouse.x <= iron.App.w()) {
				if (brushTime == 0 && undoEnabled) {
					pushUndo = true;
				}
				brushTime += iron.system.Time.delta;
				for (f in _onBrush) f();
			}

		}
		else if (brushTime > 0) {
			brushTime = 0;
		}

		if (kb.down("control") && kb.started("z") && undoEnabled) {
			// TODO: swap layers instead of images
			var tp = selectedLayer.texpaint;
			var tp_nor = selectedLayer.texpaint_nor;
			var tp_pack = selectedLayer.texpaint_pack;
			var tp_opt = selectedLayer.texpaint_opt;

			selectedLayer.set_texpaint(undoLayers[0].texpaint);
			selectedLayer.set_texpaint_nor(undoLayers[0].texpaint_nor);
			selectedLayer.set_texpaint_pack(undoLayers[0].texpaint_pack);
			selectedLayer.set_texpaint_opt(undoLayers[0].texpaint_opt);

			undoLayers[0].set_texpaint(tp);
			undoLayers[0].set_texpaint_nor(tp_nor);
			undoLayers[0].set_texpaint_pack(tp_pack);
			undoLayers[0].set_texpaint_opt(tp_opt);

			// Only main layer contains the depth
			iron.RenderPath.active.depthToRenderTarget.set("paintdb", isUndo ? selectedLayer.rt : undoLayers[0].rt);
			isUndo = !isUndo;

			dirty = 2;
		}

		#if arm_editor
		updateGizmo();
		#end
	}

	#if arm_editor
	function updateGizmo() {
		if (!gizmo.visible) return;

		if (selectedObject != null) {
			gizmo.transform.loc.setFrom(selectedObject.transform.loc);
			gizmo.transform.buildMatrix();
		}

		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if (mouse.x < App.w()) {
			if (kb.started("x") || kb.started("backspace")) {
				if (selectedObject != null) {
					selectedObject.remove();
					selectObject(iron.Scene.active.getChild("Scene"));
				}
			}
			if (kb.started("c") && selectedObject != null) {
				if (Std.is(selectedObject, iron.object.MeshObject)) {
					var mo = cast(selectedObject, iron.object.MeshObject);
					var object = iron.Scene.active.addMeshObject(mo.data, mo.materials, iron.Scene.active.getChild("Scene"));
					object.name = mo.name + '.1';

					object.transform.loc.setFrom(mo.transform.loc);
					object.transform.rot.setFrom(mo.transform.rot);
					object.transform.scale.setFrom(mo.transform.scale);

					var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.zAxis(), new iron.math.Vec4(), mouse.x, mouse.y, iron.Scene.active.camera);
					if (hit != null) {
						object.transform.loc.x = hit.x;
						object.transform.loc.y = hit.y;
						object.transform.setRotation(0, 0, Math.random() * 3.1516 * 2);
					}

					object.transform.buildMatrix();

					object.addTrait(new armory.trait.physics.RigidBody(0.0));

					selectObject(object);
				}
				else if (Std.is(selectedObject, iron.object.LightObject)) {
					var lo = cast(selectedObject, iron.object.LightObject);
					var object = iron.Scene.active.addLightObject(lo.data, iron.Scene.active.getChild("Scene"));
					object.name = lo.name + '.1';

					object.transform.loc.setFrom(lo.transform.loc);
					object.transform.rot.setFrom(lo.transform.rot);
					object.transform.scale.setFrom(lo.transform.scale);
					object.transform.buildMatrix();

					object.addTrait(new armory.trait.physics.RigidBody(0.0));

					selectObject(object);
				}
			}
			if (kb.started("m")) { // skip voxel
				selectedMaterial2.data.raw.skip_context = selectedMaterial2.data.raw.skip_context == '' ? 'voxel' : '';
			}
		}

		if (mouse.started("middle")) {
			var physics = armory.trait.physics.PhysicsWorld.active;
			var rb = physics.pickClosest(mouse.x, mouse.y);
			if (rb != null) selectObject(rb.object);
		}

		if (mouse.started("left") && selectedObject.name != "Scene") {
			gizmo.transform.buildMatrix();
			var trs = [gizmoX.transform, gizmoY.transform, gizmoZ.transform];
			var hit = iron.math.RayCaster.closestBoxIntersect(trs, mouse.x, mouse.y, iron.Scene.active.camera);
			if (hit != null) {
				if (hit.object == gizmoX) axisX = true;
				else if (hit.object == gizmoY) axisY = true;
				else if (hit.object == gizmoZ) axisZ = true;
				if (axisX || axisY || axisZ) axisStart = 0.0;
			}
		}
		else if (mouse.released("left")) {
			axisX = axisY = axisZ = false;
		}

		if (axisX || axisY || axisZ) {
			var t = selectedObject.transform;
			var v = new iron.math.Vec4();
			v.set(t.worldx(), t.worldy(), t.worldz());
			
			if (axisX) {
				var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.yAxis(), v, mouse.x, mouse.y, iron.Scene.active.camera);
				if (hit != null) {
					if (axisStart == 0) axisStart = hit.x - selectedObject.transform.loc.x;
					selectedObject.transform.loc.x = hit.x - axisStart;
					selectedObject.transform.buildMatrix();
					var rb = selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
				}
			}
			else if (axisY) {
				var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.xAxis(), v, mouse.x, mouse.y, iron.Scene.active.camera);
				if (hit != null) {
					if (axisStart == 0) axisStart = hit.y - selectedObject.transform.loc.y;
					selectedObject.transform.loc.y = hit.y - axisStart;
					selectedObject.transform.buildMatrix();
					var rb = selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
				}
			}
			else if (axisZ) {
				var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.xAxis(), v, mouse.x, mouse.y, iron.Scene.active.camera);
				if (hit != null) {
					if (axisStart == 0) axisStart = hit.z - selectedObject.transform.loc.z;
					selectedObject.transform.loc.z = hit.z - axisStart;
					selectedObject.transform.buildMatrix();
					var rb = selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
				}
			}
		}

		arm.FlyCamera.inst.enabled = !(axisX || axisY || axisZ) && mouse.x < App.w();
	}
	#end

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

		g.begin();
		iron.App.removeRender(initLayers);
	}

	function initHeightLayer(g:kha.graphics4.Graphics) {
		g.end();

		layers[0].texpaint_opt.g4.begin();
		layers[0].texpaint_opt.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Opac, emis, height
		layers[0].texpaint_opt.g4.end();

		g.begin();
		iron.App.removeRender(initHeightLayer);
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

		if (layers[i].texpaint_opt != null) {
			layers[i].texpaint_opt.g4.begin();
			layers[i].texpaint_opt.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Opac, emis, height
			layers[i].texpaint_opt.g4.end();
		}

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
			if (rttexpaint_opt != null) rttexpaint_opt.image = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);

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

			if (rttexpaint_opt != null) {
				rttexpaint_opt.image.g2.begin();
				rttexpaint_opt.image.g2.drawScaledImage(l.texpaint_opt, 0, 0, res, res);
				rttexpaint_opt.image.g2.end();
			}
			g.begin();

			l.unload();

			l.texpaint = rts.get("texpaint" + l.id).image;
			l.texpaint_nor = rts.get("texpaint_nor" + l.id).image;
			l.texpaint_pack = rts.get("texpaint_pack" + l.id).image;
			if (rttexpaint_opt != null) l.texpaint_opt = rts.get("texpaint_opt" + l.id).image;
		}

		dirty = 2;
		iron.App.removeRender(resizeLayers);
	}

	function deleteSelectedLayer() {
		selectedLayer.unload();
		layers.remove(selectedLayer);
		selectedLayer = layers[0];
		UINodes.inst.parseMeshMaterial();
		UINodes.inst.parsePaintMaterial();
		dirty = 2;
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

		if (l0.texpaint_opt != null) {
			l0.texpaint_opt.g2.begin(false);
			l0.texpaint_opt.g2.drawImage(l1.texpaint_opt, 0, 0);
			l0.texpaint_opt.g2.end();
		}

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

	#if arm_editor
	public var cameraType = 1;
	#else
	public var cameraType = 0;
	#end
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
		if (UINodes.inst.show && UINodes.inst.canvasType == 0) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 0; }
		arm.App.resize();
	}

	function showBrushNodes() {
		UIView2D.inst.show = false;
		if (UINodes.inst.show && UINodes.inst.canvasType == 1) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 1; }
		arm.App.resize();
	}

	function showLogicNodes() {
		UIView2D.inst.show = false;
		if (UINodes.inst.show && UINodes.inst.canvasType == 2) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 2; }
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

	function selectObject(o:iron.object.Object) {
		selectedObject = o;

		#if arm_editor
		if (Std.is(o, iron.object.MeshObject)) {
			for (i in 0...materials2.length) {
				if (materials2[i].data == cast(o, iron.object.MeshObject).materials[0]) {
					// selectMaterial(i); // loop
					selectedMaterial2 = materials2[i];
					hwnd.redraws = 2;
					break;
				}
			}
		}
		#end
	}

	function renderUI(g:kha.graphics2.Graphics) {
		if (!show) return;

		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();

		var envThumb = bundled.get('env_thumb.jpg');
		var cursorImg = bundled.get('cursor.png');
		var mouse = iron.system.Input.getMouse();
		g.color = 0xffffffff;

		// Brush
		if (arm.App.uienabled) {
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
			var pen = iron.system.Input.getPen();
			if (pen.down()) {
				mx = pen.x + iron.App.x();
				my = pen.y + iron.App.y();
			}

			if (brushPaint == 2) { // Sticker
				// var psize = Std.int(stickerImage.width * (brushRadius * brushNodesRadius));
				var psize = Std.int(256 * (brushRadius * brushNodesRadius));
				g.drawScaledImage(stickerImage, mx - psize / 2, my - psize / 2 + psize, psize, -psize);
			}
			else {
				var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));
				g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);

				if (mirrorX) {
					var cx = iron.App.x() + iron.App.w() / 2;
					var nx = cx + (cx - mx);
					// Separator line
					g.color = 0x66ffffff;
					g.fillRect(cx - 1, 0, 2, iron.App.h());
					// Cursor
					g.drawScaledImage(cursorImg, nx - psize / 2, my - psize / 2, psize, psize);
					g.color = 0xffffffff;
				}
			}
		}

		g.end();
		ui.begin(g);
		// ui.begin(rt.g2); ////

		// if (ui.window(Id.handle(), 0, 0, toolbarw, kha.System.windowHeight())) {
			// if (ui.tab(Id.handle(), "Tools")) {
			// }
		// }

		// if (ui.window(Id.handle(), toolbarw, 0, arm.App.realw() - windowW - toolbarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			// ui.tab(Id.handle(), "3D View");
		// }
		
		var wx = apconfig.ui_layout == 0 ? arm.App.realw() - windowW : 0;
		if (ui.window(hwnd, wx, 0, windowW, arm.App.realh())) {

			#if arm_editor

			gizmo.visible = false;
			grid.visible = false;

			if (ui.tab(htab, "Scene")) {
				gizmo.visible = true;
				grid.visible = true;
				if (ui.panel(Id.handle({selected: true}), "Outliner", 1)) {
					ui.indent();
					
					var i = 0;
					function drawList(h:zui.Zui.Handle, o:iron.object.Object) {
						if (o.name.charAt(0) == '.') return; // Hidden
						var b = false;
						if (selectedObject == o) {
							ui.g.color = 0xff205d9c;
							ui.g.fillRect(0, ui._y, ui._windowW, ui.t.ELEMENT_H);
							ui.g.color = 0xffffffff;
						}
						if (o.children.length > 0) {
							b = ui.panel(h.nest(i, {selected: true}), o.name, 0, true);
						}
						else {
							ui._x += 18; // Sign offset
							ui.text(o.name);
							ui._x -= 18;
						}
						if (ui.isReleased) {
							selectObject(o);
						}
						i++;
						if (b) {
							for (c in o.children) {
								ui.indent();
								drawList(h, c);
								ui.unindent();
							}
						}
					}
					for (c in iron.Scene.active.root.children) {
						drawList(Id.handle(), c);
					}

					ui.unindent();
				}
				
				if (selectedObject == null) selectedType = "";

				if (ui.panel(Id.handle({selected: true}), 'Properties $selectedType', 1)) {
					ui.indent();
					
					if (selectedObject != null) {

						var h = Id.handle();
						h.selected = selectedObject.visible;
						selectedObject.visible = ui.check(h, "Visible");

						var loc = selectedObject.transform.loc;
						var scale = selectedObject.transform.scale;
						var rot = selectedObject.transform.rot.getEuler();
						rot.mult(180 / 3.141592);
						var f = 0.0;

						ui.row(row4);
						ui.text("Location");

						h = Id.handle();
						h.text = Math.roundfp(loc.x) + "";
						f = Std.parseFloat(ui.textInput(h, "X"));
						if (ui.changed) loc.x = f;

						h = Id.handle();
						h.text = Math.roundfp(loc.y) + "";
						f = Std.parseFloat(ui.textInput(h, "Y"));
						if (ui.changed) loc.y = f;

						h = Id.handle();
						h.text = Math.roundfp(loc.z) + "";
						f = Std.parseFloat(ui.textInput(h, "Z"));
						if (ui.changed) loc.z = f;

						ui.row(row4);
						ui.text("Rotation");
						
						h = Id.handle();
						h.text = Math.roundfp(rot.x) + "";
						f = Std.parseFloat(ui.textInput(h, "X"));
						var changed = false;
						if (ui.changed) { changed = true; rot.x = f; }

						h = Id.handle();
						h.text = Math.roundfp(rot.y) + "";
						f = Std.parseFloat(ui.textInput(h, "Y"));
						if (ui.changed) { changed = true; rot.y = f; }

						h = Id.handle();
						h.text = Math.roundfp(rot.z) + "";
						f = Std.parseFloat(ui.textInput(h, "Z"));
						if (ui.changed) { changed = true; rot.z = f; }

						if (changed && selectedObject.name != "Scene") {
							rot.mult(3.141592 / 180);
							selectedObject.transform.rot.fromEuler(rot.x, rot.y, rot.z);
							selectedObject.transform.buildMatrix();
							var rb = selectedObject.getTrait(armory.trait.physics.RigidBody);
							if (rb != null) rb.syncTransform();
						}

						ui.row(row4);
						ui.text("Scale");
						
						h = Id.handle();
						h.text = Math.roundfp(scale.x) + "";
						f = Std.parseFloat(ui.textInput(h, "X"));
						if (ui.changed) scale.x = f;

						h = Id.handle();
						h.text = Math.roundfp(scale.y) + "";
						f = Std.parseFloat(ui.textInput(h, "Y"));
						if (ui.changed) scale.y = f;

						h = Id.handle();
						h.text = Math.roundfp(scale.z) + "";
						f = Std.parseFloat(ui.textInput(h, "Z"));
						if (ui.changed) scale.z = f;

						selectedObject.transform.dirty = true;

						if (selectedObject.name == "Scene") {
							selectedType = "(Scene)";
							// ui.image(envThumb);
							var p = iron.Scene.active.world.getGlobalProbe();
							ui.row([1/2, 1/2]);
							var envType = ui.combo(Id.handle({position: 0}), ["Outdoor"], "Map");
							p.raw.strength = ui.slider(Id.handle({value: p.raw.strength}), "Strength", 0.0, 5.0, true);
						}
						else if (Std.is(selectedObject, iron.object.LightObject)) {
							selectedType = "(Lamp)";
							var light = cast(selectedObject, iron.object.LightObject);
							light.data.raw.strength = ui.slider(Id.handle({value: light.data.raw.strength / 10}), "Strength", 0.0, 5.0, true) * 10;
						}
						else if (Std.is(selectedObject, iron.object.CameraObject)) {
							selectedType = "(Camera)";
							var scene = iron.Scene.active;
							var cam = scene.cameras[0];
							var fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
							cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
							if (ui.changed) {
								cam.buildProjection();
							}
						}
						else {
							selectedType = "(Object)";
							
						}
					}

					if (ui.button("LNodes")) showLogicNodes();

					ui.unindent();
				}
				if (ui.panel(Id.handle({selected: true}), "Material", 1)) {

					var img2 = bundled.get("mat_slot.jpg");

					for (row in 0...Std.int(Math.ceil(materials2.length / 5))) { 
						ui.row([1/5,1/5,1/5,1/5,1/5]);

						if (row > 0) ui._y += 6;

						for (j in 0...5) {
							ui.imageInvertY = true; // Material preview
							
							var i = j + row * 5;
							var im = i >= materials2.length ? img2 : materials2[i].image;

							if (im != img2 && selectedMaterial2 == materials2[i]) {
								ui.fill(1, -2, im.width + 3, im.height + 3, 0xff205d9c);
							}

							if (ui.image(im) == State.Started && im != img2) {
								if (selectedMaterial2 != materials2[i]) {
									selectMaterial2(i);
								}
								if (iron.system.Time.time() - selectTime < 0.3) {
									showMaterialNodes();
								}
								selectTime = iron.system.Time.time();
							}

							ui.imageInvertY = false; // Material preview
						}
					}

					ui.row([1/2,1/2]);
					if (ui.button("New")) {

						iron.data.Data.cachedMaterials.remove("SceneMaterial2");
						iron.data.Data.cachedShaders.remove("Material2_data");
						iron.data.Data.cachedSceneRaws.remove("Material2_data");
						iron.data.Data.cachedBlobs.remove("Material2_data.arm");
						iron.data.Data.getMaterial("Scene", "Material2", function(md:iron.data.MaterialData) {
							md.name = "Material2." + materials2.length;
							var m = new MaterialSlot(md);
							materials2.push(m);
							selectMaterial2(materials2.length - 1);
							UITrait.inst.makeMaterialPreview();
						});
					}
					if (ui.button("Nodes")) {
						showMaterialNodes();
					}
				}
			}
			else {
				selectedObject = paintObject;
			}
			#end // arm_editor


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

				if (ui.panel(Id.handle({selected: true}), tool, 1)) {
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
						brushPaint = ui.combo(paintHandle, ["UV", "Project", "Sticker"], "Paint");
						if (paintHandle.changed) {
							UINodes.inst.parsePaintMaterial();

							if (brushPaint == 2) { // Sticker
								UITrait.inst.makeStickerPreview();
							}
						}
						brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 1.0, true);
						ui.row([1/2, 1/2]);
						brushRadius = ui.slider(Id.handle({value: brushRadius}), "Radius", 0.0, 2.0, true);
						brushOpacity = ui.slider(Id.handle({value: brushOpacity}), "Opacity", 0.0, 1.0, true);
						ui.row([1/2, 1/2]);
						brushScale = ui.slider(Id.handle({value: brushScale}), "UV Scale", 0.0, 2.0, true);
						brushStrength = ui.slider(Id.handle({value: brushStrength}), "Strength", 0.0, 1.0, true);
					}
				}

				if (ui.panel(Id.handle({selected: true}), "Paint Channels", 1)) {
					ui.row([1/3,1/3,1/3]);

					var baseHandle = Id.handle({selected: paintBase});
					paintBase = ui.check(baseHandle, "Base");
					if (baseHandle.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}

					var norHandle = Id.handle({selected: paintNor});
					paintNor = ui.check(norHandle, "Normal");
					if (norHandle.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}

					var heightHandle = Id.handle({selected: paintHeight});
					paintHeight = ui.check(heightHandle, "Height");
					if (heightHandle.changed) {
						for (l in layers) l.make_texpaint_opt();
						for (l in undoLayers) l.make_texpaint_opt();
						iron.App.notifyOnRender(initHeightLayer);
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}

					ui.row([1/3,1/3,1/3]);

					var occHandle = Id.handle({selected: paintOcc});
					paintOcc = ui.check(occHandle, "Occlusion");
					if (occHandle.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}

					var roughHandle = Id.handle({selected: paintRough});
					paintRough = ui.check(roughHandle, "Roughness");
					if (roughHandle.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}

					var metHandle = Id.handle({selected: paintMet});
					paintMet = ui.check(metHandle, "Metallic");
					if (metHandle.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}

					ui.row([1/2,1/2]);
					paintVisible = ui.check(Id.handle({selected: paintVisible}), "Visible Only");
					mirrorX = ui.check(Id.handle({selected: mirrorX}), "Mirror");
					if (ui.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}
				}

				if (ui.panel(Id.handle({selected: true}), "Material", 1)) {

					var img2 = bundled.get("mat_slot.jpg");

					for (row in 0...Std.int(Math.ceil(materials.length / 5))) { 
						ui.row([1/5,1/5,1/5,1/5,1/5]);

						if (row > 0) ui._y += 6;

						for (j in 0...5) {
							ui.imageInvertY = true; // Material preview
							
							var i = j + row * 5;
							var im = i >= materials.length ? img2 : materials[i].image;

							if (im != img2 && selectedMaterial == materials[i]) {
								ui.fill(1, -2, im.width + 3, im.height + 3, 0xff205d9c);
							}

							if (ui.image(im) == State.Started && im != img2) {
								if (selectedMaterial != materials[i]) {
									selectMaterial(i);
								}
								if (iron.system.Time.time() - selectTime < 0.3) {
									showMaterialNodes();
								}
								selectTime = iron.system.Time.time();
							}

							ui.imageInvertY = false; // Material preview
						}
					}

					ui.row([1/2,1/2]);
					if (ui.button("New")) {
						selectedMaterial = new MaterialSlot();
						materials.push(selectedMaterial);
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
						UITrait.inst.makeMaterialPreview();
					}
					if (ui.button("Nodes")) {
						showMaterialNodes();
					}
				}

				if (ui.panel(Id.handle({selected: true}), "Layers", 1)) {

					function drawList(h:zui.Zui.Handle, l:LayerSlot, i:Int) {
						if (selectedLayer == l) {
							ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, 0xff205d9c);
						}
						if (i > 0) ui.row([1/10, 5/10, 2/10, 2/10]);
						else ui.row([1/10, 9/10]);
						l.visible = ui.check(Id.handle().nest(l.id, {selected: l.visible}), "");
						if (ui.changed) {
							UINodes.inst.parseMeshMaterial();
							dirty = 2;
						}
						ui.text("Layer " + (i + 1));
						if (ui.isReleased) {
							selectedLayer = l;
							UINodes.inst.parsePaintMaterial(); // Different blending for layer on top
							dirty = 2;
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
						if (i >= layers.length) break; // Layer was deleted
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
								dirty = 2;
								iron.App.notifyOnRender(clearLastLayer);
							}
						}
					}
					if (ui.button("2D View")) show2DView();
				}

				if (ui.panel(Id.handle({selected: false}), "Camera", 1)) {
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
								dirty = 2;
								break;
							}
						}
					}
				}

				if (ui.panel(Id.handle({selected: false}), "Lighting", 1)) {
					ui.image(envThumb);
					var p = iron.Scene.active.world.getGlobalProbe();
					ui.row([1/2, 1/2]);
					var envType = ui.combo(Id.handle({position: 0}), ["Default"], "Map");
					p.raw.strength = ui.slider(Id.handle({value: p.raw.strength}), "Environment", 0.0, 5.0, true);
					
					ui.row([1/2, 1/2]);
					var showType = ui.combo(Id.handle({position: 0}), ["Render", "Base Color", "Normal", "Occlusion", "Roughness", "Metallic"], "Show");
					if (iron.Scene.active.lights.length > 0) {
						var light = iron.Scene.active.lights[0];
						light.data.raw.strength = ui.slider(Id.handle({value: light.data.raw.strength / 10}), "Light", 0.0, 5.0, true) * 10;
					}

					displaceStrength = ui.slider(Id.handle({value: displaceStrength}), "Displace", 0.0, 2.0, true);
					if (ui.changed) {
						UINodes.inst.parseMeshMaterial();
					}
				}

				// Draw plugins
				for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
			}
			if (ui.tab(htab, "Library")) {

				if (ui.panel(Id.handle({selected: true}), "Import", 1)) {
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
							importAsset(path);
						}
					}
				}

				if (ui.panel(Id.handle({selected: true}), "Assets", 1)) {
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
			}

			if (ui.tab(htab, "File")) {

				if (ui.panel(Id.handle({selected: true}), "Project", 1)) {
					ui.row([1/3,1/3,1/3]);
					ui.button("Open..");
					ui.button("Save..");
					ui.button("Save As..");
				}

				if (ui.panel(Id.handle({selected: true}), "Export", 1)) {

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

							if (isHeight && selectedLayer.texpaint_opt != null) {
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
								Krom.fileSaveBytes(path + "/tex_height.png", bo.getBytes().getData());
								#end
							}

							// if (isOpac) Krom.fileSaveBytes(path + "/tex_opac.png", bo.getBytes().getData());
							// if (isEmis) Krom.fileSaveBytes(path + "/tex_emis.png", bo.getBytes().getData());
							// if (isSubs) Krom.fileSaveBytes(path + "/tex_subs.png", bo.getBytes().getData());
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
					isHeight = ui.check(Id.handle({selected: isHeight}), "Height");
				}
			}

			if (ui.tab(htab, "Preferences")) {
				if (ui.panel(Id.handle({selected: true}), "Interface", 1)) {
					var hscale = Id.handle({value: apconfig.window_scale});
					ui.slider(hscale, "UI Scale", 0.5, 4.0, true);
					if (ui.changed && !iron.system.Input.getMouse().down()) {
						apconfig.window_scale = hscale.value;
						ui.setScale(hscale.value);
						windowW = Std.int(defaultWindowW * apconfig.window_scale);
						arm.App.resize();
					}
					apconfig.ui_layout = ui.combo(Id.handle({position: apconfig.ui_layout}), ["Right", "Left"], "UI Layout", true);
				}

				if (ui.panel(Id.handle({selected: true}), "Usage", 1)) {
					penPressure = ui.check(Id.handle({selected: penPressure}), "Pen Pressure");
					undoEnabled = ui.check(Id.handle({selected: true}), "Undo");
				}

				#if arm_debug
				iron.Scene.active.sceneParent.getTrait(armory.trait.internal.DebugConsole).visible = ui.check(Id.handle({selected: false}), "Debug Console");
				#end

				var hssgi = Id.handle({selected: apconfig.rp_ssgi});
				var hssr = Id.handle({selected: apconfig.rp_ssr});
				var hbloom = Id.handle({selected: apconfig.rp_bloom});
				var hshadowmap = Id.handle({position: getShadowQuality(apconfig.rp_shadowmap)});
				if (ui.panel(Id.handle({selected: true}), "Viewport", 1)) {
					apconfig.window_vsync = ui.check(Id.handle({selected: apconfig.window_vsync}), "VSync");
					drawWorld = ui.check(Id.handle({selected: drawWorld}), "Envmap");
					if (ui.changed) {
						dirty = 2;
					}
					if (!drawWorld) {
						var hwheel = Id.handle();
						worldColor = Ext.colorWheel(ui, hwheel);
					}
					ui.check(hssgi, "SSAO");
					ui.check(hssr, "SSR");
					ui.check(hbloom, "Bloom");
					ui.combo(hshadowmap, ["High", "Medium", "Low"], "Shadows", true);
				}

				if (ui.button("Apply and Save")) {
					apconfig.rp_ssgi = hssgi.selected;
					apconfig.rp_ssr = hssr.selected;
					apconfig.rp_bloom = hbloom.selected;
					apconfig.rp_shadowmap = getShadowMapSize(hshadowmap.position);
					armory.data.Config.save();
					armory.renderpath.RenderPathCreator.applyConfig();
				}

				if (ui.panel(Id.handle({selected: false}), "About", 1)) {
					ui.text("v0.5");
					// ui.text(Macro.buildSha());
					// ui.text(Macro.buildDate());
					ui.text("armorpaint.org");
				}
			}
		}
		ui.end();
		g.begin(false);

		if (arm.App.dragAsset != null) {
			dirty = 2;
			var mouse = iron.system.Input.getMouse();
			var ratio = 128 / getImage(arm.App.dragAsset).width;
			var h = getImage(arm.App.dragAsset).height * ratio;
			g.drawScaledImage(getImage(arm.App.dragAsset), mouse.x, mouse.y, 128, h);
		}
	}

	inline function getShadowQuality(i:Int):Int {
		// 0 - High, 1 - Medium, 2 - Low
		return i == 4096 ? 0 : i == 2048 ? 1 : 2;
	}

	inline function getShadowMapSize(i:Int):Int {
		return i == 0 ? 4096 : i == 1 ? 2048 : 1024;
	}

	public function getImage(asset:TAsset):kha.Image {
		return Canvas.assetMap.get(asset.id);
	}

	public static function checkMeshFormat(path:String):Bool {
		var p = path.toLowerCase();
		if (!StringTools.endsWith(p, ".obj") &&
			!StringTools.endsWith(p, ".gltf") &&
			// !StringTools.endsWith(p, ".blend") &&
			!StringTools.endsWith(p, ".fbx")) {
			return false;
		}
		return true;
	}

	public function importMesh(path:String) {
		if (!checkMeshFormat(path)) {
			showMessage("Error: Unknown mesh format");
			return;
		}
		var p = path.toLowerCase();
		if (StringTools.endsWith(p, ".obj")) importObj(path);
		else if (StringTools.endsWith(p, ".gltf")) importGltf(path);
		else if (StringTools.endsWith(p, ".fbx")) importFbx(path);
	}

	function importObj(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.obj.ObjParser(b);
			makeMesh(obj, path);
		});
	}

	function importGltf(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.gltf.GltfParser(b);
			makeMesh(obj, path);
		});
	}

	function importFbx(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.fbx.FbxParser(b);
			makeMesh(obj, path);
		});
	}

	function importBlend(path:String) {
		// iron.data.Data.getBlob(path, function(b:kha.Blob) {
			// var obj = new blend.Loader(b);
			// if (obj.texa == null) {
				// showMessage("Error: Invalid mesh - no UVs found");
				// return;
			// }
			// makeMesh(obj, path);
		// });
	}

	function makeMesh(mesh:Dynamic, path:String) {
		
		#if arm_editor
		var raw:TMeshData = {
			name: "Mesh",
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos" },
				{ values: mesh.nora, attrib: "nor" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			]
		};
		if (mesh.texa != null) raw.vertex_arrays.push({ values: mesh.texa, attrib: "tex" });
		#else
		if (mesh.texa == null) {
			showMessage("Error: Invalid mesh - no UVs found");
			return;
		}
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
		#end

		new MeshData(raw, function(md:MeshData) {
			
			#if arm_editor // Append
			if (htab.position == 0) {
				var mats = new haxe.ds.Vector(1);
				mats[0] = selectedMaterial2.data;
				var object = iron.Scene.active.addMeshObject(md, mats, iron.Scene.active.getChild("Scene"));
				path = StringTools.replace(path, "\\", "/");
				var ar = path.split("/");
				var s = ar[ar.length - 1];
				object.name = s.substring(0, s.length - 4);

				// md.geom.calculateAABB();
				// var aabb = md.geom.aabb;
				// var dim = new TFloat32Array(3);
				// dim[0] = aabb.x;
				// dim[1] = aabb.y;
				// dim[2] = aabb.z;
				// object.raw.dimensions = dim;
				object.addTrait(new armory.trait.physics.RigidBody(0.0));
				selectObject(object);
			}
			else {
			#end
			{ // Replace
				currentObject.data.delete();
				iron.App.notifyOnRender(initLayers);
				if (paintHeight) iron.App.notifyOnRender(initHeightLayer);
				
				currentObject.setData(md);
				
				// Scale to bounds
				md.geom.calculateAABB();
				var r = Math.sqrt(md.geom.aabb.x * md.geom.aabb.x + md.geom.aabb.y * md.geom.aabb.y + md.geom.aabb.z * md.geom.aabb.z);
				currentObject.transform.scale.set(3 / r, 3 / r, 3 / r);
				currentObject.transform.buildMatrix();

				// Face camera
				// currentObject.transform.setRotation(Math.PI / 2, 0, 0);

				paintObject = currentObject;
			}

			#if arm_editor
			}
			#end

			first = 0; // Needs 2 redraws to clear textures after import
			dirty = 2;
		});
	}
}

typedef TAPConfig = {
	@:optional var debug_console:Null<Bool>;
	@:optional var window_mode:Null<Int>; // window, fullscreen
	@:optional var window_w:Null<Int>;
	@:optional var window_h:Null<Int>;
	@:optional var window_resizable:Null<Bool>;
	@:optional var window_maximizable:Null<Bool>;
	@:optional var window_minimizable:Null<Bool>;
	@:optional var window_vsync:Null<Bool>;
	@:optional var window_msaa:Null<Int>;
	@:optional var window_scale:Null<Float>;
	// @:optional var rp_supersample:Null<Float>;
	@:optional var rp_shadowmap:Null<Int>; // size
	@:optional var rp_ssgi:Null<Bool>;
	@:optional var rp_ssr:Null<Bool>;
	@:optional var rp_bloom:Null<Bool>;
	@:optional var rp_motionblur:Null<Bool>;
	@:optional var rp_gi:Null<Bool>;
	// Ext
	// @:optional var version:Null<Float>;
	@:optional var plugins:Array<String>;
	@:optional var ui_layout:Null<Int>;
	
}
