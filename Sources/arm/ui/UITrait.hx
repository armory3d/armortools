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
import arm.node.MaterialParser;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.UVUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.io.ImportFont;
import arm.io.ExportTexture;
import arm.App.tr;
import arm.Tool;
import arm.Project;
import arm.Res;

@:access(zui.Zui)
class UITrait {

	public static var inst: UITrait;
	public static var defaultWindowW = 280;
	public static inline var defaultToolbarW = 54;
	public static inline var defaultHeaderH = 28;
	public static inline var defaultStatusH = 32;
	public static inline var defaultMenubarW = 330;
	public static var penPressureRadius = true;
	public static var penPressureHardness = true;
	public static var penPressureOpacity = false;

	public var windowW = 280; // Panel width
	public var toolbarw = defaultToolbarW;
	public var headerh = defaultHeaderH;
	public var statush = defaultStatusH;
	public var menubarw = defaultMenubarW;
	public var tabx = 0;
	public var tabh = 0;
	public var tabh1 = 0;
	public var tabh2 = 0;

	public var isScrolling = false;
	public var colorIdPicked = false;
	public var show = true;
	public var splitView = false;
	public var viewIndex = -1;
	public var viewIndexLast = -1;
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
	public var materialIdPicked = 0;
	public var uvxPicked = 0.0;
	public var uvyPicked = 0.0;
	public var pickerSelectMaterial = true;
	public var pickerMaskHandle = new Handle();
	var borderStarted = 0;
	var borderHandle: Handle = null;

	public var defaultEnvmap: Image = null;
	public var defaultIrradiance: kha.arrays.Float32Array = null;
	public var defaultRadiance: Image = null;
	public var defaultRadianceMipmaps: Array<Image> = null;
	public var savedEnvmap: Image = null;
	public var emptyEnvmap: Image = null;
	public var previewEnvmap: Image = null;
	public var showEnvmap = false;
	public var showEnvmapHandle = new Handle({selected: false});
	public var showEnvmapBlur = false;
	public var showEnvmapBlurHandle = new Handle({selected: false});
	public var drawWireframe = false;
	public var wireframeHandle = new Handle({selected: false});
	public var drawTexels = false;
	public var texelsHandle = new Handle({selected: false});
	public var cullBackfaces = true;
	public var textureFilter = true;

	public var ui: Zui;
	public var colorIdHandle = Id.handle();
	public var formatType = FormatPng;
	public var formatQuality = 100.0;
	public var layersExport = 0;
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
	public var splitBy = SplitObject;
	public var parseTransform = false;
	public var parseVCols = false;
	public var hwnd = Id.handle();
	public var hwnd1 = Id.handle();
	public var hwnd2 = Id.handle();
	public var selectTime = 0.0;
	#if arm_creator
	public var displaceStrength = 100.0;
	#else
	public var displaceStrength = 1.0;
	#end
	public var decalImage: Image = null;
	public var decalPreview = false;
	public var viewportMode = ViewRender;
	public var hscaleWasChanged = false;
	public var exportMeshFormat = FormatObj;
	#if (krom_android || krom_ios)
	public var nativeBrowser = false;
	#else
	public var nativeBrowser = true;
	#end
	public var cacheDraws = false;
	public var showAssetNames = false;

	public var textToolImage: Image = null;
	public var textToolText = tr("Text");
	public var textToolHandle = new Handle();
	public var particleMaterial: MaterialData = null;

	public var layerFilter = 0;

	public var runBrush: Int->Void = null;
	public var parseBrushInputs: Void->Void = null;
	public var paintVec = new Vec4();
	public var lastPaintX = -1.0;
	public var lastPaintY = -1.0;
	public var painted = 0;
	public var brushTime = 0.0;
	public var cloneStartX = -1.0;
	public var cloneStartY = -1.0;
	public var cloneDeltaX = 0.0;
	public var cloneDeltaY = 0.0;

	public var gizmo: Object = null;
	public var gizmoX: Object = null;
	public var gizmoY: Object = null;
	public var gizmoZ: Object = null;
	public var axisX = false;
	public var axisY = false;
	public var axisZ = false;
	public var axisStart = 0.0;
	public var row4 = [1 / 4, 1 / 4, 1 / 4, 1 / 4];

	public var brushNodesRadius = 1.0;
	public var brushNodesOpacity = 1.0;
	public var brushMaskImage: Image = null;
	public var brushStencilImage: Image = null;
	public var brushStencilX = 0.02;
	public var brushStencilY = 0.02;
	public var brushStencilRot = 0.0;
	public var brushStencilScale = 0.9;
	public var brushStencilScaling = false;
	public var brushNodesScale = 1.0;
	public var brushNodesHardness = 1.0;

	public var brushRadius = 0.5;
	public var brushRadiusHandle = new Handle({value: 0.5});
	public var brushScaleX = 1.0;
	public var brushScaleXHandle = new Handle({value: 1.0});
	public var brushBlending = BlendMix;
	public var brushOpacity = 1.0;
	public var brushOpacityHandle = new Handle({value: 1.0});
	public var brushScale = 1.0;
	public var brushRot = 0.0;
	public var brushHardness = 0.8;
	public var brushLazy = 0.0;
	public var brushBias = 1.0;
	public var brushPaint = UVMap;
	public var brush3d = true;
	public var brushLive = false;
	public var brushDepthReject = true;
	public var brushAngleReject = true;
	public var brushAngleRejectDot = 0.5;
	public var bakeType = BakeAO;
	public var bakeAxis = BakeXYZ;
	public var bakeUpAxis = BakeUpZ;
	public var bakeAoStrength = 1.0;
	public var bakeAoRadius = 1.0;
	public var bakeAoOffset = 1.0;
	public var bakeCurvStrength = 1.0;
	public var bakeCurvRadius = 1.0;
	public var bakeCurvOffset = 0.0;
	public var bakeCurvSmooth = 1;
	public var bakeHighPoly = 0;
	public var dilateRadius = 8.0;

