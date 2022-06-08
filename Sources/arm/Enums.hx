package arm;

@:enum abstract WorkspaceTool(Int) from Int to Int {
	var ToolBrush = 0;
	var ToolEraser = 1;
	var ToolFill = 2;
	var ToolDecal = 3;
	var ToolText = 4;
	var ToolClone = 5;
	var ToolBlur = 6;
	var ToolParticle = 7;
	var ToolColorId = 8;
	var ToolPicker = 9;
	var ToolGizmo = 10;
	var ToolBake = 11;
}

@:enum abstract SpaceType(Int) from Int to Int {
	var SpacePaint = 0;
	// var SpaceSculpt = 0;
	var SpaceMaterial = 1;
	var SpaceBake = 2;
}

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
	var ViewEmission = 8;
	var ViewSubsurface = 9;
	var ViewTexCoord = 10;
	var ViewObjectNormal = 11;
	var ViewMaterialID = 12;
	var ViewObjectID = 13;
	var ViewMask = 14;
	var ViewPathTrace = 15;
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

#if (kha_direct3d12 || kha_vulkan)
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

@:enum abstract TextureRes(Int) from Int to Int {
	var Res128 = 0;
	var Res256 = 1;
	var Res512 = 2;
	var Res1024 = 3;
	var Res2048 = 4;
	var Res4096 = 5;
	var Res8192 = 6;
	var Res16384 = 7;
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
	var CanvasBrush = 1;
}

@:enum abstract View2DType(Int) from Int to Int {
	var View2DLayer = 0;
	var View2DAsset = 1;
	var View2DFont = 2;
	var View2DNode = 3;
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

@:enum abstract LayoutSize(Int) from Int to Int {
	var LayoutSidebarW = 0;
	var LayoutSidebarH0 = 1;
	var LayoutSidebarH1 = 2;
	var LayoutNodesW = 3;
	var LayoutNodesH = 4;
	var LayoutStatusH = 5;
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
