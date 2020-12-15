package arm;

import kha.Image;
import zui.Zui;
import zui.Id;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.object.Object;
import iron.object.MeshObject;
import iron.data.MaterialData;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.util.ParticleUtil;
import arm.ui.UISidebar;
import arm.ui.UIToolbar;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIHeader;
import arm.node.MakeMaterial;
import arm.Enums;
import arm.ProjectFormat;

class Context {

	public static var material: MaterialSlot;
	public static var materialScene: MaterialSlot;
	public static var layer: LayerSlot;
	public static var layerIsMask = false; // Mask selected for active layer
	public static var brush: BrushSlot;
	public static var font: FontSlot;
	public static var texture: TAsset = null;
	public static var object: Object;
	public static var paintObject: MeshObject;
	public static var mergedObject: MeshObject = null; // For object mask
	public static var tool = 0;

	public static var ddirty = 0; // depth
	public static var pdirty = 0; // paint
	public static var rdirty = 0; // render
	public static var brushBlendDirty = true;
	public static var layerPreviewDirty = true;
	public static var layersPreviewDirty = false;
	public static var nodePreviewDirty = false;
	public static var nodePreviewSocket = 0;
	public static var nodePreview: Image = null;
	public static var nodePreviewsBlur: Map<String, Image> = null;
	public static var nodePreviewsWarp: Map<String, Image> = null;


	public static var colorIdPicked = false;
	public static var splitView = false;
	public static var viewIndex = -1;
	public static var viewIndexLast = -1;
	public static var materialPreview = false; // Drawing material previews
	public static var savedCamera = Mat4.identity();
	public static var baseRPicked = 0.0;
	public static var baseGPicked = 0.0;
	public static var baseBPicked = 0.0;
	public static var normalRPicked = 0.0;
	public static var normalGPicked = 0.0;
	public static var normalBPicked = 0.0;
	public static var roughnessPicked = 0.0;
	public static var metallicPicked = 0.0;
	public static var occlusionPicked = 0.0;
	public static var materialIdPicked = 0;
	public static var uvxPicked = 0.0;
	public static var uvyPicked = 0.0;
	public static var pickerSelectMaterial = true;
	public static var pickerMaskHandle = new Handle();
	public static var pickPosNor = false;
	public static var posXPicked = 0.0;
	public static var posYPicked = 0.0;
	public static var posZPicked = 0.0;
	public static var norXPicked = 0.0;
	public static var norYPicked = 0.0;
	public static var norZPicked = 0.0;

	public static var defaultEnvmap: Image = null;
	public static var defaultIrradiance: kha.arrays.Float32Array = null;
	public static var defaultRadiance: Image = null;
	public static var defaultRadianceMipmaps: Array<Image> = null;
	public static var savedEnvmap: Image = null;
	public static var emptyEnvmap: Image = null;
	public static var previewEnvmap: Image = null;
	public static var envmapLoaded = false;
	public static var showEnvmap = false;
	public static var showEnvmapHandle = new Handle({selected: false});
	public static var showEnvmapBlur = false;
	public static var showEnvmapBlurHandle = new Handle({selected: false});
	public static var envmapAngle = 0.0;
	public static var drawWireframe = false;
	public static var wireframeHandle = new Handle({selected: false});
	public static var drawTexels = false;
	public static var texelsHandle = new Handle({selected: false});
	public static var cullBackfaces = true;
	public static var textureFilter = true;

	public static var colorIdHandle = Id.handle();
	public static var formatType = FormatPng;
	public static var formatQuality = 100.0;
	public static var layersExport = ExportVisible;
	public static var splitBy = SplitObject;
	public static var parseTransform = false;
	public static var parseVCols = false;

	public static var selectTime = 0.0;
	public static var decalImage: Image = null;
	public static var decalPreview = false;
	public static var decalX = 0.0;
	public static var decalY = 0.0;
	public static var viewportMode = ViewLit;
	#if (krom_android || krom_ios || arm_vr)
	public static var renderMode = RenderForward;
	#else
	public static var renderMode = RenderDeferred;
	#end
	#if (kha_direct3d12 || kha_vulkan)
	public static var pathTraceMode = TraceCore;
	#end
	public static var hscaleWasChanged = false;
	public static var exportMeshFormat = FormatObj;
	public static var cacheDraws = false;

