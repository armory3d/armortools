package arm;

import kha.Image;
import zui.Zui;
import zui.Id;
import iron.RenderPath;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.object.Object;
import iron.object.MeshObject;
import iron.data.MaterialData;
import arm.shader.NodeShader;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathForward;
import arm.ui.UISidebar;
import arm.ui.UINodes;
import arm.ui.UIHeader;
import arm.ui.BoxPreferences;
import arm.node.MakeMaterial;
import arm.Enums;
import arm.ProjectFormat;

class Context {

	public static var material: Dynamic; ////
	public static var paintObject: MeshObject;
	public static var mergedObject: MeshObject = null;
	public static var texture: TAsset = null;
	public static var tool = ToolEraser;

	public static var ddirty = 0; // depth
	public static var pdirty = 0; // paint
	public static var rdirty = 0; // render
	public static var brushBlendDirty = true;
	public static var nodePreviewSocket = 0;

	public static var viewIndexLast = -1;

	public static var swatch: TSwatchColor;
	public static var pickedColor: TSwatchColor = Project.makeSwatch();
	public static var colorPickerCallback: TSwatchColor->Void = null;
	public static var colorPickerPreviousTool = ToolEraser;

	public static var defaultIrradiance: kha.arrays.Float32Array = null;
	public static var defaultRadiance: Image = null;
	public static var defaultRadianceMipmaps: Array<Image> = null;
	public static var savedEnvmap: Image = null;
	public static var emptyEnvmap: Image = null;
	public static var previewEnvmap: Image = null;
	public static var envmapLoaded = false;
	public static var showEnvmap = false;
	public static var showEnvmapHandle = new Handle({ selected: false });
	public static var showEnvmapBlur = false;
	public static var showEnvmapBlurHandle = new Handle({ selected: false });
	public static var envmapAngle = 0.0;
	public static var lightAngle = 0.0;
	public static var cullBackfaces = true;
	public static var textureFilter = true;

	public static var formatType = FormatPng;
	public static var formatQuality = 100.0;
	public static var layersDestination = DestinationDisk;
	public static var splitBy = SplitObject;
	public static var parseTransform = true;
	public static var parseVCols = false;

	public static var selectTime = 0.0;
	#if (kha_direct3d12 || kha_vulkan)
	public static var viewportMode = ViewPathTrace;
	#else
	public static var viewportMode = ViewLit;
	#end
	#if (krom_android || krom_ios || arm_vr)
	public static var renderMode = RenderForward;
	#else
	public static var renderMode = RenderDeferred;
	#end
	#if (kha_direct3d12 || kha_vulkan)
	public static var pathTraceMode = TraceCore;
	#end
	public static var viewportShader: NodeShader->String = null;
	public static var hscaleWasChanged = false;
	public static var exportMeshFormat = FormatObj;
	public static var exportMeshIndex = 0;
	public static var packAssetsOnExport = true;

	public static var paintVec = new Vec4();
	public static var lastPaintX = -1.0;
	public static var lastPaintY = -1.0;
	public static var foregroundEvent = false;
	public static var painted = 0;
	public static var brushTime = 0.0;
	public static var cloneStartX = -1.0;
	public static var cloneStartY = -1.0;
	public static var cloneDeltaX = 0.0;
	public static var cloneDeltaY = 0.0;

	public static var brushRadius = 0.25;
	public static var brushRadiusHandle = new Handle({ value: 0.25 });
	public static var brushScale = 1.0;

	public static var blurDirectional = false;
	public static var showCompass = true;
	public static var projectType = ModelRoundedCube;
	public static var projectAspectRatio = 0; // 1:1, 2:1, 1:2
	public static var projectObjects: Array<MeshObject>;

	public static var lastPaintVecX = -1.0;
	public static var lastPaintVecY = -1.0;
	public static var prevPaintVecX = -1.0;
	public static var prevPaintVecY = -1.0;
	public static var frame = 0;

	public static var lockStartedX = -1.0;
	public static var lockStartedY = -1.0;
	public static var brushLocked = false;
	public static var brushCanLock = false;
	public static var brushCanUnlock = false;
	public static var cameraType = CameraPerspective;
	public static var camHandle = new Handle();
	public static var fovHandle: Handle = null;
	public static var undoHandle: Handle = null;
	public static var hssgi: Handle = null;
	public static var hssr: Handle = null;
	public static var hbloom: Handle = null;
	public static var hsupersample: Handle = null;
	public static var hvxao: Handle = null;
	public static var vxaoExt = 1.0;
	public static var vxaoOffset = 1.5;
	public static var vxaoAperture = 1.2;
	public static var textureExportPath = "";
	public static var lastStatusPosition = 0;
	public static var cameraControls = ControlsOrbit;
	public static var penPaintingOnly = false; // Reject painting with finger when using pen

	static var coords = new Vec4();
	static var startX = 0.0;
	static var startY = 0.0;

	// Brush ruler
	static var lockBegin = false;
	static var lockX = false;
	static var lockY = false;
	static var lockStartX = 0.0;
	static var lockStartY = 0.0;
	static var registered = false;

