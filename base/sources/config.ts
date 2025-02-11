
let config_raw: config_t = null;
let config_keymap: map_t<string, string>;
let config_loaded: bool = false;
let config_button_align: ui_align_t = ui_align_t.LEFT;
let config_default_button_spacing: string = "       ";
let config_button_spacing: string = config_default_button_spacing;

function config_load() {
	let path: string = "";
	if (path_is_protected()) {
		path += iron_save_path();
	}
	path += "config.json";
	let blob: buffer_t = data_get_blob(path);

	///if arm_linux
	if (blob == null) { // Protected directory
		blob = data_get_blob(iron_save_path() + "config.json");
	}
	///end

	if (blob != null) {
		config_loaded = true;
		config_raw = json_parse(sys_buffer_to_string(blob));
	}
}

function config_save() {
	// Use system application data folder
	// when running from protected path like "Program Files"
	let path: string = "";
	if (path_is_protected()) {
		path += iron_save_path();
	}
	else {
		path += path_data();
		path += path_sep;
	}
	path += "config.json";

	json_encode_begin();
	json_encode_string("locale", config_raw.locale);
	json_encode_i32("window_mode", config_raw.window_mode);
	json_encode_i32("window_w", config_raw.window_w);
	json_encode_i32("window_h", config_raw.window_h);
	json_encode_i32("window_x", config_raw.window_x);
	json_encode_i32("window_y", config_raw.window_y);
	json_encode_bool("window_resizable", config_raw.window_resizable);
	json_encode_bool("window_maximizable", config_raw.window_maximizable);
	json_encode_bool("window_minimizable", config_raw.window_minimizable);
	json_encode_bool("window_vsync", config_raw.window_vsync);
	json_encode_i32("window_frequency", config_raw.window_frequency);
	json_encode_f32("window_scale", config_raw.window_scale);
	json_encode_f32("rp_supersample", config_raw.rp_supersample);
	json_encode_bool("rp_ssao", config_raw.rp_ssao);
	json_encode_bool("rp_bloom", config_raw.rp_bloom);
	json_encode_bool("rp_motionblur", config_raw.rp_motionblur);
	json_encode_bool("rp_gi", config_raw.rp_gi);
	json_encode_f32("rp_vignette", config_raw.rp_vignette);
	json_encode_f32("rp_grain", config_raw.rp_grain);
	json_encode_string("version", config_raw.version);
	json_encode_string("sha", config_raw.sha);
	json_encode_string_array("recent_projects", config_raw.recent_projects);
	json_encode_string_array("bookmarks", config_raw.bookmarks);
	json_encode_string_array("plugins", config_raw.plugins);
	json_encode_string("keymap", config_raw.keymap);
	json_encode_string("theme", config_raw.theme);
	json_encode_i32("undo_steps", config_raw.undo_steps);
	json_encode_f32("camera_pan_speed", config_raw.camera_pan_speed);
	json_encode_f32("camera_zoom_speed", config_raw.camera_zoom_speed);
	json_encode_f32("camera_rotation_speed", config_raw.camera_rotation_speed);
	json_encode_i32("zoom_direction", config_raw.zoom_direction);
	json_encode_bool("wrap_mouse", config_raw.wrap_mouse);
	json_encode_bool("show_asset_names", config_raw.show_asset_names);
	json_encode_bool("touch_ui", config_raw.touch_ui);
	json_encode_bool("splash_screen", config_raw.splash_screen);
	json_encode_i32_array("layout", config_raw.layout);
	json_encode_i32_array("layout_tabs", config_raw.layout_tabs);
	json_encode_i32("workspace", config_raw.workspace);
	json_encode_i32("camera_controls", config_raw.camera_controls);
	json_encode_string("server", config_raw.server);
	json_encode_i32("viewport_mode", config_raw.viewport_mode);
	json_encode_i32("pathtrace_mode", config_raw.pathtrace_mode);
	json_encode_bool("pressure_radius", config_raw.pressure_radius);
	json_encode_f32("pressure_sensitivity", config_raw.pressure_sensitivity);
	json_encode_f32("displace_strength", config_raw.displace_strength);
	json_encode_i32("layer_res", config_raw.layer_res);
	json_encode_bool("brush_live", config_raw.brush_live);
	json_encode_bool("brush_3d", config_raw.brush_3d);
	json_encode_bool("node_preview", config_raw.node_preview);
	json_encode_bool("pressure_hardness", config_raw.pressure_hardness);
	json_encode_bool("pressure_angle", config_raw.pressure_angle);
	json_encode_bool("pressure_opacity", config_raw.pressure_opacity);
	json_encode_bool("material_live", config_raw.material_live);
	json_encode_bool("brush_depth_reject", config_raw.brush_depth_reject);
	json_encode_bool("brush_angle_reject", config_raw.brush_angle_reject);
	json_encode_i32("dilate", config_raw.dilate);
	json_encode_i32("dilate_radius", config_raw.dilate_radius);
	json_encode_bool("gpu_inference", config_raw.gpu_inference);
	json_encode_string("blender", config_raw.blender);
	json_encode_i32("atlas_res", config_raw.atlas_res);
	json_encode_bool("grid_snap", config_raw.grid_snap);
	let config_json: string = json_encode_end();

	let buffer: buffer_t = sys_string_to_buffer(config_json);
	iron_file_save_bytes(path, buffer, 0);

	///if arm_linux // Protected directory
	if (!file_exists(path)) {
		iron_file_save_bytes(iron_save_path() + "config.json", buffer, 0);
	}
	///end
}

