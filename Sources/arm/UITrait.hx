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
import arm.ProjectFormat;
import arm.ProjectFormat.TAPConfig;

@:access(zui.Zui)
@:access(iron.data.Data)
class UITrait extends iron.Trait {

	var version = "0.6";

	public var project:TProjectFormat;
	var projectPath = "";

	public var assets:Array<TAsset> = [];
	public var assetNames:Array<String> = [];
	public var assetId = 0;

	public static var inst:UITrait;
	public static var defaultWindowW = 280;

	public static var penPressure = true;
	public var undoI = 0; // Undo layer
	public var undos = 0; // Undos available
	public var redos = 0; // Redos available
	public var pushUndo = false; // Store undo on next paint

	public var isScrolling = false;
	public var colorIdPicked = false;
	public var show = true;
	public var materialPreview = false; // Drawing material previews
	var savedCamera = Mat4.identity();
	var message = "";
	var messageTimer = 0.0;

	var savedEnvmap:kha.Image = null;
	var emptyEnvmap:kha.Image = null;
	var previewEnvmap:kha.Image = null;
	var showEnvmap = false;
	public var culling = true;

	public var ddirty = 0;
	public var pdirty = 0;
	public var rdirty = 0;

	public var bundled:Map<String, kha.Image> = new Map();
	var ui:Zui;
	public var windowW = 280; // Panel width
	public var toolbarw = 50;
	var systemId = "";

	var colorIdHandle = Id.handle();

	var formatType = 0;
	var formatQuality = 80.0;
	var outputType = 0;
	var isBase = true;
	var isBaseSpace = 0;
	var isOpac = true;
	var isOpacSpace = 0;
	var isOcc = true;
	var isOccSpace = 0;
	var isRough = true;
	var isRoughSpace = 0;
	var isMet = true;
	var isMetSpace = 0;
	var isNor = true;
	var isNorSpace = 0;
	var isHeight = true;
	var isHeightSpace = 0;
	public var hwnd = Id.handle();
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
	public var viewportMode = 0;
	public var instantMat = true;
	var hscaleWasChanged = false;
	
	var _onBrush:Array<Int->Void> = [];

	public var paintVec = new iron.math.Vec4();
	public var lastPaintX = 0.0;
	public var lastPaintY = 0.0;
	public var painted = 0;
	public var brushTime = 0.0;

	public var selectedObject:iron.object.Object;
	public var paintObject:iron.object.MeshObject;
	public var paintObjects:Array<iron.object.MeshObject> = null;
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

	public var bakeStrength = 1.0;
	public var bakeRadius = 1.0;
	public var bakeOffset = 1.0;
	
	public var paintVisible = true;
	public var mirrorX = false;
	public var showGrid = false;
	public var autoFillHandle = new Zui.Handle({selected: false});
	public var resHandle = new Zui.Handle({position: 1}); // 2048
	var objectsHandle = new Zui.Handle({selected: false});
	var maskHandle = new Zui.Handle({position: 0});
	var mergedObject:MeshObject = null; // For object mask
	var newConfirm = false;
	var newObject = 0;
	var newObjectNames = ["Cube", "Plane", "Sphere", "Cylinder"];

	var sub = 0;
	var vec2 = new iron.math.Vec4();

	public var lastPaintVecX = -1.0;
	public var lastPaintVecY = -1.0;
	var frame = 0;

	public var C:TAPConfig;

	var lastBrushType = -1;

	#if arm_editor
	public var cameraControls = 2;
	#else
	public var cameraControls = 0;
	#end
	public var cameraType = 0;
	var originalShadowBias = 0.0;
	var camHandle = new Zui.Handle({position: 0});
	var fovHandle:Zui.Handle = null;
	var undoHandle:Zui.Handle = null;
	var hssgi:Zui.Handle = null;
	var hssr:Zui.Handle = null;
	var hbloom:Zui.Handle = null;
	var hshadowmap:Zui.Handle = null;
	var hsupersample:Zui.Handle = null;
	var textureExport = false;
	var textureExportPath = "";
	var projectExport = false;

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

	public function notifyOnBrush(f:Int->Void) {
		_onBrush.push(f);
	}

	public function dirty():Bool {
		return paintDirty() || depthDirty() || rdirty > 0;
	}

	public function depthDirty():Bool {
		return ddirty > 0;
	}

	public function paintDirty():Bool {
		#if arm_editor
		if (htab.position == 0) return false;
		#end
		return pdirty > 0;
	}

