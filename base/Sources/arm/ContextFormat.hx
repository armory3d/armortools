package arm;

import kha.Image;
import zui.Zui;
import zui.Id;
import iron.math.Vec4;
import iron.object.MeshObject;
import arm.shader.NodeShader;
import arm.ProjectFormat;
#if (is_paint || is_sculpt)
import iron.math.Mat4;
import iron.object.Object;
import iron.data.MaterialData;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
#end

@:structInit class TContext {
	@:optional public var texture: TAsset = null;
	@:optional public var paintObject: MeshObject;
	@:optional public var mergedObject: MeshObject = null; // For object mask
	@:optional public var mergedObjectIsAtlas = false; // Only objects referenced by atlas are merged

	@:optional public var ddirty = 0; // depth
	@:optional public var pdirty = 0; // paint
	@:optional public var rdirty = 0; // render
	@:optional public var brushBlendDirty = true;
	@:optional public var nodePreviewSocket = 0;

	@:optional public var splitView = false;
	@:optional public var viewIndex = -1;
	@:optional public var viewIndexLast = -1;

	@:optional public var swatch: TSwatchColor;
	@:optional public var pickedColor: TSwatchColor = Project.makeSwatch();
	@:optional public var colorPickerCallback: TSwatchColor->Void = null;

	@:optional public var defaultIrradiance: kha.arrays.Float32Array = null;
	@:optional public var defaultRadiance: Image = null;
	@:optional public var defaultRadianceMipmaps: Array<Image> = null;
	@:optional public var savedEnvmap: Image = null;
	@:optional public var emptyEnvmap: Image = null;
	@:optional public var previewEnvmap: Image = null;
	@:optional public var envmapLoaded = false;
	@:optional public var showEnvmap = false;
	@:optional public var showEnvmapHandle = new Handle({ selected: false });
	@:optional public var showEnvmapBlur = false;
	@:optional public var showEnvmapBlurHandle = new Handle({ selected: false });
	@:optional public var envmapAngle = 0.0;
	@:optional public var lightAngle = 0.0;
	@:optional public var cullBackfaces = true;
	@:optional public var textureFilter = true;

	@:optional public var formatType = FormatPng;
	@:optional public var formatQuality = 100.0;
	@:optional public var layersDestination = DestinationDisk;
	@:optional public var splitBy = SplitObject;
	@:optional public var parseTransform = true;
	@:optional public var parseVCols = false;

	@:optional public var selectTime = 0.0;
	#if (kha_direct3d12 || kha_vulkan)
	@:optional public var pathTraceMode = TraceCore;
	#end
	#if (kha_direct3d12 || kha_vulkan)
	@:optional public var viewportMode = ViewPathTrace;
	#else
	@:optional public var viewportMode = ViewLit;
	#end
	#if (krom_android || krom_ios || arm_vr)
	@:optional public var renderMode = RenderForward;
	#else
	@:optional public var renderMode = RenderDeferred;
	#end

	@:optional public var viewportShader: NodeShader->String = null;
	@:optional public var hscaleWasChanged = false;
	@:optional public var exportMeshFormat = FormatObj;
	@:optional public var exportMeshIndex = 0;
	@:optional public var packAssetsOnExport = true;

	@:optional public var paintVec = new Vec4();
	@:optional public var lastPaintX = -1.0;
	@:optional public var lastPaintY = -1.0;
	@:optional public var foregroundEvent = false;
	@:optional public var painted = 0;
	@:optional public var brushTime = 0.0;
	@:optional public var cloneStartX = -1.0;
	@:optional public var cloneStartY = -1.0;
	@:optional public var cloneDeltaX = 0.0;
	@:optional public var cloneDeltaY = 0.0;

	@:optional public var showCompass = true;
	@:optional public var projectType = ModelRoundedCube;
	@:optional public var projectAspectRatio = 0; // 1:1, 2:1, 1:2
	@:optional public var projectObjects: Array<MeshObject>;

	@:optional public var lastPaintVecX = -1.0;
	@:optional public var lastPaintVecY = -1.0;
	@:optional public var prevPaintVecX = -1.0;
	@:optional public var prevPaintVecY = -1.0;
	@:optional public var frame = 0;
	@:optional public var paint2dView = false;

	@:optional public var lockStartedX = -1.0;
	@:optional public var lockStartedY = -1.0;
	@:optional public var brushLocked = false;
	@:optional public var brushCanLock = false;
	@:optional public var brushCanUnlock = false;
	@:optional public var cameraType = CameraPerspective;
	@:optional public var camHandle = new Handle();
	@:optional public var fovHandle: Handle = null;
	@:optional public var undoHandle: Handle = null;
	@:optional public var hssao: Handle = null;
	@:optional public var hssr: Handle = null;
	@:optional public var hbloom: Handle = null;
	@:optional public var hsupersample: Handle = null;
	@:optional public var hvxao: Handle = null;
	@:optional public var vxaoExt = 1.0;
	@:optional public var vxaoOffset = 1.5;
	@:optional public var vxaoAperture = 1.2;
	@:optional public var textureExportPath = "";
	@:optional public var lastStatusPosition = 0;
	@:optional public var cameraControls = ControlsOrbit;
	@:optional public var penPaintingOnly = false; // Reject painting with finger when using pen

	#if (is_paint || is_sculpt)
	@:optional public var material: MaterialSlot;
	@:optional public var layer: LayerSlot;
	@:optional public var brush: BrushSlot;
	@:optional public var font: FontSlot;
	@:optional public var tool = ToolBrush;

	@:optional public var layerPreviewDirty = true;
	@:optional public var layersPreviewDirty = false;
	@:optional public var nodePreviewDirty = false;
	@:optional public var nodePreview: Image = null;
	@:optional public var nodePreviews: Map<String, Image> = null;
	@:optional public var nodePreviewsUsed: Array<String> = null;
	@:optional public var maskPreviewRgba32: kha.Image = null;
	@:optional public var maskPreviewLast: LayerSlot = null;

	@:optional public var colorIdPicked = false;
	@:optional public var materialPreview = false; // Drawing material previews
	@:optional public var savedCamera = Mat4.identity();

	@:optional public var colorPickerPreviousTool = ToolBrush;
	@:optional public var materialIdPicked = 0;
	@:optional public var uvxPicked = 0.0;
	@:optional public var uvyPicked = 0.0;
	@:optional public var pickerSelectMaterial = true;
	@:optional public var pickerMaskHandle = new Handle();
	@:optional public var pickPosNorTex = false;
	@:optional public var posXPicked = 0.0;
	@:optional public var posYPicked = 0.0;
	@:optional public var posZPicked = 0.0;
	@:optional public var norXPicked = 0.0;
	@:optional public var norYPicked = 0.0;
	@:optional public var norZPicked = 0.0;

	@:optional public var drawWireframe = false;
	@:optional public var wireframeHandle = new Handle({ selected: false });
	@:optional public var drawTexels = false;
	@:optional public var texelsHandle = new Handle({ selected: false });

	@:optional public var colorIdHandle = Id.handle();
	@:optional public var layersExport = ExportVisible;

	@:optional public var decalImage: Image = null;
	@:optional public var decalPreview = false;
	@:optional public var decalX = 0.0;
	@:optional public var decalY = 0.0;

	@:optional public var cacheDraws = false;
	@:optional public var writeIconOnExport = false;

	@:optional public var textToolImage: Image = null;
	@:optional public var textToolText: String;
	@:optional public var particleMaterial: MaterialData = null;
	#if arm_physics
	@:optional public var particlePhysics = false;
	@:optional public var particleHitX = 0.0;
	@:optional public var particleHitY = 0.0;
	@:optional public var particleHitZ = 0.0;
	@:optional public var lastParticleHitX = 0.0;
	@:optional public var lastParticleHitY = 0.0;
	@:optional public var lastParticleHitZ = 0.0;
	@:optional public var particleTimer: iron.system.Tween.TAnim = null;
	@:optional public var paintBody: arm.plugin.PhysicsBody = null;
	#end

	@:optional public var layerFilter = 0;
	@:optional public var runBrush: Int->Void = null;
	@:optional public var parseBrushInputs: Void->Void = null;

	@:optional public var gizmo: Object = null;
	@:optional public var gizmoTranslateX: Object = null;
	@:optional public var gizmoTranslateY: Object = null;
	@:optional public var gizmoTranslateZ: Object = null;
	@:optional public var gizmoScaleX: Object = null;
	@:optional public var gizmoScaleY: Object = null;
	@:optional public var gizmoScaleZ: Object = null;
	@:optional public var gizmoRotateX: Object = null;
	@:optional public var gizmoRotateY: Object = null;
	@:optional public var gizmoRotateZ: Object = null;
	@:optional public var gizmoStarted = false;
	@:optional public var gizmoOffset = 0.0;
	@:optional public var gizmoDrag = 0.0;
	@:optional public var gizmoDragLast = 0.0;
	@:optional public var translateX = false;
	@:optional public var translateY = false;
	@:optional public var translateZ = false;
	@:optional public var scaleX = false;
	@:optional public var scaleY = false;
	@:optional public var scaleZ = false;
	@:optional public var rotateX = false;
	@:optional public var rotateY = false;
	@:optional public var rotateZ = false;

	@:optional public var brushNodesRadius = 1.0;
	@:optional public var brushNodesOpacity = 1.0;
	@:optional public var brushMaskImage: Image = null;
	@:optional public var brushMaskImageIsAlpha = false;
	@:optional public var brushStencilImage: Image = null;
	@:optional public var brushStencilImageIsAlpha = false;
	@:optional public var brushStencilX = 0.02;
	@:optional public var brushStencilY = 0.02;
	@:optional public var brushStencilScale = 0.9;
	@:optional public var brushStencilScaling = false;
	@:optional public var brushStencilAngle = 0.0;
	@:optional public var brushStencilRotating = false;
	@:optional public var brushNodesScale = 1.0;
	@:optional public var brushNodesAngle = 0.0;
	@:optional public var brushNodesHardness = 1.0;
	@:optional public var brushDirectional = false;

	@:optional public var brushRadius = 0.5;
	@:optional public var brushRadiusHandle = new Handle({ value: 0.5 });
	@:optional public var brushScaleX = 1.0;
	@:optional public var brushDecalMaskRadius = 0.5;
	@:optional public var brushDecalMaskRadiusHandle = new Handle({ value: 0.5 });
	@:optional public var brushScaleXHandle = new Handle({ value: 1.0 });
	@:optional public var brushBlending = BlendMix;
	@:optional public var brushOpacity = 1.0;
	@:optional public var brushOpacityHandle = new Handle({ value: 1.0 });
	@:optional public var brushScale = 1.0;
	@:optional public var brushAngle = 0.0;
	@:optional public var brushAngleHandle = new Handle({ value: 0.0 });
	#if is_paint
	@:optional public var brushHardness = 0.8;
	#end
	#if is_sculpt
	@:optional public var brushHardness = 0.05;
	#end
	@:optional public var brushLazyRadius = 0.0;
	@:optional public var brushLazyStep = 0.0;
	@:optional public var brushLazyX = 0.0;
	@:optional public var brushLazyY = 0.0;
	@:optional public var brushPaint = UVMap;
	@:optional public var brushAngleRejectDot = 0.5;
	@:optional public var bakeType = BakeAO;
	@:optional public var bakeAxis = BakeXYZ;
	@:optional public var bakeUpAxis = BakeUpZ;
	@:optional public var bakeSamples = 128;
	@:optional public var bakeAoStrength = 1.0;
	@:optional public var bakeAoRadius = 1.0;
	@:optional public var bakeAoOffset = 1.0;
	@:optional public var bakeCurvStrength = 1.0;
	@:optional public var bakeCurvRadius = 1.0;
	@:optional public var bakeCurvOffset = 0.0;
	@:optional public var bakeCurvSmooth = 1;
	@:optional public var bakeHighPoly = 0;

	@:optional public var xray = false;
	@:optional public var symX = false;
	@:optional public var symY = false;
	@:optional public var symZ = false;
	@:optional public var fillTypeHandle = new Handle();

	@:optional public var paint2d = false;

	@:optional public var lastHtab0Position = 0;
	@:optional public var maximizedSidebarWidth = 0;
	@:optional public var dragDestination = 0;
	#end

	#if is_lab
	@:optional public var material: Dynamic; ////
	@:optional public var layer: Dynamic; ////
	@:optional public var tool = ToolEraser;

	@:optional public var colorPickerPreviousTool = ToolEraser;

	@:optional public var brushRadius = 0.25;
	@:optional public var brushRadiusHandle = new Handle({ value: 0.25 });
	@:optional public var brushScale = 1.0;

	@:optional public var coords = new Vec4();
	@:optional public var startX = 0.0;
	@:optional public var startY = 0.0;

	// Brush ruler
	@:optional public var lockBegin = false;
	@:optional public var lockX = false;
	@:optional public var lockY = false;
	@:optional public var lockStartX = 0.0;
	@:optional public var lockStartY = 0.0;
	@:optional public var registered = false;
	#end
}