function config_init() {
	if (!config_loaded || config_raw == null) {
		config_raw = {};
		config_raw.locale = "system";
		config_raw.window_mode = 0;
		config_raw.window_resizable = true;
		config_raw.window_minimizable = true;
		config_raw.window_maximizable = true;
		config_raw.window_w = 1600;
		config_raw.window_h = 900;
		///if arm_macos
		config_raw.window_w *= 2;
		config_raw.window_h *= 2;
		///end
		config_raw.window_x = -1;
		config_raw.window_y = -1;
		config_raw.window_scale = 1.0;
		if (sys_display_width() >= 2560 && sys_display_height() >= 1600) {
			config_raw.window_scale = 2.0;
		}
		///if (arm_android || arm_ios || arm_macos)
		config_raw.window_scale = 2.0;
		///end
		config_raw.window_vsync = true;
		config_raw.window_frequency = sys_display_frequency();
		config_raw.rp_bloom = false;
		config_raw.rp_gi = false;
		config_raw.rp_vignette = 0.2;
		config_raw.rp_grain = 0.09;
		config_raw.rp_motionblur = false;
		///if (arm_android || arm_ios || is_forge)
		config_raw.rp_ssao = false;
		///else
		config_raw.rp_ssao = true;
		///end
		config_raw.rp_supersample = 1.0;
		config_raw.version = manifest_version;
		config_raw.sha = config_get_sha();
		base_init_config();
	}
	else {
		// Upgrade config format created by older ArmorPaint build
		// if (config_raw.version != manifest_version) {
		// 	config_raw.version = manifest_version;
		// 	save();
		// }
		if (config_raw.sha != config_get_sha()) {
			config_loaded = false;
			config_init();
			return;
		}
	}

	ui_touch_scroll = config_raw.touch_ui;
	ui_touch_hold = config_raw.touch_ui;
	ui_touch_tooltip = config_raw.touch_ui;
	base_res_handle.position = config_raw.layer_res;
	keymap_load();
}

function config_get_sha(): string {
	let blob: buffer_t = data_get_blob("version.json");
	if (blob == null) {
		return "undefined";
	}
	let v: version_t = json_parse(sys_buffer_to_string(blob));
	return v.sha;
}

function config_get_date(): string {
	let blob: buffer_t = data_get_blob("version.json");
	if (blob == null) {
		return "undefined";
	}
	let v: version_t = json_parse(sys_buffer_to_string(blob));
	return v.date;
}

