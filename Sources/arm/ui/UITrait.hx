package arm.ui;

import haxe.io.Bytes;
import kha.Image;
import kha.System;
import zui.Zui;
import zui.Id;
import iron.data.Data;
import iron.data.MaterialData;
import iron.object.Object;
import iron.object.MeshObject;
import iron.math.Mat4;
import iron.math.Vec4;
import iron.system.Input;
import iron.system.Time;
import iron.RenderPath;
import iron.Scene;
import arm.nodes.MaterialParser;
import arm.util.MeshUtil;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.UVUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.io.Importer;
import arm.io.Exporter;
import arm.io.ExportArm;
import arm.Project;
import arm.Tool;

@:access(zui.Zui)
class UITrait {

	public var project:TProjectFormat;
	public var projectPath = "";

	public var assets:Array<zui.Canvas.TAsset> = [];
	public var assetNames:Array<String> = [];
	public var assetId = 0;

	public static var inst:UITrait;
	public static var defaultWindowW = 280;
	public static var defaultToolbarW = 54;
	public static var defaultHeaderH = 24;

	public static var penPressureRadius = true;
	public static var penPressureOpacity = false;
	public static var penPressureHardness = false;
	public var undoI = 0; // Undo layer
	public var undos = 0; // Undos available
	public var redos = 0; // Redos available
	public var pushUndo = false; // Store undo on next paint

	public var isScrolling = false;
	public var colorIdPicked = false;
	public var show = true;
	public var materialPreview = false; // Drawing material previews
	public var savedCamera = Mat4.identity();
	public var baseRPicked = 0.0;
	public var baseGPicked = 0.0;
	public var baseBPicked = 0.0;
	public var normalRPicked = 0.0;
	public var normalGPicked = 0.0;
	public var normalBPicked = 0.0;
	public var roughnessPicked = 0.0;
	public var metallicPicked = 0.0;
	public var occlusionPicked = 0.0;
	public var materialIdPicked = 0.0;
	public var pickerSelectMaterial = true;
	public var pickerMaskHandle = new Handle({position: 0});
	var message = "";
	var messageTimer = 0.0;
	var messageColor = 0x00000000;

	public var savedEnvmap:Image = null;
	public var emptyEnvmap:Image = null;
	public var previewEnvmap:Image = null;
	public var showEnvmap = false;
	public var showEnvmapHandle = new Handle({selected: false});
	public var showEnvmapBlur = false;
	public var showEnvmapBlurHandle = new Handle({selected: false});
	public var drawWireframe = false;
	public var wireframeHandle = new Handle({selected: false});
	public var culling = true;

	public var ddirty = 0;
	public var pdirty = 0;
	public var rdirty = 0;
	public var brushBlendDirty = true;
	public var layerPreviewDirty = true;
	public var layersPreviewDirty = false;

	public var windowW = 280; // Panel width
	public var toolbarw = 54;
	public var headerh = 24;
	public var menubarw = 215;
	public var tabx = 0;
	public var tabh = 0;

	public var ui:Zui;
	public var colorIdHandle = Id.handle();

	public var formatType = 0;
	public var formatQuality = 100.0;
	public var layersExport = 0;
	public var outputType = 0;
	public var isBase = true;
	public var isBaseSpace = 0;
	public var isOpac = false;
	public var isOpacSpace = 0;
	public var isOcc = true;
	public var isOccSpace = 0;
	public var isRough = true;
	public var isRoughSpace = 0;
	public var isMet = true;
	public var isMetSpace = 0;
	public var isNor = true;
	public var isNorSpace = 0;
	public var isEmis = false;
	public var isEmisSpace = 0;
	public var isHeight = false;
	public var isHeightSpace = 0;
	public var isSubs = false;
	public var isSubsSpace = 0;
	public var isUdim = false;
	public var parseTransform = false;
	public var hwnd = Id.handle();
	public var hwnd1 = Id.handle();
	public var hwnd2 = Id.handle();
	public var materials:Array<MaterialSlot> = null;
	public var selectedMaterial:MaterialSlot;
	public var materialsScene:Array<MaterialSlot> = null;
	public var selectedMaterialScene:MaterialSlot;
	public var selectedTexture:zui.Canvas.TAsset = null;
	public var brushes:Array<BrushSlot> = null;
	public var selectedBrush:BrushSlot;
	public var layers:Array<LayerSlot> = null;
	public var undoLayers:Array<LayerSlot> = null;
	public var selectedLayer:LayerSlot;
	public var selectedLayerIsMask = false; // Mask selected for active layer
	public var selectTime = 0.0;
	public var displaceStrength = 1.0;
	public var decalImage:Image = null;
	public var decalPreview = false;
	public var viewportMode = 0;
	public var hscaleWasChanged = false;
	public var exportMeshFormat = 0;

	public var textToolImage:Image = null;
	public var textToolText = "Text";
	public var textToolHandle = new Handle({position: 0});
	public var decalMaskImage:Image = null;
	public var decalMaskHandle = new Handle({position: 0});
	public var particleMaterial:MaterialData = null;

	public var layerFilter = 0;

	var _onBrush:Array<Int->Void> = [];

	public var paintVec = new Vec4();
	public var lastPaintX = -1.0;
	public var lastPaintY = -1.0;
	public var painted = 0;
	public var brushTime = 0.0;
	public var cloneStartX = -1.0;
	public var cloneStartY = -1.0;
	public var cloneDeltaX = 0.0;
	public var cloneDeltaY = 0.0;

	public var selectedObject:Object;
	public var paintObject:MeshObject;
	public var paintObjects:Array<MeshObject> = null;
	public var gizmo:Object = null;
	public var gizmoX:Object = null;
	public var gizmoY:Object = null;
	public var gizmoZ:Object = null;
	public var axisX = false;
	public var axisY = false;
	public var axisZ = false;
	public var axisStart = 0.0;
	public var row4 = [1/4, 1/4, 1/4, 1/4];

