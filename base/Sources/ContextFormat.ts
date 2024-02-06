/// <reference path='./Project.ts'/>
/// <reference path='./Enums.ts'/>

// type TContext = {
class TContext {
	texture?: TAsset = null;
	paintObject?: mesh_object_t;
	mergedObject?: mesh_object_t = null; // For object mask
	mergedObjectIsAtlas? = false; // Only objects referenced by atlas are merged

	ddirty? = 0; // depth
	pdirty? = 0; // paint
	rdirty? = 0; // render
	brushBlendDirty? = true;
	nodePreviewSocket? = 0;

	splitView? = false;
	viewIndex? = -1;
	viewIndexLast? = -1;

	swatch?: TSwatchColor;
	pickedColor?: TSwatchColor = Project.makeSwatch();
	colorPickerCallback?: (sc: TSwatchColor)=>void = null;

	defaultIrradiance?: Float32Array = null;
	defaultRadiance?: image_t = null;
	defaultRadianceMipmaps?: image_t[] = null;
	savedEnvmap?: image_t = null;
	emptyEnvmap?: image_t = null;
	previewEnvmap?: image_t = null;
	envmapLoaded? = false;
	showEnvmap? = false;
	showEnvmapHandle? = zui_handle_create({ selected: false });
	showEnvmapBlur? = false;
	showEnvmapBlurHandle? = zui_handle_create({ selected: false });
	envmapAngle? = 0.0;
	lightAngle? = 0.0;
	cullBackfaces? = true;
	textureFilter? = true;

	formatType? = TextureLdrFormat.FormatPng;
	formatQuality? = 100.0;
	layersDestination? = ExportDestination.DestinationDisk;
	splitBy? = SplitType.SplitObject;
	parseTransform? = true;
	parseVCols? = false;

	selectTime? = 0.0;
	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	pathTraceMode? = PathTraceMode.TraceCore;
	///end
	///if (krom_direct3d12 || krom_vulkan) // || krom_metal)
	viewportMode? = ViewportMode.ViewPathTrace;
	///else
	viewportMode? = ViewportMode.ViewLit;
	///end
	///if (krom_android || krom_ios)
	renderMode? = RenderMode.RenderForward;
	///else
	renderMode? = RenderMode.RenderDeferred;
	///end

	viewportShader?: (ns: NodeShaderRaw)=>string = null;
	hscaleWasChanged? = false;
	exportMeshFormat? = MeshFormat.FormatObj;
	exportMeshIndex? = 0;
	packAssetsOnExport? = true;

	paintVec? = vec4_create();
	lastPaintX? = -1.0;
	lastPaintY? = -1.0;
	foregroundEvent? = false;
	painted? = 0;
	brushTime? = 0.0;
	cloneStartX? = -1.0;
	cloneStartY? = -1.0;
	cloneDeltaX? = 0.0;
	cloneDeltaY? = 0.0;

	showCompass? = true;
	projectType? = ProjectModel.ModelRoundedCube;
	projectAspectRatio? = 0; // 1:1, 2:1, 1:2
	projectObjects?: mesh_object_t[];

	lastPaintVecX? = -1.0;
	lastPaintVecY? = -1.0;
	prevPaintVecX? = -1.0;
	prevPaintVecY? = -1.0;
	frame? = 0;
	paint2dView? = false;

	lockStartedX? = -1.0;
	lockStartedY? = -1.0;
	brushLocked? = false;
	brushCanLock? = false;
	brushCanUnlock? = false;
	cameraType? = CameraType.CameraPerspective;
	camHandle? = zui_handle_create();
	fovHandle?: zui_handle_t = null;
	undoHandle?: zui_handle_t = null;
	hssao?: zui_handle_t = null;
	hssr?: zui_handle_t = null;
	hbloom?: zui_handle_t = null;
	hsupersample?: zui_handle_t = null;
	hvxao?: zui_handle_t = null;
	///if is_forge
	vxaoExt? = 2.0;
	///else
	vxaoExt? = 1.0;
	///end
	vxaoOffset? = 1.5;
	vxaoAperture? = 1.2;
	textureExportPath? = "";
	lastStatusPosition? = 0;
	cameraControls? = CameraControls.ControlsOrbit;
	penPaintingOnly? = false; // Reject painting with finger when using pen

	///if (is_paint || is_sculpt)
	material?: SlotMaterialRaw;
	layer?: SlotLayerRaw;
	brush?: SlotBrushRaw;
	font?: SlotFontRaw;
	tool? = WorkspaceTool.ToolBrush;

	layerPreviewDirty? = true;
	layersPreviewDirty? = false;
	nodePreviewDirty? = false;
	nodePreview?: image_t = null;
	nodePreviews?: Map<string, image_t> = null;
	nodePreviewsUsed?: string[] = null;
	nodePreviewName? = "";
	maskPreviewRgba32?: image_t = null;
	maskPreviewLast?: SlotLayerRaw = null;

	colorIdPicked? = false;
	materialPreview? = false; // Drawing material previews
	savedCamera? = mat4_identity();

	colorPickerPreviousTool? = WorkspaceTool.ToolBrush;
	materialIdPicked? = 0;
	uvxPicked? = 0.0;
	uvyPicked? = 0.0;
	pickerSelectMaterial? = true;
	pickerMaskHandle? = zui_handle_create();
	pickPosNorTex? = false;
	posXPicked? = 0.0;
	posYPicked? = 0.0;
	posZPicked? = 0.0;
	norXPicked? = 0.0;
	norYPicked? = 0.0;
	norZPicked? = 0.0;

