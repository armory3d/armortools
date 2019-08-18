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
import arm.Tool;

@:access(zui.Zui)
class UITrait {

	public static var inst:UITrait;
	public static var defaultWindowW = 280;
	public static var defaultToolbarW = 54;
	public static var defaultHeaderH = 24;
	public static var penPressureRadius = true;
	public static var penPressureOpacity = false;
	public static var penPressureHardness = false;

	public var windowW = 280; // Panel width
	public var toolbarw = 54;
	public var headerh = 24;
	public var menubarw = 215;
	public var tabx = 0;
	public var tabh = 0;

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
	public var materialIdPicked = 0;
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
	public var drawTexels = false;
	public var texelsHandle = new Handle({selected: false});
	public var culling = true;
	public var textureFilter = true;

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

	public var onBrush:Int->Void = null;
	public var paintVec = new Vec4();
	public var lastPaintX = -1.0;
	public var lastPaintY = -1.0;
	public var painted = 0;
	public var brushTime = 0.0;
	public var cloneStartX = -1.0;
	public var cloneStartY = -1.0;
	public var cloneDeltaX = 0.0;
	public var cloneDeltaY = 0.0;

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
	public var brushMaskImage:Image = null;
	public var brushNodesScale = 1.0;
	public var brushNodesHardness = 1.0;

	public var brushRadius = 0.5;
	public var brushRadiusHandle = new Handle({value: 0.5});
	public var brushOpacity = 1.0;
	public var brushOpacityHandle = new Handle({value: 1.0});
	public var brushScale = 1.0;
	public var brushRot = 0.0;
	public var brushHardness = 0.8;
	public var brushBias = 1.0;
	public var brushPaint = 0;
	public var brush3d = true;
	public var brushDepthReject = true;
	public var brushAngleReject = true;
	public var brushAngleRejectDot = 0.5;
	public var bakeType = 0;
	public var bakeAxis = 0;
	public var bakeAoStrength = 1.0;
	public var bakeAoRadius = 1.0;
	public var bakeAoOffset = 1.0;
	public var bakeCurvStrength = 1.0;
	public var bakeCurvRadius = 1.0;
	public var bakeCurvOffset = 0.0;
	public var bakeCurvSmooth = 1;
	public var bakeHighPoly = 0;
	
	public var xray = false;
	public var symX = false;
	public var symY = false;
	public var symZ = false;
	public var showCompass = true;
	public var fillTypeHandle = new Handle();
	public var resHandle = new Handle({position: 4}); // 2048
	public var bitsHandle = new Handle({position: 0}); // 8bit
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

	public var cameraControls = 0;
	public var htab = Id.handle({position: 0});
	public var htab1 = Id.handle({position: 0});
	public var htab2 = Id.handle({position: 0});
	public var worktab = Id.handle({position: 0});
	public var toolNames = ["Brush", "Eraser", "Fill", "Decal", "Text", "Clone", "Blur", "Particle", "Bake", "ColorID", "Picker"];

