package arm;

import arm.ConfigBaseFormat;

@:structInit class TConfig extends TConfigBase {
	@:optional public var pressure_radius: Null<Bool>; // Pen pressure controls
	@:optional public var pressure_sensitivity: Null<Float>;
	@:optional public var displace_strength: Null<Float>;
	@:optional public var layer_res: Null<Int>;
	@:optional public var gpu_inference: Null<Bool>;
	@:optional public var brush_live: Null<Bool>; ////
	@:optional public var brush_3d: Null<Bool>; ////
}
