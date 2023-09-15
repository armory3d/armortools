package arm;

@:enum abstract DilateType(Int) from Int to Int {
	var DilateInstant = 0;
	var DilateDelayed = 1;
}

@:enum abstract BakeType(Int) from Int to Int {
	var BakeInit = -1;
	var BakeAO = 0;
	var BakeCurvature = 1;
	var BakeNormal = 2;
	var BakeNormalObject = 3;
	var BakeHeight = 4;
	var BakeDerivative = 5;
	var BakePosition = 6;
	var BakeTexCoord = 7;
	var BakeMaterialID = 8;
	var BakeObjectID = 9;
	var BakeVertexColor = 10;
	var BakeLightmap = 11;
	var BakeBentNormal = 12;
	var BakeThickness = 13;
}

@:enum abstract SplitType(Int) from Int to Int {
	var SplitObject = 0;
	var SplitGroup = 1;
	var SplitMaterial = 2;
	var SplitUdim = 3;
}

@:enum abstract BakeAxis(Int) from Int to Int {
	var BakeXYZ = 0;
	var BakeX = 1;
	var BakeY = 2;
	var BakeZ = 3;
	var BakeMX = 4;
	var BakeMY = 5;
	var BakeMZ = 6;
}

@:enum abstract BakeUpAxis(Int) from Int to Int {
	var BakeUpZ = 0;
	var BakeUpY = 1;
	var BakeUpX = 2;
}

@:enum abstract ViewportMode(Int) from Int to Int {
	var ViewLit = 0;
	var ViewBaseColor = 1;
	var ViewNormalMap = 2;
	var ViewOcclusion = 3;
	var ViewRoughness = 4;
	var ViewMetallic = 5;
	var ViewOpacity = 6;
	var ViewHeight = 7;
	#if (is_paint || is_sculpt)
	var ViewEmission = 8;
	var ViewSubsurface = 9;
	var ViewTexCoord = 10;
	var ViewObjectNormal = 11;
	var ViewMaterialID = 12;
	var ViewObjectID = 13;
	var ViewMask = 14;
	var ViewPathTrace = 15;
	#else
	var ViewPathTrace = 8;
	#end
}

@:enum abstract ChannelType(Int) from Int to Int {
	var ChannelBaseColor = 0;
	var ChannelOcclusion = 1;
	var ChannelRoughness = 2;
	var ChannelMetallic = 3;
	var ChannelNormalMap = 4;
	var ChannelHeight = 5;
}

@:enum abstract RenderMode(Int) from Int to Int {
	var RenderDeferred = 0;
	var RenderForward = 1;
	var RenderPathTrace = 2;
}

@:enum abstract ExportMode(Int) from Int to Int {
	var ExportVisible = 0;
	var ExportSelected = 1;
	var ExportPerObject = 2;
	var ExportPerUdimTile = 3;
}

@:enum abstract ExportDestination(Int) from Int to Int {
	var DestinationDisk = 0;
	var DestinationPacked = 1;
}

#if (kha_direct3d12 || kha_vulkan || kha_metal)
@:enum abstract PathTraceMode(Int) from Int to Int {
	var TraceCore = 0;
	var TraceFull = 1;
}
#end

@:enum abstract FillType(Int) from Int to Int {
	var FillObject = 0;
	var FillFace = 1;
	var FillAngle = 2;
	var FillUVIsland = 3;
}

@:enum abstract UVType(Int) from Int to Int {
	var UVMap = 0;
	var UVTriplanar = 1;
	var UVProject = 2;
}

@:enum abstract PickerMask(Int) from Int to Int {
	var MaskNone = 0;
	var MaskMaterial = 1;
}

@:enum abstract BlendType(Int) from Int to Int {
	var BlendMix = 0;
	var BlendDarken = 1;
	var BlendMultiply = 2;
	var BlendBurn = 3;
	var BlendLighten = 4;
	var BlendScreen = 5;
	var BlendDodge = 6;
	var BlendAdd = 7;
	var BlendOverlay = 8;
	var BlendSoftLight = 9;
	var BlendLinearLight = 10;
	var BlendDifference = 11;
	var BlendSubtract = 12;
	var BlendDivide = 13;
	var BlendHue = 14;
	var BlendSaturation = 15;
	var BlendColor = 16;
	var BlendValue = 17;
}

@:enum abstract CameraControls(Int) from Int to Int {
	var ControlsOrbit = 0;
	var ControlsRotate = 1;
	var ControlsFly = 2;
}

@:enum abstract CameraType(Int) from Int to Int {
	var CameraPerspective = 0;
	var CameraOrthographic = 1;
}

@:enum abstract TextureBits(Int) from Int to Int {
	var Bits8 = 0;
	var Bits16 = 1;
	var Bits32 = 2;
}

@:enum abstract TextureLdrFormat(Int) from Int to Int {
	var FormatPng = 0;
	var FormatJpg = 1;
}

@:enum abstract TextureHdrFormat(Int) from Int to Int {
	var FormatExr = 0;
}

@:enum abstract MeshFormat(Int) from Int to Int {
	var FormatObj = 0;
	var FormatArm = 1;
}

@:enum abstract MenuCategory(Int) from Int to Int {
	var MenuFile = 0;
	var MenuEdit = 1;
	var MenuViewport = 2;
	var MenuMode = 3;
	var MenuCamera = 4;
	var MenuHelp = 5;
}

