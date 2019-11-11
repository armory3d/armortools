package arm;

@:enum abstract PaintTool(Int) from Int to Int {
	var ToolBrush = 0;
	var ToolEraser = 1;
	var ToolFill = 2;
	var ToolDecal = 3;
	var ToolText = 4;
	var ToolClone = 5;
	var ToolBlur = 6;
	var ToolParticle = 7;
	var ToolBake = 8;
	var ToolColorId = 9;
	var ToolPicker = 10;
}

@:enum abstract SceneTool(Int) from Int to Int {
	var ToolGizmo = 0;
}

@:enum abstract Workspace(Int) from Int to Int {
	var SpacePaint = 0;
	// var SpaceSculpt = 1;
	var SpaceScene = 1;
}
