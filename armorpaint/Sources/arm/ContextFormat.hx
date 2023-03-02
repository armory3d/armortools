package arm;

import kha.Image;
import zui.Zui;
import zui.Id;
import iron.math.Mat4;
import iron.object.Object;
import iron.data.MaterialData;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.ProjectBaseFormat;
import arm.ContextBaseFormat;

@:structInit class TContext extends TContextBase {
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
	@:optional public var brushHardness = 0.8;
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
}
