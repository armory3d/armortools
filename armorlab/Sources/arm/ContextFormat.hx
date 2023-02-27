package arm;

import kha.Image;
import zui.Zui;
import iron.math.Vec4;
import iron.object.MeshObject;
import arm.shader.NodeShader;
import arm.ProjectBaseFormat;
import arm.ContextBaseFormat;

@:structInit class TContext extends TContextBase {
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
}
