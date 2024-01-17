
type TConfig = {
	// The locale should be specified in ISO 639-1 format: https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
	// "system" is a special case that will use the system locale
	locale?: string;
	// Window
	window_mode?: Null<i32>; // window, fullscreen
	window_w?: Null<i32>;
	window_h?: Null<i32>;
	window_x?: Null<i32>;
	window_y?: Null<i32>;
	window_resizable?: Null<bool>;
	window_maximizable?: Null<bool>;
	window_minimizable?: Null<bool>;
	window_vsync?: Null<bool>;
	window_frequency?: Null<i32>;
	window_scale?: Null<f32>;
	// Render path
	rp_supersample?: Null<f32>;
	rp_ssao?: Null<bool>;
	rp_ssr?: Null<bool>;
	rp_bloom?: Null<bool>;
	rp_motionblur?: Null<bool>;
	rp_gi?: Null<bool>;
	rp_vignette?: Null<f32>;
	rp_grain?: Null<f32>;
	// Application
	version?: string;
	sha?: string; // Commit id
	recent_projects?: string[]; // Recently opened projects
	bookmarks?: string[]; // Bookmarked folders in browser
	plugins?: string[]; // List of enabled plugins
	keymap?: string; // Link to keymap file
	theme?: string; // Link to theme file
	undo_steps?: Null<i32>; // Number of undo steps to preserve
	camera_pan_speed?: Null<f32>;
	camera_zoom_speed?: Null<f32>;
	camera_rotation_speed?: Null<f32>;
	zoom_direction?: Null<i32>;
	wrap_mouse?: Null<bool>;
	show_asset_names?: Null<bool>;
	touch_ui?: Null<bool>;
	splash_screen?: Null<bool>;
	layout?: i32[]; // Sizes
	layout_tabs?: i32[]; // Active tabs
	workspace?: Null<i32>;
	camera_controls?: Null<i32>; // Orbit, rotate
	server?: string;

	pressure_radius?: Null<bool>; // Pen pressure controls
	pressure_sensitivity?: Null<f32>;
	displace_strength?: Null<f32>;
	layer_res?: Null<i32>;
	brush_live?: Null<bool>;
	brush_3d?: Null<bool>;
	node_preview?: Null<bool>;

	pressure_hardness?: Null<bool>;
	pressure_angle?: Null<bool>;
	pressure_opacity?: Null<bool>;
	material_live?: Null<bool>;
	brush_depth_reject?: Null<bool>;
	brush_angle_reject?: Null<bool>;

	dilate?: Null<i32>;
	dilate_radius?: Null<i32>;

	gpu_inference?: Null<bool>;
}