function config_get_options(): kinc_sys_ops_t {
	let window_mode: window_mode_t = config_raw.window_mode == 0 ? window_mode_t.WINDOWED : window_mode_t.FULLSCREEN;
	let window_features: window_features_t = window_features_t.NONE;
	if (config_raw.window_resizable) {
		window_features |= window_features_t.RESIZABLE;
	}
	if (config_raw.window_maximizable) {
		window_features |= window_features_t.MAXIMIZABLE;
	}
	if (config_raw.window_minimizable) {
		window_features |= window_features_t.MINIMIZABLE;
	}
	let title: string = "untitled - " + manifest_title;
	let ops: kinc_sys_ops_t = {
		title: title,
		width: config_raw.window_w,
		height: config_raw.window_h,
		x: config_raw.window_x,
		y: config_raw.window_y,
		mode: window_mode,
		features: window_features,
		vsync: config_raw.window_vsync,
		frequency: config_raw.window_frequency
	};
	return ops;
}

function config_restore() {
	ui_children = map_create(); // Reset ui handles
	config_loaded = false;
	let _layout: i32[] = config_raw.layout;
	config_init();
	config_raw.layout = _layout;
	base_init_layout();
	translator_load_translations(config_raw.locale);
	config_apply();
	config_load_theme(config_raw.theme);
}

function config_import_from(from: config_t) {
	let _sha: string = config_raw.sha;
	let _version: string = config_raw.version;
	config_raw = from;
	config_raw.sha = _sha;
	config_raw.version = _version;
	ui_children = map_create(); // Reset ui handles
	keymap_load();
	base_init_layout();
	translator_load_translations(config_raw.locale);
	config_apply();
	config_load_theme(config_raw.theme);
}

function config_apply() {
	config_raw.rp_ssao = context_raw.hssao.selected;
	config_raw.rp_bloom = context_raw.hbloom.selected;
	config_raw.rp_gi = context_raw.hvxao.selected;
	config_raw.rp_supersample = config_get_super_sample_size(context_raw.hsupersample.position);
	config_save();
	context_raw.ddirty = 2;

	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();
	render_path_base_apply_config();
	if (g2_in_use) g2_begin(current);
}

function config_get_super_sample_quality(f: f32): i32 {
	return f == 0.25 ? 0 :
		   f == 0.5 ? 1 :
		   f == 1.0 ? 2 :
		   f == 1.5 ? 3 :
		   f == 2.0 ? 4 : 5;
}

function config_get_super_sample_size(i: i32): f32 {
	return i == 0 ? 0.25 :
		   i == 1 ? 0.5 :
		   i == 2 ? 1.0 :
		   i == 3 ? 1.5 :
		   i == 4 ? 2.0 : 4.0;
}

function config_texture_res_size(pos: i32): i32 {
	return pos == texture_res_t.RES128 ? 128 :
		   pos == texture_res_t.RES256 ? 256 :
		   pos == texture_res_t.RES512 ? 512 :
		   pos == texture_res_t.RES1024 ? 1024 :
		   pos == texture_res_t.RES2048 ? 2048 :
		   pos == texture_res_t.RES4096 ? 4096 :
		   pos == texture_res_t.RES8192 ? 8192 :
		   pos == texture_res_t.RES16384 ? 16384 : 0;
}

function config_get_texture_res(): i32 {
	let res: i32 = base_res_handle.position;
	return config_texture_res_size(res);
}

function config_get_layer_res(): i32 {
	let res: i32 = config_raw.layer_res;
	return config_texture_res_size(res);
}

function config_get_atlas_res(): i32 {
	let res: i32 = config_raw.atlas_res;
	return config_texture_res_size(res);
}

function config_get_texture_res_x(): i32 {
	return context_raw.project_aspect_ratio == 2 ? math_floor(config_get_texture_res() / 2) : config_get_texture_res();
}

function config_get_texture_res_y(): i32 {
	return context_raw.project_aspect_ratio == 1 ? math_floor(config_get_texture_res() / 2) : config_get_texture_res();
}

function config_get_texture_res_pos(i: i32): i32 {
	return i == 128 ? texture_res_t.RES128 :
		   i == 256 ? texture_res_t.RES256 :
		   i == 512 ? texture_res_t.RES512 :
		   i == 1024 ? texture_res_t.RES1024 :
		   i == 2048 ? texture_res_t.RES2048 :
		   i == 4096 ? texture_res_t.RES4096 :
		   i == 8192 ? texture_res_t.RES8192 :
		   i == 16384 ? texture_res_t.RES16384 : 0;
}