	public function new() {
		inst = this;

		windowW = Std.int(defaultWindowW * Config.raw.window_scale);
		toolbarw = Std.int(defaultToolbarW * Config.raw.window_scale);
		headerh = Std.int(defaultHeaderH * Config.raw.window_scale);
		menubarw = Std.int(215 * Config.raw.window_scale);

		arm.render.Uniforms.init();

		if (Project.materials == null) {
			Project.materials = [];
			Data.getMaterial("Scene", "Material", function(m:MaterialData) {
				Project.materials.push(new MaterialSlot(m));
				Context.material = Project.materials[0];
			});
		}
		if (Project.materialsScene == null) {
			Project.materialsScene = [];
			Data.getMaterial("Scene", "Material2", function(m:MaterialData) {
				Project.materialsScene.push(new MaterialSlot(m));
				Context.materialScene = Project.materialsScene[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(new BrushSlot());
			Context.brush = Project.brushes[0];
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
			savedEnvmap = world.envmap;
		}
		world.envmap = showEnvmap ? savedEnvmap : emptyEnvmap;
		Context.ddirty = 1;

		// Save last pos for continuos paint
		iron.App.notifyOnRender(function(g:kha.graphics4.Graphics) { //
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

			var m = Input.getMouse();
			if (m.down()) { //
				lastPaintVecX = paintVec.x; //
				lastPaintVecY = paintVec.y; //
			}//
			else {
				lastPaintVecX = m.x / iron.App.w();
				lastPaintVecY = m.y / iron.App.h();
			}
		});//

		var scale = Config.raw.window_scale;
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

		Context.object = Scene.active.getChild("Cube");
		Context.paintObject = cast(Context.object, MeshObject);
		Project.paintObjects = [Context.paintObject];

		if (App.fileArg == "") {
			iron.App.notifyOnRender(Layers.initLayers);
		}

		// Init plugins
		Plugin.keep();
		if (Config.raw.plugins != null) {
			for (plugin in Config.raw.plugins) {
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
		if (!App.uibox.isTyping) {
			if (kb.started("escape")) {
				UIBox.show = false;
			}
		}

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
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.projectSaveAs();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.projectSave();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.projectOpen();
		else if (Operator.shortcut(Config.keymap.file_new)) UIBox.newProject();
		else if (Operator.shortcut(Config.keymap.export_textures)) {
			if (textureExportPath == "") { // First export, ask for path
				UIFiles.show = true;
				UIFiles.isSave = true;
				UIFiles.filters = bitsHandle.position > 0 ? "exr" : formatType == 0 ? "png" : "jpg";
				UIFiles.filesDone = function(path:String) {
					textureExport = true;
					textureExportPath = path;
				}
			}
			else textureExport = true;
		}
		else if (Operator.shortcut(Config.keymap.import_assets)) {
			UIFiles.show = true;
			UIFiles.isSave = false;
			UIFiles.filters = "jpg,png,tga,bmp,psd,gif,hdr,obj,fbx,blend,arm";
			UIFiles.filesDone = function(path:String) {
				Importer.importFile(path);
			}
		}

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
		if (mouse.x > 0 && mouse.x < right &&
			mouse.y > 0 && mouse.y < iron.App.h() &&
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
						mouse.lock();
						lockStartedX = mouse.x + iron.App.x();
						lockStartedY = mouse.y + iron.App.y();
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
				cameraType = cameraType == 0 ? 1 : 0;
				camHandle.position = cameraType;
				ViewportUtil.updateCameraType(cameraType);
				statusHandle.redraws = 2;
			}
			else if (Operator.shortcut(Config.keymap.view_orbit_left)) ViewportUtil.orbit(-Math.PI / 12, 0);
			else if (Operator.shortcut(Config.keymap.view_orbit_right)) ViewportUtil.orbit(Math.PI / 12, 0);
			else if (Operator.shortcut(Config.keymap.view_orbit_top)) ViewportUtil.orbit(0, -Math.PI / 12);
			else if (Operator.shortcut(Config.keymap.view_orbit_bottom)) ViewportUtil.orbit(0, Math.PI / 12);
			else if (Operator.shortcut(Config.keymap.view_orbit_opposite)) ViewportUtil.orbit(Math.PI, 0);
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
	}

	public function toggleDistractFree() {
		show = !show;
		App.resize();
	}

	function updateUI() {

		if (messageTimer > 0) {
			messageTimer -= Time.delta;
			if (messageTimer <= 0) statusHandle.redraws = 2;
		}

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		if (!App.uienabled) return;

		var down = Operator.shortcut(Config.keymap.action_paint) ||
				   (Operator.shortcut("alt+" + Config.keymap.action_paint) && Context.tool == ToolClone) ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint) ||
				   (Input.getPen().down() && !kb.down("alt"));
		if (down) {
			var mx = mouse.x;
			var my = mouse.y;
			if (paint2d) mx -= iron.App.w();

			if (mx < iron.App.w() && mx > iron.App.x() &&
				my < iron.App.h() && my > iron.App.y()) {
				
				if (Context.tool == ToolClone && kb.down("alt")) { // Clone source
					cloneStartX = mx;
					cloneStartY = my;
				}
				else {
					if (brushTime == 0 &&
						!App.isDragging &&
						@:privateAccess ui.comboSelectedHandle == null) { // Paint started
						History.pushUndo = true;
						if (Context.tool == ToolClone && cloneStartX >= 0.0) { // Clone delta
							cloneDeltaX = (cloneStartX - mx) / iron.App.w();
							cloneDeltaY = (cloneStartY - my) / iron.App.h();
							cloneStartX = -1;
						}
						else if (Context.tool == ToolParticle) {
							// Reset particles
							var emitter:MeshObject = cast Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							@:privateAccess psys.time = 0;
							// @:privateAccess psys.time = @:privateAccess psys.seed * @:privateAccess psys.animtime;
							// @:privateAccess psys.seed++;
						}
					}
					brushTime += Time.delta;
					if (onBrush != null) onBrush(0);
				}
			}

		}
		else if (brushTime > 0) { // Brush released
			brushTime = 0;
			Context.ddirty = 3;
			Context.brushBlendDirty = true; // Update brush mask
			Context.layerPreviewDirty = true; // Update layer preview
		}

		if (Context.layersPreviewDirty) {
			Context.layersPreviewDirty = false;
			Context.layerPreviewDirty = false;
			// Update all layer previews
			for (l in Project.layers) {
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
		if (Context.layerPreviewDirty) {
			Context.layerPreviewDirty = false;
			// Update layer preview
			var l = Context.layer;
			var target = Context.layerIsMask ? l.texpaint_mask_preview : l.texpaint_preview;
			var source = Context.layerIsMask ? l.texpaint_mask : l.texpaint;
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
			if (Context.tool == ToolPicker && pickerSelectMaterial) {
				var img = Context.material.imageIcon;
				g.drawImage(img, mx + 10, my + 10);
			}

			var cursorImg = Res.get('cursor.png');
			var psize = Std.int(cursorImg.width * (brushRadius * brushNodesRadius));

			// Clone source cursor
			var kb = Input.getKeyboard();
			if (Context.tool == ToolClone && !kb.down("alt") && (mouse.down() || pen.down())) {
				g.color = 0x66ffffff;
				g.drawScaledImage(cursorImg, mx + cloneDeltaX * iron.App.w() - psize / 2, my + cloneDeltaY * iron.App.h() - psize / 2, psize, psize);
				g.color = 0xffffffff;
			}

			var in2dView = UIView2D.inst.show && UIView2D.inst.type == 0 &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var decal = Context.tool == ToolDecal || Context.tool == ToolText;
			
			if (!brush3d || in2dView || decal) {
				if (decal) {
					psize = Std.int(256 * (brushRadius * brushNodesRadius));
					#if (kha_direct3d11 || kha_direct3d12)
					g.drawScaledImage(decalImage, mx - psize / 2, my - psize / 2, psize, psize);
					#else
					g.drawScaledImage(decalImage, mx - psize / 2, my - psize / 2 + psize, psize, -psize);
					#end
				}
				else if (Context.tool == ToolBrush  ||
						 Context.tool == ToolEraser ||
						 Context.tool == ToolClone  ||
						 Context.tool == ToolBlur   ||
						 Context.tool == ToolParticle) {
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

	public function show2DView(type = 0) {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UIView2D.inst.ui.endInput();
		if (UIView2D.inst.type != type) UIView2D.inst.show = true;
		else UIView2D.inst.show = !UIView2D.inst.show;
		UIView2D.inst.type = type;
		App.resize();
	}

	public function getImage(asset:zui.Canvas.TAsset):Image {
		return asset != null ? zui.Canvas.assetMap.get(asset.id) : null;
	}

	function renderUI(g:kha.graphics2.Graphics) {
		
		if (!App.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (App.uienabled && !ui.inputRegistered) ui.registerInput();

		if (!show) return;

		g.end();
		ui.begin(g);

		var panelx = (iron.App.x() - toolbarw);
		if (Config.raw.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(toolbarHandle, panelx, headerh, toolbarw, System.windowHeight())) {
			ui._y += 2;

			ui.imageScrollAlign = false;

			if (worktab.position == SpacePaint) {
				var keys = ['(B)', '(E)', '(G)', '(D)', '(T)', '(L) - Hold ALT to set source', '(U)', '(P)', '(K)', '(C)', '(V)'];
				var img = Res.get("icons.png");
				var imgw = ui.SCALE > 1 ? 100 : 50;
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
				var img = Res.get("icons.png");
				var imgw = ui.SCALE > 1 ? 100 : 50;
				ui._x += 2;
				if (Context.tool == ToolGizmo) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) Context.selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip("Gizmo (G)");
				ui._x -= 2;
				ui._y += 2;
			}

			ui.imageScrollAlign = true;
		}

		var panelx = iron.App.x() - toolbarw;
		if (Config.raw.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		
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

			if (ui.button("File", Left) || (UIMenu.show && ui.isHovered)) { UIMenu.show = true; UIMenu.menuCategory = 0; };
			if (ui.button("Edit", Left) || (UIMenu.show && ui.isHovered)) { UIMenu.show = true; UIMenu.menuCategory = 1; };
			if (ui.button("View", Left) || (UIMenu.show && ui.isHovered)) { UIMenu.show = true; UIMenu.menuCategory = 2; };
			if (ui.button("Help", Left) || (UIMenu.show && ui.isHovered)) { UIMenu.show = true; UIMenu.menuCategory = 3; };
			
			ui._w = _w;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.BUTTON_COL = BUTTON_COL;
		}
		ui.t.WINDOW_BG_COL = WINDOW_BG_COL;

		var panelx = (iron.App.x() - toolbarw) + menubarw;
		if (Config.raw.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(workspaceHandle, panelx, 0, System.windowWidth() - windowW - menubarw, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {
			ui.tab(worktab, "Paint");
			// ui.tab(worktab, "Sculpt");
			ui.tab(worktab, "Scene");
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
		if (Config.raw.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - toolbarw;
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth() - toolbarw - windowW, Std.int((ui.t.ELEMENT_H + 2) * ui.SCALE))) {

			if (worktab.position == SpacePaint) {

				if (Context.tool == ToolColorId) {
					ui.text("Picked Color");
					if (colorIdPicked) {
						ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
					}
					if (ui.button("Clear")) colorIdPicked = false;
					ui.text("Color ID Map");
					var cid = ui.combo(colorIdHandle, App.getEnumTexts(), "Color ID");
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
				else if (Context.tool == ToolBake) {
					ui.changed = false;
					var bakeHandle = Id.handle({position: bakeType});
					bakeType = ui.combo(bakeHandle, ["AO", "Curvature", "Normal (Tang)", "Normal (World)", "Position", "TexCoord", "Material ID", "Object ID"], "Bake");
					if (bakeType == 0 || bakeType == 1) {
						var bakeAxisHandle = Id.handle({position: bakeAxis});
						bakeAxis = ui.combo(bakeAxisHandle, ["XYZ", "X", "Y", "Z", "-X", "-Y", "-Z"], "Axis");
					}
					if (bakeType == 0) { // AO
						var strengthHandle = Id.handle({value: bakeAoStrength});
						bakeAoStrength = ui.slider(strengthHandle, "Strength", 0.0, 2.0, true);
						var radiusHandle = Id.handle({value: bakeAoRadius});
						bakeAoRadius = ui.slider(radiusHandle, "Radius", 0.0, 2.0, true);
						var offsetHandle = Id.handle({value: bakeAoOffset});
						bakeAoOffset = ui.slider(offsetHandle, "Offset", 0.0, 2.0, true);
					}
					if (bakeType == 1) { // Curvature
						var strengthHandle = Id.handle({value: bakeCurvStrength});
						bakeCurvStrength = ui.slider(strengthHandle, "Strength", 0.0, 2.0, true);
						var radiusHandle = Id.handle({value: bakeCurvRadius});
						bakeCurvRadius = ui.slider(radiusHandle, "Radius", 0.0, 2.0, true);
						var offsetHandle = Id.handle({value: bakeCurvOffset});
						bakeCurvOffset = ui.slider(offsetHandle, "Offset", 0.0, 2.0, true);
						var smoothHandle = Id.handle({value: bakeCurvSmooth});
						bakeCurvSmooth = Std.int(ui.slider(smoothHandle, "Smooth", 0, 5, false, 1));
					}
					if (bakeType == 2) { // Normal (Tang)
						var ar = [for (p in Project.paintObjects) p.name];
						var polyHandle = Id.handle({position: bakeHighPoly});
						bakeHighPoly = ui.combo(polyHandle, ar, "High Poly");
					}
					if (ui.changed) {
						MaterialParser.parsePaintMaterial();
						// Context.pdirty = 4;
						// pushUndo
					}
				}
				else {
					if (Context.tool != ToolFill) {
						brushRadius = ui.slider(brushRadiusHandle, "Radius", 0.0, 2.0, true);
					}
					
					if (Context.tool == ToolBrush  ||
						Context.tool == ToolFill   ||
						Context.tool == ToolDecal  ||
						Context.tool == ToolText) {
						var brushScaleHandle = Id.handle({value: brushScale});
						brushScale = ui.slider(brushScaleHandle, "UV Scale", 0.0, 2.0, true);
						if (brushScaleHandle.changed) {
							if (Context.tool == ToolDecal || Context.tool == ToolText) {
								ui.g.end();
								RenderUtil.makeDecalMaskPreview();
								RenderUtil.makeDecalPreview();
								ui.g.begin(false);
							}
							if (Context.layer.material_mask != null) {
								Layers.updateFillLayers();
							}
						}

						var brushRotHandle = Id.handle({value: brushRot});
						brushRot = ui.slider(brushRotHandle, "UV Rotate", 0.0, 360.0, true, 1);
						if (brushRotHandle.changed) {
							MaterialParser.parsePaintMaterial();
							if (Context.layer.material_mask != null) {
								Layers.updateFillLayers();
							}
						}
					}
					
					brushOpacity = ui.slider(brushOpacityHandle, "Opacity", 0.0, 1.0, true);
					
					if (Context.tool == ToolBrush || Context.tool == ToolEraser) {
						brushHardness = ui.slider(Id.handle({value: brushHardness}), "Hardness", 0.0, 1.0, true);
					}

					ui.combo(Id.handle(), ["Add"], "Blending");

					if (Context.tool == ToolBrush || Context.tool == ToolFill) {
						var paintHandle = Id.handle();
						brushPaint = ui.combo(paintHandle, ["UV Map", "Project", "Triplanar"], "TexCoord");
						if (paintHandle.changed) {
							MaterialParser.parsePaintMaterial();
							if (Context.layer.material_mask != null) {
								Layers.updateFillLayers();
							}
						}
					}
					if (Context.tool == ToolDecal) {
						ui.combo(decalMaskHandle, ["Rectangle", "Circle", "Triangle"], "Mask");
						if (decalMaskHandle.changed) {
							ui.g.end();
							RenderUtil.makeDecalMaskPreview();
							RenderUtil.makeDecalPreview();
							ui.g.begin(false);
						}
					}
					if (Context.tool == ToolText) {
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

					if (Context.tool == ToolFill) {
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
			cameraControls = ui.combo(Id.handle({position: cameraControls}), ["Orbit", "Rotate", "Fly"], "Controls");
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
		
		tabx = Config.raw.ui_layout == 0 ? System.windowWidth() - windowW : 0;
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
				Context.object = Context.paintObject;
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
				TabMeshes.draw();
				TabViewport.draw();
			}
		}

		ui.end();
		g.begin(false);
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
}
