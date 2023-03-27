package arm;

@:enum abstract WorkspaceTool(Int) from Int to Int {
	var ToolEraser = 0;
	var ToolClone = 1;
	var ToolBlur = 2;
	var ToolSmudge = 3;
	var ToolPicker = 4;

	var ToolDecal = 5; // Unused
	var ToolText = 6;
}

@:enum abstract SpaceType(Int) from Int to Int {
	var Space3D = 0;
	var Space2D = 1;
}

@:enum abstract AreaType(Int) from Int to Int {
	var AreaViewport = 0;
	var AreaNodes = 1;
	var AreaBrowser = 2;
}

@:enum abstract TabArea(Int) from Int to Int {
	var TabStatus = 0;
}

@:enum abstract TextureRes(Int) from Int to Int {
	var Res2048 = 0;
	var Res4096 = 1;
	var Res8192 = 2;
	var Res16384 = 3;
	var Res128 = 4; // Unused
	var Res256 = 5;
	var Res512 = 6;
	var Res1024 = 7;
}

@:enum abstract LayoutSize(Int) from Int to Int {
	var LayoutNodesW = 0;
	var LayoutStatusH = 1;
}
