
enum DilateType {
	DilateInstant,
	DilateDelayed,
}

enum BakeType {
	BakeInit = -1,
	BakeAO = 0,
	BakeCurvature = 1,
	BakeNormal = 2,
	BakeNormalObject = 3,
	BakeHeight = 4,
	BakeDerivative = 5,
	BakePosition = 6,
	BakeTexCoord = 7,
	BakeMaterialID = 8,
	BakeObjectID = 9,
	BakeVertexColor = 10,
	BakeLightmap = 11,
	BakeBentNormal = 12,
	BakeThickness = 13,
}

enum SplitType {
	SplitObject = 0,
	SplitGroup = 1,
	SplitMaterial = 2,
	SplitUdim = 3,
}

enum BakeAxis {
	BakeXYZ = 0,
	BakeX = 1,
	BakeY = 2,
	BakeZ = 3,
	BakeMX = 4,
	BakeMY = 5,
	BakeMZ = 6,
}

enum BakeUpAxis {
	BakeUpZ = 0,
	BakeUpY = 1,
	BakeUpX = 2,
}

enum ViewportMode {
	ViewLit = 0,
	ViewBaseColor = 1,
	ViewNormalMap = 2,
	ViewOcclusion = 3,
	ViewRoughness = 4,
	ViewMetallic = 5,
	ViewOpacity = 6,
	ViewHeight = 7,
	ViewPathTrace = 8,
	ViewEmission = 9,
	ViewSubsurface = 10,
	ViewTexCoord = 11,
	ViewObjectNormal = 12,
	ViewMaterialID = 13,
	ViewObjectID = 14,
	ViewMask = 15,
}

enum ChannelType {
	ChannelBaseColor = 0,
	ChannelOcclusion = 1,
	ChannelRoughness = 2,
	ChannelMetallic = 3,
	ChannelNormalMap = 4,
	ChannelHeight = 5,
}

enum RenderMode {
	RenderDeferred = 0,
	RenderForward = 1,
	RenderPathTrace = 2,
}

enum ExportMode {
	ExportVisible = 0,
	ExportSelected = 1,
	ExportPerObject = 2,
	ExportPerUdimTile = 3,
}

enum ExportDestination {
	DestinationDisk = 0,
	DestinationPacked = 1,
}

enum PathTraceMode {
	TraceCore = 0,
	TraceFull = 1,
}

enum FillType {
	FillObject = 0,
	FillFace = 1,
	FillAngle = 2,
	FillUVIsland = 3,
}

enum UVType {
	UVMap = 0,
	UVTriplanar = 1,
	UVProject = 2,
}

enum PickerMask {
	MaskNone = 0,
	MaskMaterial = 1,
}

enum BlendType {
	BlendMix = 0,
	BlendDarken = 1,
	BlendMultiply = 2,
	BlendBurn = 3,
	BlendLighten = 4,
	BlendScreen = 5,
	BlendDodge = 6,
	BlendAdd = 7,
	BlendOverlay = 8,
	BlendSoftLight = 9,
	BlendLinearLight = 10,
	BlendDifference = 11,
	BlendSubtract = 12,
	BlendDivide = 13,
	BlendHue = 14,
	BlendSaturation = 15,
	BlendColor = 16,
	BlendValue = 17,
}

enum CameraControls {
	ControlsOrbit = 0,
	ControlsRotate = 1,
	ControlsFly = 2,
}

enum CameraType {
	CameraPerspective = 0,
	CameraOrthographic = 1,
}

enum TextureBits {
	Bits8 = 0,
	Bits16 = 1,
	Bits32 = 2,
}

enum TextureLdrFormat {
	FormatPng = 0,
	FormatJpg = 1,
}

enum TextureHdrFormat {
	FormatExr = 0,
}

enum MeshFormat {
	FormatObj = 0,
	FormatArm = 1,
}

enum MenuCategory {
	MenuFile = 0,
	MenuEdit = 1,
	MenuViewport = 2,
	MenuMode = 3,
	MenuCamera = 4,
	MenuHelp = 5,
}

enum CanvasType {
	CanvasMaterial = 0,
	CanvasBrush = 1,
}

enum View2DType {
	View2DAsset = 0,
	View2DNode = 1,
	View2DFont = 2,
	View2DLayer = 3,
}

enum View2DLayerMode {
	View2DVisible = 0,
	View2DSelected = 1,
}

enum BorderSide {
	SideLeft = 0,
	SideRight = 1,
	SideTop = 2,
	SideBottom = 3,
}

enum PaintTex {
	TexBase = 0,
	TexNormal = 1,
	TexOcclusion = 2,
	TexRoughness = 3,
	TexMetallic = 4,
	TexOpacity = 5,
	TexHeight = 6,
}

enum ProjectModel {
	ModelRoundedCube = 0,
	ModelSphere = 1,
	ModelTessellatedPlane = 2,
	ModelCustom = 3,
}

enum ZoomDirection {
	ZoomVertical = 0,
	ZoomVerticalInverted = 1,
	ZoomHorizontal = 2,
	ZoomHorizontalInverted = 3,
	ZoomVerticalAndHorizontal = 4,
	ZoomVerticalAndHorizontalInverted = 5,
}

enum LayerSlotType {
	SlotLayer = 0,
	SlotMask = 1,
	SlotGroup = 2,
}

enum SpaceType {
	Space3D = 0,
	Space2D = 1,
}

enum WorkspaceTool {
	ToolBrush = 0,
	ToolEraser = 1,
	ToolFill = 2,
	ToolDecal = 3,
	ToolText = 4,
	ToolClone = 5,
	ToolBlur = 6,
	ToolSmudge = 7,
	ToolParticle = 8,
	ToolColorId = 9,
	ToolPicker = 10,
	ToolBake = 11,
	ToolGizmo = 12,
	ToolMaterial = 13,
}

enum AreaType {
	AreaViewport = 0,
	Area2DView = 1,
	AreaLayers = 2,
	AreaMaterials = 3,
	AreaNodes = 4,
	AreaBrowser = 5,
}

enum TabArea {
	TabSidebar0 = 0,
	TabSidebar1 = 1,
	TabStatus = 2,
}

enum TextureRes {
	Res128 = 0,
	Res256 = 1,
	Res512 = 2,
	Res1024 = 3,
	Res2048 = 4,
	Res4096 = 5,
	Res8192 = 6,
	Res16384 = 7,
}

enum LayoutSize {
	LayoutSidebarW = 0,
	LayoutSidebarH0 = 1,
	LayoutSidebarH1 = 2,
	LayoutNodesW = 3,
	LayoutNodesH = 4,
	LayoutStatusH = 5,
	LayoutHeader = 6, // 0 - hidden, 1 - visible
}