	public var brushNodesRadius = 1.0;
	public var brushNodesOpacity = 1.0;
	public var brushNodesScale = 1.0;
	public var brushNodesHardness = 1.0;

	public var brushRadius = 0.5;
	public var brushRadiusHandle = new Handle({value: 0.5});
	public var brushOpacity = 1.0;
	public var brushScale = 1.0;
	public var brushRot = 0.0;
	public var brushHardness = 0.8;
	public var brushBias = 1.0;
	public var brushPaint = 0;
	public var selectedTool = 0;
	public var brush3d = true;
	public var brushDepthReject = true;
	public var brushAngleReject = true;
	public var brushAngleRejectDot = 0.5;
	public var bakeType = 0;
	public var bakeStrength = 1.0;
	public var bakeRadius = 1.0;
	public var bakeOffset = 1.0;
	
	public var xray = false;
	public var symX = false;
	public var symY = false;
	public var symZ = false;
	public var showCompass = true;
	public var fillTypeHandle = new Handle();
	public var resHandle = new Handle({position: 1}); // 2048
	public var bitsHandle = new Handle({position: 0}); // 8bit
	public var mergedObject:MeshObject = null; // For object mask
	var newConfirm = false;
	public var projectType = 0; // paint, material
	public var projectObjects:Array<MeshObject>;

	public var sub = 0;
	public var vec2 = new Vec4();

	public var lastPaintVecX = -1.0;
	public var lastPaintVecY = -1.0;
	public var frame = 0;
	public var paint2d = false;

	var altStartedX = -1.0;
	var altStartedY = -1.0;
	public var lockStartedX = -1.0;
	public var lockStartedY = -1.0;
	public var brushLocked = false;
	var brushCanLock = false;
	var brushCanUnlock = false;
	public var cameraType = 0;
	public var camHandle = new Handle({position: 0});
	public var fovHandle:Handle = null;
	public var undoHandle:Handle = null;
	public var hssgi:Handle = null;
	public var hssr:Handle = null;
	public var hbloom:Handle = null;
	public var hsupersample:Handle = null;
	public var hvxao:Handle = null;
	public var textureExport = false;
	public var textureExportPath = "";
	public var projectExport = false;
	public var headerHandle = new Handle({layout:Horizontal});
	public var toolbarHandle = new Handle();
	public var statusHandle = new Handle({layout:Horizontal});
	public var menuHandle = new Handle({layout:Horizontal});
	public var workspaceHandle = new Handle({layout:Horizontal});
	var lastCombo:Handle = null;
	var lastTooltip:Image = null;