	public var xray = false;
	public var symX = false;
	public var symY = false;
	public var symZ = false;
	public var showCompass = true;
	public var fillTypeHandle = new Handle();
	#if arm_creator
	public var projectType = ModelTessellatedPlane;
	#else
	public var projectType = ModelCube;
	#end
	public var projectObjects: Array<MeshObject>;

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
	public var cameraType = CameraPerspective;
	public var camHandle = new Handle();
	public var fovHandle: Handle = null;
	public var undoHandle: Handle = null;
	public var hssgi: Handle = null;
	public var hssr: Handle = null;
	public var hbloom: Handle = null;
	public var hsupersample: Handle = null;
	public var hvxao: Handle = null;
	#if arm_creator
	public var vxaoExt = 5.0;
	#else
	public var vxaoExt = 1.0;
	#end
	public var vxaoOffset = 1.5;
	public var vxaoAperture = 1.2;
	public var vignetteStrength = 0.4;
	// public var autoExposureStrength = 1.0;
	public var textureExportPath = "";
	public var headerHandle = new Handle({layout: Horizontal});
	public var toolbarHandle = new Handle();
	public var statusHandle = new Handle();
	public var menuHandle = new Handle({layout: Horizontal});
	public var workspaceHandle = new Handle({layout: Horizontal});
	var lastCombo: Handle = null;
	var lastTooltip: Image = null;
	var lastStatusPosition = 0;

	public var cameraControls = ControlsOrbit;
	public var htab = Id.handle();
	public var htab1 = Id.handle();
	public var htab2 = Id.handle();
	public var worktab = Id.handle();
	public var statustab = Id.handle();
	public var toolNames = [
		tr("Brush"),
		tr("Eraser"),
		tr("Fill"),
		tr("Decal"),
		tr("Text"),
		tr("Clone"),
		tr("Blur"),
		tr("Particle"),
		tr("Bake"),
		tr("ColorID"),
		tr("Picker"),
	];

