
let config_raw: config_t = null;
let config_keymap: any;
let config_loaded: bool = false;
let config_button_align: zui_align_t = zui_align_t.LEFT;
let config_default_button_spacing: string = "       ";
let config_button_spacing: string = config_default_button_spacing;

function config_load(done: ()=>void) {
	let blob: buffer_t = data_get_blob((path_is_protected() ? krom_save_path() : "") + "config.json");

	///if krom_linux
	if (blob == null) { // Protected directory
		blob = data_get_blob(krom_save_path() + "config.json");
	}
	///end

	if (blob != null) {
		config_loaded = true;
		config_raw = json_parse(sys_buffer_to_string(blob));
	}

	done();
}

function config_save() {
	// Use system application data folder
	// when running from protected path like "Program Files"
	let path: string = (path_is_protected() ? krom_save_path() : path_data() + path_sep) + "config.json";
	let buffer: buffer_t = sys_string_to_buffer(json_stringify(config_raw));
	krom_file_save_bytes(path, buffer);

	///if krom_linux // Protected directory
	if (!file_exists(path)) {
		krom_file_save_bytes(krom_save_path() + "config.json", buffer);
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
		///if krom_darwin
		config_raw.window_w *= 2;
		config_raw.window_h *= 2;
		///end
		config_raw.window_x = -1;
		config_raw.window_y = -1;
		config_raw.window_scale = 1.0;
		if (sys_display_width() >= 2560 && sys_display_height() >= 1600) {
			config_raw.window_scale = 2.0;
		}
		///if (krom_android || krom_ios || krom_darwin)
		config_raw.window_scale = 2.0;
		///end
		config_raw.window_vsync = true;
		config_raw.window_frequency = sys_display_frequency();
		config_raw.rp_bloom = false;
		config_raw.rp_gi = false;
		config_raw.rp_vignette = 0.2;
		config_raw.rp_grain = 0.09;
		config_raw.rp_motionblur = false;
		///if (krom_android || krom_ios)
		config_raw.rp_ssao = false;
		///else
		config_raw.rp_ssao = true;
		///end
		config_raw.rp_ssr = false;
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

	zui_set_touch_scroll(config_raw.touch_ui);
	zui_set_touch_hold(config_raw.touch_ui);
	zui_set_touch_tooltip(config_raw.touch_ui);
	base_res_handle.position = config_raw.layer_res;
	config_load_keymap();
}

function config_get_sha(): string {
	let sha: string = "";
	let blob: buffer_t = data_get_blob("version.json");
	sha = json_parse(sys_buffer_to_string(blob)).sha;
	return sha;
}

function config_get_date(): string {
	let date: string = "";
	let blob: buffer_t = data_get_blob("version.json");
	date = json_parse(sys_buffer_to_string(blob)).date;
	return date;
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
	return {
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
}

function config_restore() {
	zui_children = map_create(); // Reset ui handles
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
	zui_children = map_create(); // Reset ui handles
	config_load_keymap();
	base_init_layout();
	translator_load_translations(config_raw.locale);
	config_apply();
	config_load_theme(config_raw.theme);
}

function config_apply() {
	config_raw.rp_ssao = context_raw.hssao.selected;
	config_raw.rp_ssr = context_raw.hssr.selected;
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

function config_load_keymap() {
	if (config_raw.keymap == "default.json") { // Built-in default
		config_keymap = base_default_keymap;
	}
	else {
		let blob: buffer_t = data_get_blob("keymap_presets/" + config_raw.keymap);
		config_keymap = json_parse(sys_buffer_to_string(blob));
		// Fill in undefined keys with defaults
		for (let i: i32 = 0; i < base_keymap_keys.length; ++i) {
			let key: string = base_keymap_keys[i];
			if (!(key in config_keymap)) {
				let adefault_keymap: any = base_default_keymap;
				config_keymap[key] = adefault_keymap[key];
			}
		}
	}
}

function config_save_keymap() {
	if (config_raw.keymap == "default.json") {
		return;
	}
	let path: string = data_path() + "keymap_presets/" + config_raw.keymap;
	let buffer: buffer_t = sys_string_to_buffer(json_stringify(config_keymap));
	krom_file_save_bytes(path, buffer);
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

function config_get_texture_res(): i32 {
	let res: i32 = base_res_handle.position;
	return res == texture_res_t.RES128 ? 128 :
		   res == texture_res_t.RES256 ? 256 :
		   res == texture_res_t.RES512 ? 512 :
		   res == texture_res_t.RES1024 ? 1024 :
		   res == texture_res_t.RES2048 ? 2048 :
		   res == texture_res_t.RES4096 ? 4096 :
		   res == texture_res_t.RES8192 ? 8192 :
		   res == texture_res_t.RES16384 ? 16384 : 0;
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

function config_load_theme(theme: string, tagRedraw: bool = true) {
	if (theme == "default.json") { // Built-in default
		base_theme = zui_theme_create();
	}
	else {
		let b: buffer_t = data_get_blob("themes/" + theme);
		let parsed: any = json_parse(sys_buffer_to_string(b));
		base_theme = zui_theme_create();
		for (let key in base_theme) {
			if (key == "theme_") {
				continue;
			}
			if (starts_with(key, "set_")) {
				continue;
			}
			if (starts_with(key, "get_")) {
				key = substring(key, 4, key.length);
			}
			let atheme: any = base_theme;
			atheme[key] = parsed[key];
		}
	}
	base_theme.FILL_WINDOW_BG = true;
	if (tagRedraw) {
		for (let i: i32 = 0; i < base_get_uis().length; ++i) {
			let ui: zui_t = base_get_uis()[i];
			ui.t = base_theme;
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
		config_button_align = zui_align_t.LEFT;
		config_button_spacing = "";
	}
	else {
		base_theme.FULL_TABS = false;
		config_button_align = zui_align_t.LEFT;
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

type config_t = {
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
};