	function linkFloat(object:Object, mat:MaterialData, link:String):Null<kha.FastFloat> {

		if (link == '_brushRadius') {
			var r = (brushRadius * brushNodesRadius) / 15.0;
			var pen = iron.system.Input.getPen();
			if (penPressure && pen.down()) r *= pen.pressure;
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

	function linkVec2(object:Object, mat:MaterialData, link:String):iron.math.Vec4 {

		if (link == '_sub') {
			var seps = brushBias * 0.0004 * getTextureResBias();
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
			if (UITrait.inst.assets.length == 0) return bundled.get("empty.jpg");
			else return UITrait.inst.getImage(UITrait.inst.assets[colorIdHandle.position]);
		}
		return null;
	}

	public function new() {
		super();

		inst = this;
		systemId = kha.System.systemId;

		// Init config
		C = cast armory.data.Config.raw;
		if (C.ui_layout == null) C.ui_layout = 0;
		if (C.undo_steps == null) C.undo_steps = 4; // Max steps to keep

		windowW = Std.int(defaultWindowW * C.window_scale);

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
			for (i in 0...C.undo_steps) undoLayers.push(new LayerSlot("_undo" + undoLayers.length));
		}

		if (savedEnvmap == null) {
			savedEnvmap = iron.Scene.active.world.envmap;
		}
		if (emptyEnvmap == null) {
			var b = haxe.io.Bytes.alloc(4);
			b.set(0, 5);
			b.set(1, 5);
			b.set(2, 5);
			b.set(3, 255);
			emptyEnvmap = kha.Image.fromBytes(b, 1, 1);
		}
		if (previewEnvmap == null) {
			var b = haxe.io.Bytes.alloc(4);
			b.set(0, 0);
			b.set(1, 0);
			b.set(2, 0);
			b.set(3, 255);
			previewEnvmap = kha.Image.fromBytes(b, 1, 1);
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

		var scale = C.window_scale;
		ui = new Zui( { theme: arm.App.theme, font: arm.App.font, scaleFactor: scale, color_wheel: arm.App.color_wheel } );
		loadBundled(['cursor.png', 'empty.jpg', 'brush_draw.png', 'brush_erase.png', 'brush_fill.png', 'brush_bake.png', 'brush_colorid.png'], done);
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

	public function importFile(path:String, dropX = -1.0, dropY = -1.0) {
		var p = path.toLowerCase();
		// Mesh
		if (StringTools.endsWith(p, ".obj") ||
			StringTools.endsWith(p, ".fbx") ||
			StringTools.endsWith(p, ".blend") ||
			StringTools.endsWith(p, ".gltf")) {
			UITrait.inst.importMesh(path);
		}
		// Image
		else if (StringTools.endsWith(p, ".jpg") ||
				 StringTools.endsWith(p, ".png") ||
				 StringTools.endsWith(p, ".tga") ||
				 StringTools.endsWith(p, ".hdr")) {
			UITrait.inst.importAsset(path);
			// Place image node
			if (UINodes.inst.show && dropX > UINodes.inst.wx && dropX < UINodes.inst.wx + UINodes.inst.ww) {
				UINodes.inst.acceptDrag(UITrait.inst.assets.length - 1);
				UINodes.inst.nodes.nodeDrag = null;
				UINodes.inst.hwnd.redraws = 2;
			}
		}
		// Project
		else if (StringTools.endsWith(p, ".arm")) {
			UITrait.inst.importProject(path);
		}
		// Folder
		else if (p.indexOf(".") == -1) {
			#if kha_krom
			var systemId = kha.System.systemId;
			var cmd = systemId == "Windows" ? "dir /b " : "ls ";
			var sep = systemId == "Windows" ? "\\" : "/";
			var save = systemId == "Linux" ? "/tmp" : Krom.savePath();
			save += sep + "dir.txt";
			Krom.sysCommand(cmd + '"' + path + '"' + ' > ' + '"' + save + '"');
			var str = haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString();
			var files = str.split("\n");
			var mapbase = "";
			var mapnor = "";
			var mapocc = "";
			var maprough = "";
			var mapmet = "";
			var mapheight = "";
			// Import maps
			for (f in files) {
				if (f.length == 0) continue;
				f = StringTools.rtrim(f);
				var known = 
					StringTools.endsWith(f, ".jpg") ||
					StringTools.endsWith(f, ".png") ||
					StringTools.endsWith(f, ".tga") ||
					StringTools.endsWith(f, ".hdr");
				if (!known) continue;
				
				f = path + sep + f;
				if (systemId == "Windows") f = StringTools.replace(f, "/", "\\");
				
				var base = f.substr(0, f.lastIndexOf(".")).toLowerCase();
				var valid = false;
				if (mapbase == "" && (StringTools.endsWith(base, "_albedo") ||
									  StringTools.endsWith(base, "_alb") ||
									  StringTools.endsWith(base, "_basecol") ||
									  StringTools.endsWith(base, "_basecolor") ||
									  StringTools.endsWith(base, "_diffuse") ||
									  StringTools.endsWith(base, "_base") ||
									  StringTools.endsWith(base, "_bc") ||
									  StringTools.endsWith(base, "_d") ||
									  StringTools.endsWith(base, "_col"))) {
					mapbase = f;
					valid = true;
				}
				if (mapnor == "" && (StringTools.endsWith(base, "_normal") ||
									 StringTools.endsWith(base, "_nor") ||
									 StringTools.endsWith(base, "_n") ||
									 StringTools.endsWith(base, "_nrm"))) {
					mapnor = f;
					valid = true;
				}
				if (mapocc == "" && (StringTools.endsWith(base, "_ao") ||
									 StringTools.endsWith(base, "_occlusion") ||
									 StringTools.endsWith(base, "_o") ||
									 StringTools.endsWith(base, "_occ"))) {
					mapocc = f;
					valid = true;
				}
				if (maprough == "" && (StringTools.endsWith(base, "_roughness") ||
									   StringTools.endsWith(base, "_roug") ||
									   StringTools.endsWith(base, "_r") ||
									   StringTools.endsWith(base, "_rough") ||
									   StringTools.endsWith(base, "_rgh"))) {
					maprough = f;
					valid = true;
				}
				if (mapmet == "" && (StringTools.endsWith(base, "_metallic") ||
									 StringTools.endsWith(base, "_metal") ||
									 StringTools.endsWith(base, "_metalness") ||
									 StringTools.endsWith(base, "_m") ||
									 StringTools.endsWith(base, "_met"))) {
					mapmet = f;
					valid = true;
				}
				if (mapheight == "" && (StringTools.endsWith(base, "_displacement") ||
									    StringTools.endsWith(base, "_height") ||
									    StringTools.endsWith(base, "_h") ||
										StringTools.endsWith(base, "_disp"))) {
					mapheight = f;
					valid = true;
				}

				if (valid) UITrait.inst.importAsset(f);
			}
			// Create material
			autoFillHandle.selected = false;
			UITrait.inst.selectedMaterial = new MaterialSlot();
			UITrait.inst.materials.push(UITrait.inst.selectedMaterial);
			UINodes.inst.updateCanvasMap();
			var nodes = UINodes.inst.nodes;
			var canvas = UINodes.inst.canvas;
			var nout:Nodes.TNode = null;
			for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { nout = n; break; }
			for (n in canvas.nodes) if (n.name == "RGB") { nodes.removeNode(n, canvas); break; }
			
			var pos = 0;
			if (mapbase != "") {
				var n = NodeCreator.createImageTexture();
				n.buttons[0].default_value = arm.App.getAssetIndex(mapbase);
				n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
				n.x = 72;
				n.y = 192 + 160 * pos;
				pos++;
				var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 0 };
				canvas.links.push(l);
			}
			if (mapocc != "") {
				var n = NodeCreator.createImageTexture();
				n.buttons[0].default_value = arm.App.getAssetIndex(mapocc);
				n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
				n.x = 72;
				n.y = 192 + 160 * pos;
				pos++;
				var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 2 };
				canvas.links.push(l);
			}
			if (maprough != "") {
				var n = NodeCreator.createImageTexture();
				n.buttons[0].default_value = arm.App.getAssetIndex(maprough);
				n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
				n.x = 72;
				n.y = 192 + 160 * pos;
				pos++;
				var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 3 };
				canvas.links.push(l);
			}
			if (mapmet != "") {
				var n = NodeCreator.createImageTexture();
				n.buttons[0].default_value = arm.App.getAssetIndex(mapmet);
				n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
				n.x = 72;
				n.y = 192 + 160 * pos;
				pos++;
				var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 4 };
				canvas.links.push(l);
			}
			if (mapnor != "") {
				var n = NodeCreator.createImageTexture();
				n.buttons[0].default_value = arm.App.getAssetIndex(mapnor);
				n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
				n.x = 72;
				n.y = 192 + 160 * pos;
				pos++;
				var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 5 };
				canvas.links.push(l);
			}
			if (mapheight != "") {
				var n = NodeCreator.createImageTexture();
				n.buttons[0].default_value = arm.App.getAssetIndex(mapheight);
				n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
				n.x = 72;
				n.y = 192 + 160 * pos;
				pos++;
				var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 7 };
				canvas.links.push(l);
			}
			iron.system.Tween.timer(0.01, function() {
				UINodes.inst.parsePaintMaterial();
				UITrait.inst.makeMaterialPreview();
			});
			#end
		}
	}

	public function importAsset(path:String) {
		if (!checkImageFormat(path)) {
			showMessage("Error: Unknown asset format");
			return;
		}

		for (a in assets) if (a.file == path) { showMessage("Info: Asset already imported"); return; }
		
		iron.data.Data.getImage(path, function(image:kha.Image) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			var asset:TAsset = {name: name, file: path, id: UITrait.inst.assetId++};
			assets.push(asset);
			assetNames.push(name);
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
				#else
				var tmp = "";
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
					ddirty = 2;
				});

				// World envmap
				iron.Scene.active.world.envmap = image;
				savedEnvmap = image;

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
							iron.Scene.active.world.probe.radianceMipmaps = mips;
							ddirty = 2;
						}
					}, true); // Readable
					mw = Std.int(mw / 2);
					mh = Std.int(mh / 2);
				}
			}
		});
	}

	function done() {

		// notifyOnInit(function() {
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
			paintObject = cast (selectedObject, MeshObject);
			paintObjects = [paintObject];

			iron.App.notifyOnUpdate(update);
			iron.App.notifyOnRender2D(render);
			iron.App.notifyOnRender(initLayers);

			// Init plugins
			Plugin.keep();
			if (C.plugins != null) {
				for (plugin in C.plugins) {
					iron.data.Data.getBlob(plugin, function(blob:kha.Blob) {
						#if js
						untyped __js__("(1, eval)({0})", blob.toString());
						#end
					});
				}
			}
		// });
	}

	function update() {
		if (textureExport) {
			textureExport = false;
			exportTextures(textureExportPath);
		}
		if (projectExport) {
			projectExport = false;
			exportProject();
		}

		isScrolling = ui.isScrolling;
		updateUI();

		var kb = iron.system.Input.getKeyboard();
		var shift = kb.down("shift");
		var ctrl = kb.down("control");
		if (kb.started("tab")) {
			if (ctrl) { // Cycle objects
				var i = (paintObjects.indexOf(paintObject) + 1) % paintObjects.length;
				selectPaintObject(paintObjects[i]);
				hwnd.redraws = 2;
			}
			else { // Toggle node editor
				UIView2D.inst.show = false;
				if (!UINodes.inst.ui.isTyping && !UITrait.inst.ui.isTyping) {
					UINodes.inst.show = !UINodes.inst.show;
					arm.App.resize();
				}
			}
		}
		else if (kb.started("1") && (shift || ctrl)) shift ? setBrushType(0) : selectMaterial(0);
		else if (kb.started("2") && (shift || ctrl)) shift ? setBrushType(1) : selectMaterial(1);
		else if (kb.started("3") && (shift || ctrl)) shift ? setBrushType(2) : selectMaterial(2);
		else if (kb.started("4") && (shift || ctrl)) shift ? setBrushType(3) : selectMaterial(3);
		else if (kb.started("5") && (shift || ctrl)) shift ? setBrushType(4) : selectMaterial(4);

		if (ctrl && !shift && kb.started("s")) projectSave();
		else if (ctrl && shift && kb.started("s")) projectSaveAs();
		else if (ctrl && kb.started("o")) projectOpen();

		if (!arm.App.uimodal.isTyping) {
			if (kb.started("escape")) arm.App.showFiles = false;
			if (arm.App.showFiles && kb.started("enter")) {
				arm.App.showFiles = false;
				arm.App.filesDone(arm.App.path);
				UITrait.inst.ddirty = 2;
			}
		}

		if (kb.started("f12")) {
			show = !show;
			arm.App.resize();
		}

		if (kb.started("g") && (brushType == 2 || brushType == 3)) {
			autoFillHandle.selected = !autoFillHandle.selected;
			hwnd.redraws = 2;
			UINodes.inst.updateCanvasMap();
			UINodes.inst.parsePaintMaterial();
		}

		// Color pick shortcut
		var mouse = iron.system.Input.getMouse();
		// if (mouse.x > 0 && mouse.x < iron.App.w() && kb.started("alt")) {
		// 	lastBrushType = brushType;
		// 	setBrushType(4);
		// }
		// if (kb.released("alt") && lastBrushType != -1) {
		// 	setBrushType(lastBrushType);
		// 	lastBrushType = -1;
		// }
		if (mouse.x > 0 && mouse.x < iron.App.w()) {

			// kha.input.Mouse.get().hideSystemCursor();

			if (kb.released("alt")) {
				if (lastBrushType == -1) {
					lastBrushType = brushType;
					setBrushType(4);
				}
				else {
					setBrushType(lastBrushType);
					lastBrushType = -1;
				}
			}
		}
		// else {
			// kha.input.Mouse.get().showSystemCursor();
		// }

		for (p in Plugin.plugins) if (p.update != null) p.update();
	}

	public function makeStickerPreview() {
		if (stickerImage == null) stickerImage = kha.Image.createRenderTarget(512, 512);
		stickerPreview = true;

		var painto = paintObject;
		for (p in paintObjects) p.visible = false;

		var plane:MeshObject = cast iron.Scene.active.getChild("Plane");
		plane.visible = true;
		paintObject = plane;
		
		savedCamera.setFrom(iron.Scene.active.camera.transform.local);
		var m = Mat4.identity();
		m.translate(0, 0, 1);
		iron.Scene.active.camera.transform.setMatrix(m);
		var savedFov = iron.Scene.active.camera.data.raw.fov;
		iron.Scene.active.camera.data.raw.fov = 0.92;
		var light = iron.Scene.active.lights[0];
		light.data.raw.cast_shadow = false;

		// No jitter
		// @:privateAccess iron.Scene.active.camera.frame = 0;
		// No resize
		@:privateAccess iron.RenderPath.active.lastW = 512;
		@:privateAccess iron.RenderPath.active.lastH = 512;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();

		UINodes.inst.parseMeshPreviewMaterial();
		iron.RenderPath.active.commands = arm.renderpath.RenderPathDeferred.commandsSticker;
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
		iron.RenderPath.active.commands = arm.renderpath.RenderPathDeferred.commands;

		stickerPreview = false;
		@:privateAccess iron.RenderPath.active.lastW = iron.App.w();
		@:privateAccess iron.RenderPath.active.lastH = iron.App.h();

		// Restore
		plane.visible = false;
		for (p in paintObjects) p.visible = true;
		paintObject = painto;

		iron.Scene.active.camera.transform.setMatrix(savedCamera);
		iron.Scene.active.camera.data.raw.fov = savedFov;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();
		var light = iron.Scene.active.lights[0];
		light.data.raw.cast_shadow = true;
		UINodes.inst.parseMeshMaterial();
		ddirty = 2;
	}

	public function makeMaterialPreview() {
		materialPreview = true;

		var painto = paintObject;
		for (p in paintObjects) p.visible = false;

		var sphere:MeshObject = cast iron.Scene.active.getChild("Sphere");
		sphere.visible = true;
		paintObject = sphere;

		#if arm_editor
		sphere.materials[0] = htab.position == 0 ? selectedMaterial2.data : materials[0].data;
		var gizmo_vis = gizmo.visible;
		var grid_vis = grid.visible;
		gizmo.visible = false;
		grid.visible = false;
		#end
		
		savedCamera.setFrom(iron.Scene.active.camera.transform.local);
		var m = new Mat4(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		iron.Scene.active.camera.transform.setMatrix(m);
		var savedFov = iron.Scene.active.camera.data.raw.fov;
		iron.Scene.active.camera.data.raw.fov = 0.92;
		var light = iron.Scene.active.lights[0];
		light.data.raw.cast_shadow = false;
		iron.Scene.active.world.envmap = previewEnvmap;

		// No jitter
		// @:privateAccess iron.Scene.active.camera.frame = 0;
		// No resize
		@:privateAccess iron.RenderPath.active.lastW = 100;
		@:privateAccess iron.RenderPath.active.lastH = 100;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();

		UINodes.inst.parseMeshPreviewMaterial();
		iron.RenderPath.active.commands = arm.renderpath.RenderPathDeferred.commandsPreview;
		iron.RenderPath.active.renderFrame(iron.RenderPath.active.frameG);
		iron.RenderPath.active.commands = arm.renderpath.RenderPathDeferred.commands;

		materialPreview = false;
		@:privateAccess iron.RenderPath.active.lastW = iron.App.w();
		@:privateAccess iron.RenderPath.active.lastH = iron.App.h();

		// Restore
		sphere.visible = false;
		for (p in paintObjects) p.visible = true;
		paintObject = painto;

		#if arm_editor
		gizmo.visible = gizmo_vis;
		grid.visible = grid_vis;
		#end

		iron.Scene.active.camera.transform.setMatrix(savedCamera);
		iron.Scene.active.camera.data.raw.fov = savedFov;
		iron.Scene.active.camera.buildProjection();
		iron.Scene.active.camera.buildMatrix();
		var light = iron.Scene.active.lights[0];
		light.data.raw.cast_shadow = true;
		iron.Scene.active.world.envmap = showEnvmap ? savedEnvmap : emptyEnvmap;
		UINodes.inst.parseMeshMaterial();
	}

	function selectMaterial(i:Int) {
		if (materials.length <= i) return;
		selectedMaterial = materials[i];

		autoFillHandle.selected = false; // Auto-disable

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

	function updateUI() {

		messageTimer -= iron.system.Time.delta;

		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		// if (!show) return;
		if (!arm.App.uienabled) return;

		var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
		if (down && !kb.down("control")) {

			if (mouse.x <= iron.App.w()) {
				if (brushTime == 0 && C.undo_steps > 0) { // Paint started
					pushUndo = true;
					if (projectPath != "") {
						kha.Window.get(0).title = arm.App.filenameHandle.text + "* - ArmorPaint";
					}
				}
				brushTime += iron.system.Time.delta;
				for (f in _onBrush) f(0);
			}

		}
		else if (brushTime > 0) {
			brushTime = 0;
			ddirty = 3;
		}

		var undoPressed = kb.down("control") && !kb.down("shift") && kb.started("z");
		if (systemId == 'OSX') undoPressed = !kb.down("shift") && kb.started("z"); // cmd+z on macos

		var redoPressed = (kb.down("control") && kb.down("shift") && kb.started("z")) ||
						   kb.down("control") && kb.started("y");
		if (systemId == 'OSX') redoPressed = (kb.down("shift") && kb.started("z")) || kb.started("y"); // cmd+y on macos

		if (C.undo_steps > 0) {
			if (undoPressed && undos > 0) {
				undoI = undoI - 1 < 0 ? C.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				var opos = paintObjects.indexOf(lay.targetObject);
				var lpos = layers.indexOf(lay.targetLayer);
				if (opos >= 0 && lpos >= 0) {
					hwnd.redraws = 2;
					selectPaintObject(paintObjects[opos]);
					selectedLayer = layers[lpos];
					selectedLayer.swap(lay);
					ddirty = 2;
				}
				undos--;
				redos++;
			}
			else if (redoPressed && redos > 0) {
				var lay = undoLayers[undoI];
				var opos = paintObjects.indexOf(lay.targetObject);
				var lpos = layers.indexOf(lay.targetLayer);
				if (opos >= 0 && lpos >= 0) {
					hwnd.redraws = 2;
					selectPaintObject(paintObjects[opos]);
					selectedLayer = layers[lpos];
					selectedLayer.swap(lay);
					ddirty = 2;
				}
				undoI = (undoI + 1) % C.undo_steps;
				undos++;
				redos--;
			}
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

		iron.system.Input.occupied = (axisX || axisY || axisZ) && mouse.x < App.w();
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

		ddirty = 3;
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

	function resizeLayer(l:LayerSlot) {
		var res = getTextureRes();
		var rts = RenderPath.active.renderTargets;

		var texpaint = l.texpaint;
		var texpaint_nor = l.texpaint_nor;
		var texpaint_pack = l.texpaint_pack;
		var texpaint_opt = l.texpaint_opt;

		l.texpaint = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.Depth16);
		l.texpaint_nor = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		l.texpaint_pack = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		if (l.texpaint_opt != null) l.texpaint_opt = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);

		l.texpaint.g2.begin(false);
		l.texpaint.g2.drawScaledImage(texpaint, 0, 0, res, res);
		l.texpaint.g2.end();

		l.texpaint_nor.g2.begin(false);
		l.texpaint_nor.g2.drawScaledImage(texpaint_nor, 0, 0, res, res);
		l.texpaint_nor.g2.end();

		l.texpaint_pack.g2.begin(false);
		l.texpaint_pack.g2.drawScaledImage(texpaint_pack, 0, 0, res, res);
		l.texpaint_pack.g2.end();

		if (texpaint_opt != null) {
			l.texpaint_opt.g2.begin(false);
			l.texpaint_opt.g2.drawScaledImage(texpaint_opt, 0, 0, res, res);
			l.texpaint_opt.g2.end();
			rts.get("texpaint_opt" + l.ext).image = l.texpaint_opt;
		}

		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();
		if (texpaint_opt != null) texpaint_opt.unload();

		rts.get("texpaint" + l.ext).image = l.texpaint;
		rts.get("texpaint_nor" + l.ext).image = l.texpaint_nor;
		rts.get("texpaint_pack" + l.ext).image = l.texpaint_pack;
		
	}

	function resizeLayers(g:kha.graphics4.Graphics) {
		if (resHandle.position >= 4) { // Save memory for >=16k
			C.undo_steps = resHandle.position == 4 ? 1 : 0; // 1 undo for 16k, 0 for 20k
			if (undoHandle != null) undoHandle.value = C.undo_steps;
			while (undoLayers.length > C.undo_steps) { var l = undoLayers.pop(); l.unload(); }
		}
		g.end();
		for (l in layers) resizeLayer(l);
		for (l in undoLayers) resizeLayer(l);
		g.begin();
		ddirty = 2;
		iron.App.removeRender(resizeLayers);
	}

	function deleteSelectedLayer() {
		selectedLayer.unload();
		layers.remove(selectedLayer);
		selectedLayer = layers[0];
		UINodes.inst.parseMeshMaterial();
		UINodes.inst.parsePaintMaterial();
		ddirty = 2;
	}

	var pipe:kha.graphics4.PipelineState = null;
	function makePipe() {
		var frag = "
#version 330
uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;
void main() {
	vec4 texcolor = texture(tex, texCoord) * color;
	FragColor = texcolor;
}
		";
		var vert = "
#version 330
in vec3 vertexPosition;
in vec2 texPosition;
in vec4 vertexColor;
uniform mat4 projectionMatrix;
out vec2 texCoord;
out vec4 color;
void main() {
	gl_Position = projectionMatrix * vec4(vertexPosition, 1.0);
	texCoord = texPosition;
	color = vertexColor;
}
		";

		pipe = new kha.graphics4.PipelineState();
		pipe.fragmentShader = kha.graphics4.FragmentShader.fromSource(frag);
		pipe.vertexShader = kha.graphics4.VertexShader.fromSource(vert);
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

	public function getTextureRes():Int {
		if (resHandle.position == 0) return 1024;
		if (resHandle.position == 1) return 2048;
		if (resHandle.position == 2) return 4096;
		if (resHandle.position == 3) return 8192;
		if (resHandle.position == 4) return 16384;
		if (resHandle.position == 5) return 20480;
		return 0;
	}

	function getTextureResBias():Float {
		if (resHandle.position == 0) return 2.0;
		if (resHandle.position == 1) return 1.5;
		if (resHandle.position == 2) return 1.0;
		if (resHandle.position == 3) return 0.5;
		if (resHandle.position == 4) return 0.25;
		if (resHandle.position == 5) return 0.125;
		return 1.0;
	}

	function getTextureResPos(i:Int):Int {
		if (i == 1024) return 0;
		if (i == 2048) return 1;
		if (i == 4096) return 2;
		if (i == 8192) return 3;
		if (i == 16384) return 4;
		if (i == 20480) return 5;
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
		autoFillHandle.selected = false; // Auto-disable
		UINodes.inst.parsePaintMaterial();
		UINodes.inst.parseMeshMaterial();
		hwnd.redraws = 2;
		ddirty = 2;
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

	function selectPaintObject(o:iron.object.MeshObject) {
		autoFillHandle.selected = false; // Auto-disable
		for (p in paintObjects) p.skip_context = "paint";
		paintObject = o;
		if (mergedObject == null || maskHandle.position == 1) { // Single object or object mask set to none
			paintObject.skip_context = "";
		}
		UIView2D.inst.uvmapCached = false;
	}

	function renderUI(g:kha.graphics2.Graphics) {
		
		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();
		
		g.color = 0xffffffff;

		// Brush
		if (arm.App.uienabled) {
			var cursorImg = bundled.get('cursor.png');
			var mouse = iron.system.Input.getMouse();
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
			var pen = iron.system.Input.getPen();
			if (pen.down()) {
				mx = pen.x + iron.App.x();
				my = pen.y + iron.App.y();
			}

			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));

			if (brushPaint == 2) { // Sticker
				if (mouse.x > 0 && mx < iron.App.w()) {
					// var psize = Std.int(stickerImage.width * (brushRadius * brushNodesRadius));
					psize = Std.int(256 * (brushRadius * brushNodesRadius));
					g.drawScaledImage(stickerImage, mx - psize / 2, my - psize / 2 + psize, psize, -psize);
				}
			}
			else {
				// if (mouse.x > 0 && mx < iron.App.w()) {
				// if (brushType == 0 || brushType == 1 || brushType == 4) {
					g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
				// }
				// }
			}

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
			if (showGrid) {
				// Separator line
				var x1 = iron.App.x() + iron.App.w() / 3;
				var x2 = iron.App.x() + iron.App.w() / 3 * 2;
				var y1 = iron.App.y() + iron.App.h() / 3;
				var y2 = iron.App.y() + iron.App.h() / 3 * 2;
				g.color = 0x66ffffff;
				g.fillRect(x1 - 1, 0, 2, iron.App.h());
				g.fillRect(x2 - 1, 0, 2, iron.App.h());
				g.fillRect(iron.App.x(), y1 - 1, iron.App.x() + iron.App.w(), 2);
				g.fillRect(iron.App.x(), y2 - 1, iron.App.x() + iron.App.w(), 2);
				g.color = 0xffffffff;
			}
		}

		if (!show) return;

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
		
		var wx = C.ui_layout == 0 ? arm.App.realw() - windowW : 0;
		if (ui.window(hwnd, wx, 0, windowW, arm.App.realh())) {

			#if arm_editor

			gizmo.visible = false;
			grid.visible = false;

			if (ui.tab(htab, "Scene")) {
				gizmo.visible = true;
				grid.visible = true;
				ui.separator();
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

				ui.separator();
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
						if (h.changed) loc.x = f;

						h = Id.handle();
						h.text = Math.roundfp(loc.y) + "";
						f = Std.parseFloat(ui.textInput(h, "Y"));
						if (h.changed) loc.y = f;

						h = Id.handle();
						h.text = Math.roundfp(loc.z) + "";
						f = Std.parseFloat(ui.textInput(h, "Z"));
						if (h.changed) loc.z = f;

						ui.row(row4);
						ui.text("Rotation");
						
						h = Id.handle();
						h.text = Math.roundfp(rot.x) + "";
						f = Std.parseFloat(ui.textInput(h, "X"));
						var changed = false;
						if (h.changed) { changed = true; rot.x = f; }

						h = Id.handle();
						h.text = Math.roundfp(rot.y) + "";
						f = Std.parseFloat(ui.textInput(h, "Y"));
						if (h.changed) { changed = true; rot.y = f; }

						h = Id.handle();
						h.text = Math.roundfp(rot.z) + "";
						f = Std.parseFloat(ui.textInput(h, "Z"));
						if (h.changed) { changed = true; rot.z = f; }

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
						if (h.changed) scale.x = f;

						h = Id.handle();
						h.text = Math.roundfp(scale.y) + "";
						f = Std.parseFloat(ui.textInput(h, "Y"));
						if (h.changed) scale.y = f;

						h = Id.handle();
						h.text = Math.roundfp(scale.z) + "";
						f = Std.parseFloat(ui.textInput(h, "Z"));
						if (h.changed) scale.z = f;

						selectedObject.transform.dirty = true;

						if (selectedObject.name == "Scene") {
							selectedType = "(Scene)";
							// ui.image(envThumb);
							var p = iron.Scene.active.world.probe;
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
							if (fovHandle.changed) {
								cam.buildProjection();
								ddirty = 2;
							}
						}
						else {
							selectedType = "(Object)";
							
						}
					}

					if (ui.button("LNodes")) showLogicNodes();

					ui.unindent();
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Materials", 1)) {

					var empty = bundled.get("empty.jpg");

					for (row in 0...Std.int(Math.ceil(materials2.length / 5))) { 
						ui.row([1/5,1/5,1/5,1/5,1/5]);

						if (row > 0) ui._y += 6;

						ui.imageInvertY = true; // Material preview
						for (j in 0...5) {
							var i = j + row * 5;
							var img = i >= materials2.length ? empty : materials2[i].image;

							if (selectedMaterial2 == materials2[i]) {
								// ui.fill(1, -2, img.width + 3, img.height + 3, 0xff205d9c); // TODO
								var off = row % 2 == 1 ? 1 : 0;
								var w = 51 - C.window_scale;
								ui.fill(1,          -2, w + 3,       2, 0xff205d9c);
								ui.fill(1,     w - off, w + 3, 2 + off, 0xff205d9c);
								ui.fill(1,          -2,     2,   w + 3, 0xff205d9c);
								ui.fill(w + 3,      -2,     2,   w + 4, 0xff205d9c);
							}

							if (ui.image(img) == State.Started && img != empty) {
								if (selectedMaterial2 != materials2[i]) selectMaterial2(i);
								if (iron.system.Time.time() - selectTime < 0.3) showMaterialNodes();
								selectTime = iron.system.Time.time();
							}
							if (img != empty && ui.isHovered) ui.tooltipImage(img);
						}
						ui.imageInvertY = false; // Material preview
					}

					ui.row([1/2,1/2]);
					if (ui.button("New")) {

						iron.data.Data.cachedMaterials.remove("SceneMaterial2");
						iron.data.Data.cachedShaders.remove("Material2_data");
						iron.data.Data.cachedSceneRaws.remove("Material2_data");
						// iron.data.Data.cachedBlobs.remove("Material2_data.arm");
						iron.data.Data.getMaterial("Scene", "Material2", function(md:iron.data.MaterialData) {
							md.name = "Material2." + materials2.length;
							ui.g.end();
							var m = new MaterialSlot(md);
							materials2.push(m);
							selectMaterial2(materials2.length - 1);
							UITrait.inst.makeMaterialPreview();
							ui.g.begin(false);
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

				ui.separator();
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
						ui.row([1/3, 1/3, 1/3]);
						var h = Id.handle({value: bakeStrength});
						bakeStrength = ui.slider(h, "Strength", 0.0, 2.0, true);
						if (h.changed) UINodes.inst.parsePaintMaterial();
						var h = Id.handle({value: bakeRadius});
						bakeRadius = ui.slider(h, "Radius", 0.0, 2.0, true);
						if (h.changed) UINodes.inst.parsePaintMaterial();
						var h = Id.handle({value: bakeOffset});
						bakeOffset = ui.slider(h, "Offset", 0.0, 2.0, true);
						if (h.changed) UINodes.inst.parsePaintMaterial();
						ui.check(autoFillHandle, "Auto-Fill");
						if (autoFillHandle.changed) {
							UINodes.inst.updateCanvasMap();
							UINodes.inst.parsePaintMaterial();
						}
					}
					else { // Draw, Erase, Fil;
						ui.row([1/2, 1/2]);
						ui.combo(Id.handle(), ["Add"], "Blending");
						if (ui.button("Nodes")) showBrushNodes();
						ui.row([1/2, 1/2]);
						var paintHandle = Id.handle();
						brushPaint = ui.combo(paintHandle, ["UV", "Project", "Sticker"], "Paint");
						if (paintHandle.changed) {
							UINodes.inst.parsePaintMaterial();
							if (brushPaint == 2) { // Sticker
								ui.g.end();
								UITrait.inst.makeStickerPreview();
								ui.g.begin(false);
							}
						}
						brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 1.0, true);
						ui.row([1/2, 1/2]);
						brushRadius = ui.slider(Id.handle({value: brushRadius}), "Radius", 0.0, 2.0, true);
						brushOpacity = ui.slider(Id.handle({value: brushOpacity}), "Opacity", 0.0, 1.0, true);
						ui.row([1/2, 1/2]);
						var brushScaleHandle = Id.handle({value: brushScale});
						brushScale = ui.slider(brushScaleHandle, "UV Scale", 0.0, 2.0, true);
						if (brushScaleHandle.changed && autoFillHandle.selected) UINodes.inst.parsePaintMaterial();
						brushStrength = ui.slider(Id.handle({value: brushStrength}), "Strength", 0.0, 1.0, true);

						if (brushType == 2) { // Fill, Bake
							ui.check(autoFillHandle, "Auto-Fill");
							if (autoFillHandle.changed) {
								UINodes.inst.updateCanvasMap();
								UINodes.inst.parsePaintMaterial();
							}
						}
						else { // Draw, Erase
							ui.row([1/2,1/2]);
							paintVisible = ui.check(Id.handle({selected: paintVisible}), "Visible Only");
							var mirrorHandle = Id.handle({selected: mirrorX});
							mirrorX = ui.check(mirrorHandle, "Mirror Screen");
							if (mirrorHandle.changed) {
								UINodes.inst.updateCanvasMap();
								UINodes.inst.parsePaintMaterial();
							}
						}
					}
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Paint Maps", 1)) {
					ui.row([1/2,1/2]);
					var baseHandle = Id.handle({selected: paintBase});
					paintBase = ui.check(baseHandle, "Base Color");
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

					ui.row([1/2,1/2]);
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

					ui.row([1/2,1/2]);
					var metHandle = Id.handle({selected: paintMet});
					paintMet = ui.check(metHandle, "Metallic");
					if (metHandle.changed) {
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
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Materials", 1)) {

					var empty = bundled.get("empty.jpg");

					for (row in 0...Std.int(Math.ceil(materials.length / 5))) { 
						ui.row([1/5,1/5,1/5,1/5,1/5]);

						if (row > 0) ui._y += 6;

						ui.imageInvertY = true; // Material preview
						for (j in 0...5) {
							var i = j + row * 5;
							var img = i >= materials.length ? empty : materials[i].image;

							if (selectedMaterial == materials[i]) {
								// ui.fill(1, -2, img.width + 3, img.height + 3, 0xff205d9c); // TODO
								var off = row % 2 == 1 ? 1 : 0;
								var w = 51 - C.window_scale;
								ui.fill(1,          -2, w + 3,       2, 0xff205d9c);
								ui.fill(1,     w - off, w + 3, 2 + off, 0xff205d9c);
								ui.fill(1,          -2,     2,   w + 3, 0xff205d9c);
								ui.fill(w + 3,      -2,     2,   w + 4, 0xff205d9c);
							}

							if (ui.image(img) == State.Started && img != empty) {
								if (selectedMaterial != materials[i]) selectMaterial(i);
								if (iron.system.Time.time() - selectTime < 0.3) showMaterialNodes();
								selectTime = iron.system.Time.time();
							}
							if (img != empty && ui.isHovered) ui.tooltipImage(img);
						}
						ui.imageInvertY = false; // Material preview
					}

					ui.row([1/2,1/2]);
					if (ui.button("New")) {
						ui.g.end();
						autoFillHandle.selected = false; // Auto-disable
						selectedMaterial = new MaterialSlot();
						materials.push(selectedMaterial);
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
						UITrait.inst.makeMaterialPreview();
						ui.g.begin(false);
					}
					if (ui.button("Nodes")) {
						showMaterialNodes();
					}
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Layers", 1)) {

					function drawList(h:zui.Zui.Handle, l:LayerSlot, i:Int) {
						if (selectedLayer == l) {
							ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, 0xff205d9c);
						}
						if (i > 0) ui.row([1/10, 5/10, 2/10, 2/10]);
						else ui.row([1/10, 9/10]);
						var h = Id.handle().nest(l.id, {selected: l.visible});
						l.visible = ui.check(h, "");
						if (h.changed) {
							UINodes.inst.parseMeshMaterial();
							ddirty = 2;
						}
						ui.text("Layer " + (i + 1));
						if (ui.isReleased) {
							selectedLayer = l;
							UINodes.inst.parsePaintMaterial(); // Different blending for layer on top
							ddirty = 2;
						}
						if (i > 0) {
							if (ui.button("Apply")) {
								selectedLayer = layers[i];
								iron.App.notifyOnRender(applySelectedLayer);
							}
							if (ui.button("Delete")) {
								selectedLayer = layers[i];
								deleteSelectedLayer();
							}
						}
					}
					for (i in 0...layers.length) {
						if (i >= layers.length) break; // Layer was deleted
						var j = layers.length - 1 - i;
						var l = layers[j];
						drawList(Id.handle(), l, j);
					}

					if (layers.length < 4) { // TODO: Samplers limit in Kore, use arrays
						ui.row([1/2, 1/2]);
						if (ui.button("New")) {
							autoFillHandle.selected = false; // Auto-disable
							selectedLayer = new LayerSlot();
							layers.push(selectedLayer);
							ui.g.end();
							UINodes.inst.parseMeshMaterial();
							UINodes.inst.parsePaintMaterial();
							ui.g.begin(false);
							ddirty = 2;
							iron.App.notifyOnRender(clearLastLayer);
						}
					}
					if (ui.button("2D View")) show2DView();
				}

				ui.separator();
				if (ui.panel(objectsHandle, "Objects", 1)) {

					var i = 0;
					function drawList(h:zui.Zui.Handle, o:iron.object.MeshObject) {
						// if (o.name.charAt(0) == '.') return; // Hidden
						var b = false;
						if (paintObject == o) {
							ui.g.color = 0xff205d9c;
							ui.g.fillRect(0, ui._y, ui._windowW, ui.t.ELEMENT_H);
							ui.g.color = 0xffffffff;
						}
						ui.row([1/10, 9/10]);
						var h = Id.handle().nest(i, {selected: o.visible});
						o.visible = ui.check(h, "");
						if (h.changed) ddirty = 2;
						ui.text(o.name);
						if (ui.isReleased) {
							selectPaintObject(o);
						}
						i++;
					}
					for (c in paintObjects) {
						drawList(Id.handle(), c);
					}

					var loc = paintObject.transform.loc;
					var scale = paintObject.transform.scale;
					var rot = paintObject.transform.rot.getEuler();
					rot.mult(180 / 3.141592);
					var f = 0.0;

					if (paintObjects.length > 1) {
						ui.row([1/2, 1/2]);
						var maskType = ui.combo(maskHandle, ["None", "Object"], "Mask", true);
						if (maskHandle.changed) {
							if (maskType == 1) {
								// if (mergedObject != null) mergedObject.remove();
								if (mergedObject != null) mergedObject.visible = false;
								selectPaintObject(paintObject);
							}
							else {
								if (mergedObject == null) {
									ui.g.end();
									mergeMesh();
									ui.g.begin(false);
								}
								selectPaintObject(mainObject());
								paintObject.skip_context = "paint";
								// if (mergedObject.parent == null) paintObject.addChild(mergedObject);
								mergedObject.visible = true;
							}
						}
						ui.combo(Id.handle({position: 0}), ["Combined"], "UV Map", true);
					}

					// ui.text("Transform");

					// ui.row(row4);
					// ui.text("Location");

					// var h = Id.handle();
					// h.text = Math.roundfp(loc.x) + "";
					// f = Std.parseFloat(ui.textInput(h, "X"));
					// var changed = false;
					// if (h.changed) { loc.x = f; changed = true; }

					// h = Id.handle();
					// h.text = Math.roundfp(loc.y) + "";
					// f = Std.parseFloat(ui.textInput(h, "Y"));
					// if (h.changed) { loc.y = f; changed = true; }

					// h = Id.handle();
					// h.text = Math.roundfp(loc.z) + "";
					// f = Std.parseFloat(ui.textInput(h, "Z"));
					// if (h.changed) { loc.z = f; changed = true; }

					// if (changed) {
					// 	paintObject.transform.dirty = true;
					// 	ddirty = 2;
					// }

					// ui.row(row4);
					// ui.text("Rotation");
					
					// h = Id.handle();
					// h.text = Math.roundfp(rot.x) + "";
					// f = Std.parseFloat(ui.textInput(h, "X"));
					// var changed = false;
					// if (h.changed) { changed = true; rot.x = f; }

					// h = Id.handle();
					// h.text = Math.roundfp(rot.y) + "";
					// f = Std.parseFloat(ui.textInput(h, "Y"));
					// if (h.changed) { changed = true; rot.y = f; }

					// h = Id.handle();
					// h.text = Math.roundfp(rot.z) + "";
					// f = Std.parseFloat(ui.textInput(h, "Z"));
					// if (h.changed) { changed = true; rot.z = f; }

					// if (changed) {
					// 	rot.mult(3.141592 / 180);
					// 	paintObject.transform.rot.fromEuler(rot.x, rot.y, rot.z);
					// 	paintObject.transform.buildMatrix();
					// 	ddirty = 2;
					// }

					// ui.row(row4);
					// ui.text("Scale");
					
					// h = Id.handle();
					// h.text = Math.roundfp(scale.x) + "";
					// f = Std.parseFloat(ui.textInput(h, "X"));
					// if (h.changed) { scale.x = f; ddirty = 2; paintObject.transform.dirty = true; }

					// h = Id.handle();
					// h.text = Math.roundfp(scale.y) + "";
					// f = Std.parseFloat(ui.textInput(h, "Y"));
					// if (h.changed) { scale.y = f; ddirty = 2; paintObject.transform.dirty = true; }

					// h = Id.handle();
					// h.text = Math.roundfp(scale.z) + "";
					// f = Std.parseFloat(ui.textInput(h, "Z"));
					// if (h.changed) { scale.z = f; ddirty = 2; paintObject.transform.dirty = true; }
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Camera", 1)) {
					var scene = iron.Scene.active;
					var cam = scene.cameras[0];
					ui.row([1/2,1/2]);
					cameraControls = ui.combo(Id.handle({position: cameraControls}), ["ArcBall", "Orbit", "Fly"], "Controls");
					cameraType = ui.combo(camHandle, ["Perspective", "Orhographic"], "Type");
					if (camHandle.changed) {
						if (cameraType == 0) cam.data.raw.ortho = null;
						else cam.data.raw.ortho = [-2, 2, -2 * (iron.App.h() / iron.App.w()), 2 * (iron.App.h() / iron.App.w())];
						
						if (originalShadowBias <= 0) originalShadowBias = iron.Scene.active.lights[0].data.raw.shadows_bias;
						iron.Scene.active.lights[0].data.raw.shadows_bias = cameraType == 0 ? originalShadowBias : originalShadowBias * 15;
						cam.buildProjection();
						
						ddirty = 2;
					}

					ui.row([1/2,1/2]);
					fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
					cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
					if (fovHandle.changed) {
						cam.buildProjection();
						ddirty = 2;
					}
					if (ui.button("Reset")) {
						resetViewport();
						scaleToBounds();
					}
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Viewport", 1)) {
					if (iron.Scene.active.world.probe.radianceMipmaps.length > 0) {
						ui.image(iron.Scene.active.world.probe.radianceMipmaps[0]);
					}
					var p = iron.Scene.active.world.probe;
					ui.row([1/2, 1/2]);
					var envType = ui.combo(Id.handle({position: 0}), ["Default"], "Envmap");
					var envHandle = Id.handle({value: p.raw.strength});
					p.raw.strength = ui.slider(envHandle, "Environment", 0.0, 8.0, true);
					if (envHandle.changed) ddirty = 2;
					ui.row([1/2, 1/2]);
					var modeHandle = Id.handle({position: 0});
					viewportMode = ui.combo(modeHandle, ["Render", "Base Color", "Normal", "Occlusion", "Roughness", "Metallic"], "Mode");
					if (modeHandle.changed) {
						UINodes.inst.parseMeshMaterial();
						ddirty = 2;
					}
					if (iron.Scene.active.lights.length > 0) {
						var light = iron.Scene.active.lights[0];
						var lhandle = Id.handle();
						lhandle.value = light.data.raw.strength / 10;
						light.data.raw.strength = ui.slider(lhandle, "Light", 0.0, 4.0, true) * 10;
						if (lhandle.changed) ddirty = 2;
					}

					ui.row([1/2, 1/2]);
					var upHandle = Id.handle();
					var lastUp = upHandle.position;
					var axisUp = ui.combo(upHandle, ["Z", "Y"], "Up Axis", true);
					if (upHandle.changed && axisUp != lastUp) {
						switchUpAxis(axisUp);
						ddirty = 2;
					}
					
					var dispHandle = Id.handle({value: displaceStrength});
					displaceStrength = ui.slider(dispHandle, "Displace", 0.0, 2.0, true);
					if (dispHandle.changed) {
						UINodes.inst.parseMeshMaterial();
						ddirty = 2;
					}

					ui.row([1/2, 1/2]);
					var showEnvmapHandle = Id.handle({selected: showEnvmap});
					showEnvmap = ui.check(showEnvmapHandle, "Show Envmap");
					if (showEnvmapHandle.changed) {
						ddirty = 2;
					}
					showGrid = ui.check(Id.handle({selected: showGrid}), "Show Grid");

					if (!showEnvmap) {
						if (ui.panel(Id.handle({selected: false}), "Viewport Color")) {
							var hwheel = Id.handle({color: 0xff030303});
							var worldColor:kha.Color = Ext.colorWheel(ui, hwheel);
							if (hwheel.changed) {
								var b = emptyEnvmap.lock();
								b.set(0, worldColor.Rb);
								b.set(1, worldColor.Gb);
								b.set(2, worldColor.Bb);
								emptyEnvmap.unlock();
								ddirty = 2;
							}
						}
					}
					iron.Scene.active.world.envmap = showEnvmap ? savedEnvmap : emptyEnvmap;
				}

				// Draw plugins
				for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
			}
			if (ui.tab(htab, "Library")) {

				ui.separator();
				if (ui.button("Import")) {
					arm.App.showFiles = true;
					@:privateAccess zui.Ext.lastPath = ""; // Refresh
					arm.App.whandle.redraws = 2;
					arm.App.foldersOnly = false;
					arm.App.showFilename = false;
					arm.App.filesDone = function(path:String) {
						importFile(path);
					}
				}

				ui.separator();
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
								iron.data.Data.deleteImage(asset.file);
								Canvas.assetMap.remove(asset.id);
								assets.splice(i, 1);
								assetNames.splice(i, 1);
							}
							i--;
						}
					}
					else {
						ui.text("(Drag & drop files onto window)");
					}
				}
			}

			if (ui.tab(htab, "File")) {

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Project", 1)) {
					ui.row([1/2,1/2]);
					if (newConfirm) {
						if (ui.button("Confirm")) {
							newConfirm = false;
							projectNew();
							scaleToBounds();
						}
					}
					else if (ui.button("New")) {
						newConfirm = true;
					}
					newObject = ui.combo(Id.handle(), newObjectNames, "Default Object");
					ui.row([1/3,1/3,1/3]);
					if (ui.button("Open")) projectOpen();
					if (ui.button("Save")) projectSave();
					if (ui.button("Save As")) projectSaveAs();
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Project Quality", 1)) {
					ui.combo(resHandle, ["1K", "2K", "4K", "8K", "16K", "20K"], "Res", true);
					if (resHandle.changed) {
						iron.App.notifyOnRender(resizeLayers);
					}
					ui.combo(Id.handle(), ["8bit"], "Color", true);
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Export Mesh", 1)) {
					if (ui.button("Export")) {
						arm.App.showFiles = true;
						@:privateAccess zui.Ext.lastPath = ""; // Refresh
						arm.App.whandle.redraws = 2;
						arm.App.foldersOnly = true;
						arm.App.showFilename = true;
						arm.App.filesDone = function(path:String) {
							
							var f = arm.App.filenameHandle.text;
							if (f == "") f = "untitled";

							var s = "";
							var off = 0;
							for (p in paintObjects) {
								var mesh = p.data.raw;
								var posa = mesh.vertex_arrays[0].values;
								var nora = mesh.vertex_arrays[1].values;
								var texa = mesh.vertex_arrays[2].values;
								var len = Std.int(posa.length / 3);
								s += "o " + p.name + "\n";
								for (i in 0...len) {
									s += "v " + posa[i * 3 + 0] + " " +
												posa[i * 3 + 2] + " " +
												(-posa[i * 3 + 1]) + "\n";
								}
								for (i in 0...len) {
									s += "vn " + nora[i * 3 + 0] + " " +
												 nora[i * 3 + 2] + " " +
												 (-nora[i * 3 + 1]) + "\n";
								}
								for (i in 0...len) {
									s += "vt " + texa[i * 2 + 0] + " " +
												 (1.0 - texa[i * 2 + 1]) + "\n";
								}
								var inda = mesh.index_arrays[0].values;
								for (i in 0...Std.int(inda.length / 3)) {

									var i1 = inda[i * 3 + 0] + 1 + off;
									var i2 = inda[i * 3 + 1] + 1 + off;
									var i3 = inda[i * 3 + 2] + 1 + off;
									s += "f " + i1 + "/" + i1 + "/" + i1 + " " +
												i2 + "/" + i2 + "/" + i2 + " " +
												i3 + "/" + i3 + "/" + i3 + "\n";
								}
								off += inda.length;
							}
							#if kha_krom
							var objpath = path + "/" + f;
							if (!StringTools.endsWith(objpath, ".obj")) objpath += ".obj";
							Krom.fileSaveBytes(objpath, haxe.io.Bytes.ofString(s).getData());
							#end
						};
					}
					ui.combo(Id.handle(), ["obj"], "Format", true);
					var mesh = paintObject.data.raw;
					var inda = mesh.index_arrays[0].values;
					var tris = Std.int(inda.length / 3);
					ui.text(tris + " triangles");
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Export Textures", 1)) {

					if (ui.button("Export")) {
						arm.App.showFiles = true;
						@:privateAccess zui.Ext.lastPath = ""; // Refresh
						arm.App.whandle.redraws = 2;
						arm.App.foldersOnly = true;
						arm.App.showFilename = true;
						// var path = 'C:\\Users\\lubos\\Documents\\';
						arm.App.filesDone = function(path:String) {
							textureExport = true;
							textureExportPath = path;
						}
					}

					formatType = ui.combo(Id.handle({position: formatType}), ["jpg", "png"], "Format", true);
					if (formatType == 0) {
						formatQuality = ui.slider(Id.handle({value: formatQuality}), "Quality", 0.0, 100.0, true, 1);
					}
					outputType = ui.combo(Id.handle(), ["Generic", "UE4 (ORM)"], "Output", true);
					ui.text("Export Maps");
					ui.row([1/2, 1/2]);
					isBase = ui.check(Id.handle({selected: isBase}), "Base Color");
					isBaseSpace = ui.combo(Id.handle({position: isBaseSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isOpac = ui.check(Id.handle({selected: isOpac}), "Opacity");
					isOpacSpace = ui.combo(Id.handle({position: isOpacSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isOcc = ui.check(Id.handle({selected: isOcc}), "Occlusion");
					isOccSpace = ui.combo(Id.handle({position: isOccSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isRough = ui.check(Id.handle({selected: isRough}), "Roughness");
					isRoughSpace = ui.combo(Id.handle({position: isRoughSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isMet = ui.check(Id.handle({selected: isMet}), "Metallic");
					isMetSpace = ui.combo(Id.handle({position: isMetSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isNor = ui.check(Id.handle({selected: isNor}), "Normal");
					isNorSpace = ui.combo(Id.handle({position: isNorSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isHeight = ui.check(Id.handle({selected: isHeight}), "Height");
					isHeightSpace = ui.combo(Id.handle({position: isHeightSpace}), ["linear", "srgb"], "Space");
				}
			}

			if (ui.tab(htab, "Preferences")) {
				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Interface", 1)) {
					var hscale = Id.handle({value: C.window_scale});
					ui.slider(hscale, "UI Scale", 0.5, 4.0, true);
					if (!hscale.changed && hscaleWasChanged) {
						C.window_scale = hscale.value;
						ui.setScale(hscale.value);
						arm.App.uimodal.setScale(hscale.value);
						UINodes.inst.ui.setScale(hscale.value);
						UIView2D.inst.ui.setScale(hscale.value);
						windowW = Std.int(defaultWindowW * C.window_scale);
						arm.App.resize();
						armory.data.Config.save();
					}
					hscaleWasChanged = hscale.changed;
					ui.row([1/2, 1/2]);
					var layHandle = Id.handle({position: C.ui_layout});
					C.ui_layout = ui.combo(layHandle, ["Right", "Left"], "Layout", true);
					if (layHandle.changed) {
						arm.App.resize();
						armory.data.Config.save();
					}
					ui.combo(Id.handle({position: 0}), ["Dark"], "Theme", true);
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Usage", 1)) {
					undoHandle = Id.handle({value: C.undo_steps});
					C.undo_steps = Std.int(ui.slider(undoHandle, "Undo Steps", 0, 64, false, 1));
					if (undoHandle.changed) {
						ui.g.end();
						while (undoLayers.length < C.undo_steps) undoLayers.push(new LayerSlot("_undo" + undoLayers.length));
						while (undoLayers.length > C.undo_steps) { var l = undoLayers.pop(); l.unload(); }
						undos = 0;
						redos = 0;
						undoI = 0;
						ui.g.begin(false);
						armory.data.Config.save();
					}
					ui.row([1/2, 1/2]);
					penPressure = ui.check(Id.handle({selected: penPressure}), "Pen Pressure");
					instantMat = ui.check(Id.handle({selected: instantMat}), "Material Preview");
				}

				#if arm_debug
				iron.Scene.active.sceneParent.getTrait(armory.trait.internal.DebugConsole).visible = ui.check(Id.handle({selected: false}), "Debug Console");
				#end

				hssgi = Id.handle({selected: C.rp_ssgi});
				hssr = Id.handle({selected: C.rp_ssr});
				hbloom = Id.handle({selected: C.rp_bloom});
				hshadowmap = Id.handle({position: getShadowQuality(C.rp_shadowmap)});
				hsupersample = Id.handle({position: getSuperSampleQuality(C.rp_supersample)});
				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Viewport", 1)) {
					ui.row([1/2, 1/2]);
					ui.combo(hshadowmap, ["Ultra", "High", "Medium", "Low", "Off"], "Shadows", true);
					if (hshadowmap.changed) applyConfig();
					ui.combo(hsupersample, ["0.5x", "1.0x", "1.5x", "2.0x"], "Super Sample", true);
					if (hsupersample.changed) applyConfig();
					ui.row([1/2, 1/2]);
					var vsyncHandle = Id.handle({selected: C.window_vsync});
					C.window_vsync = ui.check(vsyncHandle, "VSync");
					if (vsyncHandle.changed) armory.data.Config.save();
					var cullHandle = Id.handle({selected: culling});
					culling = ui.check(cullHandle, "Cull Backfaces");
					if (cullHandle.changed) {
						UINodes.inst.parseMeshMaterial();
						ddirty = 2;
					}
					ui.row([1/2, 1/2]);
					ui.check(hssgi, "SSAO");
					if (hssgi.changed) applyConfig();
					ui.check(hssr, "SSR");
					if (hssr.changed) applyConfig();
					ui.check(hbloom, "Bloom");
					if (hbloom.changed) applyConfig();
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Plugins", 1)) {
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Controls", 1)) {
					ui.text("Distract Free - F12");
					ui.text("Node Editor - Tab");
					ui.text("Select Tool - Shift+1-9");
					ui.text("Select Material - Ctrl+1-9");
					ui.text("Pick Color ID - Alt");
					ui.text("Undo - Ctrl+Z");
					ui.text("Redo - Ctrl+Shift+Z");
					ui.text("Next Object - Ctrl+Tab");
					ui.text("Save - Ctrl+S");
					ui.text("Save As - Ctrl+Shift+S");
					ui.text("Open - Ctrl+O");
					ui.text("Auto-Fill - G");
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "About", 1)) {
					ui.text("v" + version + " - " +  Macro.buildSha() + " - armorpaint.org");
					// ui.text(Macro.buildDate());
					var renderer = #if (rp_renderer == "Deferred") "Deferred" #else "Forward" #end;
					ui.text("System: " + kha.System.systemId + " - Renderer: " + renderer);
				}
			}
		}
		ui.end();
		g.begin(false);
	}

	inline function getShadowQuality(i:Int):Int {
		// 0 - Ultra, 1- High, 2 - Medium, 3 - Low, 4 - Off
		return i == 8192 ? 0 : i == 4096 ? 1 : i == 2048 ? 2 : i == 1024 ? 3 : 4;
	}

	inline function getShadowMapSize(i:Int):Int {
		return i == 0 ? 8192 : i == 1 ? 4096 : i == 2 ? 2048 : i == 3 ? 1024 : 1;
	}

	inline function getSuperSampleQuality(f:Float):Int {
		return f == 0.5 ? 0 : f == 1.0 ? 1 : f == 1.5 ? 2 : f == 2.0 ? 3 : 4;
	}

	inline function getSuperSampleSize(i:Int):Float {
		return i == 0 ? 0.5 : i == 1 ? 1.0 : i == 2 ? 1.5 : i == 3 ? 2.0 : 4.0;
	}

	public function getImage(asset:TAsset):kha.Image {
		return Canvas.assetMap.get(asset.id);
	}

	public static function checkProjectFormat(path:String):Bool {
		var p = path.toLowerCase();
		if (!StringTools.endsWith(p, ".arm")) {
			return false;
		}
		return true;
	}

	public static function checkMeshFormat(path:String):Bool {
		var p = path.toLowerCase();
		if (!StringTools.endsWith(p, ".obj") &&
			!StringTools.endsWith(p, ".gltf") &&
			!StringTools.endsWith(p, ".blend") &&
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

		#if arm_debug
		var timer = iron.system.Time.realTime();
		#end

		var p = path.toLowerCase();
		if (StringTools.endsWith(p, ".obj")) importObj(path);
		else if (StringTools.endsWith(p, ".gltf")) importGltf(path);
		else if (StringTools.endsWith(p, ".fbx")) importFbx(path);
		else if (StringTools.endsWith(p, ".blend")) importBlend(path);

		if (mergedObject != null) {
			mergedObject.remove();
			iron.data.Data.deleteMesh(mergedObject.data.handle);
			mergedObject = null;
		}

		selectPaintObject(mainObject());

		if (paintObjects.length > 1) {
			objectsHandle.selected = true;

			// Sort by name
			paintObjects.sort(function(a, b):Int {
				if (a.name < b.name) return -1;
				else if (a.name > b.name) return 1;
				return 0;
			});

			// No mask by default
			if (mergedObject == null) mergeMesh();
			paintObject.skip_context = "paint";
			mergedObject.visible = true;
		}

		// Import is synchronous for now
		scaleToBounds();

		if (paintObject.name == "") paintObject.name = "Object";

		UIView2D.inst.hwnd.redraws = 2;

		#if arm_debug
		trace("Mesh imported in " + (iron.system.Time.realTime() - timer));
		#end
	}

	function importObj(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.obj.ObjParser(b);
			makeMesh(obj, path);
			while (obj.hasNext) {
				obj = new iron.format.obj.ObjParser(b, obj.pos);
				addMesh(obj);
			}
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
			while (obj.next()) {
				addMesh(obj);
			}
		});
	}

	function mainObject():MeshObject {
		for (po in paintObjects) if (po.children.length > 0) return po;
		return paintObjects[0];
	}

	function scaleToBounds() {
		var po = mergedObject == null ? mainObject() : mergedObject;
		var md = po.data;
		md.geom.calculateAABB();
		var r = Math.sqrt(md.geom.aabb.x * md.geom.aabb.x + md.geom.aabb.y * md.geom.aabb.y + md.geom.aabb.z * md.geom.aabb.z);
		po = mainObject();
		po.transform.dim.x = md.geom.aabb.x;
		po.transform.dim.y = md.geom.aabb.y;
		po.transform.dim.z = md.geom.aabb.z;
		po.transform.scale.set(2 / r, 2 / r, 2 / r);
		po.transform.buildMatrix();
	}

	function importBlend(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var bl = new iron.format.blend.Blend(b);

			// var obs = bl.get("Object");
			// var ob = obs[0];
			// var name:String = ob.get("id").get("name");
			// name = name.substring(2, name.length);
			// trace(ob.get("type")); // 1

			var m = bl.get("Mesh")[0];

			var totpoly = m.get("totpoly");
			var numtri = 0;
			for (i in 0...totpoly) {
				var poly = m.get("mpoly", i);
				var totloop = poly.get("totloop");
				numtri += totloop == 3 ? 1 : 2;
			}
			var inda = new kha.arrays.Uint32Array(numtri * 3);
			for (i in 0...inda.length) inda[i] = i;

			var posa32 = new kha.arrays.Float32Array(numtri * 3 * 4);
			var posa = new kha.arrays.Int16Array(numtri * 3 * 4);
			var nora = new kha.arrays.Int16Array(numtri * 3 * 2);
			var hasuv = m.get("mloopuv") != null;
			var texa = hasuv ? new kha.arrays.Int16Array(numtri * 3 * 2) : null;
			
			var tri = 0;
			var vec0 = new iron.math.Vec4();
			var vec1 = new iron.math.Vec4();
			var vec2 = new iron.math.Vec4();
			var vec3 = new iron.math.Vec4();
			for (i in 0...totpoly) {
				var poly = m.get("mpoly", i);
				var loopstart = poly.get("loopstart");
				var totloop = poly.get("totloop");
				if (totloop >= 3) {
					var v0 = m.get("mvert", m.get("mloop", loopstart + 0).get("v"));
					var v1 = m.get("mvert", m.get("mloop", loopstart + 1).get("v"));
					var v2 = m.get("mvert", m.get("mloop", loopstart + 2).get("v"));
					var co0 = v0.get("co");
					var co1 = v1.get("co");
					var co2 = v2.get("co");
					var no0 = v0.get("no");
					var no1 = v1.get("no");
					var no2 = v2.get("no");
					vec0.set(no0[0] / 32767, no0[1] / 32767, no0[2] / 32767).normalize(); // shortmax
					vec1.set(no1[0] / 32767, no1[1] / 32767, no1[2] / 32767).normalize();
					vec2.set(no2[0] / 32767, no2[1] / 32767, no2[2] / 32767).normalize();
					posa32[tri * 9    ] = co0[0];
					posa32[tri * 9 + 1] = co0[1];
					posa32[tri * 9 + 2] = co0[2];
					posa32[tri * 9 + 3] = co1[0];
					posa32[tri * 9 + 4] = co1[1];
					posa32[tri * 9 + 5] = co1[2];
					posa32[tri * 9 + 6] = co2[0];
					posa32[tri * 9 + 7] = co2[1];
					posa32[tri * 9 + 8] = co2[2];
					posa[tri * 12 + 3] = Std.int(vec0.z * 32767);
					posa[tri * 12 + 7] = Std.int(vec1.z * 32767);
					posa[tri * 12 + 11] = Std.int(vec2.z * 32767);
					nora[tri * 6    ] = Std.int(vec0.x * 32767);
					nora[tri * 6 + 1] = Std.int(vec0.y * 32767);
					nora[tri * 6 + 2] = Std.int(vec1.x * 32767);
					nora[tri * 6 + 3] = Std.int(vec1.y * 32767);
					nora[tri * 6 + 4] = Std.int(vec2.x * 32767);
					nora[tri * 6 + 5] = Std.int(vec2.y * 32767);
					
					var uv0:kha.arrays.Float32Array = null;
					var uv1:kha.arrays.Float32Array = null;
					var uv2:kha.arrays.Float32Array = null;
					if (hasuv) {
						uv0 = m.get("mloopuv", loopstart + 0).get("uv");
						uv1 = m.get("mloopuv", loopstart + 1).get("uv");
						uv2 = m.get("mloopuv", loopstart + 2).get("uv");
						texa[tri * 6    ] = Std.int(uv0[0] * 32767);
						texa[tri * 6 + 1] = Std.int((1.0 - uv0[1]) * 32767);
						texa[tri * 6 + 2] = Std.int(uv1[0] * 32767);
						texa[tri * 6 + 3] = Std.int((1.0 - uv1[1]) * 32767);
						texa[tri * 6 + 4] = Std.int(uv2[0] * 32767);
						texa[tri * 6 + 5] = Std.int((1.0 - uv2[1]) * 32767);
					}
					tri++;

					if (totloop >= 4) {
						var v3 = m.get("mvert", m.get("mloop", loopstart + 3).get("v"));
						var co3 = v3.get("co");
						var no3 = v3.get("no");
						vec3.set(no3[0] / 32767, no3[1] / 32767, no3[2] / 32767).normalize();
						posa32[tri * 9    ] = co2[0];
						posa32[tri * 9 + 1] = co2[1];
						posa32[tri * 9 + 2] = co2[2];
						posa32[tri * 9 + 3] = co3[0];
						posa32[tri * 9 + 4] = co3[1];
						posa32[tri * 9 + 5] = co3[2];
						posa32[tri * 9 + 6] = co0[0];
						posa32[tri * 9 + 7] = co0[1];
						posa32[tri * 9 + 8] = co0[2];
						posa[tri * 12 + 3] = Std.int(vec2.z * 32767);
						posa[tri * 12 + 7] = Std.int(vec3.z * 32767);
						posa[tri * 12 + 11] = Std.int(vec0.z * 32767);
						nora[tri * 6    ] = Std.int(vec2.x * 32767);
						nora[tri * 6 + 1] = Std.int(vec2.y * 32767);
						nora[tri * 6 + 2] = Std.int(vec3.x * 32767);
						nora[tri * 6 + 3] = Std.int(vec3.y * 32767);
						nora[tri * 6 + 4] = Std.int(vec0.x * 32767);
						nora[tri * 6 + 5] = Std.int(vec0.y * 32767);
						
						if (hasuv) {
							var uv3 = m.get("mloopuv", loopstart + 3).get("uv");
							texa[tri * 6    ] = Std.int(uv2[0] * 32767);
							texa[tri * 6 + 1] = Std.int((1.0 - uv2[1]) * 32767);
							texa[tri * 6 + 2] = Std.int(uv3[0] * 32767);
							texa[tri * 6 + 3] = Std.int((1.0 - uv3[1]) * 32767);
							texa[tri * 6 + 4] = Std.int(uv0[0] * 32767);
							texa[tri * 6 + 5] = Std.int((1.0 - uv0[1]) * 32767);
						}
						tri++;
					}
				}
			}

			// Pack positions to (-1, 1) range
			var hx = 0.0;
			var hy = 0.0;
			var hz = 0.0;
			for (i in 0...Std.int(posa32.length / 3)) {
				var f = Math.abs(posa32[i * 3]);
				if (hx < f) hx = f;
				f = Math.abs(posa32[i * 3 + 1]);
				if (hy < f) hy = f;
				f = Math.abs(posa32[i * 3 + 2]);
				if (hz < f) hz = f;
			}
			var scalePos = Math.max(hx, Math.max(hy, hz));
			var inv = 1 / scalePos;
			for (i in 0...Std.int(posa32.length / 3)) {
				posa[i * 4    ] = Std.int(posa32[i * 3    ] * 32767 * inv);
				posa[i * 4 + 1] = Std.int(posa32[i * 3 + 1] * 32767 * inv);
				posa[i * 4 + 2] = Std.int(posa32[i * 3 + 2] * 32767 * inv);
			}

			var name:String = m.get("id").get("name");
			name = name.substring(2, name.length);
			var obj = {posa: posa, nora: nora, texa: texa, inda: inda, name: name, scalePos: scalePos, scaleTes: 1.0};
			makeMesh(obj, path);
		});
	}

	function makeMesh(mesh:Dynamic, path:String) {
		if (mesh.posa == null || mesh.nora == null || mesh.inda == null) {
			showMessage("Error: Failed to read mesh data");
			return;
		}

		#if arm_editor
		var raw:TMeshData = {
			name: mesh.name,
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
			showMessage("Error: Mesh has no UVs, generating defaults");
			var verts = Std.int(mesh.posa.length / 4);
			mesh.texa = new kha.arrays.Int16Array(verts * 2);
			var n = new iron.math.Vec4();
			for (i in 0...verts) {
				n.set(mesh.posa[i * 4 + 0] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767).normalize();
				// Sphere projection
				// mesh.texa[i * 2 + 0] = Math.atan2(n.x, n.y) / (Math.PI * 2) + 0.5;
				// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
				// Equirect
				mesh.texa[i * 2    ] = Std.int(((Math.atan2(-n.z, n.x) + Math.PI) / (Math.PI * 2)) * 32767);
				mesh.texa[i * 2 + 1] = Std.int((Math.acos(n.y) / Math.PI) * 32767);
			}
		}
		var raw:TMeshData = {
			name: mesh.name,
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos" },
				{ values: mesh.nora, attrib: "nor" },
				{ values: mesh.texa, attrib: "tex" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			],
			scale_pos: mesh.scalePos,
			scale_tex: mesh.scaleTex
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

				selectPaintObject(mainObject());
				for (i in 1...paintObjects.length) {
					var p = paintObjects[i];
					iron.data.Data.deleteMesh(p.data.handle);
					p.remove();
				}
				iron.data.Data.deleteMesh(paintObject.data.handle);
				autoFillHandle.selected = false;

				while (layers.length > 1) { var l = layers.pop(); l.unload(); }
				selectedLayer = layers[0];
				UINodes.inst.parseMeshMaterial();
				UINodes.inst.parsePaintMaterial();
				iron.App.notifyOnRender(initLayers);
				if (paintHeight) iron.App.notifyOnRender(initHeightLayer);
				
				paintObject.setData(md);
				paintObject.name = mesh.name;

				// Face camera
				// paintObject.transform.setRotation(Math.PI / 2, 0, 0);

				paintObjects = [paintObject];
			}

			#if arm_editor
			}
			#end

			ddirty = 4;
			hwnd.redraws = 2;
			arm.UIView2D.inst.uvmapCached = false;
		});
	}

	function addMesh(mesh:Dynamic) {
		// #if arm_editor
		// #else
		// #end

		if (mesh.texa == null) {
			showMessage("Error: Mesh has no UVs, generating defaults");
			var verts = Std.int(mesh.posa.length / 4);
			mesh.texa = new kha.arrays.Int16Array(verts * 2);
			var n = new iron.math.Vec4();
			for (i in 0...verts) {
				n.set(mesh.posa[i * 4] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767).normalize();
				// Sphere projection
				// mesh.texa[i * 2 + 0] = Math.atan2(n.x, n.y) / (Math.PI * 2) + 0.5;
				// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
				// Equirect
				mesh.texa[i * 2    ] = Std.int(((Math.atan2(-n.z, n.x) + Math.PI) / (Math.PI * 2)) * 32767);
				mesh.texa[i * 2 + 1] = Std.int((Math.acos(n.y) / Math.PI) * 32767);
			}
		}
		var raw:TMeshData = {
			name: mesh.name,
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos" },
				{ values: mesh.nora, attrib: "nor" },
				{ values: mesh.texa, attrib: "tex" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			],
			scale_pos: mesh.scalePos,
			scale_tex: mesh.scaleTex
		};

		new MeshData(raw, function(md:MeshData) {
			
			var object = iron.Scene.active.addMeshObject(md, paintObject.materials, paintObject);
			object.name = mesh.name;
			object.skip_context = "paint";

			// iron.App.notifyOnRender(initLayers);
			// if (paintHeight) iron.App.notifyOnRender(initHeightLayer);
			
			// object.transform.scale.setFrom(paintObject.transform.scale);
			// object.transform.buildMatrix();

			paintObjects.push(object);

			ddirty = 4;
			hwnd.redraws = 2;
			arm.UIView2D.inst.uvmapCached = false;
		});
	}

	function projectOpen() {
		arm.App.showFiles = true;
		@:privateAccess zui.Ext.lastPath = ""; // Refresh
		arm.App.whandle.redraws = 2;
		arm.App.foldersOnly = false;
		arm.App.showFilename = false;
		arm.App.filesDone = function(path:String) {
			if (!StringTools.endsWith(path, ".arm")) {
				showMessage(".arm file expected");
				return;
			}
			importProject(path);
		};
	}

	function exportProject() {
		var mnodes:Array<zui.Nodes.TNodeCanvas> = [];
		var bnodes:Array<zui.Nodes.TNodeCanvas> = [];

		for (m in materials) mnodes.push(UINodes.inst.canvasMap.get(m));
		for (b in brushes) bnodes.push(UINodes.inst.canvasBrushMap.get(b));

		var md:Array<TMeshData> = [];
		for (p in paintObjects) md.push(p.data.raw);

		var asset_files:Array<String> = [];
		for (a in assets) asset_files.push(a.file);

		var ld:Array<TLayerData> = [];
		for (l in layers) {
			ld.push({
				res: l.texpaint.width,
				texpaint: l.texpaint.getPixels(),
				texpaint_nor: l.texpaint_nor.getPixels(),
				texpaint_pack: l.texpaint_pack.getPixels(),
				texpaint_opt: l.texpaint_opt != null ? l.texpaint_opt.getPixels() : null,
			});
		}

		project = {
			version: version,
			material_nodes: mnodes,
			brush_nodes: bnodes,
			mesh_datas: md,
			layer_datas: ld,
			assets: asset_files
		};
		
		var bytes = iron.system.ArmPack.encode(project);

		#if kha_krom
		Krom.fileSaveBytes(projectPath, bytes.getData());
		#elseif kha_kore
		sys.io.File.saveBytes(projectPath, bytes);
		#end
	}

	function projectSave() {
		if (projectPath == "") {
			projectSaveAs();
			return;
		}
		kha.Window.get(0).title = arm.App.filenameHandle.text + " - ArmorPaint";
		projectExport = true;
	}

	function projectSaveAs() {
		arm.App.showFiles = true;
		@:privateAccess zui.Ext.lastPath = ""; // Refresh
		arm.App.whandle.redraws = 2;
		arm.App.foldersOnly = true;
		arm.App.showFilename = true;
		arm.App.filesDone = function(path:String) {
			var f = arm.App.filenameHandle.text;
			if (f == "") f = "untitled";
			projectPath = path + "/" + f;
			if (!StringTools.endsWith(projectPath, ".arm")) projectPath += ".arm";
			projectSave();
		};
	}

	function projectNew(resetLayers = true) {
		kha.Window.get(0).title = "ArmorPaint";
		projectPath = "";
		if (mergedObject != null) {
			mergedObject.remove();
			iron.data.Data.deleteMesh(mergedObject.data.handle);
			mergedObject = null;
		}
		selectPaintObject(mainObject());
		for (i in 1...paintObjects.length) {
			var p = paintObjects[i];
			iron.data.Data.deleteMesh(p.data.handle);
			p.remove();
		}
		var n = newObjectNames[newObject];
		iron.data.Data.deleteMesh(paintObject.data.handle);
		iron.data.Data.getMesh("mesh_" + n, n, function(md:MeshData) {
			autoFillHandle.selected = false;
			paintObject.setData(md);
			paintObject.transform.scale.set(1, 1, 1);
			paintObject.transform.buildMatrix();
			paintObject.name = n;
			paintObjects = [paintObject];
			maskHandle.position = 0;
			ui.g.end();
			materials = [new MaterialSlot()];
			selectedMaterial = materials[0];
			UINodes.inst.canvasMap = new Map();
			UINodes.inst.canvasBrushMap = new Map();
			brushes = [new BrushSlot()];
			selectedBrush = brushes[0];
			
			if (resetLayers) {
				// for (l in layers) l.unload();
				layers = [new LayerSlot()];
				selectedLayer = layers[0];
				iron.App.notifyOnRender(initLayers);
				if (paintHeight) iron.App.notifyOnRender(initHeightLayer);
			}
			
			UINodes.inst.updateCanvasMap();
			UINodes.inst.parsePaintMaterial();
			makeMaterialPreview();
			ui.g.begin(false);
			assets = [];
			assetNames = [];
			assetId = 0;

			resetViewport();
		});
	}

	public function importProject(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var resetLayers = false;
			projectNew(resetLayers);
			projectPath = path;
			arm.App.filenameHandle.text = new haxe.io.Path(projectPath).file;

			kha.Window.get(0).title = arm.App.filenameHandle.text + " - ArmorPaint";

			project = iron.system.ArmPack.decode(b.toBytes());

			for (file in project.assets) importAsset(file);

			materials = [];
			for (n in project.material_nodes) {
				var mat = new MaterialSlot();
				UINodes.inst.canvasMap.set(mat, n);
				materials.push(mat);

				selectedMaterial = mat;
				UINodes.inst.updateCanvasMap();
				UINodes.inst.parsePaintMaterial();
				makeMaterialPreview();
			}

			brushes = [];
			for (n in project.brush_nodes) {
				var brush = new BrushSlot();
				UINodes.inst.canvasBrushMap.set(brush, n);
				brushes.push(brush);
			}

			// Synchronous for now
			new MeshData(project.mesh_datas[0], function(md:MeshData) {
				paintObject.setData(md);
				paintObject.transform.scale.set(1, 1, 1);
				paintObject.transform.buildMatrix();
				paintObject.name = md.name;
				paintObjects = [paintObject];
			});

			for (i in 1...project.mesh_datas.length) {
				var raw = project.mesh_datas[i];  
				new MeshData(raw, function(md:MeshData) {
					var object = iron.Scene.active.addMeshObject(md, paintObject.materials, paintObject);
					object.name = md.name;
					object.skip_context = "paint";
					paintObjects.push(object);					
				});
			}

			// No mask by default
			if (mergedObject == null) mergeMesh();
			selectPaintObject(mainObject());
			scaleToBounds();
			paintObject.skip_context = "paint";
			mergedObject.visible = true;

			resHandle.position = getTextureResPos(project.layer_datas[0].res);

			if (undoLayers[0].texpaint.width != getTextureRes()) {
				for (l in undoLayers) resizeLayer(l); // TODO
				for (l in layers) resizeLayer(l);
			}

			// for (l in layers) l.unload();
			layers = [];
			for (i in 0...project.layer_datas.length) {
				var ld = project.layer_datas[i];
				var l = new LayerSlot();
				layers.push(l);

				// TODO: create render target from bytes
				var texpaint = kha.Image.fromBytes(ld.texpaint, ld.res, ld.res);
				l.texpaint.g2.begin(false);
				l.texpaint.g2.drawImage(texpaint, 0, 0);
				l.texpaint.g2.end();
				// texpaint.unload();

				var texpaint_nor = kha.Image.fromBytes(ld.texpaint_nor, ld.res, ld.res);
				l.texpaint_nor.g2.begin(false);
				l.texpaint_nor.g2.drawImage(texpaint_nor, 0, 0);
				l.texpaint_nor.g2.end();
				// texpaint_nor.unload();

				var texpaint_pack = kha.Image.fromBytes(ld.texpaint_pack, ld.res, ld.res);
				l.texpaint_pack.g2.begin(false);
				l.texpaint_pack.g2.drawImage(texpaint_pack, 0, 0);
				l.texpaint_pack.g2.end();
				// texpaint_pack.unload();

				if (ld.texpaint_opt != null) {
					var texpaint_opt = kha.Image.fromBytes(ld.texpaint_opt, ld.res, ld.res);
					l.texpaint_opt.g2.begin(false);
					l.texpaint_opt.g2.drawImage(texpaint_opt, 0, 0);
					l.texpaint_opt.g2.end();
					// texpaint_opt.unload();
				}
			}
			selectedLayer = layers[0];

			if (layers.length > 0) UINodes.inst.parseMeshMaterial();

			ddirty = 4;
			hwnd.redraws = 2;
		});
	}

	function mergeMesh() {
		var vlen = 0;
		var ilen = 0;
		var maxScale = 0.0;
		for (i in 0...paintObjects.length) {
			vlen += paintObjects[i].data.raw.vertex_arrays[0].values.length;
			ilen += paintObjects[i].data.raw.index_arrays[0].values.length;
			if (paintObjects[i].data.scalePos > maxScale) maxScale = paintObjects[i].data.scalePos;
		}
		vlen = Std.int(vlen / 4);
		var va0 = new kha.arrays.Int16Array(vlen * 4);
		var va1 = new kha.arrays.Int16Array(vlen * 2);
		var va2 = new kha.arrays.Int16Array(vlen * 2);
		var ia = new kha.arrays.Uint32Array(ilen);

		var voff = 0;
		var ioff = 0;
		for (i in 0...paintObjects.length) {
			var vas = paintObjects[i].data.raw.vertex_arrays;
			var ias = paintObjects[i].data.raw.index_arrays;
			var scale = paintObjects[i].data.scalePos;	

			for (j in 0...vas[0].values.length) va0[j + voff * 4] = vas[0].values[j];
			for (j in 0...Std.int(va0.length / 4)) {
				va0[j * 4     + voff * 4] = Std.int((va0[j * 4     + voff * 4] * scale) / maxScale);
				va0[j * 4 + 1 + voff * 4] = Std.int((va0[j * 4 + 1 + voff * 4] * scale) / maxScale);
				va0[j * 4 + 2 + voff * 4] = Std.int((va0[j * 4 + 2 + voff * 4] * scale) / maxScale);
			}
			for (j in 0...vas[1].values.length) va1[j + voff * 2] = vas[1].values[j];
			for (j in 0...vas[2].values.length) va2[j + voff * 2] = vas[2].values[j];
			for (j in 0...ias[0].values.length) ia[j + ioff] = ias[0].values[j] + voff;

			voff += Std.int(vas[0].values.length / 4);
			ioff += Std.int(ias[0].values.length);
		}

		var raw:TMeshData = {
			name: paintObject.name,
			vertex_arrays: [
				{ values: va0, attrib: "pos" },
				{ values: va1, attrib: "nor" },
				{ values: va2, attrib: "tex" }
			],
			index_arrays: [
				{ values: ia, material: 0 }
			],
			scale_pos: maxScale,
			scale_tex: 1.0
		};

		new MeshData(raw, function(md:MeshData) {
			mergedObject = new MeshObject(md, paintObject.materials);
			mergedObject.name = paintObject.name;
			mergedObject.force_context = "paint";
			mainObject().addChild(mergedObject);
		});
	}

	function resetViewport() {
		var scene = iron.Scene.active;
		var cam = scene.cameras[0];
		for (o in scene.raw.objects) {
			if (o.type == 'camera_object') {
				cam.transform.local.setF32(o.transform.values);
				cam.transform.decompose();
				if (fovHandle != null) fovHandle.value = 0.92;
				camHandle.position = 0;
				cam.data.raw.ortho = null;
				if (originalShadowBias > 0) {
					iron.Scene.active.lights[0].data.raw.shadows_bias = originalShadowBias;
				}
				cam.buildProjection();
				selectedObject.transform.reset();
				ddirty = 2;
				break;
			}
		}
	}

	function switchUpAxis(axisUp:Int) {
		for (p in paintObjects) {
			var g = p.data.geom;

			// position, normals

			var vertices = g.vertexBuffer.lock(); // posnortex
			var verticesDepth = g.vertexBufferMap.get("pos").lock();
			if (!g.vertexBufferMap.exists("posnor")) g.get([{name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}]);
			var verticesVox = g.vertexBufferMap.get("posnor").lock();
			if (axisUp == 1) { // Y
				for (i in 0...Std.int(vertices.length / g.structLength)) {
					var f = vertices[i * g.structLength + 1];
					vertices[i * g.structLength + 1] = vertices[i * g.structLength + 2];
					vertices[i * g.structLength + 2] = -f;
					f = vertices[i * g.structLength + 4];
					vertices[i * g.structLength + 4] = vertices[i * g.structLength + 5];
					vertices[i * g.structLength + 5] = -f;

					f = verticesDepth[i * 3 + 1];
					verticesDepth[i * 3 + 1] = verticesDepth[i * 3 + 2];
					verticesDepth[i * 3 + 2] = -f;

					f = verticesVox[i * 6 + 1];
					verticesVox[i * 6 + 1] = verticesVox[i * 6 + 2];
					verticesVox[i * 6 + 2] = -f;
				}
			}
			else { // Z
				for (i in 0...Std.int(vertices.length / g.structLength)) {
					var f = vertices[i * g.structLength + 1];
					vertices[i * g.structLength + 1] = -vertices[i * g.structLength + 2];
					vertices[i * g.structLength + 2] = f;
					f = vertices[i * g.structLength + 4];
					vertices[i * g.structLength + 4] = -vertices[i * g.structLength + 5];
					vertices[i * g.structLength + 5] = f;

					f = verticesDepth[i * 3 + 1];
					verticesDepth[i * 3 + 1] = -verticesDepth[i * 3 + 2];
					verticesDepth[i * 3 + 2] = f;

					f = verticesVox[i * 6 + 1];
					verticesVox[i * 6 + 1] = -verticesVox[i * 6 + 2];
					verticesVox[i * 6 + 2] = f;
				}
			}
			g.vertexBuffer.unlock();
			g.vertexBufferMap.get("pos").unlock();
			g.vertexBufferMap.get("posnor").unlock();
		}
	}

	function applyConfig() {
		C.rp_ssgi = hssgi.selected;
		C.rp_ssr = hssr.selected;
		C.rp_bloom = hbloom.selected;
		var wasOff = C.rp_shadowmap == 1;
		C.rp_shadowmap = getShadowMapSize(hshadowmap.position);
		var light = iron.Scene.active.lights[0];
		if (C.rp_shadowmap == 1) {
			light.data.raw.strength = 0;
		}
		else if (wasOff) {
			light.data.raw.strength = 6.5;
		}
		C.rp_supersample = getSuperSampleSize(hsupersample.position);
		ui.g.end();
		armory.data.Config.save();
		armory.renderpath.RenderPathCreator.applyConfig();
		ui.g.begin(false);
		ddirty = 2;
	}

	function exportTextures(path:String) {
		
		var textureSize = getTextureRes();

		var f = arm.App.filenameHandle.text;
		if (f == "") f = "untitled";

		var ext = formatType == 0 ? ".jpg" : formatType == 1 ? ".png" : ".tga";
		var bo = new haxe.io.BytesOutput();
		
		var pixels = selectedLayer.texpaint.getPixels(); // bgra
		if (isBaseSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				// pixels.set(i * 4 + 3, 255);
			}
		}
		if (formatType == 0) {
			var jpgdata:iron.format.jpg.Data.Data = {
				width: textureSize,
				height: textureSize,
				quality: formatQuality,
				pixels: pixels
			};
			var jpgwriter = new iron.format.jpg.Writer(bo);
			jpgwriter.write(jpgdata, 1);
		}
		else {
			var pngwriter = new iron.format.png.Writer(bo);
			pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
		}
		#if kha_krom
		if (isBase) Krom.fileSaveBytes(path + "/" + f + "_base" + ext, bo.getBytes().getData());
		#end

		pixels = selectedLayer.texpaint_nor.getPixels();
		if (isNorSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				// pixels.set(i * 4 + 3, 255);
			}
		}
		bo = new haxe.io.BytesOutput();
		if (formatType == 0) {
			var jpgdata:iron.format.jpg.Data.Data = {
				width: textureSize,
				height: textureSize,
				quality: formatQuality,
				pixels: pixels
			};
			var jpgwriter = new iron.format.jpg.Writer(bo);
			jpgwriter.write(jpgdata, 1);
		}
		else {
			var pngwriter = new iron.format.png.Writer(bo);
			pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
		}
		#if kha_krom
		if (isNor) Krom.fileSaveBytes(path + "/" + f + "_nor" + ext, bo.getBytes().getData());
		#end

		pixels = selectedLayer.texpaint_pack.getPixels(); // occ, rough, met

		if (isOccSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
			}
		}
		if (isRoughSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
			}
		}
		if (isMetSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
			}
		}

		if (outputType == 0) {
			bo = new haxe.io.BytesOutput();
			if (formatType == 0) {
				var jpgdata:iron.format.jpg.Data.Data = {
					width: textureSize,
					height: textureSize,
					quality: formatQuality,
					pixels: pixels
				};
				var jpgwriter = new iron.format.jpg.Writer(bo);
				jpgwriter.write(jpgdata, 2, 0);
			}
			else {
				var pngwriter = new iron.format.png.Writer(bo);
				pngwriter.write(iron.format.png.Tools.build32RGBA_(textureSize, textureSize, pixels, 0));
			}
			#if kha_krom
			if (isOcc) Krom.fileSaveBytes(path + "/" + f + "_occ" + ext, bo.getBytes().getData());
			#end

			bo = new haxe.io.BytesOutput();
			if (formatType == 0) {
				var jpgdata:iron.format.jpg.Data.Data = {
					width: textureSize,
					height: textureSize,
					quality: formatQuality,
					pixels: pixels
				};
				var jpgwriter = new iron.format.jpg.Writer(bo);
				jpgwriter.write(jpgdata, 2, 1);
			}
			else {
				var pngwriter = new iron.format.png.Writer(bo);
				pngwriter.write(iron.format.png.Tools.build32RGBA_(textureSize, textureSize, pixels, 1));
			}
			#if kha_krom
			if (isRough) Krom.fileSaveBytes(path + "/" + f + "_rough" + ext, bo.getBytes().getData());
			#end
			
			bo = new haxe.io.BytesOutput();
			if (formatType == 0) {
				var jpgdata:iron.format.jpg.Data.Data = {
					width: textureSize,
					height: textureSize,
					quality: formatQuality,
					pixels: pixels
				};
				var jpgwriter = new iron.format.jpg.Writer(bo);
				jpgwriter.write(jpgdata, 2, 2);
			}
			else {
				var pngwriter = new iron.format.png.Writer(bo);
				pngwriter.write(iron.format.png.Tools.build32RGBA_(textureSize, textureSize, pixels, 2));
			}
			#if kha_krom
			if (isMet) Krom.fileSaveBytes(path + "/" + f + "_met" + ext, bo.getBytes().getData());
			#end
		}
		else { // UE4
			bo = new haxe.io.BytesOutput();
			if (formatType == 0) {
				var jpgdata:iron.format.jpg.Data.Data = {
					width: textureSize,
					height: textureSize,
					quality: formatQuality,
					pixels: pixels
				};
				var jpgwriter = new iron.format.jpg.Writer(bo);
				jpgwriter.write(jpgdata, 1);
			}
			else {
				var pngwriter = new iron.format.png.Writer(bo);
				pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
			}
			#if kha_krom
			if (isOcc) Krom.fileSaveBytes(path + "/" + f + "_orm" + ext, bo.getBytes().getData());
			#end
		}

		if (isHeight && selectedLayer.texpaint_opt != null) {
			pixels = selectedLayer.texpaint_opt.getPixels();
			if (isHeightSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
					// pixels.set(i * 4 + 3, 255);
				}
			}
			bo = new haxe.io.BytesOutput();
			if (formatType == 0) {
				var jpgdata:iron.format.jpg.Data.Data = {
					width: textureSize,
					height: textureSize,
					quality: formatQuality,
					pixels: pixels
				};
				var jpgwriter = new iron.format.jpg.Writer(bo);
				jpgwriter.write(jpgdata, 1);
			}
			else {
				var pngwriter = new iron.format.png.Writer(bo);
				pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
			}
			#if kha_krom
			Krom.fileSaveBytes(path + "/" + f + "_height" + ext, bo.getBytes().getData());
			#end
		}

		// if (isOpac) Krom.fileSaveBytes(path + "/tex_opac" + ext, bo.getBytes().getData());
		// if (isEmis) Krom.fileSaveBytes(path + "/tex_emis" + ext, bo.getBytes().getData());
		// if (isSubs) Krom.fileSaveBytes(path + "/tex_subs" + ext, bo.getBytes().getData());
	}
}
