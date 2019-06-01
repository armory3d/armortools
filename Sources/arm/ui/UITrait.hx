package arm.ui;

import zui.Zui;
import zui.Id;
import zui.Ext;
import zui.Canvas;
import iron.data.MaterialData;
import iron.object.Object;
import iron.object.MeshObject;
import iron.math.Mat4;
import iron.math.Math;
import iron.RenderPath;
import arm.MaterialParser;
import arm.ProjectFormat;
import arm.util.MeshUtil;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.UVUtil;
import arm.Tool;

@:access(zui.Zui)
class UITrait extends iron.Trait {

	public var project:TProjectFormat;
	public var projectPath = "";

	public var assets:Array<TAsset> = [];
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

	public var savedEnvmap:kha.Image = null;
	public var emptyEnvmap:kha.Image = null;
	public var previewEnvmap:kha.Image = null;
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
	var materialPreviewReady = false;

	public var windowW = 280; // Panel width
	public var toolbarw = 54;
	public var headerh = 24;
	public var menubarw = 215;
	public var tabx = 0;
	public var tabh = 0;

	public var ui:Zui;
	public var systemId = "";
	public var colorIdHandle = Id.handle();

	public var formatType = 0;
	public var formatQuality = 80.0;
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
	public var hwnd = Id.handle();
	public var hwnd1 = Id.handle();
	public var hwnd2 = Id.handle();
	public var materials:Array<MaterialSlot> = null;
	public var selectedMaterial:MaterialSlot;
	public var materials2:Array<MaterialSlot> = null;
	public var selectedMaterial2:MaterialSlot;
	public var selectedTexture:TAsset = null;
	public var brushes:Array<BrushSlot> = null;
	public var selectedBrush:BrushSlot;
	public var selectedLogic:BrushSlot;
	public var layers:Array<LayerSlot> = null;
	public var undoLayers:Array<LayerSlot> = null;
	public var selectedLayer:LayerSlot;
	public var selectedLayerIsMask = false; // Mask selected for active layer
	var selectTime = 0.0;
	public var displaceStrength = 1.0;
	public var decalImage:kha.Image = null;
	public var decalPreview = false;
	public var viewportMode = 0;
	var hscaleWasChanged = false;
	public var exportMeshFormat = 0;

	public var textToolImage:kha.Image = null;
	public var textToolText = "Text";
	public var textToolHandle = new Handle({position: 0});
	public var decalMaskImage:kha.Image = null;
	public var decalMaskHandle = new Handle({position: 0});
	public var particleMaterial:MaterialData = null;

	var layerFilter = 0;

	var _onBrush:Array<Int->Void> = [];

	public var paintVec = new iron.math.Vec4();
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
	public var grid:Object = null;
	public var axisX = false;
	public var axisY = false;
	public var axisZ = false;
	public var axisStart = 0.0;
	var row4 = [1/4, 1/4, 1/4, 1/4];

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
	public var bakeType = 0;
	public var bakeStrength = 1.0;
	public var bakeRadius = 1.0;
	public var bakeOffset = 1.0;
	
	public var xray = false;
	public var mirrorX = false;
	public var symX = false;
	public var symY = false;
	public var symZ = false;
	public var showGrid = false;
	public var showCompass = true;
	public var fillTypeHandle = new Handle();
	public var resHandle = new Handle({position: 1}); // 2048
	public var mergedObject:MeshObject = null; // For object mask
	var newConfirm = false;
	public var newObject = 0;
	public var newObjectNames = ["Cube", "Plane", "Sphere", "Cylinder"];

	public var sub = 0;
	public var vec2 = new iron.math.Vec4();