	public var cameraControls = 1;
	public var htab = Id.handle({position: 0});
	public var htab1 = Id.handle({position: 0});
	public var htab2 = Id.handle({position: 0});
	public var worktab = Id.handle({position: 0});
	public var toolNames = ["Brush", "Eraser", "Fill", "Decal", "Text", "Clone", "Blur", "Particle", "Bake", "ColorID", "Picker"];

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
		if (UITrait.inst.worktab.position == SpaceScene) return false;
		return pdirty > 0;
	}

	public function new() {
		inst = this;

		windowW = Std.int(defaultWindowW * App.C.window_scale);
		toolbarw = Std.int(defaultToolbarW * App.C.window_scale);
		headerh = Std.int(defaultHeaderH * App.C.window_scale);
		menubarw = Std.int(215 * App.C.window_scale);

		arm.render.Uniforms.init();

		if (materials == null) {
			materials = [];
			//
			Data.getMaterial("Scene", "Material", function(m:MaterialData) {
				materials.push(new MaterialSlot(m));
				selectedMaterial = materials[0];
			});
			//
			// materials.push(new MaterialSlot());
			// selectedMaterial = materials[0];
		}
		if (materialsScene == null) {
			materialsScene = [];
			Data.getMaterial("Scene", "Material2", function(m:MaterialData) {
				materialsScene.push(new MaterialSlot(m));
				selectedMaterialScene = materialsScene[0];
			});
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

		if (emptyEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 3);
			b.set(1, 3);
			b.set(2, 3);
			b.set(3, 255);
			emptyEnvmap = Image.fromBytes(b, 1, 1);
		}
		if (previewEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 0);
			b.set(1, 0);
			b.set(2, 0);
			b.set(3, 255);
			previewEnvmap = Image.fromBytes(b, 1, 1);
		}

		var world = Scene.active.world;
		if (savedEnvmap == null) {
			savedEnvmap = world.envmap;
		}
		world.envmap = UITrait.inst.showEnvmap ? UITrait.inst.savedEnvmap : UITrait.inst.emptyEnvmap;
		ddirty = 1;

		// Save last pos for continuos paint
		iron.App.notifyOnRender(function(g:kha.graphics4.Graphics) { //
			if (frame == 2) {
				RenderUtil.makeMaterialPreview();
				hwnd1.redraws = 2;
				MaterialParser.parseMeshMaterial();
				MaterialParser.parsePaintMaterial();
				ddirty = 0;
				if (undoLayers == null) {
					undoLayers = [];
					for (i in 0...App.C.undo_steps) {
						var l = new LayerSlot("_undo" + undoLayers.length);
						l.createMask(0, false);
						undoLayers.push(l);
					}
				}
			}
			else if (frame == 3) {
				ddirty = 1;
			}
			frame++;

			var m = Input.getMouse();
			if (m.down()) { //
				UITrait.inst.lastPaintVecX = UITrait.inst.paintVec.x; //
				UITrait.inst.lastPaintVecY = UITrait.inst.paintVec.y; //
			}//
			else {
				UITrait.inst.lastPaintVecX = m.x / iron.App.w();
				UITrait.inst.lastPaintVecY = m.y / iron.App.h();
			}
		});//

		var scale = App.C.window_scale;
		ui = new Zui( { theme: App.theme, font: App.font, scaleFactor: scale, color_wheel: App.color_wheel } );
		
		var resources = ['cursor.png', 'icons.png'];
		Res.load(resources, done);

		projectObjects = [];
		for (m in Scene.active.meshes) projectObjects.push(m);
	}

	public function showMessage(s:String) {
		messageTimer = 8.0;
		message = s;
		messageColor = 0x00000000;
		statusHandle.redraws = 2;
	}

	public function showError(s:String) {
		messageTimer = 8.0;
		message = s;
		messageColor = 0xffff0000;
		statusHandle.redraws = 2;
	}

	function done() {
		if (ui.SCALE > 1) setIconScale();
		//
		gizmo = Scene.active.getChild(".GizmoTranslate");
		gizmo.transform.scale.set(0.5, 0.5, 0.5);
		gizmo.transform.buildMatrix();
		gizmoX = Scene.active.getChild("GizmoX");
		gizmoY = Scene.active.getChild("GizmoY");
		gizmoZ = Scene.active.getChild("GizmoZ");
		//

		selectedObject = Scene.active.getChild("Cube");
		paintObject = cast(selectedObject, MeshObject);
		paintObjects = [paintObject];

		if (App.fileArg == "") {
			iron.App.notifyOnRender(Layers.initLayers);
		}

		// Init plugins
		Plugin.keep();
		if (App.C.plugins != null) {
			for (plugin in App.C.plugins) {
				Data.getBlob("plugins/" + plugin, function(blob:kha.Blob) {
					#if js
					untyped __js__("(1, eval)({0})", blob.toString());
					#end
				});
			}
		}
	}

	var checkArg = true;

	function update() {
		if (textureExport) {
			textureExport = false;
			Exporter.exportTextures(textureExportPath);
		}
		if (projectExport) {
			projectExport = false;
			ExportArm.runProject();
			if (App.saveAndQuit) System.stop();
		}

		isScrolling = ui.isScrolling;
		updateUI();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		var kb = Input.getKeyboard();
		var shift = kb.down("shift");
		var ctrl = kb.down("control");
		var alt = kb.down("alt");
		alt = false; // TODO: gets stuck after alt+tab

		if (!App.uibox.isTyping) {
			if (kb.started("escape")) {
				App.showFiles = false;
				App.showBox = false;
			}
			if (App.showFiles) {
				if (kb.started("enter")) {
					App.showFiles = false;
					App.filesDone(App.path);
					UITrait.inst.ddirty = 2;
				}
			}
		}

		if (!App.uienabled) return;

		if (!UINodes.inst.ui.isTyping && !UITrait.inst.ui.isTyping) {
			if (shortcut(App.K.cycle_layers)) {
				var i = (layers.indexOf(selectedLayer) + 1) % layers.length;
				setLayer(layers[i]);
			}
			else if (shortcut(App.K.toggle_2d_view)) {
				show2DView();
			}
			else if (shortcut(App.K.toggle_node_editor)) {
				showMaterialNodes();
			}
		}

		if (shortcut(App.K.file_save_as)) Project.projectSaveAs();
		else if (shortcut(App.K.file_save)) Project.projectSave();
		else if (shortcut(App.K.file_open)) Project.projectOpen();
		else if (shortcut(App.K.file_new)) UIBox.newProject();
		else if (shortcut(App.K.export_textures)) {
			if (textureExportPath == "") { // First export, ask for path
				App.showFiles = true;
				App.whandle.redraws = 2;
				App.foldersOnly = true;
				App.showFilename = true;
				UIFiles.filters = bitsHandle.position > 0 ? "exr" : formatType == 0 ? "png" : "jpg";
				App.filesDone = function(path:String) {
					textureExport = true;
					textureExportPath = path;
				}
			}
			else textureExport = true;
		}
		else if (shortcut(App.K.import_assets)) {
			App.showFiles = true;
			App.whandle.redraws = 2;
			App.foldersOnly = false;
			App.showFilename = false;
			UIFiles.filters = "jpg,png,tga,hdr,obj,fbx,blend,arm";
			App.filesDone = function(path:String) {
				Importer.importFile(path);
			}
		}

		if (kb.started(App.K.view_distract_free) ||
		   (kb.started("escape") && !show && !App.showFiles && !App.showBox)) {
			toggleDistractFree();
		}

		var mouse = Input.getMouse();

		if (brushCanLock || brushLocked) {
			if (kb.down(App.K.brush_radius) && mouse.moved) {
				if (brushLocked) {
					brushRadius += mouse.movementX / 100;
					brushRadius = Math.max(0.05, Math.min(4.0, brushRadius));
					brushRadius = Math.round(brushRadius * 100) / 100;
					brushRadiusHandle.value = brushRadius;
					headerHandle.redraws = 2;
				}
				else if (brushCanLock) {
					brushCanLock = false;
					brushLocked = true;
				}
			}
		}

		var right = iron.App.w();
		if (UIView2D.inst.show) right = iron.App.w() * 2;

		// Viewport shortcuts
		if (mouse.x > 0 && mouse.x < right &&
			mouse.y > 0 && mouse.y < iron.App.h() &&
			!ui.isTyping && !UIView2D.inst.ui.isTyping && !UINodes.inst.ui.isTyping) {

			if (UITrait.inst.worktab.position == SpacePaint) {
				if (shift) {
					if (kb.started("1")) selectMaterial(0);
					else if (kb.started("2")) selectMaterial(1);
					else if (kb.started("3")) selectMaterial(2);
					else if (kb.started("4")) selectMaterial(3);
					else if (kb.started("5")) selectMaterial(4);
					else if (kb.started("6")) selectMaterial(5);
				}

				if (!ctrl && !mouse.down("right")) {
					if (shortcut(App.K.tool_brush)) selectTool(ToolBrush);
					else if (shortcut(App.K.tool_eraser)) selectTool(ToolEraser);
					else if (shortcut(App.K.tool_fill)) selectTool(ToolFill);
					else if (shortcut(App.K.tool_bake)) selectTool(ToolBake);
					else if (shortcut(App.K.tool_colorid)) selectTool(ToolColorId);
					else if (shortcut(App.K.tool_decal)) selectTool(ToolDecal);
					else if (shortcut(App.K.tool_text)) selectTool(ToolText);
					else if (shortcut(App.K.tool_clone)) selectTool(ToolClone);
					else if (shortcut(App.K.tool_blur)) selectTool(ToolBlur);
					else if (shortcut(App.K.tool_particle)) selectTool(ToolParticle);
					else if (shortcut(App.K.tool_picker)) selectTool(ToolPicker);
				}

				// Radius
				if (!ctrl && !shift) {
					if (selectedTool == ToolBrush  ||
						selectedTool == ToolEraser ||
						selectedTool == ToolDecal  ||
						selectedTool == ToolText   ||
						selectedTool == ToolClone  ||
						selectedTool == ToolBlur   ||
						selectedTool == ToolParticle) {
						if (shortcut(App.K.brush_radius)) {
							brushCanLock = true;
							mouse.lock();
							lockStartedX = mouse.x + iron.App.x();
							lockStartedY = mouse.y + iron.App.y();
						}
					}
				}
			}

			// Viewpoint
			if (!shift && !alt) {
				if (shortcut(App.K.view_reset)) {
					ViewportUtil.resetViewport();
					ViewportUtil.scaleToBounds();
				}
				else if (shortcut(App.K.view_back)) ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI);
				else if (shortcut(App.K.view_front)) ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0);
				else if (shortcut(App.K.view_left)) ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
				else if (shortcut(App.K.view_right)) ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
				else if (shortcut(App.K.view_bottom)) ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI);
				else if (shortcut(App.K.view_top)) ViewportUtil.setView(0, 0, 1, 0, 0, 0);
				else if (shortcut(App.K.view_camera_type)) {
					cameraType = cameraType == 0 ? 1 : 0;
					camHandle.position = cameraType;
					ViewportUtil.updateCameraType(cameraType);
					statusHandle.redraws = 2;
				}
				else if (shortcut(App.K.view_orbit_left)) ViewportUtil.orbit(-Math.PI / 12, 0);
				else if (shortcut(App.K.view_orbit_right)) ViewportUtil.orbit(Math.PI / 12, 0);
				else if (shortcut(App.K.view_orbit_top)) ViewportUtil.orbit(0, -Math.PI / 12);
				else if (shortcut(App.K.view_orbit_bottom)) ViewportUtil.orbit(0, Math.PI / 12);
				else if (shortcut(App.K.view_orbit_opposite)) ViewportUtil.orbit(Math.PI, 0);
			}
		}

		if (brushCanLock || brushLocked) {
			if (mouse.moved && brushCanUnlock) {
				brushLocked = false;
				brushCanUnlock = false;
			}
			if (kb.released(App.K.brush_radius)) {
				mouse.unlock();
				brushCanUnlock = true;
				lastPaintX = -1;
				lastPaintY = -1;
			}
		}
	}

	public function toggleDistractFree() {
		show = !show;
		App.resize();
	}

	public function selectMaterialScene(i:Int) {
		if (materialsScene.length <= i || selectedObject == paintObject) return;
		selectedMaterialScene = materialsScene[i];
		if (Std.is(selectedObject, MeshObject)) {
			cast(selectedObject, MeshObject).materials[0] = selectedMaterialScene.data;
		}
		UINodes.inst.updateCanvasMap();
		MaterialParser.parsePaintMaterial();
		hwnd.redraws = 2;
	}

	public function selectMaterial(i:Int) {
		if (materials.length <= i) return;
		setMaterial(materials[i]);
	}

	public function setMaterial(m:MaterialSlot) {
		selectedMaterial = m;
		UINodes.inst.updateCanvasMap();
		MaterialParser.parsePaintMaterial();
		hwnd1.redraws = 2;
		headerHandle.redraws = 2;

		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();
		var decal = selectedTool == ToolDecal || selectedTool == ToolText;
		if (decal) RenderUtil.makeDecalPreview();
		if (current != null) current.begin(false);
	}

	public function selectBrush(i:Int) {
		if (brushes.length <= i) return;
		selectedBrush = brushes[i];
		UINodes.inst.updateCanvasBrushMap();
		MaterialParser.parseBrush();
		hwnd1.redraws = 2;
	}

	public function setLayer(l:LayerSlot, isMask = false) {
		selectedLayer = l;
		selectedLayerIsMask = isMask;
		headerHandle.redraws = 2;

		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		setObjectMask();
		MaterialParser.parseMeshMaterial();
		MaterialParser.parsePaintMaterial();

		if (current != null) current.begin(false);

		hwnd.redraws = 2;
	}

	function updateUI() {

		if (messageTimer > 0) {
			messageTimer -= Time.delta;
			if (messageTimer <= 0) statusHandle.redraws = 2;
		}

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		if (!App.uienabled) return;

		var down = Input.getMouse().down() || Input.getPen().down();
		if (down && !kb.down("control")) {
			var mx = mouse.x;
			var my = mouse.y;
			if (paint2d) mx -= iron.App.w();

			if (mx < iron.App.w() && mx > iron.App.x() &&
				my < iron.App.h() && my > iron.App.y()) {
				
				if (selectedTool == ToolClone && kb.down("alt")) { // Clone source
					cloneStartX = mx;
					cloneStartY = my;
				}
				else {
					if (brushTime == 0) { // Paint started
						pushUndo = true;
						kha.Window.get(0).title = App.filenameHandle.text + "* - ArmorPaint";
						if (selectedTool == ToolClone && cloneStartX >= 0.0) { // Clone delta
							cloneDeltaX = (cloneStartX - mx) / iron.App.w();
							cloneDeltaY = (cloneStartY - my) / iron.App.h();
							cloneStartX = -1;
						}
						else if (selectedTool == ToolParticle) {
							// Reset particles
							var emitter:MeshObject = cast Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							@:privateAccess psys.time = 0;
							// @:privateAccess psys.time = @:privateAccess psys.seed * @:privateAccess psys.animtime;
							// @:privateAccess psys.seed++;
						}
					}
					brushTime += Time.delta;
					for (f in _onBrush) f(0);
				}
			}

		}
		else if (brushTime > 0) { // Brush released
			brushTime = 0;
			ddirty = 3;
			brushBlendDirty = true; // Update brush mask
			layerPreviewDirty = true; // Update layer preview
		}

		if (layersPreviewDirty) {
			layersPreviewDirty = false;
			layerPreviewDirty = false;
			// Update all layer previews
			for (l in layers) {
				var target = l.texpaint_preview;
				var source = l.texpaint;
				var g2 = target.g2;
				g2.begin(true, 0xff000000);
				g2.drawScaledImage(source, 0, 0, target.width, target.height);
				g2.end();
				if (l.texpaint_mask != null) {
					var target = l.texpaint_mask_preview;
					var source = l.texpaint_mask;
					var g2 = target.g2;
					g2.begin(true, 0xff000000);
					g2.drawScaledImage(source, 0, 0, target.width, target.height);
					g2.end();
				}
			}
			hwnd.redraws = 2;
		}
		if (layerPreviewDirty) {
			layerPreviewDirty = false;
			// Update layer preview
			var l = selectedLayer;
			var target = selectedLayerIsMask ? l.texpaint_mask_preview : l.texpaint_preview;
			var source = selectedLayerIsMask ? l.texpaint_mask : l.texpaint;
			var g2 = target.g2;
			g2.begin(true, 0xff000000);
			g2.drawScaledImage(source, 0, 0, target.width, target.height);
			g2.end();
			hwnd.redraws = 2;
		}

		#if krom_darwin
		var undoPressed = !kb.down("shift") && kb.started("z"); // cmd+z on macos
		var redoPressed = (kb.down("shift") && kb.started("z")) || kb.started("y"); // cmd+y on macos
		#else
		var undoPressed = shortcut(App.K.edit_undo);
		var redoPressed = shortcut(App.K.edit_redo) ||
						  (kb.down("control") && kb.started("y"));
		#end

		if (undoPressed) History.doUndo();
		else if (redoPressed) History.doRedo();

		if (UITrait.inst.worktab.position == SpaceScene) {
			arm.plugin.Gizmo.update();
		}

		if (lastCombo != null || (ui.tooltipImg == null && lastTooltip != null)) App.redrawUI();
		lastCombo = ui.comboSelectedHandle;
		lastTooltip = ui.tooltipImg;
	}

	function render(g:kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		renderUI(g);
	}

	function renderCursor(g:kha.graphics2.Graphics) {
		// if (cursorImg == null) {
		// 	g.end();
		// 	cursorImg = Image.createRenderTarget(256, 256);
		// 	cursorImg.g2.begin(true, 0x00000000);
		// 	cursorImg.g2.color = 0xffcccccc;
		// 	kha.graphics2.GraphicsExtension.drawCircle(cursorImg.g2, 128, 128, 124, 8);
		// 	cursorImg.g2.end();
		// 	g.begin(false);
		// }

		g.color = 0xffffffff;

		// Brush
		if (App.uienabled && worktab.position == SpacePaint) {
			var mouse = Input.getMouse();
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
			var pen = Input.getPen();
			if (pen.down()) {
				mx = pen.x + iron.App.x();
				my = pen.y + iron.App.y();
			}

			// Radius being scaled
			if (brushLocked) {
				mx += lockStartedX - System.windowWidth() / 2;
				my += lockStartedY - System.windowHeight() / 2;
			}

			// Show picked material next to cursor
			if (selectedTool == ToolPicker && UITrait.inst.pickerSelectMaterial) {
				var img = selectedMaterial.imageIcon;
				g.drawImage(img, mx + 10, my + 10);
			}

			var cursorImg = Res.get('cursor.png');
			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));

			// Clone source cursor
			var kb = Input.getKeyboard();
			if (selectedTool == ToolClone && !kb.down("alt") && (mouse.down() || pen.down())) {
				g.color = 0x66ffffff;
				g.drawScaledImage(cursorImg, mx + cloneDeltaX * iron.App.w() - psize / 2, my + cloneDeltaY * iron.App.h() - psize / 2, psize, psize);
				g.color = 0xffffffff;
			}

			var in2dView = UIView2D.inst.show && UIView2D.inst.type == 0 &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var decal = selectedTool == ToolDecal || selectedTool == ToolText;
			
			if (!brush3d || in2dView || decal) {
				if (decal) {
					psize = Std.int(256 * (brushRadius * brushNodesRadius));
					#if kha_direct3d11
					g.drawScaledImage(decalImage, mx - psize / 2, my - psize / 2, psize, psize);
					#else
					g.drawScaledImage(decalImage, mx - psize / 2, my - psize / 2 + psize, psize, -psize);
					#end
				}
				else if (selectedTool == ToolBrush  ||
						 selectedTool == ToolEraser ||
						 selectedTool == ToolClone  ||
						 selectedTool == ToolBlur   ||
						 selectedTool == ToolParticle) {
						g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
				}
			}
		}
	}

	public function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		if (UINodes.inst.show && UINodes.inst.canvasType == 0) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 0; }
		App.resize();
	}

	public function showBrushNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		if (UINodes.inst.show && UINodes.inst.canvasType == 1) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 1; }
		App.resize();
	}

	// function showParticleNodes() {}

	public function show2DView(type = 0) {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UIView2D.inst.ui.endInput();
		if (UIView2D.inst.type != type) UIView2D.inst.show = true;
		else UIView2D.inst.show = !UIView2D.inst.show;
		UIView2D.inst.type = type;
		App.resize();
	}

	function selectTool(i:Int) {
		selectedTool = i;
		MaterialParser.parsePaintMaterial();
		MaterialParser.parseMeshMaterial();
		headerHandle.redraws = 2;
		toolbarHandle.redraws = 2;

		var decal = selectedTool == ToolDecal || selectedTool == ToolText;
		if (decal) {
			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			if (selectedTool == ToolDecal) {
				RenderUtil.makeDecalMaskPreview();
			}
			else if (selectedTool == ToolText) {
				RenderUtil.makeTextPreview();
			}

			RenderUtil.makeDecalPreview();
			ddirty = 2;
			
			if (current != null) current.begin(false);
		}

		if (selectedTool == ToolParticle) {
			Tool.initParticle();
			MaterialParser.parseParticleMaterial();
		}
	}

	public function selectObject(o:Object) {
		selectedObject = o;

		if (UITrait.inst.worktab.position == SpaceScene) {
			if (Std.is(o, MeshObject)) {
				for (i in 0...materialsScene.length) {
					if (materialsScene[i].data == cast(o, MeshObject).materials[0]) {
						// selectMaterial(i); // loop
						selectedMaterialScene = materialsScene[i];
						hwnd.redraws = 2;
						break;
					}
				}
			}
		}
	}

	public function selectPaintObject(o:MeshObject) {
		headerHandle.redraws = 2;
		for (p in paintObjects) p.skip_context = "paint";
		paintObject = o;

		var mask = selectedLayer.objectMask;
		if (layerFilter > 0) mask = layerFilter;
		
		if (mergedObject == null || mask > 0) {
			paintObject.skip_context = "";
		}
		UVUtil.uvmapCached = false;
		UVUtil.trianglemapCached = false;
	}

	public function getImage(asset:zui.Canvas.TAsset):Image {
		return asset != null ? zui.Canvas.assetMap.get(asset.id) : null;
	}

	public function mainObject():MeshObject {
		for (po in paintObjects) if (po.children.length > 0) return po;
		return paintObjects[0];
	}

	function renderUI(g:kha.graphics2.Graphics) {
		
		if (!App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (App.uienabled && !ui.inputRegistered) ui.registerInput();

		if (!show) return;

		g.end();
		ui.begin(g);

		var panelx = (iron.App.x() - toolbarw);
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(toolbarHandle, panelx, headerh, toolbarw, System.windowHeight())) {
			ui._y += 2;

			ui.imageScrollAlign = false;

			if (UITrait.inst.worktab.position == SpacePaint) {
				var keys = ['(B)', '(E)', '(G)', '(D)', '(T)', '(L) - Hold ALT to set source', '(U)', '(P)', '(K)', '(C)', '(V)'];
				var img = Res.get("icons.png");
				var imgw = ui.SCALE > 1 ? 100 : 50;
				for (i in 0...toolNames.length) {
					ui._x += 2;
					if (selectedTool == i) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
					if (ui.image(img, -1, null, i * imgw, 0, imgw, imgw) == State.Started) selectTool(i);
					if (ui.isHovered) ui.tooltip(toolNames[i] + " " + keys[i]);
					ui._x -= 2;
					ui._y += 2;
				}
			}
			else if (UITrait.inst.worktab.position == SpaceScene) {
				var img = Res.get("icons.png");
				var imgw = ui.SCALE > 1 ? 100 : 50;
				ui._x += 2;
				if (selectedTool == ToolGizmo) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip("Gizmo (G)");
				ui._x -= 2;
				ui._y += 2;
			}

			ui.imageScrollAlign = true;
		}

		var panelx = iron.App.x() - toolbarw;
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		
		var WINDOW_BG_COL = ui.t.WINDOW_BG_COL;
		ui.t.WINDOW_BG_COL = ui.t.SEPARATOR_COL;
		if (ui.window(menuHandle, panelx, 0, menubarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			var _w = ui._w;
			ui._w = Std.int(ui._w * 0.5);
			ui._x += 1; // Prevent "File" button highlight on startup
			
			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

			if (ui.button("File", Left) || (App.showMenu && ui.isHovered)) { App.showMenu = true; UIMenu.menuCategory = 0; };
			if (ui.button("Edit", Left) || (App.showMenu && ui.isHovered)) { App.showMenu = true; UIMenu.menuCategory = 1; };
			if (ui.button("View", Left) || (App.showMenu && ui.isHovered)) { App.showMenu = true; UIMenu.menuCategory = 2; };
			if (ui.button("Help", Left) || (App.showMenu && ui.isHovered)) { App.showMenu = true; UIMenu.menuCategory = 3; };
			
			ui._w = _w;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.BUTTON_COL = BUTTON_COL;
		}
		ui.t.WINDOW_BG_COL = WINDOW_BG_COL;

		var panelx = (iron.App.x() - toolbarw) + menubarw;
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(workspaceHandle, panelx, 0, System.windowWidth() - windowW - menubarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			ui.tab(worktab, "Paint");
			// ui.tab(worktab, "Sculpt");
			ui.tab(worktab, "Scene");
			if (worktab.changed) {
				ddirty = 2;
				toolbarHandle.redraws = 2;
				headerHandle.redraws = 2;
				hwnd.redraws = 2;
				hwnd1.redraws = 2;
				hwnd2.redraws = 2;

				if (worktab.position == SpaceScene) {
					selectTool(ToolGizmo);
				}

				MaterialParser.parseMeshMaterial();
				mainObject().skip_context = null;
			}
		}

		var panelx = iron.App.x();
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth() - toolbarw - windowW, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {

			if (UITrait.inst.worktab.position == SpacePaint) {

				if (selectedTool == ToolColorId) {
					ui.text("Picked Color");
					if (colorIdPicked) {
						ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
					}
					if (ui.button("Clear")) colorIdPicked = false;
					ui.text("Color ID Map");
					var cid = ui.combo(colorIdHandle, App.getEnumTexts(), "Color ID");
					if (UITrait.inst.assets.length > 0) ui.image(UITrait.inst.getImage(UITrait.inst.assets[cid]));
				}
				else if (selectedTool == ToolPicker) {
					baseRPicked = Math.round(baseRPicked * 10) / 10;
					baseGPicked = Math.round(baseGPicked * 10) / 10;
					baseBPicked = Math.round(baseBPicked * 10) / 10;
					normalRPicked = Math.round(normalRPicked * 10) / 10;
					normalGPicked = Math.round(normalGPicked * 10) / 10;
					normalBPicked = Math.round(normalBPicked * 10) / 10;
					occlusionPicked = Math.round(occlusionPicked * 100) / 100;
					roughnessPicked = Math.round(roughnessPicked * 100) / 100;
					metallicPicked = Math.round(metallicPicked * 100) / 100;
					ui.text('Base $baseRPicked,$baseGPicked,$baseBPicked');
					ui.text('Nor $normalRPicked,$normalGPicked,$normalBPicked');
					ui.text('Occlusion $occlusionPicked');
					ui.text('Roughness $roughnessPicked');
					ui.text('Metallic $metallicPicked');
					pickerSelectMaterial = ui.check(Id.handle({selected: pickerSelectMaterial}), "Select Material");
					ui.combo(pickerMaskHandle, ["None", "Material"], "Mask", true);
					if (pickerMaskHandle.changed) {
						UINodes.inst.updateCanvasMap();
						MaterialParser.parsePaintMaterial();
					}
				}
				else if (selectedTool == ToolBake) {
					var bakeHandle = Id.handle({position: bakeType});
					bakeType = ui.combo(bakeHandle, ["AO", "Position", "TexCoord", "Material ID", "Normal (World)"], "Bake");
					if (bakeHandle.changed) {
						MaterialParser.parsePaintMaterial();
					}
					if (bakeType == 0) {
						var h = Id.handle({value: bakeStrength});
						bakeStrength = ui.slider(h, "Strength", 0.0, 2.0, true);
						if (h.changed) MaterialParser.parsePaintMaterial();
						var h = Id.handle({value: bakeRadius});
						bakeRadius = ui.slider(h, "Radius", 0.0, 2.0, true);
						if (h.changed) MaterialParser.parsePaintMaterial();
						var h = Id.handle({value: bakeOffset});
						bakeOffset = ui.slider(h, "Offset", 0.0, 2.0, true);
						if (h.changed) MaterialParser.parsePaintMaterial();
					}
				}
				else {
					if (selectedTool != ToolFill) {
						brushRadius = ui.slider(brushRadiusHandle, "Radius", 0.0, 2.0, true);
					}
					
					if (selectedTool == ToolBrush  ||
						selectedTool == ToolFill   ||
						selectedTool == ToolDecal  ||
						selectedTool == ToolText) {
						var brushScaleHandle = Id.handle({value: brushScale});
						brushScale = ui.slider(brushScaleHandle, "UV Scale", 0.0, 2.0, true);
						if (brushScaleHandle.changed) {
							if (selectedTool == ToolDecal || selectedTool == ToolText) {
								ui.g.end();
								RenderUtil.makeDecalMaskPreview();
								RenderUtil.makeDecalPreview();
								ui.g.begin(false);
							}
						}

						var brushRotHandle = Id.handle({value: brushRot});
						brushRot = ui.slider(brushRotHandle, "UV Rotate", 0.0, 360.0, true, 1);
						if (brushRotHandle.changed) MaterialParser.parsePaintMaterial();
					}
					
					brushOpacity = ui.slider(Id.handle({value: brushOpacity}), "Opacity", 0.0, 1.0, true);
					
					if (selectedTool == ToolBrush || selectedTool == ToolEraser) {
						brushHardness = ui.slider(Id.handle({value: brushHardness}), "Hardness", 0.0, 1.0, true);
					}

					ui.combo(Id.handle(), ["Add"], "Blending");

					if (selectedTool == ToolBrush || selectedTool == ToolFill) {
						var paintHandle = Id.handle();
						brushPaint = ui.combo(paintHandle, ["UV Map", "Project", "Triplanar"], "TexCoord");
						if (paintHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}
					if (selectedTool == ToolDecal) {
						ui.combo(decalMaskHandle, ["Rectangle", "Circle", "Triangle"], "Mask");
						if (decalMaskHandle.changed) {
							ui.g.end();
							RenderUtil.makeDecalMaskPreview();
							RenderUtil.makeDecalPreview();
							ui.g.begin(false);
						}
					}
					if (selectedTool == ToolText) {
						ui.combo(textToolHandle, Importer.fontList, "Font");
						var h = Id.handle();
						h.text = textToolText;
						textToolText = ui.textInput(h, "");
						if (h.changed || textToolHandle.changed) {
							ui.g.end();
							RenderUtil.makeTextPreview();
							RenderUtil.makeDecalPreview();
							ui.g.begin(false);
						}
					}

					if (selectedTool == ToolFill) {
						ui.combo(fillTypeHandle, ["Object", "Face", "Angle"], "Fill Mode");
						if (fillTypeHandle.changed) {
							if (fillTypeHandle.position == 1) {
								ui.g.end();
								// UVUtil.cacheUVMap();
								UVUtil.cacheTriangleMap();
								ui.g.begin(false);
								// wireframeHandle.selected = drawWireframe = true;
							}
							MaterialParser.parsePaintMaterial();
							MaterialParser.parseMeshMaterial();
						}
					}
					else {
						var _w = ui._w;
						var sc = ui.SCALE;
						ui._w = Std.int(60 * sc);

						var xrayHandle = Id.handle({selected: xray});
						xray = ui.check(xrayHandle, "X-Ray");
						if (xrayHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
						
						var symXHandle = Id.handle({selected: false});
						var symYHandle = Id.handle({selected: false});
						var symZHandle = Id.handle({selected: false});
						ui._w = Std.int(55 * sc);
						ui.text("Symmetry");
						ui._w = Std.int(25 * sc);
						symX = ui.check(symXHandle, "X");
						symY = ui.check(symYHandle, "Y");
						symZ = ui.check(symZHandle, "Z");
						if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
						
						ui._w = _w;
					}
				}
			}
		}

		if (ui.window(statusHandle, iron.App.x(), System.windowHeight() - headerh, System.windowWidth() - toolbarw - windowW, headerh)) {

			var scene = Scene.active;
			var cam = scene.cameras[0];
			cameraControls = ui.combo(Id.handle({position: cameraControls}), ["Rotate", "Orbit", "Fly"], "Controls");
			cameraType = ui.combo(camHandle, ["Perspective", "Orthographic"], "Type");
			if (ui.isHovered) ui.tooltip("Camera Type (5)");
			if (camHandle.changed) {
				ViewportUtil.updateCameraType(cameraType);
			}

			fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
			cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
			if (fovHandle.changed) {
				ViewportUtil.updateCameraType(cameraType);
			}

			if (messageTimer > 0) {
				var _w = ui._w;
				ui._w = Std.int(ui.ops.font.width(ui.fontSize, message) + 65 * ui.SCALE);
				ui.fill(0, 0, ui._w, ui._h, messageColor);
				ui.text(message);
				ui._w = _w;
			}
		}
		
		tabx = App.C.ui_layout == 0 ? System.windowWidth() - windowW : 0;
		tabh = Std.int(System.windowHeight() / 3);
		gizmo.visible = false;

		if (worktab.position == SpacePaint) {
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				TabLayers.draw();
				TabHistory.draw();
				TabPlugins.draw();
				TabPreferences.draw();
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh)) {
				selectedObject = paintObject;
				TabMaterials.draw();
				TabBrushes.draw();
				TabParticles.draw();
			}
			if (ui.window(hwnd2, tabx, tabh * 2, windowW, tabh)) {
				TabTextures.draw();
				TabMeshes.draw();
				TabExport.draw();
				TabViewport.draw();
			}
		}
		else if (worktab.position == SpaceScene) {
			gizmo.visible = true;
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				TabOutliner.draw();
				TabPlugins.draw();
				TabPreferences.draw();
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh)) {
				TabMaterials.draw();
				TabProperties.draw();
			}
			if (ui.window(hwnd2, tabx, tabh * 2, windowW, tabh)) {
				TabTextures.draw();
				TabViewport.draw();
			}
		}

		ui.end();
		g.begin(false);
	}

	public function setObjectMask() {
		var ar = ["None"];
		for (p in paintObjects) ar.push(p.name);

		var mask = selectedLayer.objectMask;
		if (layerFilter > 0) mask = layerFilter;
		if (mask > 0) {
			if (mergedObject != null) mergedObject.visible = false;
			var o = paintObjects[0];
			for (p in paintObjects) if (p.name == ar[mask]) { o = p; break; }
			selectPaintObject(o);
		}
		else {
			if (mergedObject == null) {
				MeshUtil.mergeMesh();
			}
			selectPaintObject(mainObject());
			paintObject.skip_context = "paint";
			mergedObject.visible = true;
		}
	}

	public function newLayer():LayerSlot {
		if (layers.length > 255) return null;
		var l = new LayerSlot();
		layers.push(l);
		setLayer(l);
		iron.App.notifyOnRender(Layers.clearLastLayer);
		layerPreviewDirty = true;
		return l;
	}

	public function toFillLayer(l:LayerSlot) {
		setLayer(l);
		l.material_mask = UITrait.inst.selectedMaterial;
		function makeFill(g:kha.graphics4.Graphics) {
			g.end();
			Layers.updateFillLayers(4);
			MaterialParser.parsePaintMaterial();
			UITrait.inst.layerPreviewDirty = true;
			hwnd.redraws = 2;
			g.begin();
			iron.App.removeRender(makeFill);
		}
		iron.App.notifyOnRender(makeFill);
	}

	public function toPaintLayer(l:LayerSlot) {
		setLayer(l);
		l.material_mask = null;
		MaterialParser.parsePaintMaterial();
		UITrait.inst.layerPreviewDirty = true;
		hwnd.redraws = 2;
	}

	public function setIconScale() {
		if (ui.SCALE > 1) {
			Res.load(["icons2x.png"], function() {
				@:privateAccess Res.bundled.set("icons.png", Res.get("icons2x.png"));
			});
		}
		else {
			Res.load(["icons.png"], function() {});
		}
	}

	public function removeMaterialCache() {
		Data.cachedMaterials.remove("SceneMaterial2");
		Data.cachedShaders.remove("Material2_data");
		Data.cachedSceneRaws.remove("Material2_data");
		// Data.cachedBlobs.remove("Material2_data.arm");
	}

	public function createFillLayer() {
		var l = UITrait.inst.newLayer();
		l.objectMask = UITrait.inst.layerFilter;
		UITrait.inst.toFillLayer(l);
	}

	public function createImageMask(asset:zui.Canvas.TAsset) {
		var l = UITrait.inst.selectedLayer;
		if (l != UITrait.inst.layers[0]) {
			l.createMask(0x00000000, true, UITrait.inst.getImage(asset));
			UITrait.inst.setLayer(l, true);
			UITrait.inst.layerPreviewDirty = true;
		}
	}

	public function importMesh() {
		App.showFiles = true;
		App.whandle.redraws = 2;
		App.foldersOnly = false;
		App.showFilename = false;
		UIFiles.filters = "obj,fbx,blend,arm";
		App.filesDone = function(path:String) {
			Importer.importFile(path);
		}
	}

	function shortcut(s:String):Bool {
		var kb = Input.getKeyboard();
		var flag = true;
		var plus = s.indexOf("+");
		if (plus > 0) {
			var shift = s.indexOf("shift") >= 0;
			var ctrl = s.indexOf("ctrl") >= 0;
			flag = shift == kb.down("shift") && ctrl == kb.down("control");
			s = s.substr(s.lastIndexOf("+") + 1);
		}
		return flag && kb.started(s);
	}
}
