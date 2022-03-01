package arm;

typedef TConfig = {
	// The locale should be specified in ISO 639-1 format: https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
	// "system" is a special case that will use the system locale
	@:optional var locale: String;
	// Window
	@:optional var window_mode: Null<Int>; // window, fullscreen
	@:optional var window_w: Null<Int>;
	@:optional var window_h: Null<Int>;
	@:optional var window_x: Null<Int>;
	@:optional var window_y: Null<Int>;
	@:optional var window_resizable: Null<Bool>;
	@:optional var window_maximizable: Null<Bool>;
	@:optional var window_minimizable: Null<Bool>;
	@:optional var window_vsync: Null<Bool>;
	@:optional var window_frequency: Null<Int>;
	@:optional var window_scale: Null<Float>;
	// Render path
	@:optional var rp_supersample: Null<Float>;
	@:optional var rp_ssgi: Null<Bool>;
	@:optional var rp_ssr: Null<Bool>;
	@:optional var rp_bloom: Null<Bool>;
	@:optional var rp_motionblur: Null<Bool>;
	@:optional var rp_gi: Null<Bool>;
	@:optional var rp_vignette: Null<Float>;
	// Application
	@:optional var version: String; // ArmorPaint version
	@:optional var sha: String; // Commit id
	@:optional var recent_projects: Array<String>; // Recently opened projects
	@:optional var bookmarks: Array<String>; // Bookmarked folders in browser
	@:optional var plugins: Array<String>; // List of enabled plugins
	@:optional var keymap: String; // Link to keymap file
	@:optional var theme: String; // Link to theme file
	@:optional var undo_steps: Null<Int>; // Number of undo steps to preserve
	@:optional var pressure_radius: Null<Bool>; // Pen pressure controls
	@:optional var pressure_hardness: Null<Bool>;
	@:optional var pressure_angle: Null<Bool>;
	@:optional var pressure_opacity: Null<Bool>;
	@:optional var pressure_sensitivity: Null<Float>;
	@:optional var material_live: Null<Bool>;
	@:optional var brush_live: Null<Bool>;
	@:optional var brush_3d: Null<Bool>;
	@:optional var brush_depth_reject: Null<Bool>;
	@:optional var brush_angle_reject: Null<Bool>;
	@:optional var wrap_mouse: Null<Bool>;
	@:optional var node_preview: Null<Bool>;
	@:optional var camera_pan_speed: Null<Float>;
	@:optional var camera_zoom_speed: Null<Float>;
	@:optional var camera_rotation_speed: Null<Float>;
	@:optional var zoom_direction: Null<Int>;
	@:optional var displace_strength: Null<Float>;
	@:optional var show_asset_names: Null<Bool>;
	@:optional var layout: Array<Int>;
	@:optional var workspace: Null<Int>;
	@:optional var layer_res: Null<Int>;
	@:optional var dilate: Null<Int>;
	@:optional var dilate_radius: Null<Int>;
	@:optional var server: String;
}
