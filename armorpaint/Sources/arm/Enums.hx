package arm;

@:enum abstract WorkspaceTool(Int) from Int to Int {
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
}

@:enum abstract SpaceType(Int) from Int to Int {
	var SpacePaint = 0;
	var SpaceMaterial = 1;
	var SpaceBake = 2;
}

@:enum abstract AreaType(Int) from Int to Int {
	var AreaViewport = 0;
	var Area2DView = 1;
	var AreaLayers = 2;
	var AreaMaterials = 3;
	var AreaNodes = 4;
	var AreaBrowser = 5;
}

@:enum abstract TabArea(Int) from Int to Int {
	var TabSidebar0 = 0;
	var TabSidebar1 = 1;
	var TabStatus = 2;
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

@:enum abstract LayoutSize(Int) from Int to Int {
	var LayoutSidebarW = 0;
	var LayoutSidebarH0 = 1;
	var LayoutSidebarH1 = 2;
	var LayoutNodesW = 3;
	var LayoutNodesH = 4;
	var LayoutStatusH = 5;
}