@:enum abstract CanvasType(Int) from Int to Int {
	var CanvasMaterial = 0;
	#if (is_paint || is_sculpt)
	var CanvasBrush = 1;
	#end
}

@:enum abstract View2DType(Int) from Int to Int {
	var View2DAsset = 0;
	var View2DNode = 1;
	#if (is_paint || is_sculpt)
	var View2DFont = 2;
	var View2DLayer = 3;
	#end
}

@:enum abstract View2DLayerMode(Int) from Int to Int {
	var View2DVisible = 0;
	var View2DSelected = 1;
}

@:enum abstract BorderSide(Int) from Int to Int {
	var SideLeft = 0;
	var SideRight = 1;
	var SideTop = 2;
	var SideBottom = 3;
}

@:enum abstract PaintTex(Int) from Int to Int {
	var TexBase = 0;
	var TexNormal = 1;
	var TexOcclusion = 2;
	var TexRoughness = 3;
	var TexMetallic = 4;
	var TexOpacity = 5;
	var TexHeight = 6;
}

@:enum abstract ProjectModel(Int) from Int to Int {
	var ModelRoundedCube = 0;
	var ModelSphere = 1;
	var ModelTessellatedPlane = 2;
	var ModelCustom = 3;
}

@:enum abstract ZoomDirection(Int) from Int to Int {
	var ZoomVertical = 0;
	var ZoomVerticalInverted = 1;
	var ZoomHorizontal = 2;
	var ZoomHorizontalInverted = 3;
	var ZoomVerticalAndHorizontal = 4;
	var ZoomVerticalAndHorizontalInverted = 5;
}

@:enum abstract LayerSlotType(Int) from Int to Int {
	var SlotLayer = 0;
	var SlotMask = 1;
	var SlotGroup = 2;
}

@:enum abstract SpaceType(Int) from Int to Int {
	var Space3D = 0;
	#if is_lab
	var Space2D = 1;
	#end
}

@:enum abstract WorkspaceTool(Int) from Int to Int {
	#if is_paint
	var ToolBrush = 0;
	var ToolEraser = 1;
	var ToolFill = 2;
	var ToolDecal = 3;
	var ToolText = 4;
	var ToolClone = 5;
	var ToolBlur = 6;
	var ToolSmudge = 7;
	var ToolParticle = 8;
	var ToolColorId = 9;
	var ToolPicker = 10;
	var ToolBake = 11;
	var ToolGizmo = 12;
	var ToolMaterial = 13;
	#end
	#if is_sculpt
	var ToolBrush = 0;
	var ToolEraser = 1;
	var ToolFill = 2;
	var ToolDecal = 3;
	var ToolText = 4;
	var ToolClone = 5;
	var ToolBlur = 6;
	var ToolSmudge = 7;
	var ToolParticle = 8;
	var ToolPicker = 9;
	var ToolGizmo = 10;
	var ToolMaterial = 11;
	var ToolBake = 12; // Unused
	#end
	#if is_lab
	var ToolEraser = 0;
	var ToolClone = 1;
	var ToolBlur = 2;
	var ToolSmudge = 3;
	var ToolPicker = 4;
	var ToolDecal = 5; // Unused
	var ToolText = 6;
	#end
}

@:enum abstract AreaType(Int) from Int to Int {
	#if (is_paint || is_sculpt)
	var AreaViewport = 0;
	var Area2DView = 1;
	var AreaLayers = 2;
	var AreaMaterials = 3;
	var AreaNodes = 4;
	var AreaBrowser = 5;
	#end
	#if is_lab
	var AreaViewport = 0;
	var AreaNodes = 1;
	var AreaBrowser = 2;
	#end
}

@:enum abstract TabArea(Int) from Int to Int {
	#if (is_paint || is_sculpt)
	var TabSidebar0 = 0;
	var TabSidebar1 = 1;
	var TabStatus = 2;
	#end
	#if is_lab
	var TabStatus = 0;
	#end
}

@:enum abstract TextureRes(Int) from Int to Int {
	#if (is_paint || is_sculpt)
	var Res128 = 0;
	var Res256 = 1;
	var Res512 = 2;
	var Res1024 = 3;
	var Res2048 = 4;
	var Res4096 = 5;
	var Res8192 = 6;
	var Res16384 = 7;
	#end
	#if is_lab
	var Res2048 = 0;
	var Res4096 = 1;
	var Res8192 = 2;
	var Res16384 = 3;
	var Res128 = 4; // Unused
	var Res256 = 5;
	var Res512 = 6;
	var Res1024 = 7;
	#end
}

@:enum abstract LayoutSize(Int) from Int to Int {
	#if (is_paint || is_sculpt)
	var LayoutSidebarW = 0;
	var LayoutSidebarH0 = 1;
	var LayoutSidebarH1 = 2;
	var LayoutNodesW = 3;
	var LayoutNodesH = 4;
	var LayoutStatusH = 5;
	var LayoutHeader = 6; // 0 - hidden, 1 - visible
	#end
	#if is_lab
	var LayoutNodesW = 0;
	var LayoutNodesH = 1;
	var LayoutStatusH = 2;
	var LayoutHeader = 3;
	#end
}
