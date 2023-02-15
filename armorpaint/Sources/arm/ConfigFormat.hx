package arm;

import arm.ConfigBaseFormat;

@:structInit class TConfig extends TConfigBase {
	@:optional public var pressure_radius: Null<Bool>; // Pen pressure controls
	@:optional public var pressure_hardness: Null<Bool>;
	@:optional public var pressure_angle: Null<Bool>;
	@:optional public var pressure_opacity: Null<Bool>;
	@:optional public var pressure_sensitivity: Null<Float>;
	@:optional public var material_live: Null<Bool>;
	@:optional public var brush_live: Null<Bool>;
	@:optional public var brush_3d: Null<Bool>;
	@:optional public var brush_depth_reject: Null<Bool>;
	@:optional public var brush_angle_reject: Null<Bool>;
	@:optional public var node_preview: Null<Bool>;
	@:optional public var displace_strength: Null<Float>;
	@:optional public var layer_res: Null<Int>;
	@:optional public var dilate: Null<Int>;
	@:optional public var dilate_radius: Null<Int>;
}