	public function new() {
		inst = this;

		windowW = Std.int(defaultWindowW * Config.raw.window_scale);
		toolbarw = Std.int(defaultToolbarW * Config.raw.window_scale);
		headerh = Std.int(defaultHeaderH * Config.raw.window_scale);
		statush = Std.int(defaultStatusH * Config.raw.window_scale);
		menubarw = Std.int(defaultMenubarW * Config.raw.window_scale);

		if (Project.materials == null) {
			Project.materials = [];
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				Project.materials.push(new MaterialSlot(m));
				Context.material = Project.materials[0];
			});
		}
		if (Project.materialsScene == null) {
			Project.materialsScene = [];
			Data.getMaterial("Scene", "Material2", function(m: MaterialData) {
				Project.materialsScene.push(new MaterialSlot(m));
				Context.materialScene = Project.materialsScene[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(new BrushSlot());
			Context.brush = Project.brushes[0];
			MaterialParser.parseBrush();
		}

		if (Project.layers == null) {
			Project.layers = [];
			Project.layers.push(new LayerSlot());
			Context.layer = Project.layers[0];
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
			// savedEnvmap = world.envmap;
			// defaultEnvmap = world.envmap;
			defaultIrradiance = world.probe.irradiance;
			defaultRadiance = world.probe.radiance;
			defaultRadianceMipmaps = world.probe.radianceMipmaps;
		}
		world.envmap = showEnvmap ? savedEnvmap : emptyEnvmap;
		Context.ddirty = 1;

		// Save last pos for continuos paint
		iron.App.notifyOnRender(function(g: kha.graphics4.Graphics) { //
			if (frame == 2) {
				RenderUtil.makeMaterialPreview();
				hwnd1.redraws = 2;
				MaterialParser.parseMeshMaterial();
				MaterialParser.parsePaintMaterial();
				Context.ddirty = 0;
				History.reset();
				if (History.undoLayers == null) {
					History.undoLayers = [];
					for (i in 0...Config.raw.undo_steps) {
						var l = new LayerSlot("_undo" + History.undoLayers.length);
						l.createMask(0, false);
						History.undoLayers.push(l);
					}
				}
			}
			else if (frame == 3) {
				Context.ddirty = 1;
			}
			frame++;

			var mouse = Input.getMouse();
			if (mouse.down()) { //
				lastPaintVecX = paintVec.x; //
				lastPaintVecY = paintVec.y; //
			} //
			else {
				if (splitView) {
					viewIndex = Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
				}

				lastPaintVecX = mouse.viewX / iron.App.w();
				lastPaintVecY = mouse.viewY / iron.App.h();

				viewIndex = -1;

				#if (kha_android || kha_ios)
				// No mouse move events for touch, re-init last paint position on touch start
				lastPaintX = -1;
				lastPaintY = -1;
				#end
			}
		}); //

		var scale = Config.raw.window_scale;
		ui = new Zui( { theme: App.theme, font: App.font, scaleFactor: scale, color_wheel: App.color_wheel } );
		Zui.onBorderHover = onBorderHover;
		Zui.onTextHover = onTextHover;

		var resources = ["cursor.k", "icons.k"];
		Res.load(resources, done);

		projectObjects = [];
		for (m in Scene.active.meshes) projectObjects.push(m);
	}

	function done() {
		if (ui.SCALE() > 1) setIconScale();
		//
		gizmo = Scene.active.getChild(".GizmoTranslate");
		gizmo.transform.scale.set(0.5, 0.5, 0.5);
		gizmo.transform.buildMatrix();
		gizmoX = Scene.active.getChild("GizmoX");
		gizmoY = Scene.active.getChild("GizmoY");
		gizmoZ = Scene.active.getChild("GizmoZ");
		//

		Context.object = Scene.active.getChild("Cube");
		Context.paintObject = cast(Context.object, MeshObject);
		Project.paintObjects = [Context.paintObject];

		if (App.fileArg == "") {
			iron.App.notifyOnRender(Layers.initLayers);
		}

		// Init plugins
		if (Config.raw.plugins != null) {
			for (plugin in Config.raw.plugins) {
				Plugin.start(plugin);
			}
		}
	}

	public function update() {
		isScrolling = ui.isScrolling;
		updateUI();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		if (!App.uienabled) return;

		if (!UINodes.inst.ui.isTyping && !ui.isTyping) {
			if (Operator.shortcut(Config.keymap.cycle_layers)) {
				var i = (Project.layers.indexOf(Context.layer) + 1) % Project.layers.length;
				Context.setLayer(Project.layers[i]);
			}
			else if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
				show2DView();
			}
			else if (Operator.shortcut(Config.keymap.toggle_node_editor)) {
				showMaterialNodes();
			}
			else if (Operator.shortcut(Config.keymap.toggle_browser)) {
				toggleBrowser();
			}
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.projectSaveAs();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.projectSave();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.projectOpen();
		else if (Operator.shortcut(Config.keymap.file_reimport_mesh)) Project.reimportMesh();
		else if (Operator.shortcut(Config.keymap.file_new)) Project.projectNewBox();
		else if (Operator.shortcut(Config.keymap.file_export_textures)) {
			if (textureExportPath == "") { // First export, ask for path
				BoxExport.showTextures();
			}
			else {
				function export(_) {
					ExportTexture.run(textureExportPath);
					iron.App.removeRender(export);
				}
				iron.App.notifyOnRender(export);
			}
		}
		else if (Operator.shortcut(Config.keymap.file_export_textures_as)) BoxExport.showTextures();
		else if (Operator.shortcut(Config.keymap.file_import_assets)) Project.importAsset();
		else if (Operator.shortcut(Config.keymap.edit_prefs)) BoxPreferences.show();

		var kb = Input.getKeyboard();
		if (kb.started(Config.keymap.view_distract_free) ||
		   (kb.started("escape") && !show && !UIBox.show)) {
			toggleDistractFree();
		}

		var mouse = Input.getMouse();

		if (brushCanLock || brushLocked) {
			if (kb.down(Config.keymap.brush_radius) && mouse.moved) {
				if (brushLocked) {
					if (kb.down("shift")) {
						brushOpacity += mouse.movementX / 500;
						brushOpacity = Math.max(0.0, Math.min(1.0, brushOpacity));
						brushOpacity = Math.round(brushOpacity * 100) / 100;
						brushOpacityHandle.value = brushOpacity;
					}
					else {
						brushRadius += mouse.movementX / 150;
						brushRadius = Math.max(0.05, Math.min(4.0, brushRadius));
						brushRadius = Math.round(brushRadius * 100) / 100;
						brushRadiusHandle.value = brushRadius;
					}
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
		if (mouse.viewX > 0 && mouse.viewX < right &&
			mouse.viewY > 0 && mouse.viewY < iron.App.h() &&
			!ui.isTyping && !UIView2D.inst.ui.isTyping && !UINodes.inst.ui.isTyping) {

			if (worktab.position == SpacePaint) {
				if (kb.down("shift")) {
					if (kb.started("1")) Context.selectMaterial(0);
					else if (kb.started("2")) Context.selectMaterial(1);
					else if (kb.started("3")) Context.selectMaterial(2);
					else if (kb.started("4")) Context.selectMaterial(3);
					else if (kb.started("5")) Context.selectMaterial(4);
					else if (kb.started("6")) Context.selectMaterial(5);
				}

				if (!mouse.down("right")) { // Fly mode off
					if (Operator.shortcut(Config.keymap.tool_brush)) Context.selectTool(ToolBrush);
					else if (Operator.shortcut(Config.keymap.tool_eraser)) Context.selectTool(ToolEraser);
					else if (Operator.shortcut(Config.keymap.tool_fill)) Context.selectTool(ToolFill);
					else if (Operator.shortcut(Config.keymap.tool_bake)) Context.selectTool(ToolBake);
					else if (Operator.shortcut(Config.keymap.tool_colorid)) Context.selectTool(ToolColorId);
					else if (Operator.shortcut(Config.keymap.tool_decal)) Context.selectTool(ToolDecal);
					else if (Operator.shortcut(Config.keymap.tool_text)) Context.selectTool(ToolText);
					else if (Operator.shortcut(Config.keymap.tool_clone)) Context.selectTool(ToolClone);
					else if (Operator.shortcut(Config.keymap.tool_blur)) Context.selectTool(ToolBlur);
					else if (Operator.shortcut(Config.keymap.tool_particle)) Context.selectTool(ToolParticle);
					else if (Operator.shortcut(Config.keymap.tool_picker)) Context.selectTool(ToolPicker);
				}

				// Radius
				if (Context.tool == ToolBrush  ||
					Context.tool == ToolEraser ||
					Context.tool == ToolDecal  ||
					Context.tool == ToolText   ||
					Context.tool == ToolClone  ||
					Context.tool == ToolBlur   ||
					Context.tool == ToolParticle) {
					if (Operator.shortcut(Config.keymap.brush_radius) ||
						Operator.shortcut(Config.keymap.brush_opacity)) {
						brushCanLock = true;
						if (!Input.getPen().connected) mouse.lock();
						lockStartedX = mouse.x;
						lockStartedY = mouse.y;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_decrease, true)) {
						brushRadius -= getRadiusIncrement();
						brushRadius = Math.round(brushRadius * 100) / 100;
						brushRadiusHandle.value = brushRadius;
						headerHandle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_increase, true)) {
						brushRadius += getRadiusIncrement();
						brushRadius = Math.round(brushRadius * 100) / 100;
						brushRadiusHandle.value = brushRadius;
						headerHandle.redraws = 2;
					}
				}
			}

			// Viewpoint
			if (Operator.shortcut(Config.keymap.view_reset)) {
				ViewportUtil.resetViewport();
				ViewportUtil.scaleToBounds();
			}
			else if (Operator.shortcut(Config.keymap.view_back)) ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI);
			else if (Operator.shortcut(Config.keymap.view_front)) ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0);
			else if (Operator.shortcut(Config.keymap.view_left)) ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
			else if (Operator.shortcut(Config.keymap.view_right)) ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
			else if (Operator.shortcut(Config.keymap.view_bottom)) ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI);
			else if (Operator.shortcut(Config.keymap.view_top)) ViewportUtil.setView(0, 0, 1, 0, 0, 0);
			else if (Operator.shortcut(Config.keymap.view_camera_type)) {
				cameraType = cameraType == CameraPerspective ? CameraOrthographic : CameraPerspective;
				camHandle.position = cameraType;
				ViewportUtil.updateCameraType(cameraType);
			}
			else if (Operator.shortcut(Config.keymap.view_orbit_left, true)) ViewportUtil.orbit(-Math.PI / 12, 0);
			else if (Operator.shortcut(Config.keymap.view_orbit_right, true)) ViewportUtil.orbit(Math.PI / 12, 0);
			else if (Operator.shortcut(Config.keymap.view_orbit_up, true)) ViewportUtil.orbit(0, -Math.PI / 12);
			else if (Operator.shortcut(Config.keymap.view_orbit_down, true)) ViewportUtil.orbit(0, Math.PI / 12);
			else if (Operator.shortcut(Config.keymap.view_orbit_opposite)) ViewportUtil.orbit(Math.PI, 0);
			else if (Operator.shortcut(Config.keymap.view_zoom_in, true)) ViewportUtil.zoom(0.2);
			else if (Operator.shortcut(Config.keymap.view_zoom_out, true)) ViewportUtil.zoom(-0.2);
		}

		if (brushCanLock || brushLocked) {
			if (mouse.moved && brushCanUnlock) {
				brushLocked = false;
				brushCanUnlock = false;
			}
			if (kb.released(Config.keymap.brush_radius)) {
				mouse.unlock();
				brushCanUnlock = true;
				lastPaintX = -1;
				lastPaintY = -1;
			}
		}

		if (borderHandle != null) {
			if (borderHandle == UINodes.inst.hwnd || borderHandle == UIView2D.inst.hwnd) {
				if (borderStarted == SideLeft) {
					UINodes.inst.defaultWindowW -= Std.int(mouse.movementX);
					if (UINodes.inst.defaultWindowW < 32) UINodes.inst.defaultWindowW = 32;
					else if (UINodes.inst.defaultWindowW > System.windowWidth() * 0.7) UINodes.inst.defaultWindowW = Std.int(System.windowWidth() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					UINodes.inst.defaultWindowH -= Std.int(mouse.movementY);
					if (UINodes.inst.defaultWindowH < 32) UINodes.inst.defaultWindowH = 32;
					else if (UINodes.inst.defaultWindowH > iron.App.h() * 0.95) UINodes.inst.defaultWindowH = Std.int(iron.App.h() * 0.95);
				}
			}
			else if (borderHandle == statusHandle) {
				var my = Std.int(mouse.movementY);
				if (statush - my >= defaultStatusH * Config.raw.window_scale && statush - my < System.windowHeight() * 0.7) {
					statush -= my;
				}
			}
			else {
				if (borderStarted == SideLeft) {
					defaultWindowW -= Std.int(mouse.movementX);
					if (defaultWindowW < 32) defaultWindowW = 32;
					else if (defaultWindowW > System.windowWidth() - 32) defaultWindowW = System.windowWidth() - 32;
					windowW = Std.int(defaultWindowW * Config.raw.window_scale);
				}
				else {
					var my = Std.int(mouse.movementY);
					if (borderHandle == hwnd1 && borderStarted == SideTop) {
						if (tabh + my > 32 && tabh1 - my > 32) {
							tabh += my;
							tabh1 -= my;
						}
					}
					else if (borderHandle == hwnd2 && borderStarted == SideTop) {
						if (tabh1 + my > 32 && tabh2 - my > 32) {
							tabh1 += my;
							tabh2 -= my;
						}
					}
				}
			}
		}
		if (!mouse.down()) {
			borderHandle = null;
			App.isResizing = false;
		}
	}

	public function toggleDistractFree() {
		show = !show;
		App.resize();
	}

	inline function getRadiusIncrement(): Float {
		return 0.1;
	}

	static function hitRect(mx: Float, my: Float, x: Int, y: Int, w: Int, h: Int) {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	function getBrushRect(): TRect {
		var w = Std.int(brushStencilImage.width * (App.h() / brushStencilImage.height) * brushStencilScale);
		var h = Std.int(App.h() * brushStencilScale);
		var x = Std.int(App.x() + brushStencilX * App.w());
		var y = Std.int(App.y() + brushStencilY * App.h());
		return { w: w, h: h, x: x, y: y };
	}

	function updateUI() {

		if (Log.messageTimer > 0) {
			Log.messageTimer -= Time.delta;
			if (Log.messageTimer <= 0) statusHandle.redraws = 2;
		}

		if (!App.uienabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var setCloneSource = Context.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint);

		if (brushStencilImage != null && Operator.shortcut(Config.keymap.stencil_transform)) {
			var r = getBrushRect();
			if (mouse.started("left")) {
				brushStencilScaling =
					hitRect(mouse.x, mouse.y, r.x - 8,       r.y - 8,       16, 16) ||
					hitRect(mouse.x, mouse.y, r.x - 8,       r.h + r.y - 8, 16, 16) ||
					hitRect(mouse.x, mouse.y, r.w + r.x - 8, r.y - 8,       16, 16) ||
					hitRect(mouse.x, mouse.y, r.w + r.x - 8, r.h + r.y - 8, 16, 16);
			}
			var _scale = brushStencilScale;
			if (mouse.down("left")) {
				if (brushStencilScaling) {
					var mult = mouse.x > r.x + r.w / 2 ? 1 : -1;
					brushStencilScale += mouse.movementX / 500 * mult;
				}
				else {
					brushStencilX += mouse.movementX / App.w();
					brushStencilY += mouse.movementY / App.h();
				}
			}
			else brushStencilScaling = false;
			if (mouse.wheelDelta != 0) {
				brushStencilScale -= mouse.wheelDelta / 10;
			}
			// Center after scale
			brushStencilX += (_scale * brushStencilImage.width - brushStencilScale * brushStencilImage.width) / 2 / App.w();
			brushStencilY += (_scale * brushStencilImage.height - brushStencilScale * brushStencilImage.height) / 2 / App.h();
		}

		var down = Operator.shortcut(Config.keymap.action_paint) ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint) ||
				   (Input.getPen().down() && !kb.down("alt"));
		if (down) {
			var mx = mouse.viewX;
			var my = mouse.viewY;
			if (paint2d) mx -= iron.App.w();

			if (mx < iron.App.w() && mx > iron.App.x() &&
				my < iron.App.h() && my > iron.App.y()) {

				if (setCloneSource) {
					cloneStartX = mx;
					cloneStartY = my;
				}
				else {
					if (brushTime == 0 &&
						!App.isDragging &&
						!App.isResizing &&
						@:privateAccess ui.comboSelectedHandle == null) { // Paint started

						// Draw line
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint)) {
							lastPaintVecX = lastPaintX;
							lastPaintVecY = lastPaintY;
						}

						History.pushUndo = true;
						if (Context.tool == ToolClone && cloneStartX >= 0.0) { // Clone delta
							cloneDeltaX = (cloneStartX - mx) / iron.App.w();
							cloneDeltaY = (cloneStartY - my) / iron.App.h();
							cloneStartX = -1;
						}
						else if (Context.tool == ToolParticle) {
							// Reset particles
							#if arm_particles
							var emitter: MeshObject = cast Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							@:privateAccess psys.time = 0;
							// @:privateAccess psys.time = @:privateAccess psys.seed * @:privateAccess psys.animtime;
							// @:privateAccess psys.seed++;
							#end
						}
					}
					brushTime += Time.delta;
					if (runBrush != null) runBrush(0);
				}
			}
		}
		else if (brushTime > 0) { // Brush released
			brushTime = 0;
			#if (!kha_direct3d12) // Keep accumulated samples for D3D12
			Context.ddirty = 3;
			#end
			Context.brushBlendDirty = true; // Update brush mask
			Context.layerPreviewDirty = true; // Update layer preview
		}

		if (Context.layersPreviewDirty) {
			Context.layersPreviewDirty = false;
			Context.layerPreviewDirty = false;
			// Update all layer previews
			for (l in Project.layers) {
				if (l.getChildren() != null) continue;
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
					g2.begin(true, 0x00000000);
					g2.drawScaledImage(source, 0, 0, target.width, target.height);
					g2.end();
				}
			}
			hwnd.redraws = 2;
		}
		if (Context.layerPreviewDirty && Context.layer.getChildren() == null) {
			Context.layerPreviewDirty = false;
			// Update layer preview
			var l = Context.layer;
			var target = Context.layerIsMask ? l.texpaint_mask_preview : l.texpaint_preview;
			var source = Context.layerIsMask ? l.texpaint_mask : l.texpaint;
			var g2 = target.g2;
			g2.begin(true, 0x00000000);
			g2.drawScaledImage(source, 0, 0, target.width, target.height);
			g2.end();
			hwnd.redraws = 2;
		}

		#if krom_darwin
		var undoPressed = !kb.down("shift") && kb.started("z"); // cmd+z on macos
		var redoPressed = (kb.down("shift") && kb.started("z")) || kb.started("y"); // cmd+y on macos
		#else
		var undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		var redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (kb.down("control") && kb.started("y"));
		#end

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();

		if (worktab.position == SpaceScene) {
			arm.plugin.Gizmo.update();
		}

		if (lastCombo != null || (ui.tooltipImg == null && lastTooltip != null)) App.redrawUI();
		lastCombo = ui.comboSelectedHandle;
		lastTooltip = ui.tooltipImg;
	}

	public function render(g: kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		renderUI(g);
	}

	public function renderCursor(g: kha.graphics2.Graphics) {
		g.color = 0xffffffff;

		// Brush
		if (App.uienabled && worktab.position == SpacePaint) {
			var mouse = Input.getMouse();
			var mx = mouse.x;
			var my = mouse.y;
			var pen = Input.getPen();
			if (pen.down()) {
				mx = pen.x;
				my = pen.y;
			}

			// Radius being scaled
			if (brushLocked) {
				mx += lockStartedX - System.windowWidth() / 2;
				my += lockStartedY - System.windowHeight() / 2;
			}

			if (brushStencilImage != null && Context.tool != ToolBake && Context.tool != ToolPicker && Context.tool != ToolColorId) {
				var transform = Operator.shortcut(Config.keymap.stencil_transform);
				g.color = 0x88ffffff;
				var r = getBrushRect();
				g.drawScaledImage(brushStencilImage, r.x, r.y, r.w, r.h);
				g.color = 0xffffffff;
				if (transform) {
					g.drawRect(r.x, r.y, r.w, r.h);
					g.drawRect(r.x - 8,       r.y - 8,       16, 16);
					g.drawRect(r.x - 8 + r.w, r.y - 8,       16, 16);
					g.drawRect(r.x - 8,       r.y - 8 + r.h, 16, 16);
					g.drawRect(r.x - 8 + r.w, r.y - 8 + r.h, 16, 16);
				}
			}

			// Show picked material next to cursor
			if (Context.tool == ToolPicker && pickerSelectMaterial) {
				var img = Context.material.imageIcon;
				g.drawImage(img, mx + 10, my + 10);
			}

			var cursorImg = Res.get("cursor.k");
			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));

			// Clone source cursor
			var kb = Input.getKeyboard();
			if (Context.tool == ToolClone && !kb.down("alt") && (mouse.down() || pen.down())) {
				g.color = 0x66ffffff;
				g.drawScaledImage(cursorImg, mx + cloneDeltaX * iron.App.w() - psize / 2, my + cloneDeltaY * iron.App.h() - psize / 2, psize, psize);
				g.color = 0xffffffff;
			}

			var in2dView = UIView2D.inst.show && UIView2D.inst.type == View2DLayer &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var inNodes = UINodes.inst.show &&
						  mx > UINodes.inst.wx && mx < UINodes.inst.wx + UINodes.inst.ww &&
						  my > UINodes.inst.wy && my < UINodes.inst.wy + UINodes.inst.wh;
			var decal = Context.tool == ToolDecal || Context.tool == ToolText;

			if (!brush3d || in2dView || (decal && !brushLive)) {
				if (decal && !inNodes) {
					var psizex = Std.int(256 * (brushRadius * brushNodesRadius * brushScaleX));
					var psizey = Std.int(256 * (brushRadius * brushNodesRadius));
					g.color = kha.Color.fromFloats(1, 1, 1, brushOpacity);
					#if (kha_direct3d11 || kha_direct3d12)
					g.drawScaledImage(decalImage, mx - psizex / 2, my - psizey / 2, psizex, psizey);
					#else
					g.drawScaledImage(decalImage, mx - psizex / 2, my - psizey / 2 + psizey, psizex, -psizey);
					#end
					g.color = 0xffffffff;
				}
				else if (Context.tool == ToolBrush  ||
						 Context.tool == ToolEraser ||
						 Context.tool == ToolClone  ||
						 Context.tool == ToolBlur   ||
						 Context.tool == ToolParticle) {
						if (brushLazy > 0 && (Context.tool == ToolBrush || Context.tool == ToolEraser)) {
							var radius = psize + brushLazy;
							g.color = 0x66ffffff;
							g.drawScaledImage(cursorImg, mx - radius / 2, my - radius / 2, radius, radius);
							g.color = 0xffffffff;
						}
						g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
				}
			}
		}
	}

	public function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		if (UINodes.inst.show && UINodes.inst.canvasType == CanvasMaterial) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = CanvasMaterial; }
		App.resize();
	}

	public function showBrushNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		if (UINodes.inst.show && UINodes.inst.canvasType == CanvasBrush) UINodes.inst.show = false;
		else { UINodes.inst.show = true; UINodes.inst.canvasType = CanvasBrush; }
		App.resize();
	}

	public function show2DView(type = 0) {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UIView2D.inst.ui.endInput();
		if (UIView2D.inst.type != type) UIView2D.inst.show = true;
		else UIView2D.inst.show = !UIView2D.inst.show;
		UIView2D.inst.type = type;
		App.resize();
	}

	function toggleBrowser() {
		var minimized = statush <= defaultStatusH * Config.raw.window_scale;
		statush = minimized ? 240 : defaultStatusH;
		statush = Std.int(statush * Config.raw.window_scale);
	}

	public function getImage(asset: TAsset): Image {
		return asset != null ? Project.assetMap.get(asset.id) : null;
	}

	function renderUI(g: kha.graphics2.Graphics) {

		if (!App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (App.uienabled && !ui.inputRegistered) ui.registerInput();

		if (!show) return;

		g.end();
		ui.begin(g);

		if (ui.window(toolbarHandle, 0, headerh, toolbarw, System.windowHeight() - headerh)) {
			ui._y += 2;

			ui.imageScrollAlign = false;

			if (worktab.position == SpacePaint) {
				var keys = ["(B)", "(E)", "(G)", "(D)", "(T)", "(L) - " + tr("Hold ALT to set source"), "(U)", "(P)", "(K)", "(C)", "(V)"];
				var img = Res.get("icons.k");
				var imgw = ui.SCALE() > 1 ? 100 : 50;
				for (i in 0...toolNames.length) {
					ui._x += 2;
					if (Context.tool == i) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
					if (ui.image(img, -1, null, i * imgw, 0, imgw, imgw) == State.Started) Context.selectTool(i);
					if (ui.isHovered) ui.tooltip(toolNames[i] + " " + keys[i]);
					ui._x -= 2;
					ui._y += 2;
				}
			}
			else if (worktab.position == SpaceScene) {
				var img = Res.get("icons.k");
				var imgw = ui.SCALE() > 1 ? 100 : 50;
				ui._x += 2;
				if (Context.tool == ToolGizmo) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) Context.selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip(tr("Gizmo") + " (G)");
				ui._x -= 2;
				ui._y += 2;
			}

			ui.imageScrollAlign = true;
		}

		var panelx = iron.App.x() - toolbarw;

		var WINDOW_BG_COL = ui.t.WINDOW_BG_COL;
		ui.t.WINDOW_BG_COL = ui.t.SEPARATOR_COL;
		if (ui.window(menuHandle, panelx, 0, menubarw, Std.int(defaultHeaderH * ui.SCALE()))) {
			var _w = ui._w;
			ui._x += 1; // Prevent "File" button highlight on startup

			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

			menuButton(tr("File"), MenuFile);
			menuButton(tr("Edit"), MenuEdit);
			menuButton(tr("Viewport"), MenuViewport);
			menuButton(tr("Mode"), MenuMode);
			menuButton(tr("Camera"), MenuCamera);
			menuButton(tr("Help"), MenuHelp);

			ui._w = _w;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.BUTTON_COL = BUTTON_COL;
		}
		ui.t.WINDOW_BG_COL = WINDOW_BG_COL;

		var panelx = (iron.App.x() - toolbarw) + menubarw;
		if (ui.window(workspaceHandle, panelx, 0, System.windowWidth() - windowW - menubarw, Std.int(defaultHeaderH * ui.SCALE()))) {
			ui.tab(worktab, tr("Paint"));
			// ui.tab(worktab, "Sculpt");
			ui.tab(worktab, tr("Scene"));
			#if arm_creator
			ui.tab(worktab, tr("Render"));
			#end
			if (worktab.changed) {
				Context.ddirty = 2;
				toolbarHandle.redraws = 2;
				headerHandle.redraws = 2;
				hwnd.redraws = 2;
				hwnd1.redraws = 2;
				hwnd2.redraws = 2;

				if (worktab.position == SpaceScene) {
					Context.selectTool(ToolGizmo);
				}

				MaterialParser.parseMeshMaterial();
				Context.mainObject().skip_context = null;
			}
		}

		var panelx = iron.App.x();
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth() - toolbarw - windowW, Std.int(defaultHeaderH * ui.SCALE()))) {
			ui._y += 2;

			if (worktab.position == SpacePaint) {

				if (Context.tool == ToolColorId) {
					ui.text(tr("Picked Color"));
					if (colorIdPicked) {
						ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
					}
					if (ui.button("Clear")) colorIdPicked = false;
					ui.text(tr("Color ID Map"));
					var cid = ui.combo(colorIdHandle, App.enumTexts("TEX_IMAGE"), tr("Color ID"));
					if (colorIdHandle.changed) Context.ddirty = 2;
					if (Project.assets.length > 0) ui.image(getImage(Project.assets[cid]));
				}
				else if (Context.tool == ToolPicker) {
					baseRPicked = Math.round(baseRPicked * 10) / 10;
					baseGPicked = Math.round(baseGPicked * 10) / 10;
					baseBPicked = Math.round(baseBPicked * 10) / 10;
					normalRPicked = Math.round(normalRPicked * 10) / 10;
					normalGPicked = Math.round(normalGPicked * 10) / 10;
					normalBPicked = Math.round(normalBPicked * 10) / 10;
					occlusionPicked = Math.round(occlusionPicked * 100) / 100;
					roughnessPicked = Math.round(roughnessPicked * 100) / 100;
					metallicPicked = Math.round(metallicPicked * 100) / 100;
					ui.text(tr("Base") + ' $baseRPicked,$baseGPicked,$baseBPicked');
					ui.text(tr("Nor") + ' $normalRPicked,$normalGPicked,$normalBPicked');
					ui.text(tr("Occlusion") + ' $occlusionPicked');
					ui.text(tr("Roughness") + ' $roughnessPicked');
					ui.text(tr("Metallic") + ' $metallicPicked');
					pickerSelectMaterial = ui.check(Id.handle({selected: pickerSelectMaterial}), tr("Select Material"));
					ui.combo(pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
					if (pickerMaskHandle.changed) {
						MaterialParser.parsePaintMaterial();
					}
				}
				else if (Context.tool == ToolBake) {
					ui.changed = false;
					var bakeHandle = Id.handle({position: bakeType});
					var bakes = [
						tr("AO"),
						tr("Curvature"),
						tr("Normal"),
						tr("Normal (Object)"),
						tr("Height"),
						tr("Derivative"),
						tr("Position"),
						tr("TexCoord"),
						tr("Material ID"),
						tr("Object ID"),
						tr("Vertex Color"),
					];
					#if kha_direct3d12
					bakes.push(tr("Lightmap"));
					bakes.push(tr("Bent Normal"));
					bakes.push(tr("Thickness"));
					#end
					bakeType = ui.combo(bakeHandle, bakes, tr("Bake"));
					if (bakeType == BakeNormalObject || bakeType == BakePosition || bakeType == BakeBentNormal) {
						var bakeUpAxisHandle = Id.handle({position: bakeUpAxis});
						bakeUpAxis = ui.combo(bakeUpAxisHandle, ["Z", "Y"], tr("Up Axis"), true);
					}
					if (bakeType == BakeAO || bakeType == BakeCurvature) {
						var bakeAxisHandle = Id.handle({position: bakeAxis});
						bakeAxis = ui.combo(bakeAxisHandle, ["XYZ", "X", "Y", "Z", "-X", "-Y", "-Z"], tr("Axis"), true);
					}
					if (bakeType == BakeAO) {
						var strengthHandle = Id.handle({value: bakeAoStrength});
						bakeAoStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
						var radiusHandle = Id.handle({value: bakeAoRadius});
						bakeAoRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
						var offsetHandle = Id.handle({value: bakeAoOffset});
						bakeAoOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
					}
					#if kha_direct3d12
					if (bakeType == BakeAO || bakeType == BakeLightmap || bakeType == BakeBentNormal || bakeType == BakeThickness) {
						ui.text(tr("Rays/pix:") + ' ${arm.render.RenderPathRaytrace.raysPix}');
						ui.text(tr("Rays/sec:") + ' ${arm.render.RenderPathRaytrace.raysSec}');
					}
					#end
					if (bakeType == BakeCurvature) {
						var strengthHandle = Id.handle({value: bakeCurvStrength});
						bakeCurvStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
						var radiusHandle = Id.handle({value: bakeCurvRadius});
						bakeCurvRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
						var offsetHandle = Id.handle({value: bakeCurvOffset});
						bakeCurvOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
						var smoothHandle = Id.handle({value: bakeCurvSmooth});
						bakeCurvSmooth = Std.int(ui.slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
					}
					if (bakeType == BakeNormal || bakeType == BakeHeight || bakeType == BakeDerivative) {
						var ar = [for (p in Project.paintObjects) p.name];
						var polyHandle = Id.handle({position: bakeHighPoly});
						bakeHighPoly = ui.combo(polyHandle, ar, tr("High Poly"));
					}
					if (ui.changed) {
						MaterialParser.parsePaintMaterial();
					}
				}
				else {
					if (Context.tool != ToolFill) {
						brushRadius = ui.slider(brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					}

					if (Context.tool == ToolDecal) {
						brushScaleX = ui.slider(brushScaleXHandle, tr("Scale X"), 0.01, 2.0, true);
					}

					if (Context.tool == ToolBrush  ||
						Context.tool == ToolFill   ||
						Context.tool == ToolDecal  ||
						Context.tool == ToolText) {
						var brushScaleHandle = Id.handle({value: brushScale});
						brushScale = ui.slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
						if (brushScaleHandle.changed) {
							if (Context.tool == ToolDecal || Context.tool == ToolText) {
								ui.g.end();
								RenderUtil.makeDecalPreview();
								ui.g.begin(false);
							}
						}

						var brushRotHandle = Id.handle({value: brushRot});
						brushRot = ui.slider(brushRotHandle, tr("UV Rotate"), 0.0, 360.0, true, 1);
						if (brushRotHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}

					brushOpacity = ui.slider(brushOpacityHandle, tr("Opacity"), 0.0, 1.0, true);

					if (Context.tool == ToolBrush || Context.tool == ToolEraser) {
						brushHardness = ui.slider(Id.handle({value: brushHardness}), tr("Hardness"), 0.0, 1.0, true);
						brushLazy = ui.slider(Id.handle({value: brushLazy}), tr("Lazy"), 0.0, 1.0, true);
					}

					if (Context.tool != ToolEraser) {
						var brushBlendingHandle = Id.handle({value: brushBlending});
						brushBlending = ui.combo(brushBlendingHandle, [
							tr("Mix"),
							tr("Darken"),
							tr("Multiply"),
							tr("Burn"),
							tr("Lighten"),
							tr("Screen"),
							tr("Dodge"),
							tr("Add"),
							tr("Overlay"),
							tr("Soft Light"),
							tr("Linear Light"),
							tr("Difference"),
							tr("Subtract"),
							tr("Divide"),
							tr("Hue"),
							tr("Saturation"),
							tr("Color"),
							tr("Value"),
						], tr("Blending"));
						if (brushBlendingHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}

					if (Context.tool == ToolBrush || Context.tool == ToolFill) {
						var paintHandle = Id.handle();
						brushPaint = ui.combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
						if (paintHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}
					if (Context.tool == ToolText) {
						ui.combo(textToolHandle, ImportFont.fontList, tr("Font"));
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

					if (Context.tool == ToolFill) {
						ui.combo(fillTypeHandle, [tr("Object"), tr("Face"), tr("Angle")], tr("Fill Mode"));
						if (fillTypeHandle.changed) {
							if (fillTypeHandle.position == FillFace) {
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
						var sc = ui.SCALE();
						ui._w = Std.int(60 * sc);

						var xrayHandle = Id.handle({selected: xray});
						xray = ui.check(xrayHandle, tr("X-Ray"));
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

		if (ui.window(statusHandle, iron.App.x(), System.windowHeight() - statush, System.windowWidth() - toolbarw - windowW, statush)) {
			ui._y += 2;

			TabBrowser.draw();
			TabScript.draw();
			TabConsole.draw();

			var minimized = statush <= defaultStatusH * Config.raw.window_scale;
			if (statustab.changed && (statustab.position == lastStatusPosition || minimized)) {
				toggleBrowser();
			}
			lastStatusPosition = statustab.position;
		}

		tabx = System.windowWidth() - windowW;
		if (tabh == 0) {
			tabh = tabh1 = tabh2 = Std.int(System.windowHeight() / 3);
		}
		gizmo.visible = false;

		if (worktab.position == SpacePaint) {
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				TabLayers.draw();
				TabHistory.draw();
				TabPlugins.draw();
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh1)) {
				Context.object = Context.paintObject;
				TabMaterials.draw();
				TabBrushes.draw();
				TabParticles.draw();
			}
			if (ui.window(hwnd2, tabx, tabh + tabh1, windowW, tabh2)) {
				TabTextures.draw();
				TabMeshes.draw();
				TabFonts.draw();
			}
		}
		else if (worktab.position == SpaceScene) {
			gizmo.visible = true;
			if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
				TabOutliner.draw();
				TabPlugins.draw();
			}
			if (ui.window(hwnd1, tabx, tabh, windowW, tabh1)) {
				TabMaterials.draw();
				TabProperties.draw();
				#if arm_creator
				TabTraits.draw();
				#end
			}
			if (ui.window(hwnd2, tabx, tabh + tabh1, windowW, tabh2)) {
				TabTextures.draw();
				TabMeshes.draw();
				TabFonts.draw();
			}
		}

		ui.end();
		g.begin(false);
	}

	function menuButton(name: String, category: Int) {
		ui._w = Std.int(ui.ops.font.width(ui.fontSize, name) + 25);
		if (ui.button(name) || (UIMenu.show && UIMenu.menuCommands == null && ui.isHovered)) {
			UIMenu.show = true;
			UIMenu.menuCategory = category;
			UIMenu.menuX = Std.int(ui._x - ui._w);
			UIMenu.menuY = headerh;
		}
	}

	public function setIconScale() {
		if (ui.SCALE() > 1) {
			Res.load(["icons2x.k"], function() {
				@:privateAccess Res.bundled.set("icons.k", Res.get("icons2x.k"));
			});
		}
		else {
			Res.load(["icons.k"], function() {});
		}
	}

	function onBorderHover(handle: Handle, side: Int) {
		if (!App.uienabled) return;
		if (handle != hwnd &&
			handle != hwnd1 &&
			handle != hwnd2 &&
			handle != statusHandle &&
			handle != UINodes.inst.hwnd &&
			handle != UIView2D.inst.hwnd) return; // Scalable handles
		if (handle == UINodes.inst.hwnd && side != SideLeft && side != SideTop) return;
		if (handle == UINodes.inst.hwnd && side == SideTop && !UIView2D.inst.show) return;
		if (handle == UIView2D.inst.hwnd && side != SideLeft) return;
		if (handle == hwnd && side == SideTop) return;
		if (handle == hwnd2 && side == SideBottom) return;
		if (handle == statusHandle && side != SideTop) return;
		if (side == SideRight) return; // UI is snapped to the right side

		side == SideLeft || side == SideRight ?
			Krom.setMouseCursor(6) : // Horizontal
			Krom.setMouseCursor(5);  // Vertical

		if (ui.inputStarted) {
			borderStarted = side;
			borderHandle = handle;
			App.isResizing = true;
		}
	}

	function onTextHover() {
		Krom.setMouseCursor(3); // I-cursor
	}

	public function tagUIRedraw() {
		headerHandle.redraws = 2;
		toolbarHandle.redraws = 2;
		statusHandle.redraws = 2;
		workspaceHandle.redraws = 2;
		menuHandle.redraws = 2;
		hwnd.redraws = 2;
		hwnd1.redraws = 2;
		hwnd2.redraws = 2;
	}
}