	public static var textToolImage: Image = null;
	public static var textToolText: String;
	public static var particleMaterial: MaterialData = null;

	public static var layerFilter = 0;
	public static var runBrush: Int->Void = null;
	public static var parseBrushInputs: Void->Void = null;
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

	public static var gizmo: Object = null;
	public static var gizmoTranslateX: Object = null;
	public static var gizmoTranslateY: Object = null;
	public static var gizmoTranslateZ: Object = null;
	public static var gizmoScaleX: Object = null;
	public static var gizmoScaleY: Object = null;
	public static var gizmoScaleZ: Object = null;
	public static var gizmoRotateX: Object = null;
	public static var gizmoRotateY: Object = null;
	public static var gizmoRotateZ: Object = null;
	public static var gizmoStarted = false;
	public static var gizmoOffset = 0.0;
	public static var gizmoDrag = 0.0;
	public static var gizmoDragLast = 0.0;
	public static var translateX = false;
	public static var translateY = false;
	public static var translateZ = false;
	public static var scaleX = false;
	public static var scaleY = false;
	public static var scaleZ = false;
	public static var rotateX = false;
	public static var rotateY = false;
	public static var rotateZ = false;

	public static var brushNodesRadius = 1.0;
	public static var brushNodesOpacity = 1.0;
	public static var brushMaskImage: Image = null;
	public static var brushStencilImage: Image = null;
	public static var brushStencilX = 0.02;
	public static var brushStencilY = 0.02;
	public static var brushStencilScale = 0.9;
	public static var brushStencilScaling = false;
	public static var brushStencilAngle = 0.0;
	public static var brushStencilRotating = false;
	public static var brushNodesScale = 1.0;
	public static var brushNodesAngle = 0.0;
	public static var brushNodesHardness = 1.0;
	public static var brushDirectional = false;

	public static var brushRadius = 0.5;
	public static var brushRadiusHandle = new Handle({value: 0.5});
	public static var brushDecalMaskRadius = 0.5;
	public static var brushDecalMaskRadiusHandle = new Handle({value: 0.5});
	public static var brushScaleX = 1.0;
	public static var brushScaleXHandle = new Handle({value: 1.0});
	public static var brushBlending = BlendMix;
	public static var brushOpacity = 1.0;
	public static var brushOpacityHandle = new Handle({value: 1.0});
	public static var brushScale = 1.0;
	public static var brushAngle = 0.0;
	public static var brushAngleHandle = new Handle({value: 0.0});
	public static var brushHardness = 0.8;
	public static var brushLazyRadius = 0.0;
	public static var brushLazyStep = 0.0;
	public static var brushLazyX = 0.0;
	public static var brushLazyY = 0.0;
	public static var brushPaint = UVMap;
	public static var brushDepthReject = true;
	public static var brushAngleReject = true;
	public static var brushAngleRejectDot = 0.5;
	public static var bakeType = BakeAO;
	public static var bakeAxis = BakeXYZ;
	public static var bakeUpAxis = BakeUpZ;
	public static var bakeAoStrength = 1.0;
	public static var bakeAoRadius = 1.0;
	public static var bakeAoOffset = 1.0;
	public static var bakeCurvStrength = 1.0;
	public static var bakeCurvRadius = 1.0;
	public static var bakeCurvOffset = 0.0;
	public static var bakeCurvSmooth = 1;
	public static var bakeHighPoly = 0;

	public static var xray = false;
	public static var symX = false;
	public static var symY = false;
	public static var symZ = false;
	public static var showCompass = true;
	public static var fillTypeHandle = new Handle();
	public static var projectType = ModelRoundedCube;
	public static var projectAspectRatio = 0; // 1:1, 2:1, 1:2
	public static var projectObjects: Array<MeshObject>;

	public static var lastPaintVecX = -1.0;
	public static var lastPaintVecY = -1.0;
	public static var prevPaintVecX = -1.0;
	public static var prevPaintVecY = -1.0;
	public static var frame = 0;
	public static var paint2d = false;
	public static var paint2dView = false;

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
	public static var lastCombo: Handle = null;
	public static var lastTooltip: Image = null;
	public static var lastStatusPosition = 0;
	public static var cameraControls = ControlsOrbit;
	public static var dragDestination = 0;