	drawWireframe? = false;
	wireframeHandle? = zui_handle_create({ selected: false });
	drawTexels? = false;
	texelsHandle? = zui_handle_create({ selected: false });

	colorIdHandle? = zui_handle_create();
	layersExport? = ExportMode.ExportVisible;

	decalImage?: image_t = null;
	decalPreview? = false;
	decalX? = 0.0;
	decalY? = 0.0;

	cacheDraws? = false;
	writeIconOnExport? = false;

	textToolImage?: image_t = null;
	textToolText?: string;
	particleMaterial?: material_data_t = null;
	///if arm_physics
	particlePhysics? = false;
	particleHitX? = 0.0;
	particleHitY? = 0.0;
	particleHitZ? = 0.0;
	lastParticleHitX? = 0.0;
	lastParticleHitY? = 0.0;
	lastParticleHitZ? = 0.0;
	particleTimer?: tween_anim_t = null;
	paintBody?: PhysicsBodyRaw = null;
	///end

	layerFilter? = 0;
	runBrush?: (i: i32)=>void = null;
	parseBrushInputs?: ()=>void = null;

	gizmo?: object_t = null;
	gizmoTranslateX?: object_t = null;
	gizmoTranslateY?: object_t = null;
	gizmoTranslateZ?: object_t = null;
	gizmoScaleX?: object_t = null;
	gizmoScaleY?: object_t = null;
	gizmoScaleZ?: object_t = null;
	gizmoRotateX?: object_t = null;
	gizmoRotateY?: object_t = null;
	gizmoRotateZ?: object_t = null;
	gizmoStarted? = false;
	gizmoOffset? = 0.0;
	gizmoDrag? = 0.0;
	gizmoDragLast? = 0.0;
	translateX? = false;
	translateY? = false;
	translateZ? = false;
	scaleX? = false;
	scaleY? = false;
	scaleZ? = false;
	rotateX? = false;
	rotateY? = false;
	rotateZ? = false;

	brushNodesRadius? = 1.0;
	brushNodesOpacity? = 1.0;
	brushMaskImage?: image_t = null;
	brushMaskImageIsAlpha? = false;
	brushStencilImage?: image_t = null;
	brushStencilImageIsAlpha? = false;
	brushStencilX? = 0.02;
	brushStencilY? = 0.02;
	brushStencilScale? = 0.9;
	brushStencilScaling? = false;
	brushStencilAngle? = 0.0;
	brushStencilRotating? = false;
	brushNodesScale? = 1.0;
	brushNodesAngle? = 0.0;
	brushNodesHardness? = 1.0;
	brushDirectional? = false;

	brushRadius? = 0.5;
	brushRadiusHandle? = zui_handle_create({ value: 0.5 });
	brushScaleX? = 1.0;
	brushDecalMaskRadius? = 0.5;
	brushDecalMaskRadiusHandle? = zui_handle_create({ value: 0.5 });
	brushScaleXHandle? = zui_handle_create({ value: 1.0 });
	brushBlending? = BlendType.BlendMix;
	brushOpacity? = 1.0;
	brushOpacityHandle? = zui_handle_create({ value: 1.0 });
	brushScale? = 1.0;
	brushAngle? = 0.0;
	brushAngleHandle? = zui_handle_create({ value: 0.0 });
	///if is_paint
	brushHardness? = 0.8;
	///end
	///if is_sculpt
	brushHardness? = 0.05;
	///end
	brushLazyRadius? = 0.0;
	brushLazyStep? = 0.0;
	brushLazyX? = 0.0;
	brushLazyY? = 0.0;
	brushPaint? = UVType.UVMap;
	brushAngleRejectDot? = 0.5;
	bakeType? = BakeType.BakeAO;
	bakeAxis? = BakeAxis.BakeXYZ;
	bakeUpAxis? = BakeUpAxis.BakeUpZ;
	bakeSamples? = 128;
	bakeAoStrength? = 1.0;
	bakeAoRadius? = 1.0;
	bakeAoOffset? = 1.0;
	bakeCurvStrength? = 1.0;
	bakeCurvRadius? = 1.0;
	bakeCurvOffset? = 0.0;
	bakeCurvSmooth? = 1;
	bakeHighPoly? = 0;

	xray? = false;
	symX? = false;
	symY? = false;
	symZ? = false;
	fillTypeHandle? = zui_handle_create();

	paint2d? = false;

	lastHtab0Position? = 0;
	maximizedSidebarWidth? = 0;
	dragDestination? = 0;
	///end

	///if is_lab
	material?: any; ////
	layer?: any; ////
	tool? = WorkspaceTool.ToolEraser;

	colorPickerPreviousTool? = WorkspaceTool.ToolEraser;

	brushRadius? = 0.25;
	brushRadiusHandle? = zui_handle_create({ value: 0.25 });
	brushScale? = 1.0;

	coords? = vec4_create();
	startX? = 0.0;
	startY? = 0.0;

	// Brush ruler
	lockBegin? = false;
	lockX? = false;
	lockY? = false;
	lockStartX? = 0.0;
	lockStartY? = 0.0;
	registered? = false;
	///end

	///if is_forge
	selectedObject?: object_t = null;
	///end
}