function config_load_theme(theme: string, tag_redraw: bool = true) {
	base_theme = ui_theme_create();

	if (theme != "default.json") {
		let b: buffer_t = data_get_blob("themes/" + theme);
		let parsed: ui_theme_t = json_parse(sys_buffer_to_string(b));
		base_theme = parsed;
	}

	base_theme.FILL_WINDOW_BG = true;

	if (tag_redraw) {
		for (let i: i32 = 0; i < base_get_uis().length; ++i) {
			let ui: ui_t = base_get_uis()[i];
			ui.ops.theme = base_theme;
		}
		ui_base_tag_ui_redraw();
	}

	if (config_raw.touch_ui) {
		// Enlarge elements
		base_theme.FULL_TABS = true;
		base_theme.ELEMENT_H = 24 + 6;
		base_theme.BUTTON_H = 22 + 6;
		base_theme.FONT_SIZE = 13 + 2;
		base_theme.ARROW_SIZE = 5 + 2;
		base_theme.CHECK_SIZE = 15 + 4;
		base_theme.CHECK_SELECT_SIZE = 8 + 2;
		config_button_align = ui_align_t.LEFT;
		config_button_spacing = "";
	}
	else {
		base_theme.FULL_TABS = false;
		config_button_align = ui_align_t.LEFT;
		config_button_spacing = config_default_button_spacing;
	}
}

function config_enable_plugin(f: string) {
	array_push(config_raw.plugins, f);
	plugin_start(f);
}

function config_disable_plugin(f: string) {
	array_remove(config_raw.plugins, f);
	plugin_stop(f);
}

type version_t = {
	sha: string;
	date: string;
};

type config_t = {
	// The locale should be specified in ISO 639-1 format:
	// https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
	// "system" is a special case that will use the system locale
	locale?: string;
	// Window
	window_mode?: i32; // window, fullscreen
	window_w?: i32;
	window_h?: i32;
	window_x?: i32;
	window_y?: i32;
	window_resizable?: bool;
	window_maximizable?: bool;
	window_minimizable?: bool;
	window_vsync?: bool;
	window_frequency?: i32;
	window_scale?: f32;
	// Render path
	rp_supersample?: f32;
	rp_ssao?: bool;
	rp_bloom?: bool;
	rp_motionblur?: bool;
	rp_gi?: bool;
	rp_vignette?: f32;
	rp_grain?: f32;
	// Application
	version?: string;
	sha?: string; // Commit id
	recent_projects?: string[]; // Recently opened projects
	bookmarks?: string[]; // Bookmarked folders in browser
	plugins?: string[]; // List of enabled plugins
	keymap?: string; // Link to keymap file
	theme?: string; // Link to theme file
	undo_steps?: i32; // Number of undo steps to preserve
	camera_pan_speed?: f32;
	camera_zoom_speed?: f32;
	camera_rotation_speed?: f32;
	zoom_direction?: i32;
	wrap_mouse?: bool;
	show_asset_names?: bool;
	touch_ui?: bool;
	splash_screen?: bool;
	layout?: i32[]; // Sizes
	layout_tabs?: i32[]; // Active tabs
	workspace?: i32;
	camera_controls?: i32; // Orbit, rotate
	server?: string;
	viewport_mode: i32;
	pathtrace_mode: i32;

	pressure_radius?: bool; // Pen pressure controls
	pressure_sensitivity?: f32;
	displace_strength?: f32;
	layer_res?: i32;
	brush_live?: bool;
	brush_3d?: bool;
	node_preview?: bool;

	pressure_hardness?: bool;
	pressure_angle?: bool;
	pressure_opacity?: bool;
	material_live?: bool;
	brush_depth_reject?: bool;
	brush_angle_reject?: bool;

	dilate?: i32;
	dilate_radius?: i32;

	gpu_inference?: bool;
	blender?: string;
	atlas_res: i32; // Forge
	grid_snap: bool;
};