	public static function selectMaterialScene(i: Int) {
		if (Project.materialsScene.length <= i || object == paintObject) return;
		materialScene = Project.materialsScene[i];
		if (Std.is(object, MeshObject)) {
			var mats = cast(object, MeshObject).materials;
			for (i in 0...mats.length) mats[i] = materialScene.data;
		}
		MakeMaterial.parsePaintMaterial();
		UISidebar.inst.hwnd0.redraws = 2;
	}

	public static function selectMaterial(i: Int) {
		if (Project.materials.length <= i) return;
		setMaterial(Project.materials[i]);
	}

	public static function setMaterial(m: MaterialSlot) {
		if (Project.materials.indexOf(m) == -1) return;
		material = m;
		MakeMaterial.parsePaintMaterial();
		UISidebar.inst.hwnd1.redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;

		var decal = tool == ToolDecal || tool == ToolText;
		if (decal) {
			function _next() {
				RenderUtil.makeDecalPreview();
			}
			App.notifyOnNextFrame(_next);
		}
	}

	public static function selectBrush(i: Int) {
		if (Project.brushes.length <= i) return;
		setBrush(Project.brushes[i]);
	}

	public static function setBrush(b: BrushSlot) {
		if (Project.brushes.indexOf(b) == -1) return;
		brush = b;
		MakeMaterial.parseBrush();
		Context.parseBrushInputs();
		UISidebar.inst.hwnd1.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
	}

	public static function selectFont(i: Int) {
		if (Project.fonts.length <= i) return;
		setFont(Project.fonts[i]);
	}

	public static function setFont(f: FontSlot) {
		if (Project.fonts.indexOf(f) == -1) return;
		font = f;
		RenderUtil.makeTextPreview();
		RenderUtil.makeDecalPreview();
		UISidebar.inst.hwnd2.redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	public static function setLayer(l: LayerSlot, isMask = false) {
		if (l == layer && layerIsMask == isMask) return;
		layer = l;
		layerIsMask = isMask;
		UIHeader.inst.headerHandle.redraws = 2;

		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		Layers.setObjectMask();
		MakeMaterial.parseMeshMaterial();
		MakeMaterial.parsePaintMaterial();

		if (current != null) current.begin(false);

		UISidebar.inst.hwnd0.redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	public static function selectTool(i: Int) {
		tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		ddirty = 3;
		initTool();
	}

	public static function initTool() {
		var decal = tool == ToolDecal || tool == ToolText;
		if (decal) {
			if (tool == ToolText) {
				RenderUtil.makeTextPreview();
			}
			RenderUtil.makeDecalPreview();
		}
		if (tool == ToolParticle) {
			ParticleUtil.initParticle();
			MakeMaterial.parseParticleMaterial();
		}
	}

	public static function selectObject(o: Object) {
		object = o;

		if (UIHeader.inst.worktab.position == SpaceRender) {
			if (Std.is(o, MeshObject)) {
				for (i in 0...Project.materialsScene.length) {
					if (Project.materialsScene[i].data == cast(o, MeshObject).materials[0]) {
						// selectMaterial(i); // loop
						materialScene = Project.materialsScene[i];
						UISidebar.inst.hwnd0.redraws = 2;
						break;
					}
				}
			}
		}
	}

	public static function selectPaintObject(o: MeshObject) {
		UIHeader.inst.headerHandle.redraws = 2;
		for (p in Project.paintObjects) p.skip_context = "paint";
		paintObject = o;

		var mask = layer.objectMask;
		if (Context.layerFilter > 0) mask = Context.layerFilter;

		if (mergedObject == null || mask > 0) {
			paintObject.skip_context = "";
		}
		UVUtil.uvmapCached = false;
		UVUtil.trianglemapCached = false;
		UVUtil.dilatemapCached = false;
	}

	public static function mainObject(): MeshObject {
		for (po in Project.paintObjects) if (po.children.length > 0) return po;
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
}
