package arm;

@:structInit class TConfig {
	// The locale should be specified in ISO 639-1 format: https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
	// "system" is a special case that will use the system locale
	@:optional public var locale: String;
	// Window
	@:optional public var window_mode: Null<Int>; // window, fullscreen
	@:optional public var window_w: Null<Int>;
	@:optional public var window_h: Null<Int>;
	@:optional public var window_x: Null<Int>;
	@:optional public var window_y: Null<Int>;
	@:optional public var window_resizable: Null<Bool>;
	@:optional public var window_maximizable: Null<Bool>;
	@:optional public var window_minimizable: Null<Bool>;
	@:optional public var window_vsync: Null<Bool>;
	@:optional public var window_frequency: Null<Int>;
	@:optional public var window_scale: Null<Float>;
	// Render path
	@:optional public var rp_supersample: Null<Float>;
	@:optional public var rp_ssao: Null<Bool>;
	@:optional public var rp_ssr: Null<Bool>;
	@:optional public var rp_bloom: Null<Bool>;
	@:optional public var rp_motionblur: Null<Bool>;
	@:optional public var rp_gi: Null<Bool>;
	@:optional public var rp_vignette: Null<Float>;
	@:optional public var rp_grain: Null<Float>;
	// Application
	@:optional public var version: String;
	@:optional public var sha: String; // Commit id
	@:optional public var recent_projects: Array<String>; // Recently opened projects
	@:optional public var bookmarks: Array<String>; // Bookmarked folders in browser
	@:optional public var plugins: Array<String>; // List of enabled plugins
	@:optional public var keymap: String; // Link to keymap file
	@:optional public var theme: String; // Link to theme file
	@:optional public var undo_steps: Null<Int>; // Number of undo steps to preserve
	@:optional public var camera_pan_speed: Null<Float>;
	@:optional public var camera_zoom_speed: Null<Float>;
	@:optional public var camera_rotation_speed: Null<Float>;
	@:optional public var zoom_direction: Null<Int>;
	@:optional public var wrap_mouse: Null<Bool>;
	@:optional public var show_asset_names: Null<Bool>;
	@:optional public var touch_ui: Null<Bool>;
	@:optional public var splash_screen: Null<Bool>;
	@:optional public var layout: Array<Int>; // Sizes
	@:optional public var layout_tabs: Array<Int>; // Active tabs
	@:optional public var workspace: Null<Int>;
	@:optional public var camera_controls: Null<Int>; // Orbit, rotate
	@:optional public var server: String;

	@:optional public var pressure_radius: Null<Bool>; // Pen pressure controls
	@:optional public var pressure_sensitivity: Null<Float>;
	@:optional public var displace_strength: Null<Float>;
	@:optional public var layer_res: Null<Int>;
	@:optional public var brush_live: Null<Bool>;
	@:optional public var brush_3d: Null<Bool>;

	#if (is_paint || is_sculpt)
	@:optional public var pressure_hardness: Null<Bool>;
	@:optional public var pressure_angle: Null<Bool>;
	@:optional public var pressure_opacity: Null<Bool>;
	@:optional public var material_live: Null<Bool>;
	@:optional public var brush_depth_reject: Null<Bool>;
	@:optional public var brush_angle_reject: Null<Bool>;
	@:optional public var node_preview: Null<Bool>;
	#end

	#if is_paint
	@:optional public var dilate: Null<Int>;
	@:optional public var dilate_radius: Null<Int>;
	#end

	#if is_lab
	@:optional public var gpu_inference: Null<Bool>;
	#end
}
