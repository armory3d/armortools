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

	public var version = "0.6";

	public var project:TProjectFormat;
	public var projectPath = "";

	public var bundled:Map<String, kha.Image> = new Map();
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
	public var savedCamera = Mat4.identity();
	var message = "";
	var messageTimer = 0.0;

	public var savedEnvmap:kha.Image = null;
	public var emptyEnvmap:kha.Image = null;
	public var previewEnvmap:kha.Image = null;
	public var showEnvmap = false;
	public var drawWireframe = false;
	public var culling = true;

	public var ddirty = 0;
	public var pdirty = 0;
	public var rdirty = 0;

	public var windowW = 280; // Panel width
	public var toolbarw = 54;
	public var headerh = 24;
	public var menubarw = 215;

	public var ui:Zui;
	public var systemId = "";
	public var colorIdHandle = Id.handle();

	public var formatType = 0;
	public var formatQuality = 80.0;
	public var outputType = 0;
	public var isBase = true;
	public var isBaseSpace = 0;
	public var isOpac = true;
	public var isOpacSpace = 0;
	public var isOcc = true;
	public var isOccSpace = 0;
	public var isRough = true;
	public var isRoughSpace = 0;
	public var isMet = true;
	public var isMetSpace = 0;
	public var isNor = true;
	public var isNorSpace = 0;
	public var isHeight = true;
	public var isHeightSpace = 0;
	public var hwnd = Id.handle();
	public var materials:Array<MaterialSlot> = null;
	public var selectedMaterial:MaterialSlot;
	#if arm_editor
	public var materials2:Array<MaterialSlot> = null;
	public var selectedMaterial2:MaterialSlot;
	#end
	public var brushes:Array<BrushSlot> = null;
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
	public var lastPaintX = -1.0;
	public var lastPaintY = -1.0;
	public var painted = 0;
	public var brushTime = 0.0;

	public var selectedObject:iron.object.Object;
	public var paintObject:iron.object.MeshObject;
	public var paintObjects:Array<iron.object.MeshObject> = null;
	public var gizmo:iron.object.Object = null;
	var gizmoX:iron.object.Object = null;
	var gizmoY:iron.object.Object = null;
	var gizmoZ:iron.object.Object = null;
	public var grid:iron.object.Object = null;
	public var selectedType = "";
	var axisX = false;
	var axisY = false;
	var axisZ = false;
	var axisStart = 0.0;
	var row4 = [1/4, 1/4, 1/4, 1/4];
	public var pipe:kha.graphics4.PipelineState = null;

	public var brushNodesRadius = 1.0;
	public var brushNodesOpacity = 1.0;
	public var brushNodesScale = 1.0;
	public var brushNodesStrength = 1.0;

	public var brushRadius = 0.5;
	public var brushRadiusHandle = new Zui.Handle({value: 0.5});
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
	public var fillTypeHandle = new Zui.Handle();
	public var resHandle = new Zui.Handle({position: 1}); // 2048
	public var objectsHandle = new Zui.Handle({selected: true});
	public var maskHandle = new Zui.Handle({position: 0});
	public var mergedObject:MeshObject = null; // For object mask
	var newConfirm = false;
	public var newObject = 0;
	public var newObjectNames = ["Cube", "Plane", "Sphere", "Cylinder"];

	public var sub = 0;
	public var vec2 = new iron.math.Vec4();

	public var lastPaintVecX = -1.0;
	public var lastPaintVecY = -1.0;
	var frame = 0;

	public var C:TAPConfig;
	var lastBrushType = -1;
	var altStartedX = -1.0;
	var altStartedY = -1.0;
	public var cameraType = 0;
	// public var originalShadowBias = 0.0;
	public var camHandle = new Zui.Handle({position: 0});
	public var fovHandle:Zui.Handle = null;
	public var undoHandle:Zui.Handle = null;
	public var hssgi:Zui.Handle = null;
	public var hssr:Zui.Handle = null;
	public var hbloom:Zui.Handle = null;
	public var hshadowmap:Zui.Handle = null;
	public var hsupersample:Zui.Handle = null;
	var textureExport = false;
	var textureExportPath = "";
	public var projectExport = false;
	var headerHandle = new Zui.Handle({layout:Horizontal});
	var toolbarHandle = new Zui.Handle();
	var statusHandle = new Zui.Handle({layout:Horizontal});
	var drawMenu = false;
	var menuCategory = 0;

	#if arm_editor
	public var cameraControls = 2;
	public var htab = Id.handle({position: 1});
	#else
	public var cameraControls = 0;
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

	public function new() {
		super();

		inst = this;
		systemId = kha.System.systemId;

		// Init config
		C = cast armory.data.Config.raw;
		if (C.ui_layout == null) C.ui_layout = 0;
		if (C.undo_steps == null) C.undo_steps = 4; // Max steps to keep

		windowW = Std.int(defaultWindowW * C.window_scale);
		toolbarw = Std.int(54 * C.window_scale);
		headerh = Std.int(24 * C.window_scale);
		menubarw = Std.int(215 * C.window_scale);

		Uniforms.init();

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
			b.set(0, 3);
			b.set(1, 3);
			b.set(2, 3);
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
				RenderUtil.makeMaterialPreview();
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

	public function showMessage(s:String) {
		messageTimer = 8.0;
		message = s;
		statusHandle.redraws = 2;
	}

	function done() {
		#if arm_editor
		grid = iron.Scene.active.getChild(".Grid");
		gizmo = iron.Scene.active.getChild(".GizmoTranslate");
		gizmoX = iron.Scene.active.getChild("GizmoX");
		gizmoY = iron.Scene.active.getChild("GizmoY");
		gizmoZ = iron.Scene.active.getChild("GizmoZ");
		var light = iron.Scene.active.getChild("Light");
		// light.addTrait(new armory.trait.physics.RigidBody(0, 0));
		#end

		selectedObject = iron.Scene.active.getChild("Cube");
		paintObject = cast (selectedObject, MeshObject);
		paintObjects = [paintObject];

		iron.App.notifyOnUpdate(update);
		iron.App.notifyOnRender2D(render);
		iron.App.notifyOnRender(Layers.initLayers);

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
	}

	function update() {
		if (textureExport) {
			textureExport = false;
			Exporter.exportTextures(textureExportPath);
		}
		if (projectExport) {
			projectExport = false;
			Project.exportProject();
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
		if (kb.started("1") && (shift || ctrl)) shift ? setBrushType(0) : selectMaterial(0);
		else if (kb.started("2") && (shift || ctrl)) shift ? setBrushType(1) : selectMaterial(1);
		else if (kb.started("3") && (shift || ctrl)) shift ? setBrushType(2) : selectMaterial(2);
		else if (kb.started("4") && (shift || ctrl)) shift ? setBrushType(3) : selectMaterial(3);
		else if (kb.started("5") && (shift || ctrl)) shift ? setBrushType(4) : selectMaterial(4);

		if (ctrl && !shift && kb.started("s")) Project.projectSave();
		else if (ctrl && shift && kb.started("s")) Project.projectSaveAs();
		else if (ctrl && kb.started("o")) Project.projectOpen();

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

		// Viewport shortcuts
		var mouse = iron.system.Input.getMouse();
		if (mouse.x > 0 && mouse.x < iron.App.w() &&
			mouse.y > 0 && mouse.y < iron.App.h()) {

			// Color pick shortcut
			if (kb.started("alt")) {
				altStartedX = mouse.x;
				altStartedY = mouse.y;
			}
			else if (kb.released("alt") && altStartedX == mouse.x && altStartedY == mouse.y) {
				if (lastBrushType == -1) {
					lastBrushType = brushType;
					setBrushType(4);
				}
				else {
					setBrushType(lastBrushType);
					lastBrushType = -1;
				}
				altStartedX = -1.0;
				altStartedY = -1.0;
			}

			// Radius
			if (kb.started("-")) {
				if (brushRadius > 0.1) {
					brushRadius = Math.round((brushRadius - 0.1) * 100) / 100;
					brushRadiusHandle.value = brushRadius;
					headerHandle.redraws = 2;
				}
			}
			else if (kb.started("+")) {
				if (brushRadius < 2.0) {
					brushRadius = Math.round((brushRadius + 0.1) * 100) / 100;
					brushRadiusHandle.value = brushRadius;
					headerHandle.redraws = 2;
				}
			}

			// Viewpoint
			if (kb.started("0")) {
				ViewportUtil.resetViewport();
				ViewportUtil.scaleToBounds();
			}
			else if (kb.started("1")) {
				ViewportUtil.setView(0, -3, 0, Math.PI / 2, 0, 0);
			}
			else if (kb.started("3")) {
				ViewportUtil.setView(3, 0, 0, Math.PI / 2, 0, Math.PI / 2);
			}
			else if (kb.started("7")) {
				ViewportUtil.setView(0, 0, 3, 0, 0, 0);
			}
		}

		for (p in Plugin.plugins) if (p.update != null) p.update();
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

		if (messageTimer > 0) {
			messageTimer -= iron.system.Time.delta;
			if (messageTimer <= 0) statusHandle.redraws = 2;
		}

		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

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

					object.addTrait(new armory.trait.physics.RigidBody(0, 0));

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

					object.addTrait(new armory.trait.physics.RigidBody(0, 0));

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

	function render(g:kha.graphics2.Graphics) {
		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		renderUI(g);
		renderMenu(g);

		// var ready = showFiles || dirty;
		// TODO: Texture params get overwritten
		// if (ready) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
		// if (UINodes.inst._matcon != null) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
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
		headerHandle.redraws = 2;
		toolbarHandle.redraws = 2;
		ddirty = 2;
	}

	public function selectObject(o:iron.object.Object) {
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

	public function selectPaintObject(o:iron.object.MeshObject) {
		autoFillHandle.selected = false; // Auto-disable
		for (p in paintObjects) p.skip_context = "paint";
		paintObject = o;
		if (mergedObject == null || maskHandle.position == 1) { // Single object or object mask set to none
			paintObject.skip_context = "";
		}
		UIView2D.inst.uvmapCached = false;
		UIView2D.inst.trianglemapCached = false;
	}

	public function getImage(asset:TAsset):kha.Image {
		return Canvas.assetMap.get(asset.id);
	}

	public function mainObject():MeshObject {
		for (po in paintObjects) if (po.children.length > 0) return po;
		return paintObjects[0];
	}

	// var cursorImg:kha.Image = null;

	function renderUI(g:kha.graphics2.Graphics) {
		
		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();
		
		g.color = 0xffffffff;

		// if (cursorImg == null) {
		// 	g.end();
		// 	cursorImg = kha.Image.createRenderTarget(256, 256);
		// 	cursorImg.g2.begin(true, 0x00000000);
		// 	cursorImg.g2.color = 0xffcccccc;
		// 	kha.graphics2.GraphicsExtension.drawCircle(cursorImg.g2, 128, 128, 124, 8);
		// 	cursorImg.g2.end();
		// 	g.begin(false);
		// }

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

		// ui.t.FILL_WINDOW_BG = false;
		var panelx = (iron.App.x() - toolbarw);
		if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(toolbarHandle, panelx, headerh, toolbarw, kha.System.windowHeight())) {
			ui._y += 2;

			ui.imageScrollAlign = false;
			var img1 = bundled.get("brush_draw.png");
			var img2 = bundled.get("brush_erase.png");
			var img3 = bundled.get("brush_fill.png");
			var img4 = bundled.get("brush_bake.png");
			var img5 = bundled.get("brush_colorid.png");
			var tool = "";
			
			ui._x += 2;
			if (brushType == 0) ui.rect(-1, -1, img1.width + 2, img1.height + 2, 0xff205d9c, 2);
			if (ui.image(img1) == State.Started) setBrushType(0);
			if (ui.isHovered) ui.tooltip("Draw");
			ui._x -= 2;
			ui._y += 2;
			
			ui._x += 2;
			if (brushType == 1) ui.rect(-1, -1, img1.width + 2, img1.height + 2, 0xff205d9c, 2);
			if (ui.image(img2) == State.Started) setBrushType(1);
			if (ui.isHovered) ui.tooltip("Erase");
			ui._x -= 2;
			ui._y += 2;

			ui._x += 2;
			if (brushType == 2) ui.rect(-1, -1, img1.width + 2, img1.height + 2, 0xff205d9c, 2);
			if (ui.image(img3) == State.Started) setBrushType(2);
			if (ui.isHovered) ui.tooltip("Fill");
			ui._x -= 2;
			ui._y += 2;

			ui._x += 2;
			if (brushType == 3) ui.rect(-1, -1, img1.width + 2, img1.height + 2, 0xff205d9c, 2);
			if (ui.image(img4) == State.Started) setBrushType(3);
			if (ui.isHovered) ui.tooltip("Bake");
			ui._x -= 2;
			ui._y += 2;

			ui._x += 2;
			if (brushType == 4) ui.rect(-1, -1, img1.width + 2, img1.height + 2, 0xff205d9c, 2);
			if (ui.image(img5) == State.Started) setBrushType(4);
			if (ui.isHovered) ui.tooltip("Color ID");
			ui.imageScrollAlign = true;
			ui._x -= 2;
			ui._y += 2;
		}
		// ui.t.FILL_WINDOW_BG = true;

		var panelx = iron.App.x() - toolbarw;
		if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(Id.handle({layout:Horizontal}), panelx, 0, menubarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			var _w = ui._w;
			ui._w = Std.int(ui._w * 0.5);
			ui._x += 1; // Prevent "File" button highlight on startup
			
			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;

			if (ui.button("File", Left) || (drawMenu && ui.isHovered)) { drawMenu = true; menuCategory = 0; };
			if (ui.button("Edit", Left) || (drawMenu && ui.isHovered)) { drawMenu = true; menuCategory = 1; };
			if (ui.button("View", Left) || (drawMenu && ui.isHovered)) { drawMenu = true; menuCategory = 2; };
			if (ui.button("Help", Left) || (drawMenu && ui.isHovered)) { drawMenu = true; menuCategory = 3; };
			
			ui._w = _w;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.BUTTON_COL = BUTTON_COL;
		}

		var panelx = (iron.App.x() - toolbarw) + menubarw;
		if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(Id.handle({layout:Horizontal}), panelx, 0, arm.App.realw() - windowW - menubarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			ui.tab(Id.handle(), "Paint");
			ui.tab(Id.handle(), "Sculpt");
			ui.tab(Id.handle(), "Material");
			ui.tab(Id.handle(), "Scene");
			ui.tab(Id.handle(), "Render");
		}

		var panelx = iron.App.x();
		if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(headerHandle, panelx, headerh, arm.App.realw() - toolbarw - windowW, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {

			// Color ID
			if (brushType == 4) {
				// Picked color
				ui.text("Picked Color");
				if (colorIdPicked) {
					ui.image(iron.RenderPath.active.renderTargets.get("texpaint_colorid0").image, 0xffffffff, 64);
				}
				if (ui.button("Clear")) colorIdPicked = false;
				// Set color map
				ui.text("Color ID Map");
				var cid = ui.combo(colorIdHandle, App.getEnumTexts(), "Color ID");
				if (UITrait.inst.assets.length > 0) ui.image(UITrait.inst.getImage(UITrait.inst.assets[cid]));
			}
			else if (brushType == 3) { // Bake AO
				ui.combo(Id.handle(), ["AO"], "Bake");
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
			else { // Draw, Erase, Fill
				brushRadius = ui.slider(brushRadiusHandle, "Radius", 0.0, 2.0, true);
				var brushScaleHandle = Id.handle({value: brushScale});
				brushScale = ui.slider(brushScaleHandle, "UV Scale", 0.0, 2.0, true);
				if (brushScaleHandle.changed && autoFillHandle.selected) UINodes.inst.parsePaintMaterial();

				brushOpacity = ui.slider(Id.handle({value: brushOpacity}), "Opacity", 0.0, 1.0, true);
				brushStrength = ui.slider(Id.handle({value: brushStrength}), "Strength", 0.0, 1.0, true);
				brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 1.0, true);

				ui.combo(Id.handle(), ["Add"], "Blending");

				var paintHandle = Id.handle();
				brushPaint = ui.combo(paintHandle, ["UV", "Project", "Sticker"], "Paint");
				if (paintHandle.changed) {
					UINodes.inst.parsePaintMaterial();
					if (brushPaint == 2) { // Sticker
						ui.g.end();
						RenderUtil.makeStickerPreview();
						ui.g.begin(false);
					}
				}

				if (brushType == 2) { // Fill, Bake
					ui.combo(fillTypeHandle, ["Object", "Face"], "Fill");
					if (fillTypeHandle.changed) {
						if (fillTypeHandle.position == 1) {
							ui.g.end();
							UIView2D.inst.cacheTriangleMap();
							ui.g.begin(false);
						}
						UINodes.inst.parsePaintMaterial();
						UINodes.inst.parseMeshMaterial();
					}
					ui.check(autoFillHandle, "Auto-Fill");
					if (autoFillHandle.changed) {
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
					}
				}
				else { // Draw, Erase
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

		if (ui.window(statusHandle, iron.App.x(), arm.App.realh() - headerh, arm.App.realw() - toolbarw - windowW, headerh)) {

			var scene = iron.Scene.active;
			var cam = scene.cameras[0];
			cameraControls = ui.combo(Id.handle({position: cameraControls}), ["Rotate", "Orbit", "Fly"], "Controls");
			cameraType = ui.combo(camHandle, ["Perspective", "Orhographic"], "Type");
			if (camHandle.changed) {
				if (cameraType == 0) {
					cam.data.raw.ortho = null;
					cam.buildProjection();
				}
				else {
					var f32 = new kha.arrays.Float32Array(4);
					f32[0] = -2;
					f32[1] =  2;
					f32[2] = -2 * (iron.App.h() / iron.App.w());
					f32[3] =  2 * (iron.App.h() / iron.App.w());
					// cam.data.raw.ortho = f32; // See ViewportUtil.ortho
					// cam.buildProjection();

					cam.P = ViewportUtil.ortho(f32[0], f32[1], f32[2], f32[3], cam.data.raw.near_plane, cam.data.raw.far_plane);
					#if arm_taa
					cam.noJitterP.setFrom(cam.P);
					#end
				}
				
				ddirty = 2;
			}

			fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
			cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
			if (fovHandle.changed) {
				cam.buildProjection();
				ddirty = 2;
			}

			ui.t.FILL_BUTTON_BG = false;
			if (ui.button("Default")) {
				ViewportUtil.resetViewport();
				ViewportUtil.scaleToBounds();
			}
			ui.t.FILL_BUTTON_BG = true;

			if (messageTimer > 0) {
				var _w = ui._w;
				ui._w = Std.int(ui.ops.font.width(ui.fontSize, message) + 50 * ui.SCALE);
				ui.fill(0, 0, ui._w, ui._h, 0xffff0000);
				ui.text(message);
				ui._w = _w;
			}
		}
		
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
							selectedType = "(Light)";
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

						#if (kha_opengl || kha_webgl)
						ui.imageInvertY = true; // Material preview
						#end

						for (j in 0...5) {
							var i = j + row * 5;
							var img = i >= materials2.length ? empty : materials2[i].image;
							var tint = img == empty ? ui.t.WINDOW_BG_COL : 0xffffffff;

							if (selectedMaterial2 == materials2[i]) {
								// ui.fill(1, -2, img.width + 3, img.height + 3, 0xff205d9c); // TODO
								var off = row % 2 == 1 ? 1 : 0;
								var w = 51 - C.window_scale;
								ui.fill(1,          -2, w + 3,       2, 0xff205d9c);
								ui.fill(1,     w - off, w + 3, 2 + off, 0xff205d9c);
								ui.fill(1,          -2,     2,   w + 3, 0xff205d9c);
								ui.fill(w + 3,      -2,     2,   w + 4, 0xff205d9c);
							}

							if (ui.image(img, tint) == State.Started && img != empty) {
								if (selectedMaterial2 != materials2[i]) selectMaterial2(i);
								if (iron.system.Time.time() - selectTime < 0.3) showMaterialNodes();
								selectTime = iron.system.Time.time();
							}
							if (img != empty && ui.isHovered) ui.tooltipImage(img);
						}

						#if (kha_opengl || kha_webgl)
						ui.imageInvertY = false; // Material preview
						#end
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
							RenderUtil.makeMaterialPreview();
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

				if (ui.panel(Id.handle({selected: true}), "Brushes", 1)) {
					ui.row([1/2,1/2]);
					if (ui.button("New")) {}
					if (ui.button("Nodes")) showBrushNodes();
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
						iron.App.notifyOnRender(Layers.initHeightLayer);
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

						#if (kha_opengl || kha_webgl)
						ui.imageInvertY = true; // Material preview
						#end

						for (j in 0...5) {
							var i = j + row * 5;
							var img = i >= materials.length ? empty : materials[i].image;
							var tint = img == empty ? ui.t.WINDOW_BG_COL : 0xffffffff;

							if (selectedMaterial == materials[i]) {
								// ui.fill(1, -2, img.width + 3, img.height + 3, 0xff205d9c); // TODO
								var off = row % 2 == 1 ? 1 : 0;
								var w = 51 - C.window_scale;
								ui.fill(1,          -2, w + 3,       2, 0xff205d9c);
								ui.fill(1,     w - off, w + 3, 2 + off, 0xff205d9c);
								ui.fill(1,          -2,     2,   w + 3, 0xff205d9c);
								ui.fill(w + 3,      -2,     2,   w + 4, 0xff205d9c);
							}

							if (ui.image(img, tint) == State.Started && img != empty) {
								if (selectedMaterial != materials[i]) selectMaterial(i);
								if (iron.system.Time.time() - selectTime < 0.3) showMaterialNodes();
								selectTime = iron.system.Time.time();
							}
							if (img != empty && ui.isHovered) ui.tooltipImage(img);
						}

						#if (kha_opengl || kha_webgl)
						ui.imageInvertY = false; // Material preview
						#end
					}

					ui.row([1/2,1/2]);
					if (ui.button("New")) {
						ui.g.end();
						autoFillHandle.selected = false; // Auto-disable
						selectedMaterial = new MaterialSlot();
						materials.push(selectedMaterial);
						UINodes.inst.updateCanvasMap();
						UINodes.inst.parsePaintMaterial();
						RenderUtil.makeMaterialPreview();
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
								iron.App.notifyOnRender(Layers.applySelectedLayer);
							}
							if (ui.button("Delete")) {
								selectedLayer = layers[i];
								Layers.deleteSelectedLayer();
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
							iron.App.notifyOnRender(Layers.clearLastLayer);
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
							ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, 0xff205d9c);
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
									MeshUtil.mergeMesh();
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
					viewportMode = ui.combo(modeHandle, ["Render", "Base Color", "Normal", "Occlusion", "Roughness", "Metallic", "TexCoord"], "Mode");
					if (modeHandle.changed) {
						UINodes.inst.parseMeshMaterial();
						ddirty = 2;
					}
					if (iron.Scene.active.lights.length > 0) {
						var light = iron.Scene.active.lights[0];
						var lhandle = Id.handle();
						lhandle.value = light.data.raw.strength / 100;
						light.data.raw.strength = ui.slider(lhandle, "Light", 0.0, 4.0, true) * 100;
						if (lhandle.changed) ddirty = 2;
					}

					ui.row([1/2, 1/2]);
					var upHandle = Id.handle();
					var lastUp = upHandle.position;
					var axisUp = ui.combo(upHandle, ["Z", "Y"], "Up Axis", true);
					if (upHandle.changed && axisUp != lastUp) {
						MeshUtil.switchUpAxis(axisUp);
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

					var wireframeHandle = Id.handle({selected: drawWireframe});
					drawWireframe = ui.check(wireframeHandle, "Wireframe");
					if (wireframeHandle.changed) {
						UINodes.inst.parseMeshMaterial();
						ddirty = 2;
					}
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
						Importer.importFile(path);
					}
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Assets", 1)) {
					if (assets.length > 0) {
						var i = assets.length - 1;
						while (i >= 0) {
							
							// Align into 2 items per row
							if ((assets.length - 1 - i) % 2 == 0) {
								ui.separator();
								ui.separator();
								ui.row([1/2, 1/2]);
							}
							
							var asset = assets[i];
							if (ui.image(UITrait.inst.getImage(asset)) == State.Started) {
								arm.App.dragAsset = asset;
							}

							// ui.row([1/8, 7/8]);
							// var b = ui.button("X");
							// asset.name = ui.textInput(Id.handle().nest(asset.id, {text: asset.name}), "", Right);
							// assetNames[i] = asset.name;
							// if (b) {
							// 	iron.data.Data.deleteImage(asset.file);
							// 	Canvas.assetMap.remove(asset.id);
							// 	assets.splice(i, 1);
							// 	assetNames.splice(i, 1);
							// }

							i--;
						}

						// Fill in last odd spot
						if (assets.length % 2 == 1) {
							var empty = bundled.get("empty.jpg");
							ui.image(empty);
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
							Project.projectNew();
							ViewportUtil.scaleToBounds();
						}
					}
					else if (ui.button("New")) {
						newConfirm = true;
					}
					newObject = ui.combo(Id.handle(), newObjectNames, "Default Object");
					ui.row([1/3,1/3,1/3]);
					if (ui.button("Open")) Project.projectOpen();
					if (ui.button("Save")) Project.projectSave();
					if (ui.button("Save As")) Project.projectSaveAs();
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "Project Quality", 1)) {
					ui.combo(resHandle, ["1K", "2K", "4K", "8K", "16K", "20K"], "Res", true);
					if (resHandle.changed) {
						iron.App.notifyOnRender(Layers.resizeLayers);
						UIView2D.inst.uvmap = null;
						UIView2D.inst.uvmapCached = false;
						UIView2D.inst.trianglemap = null;
						UIView2D.inst.trianglemapCached = false;
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
						toolbarw = Std.int(54 * C.window_scale);
						headerh = Std.int(24 * C.window_scale);
						menubarw = Std.int(215 * C.window_scale);
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
				// hshadowmap = Id.handle({position: Config.getShadowQuality(C.rp_shadowmap_cascade)});
				hsupersample = Id.handle({position: Config.getSuperSampleQuality(C.rp_supersample)});
				ui.separator();
				if (ui.panel(Id.handle({selected: true}), "Viewport", 1)) {
					// ui.row([1/2, 1/2]);
					// ui.combo(hshadowmap, ["Ultra", "High", "Medium", "Low", "Off"], "Shadows", true);
					// if (hshadowmap.changed) Config.applyConfig();
					ui.combo(hsupersample, ["0.5x", "1.0x", "1.5x", "2.0x"], "Super Sample", true);
					if (hsupersample.changed) Config.applyConfig();
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
					if (hssgi.changed) Config.applyConfig();
					ui.check(hssr, "SSR");
					if (hssr.changed) Config.applyConfig();
					ui.check(hbloom, "Bloom");
					if (hbloom.changed) Config.applyConfig();
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
					ui.text("Next Object - Ctrl+Tab");
					ui.text("Auto-Fill - G");
					ui.text("Brush Radius - +/-");
					ui.text("Brush Ruler - Hold Shift");
					ui.text("View Default - 0");
					ui.text("View Front - 1");
					ui.text("View Right - 3");
					ui.text("View Top - 7");
				}

				ui.separator();
				if (ui.panel(Id.handle({selected: false}), "About", 1)) {
					ui.text("v" + version + " - " +  Macro.buildSha() + " - armorpaint.org");
					// ui.text(Macro.buildDate());
					var gapi = #if (kha_direct3d11) "Direct3D11" #else "OpenGL" #end;
					var renderer = #if (rp_renderer == "Deferred") "Deferred" #else "Forward" #end;
					ui.text(kha.System.systemId + " - " + gapi + " - " + renderer);
				}
			}
		}
		ui.end();
		g.begin(false);
	}

	function renderMenu(g:kha.graphics2.Graphics) {
		
		// Draw menu
		if (drawMenu) {

			var panelx = iron.App.x() - toolbarw;
			if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;

			var menuButtonW = Std.int(ui.ELEMENT_W() * 0.5);
			var px = panelx + menuButtonW * menuCategory;
			var py = headerh;
			var ph = 200 * ui.SCALE;
			
			g.color = ui.t.SEPARATOR_COL;
			var menuw = Std.int(ui.ELEMENT_W() * 1.5);
			g.fillRect(px, py, menuw, ph);

			ui.beginLayout(g, Std.int(px), Std.int(py), menuw);
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var ELEMENT_H = ui.t.ELEMENT_H;
			ui.t.ELEMENT_H = Std.int(24 * ui.SCALE);

			if (menuCategory == 0) {
				ui.button("New", Left);
				ui.button("Open...", Left);
				ui.button("Save", Left);
				ui.button("Save As...", Left);
				ui.button("Import Asset...", Left);
				ui.button("Export Textures...", Left);
				ui.button("Export Mesh...", Left);
				ui.button("Exit", Left);
			}
			else if (menuCategory == 1) {
				ui.button("Undo", Left);
				ui.button("Redo", Left);
				ui.button("Preferences...", Left);
			}
			else if (menuCategory == 2) {
				ui.button("Show Envmap", Left);
				ui.button("Show Grid", Left);
				ui.button("Wireframe", Left);
			}
			else if (menuCategory == 3) {
				ui.button("Manual...", Left);
				ui.button("About...", Left);
			}

			ui.t.BUTTON_COL = BUTTON_COL;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.ELEMENT_H = ELEMENT_H;
			ui.endLayout();

			var mouse = iron.system.Input.getMouse();
			var kb = iron.system.Input.getKeyboard();

			if (mouse.released()) {
				drawMenu = false;
			}
		}
	}
}