	public var lastPaintVecX = -1.0;
	public var lastPaintVecY = -1.0;
	var frame = 0;
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
	var textureExport = false;
	var textureExportPath = "";
	public var projectExport = false;
	public var headerHandle = new Handle({layout:Horizontal});
	public var toolbarHandle = new Handle();
	public var statusHandle = new Handle({layout:Horizontal});
	public var menuHandle = new Handle({layout:Horizontal});
	public var workspaceHandle = new Handle({layout:Horizontal});
	var lastCombo:Handle = null;

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
		if (UITrait.inst.worktab.position == 1) return false;
		return pdirty > 0;
	}

	public function new() {
		super();

		inst = this;
		systemId = kha.System.systemId;

		windowW = Std.int(defaultWindowW * App.C.window_scale);
		toolbarw = Std.int(defaultToolbarW * App.C.window_scale);
		headerh = Std.int(defaultHeaderH * App.C.window_scale);
		menubarw = Std.int(215 * App.C.window_scale);

		Uniforms.init();

		if (materials == null) {
			materials = [];
			//
			iron.data.Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
				materials.push(new MaterialSlot(m));
				selectedMaterial = materials[0];
			});
			//
			// materials.push(new MaterialSlot());
			// selectedMaterial = materials[0];
		}
		if (materials2 == null) {
			materials2 = [];
			iron.data.Data.getMaterial("Scene", "Material2", function(m:iron.data.MaterialData) {
				materials2.push(new MaterialSlot(m));
				selectedMaterial2 = materials2[0];
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

		var world = iron.Scene.active.world;
		if (savedEnvmap == null) {
			savedEnvmap = world.envmap;
		}
		world.envmap = UITrait.inst.showEnvmap ? UITrait.inst.savedEnvmap : UITrait.inst.emptyEnvmap;
		ddirty = 1;

		// Save last pos for continuos paint
		iron.App.notifyOnRender(function(g:kha.graphics4.Graphics) { //
			if (frame == 2) {
				RenderUtil.makeMaterialPreview();
				materialPreviewReady = true;
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

			var m = iron.system.Input.getMouse();
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
		ui = new Zui( { theme: arm.App.theme, font: arm.App.font, scaleFactor: scale, color_wheel: arm.App.color_wheel } );
		
		var resources = ['cursor.png', 'icons.png'];
		Res.load(resources, done);
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
		//
		// grid = iron.Scene.active.getChild(".Grid");
		gizmo = iron.Scene.active.getChild(".GizmoTranslate");
		gizmo.transform.scale.set(0.5, 0.5, 0.5);
		gizmo.transform.buildMatrix();
		gizmoX = iron.Scene.active.getChild("GizmoX");
		gizmoY = iron.Scene.active.getChild("GizmoY");
		gizmoZ = iron.Scene.active.getChild("GizmoZ");
		//

		selectedObject = iron.Scene.active.getChild("Cube");
		paintObject = cast(selectedObject, MeshObject);
		paintObjects = [paintObject];

		if (App.fileArg == "") {
			iron.App.notifyOnRender(Layers.initLayers);
		}

		// Init plugins
		Plugin.keep();
		if (App.C.plugins != null) {
			for (plugin in App.C.plugins) {
				iron.data.Data.getBlob("plugins/" + plugin, function(blob:kha.Blob) {
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
			Project.exportProject();
		}

		isScrolling = ui.isScrolling;
		updateUI();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		var kb = iron.system.Input.getKeyboard();
		var shift = kb.down("shift");
		var ctrl = kb.down("control");
		var alt = kb.down("alt");
		alt = false; // TODO: gets stuck after alt+tab

		if (!arm.App.uibox.isTyping) {
			if (kb.started("escape")) {
				arm.App.showFiles = false;
				arm.App.showBox = false;
			}
			if (arm.App.showFiles) {
				if (kb.started("enter")) {
					arm.App.showFiles = false;
					arm.App.filesDone(arm.App.path);
					UITrait.inst.ddirty = 2;
				}
			}
		}

		if (!arm.App.uienabled) return;

		if (kb.started("tab") && !UINodes.inst.ui.isTyping && !UITrait.inst.ui.isTyping) {
			if (ctrl) { // Cycle layers
				var i = (layers.indexOf(selectedLayer) + 1) % layers.length;
				setLayer(layers[i]);
			}
			else if (shift) show2DView();
			else showMaterialNodes();
		}

		if (ctrl && !shift && kb.started("s")) Project.projectSave();
		else if (ctrl && shift && kb.started("s")) Project.projectSaveAs();
		else if (ctrl && kb.started("o")) Project.projectOpen();
		else if (ctrl && kb.started("n")) UIBox.newProject();
		else if (ctrl && shift && kb.started("e")) { // Export textures
			if (textureExportPath == "") { // First export, ask for path
				arm.App.showFiles = true;
				@:privateAccess Ext.lastPath = "";
				arm.App.whandle.redraws = 2;
				arm.App.foldersOnly = true;
				arm.App.showFilename = true;
				UIFiles.filters = formatType == 0 ? "jpg" : "png";
				arm.App.filesDone = function(path:String) {
					textureExport = true;
					textureExportPath = path;
				}
			}
			else textureExport = true;
		}
		else if (ctrl && shift && kb.started("i")) { // Import asset
			arm.App.showFiles = true;
			@:privateAccess Ext.lastPath = ""; // Refresh
			arm.App.whandle.redraws = 2;
			arm.App.foldersOnly = false;
			arm.App.showFilename = false;
			UIFiles.filters = "jpg,png,tga,hdr,obj,fbx,blend,gltf,arm";
			arm.App.filesDone = function(path:String) {
				Importer.importFile(path);
			}
		}

		if (kb.started("f11") ||
		   (kb.started("escape") && !show && !arm.App.showFiles && !arm.App.showBox)) {
			toggleDistractFree();
		}

		var mouse = iron.system.Input.getMouse();

		if (brushCanLock || brushLocked) {
			if (kb.down("f") && mouse.moved) {
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

			if (UITrait.inst.worktab.position == 0) { // Paint
				if (shift) {
					if (kb.started("1")) selectMaterial(0);
					else if (kb.started("2")) selectMaterial(1);
					else if (kb.started("3")) selectMaterial(2);
					else if (kb.started("4")) selectMaterial(3);
					else if (kb.started("5")) selectMaterial(4);
					else if (kb.started("6")) selectMaterial(5);
				}

				if (!ctrl && !mouse.down("right")) {
					if (kb.started("b")) selectTool(ToolBrush);
					else if (kb.started("e")) selectTool(ToolEraser);
					else if (kb.started("g")) selectTool(ToolFill);
					else if (kb.started("k")) selectTool(ToolBake);
					else if (kb.started("c")) selectTool(ToolColorId);
					else if (kb.started("d")) selectTool(ToolDecal);
					else if (kb.started("t")) selectTool(ToolText);
					else if (kb.started("l")) selectTool(ToolClone);
					else if (kb.started("u")) selectTool(ToolBlur);
					else if (kb.started("p")) selectTool(ToolParticle);
					else if (kb.started("v")) selectTool(ToolPicker);
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
						if (kb.started("f")) {
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
				if (kb.started("0")) {
					ViewportUtil.resetViewport();
					ViewportUtil.scaleToBounds();
				}
				else if (kb.started("1")) {
					ctrl ?
						ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI) :
						ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0);
				}
				else if (kb.started("3")) {
					ctrl ?
						ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2) :
						ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
				}
				else if (kb.started("7")) {
					ctrl ?
						ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI) :
						ViewportUtil.setView(0, 0, 1, 0, 0, 0);
				}
				else if (kb.started("5")) {
					cameraType = cameraType == 0 ? 1 : 0;
					camHandle.position = cameraType;
					ViewportUtil.updateCameraType(cameraType);
					statusHandle.redraws = 2;
				}
				else if (kb.started("4")) { ViewportUtil.orbit(-Math.PI / 12, 0); }
				else if (kb.started("6")) { ViewportUtil.orbit(Math.PI / 12, 0); }
				else if (kb.started("8")) { ViewportUtil.orbit(0, -Math.PI / 12); }
				else if (kb.started("2")) { ViewportUtil.orbit(0, Math.PI / 12); }
				else if (kb.started("9")) { ViewportUtil.orbit(Math.PI, 0); }
			}
		}

		if (brushCanLock || brushLocked) {
			if (mouse.moved && brushCanUnlock) {
				brushLocked = false;
				brushCanUnlock = false;
			}
			if (kb.released("f")) {
				mouse.unlock();
				brushCanUnlock = true;
			}
		}
	}

	public function toggleDistractFree() {
		show = !show;
		arm.App.resize();
	}

	function selectMaterial(i:Int) {
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
		if (worktab.position == 2) MaterialParser.parseMeshPreviewMaterial();
		if (current != null) current.begin(false);
	}

	function selectMaterial2(i:Int) {
		if (materials2.length <= i) return;
		selectedMaterial2 = materials2[i];

		if (Std.is(selectedObject, MeshObject)) {
			cast(selectedObject, MeshObject).materials[0] = selectedMaterial2.data;
		}

		UINodes.inst.updateCanvasMap();
		MaterialParser.parsePaintMaterial();

		hwnd.redraws = 2;
	}

	function selectBrush(i:Int) {
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
			messageTimer -= iron.system.Time.delta;
			if (messageTimer <= 0) statusHandle.redraws = 2;
		}

		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if (!arm.App.uienabled) return;

		var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
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
						if (projectPath != "") {
							kha.Window.get(0).title = arm.App.filenameHandle.text + "* - ArmorPaint";
						}
						if (selectedTool == ToolClone && cloneStartX >= 0.0) { // Clone delta
							cloneDeltaX = (cloneStartX - mx) / iron.App.w();
							cloneDeltaY = (cloneStartY - my) / iron.App.h();
							cloneStartX = -1;
						}
						else if (selectedTool == ToolParticle) {
							// Reset particles
							var emitter:MeshObject = cast iron.Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							@:privateAccess psys.time = 0;
							// @:privateAccess psys.time = @:privateAccess psys.seed * @:privateAccess psys.animtime;
							// @:privateAccess psys.seed++;
						}
					}
					brushTime += iron.system.Time.delta;
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

		var undoPressed = kb.down("control") && !kb.down("shift") && kb.started("z");
		if (systemId == 'OSX') undoPressed = !kb.down("shift") && kb.started("z"); // cmd+z on macos

		var redoPressed = (kb.down("control") && kb.down("shift") && kb.started("z")) ||
						   kb.down("control") && kb.started("y");
		if (systemId == 'OSX') redoPressed = (kb.down("shift") && kb.started("z")) || kb.started("y"); // cmd+y on macos

		if (undoPressed) History.doUndo();
		else if (redoPressed) History.doRedo();

		if (UITrait.inst.worktab.position == 1) {
			Gizmo.update();
		}

		if (lastCombo != null) ddirty = 0;
		lastCombo = ui.comboSelectedHandle;
	}

	function render(g:kha.graphics2.Graphics) {
		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		renderUI(g);

		// var ready = showFiles || dirty;
		// TODO: Texture params get overwritten
		// if (ready) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
		// if (UINodes.inst._matcon != null) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
	}

	function renderCursor(g:kha.graphics2.Graphics) {
		// if (cursorImg == null) {
		// 	g.end();
		// 	cursorImg = kha.Image.createRenderTarget(256, 256);
		// 	cursorImg.g2.begin(true, 0x00000000);
		// 	cursorImg.g2.color = 0xffcccccc;
		// 	kha.graphics2.GraphicsExtension.drawCircle(cursorImg.g2, 128, 128, 124, 8);
		// 	cursorImg.g2.end();
		// 	g.begin(false);
		// }

		g.color = 0xffffffff;

		// Brush
		if (arm.App.uienabled && worktab.position == 0 && !brush3d) {
			var cursorImg = Res.get('cursor.png');
			var mouse = iron.system.Input.getMouse();
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
			var pen = iron.system.Input.getPen();
			if (pen.down()) {
				mx = pen.x + iron.App.x();
				my = pen.y + iron.App.y();
			}

			// Radius being scaled
			if (brushLocked) {
				mx += lockStartedX - kha.System.windowWidth() / 2;
				my += lockStartedY - kha.System.windowHeight() / 2;
			}

			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));

			var decal = selectedTool == ToolDecal || selectedTool == ToolText;
			if (decal) {
				psize = Std.int(256 * (brushRadius * brushNodesRadius));
				#if kha_direct3d11
				g.drawScaledImage(decalImage, mx - psize / 2, my - psize / 2, psize, psize);
				#else
				g.drawScaledImage(decalImage, mx - psize / 2, my - psize / 2 + psize, psize, -psize);
				#end
			}
			else {
				if (selectedTool == ToolBrush  ||
					selectedTool == ToolEraser ||
					selectedTool == ToolClone  ||
					selectedTool == ToolBlur   ||
					selectedTool == ToolParticle) {
					g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
				}
				else if (selectedTool == ToolPicker && UITrait.inst.pickerSelectMaterial) {
					// Show picked material next to cursor
					var img = selectedMaterial.imageIcon;
					g.drawImage(img, mx + 10, my + 10);
				}
			}

			if (mirrorX) {
				var cx = iron.App.x() + iron.App.w() / 2;
				var nx = cx + (cx - mx);
				// Separator line
				g.color = 0x66ffffff;
				g.fillRect(cx - 1, iron.App.y(), 2, iron.App.h());
				if (decal) {
					#if kha_direct3d11
					g.drawScaledImage(decalImage, nx - psize / 2, my - psize / 2, psize, psize);
					#else
					g.drawScaledImage(decalImage, nx - psize / 2, my - psize / 2 + psize, psize, -psize);
					#end
				}
				else { // Cursor
					g.drawScaledImage(cursorImg, nx - psize / 2, my - psize / 2, psize, psize);
				}
				g.color = 0xffffffff;
			}

			var kb = iron.system.Input.getKeyboard();
			if (selectedTool == ToolClone && !kb.down("alt") && (mouse.down() || pen.down())) { // Clone source cursor
				g.color = 0x66ffffff;
				g.drawScaledImage(cursorImg, mx + cloneDeltaX * iron.App.w() - psize / 2, my + cloneDeltaY * iron.App.h() - psize / 2, psize, psize);
				g.color = 0xffffffff;
			}

			if (showGrid) {
				// Separator line
				var x1 = iron.App.x() + iron.App.w() / 3;
				var x2 = iron.App.x() + iron.App.w() / 3 * 2;
				var y1 = iron.App.y() + iron.App.h() / 3;
				var y2 = iron.App.y() + iron.App.h() / 3 * 2;
				g.color = 0x66ffffff;
				g.fillRect(x1 - 1, iron.App.y(), 2, iron.App.h());
				g.fillRect(x2 - 1, iron.App.y(), 2, iron.App.h());
				g.fillRect(iron.App.x(), y1 - 1, iron.App.x() + iron.App.w(), 2);
				g.fillRect(iron.App.x(), y2 - 1, iron.App.x() + iron.App.w(), 2);
				g.color = 0xffffffff;
			}
		}
	}

	function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		if (UINodes.inst.show && UINodes.inst.canvasType == 0) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 0; }
		arm.App.resize();
	}

	function showBrushNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		if (UINodes.inst.show && UINodes.inst.canvasType == 1) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = 1; }
		arm.App.resize();
	}

	// function showLogicNodes() {
	//	// Clear input state as ui receives input events even when not drawn
	//	@:privateAccess UINodes.inst.ui.endInput();

	// 	if (UINodes.inst.show && UINodes.inst.canvasType == 2) UINodes.inst.show = false;
	// 	else { UINodes.inst.show = true; UINodes.inst.canvasType = 2; }
	// 	arm.App.resize();
	// }

	// function showParticleNodes() {}

	public function show2DView(type = 0) {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UIView2D.inst.ui.endInput();
		if (UIView2D.inst.type != type) UIView2D.inst.show = true;
		else UIView2D.inst.show = !UIView2D.inst.show;
		UIView2D.inst.type = type;
		arm.App.resize();
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

		if (UITrait.inst.worktab.position == 1) {
			if (Std.is(o, MeshObject)) {
				for (i in 0...materials2.length) {
					if (materials2[i].data == cast(o, MeshObject).materials[0]) {
						// selectMaterial(i); // loop
						selectedMaterial2 = materials2[i];
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

	public function getImage(asset:TAsset):kha.Image {
		return asset != null ? Canvas.assetMap.get(asset.id) : null;
	}

	public function mainObject():MeshObject {
		for (po in paintObjects) if (po.children.length > 0) return po;
		return paintObjects[0];
	}

	// var cursorImg:kha.Image = null;

	function renderUI(g:kha.graphics2.Graphics) {
		
		if (!arm.App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (arm.App.uienabled && !ui.inputRegistered) ui.registerInput();

		if (!show) return;

		g.end();
		ui.begin(g);

		// ui.t.FILL_WINDOW_BG = false;
		var panelx = (iron.App.x() - toolbarw);
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(toolbarHandle, panelx, headerh, toolbarw, kha.System.windowHeight())) {
			ui._y += 2;

			ui.imageScrollAlign = false;

			if (UITrait.inst.worktab.position == 0) {
				var keys = ['(B)', '(E)', '(G)', '(D)', '(T)', '(L) - Hold ALT to set source', '(U)', '(P)', '(K)', '(C)', '(V)'];
				var img = Res.get("icons.png");
				var imgw = ui.SCALE >= 2 ? 100 : 50;
				for (i in 0...toolNames.length) {
					ui._x += 2;
					if (selectedTool == i) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
					if (ui.image(img, -1, null, i * imgw, 0, imgw, imgw) == State.Started) selectTool(i);
					if (ui.isHovered) ui.tooltip(toolNames[i] + " " + keys[i]);
					ui._x -= 2;
					ui._y += 2;
				}
			}
			else if (UITrait.inst.worktab.position == 1) {
				var img = Res.get("icons.png");
				var imgw = ui.SCALE >= 2 ? 100 : 50;
				ui._x += 2;
				if (selectedTool == ToolGizmo) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip("Gizmo (G)");
				ui._x -= 2;
				ui._y += 2;
			}

			ui.imageScrollAlign = true;
		}
		// ui.t.FILL_WINDOW_BG = true;

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

			if (ui.button("File", Left) || (arm.App.showMenu && ui.isHovered)) { arm.App.showMenu = true; UIMenu.menuCategory = 0; };
			if (ui.button("Edit", Left) || (arm.App.showMenu && ui.isHovered)) { arm.App.showMenu = true; UIMenu.menuCategory = 1; };
			if (ui.button("View", Left) || (arm.App.showMenu && ui.isHovered)) { arm.App.showMenu = true; UIMenu.menuCategory = 2; };
			if (ui.button("Help", Left) || (arm.App.showMenu && ui.isHovered)) { arm.App.showMenu = true; UIMenu.menuCategory = 3; };
			
			ui._w = _w;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.BUTTON_COL = BUTTON_COL;
		}
		ui.t.WINDOW_BG_COL = WINDOW_BG_COL;

		var panelx = (iron.App.x() - toolbarw) + menubarw;
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(workspaceHandle, panelx, 0, kha.System.windowWidth() - windowW - menubarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			ui.tab(worktab, "Paint");
			ui.tab(worktab, "Scene");
			ui.tab(worktab, "Material");
			if (worktab.changed) {
				ddirty = 2;
				toolbarHandle.redraws = 2;
				headerHandle.redraws = 2;
				hwnd.redraws = 2;
				hwnd1.redraws = 2;
				hwnd2.redraws = 2;

				if (worktab.position == 1) {
					selectTool(ToolGizmo);
				}

				if (worktab.position == 2) {
					MaterialParser.parseMeshPreviewMaterial();
					mainObject().skip_context = "paint";
				}
				else {
					MaterialParser.parseMeshMaterial();
					mainObject().skip_context = null;
				}
			}
		}

		var panelx = iron.App.x();
		if (App.C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(headerHandle, panelx, headerh, kha.System.windowWidth() - toolbarw - windowW, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {

			if (UITrait.inst.worktab.position == 0) {

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
					if (selectedTool == ToolBrush || selectedTool == ToolEraser || selectedTool == ToolFill || selectedTool == ToolClone || selectedTool == ToolBlur) {
						brushBias = ui.slider(Id.handle({value: brushBias}), "Bias", 0.0, 1.0, true);
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
						ui.combo(fillTypeHandle, ["Object", "Face"], "Fill Mode");
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

						xray = ui.check(Id.handle({selected: xray}), "X-Ray");

						var mirrorHandle = Id.handle({selected: mirrorX});
						ui._w = Std.int(60 * sc);
						mirrorX = ui.check(mirrorHandle, "Mirror");
						if (mirrorHandle.changed) {
							UINodes.inst.updateCanvasMap();
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
			else if (UITrait.inst.worktab.position == 1) {
				// ui.button("Clone");
			}
		}

		if (ui.window(statusHandle, iron.App.x(), kha.System.windowHeight() - headerh, kha.System.windowWidth() - toolbarw - windowW, headerh)) {

			var scene = iron.Scene.active;
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
				ui._w = Std.int(ui.ops.font.width(ui.fontSize, message) + 50 * ui.SCALE);
				ui.fill(0, 0, ui._w, ui._h, messageColor);
				ui.text(message);
				ui._w = _w;
			}
		}
		
		tabx = App.C.ui_layout == 0 ? kha.System.windowWidth() - windowW : 0;
		tabh = Std.int(kha.System.windowHeight() / 3);
		gizmo.visible = false;
		// grid.visible = false;
		var work = worktab.position;

		if (work == 0) { // Paint
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				tabLayers();
				tabHistory();
				tabPlugins();
				tabPreferences();
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh)) {
				selectedObject = paintObject;
				tabMaterials();
				tabBrushes();
				tabParticles();
			}
			if (ui.window(hwnd2, tabx, tabh * 2, windowW, tabh)) {
				tabTextures();
				tabMeshes();
				tabExport();
				tabViewport();
			}
		}
		else if (work == 1) { // Scene
			gizmo.visible = true;
			// grid.visible = true;
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				tabOutliner();
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh)) {
				tabProperties();
			}
			if (ui.window(hwnd2, tabx, tabh * 2, windowW, tabh)) {
			}
		}
		else if (work == 2) { // Material
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				tabHistory();
				tabPlugins();
				tabPreferences();
				
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh)) {
				selectedObject = paintObject;
				tabMaterials();
			}
			if (ui.window(hwnd2, tabx, tabh * 2, windowW, tabh)) {
				tabTextures();
				tabMeshes();
				tabExport();
				tabViewport();
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
			Layers.updateFillLayers(8);
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

	function tabLayers() {
		if (ui.tab(htab, "Layers")) {
			ui.row([1/4,1/4,1/2]);
			if (ui.button("New")) newLayer();
			if (ui.button("2D View")) show2DView();
			else if (ui.isHovered) ui.tooltip("Show 2D View (SHIFT+TAB)");
			
			var ar = ["All"];
			for (p in paintObjects) ar.push(p.name);
			var filterHandle = Id.handle();
			layerFilter = ui.combo(filterHandle, ar, "Filter");
			if (filterHandle.changed) {
				for (p in paintObjects) {
					p.visible = layerFilter == 0 || p.name == ar[layerFilter];
					setObjectMask();
				}
				ddirty = 2;
			}

			function drawList(l:LayerSlot, i:Int) {

				if (layerFilter > 0 &&
					l.objectMask > 0 &&
					l.objectMask != layerFilter) return;

				var h = Id.handle().nest(l.id, {selected: l.visible});
				var layerPanel = h.nest(0, {selected: false});
				var off = ui.t.ELEMENT_OFFSET;
				var step = ui.t.ELEMENT_H;
				var checkw = (ui._windowW / 100 * 8) / ui.SCALE;

				if (layerPanel.selected) {
					ui.fill(checkw, step * 2, (ui._windowW / ui.SCALE - 2) - checkw, step + off, ui.t.SEPARATOR_COL);
				}

				if (selectedLayer == l) {
					if (selectedLayerIsMask) {
						ui.rect(ui._windowW / 100 * 24 - 2, 0, ui._windowW / 100 * 16, step * 2, ui.t.HIGHLIGHT_COL, 2);
					}
					else {
						ui.fill(checkw, 0, (ui._windowW / ui.SCALE - 2) - checkw, step * 2, ui.t.HIGHLIGHT_COL);
					}
				}

				if (l.texpaint_mask != null) {
					ui.row([8/100, 16/100, 16/100, 20/100, 30/100, 10/100]);
				}
				else {
					ui.row([8/100, 16/100, 36/100, 30/100, 10/100]);
				}
				
				var center = (step / 2) * ui.SCALE;
				ui._y += center;
				l.visible = ui.check(h, "");
				if (h.changed) {
					MaterialParser.parseMeshMaterial();
				}
				ui._y -= center;

				var contextMenu = false;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = l.material_mask != null;
				#end

				ui._y += 3;
				var state = ui.image(l.material_mask == null ? l.texpaint_preview : l.material_mask.imageIcon);
				ui._y -= 3;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = false;
				#end

				if (ui.isHovered) {
					ui.tooltipImage(l.texpaint_preview);
				}
				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}
				if (ui.isReleased) {
					setLayer(l);
				}
				if (state == State.Started) {
					if (iron.system.Time.time() - selectTime < 0.25) show2DView();
					selectTime = iron.system.Time.time();
				}

				// if (l.material_mask != null) {
				// 	var img = l.material_mask.imageIcon;
				// 	var w = img.width / 2;
				// 	var x = ui._x - w * 1.13;
				// 	var y = ui._y + w * 0.63;
				// 	ui.g.color = 0xff000000;
				// 	ui.g.fillRect(x, y, w, w);
				// 	ui.g.color = 0xffffffff;
				// 	#if kha_direct3d11
				// 	ui.g.drawScaledImage(img, x, y, w, w);
				// 	#else
				// 	ui.g.drawScaledImage(img, x, y + w, w, -w);
				// 	#end
				// }

				if (l.texpaint_mask != null) {
					ui._y += 3;
					var state = ui.image(l.texpaint_mask_preview);
					ui._y -= 3;
					if (ui.isHovered) {
						ui.tooltipImage(l.texpaint_mask_preview);
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.show(function(ui:Zui) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 3, ui.t.SEPARATOR_COL);
							ui.text(l.name + " Mask", Right);
							if (ui.button("Delete", Left)) {
								l.deleteMask();
								setLayer(l);
							}
							if (ui.button("Apply", Left)) {
								setLayer(l);
								l.applyMask();
								setLayer(l); // Parse mesh material
							}
						});
					}
					if (ui.isReleased) {
						setLayer(l, true);
					}
					if (state == State.Started) {
						if (iron.system.Time.time() - selectTime < 0.25) show2DView();
						selectTime = iron.system.Time.time();
					}
				}

				ui._y += center;
				ui.text(l.name);
				ui._y -= center;

				if (ui.isReleased) {
					setLayer(l);
				}

				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}

				if (contextMenu) {
					UIMenu.show(function(ui:Zui) {
						if (l == layers[0]) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 10, ui.t.SEPARATOR_COL);
							ui.text(l.name, Right);
						}
						else {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 17, ui.t.SEPARATOR_COL);
							ui.text(l.name, Right);
						}

						if (l.material_mask == null && ui.button("To Fill Layer", Left)) {
							toFillLayer(l);
						}
						if (l.material_mask != null && ui.button("To Paint Layer", Left)) {
							toPaintLayer(l);
						}

						if (l == layers[0]) {
						}
						else {
							if (ui.button("Delete", Left)) {
								selectedLayer = l;
								Layers.deleteSelectedLayer();
							}
							if (ui.button("Move Up", Left)) {
								if (i < layers.length - 1) {
									setLayer(l);
									var t = layers[i + 1];
									layers[i + 1] = layers[i];
									layers[i] = t;
									hwnd.redraws = 2;
								}
							}
							if (ui.button("Move Down", Left)) {
								if (i > 1) {
									setLayer(l);
									var t = layers[i - 1];
									layers[i - 1] = layers[i];
									layers[i] = t;
									hwnd.redraws = 2;
								}
							}
							if (ui.button("Merge Down", Left)) {
								setLayer(l);
								iron.App.notifyOnRender(Layers.mergeSelectedLayer);
							}
							if (ui.button("Duplicate", Left)) {
								setLayer(l);
								function makeDupli(g:kha.graphics4.Graphics) {
									g.end();
									l = l.duplicate();
									setLayer(l);
									g.begin();
									iron.App.removeRender(makeDupli);
								}
								iron.App.notifyOnRender(makeDupli);
							}
							if (ui.button("Black Mask", Left)) {
								l.createMask(0x00000000);
								setLayer(l, true);
								layerPreviewDirty = true;
							}
							if (ui.button("White Mask", Left)) {
								l.createMask(0xffffffff);
								setLayer(l, true);
								layerPreviewDirty = true;
							}
						}

						var baseHandle = Id.handle().nest(l.id, {selected: l.paintBase});
						var norHandle = Id.handle().nest(l.id, {selected: l.paintNor});
						var occHandle = Id.handle().nest(l.id, {selected: l.paintOcc});
						var roughHandle = Id.handle().nest(l.id, {selected: l.paintRough});
						var metHandle = Id.handle().nest(l.id, {selected: l.paintMet});
						var heightHandle = Id.handle().nest(l.id, {selected: l.paintHeight});
						var emisHandle = Id.handle().nest(l.id, {selected: l.paintEmis});
						var subsHandle = Id.handle().nest(l.id, {selected: l.paintSubs});
						l.paintBase = ui.check(baseHandle, "Base Color");
						l.paintNor = ui.check(norHandle, "Normal");
						l.paintOcc = ui.check(occHandle, "Occlusion");
						l.paintRough = ui.check(roughHandle, "Roughness");
						l.paintMet = ui.check(metHandle, "Metallic");
						l.paintHeight = ui.check(heightHandle, "Height");
						l.paintEmis = ui.check(emisHandle, "Emission");
						l.paintSubs = ui.check(subsHandle, "Subsurface");
						if (baseHandle.changed ||
							norHandle.changed ||
							occHandle.changed ||
							roughHandle.changed ||
							metHandle.changed ||
							heightHandle.changed ||
							emisHandle.changed ||
							subsHandle.changed) {
							MaterialParser.parseMeshMaterial();
							UIMenu.propChanged = true;
						}
					});
				}

				if (i == 0) {
					@:privateAccess ui.endElement();
				}
				else {
					var blend = ui.combo(Id.handle(), ["Add"], "Blending");
				}

				ui._y += center;
				var showPanel = ui.panel(layerPanel, "", 0, true);
				ui._y -= center;

				if (i == 0) {
					ui._y -= ui.t.ELEMENT_OFFSET;
					@:privateAccess ui.endElement();
				}
				else {
					ui._y -= ui.t.ELEMENT_OFFSET;
					ui.row([8/100, 16/100, 36/100, 30/100, 10/100]);
					@:privateAccess ui.endElement();
					@:privateAccess ui.endElement();
					@:privateAccess ui.endElement();

					var ar = ["Shared"];
					for (p in paintObjects) ar.push(p.name);
					var h = Id.handle().nest(l.id);
					h.position = l.objectMask;
					l.objectMask = ui.combo(h, ar, "Object");
					if (h.changed) {
						setLayer(l);
						Layers.updateFillLayers(8);
					}
					@:privateAccess ui.endElement();
				}
				ui._y -= ui.t.ELEMENT_OFFSET;

				if (showPanel) {
					ui.row([8/100,92/100]);
					@:privateAccess ui.endElement();
					var opacHandle = Id.handle().nest(l.id, {value: l.maskOpacity});
					l.maskOpacity = ui.slider(opacHandle, "Opacity", 0.0, 1.0, true);
					if (opacHandle.changed) {
						MaterialParser.parseMeshMaterial();
					}
				}
			}

			for (i in 0...layers.length) {
				if (i >= layers.length) break; // Layer was deleted
				var j = layers.length - 1 - i;
				var l = layers[j];
				drawList(l, j);
			}
		}
	}

	function tabHistory() {
		if (ui.tab(htab, "History")) {
			for (i in 0...History.stack.length) {
				var active = History.stack.length - 1 - redos;
				if (i == active) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				ui.text(History.stack[i]);
				if (ui.isReleased) { // Jump to undo step
					var diff = i - active;
					while (diff > 0) { diff--; History.doRedo(); }
					while (diff < 0) { diff++; History.doUndo(); }
				}
			}
		}
	}

	function tabPlugins() {
		if (ui.tab(htab, "Plugins")) {
			if (ui.panel(Id.handle({selected: false}), "Console", 1)) {
				Console.render(ui);
			}
			ui.separator();

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}

	function tabPreferences() {
		if (ui.tab(htab, "Preferences")) {
			if (ui.panel(Id.handle({selected: false}), "Interface", 1)) {
				var hscale = Id.handle({value: App.C.window_scale});
				ui.slider(hscale, "UI Scale", 0.5, 4.0, true);
				if (!hscale.changed && hscaleWasChanged) {
					#if kha_krom
					if (hscale.value == null || Math.isNaN(hscale.value)) hscale.value = 1.0;
					#end
					App.C.window_scale = hscale.value;
					ui.setScale(hscale.value);
					arm.App.uibox.setScale(hscale.value);
					UINodes.inst.ui.setScale(hscale.value);
					UIView2D.inst.ui.setScale(hscale.value);
					windowW = Std.int(defaultWindowW * App.C.window_scale);
					toolbarw = Std.int(defaultToolbarW * App.C.window_scale);
					headerh = Std.int(defaultHeaderH * App.C.window_scale);
					menubarw = Std.int(215 * App.C.window_scale);
					arm.App.resize();
					armory.data.Config.save();
					if (ui.SCALE >= 2) {
						Res.load(["icons2x.png"], function() {
							@:privateAccess Res.bundled.set("icons.png", Res.get("icons2x.png"));
						});
					}
					else {
						Res.load(["icons.png"], function() {});
					}
				}
				hscaleWasChanged = hscale.changed;
				ui.row([1/2, 1/2]);
				var layHandle = Id.handle({position: App.C.ui_layout});
				App.C.ui_layout = ui.combo(layHandle, ["Right", "Left"], "Layout", true);
				if (layHandle.changed) {
					arm.App.resize();
					armory.data.Config.save();
				}
				var themeHandle = Id.handle({position: 0});
				var themes = ["Dark", "Light"];
				ui.combo(themeHandle, themes, "Theme", true);
				if (themeHandle.changed) {
					iron.data.Data.getBlob("themes/theme_" + themes[themeHandle.position].toLowerCase() + ".arm", function(b:kha.Blob) {
						arm.App.parseTheme(b);
						ui.t = arm.App.theme;
						// UINodes.inst.applyTheme();
						headerHandle.redraws = 2;
						toolbarHandle.redraws = 2;
						statusHandle.redraws = 2;
						workspaceHandle.redraws = 2;
						menuHandle.redraws = 2;
						hwnd.redraws = 2;
						hwnd1.redraws = 2;
						hwnd2.redraws = 2;
					});
				}
				// var gridSnap = ui.check(Id.handle({selected: false}), "Node Grid Snap");
			}

			ui.separator();
			if (ui.panel(Id.handle({selected: false}), "Usage", 1)) {
				undoHandle = Id.handle({value: App.C.undo_steps});
				App.C.undo_steps = Std.int(ui.slider(undoHandle, "Undo Steps", 1, 64, false, 1));
				if (undoHandle.changed) {
					ui.g.end();
					while (undoLayers.length < App.C.undo_steps) {
						var l = new LayerSlot("_undo" + undoLayers.length);
						l.createMask(0, false);
						undoLayers.push(l);
					}
					while (undoLayers.length > App.C.undo_steps) {
						var l = undoLayers.pop();
						l.unload();
					}
					undos = 0;
					redos = 0;
					undoI = 0;
					ui.g.begin(false);
					armory.data.Config.save();
				}
				var brush3dHandle = Id.handle({selected: brush3d});
				brush3d = ui.check(brush3dHandle, "3D Brush Cursor");
				if (brush3dHandle.changed) MaterialParser.parsePaintMaterial();
			}

			ui.separator();
			if (ui.panel(Id.handle({selected: false}), "Pen Pressure", 1)) {
				penPressureRadius = ui.check(Id.handle({selected: penPressureRadius}), "Brush Radius");
				penPressureOpacity = ui.check(Id.handle({selected: penPressureOpacity}), "Brush Opacity");
				penPressureHardness = ui.check(Id.handle({selected: penPressureHardness}), "Brush Hardness");
			}

			#if arm_debug
			iron.Scene.active.sceneParent.getTrait(armory.trait.internal.DebugConsole).visible = ui.check(Id.handle({selected: false}), "Debug Console");
			#end

			hssgi = Id.handle({selected: App.C.rp_ssgi});
			hssr = Id.handle({selected: App.C.rp_ssr});
			hbloom = Id.handle({selected: App.C.rp_bloom});
			hsupersample = Id.handle({position: Config.getSuperSampleQuality(App.C.rp_supersample)});
			hvxao = Id.handle({selected: App.C.rp_gi});
			ui.separator();
			if (ui.panel(Id.handle({selected: false}), "Viewport Quality", 1)) {
				ui.row([1/2, 1/2]);
				var vsyncHandle = Id.handle({selected: App.C.window_vsync});
				App.C.window_vsync = ui.check(vsyncHandle, "VSync");
				if (vsyncHandle.changed) armory.data.Config.save();
				ui.combo(hsupersample, ["1.0x", "1.5x", "2.0x", "4.0x"], "Super Sample", true);
				if (hsupersample.changed) Config.applyConfig();
				ui.row([1/2, 1/2]);
				ui.check(hvxao, "Voxel AO");
				if (ui.isHovered) ui.tooltip("Cone-traced AO and shadows");
				if (hvxao.changed) Config.applyConfig();
				ui.check(hssgi, "SSAO");
				if (hssgi.changed) Config.applyConfig();
				ui.row([1/2, 1/2]);
				ui.check(hbloom, "Bloom");
				if (hbloom.changed) Config.applyConfig();
				ui.check(hssr, "SSR");
				if (hssr.changed) Config.applyConfig();
				var cullHandle = Id.handle({selected: culling});
				culling = ui.check(cullHandle, "Cull Backfaces");
				if (cullHandle.changed) {
					MaterialParser.parseMeshMaterial();
				}
			}

			ui.separator();
			if (ui.panel(Id.handle({selected: false}), "Keymap", 1)) {
				for (k in Reflect.fields(App.C.keymap)) {
					ui.button(k, Left, Reflect.field(App.C.keymap, k));
				}
			}

			// ui.separator();
			// ui.button("Restore Defaults");
			// ui.button("Confirm");
		}
	}

	function tabMaterials() {
		if (ui.tab(htab1, "Materials")) {
			ui.row([1/4,1/4,1/4]);
			if (ui.button("New")) {
				ui.g.end();
				headerHandle.redraws = 2;
				selectedMaterial = new MaterialSlot(materials[0].data);
				materials.push(selectedMaterial);
				UINodes.inst.updateCanvasMap();
				MaterialParser.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
				var decal = selectedTool == ToolDecal || selectedTool == ToolText;
				if (decal) RenderUtil.makeDecalPreview();
				ui.g.begin(false);
			}

			if (ui.button("Import")) {
				arm.App.showFiles = true;
				@:privateAccess Ext.lastPath = ""; // Refresh
				arm.App.whandle.redraws = 2;
				arm.App.foldersOnly = false;
				arm.App.showFilename = false;
				UIFiles.filters = "arm,blend";
				arm.App.filesDone = function(path:String) {
					StringTools.endsWith(path, ".blend") ?
						Importer.importBlendMaterials(path) :
						Importer.importArmMaterials(path);
				}
			}

			if (ui.button("Nodes")) {
				showMaterialNodes();
			}
			else if (ui.isHovered) ui.tooltip("Show Node Editor (TAB)");

			for (row in 0...Std.int(Math.ceil(materials.length / 5))) { 
				ui.row([1/5,1/5,1/5,1/5,1/5]);

				if (row > 0) ui._y += 6;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = true; // Material preview
				#end

				for (j in 0...5) {
					var i = j + row * 5;
					if (i >= materials.length) {
						@:privateAccess ui.endElement();
						continue;
					}
					var img = ui.SCALE >= 2.0 ? materials[i].image : materials[i].imageIcon;
					var imgFull = materials[i].image;

					if (selectedMaterial == materials[i]) {
						// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						var off = row % 2 == 1 ? 1 : 0;
						var w = 51 - App.C.window_scale;
						ui.fill(1,          -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(1,     w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(1,          -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 3,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					var imgw = ui.SCALE >= 2 ? 100 : 50;
					var uix = ui._x;
					var uiy = ui._y;
					var state = materialPreviewReady ? ui.image(img) : ui.image(Res.get('icons.png'), -1, null, imgw, imgw, imgw, imgw);
					if (state == State.Started) {
						if (selectedMaterial != materials[i]) selectMaterial(i);
						if (iron.system.Time.time() - selectTime < 0.25) showMaterialNodes();
						selectTime = iron.system.Time.time();
						var mouse = iron.system.Input.getMouse();
						arm.App.dragOffX = -(mouse.x - uix - ui._windowX + iron.App.x() - 3);
						arm.App.dragOffY = -(mouse.y - uiy - ui._windowY + iron.App.y() + 1);
						arm.App.dragMaterial = selectedMaterial;
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.show(function(ui:Zui) {
							var m = materials[i];
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 10, ui.t.SEPARATOR_COL);
							ui.text(UINodes.inst.canvasMap.get(materials[i]).name, Right);
							
							if (ui.button("Delete", Left) && materials.length > 1) {
								selectMaterial(i == 0 ? 1 : 0);
								materials.splice(i, 1);
								hwnd1.redraws = 2;
							}
							
							var baseHandle = Id.handle().nest(m.id, {selected: m.paintBase});
							var norHandle = Id.handle().nest(m.id, {selected: m.paintNor});
							var occHandle = Id.handle().nest(m.id, {selected: m.paintOcc});
							var roughHandle = Id.handle().nest(m.id, {selected: m.paintRough});
							var metHandle = Id.handle().nest(m.id, {selected: m.paintMet});
							var heightHandle = Id.handle().nest(m.id, {selected: m.paintHeight});
							var emisHandle = Id.handle().nest(m.id, {selected: m.paintEmis});
							var subsHandle = Id.handle().nest(m.id, {selected: m.paintSubs});
							m.paintBase = ui.check(baseHandle, "Base Color");
							m.paintNor = ui.check(norHandle, "Normal");
							m.paintOcc = ui.check(occHandle, "Occlusion");
							m.paintRough = ui.check(roughHandle, "Roughness");
							m.paintMet = ui.check(metHandle, "Metallic");
							m.paintHeight = ui.check(heightHandle, "Height");
							m.paintEmis = ui.check(emisHandle, "Emission");
							m.paintSubs = ui.check(subsHandle, "Subsurface");
							if (baseHandle.changed ||
								norHandle.changed ||
								occHandle.changed ||
								roughHandle.changed ||
								metHandle.changed ||
								heightHandle.changed ||
								emisHandle.changed ||
								subsHandle.changed) {
								UINodes.inst.updateCanvasMap();
								MaterialParser.parsePaintMaterial();
								UIMenu.propChanged = true;
							}
						});
					}
					if (ui.isHovered) ui.tooltipImage(imgFull);
				}

				ui._y += 6;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = false; // Material preview
				#end
			}
		}
	}

	function tabBrushes() {
		if (ui.tab(htab1, "Brushes")) {
			ui.row([1/4,1/4]);
			if (ui.button("New")) {}
			if (ui.button("Nodes")) showBrushNodes();
		}
	}

	function tabParticles() {
		if (ui.tab(htab1, "Particles")) {
			ui.row([1/4,1/4]);
			if (ui.button("New")) {}
			if (ui.button("Nodes")) {}
		}
	}

	function tabTextures() {
		if (ui.tab(htab2, "Textures")) {
			ui.row([1/4, 1/4]);

			if (ui.button("Import")) {
				arm.App.showFiles = true;
				@:privateAccess Ext.lastPath = ""; // Refresh
				arm.App.whandle.redraws = 2;
				arm.App.foldersOnly = false;
				arm.App.showFilename = false;
				UIFiles.filters = "jpg,png,tga,hdr";
				arm.App.filesDone = function(path:String) {
					Importer.importFile(path);
				}
			}
			if (ui.isHovered) ui.tooltip("Import texture file (Ctrl + Shift + I)");

			if (ui.button("2D View")) show2DView(1);

			if (assets.length > 0) {
				for (i in 0...assets.length) {
					
					// Align into 5 items per row
					if (i % 5 == 0) {
						ui._y += ui.ELEMENT_OFFSET() * 1.5;
						ui.row([1/5, 1/5, 1/5, 1/5, 1/5]);
					}
					
					var asset = assets[i];
					if (asset == selectedTexture) {
						var off = i % 2 == 1 ? 1 : 0;
						var w = 51 - App.C.window_scale;
						ui.fill(1,          -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(1,     w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(1,          -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 3,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					var img = UITrait.inst.getImage(asset);
					var uix = ui._x;
					var uiy = ui._y;
					if (ui.image(img) == State.Started) {
						var mouse = iron.system.Input.getMouse();
						arm.App.dragOffX = -(mouse.x - uix - ui._windowX + iron.App.x() - 3);
						arm.App.dragOffY = -(mouse.y - uiy - ui._windowY + iron.App.y() + 1);
						arm.App.dragAsset = asset;
						selectedTexture = asset;

						if (iron.system.Time.time() - selectTime < 0.25) show2DView(1);
						selectTime = iron.system.Time.time();
					}

					if (ui.isHovered) ui.tooltipImage(img, 256);

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.show(function(ui:Zui) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 2, ui.t.SEPARATOR_COL);
							ui.text(asset.name, Right);
							if (ui.button("Delete", Left)) {
								hwnd2.redraws = 2;
								iron.data.Data.deleteImage(asset.file);
								Canvas.assetMap.remove(asset.id);
								assets.splice(i, 1);
								assetNames.splice(i, 1);
							}
						});
					}
				}

				// Fill in unused row space
				if (assets.length % 5 > 0) {
					for (i in 0...5 - (assets.length % 5)) {
						@:privateAccess ui.endElement();
					}
				}
			}
			else {
				var img = Res.get('icons.png');
				var imgw = ui.SCALE >= 2 ? 100 : 50;
				ui.image(img, ui.t.BUTTON_COL, imgw, 0, imgw, imgw, imgw);
				if (ui.isHovered) ui.tooltip("Drag and drop files here");
			}
		}
	}

	function tabMeshes() {
		if (ui.tab(htab2, "Meshes")) {
			ui.row([1/4]);

			if (ui.button("Import")) {
				arm.App.showFiles = true;
				@:privateAccess Ext.lastPath = ""; // Refresh
				arm.App.whandle.redraws = 2;
				arm.App.foldersOnly = false;
				arm.App.showFilename = false;
				UIFiles.filters = "obj,fbx,blend,gltf,arm";
				arm.App.filesDone = function(path:String) {
					Importer.importFile(path);
				}
			}
			if (ui.isHovered) ui.tooltip("Import mesh file (Ctrl + Shift + I)");

			isUdim = ui.check(Id.handle({selected: isUdim}), "UDIM import");
			if (ui.isHovered) ui.tooltip("Split mesh per UDIM tile");

			if (ui.panel(Id.handle({selected: false}), "Scene", 0, true)) {
				var i = 0;
				function drawList(h:Handle, o:MeshObject) {
					// if (paintObject == o) {
						// ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, 0xff205d9c);
					// }
					// ui.row([1/10, 9/10]);
					// var h = Id.handle().nest(i, {selected: o.visible});
					// o.visible = ui.check(h, "");
					// if (h.changed) ddirty = 2;
					ui.text(o.name);
					// if (ui.isReleased) {
					// 	selectPaintObject(o);
					// }
					i++;
				}
				ui.indent();
				for (c in paintObjects) {
					drawList(Id.handle(), c);
				}
				ui.unindent();
			}

			if (ui.panel(Id.handle({selected: false}), "Geometry")) {

				ui.row([1/2,1/2]);
				if (ui.button("Flip Normals")) {
					MeshUtil.flipNormals();
					ddirty = 2;
				}
				if (ui.button("Calculate Normals")) {
					MeshUtil.calcNormals();
					ddirty = 2;
				}

				ui.row([1/2, 1/2]);
				var upHandle = Id.handle();
				var lastUp = upHandle.position;
				// TODO: Turn into axis rotation instead
				var axisUp = ui.combo(upHandle, ["Z", "-Z", "Y", "-Y"], "Up Axis", true);
				if (upHandle.changed && axisUp != lastUp) {
					MeshUtil.switchUpAxis(axisUp);
					ddirty = 2;
				}
				
				var dispHandle = Id.handle({value: displaceStrength});
				displaceStrength = ui.slider(dispHandle, "Displace", 0.0, 2.0, true);
				if (dispHandle.changed) {
					MaterialParser.parseMeshMaterial();
				}
			}
		}
	}

	function tabExport() {
		if (ui.tab(htab2, "Export")) {
			if (ui.panel(Id.handle({selected: true}), "Export Textures", 1)) {
				if (ui.button("Export")) {
					arm.App.showFiles = true;
					@:privateAccess Ext.lastPath = ""; // Refresh
					arm.App.whandle.redraws = 2;
					arm.App.foldersOnly = true;
					arm.App.showFilename = true;
					// var path = 'C:\\Users\\lubos\\Documents\\';
					UIFiles.filters = formatType == 0 ? "jpg" : "png";
					arm.App.filesDone = function(path:String) {
						textureExport = true;
						textureExportPath = path;
					}
				}
				if (ui.isHovered) ui.tooltip("Export texture files (Ctrl + Shift + E)");

				ui.row([1/2, 1/2]);
				ui.combo(resHandle, ["1K", "2K", "4K", "8K", "16K"], "Res", true);
				if (resHandle.changed) {
					iron.App.notifyOnRender(Layers.resizeLayers);
					UVUtil.uvmap = null;
					UVUtil.uvmapCached = false;
					UVUtil.trianglemap = null;
					UVUtil.trianglemapCached = false;
				}
				ui.combo(Id.handle(), ["8bit"], "Color", true);

				if (formatType == 0) {
					ui.row([1/2, 1/2]);
				}
				else {
					ui.row([1/2]);
				}
				formatType = ui.combo(Id.handle({position: formatType}), ["jpg", "png"], "Format", true);
				if (formatType == 0) {
					formatQuality = ui.slider(Id.handle({value: formatQuality}), "Quality", 0.0, 100.0, true, 1);
				}
				
				ui.row([1/2, 1/2]);
				layersExport = ui.combo(Id.handle({position: layersExport}), ["Visible", "Selected"], "Layers", true);
				outputType = ui.combo(Id.handle(), ["Generic", "UE4 (ORM)"], "Output", true);
				
				if (ui.panel(Id.handle({selected: false}), "Channels")) {
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
					isEmis = ui.check(Id.handle({selected: isEmis}), "Emission");
					isEmisSpace = ui.combo(Id.handle({position: isEmisSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isHeight = ui.check(Id.handle({selected: isHeight}), "Height");
					isHeightSpace = ui.combo(Id.handle({position: isHeightSpace}), ["linear", "srgb"], "Space");
					ui.row([1/2, 1/2]);
					isSubs = ui.check(Id.handle({selected: isSubs}), "Subsurface");
					isSubsSpace = ui.combo(Id.handle({position: isSubsSpace}), ["linear", "srgb"], "Space");
				}
			}

			ui.separator();
			if (ui.panel(Id.handle({selected: false}), "Export Mesh", 1)) {
				if (ui.button("Export")) {
					arm.App.showFiles = true;
					@:privateAccess Ext.lastPath = ""; // Refresh
					arm.App.whandle.redraws = 2;
					arm.App.foldersOnly = true;
					arm.App.showFilename = true;
					UIFiles.filters = exportMeshFormat == 0 ? "obj" : "arm";
					arm.App.filesDone = function(path:String) {
						var f = arm.App.filenameHandle.text;
						if (f == "") f = "untitled";
						Exporter.exportMesh(path + "/" + f);
					};
				}
				exportMeshFormat = ui.combo(Id.handle({position: exportMeshFormat}), ["obj", "arm"], "Format", true);
				var mesh = paintObject.data.raw;
				var inda = mesh.index_arrays[0].values;
				var tris = Std.int(inda.length / 3);
				ui.text(tris + " triangles");
			}
		}
	}

	function tabViewport() {
		if (ui.tab(htab2, "Viewport")) {
			if (ui.button("Import Envmap")) {
				arm.App.showFiles = true;
				@:privateAccess Ext.lastPath = ""; // Refresh
				arm.App.whandle.redraws = 2;
				arm.App.foldersOnly = false;
				arm.App.showFilename = false;
				UIFiles.filters = "hdr";
				arm.App.filesDone = function(path:String) {
					if (!StringTools.endsWith(path, ".hdr")) {
						UITrait.inst.showError("Error: .hdr file expected");
						return;
					}
					Importer.importFile(path);
				}
			}

			if (iron.Scene.active.world.probe.radianceMipmaps.length > 0) {
				ui.image(iron.Scene.active.world.probe.radianceMipmaps[0]);
			}

			ui.row([1/2, 1/2]);
			var modeHandle = Id.handle({position: 0});
			viewportMode = ui.combo(modeHandle, ["Render", "Base Color", "Normal Map", "Occlusion", "Roughness", "Metallic", "TexCoord", "Normal", "MaterialID", "Mask"], "Mode");
			if (modeHandle.changed) {
				MaterialParser.parseMeshMaterial();
			}
			var p = iron.Scene.active.world.probe;
			var envHandle = Id.handle({value: p.raw.strength});
			p.raw.strength = ui.slider(envHandle, "Environment", 0.0, 8.0, true);
			if (envHandle.changed) ddirty = 2;
			
			ui.row([1/2, 1/2]);
			if (iron.Scene.active.lights.length > 0) {
				var light = iron.Scene.active.lights[0];

				var sxhandle = Id.handle();
				var f32:kha.FastFloat = light.data.raw.size; // hl fix
				sxhandle.value = f32;
				light.data.raw.size = ui.slider(sxhandle, "Light Size", 0.0, 4.0, true);
				if (sxhandle.changed) ddirty = 2;
				// var syhandle = Id.handle();
				// syhandle.value = light.data.raw.size_y;
				// light.data.raw.size_y = ui.slider(syhandle, "Size Y", 0.0, 4.0, true);
				// if (syhandle.changed) ddirty = 2;
				
				var lhandle = Id.handle();
				lhandle.value = light.data.raw.strength / 1333;
				lhandle.value = Std.int(lhandle.value * 100) / 100;
				light.data.raw.strength = ui.slider(lhandle, "Light", 0.0, 4.0, true) * 1333;
				if (lhandle.changed) ddirty = 2;
			}

			ui.row([1/2, 1/2]);
			drawWireframe = ui.check(wireframeHandle, "Wireframe");
			if (wireframeHandle.changed) {
				ui.g.end();
				UVUtil.cacheUVMap();
				ui.g.begin(false);
				MaterialParser.parseMeshMaterial();
			}
			showGrid = ui.check(Id.handle({selected: showGrid}), "Grid");

			ui.row([1/2, 1/2]);
			showEnvmap = ui.check(showEnvmapHandle, "Envmap");
			if (showEnvmapHandle.changed) {
				var world = iron.Scene.active.world;
				world.loadEnvmap(function(_) {});
				savedEnvmap = world.envmap;
				ddirty = 2;
			}
			var compassHandle = Id.handle({selected: showCompass});
			showCompass = ui.check(compassHandle, "Compass");
			if (compassHandle.changed) ddirty = 2;

			if (showEnvmap) {
				showEnvmapBlur = ui.check(showEnvmapBlurHandle, "Blurred");
				if (showEnvmapBlurHandle.changed) {
					var probe = iron.Scene.active.world.probe;
					savedEnvmap = showEnvmapBlur ? probe.radianceMipmaps[0] : probe.radiance;
					ddirty = 2;
				}
			}
			else {
				if (ui.panel(Id.handle({selected: false}), "Viewport Color")) {
					var hwheel = Id.handle({color: 0xff030303});
					var worldColor:kha.Color = Ext.colorWheel(ui, hwheel);
					if (hwheel.changed) {
						// var b = emptyEnvmap.lock(); // No lock for d3d11
						// b.set(0, worldColor.Rb);
						// b.set(1, worldColor.Gb);
						// b.set(2, worldColor.Bb);
						// emptyEnvmap.unlock();
						// emptyEnvmap.unload(); //
						var b = haxe.io.Bytes.alloc(4);
						b.set(0, worldColor.Rb);
						b.set(1, worldColor.Gb);
						b.set(2, worldColor.Bb);
						b.set(3, 255);
						emptyEnvmap = kha.Image.fromBytes(b, 1, 1);
						ddirty = 2;
					}
				}
			}
			iron.Scene.active.world.envmap = showEnvmap ? savedEnvmap : emptyEnvmap;
		}
	}

	function tabOutliner() {
		if (ui.tab(htab, "Outliner")) {
			var i = 0;
			function drawList(h:Handle, o:Object) {
				if (o.name.charAt(0) == '.') return; // Hidden
				var b = false;
				if (selectedObject == o) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
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
					ddirty = 2;
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
		}
	}

	function tabProperties() {
		if (ui.tab(htab, 'Properties')) {
			if (selectedObject != null) {

				var h = Id.handle();
				h.selected = selectedObject.visible;
				selectedObject.visible = ui.check(h, "Visible");
				if (h.changed) ddirty = 2;

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
				if (h.changed) { loc.x = f; ddirty = 2; }

				h = Id.handle();
				h.text = Math.roundfp(loc.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { loc.y = f; ddirty = 2; }

				h = Id.handle();
				h.text = Math.roundfp(loc.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { loc.z = f; ddirty = 2; }

				ui.row(row4);
				ui.text("Rotation");
				
				h = Id.handle();
				h.text = Math.roundfp(rot.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				var changed = false;
				if (h.changed) { changed = true; rot.x = f; ddirty = 2; }

				h = Id.handle();
				h.text = Math.roundfp(rot.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { changed = true; rot.y = f; ddirty = 2; }

				h = Id.handle();
				h.text = Math.roundfp(rot.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { changed = true; rot.z = f; ddirty = 2; }

				if (changed && selectedObject.name != "Scene") {
					rot.mult(3.141592 / 180);
					selectedObject.transform.rot.fromEuler(rot.x, rot.y, rot.z);
					selectedObject.transform.buildMatrix();
					#if arm_physics
					var rb = selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
					#end
				}

				ui.row(row4);
				ui.text("Scale");
				
				h = Id.handle();
				h.text = Math.roundfp(scale.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				if (h.changed) { scale.x = f; ddirty = 2; }

				h = Id.handle();
				h.text = Math.roundfp(scale.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { scale.y = f; ddirty = 2; }

				h = Id.handle();
				h.text = Math.roundfp(scale.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { scale.z = f; ddirty = 2; }

				selectedObject.transform.dirty = true;

				if (selectedObject.name == "Scene") {
					// ui.image(envThumb);
					var p = iron.Scene.active.world.probe;
					// ui.row([1/2, 1/2]);
					// var envType = ui.combo(Id.handle({position: 0}), ["Outdoor"], "Map");
					var envHandle = Id.handle({value: p.raw.strength});
					p.raw.strength = ui.slider(envHandle, "Strength", 0.0, 5.0, true);
					if (envHandle.changed) {
						ddirty = 2;
					}
				}
				else if (Std.is(selectedObject, iron.object.LightObject)) {
					var light = cast(selectedObject, iron.object.LightObject);
					var lhandle = Id.handle();
					lhandle.value = light.data.raw.strength / 1333;
					lhandle.value = Std.int(lhandle.value * 100) / 100;
					light.data.raw.strength = ui.slider(lhandle, "Strength", 0.0, 4.0, true) * 1333;
					if (lhandle.changed) {
						ddirty = 2;
					}
				}
				else if (Std.is(selectedObject, iron.object.CameraObject)) {
					var scene = iron.Scene.active;
					var cam = scene.cameras[0];
					var fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
					cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
					if (fovHandle.changed) {
						cam.buildProjection();
						ddirty = 2;
					}
				}
			}

			// if (ui.button("LNodes")) showLogicNodes();
		}

		// ui.separator();
		// if (ui.panel(Id.handle({selected: true}), "Materials", 1)) {

		// 	for (row in 0...Std.int(Math.ceil(materials2.length / 5))) { 
		// 		ui.row([1/5,1/5,1/5,1/5,1/5]);

		// 		if (row > 0) ui._y += 6;

		// 		#if (kha_opengl || kha_webgl)
		// 		ui.imageInvertY = true; // Material preview
		// 		#end

		// 		for (j in 0...5) {
		// 			var i = j + row * 5;
		//			if (i >= materials2.length) {
		// 				@:privateAccess ui.endElement();
		//				continue;
		//			}
		// 			var img = materials2[i].imageIcon;

		// 			if (selectedMaterial2 == materials2[i]) {
		// 				// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
		// 				var off = row % 2 == 1 ? 1 : 0;
		// 				var w = 51 - App.C.window_scale;
		// 				ui.fill(1,          -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
		// 				ui.fill(1,     w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
		// 				ui.fill(1,          -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
		// 				ui.fill(w + 3,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
		// 			}

		// 			if (ui.image(img) == State.Started) {
		// 				if (selectedMaterial2 != materials2[i]) selectMaterial2(i);
		// 				if (iron.system.Time.time() - selectTime < 0.3) showMaterialNodes();
		// 				selectTime = iron.system.Time.time();
		// 			}
		// 			if (ui.isHovered) ui.tooltipImage(img);
		// 		}

		// 		#if (kha_opengl || kha_webgl)
		// 		ui.imageInvertY = false; // Material preview
		// 		#end
		// 	}

		// 	ui.row([1/2,1/2]);
		// 	if (ui.button("New")) {

		// 		iron.data.Data.cachedMaterials.remove("SceneMaterial2");
		// 		iron.data.Data.cachedShaders.remove("Material2_data");
		// 		iron.data.Data.cachedSceneRaws.remove("Material2_data");
		// 		// iron.data.Data.cachedBlobs.remove("Material2_data.arm");
		// 		iron.data.Data.getMaterial("Scene", "Material2", function(md:iron.data.MaterialData) {
		// 			md.name = "Material2." + materials2.length;
		// 			ui.g.end();
		// 			var m = new MaterialSlot(md);
		// 			materials2.push(m);
		// 			selectMaterial2(materials2.length - 1);
		// 			RenderUtil.makeMaterialPreview();
		// 			ui.g.begin(false);
		// 		});
		// 	}
		// 	if (ui.button("Nodes")) {
		// 		showMaterialNodes();
		// 	}
		// }
	}
}