	public static function setViewportMode(mode: ViewportMode) {
		if (mode == viewportMode) return;

		viewportMode = mode;
		var deferred = Context.renderMode != RenderForward && (Context.viewportMode == ViewLit || Context.viewportMode == ViewPathTrace);
		if (deferred) {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		// else if (Context.viewportMode == ViewPathTrace) {
		// }
		else {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = Space3D;
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.worktab.position = _workspace;
	}

	public static function layerFilterUsed(): Bool { return true; } ////

	public static function setSwatch(s: TSwatchColor) {
		swatch = s;
		App.notifyOnNextFrame(function() {
			// MakeMaterial.parsePaintMaterial();
			// RenderUtil.makeMaterialPreview();
			// UISidebar.inst.hwnd1.redraws = 2;
		});
	}

	public static function selectTool(i: Int) {
		tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		ddirty = 3;
	}

	public static function selectPaintObject(o: MeshObject) {
		paintObject = o;
	}

	public static function mainObject(): MeshObject {
		return Project.paintObjects[0];
	}

	public static function loadEnvmap() {
		if (!envmapLoaded) {
			// TODO: Unable to share texture for both radiance and envmap - reload image
			envmapLoaded = true;
			iron.data.Data.cachedImages.remove("World_radiance.k");
		}
		iron.Scene.active.world.loadEnvmap(function(_) {});
		if (Context.savedEnvmap == null) Context.savedEnvmap = iron.Scene.active.world.envmap;
	}

	@:keep
	public static function setViewportShader(viewportShader: NodeShader->String) {
		Context.viewportShader = viewportShader;
		setRenderPath();
	}

	public static function setRenderPath() {
		if (Context.renderMode == RenderForward || Context.viewportShader != null) {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		else {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		iron.App.notifyOnInit(function() {
			MakeMaterial.parseMeshMaterial();
		});
	}

	public static function enableImportPlugin(file: String): Bool {
		// Return plugin name suitable for importing the specified file
		if (BoxPreferences.filesPlugin == null) {
			BoxPreferences.fetchPlugins();
		}
		var ext = file.substr(file.lastIndexOf(".") + 1);
		for (f in BoxPreferences.filesPlugin) {
			if (f.startsWith("import_") && f.indexOf(ext) >= 0) {
				Config.enablePlugin(f);
				Console.info(f + " " + tr("plugin enabled"));
				return true;
			}
		}
		return false;
	}

	public static function runBrush(from: Int) {
		var left = 0.0;
		var right = 1.0;

		// First time init
		if (Context.lastPaintX < 0 || Context.lastPaintY < 0) {
			Context.lastPaintVecX = Context.paintVec.x;
			Context.lastPaintVecY = Context.paintVec.y;
		}

		var inpaint = UINodes.inst.getNodes().nodesSelected.length > 0 && UINodes.inst.getNodes().nodesSelected[0].type == "InpaintNode";

		// Paint bounds
		if (inpaint &&
			Context.paintVec.x > left &&
			Context.paintVec.x < right &&
			Context.paintVec.y > 0 &&
			Context.paintVec.y < 1 &&
			!arm.App.isDragging &&
			!arm.App.isResizing &&
			!arm.App.isScrolling() &&
			!arm.App.isComboSelected()) {

			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();

			// Prevent painting the same spot
			var sameSpot = Context.paintVec.x == Context.lastPaintX && Context.paintVec.y == Context.lastPaintY;
			if (down && sameSpot) {
				Context.painted++;
			}
			else {
				Context.painted = 0;
			}
			Context.lastPaintX = Context.paintVec.x;
			Context.lastPaintY = Context.paintVec.y;

			if (Context.painted == 0) {
				parseBrushInputs();
			}

			if (Context.painted <= 1) {
				Context.pdirty = 1;
				Context.rdirty = 2;
			}
		}
	}

	public static function parseBrushInputs() {
		if (!registered) {
			registered = true;
			iron.App.notifyOnUpdate(update);
		}

		Context.paintVec = coords;
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var paintX = mouse.viewX / iron.App.w();
		var paintY = mouse.viewY / iron.App.h();
		if (mouse.started()) {
			startX = mouse.viewX / iron.App.w();
			startY = mouse.viewY / iron.App.h();
		}

		var pen = iron.system.Input.getPen();
		if (pen.down()) {
			paintX = pen.viewX / iron.App.w();
			paintY = pen.viewY / iron.App.h();
		}
		if (pen.started()) {
			startX = pen.viewX / iron.App.w();
			startY = pen.viewY / iron.App.h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
			if (lockX) paintX = startX;
			if (lockY) paintY = startY;
		}

		coords.x = paintX;
		coords.y = paintY;

		if (lockBegin) {
			var dx = Math.abs(lockStartX - mouse.viewX);
			var dy = Math.abs(lockStartY - mouse.viewY);
			if (dx > 1 || dy > 1) {
				lockBegin = false;
				dx > dy ? lockY = true : lockX = true;
			}
		}

		var kb = iron.system.Input.getKeyboard();
		if (kb.started(Config.keymap.brush_ruler)) {
			lockStartX = mouse.viewX;
			lockStartY = mouse.viewY;
			lockBegin = true;
		}
		else if (kb.released(Config.keymap.brush_ruler)) {
			lockX = lockY = lockBegin = false;
		}

		Context.parseBrushInputs();
	}
}
