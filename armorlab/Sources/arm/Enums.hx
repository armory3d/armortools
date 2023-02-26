package arm;

@:enum abstract WorkspaceTool(Int) from Int to Int {
	var ToolEraser = 0;
	var ToolClone = 1;
	var ToolBlur = 2;
	var ToolPicker = 3;

	var ToolDecal = 4; // Unused
	var ToolText = 5;
}

@:enum abstract SpaceType(Int) from Int to Int {
	var Space3D = 0;
	var Space2D = 1;
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
