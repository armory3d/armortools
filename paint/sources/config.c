
#include "global.h"

void config_load() {
	char *path = "";
	if (path_is_protected()) {
		path = iron_internal_save_path();
	}
	path           = string("%sconfig.json", path);
	buffer_t *blob = data_get_blob(path);

#ifdef IRON_LINUX
	if (blob == NULL) { // Detect protected path
		config_init();
		config_save();
		blob = data_get_blob(path);
		if (blob == NULL) {
			path_is_protected_linux = true;
			config_load();
			return;
		}
	}
#endif

	if (blob != NULL) {
		char *config_string = sys_buffer_to_string(blob);
		if (starts_with(config_string, "{\"version\":")) { // Ensure valid config
			config_loaded = true;
			gc_unroot(config_raw);
			config_raw = json_parse(config_string);
			gc_root(config_raw);
		}
	}
}

void config_save() {
	// Use system application data folder
	// when running from protected path like "Program Files"
	char *path;
	if (path_is_protected()) {
		path = string("%sconfig.json", iron_internal_save_path());
	}
	else {
		path = string("%s%sconfig.json", path_data(), PATH_SEP);
	}

	json_encode_begin();
	json_encode_string("version", config_raw->version);
	json_encode_string("sha", config_raw->sha);
	json_encode_string("locale", config_raw->locale);
	json_encode_i32("window_mode", config_raw->window_mode);
	json_encode_i32("window_w", config_raw->window_w);
	json_encode_i32("window_h", config_raw->window_h);
	json_encode_i32("window_x", config_raw->window_x);
	json_encode_i32("window_y", config_raw->window_y);
	json_encode_bool("window_resizable", config_raw->window_resizable);
	json_encode_bool("window_maximizable", config_raw->window_maximizable);
	json_encode_bool("window_minimizable", config_raw->window_minimizable);
	json_encode_bool("window_vsync", config_raw->window_vsync);
	json_encode_i32("window_frequency", config_raw->window_frequency);
	json_encode_f32("window_scale", config_raw->window_scale);
	json_encode_f32("rp_supersample", config_raw->rp_supersample);
	json_encode_bool("rp_ssao", config_raw->rp_ssao);
	json_encode_bool("rp_bloom", config_raw->rp_bloom);
	json_encode_f32("rp_vignette", config_raw->rp_vignette);
	json_encode_f32("rp_grain", config_raw->rp_grain);
	json_encode_string_array("recent_projects", config_raw->recent_projects);
	json_encode_string_array("bookmarks", config_raw->bookmarks);
	json_encode_string_array("plugins", config_raw->plugins);
	json_encode_string("keymap", config_raw->keymap);
	json_encode_string("theme", config_raw->theme);
	json_encode_i32("undo_steps", config_raw->undo_steps);
	json_encode_f32("camera_fov", config_raw->camera_fov);
	json_encode_f32("camera_pan_speed", config_raw->camera_pan_speed);
	json_encode_f32("camera_zoom_speed", config_raw->camera_zoom_speed);
	json_encode_f32("camera_rotation_speed", config_raw->camera_rotation_speed);
	json_encode_bool("camera_upside_down", config_raw->camera_upside_down);
	json_encode_i32("zoom_direction", config_raw->zoom_direction);
	json_encode_bool("wrap_mouse", config_raw->wrap_mouse);
	json_encode_bool("show_asset_names", config_raw->show_asset_names);
	json_encode_bool("touch_ui", config_raw->touch_ui);
	json_encode_bool("splash_screen", config_raw->splash_screen);
	if (config_raw->layout != NULL) {
		json_encode_i32_array("layout", config_raw->layout);
	}
	else {
		json_encode_null("layout");
	}
	if (config_raw->layout_tabs != NULL) {
		json_encode_i32_array("layout_tabs", config_raw->layout_tabs);
	}
	else {
		json_encode_null("layout_tabs");
	}
	json_encode_i32("camera_controls", config_raw->camera_controls);
	json_encode_string("server", config_raw->server);
	json_encode_i32("viewport_mode", config_raw->viewport_mode);
	json_encode_i32("pathtrace_mode", config_raw->pathtrace_mode);
	json_encode_bool("pressure_radius", config_raw->pressure_radius);
	json_encode_f32("pressure_sensitivity", config_raw->pressure_sensitivity);
	json_encode_f32("displace_strength", config_raw->displace_strength);
	json_encode_i32("layer_res", config_raw->layer_res);
	json_encode_bool("brush_live", config_raw->brush_live);
	json_encode_bool("node_previews", config_raw->node_previews);
	json_encode_bool("pressure_hardness", config_raw->pressure_hardness);
	json_encode_bool("pressure_angle", config_raw->pressure_angle);
	json_encode_bool("pressure_opacity", config_raw->pressure_opacity);
	json_encode_bool("material_live", config_raw->material_live);
	json_encode_bool("brush_depth_reject", config_raw->brush_depth_reject);
	json_encode_bool("brush_angle_reject", config_raw->brush_angle_reject);
	json_encode_f32("brush_alpha_discard", config_raw->brush_alpha_discard);
	json_encode_i32("dilate_radius", config_raw->dilate_radius);
	json_encode_string("blender", config_raw->blender);
	json_encode_i32("scene_atlas_res", config_raw->scene_atlas_res);
	json_encode_bool("grid_snap", config_raw->grid_snap);
	json_encode_bool("experimental", config_raw->experimental);
	json_encode_i32("neural_backend", config_raw->neural_backend);
	json_encode_i32("render_mode", config_raw->render_mode);
	json_encode_i32("workspace", config_raw->workspace);
	json_encode_i32("workflow", config_raw->workflow);
	char *config_json = json_encode_end();

	buffer_t *buffer = sys_string_to_buffer(config_json);
	iron_file_save_bytes(path, buffer, 0);
}

void config_init() {
	if (!config_loaded || config_raw == NULL) {
		gc_unroot(config_raw);
		config_raw = GC_ALLOC_INIT(config_t, {0});
		gc_root(config_raw);
		config_raw->version            = string_copy(manifest_version_config);
		config_raw->sha                = string_copy(config_get_sha());
		config_raw->locale             = "en"; // "system";
		config_raw->window_mode        = 0;
		config_raw->window_resizable   = true;
		config_raw->window_minimizable = true;
		config_raw->window_maximizable = true;
		config_raw->window_w           = 1720;
		config_raw->window_h           = 960;
#ifdef IRON_MACOS
		config_raw->window_w *= 2;
		config_raw->window_h *= 2;
#endif
		config_raw->window_x     = -1;
		config_raw->window_y     = -1;
		config_raw->window_scale = 1.0;
		if (sys_display_width() >= 2560 && sys_display_height() >= 1600) {
			config_raw->window_scale = 2.0;
		}
#if defined(IRON_ANDROID) || defined(IRON_IOS) || defined(IRON_MACOS)
		config_raw->window_scale = 2.0;
#endif
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		if (sys_display_ppi() > 330) {
			config_raw->window_scale = 2.5;
		}
		if (sys_display_ppi() > 400) {
			config_raw->window_scale = 3.0;
		}
#endif
		config_raw->window_vsync     = true;
		config_raw->window_frequency = sys_display_frequency();
		config_raw->rp_bloom         = false;
		config_raw->rp_vignette      = 0.2;
		config_raw->rp_grain         = 0.09;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		config_raw->rp_ssao = false;
#else
		config_raw->rp_ssao = true;
#endif
		config_raw->rp_supersample = 1.0;
#ifdef IRON_ANDROID
		if (sys_display_width() >= 3200 && sys_display_height() >= 2136) {
			config_raw->window_scale   = 2.5;
			config_raw->rp_supersample = 0.5;
		}
#endif
		config_raw->recent_projects = any_array_create_from_raw((void *[]){}, 0);
		config_raw->bookmarks       = any_array_create_from_raw((void *[]){}, 0);
		config_raw->plugins         = any_array_create_from_raw((void *[]){}, 0);
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		config_raw->keymap = "touch.json";
#else
		config_raw->keymap = "default.json";
#endif
		config_raw->theme           = "default.json";
		config_raw->server          = "https://cloud.armory3d.com";
		config_raw->undo_steps      = 4;
		config_raw->pressure_radius = true;
#if defined(IRON_IOS) || defined(IRON_LINUX)
		config_raw->pressure_sensitivity = 1.0;
#else
		config_raw->pressure_sensitivity = 2.0;
#endif
		config_raw->camera_fov = 0.69;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		config_raw->camera_zoom_speed     = 0.5;
		config_raw->camera_pan_speed      = 0.5;
		config_raw->camera_rotation_speed = 0.5;
#else
		config_raw->camera_zoom_speed     = 1.0;
		config_raw->camera_pan_speed      = 1.0;
		config_raw->camera_rotation_speed = 1.0;
#endif
		config_raw->camera_upside_down = false;
		config_raw->zoom_direction     = ZOOM_DIRECTION_VERTICAL;
		config_raw->displace_strength  = 0.0;
		config_raw->wrap_mouse         = false;
		config_raw->camera_controls    = CAMERA_CONTROLS_ORBIT;
		config_raw->layer_res          = TEXTURE_RES_RES2048;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		config_raw->touch_ui      = true;
		config_raw->splash_screen = true;
#else
		config_raw->touch_ui      = false;
		config_raw->splash_screen = false;
#endif
		config_raw->node_previews     = false;
		config_raw->pressure_hardness = true;
		config_raw->pressure_angle    = false;
		config_raw->pressure_opacity  = false;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		config_raw->material_live = false;
#else
		config_raw->material_live = true;
#endif
		config_raw->brush_depth_reject  = true;
		config_raw->brush_angle_reject  = true;
		config_raw->brush_alpha_discard = 0.1;
		config_raw->brush_live          = false;
		config_raw->show_asset_names    = false;
		config_raw->dilate_radius       = 2;
		config_raw->blender             = "";
		config_raw->scene_atlas_res     = TEXTURE_RES_RES8192;
		config_raw->pathtrace_mode      = PATHTRACE_MODE_FAST;
		config_raw->grid_snap           = false;
		config_raw->experimental        = false;
		config_raw->neural_backend      = NEURAL_BACKEND_VULKAN;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		config_raw->render_mode = RENDER_MODE_FORWARD;
#else
		config_raw->render_mode = RENDER_MODE_DEFERRED;
#endif
		config_raw->workspace = WORKSPACE_PAINT_3D;
		config_raw->workflow  = WORKFLOW_PBR;
	}
	else {
		// Discard old config
		if (!string_equals(config_raw->sha, config_get_sha())) {
			config_loaded = false;
			config_init();
			return;
		}
	}

	ui_touch_control = config_raw->touch_ui;
	ui_touch_speed   = 1.0;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	if (sys_display_ppi() > 400) {
		ui_touch_speed = 0.5;
	}
#endif
	base_res_handle->i = config_raw->layer_res;
	keymap_load();
}

void config_init_layout() {
	config_t    *raw        = config_raw;
	bool         show2d     = (ui_nodes_show || ui_view2d_show) && raw->layout != NULL;
	i32_array_t *new_layout = i32_array_create_from_raw((i32[]){}, 0);

	i32_array_push(new_layout, math_floor(ui_sidebar_default_w * raw->window_scale)); // LayoutSidebarW
	i32_array_push(new_layout, math_floor(iron_window_height() / 2.0));          // LayoutSidebarH0
	i32_array_push(new_layout, math_floor(iron_window_height() / 2.0));          // LayoutSidebarH1

#ifdef IRON_IOS
	i32_array_push(new_layout, show2d ? math_floor((sys_w() + raw->layout->buffer[LAYOUT_SIZE_NODES_W]) * 0.473) : math_floor(sys_w() * 0.473)); // LayoutNodesW
#elif defined(IRON_ANDROID)
	i32_array_push(new_layout, show2d ? math_floor((sys_w() + raw->layout->buffer[LAYOUT_SIZE_NODES_W]) * 0.473) : math_floor(sys_w() * 0.473));
#else
	i32_array_push(new_layout,
	               show2d ? math_floor((sys_w() + raw->layout->buffer[LAYOUT_SIZE_NODES_W]) * 0.515)
	                      : math_floor(sys_w() * 0.515)); // Align with ui header controls
#endif

	i32_array_push(new_layout, math_floor(sys_h() / 2.0));                         // LayoutNodesH
	i32_array_push(new_layout, math_floor(ui_statusbar_default_h * raw->window_scale)); // LayoutStatusH

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	i32_array_push(new_layout, 0); // LayoutHeader
#else
	i32_array_push(new_layout, 1);
#endif

	raw->layout_tabs = i32_array_create_from_raw(
	    (i32[]){
	        0,
	        0,
	        0,
	    },
	    3);

	raw->layout = new_layout;
}

char *config_get_sha() {
	buffer_t *blob = data_get_blob("version.json");
	if (blob == NULL) {
		return "undefined";
	}
	version_t *v = json_parse(sys_buffer_to_string(blob));
	return v->sha;
}

char *config_get_date() {
	buffer_t *blob = data_get_blob("version.json");
	if (blob == NULL) {
		return "undefined";
	}
	version_t *v = json_parse(sys_buffer_to_string(blob));
	return v->date;
}

iron_window_options_t *config_get_options() {
	iron_window_mode_t     window_mode = config_raw->window_mode == 0 ? IRON_WINDOW_MODE_WINDOW : IRON_WINDOW_MODE_FULLSCREEN;
	iron_window_features_t features    = IRON_WINDOW_FEATURES_NONE;
	if (config_raw->window_resizable) {
		features |= IRON_WINDOW_FEATURES_RESIZABLE;
	}
	if (config_raw->window_maximizable) {
		features |= IRON_WINDOW_FEATURES_MAXIMIZABLE;
	}
	if (config_raw->window_minimizable) {
		features |= IRON_WINDOW_FEATURES_MINIMIZABLE;
	}
	char                  *title = string("untitled - %s", manifest_title);
	iron_window_options_t *ops   = GC_ALLOC_INIT(iron_window_options_t, {.title     = title,
	                                                                     .width     = config_raw->window_w,
	                                                                     .height    = config_raw->window_h,
	                                                                     .x         = config_raw->window_x,
	                                                                     .y         = config_raw->window_y,
	                                                                     .mode      = window_mode,
	                                                                     .features  = features,
	                                                                     .vsync     = config_raw->window_vsync,
	                                                                     .frequency = config_raw->window_frequency});
	return ops;
}

void config_restore() {
	gc_unroot(ui_children);
	ui_children = any_map_create(); // Reset ui handles
	gc_root(ui_children);
	config_loaded        = false;
	i32_array_t *_layout = config_raw->layout;
	config_init();
	config_raw->layout = _layout;
	config_init_layout();
	translator_load_translations(config_raw->locale);
	config_apply();
	config_load_theme(config_raw->theme, true);
}

void config_import_from(config_t *from) {
	char *_sha     = config_raw->sha;
	char *_version = config_raw->version;
	gc_unroot(config_raw);
	config_raw = from;
	gc_root(config_raw);
	config_raw->sha     = string_copy(_sha);
	config_raw->version = string_copy(_version);
	gc_unroot(ui_children);
	ui_children = any_map_create(); // Reset ui handles
	gc_root(ui_children);
	keymap_load();
	config_init_layout();
	translator_load_translations(config_raw->locale);
	config_apply();
	config_load_theme(config_raw->theme, true);
}

void config_apply() {
	config_save();
	context_raw->ddirty    = 2;
	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use)
		draw_end();
	render_path_base_apply_config();
	if (in_use)
		draw_begin(current, false, 0);
}

i32 config_get_super_sample_quality(f32 f) {
	return f == 0.25 ? 0 : f == 0.5 ? 1 : f == 1.0 ? 2 : f == 1.5 ? 3 : f == 2.0 ? 4 : 5;
}

f32 config_get_super_sample_size(i32 i) {
	return i == 0 ? 0.25 : i == 1 ? 0.5 : i == 2 ? 1.0 : i == 3 ? 1.5 : i == 4 ? 2.0 : 4.0;
}

i32 config_texture_res_size(i32 pos) {
	return pos == TEXTURE_RES_RES128     ? 128
	       : pos == TEXTURE_RES_RES256   ? 256
	       : pos == TEXTURE_RES_RES512   ? 512
	       : pos == TEXTURE_RES_RES1024  ? 1024
	       : pos == TEXTURE_RES_RES2048  ? 2048
	       : pos == TEXTURE_RES_RES4096  ? 4096
	       : pos == TEXTURE_RES_RES8192  ? 8192
	       : pos == TEXTURE_RES_RES16384 ? 16384
	                                     : 0;
}

i32 config_get_texture_res() {
	i32 res = base_res_handle->i;
	return config_texture_res_size(res);
}

i32 config_get_layer_res() {
	i32 res = config_raw->layer_res;
	return config_texture_res_size(res);
}

i32 config_get_scene_atlas_res() {
	i32 res = config_raw->scene_atlas_res;
	return config_texture_res_size(res);
}

i32 config_get_texture_res_x() {
	return context_raw->project_aspect_ratio == 2 ? math_floor(config_get_texture_res() / 2.0) : config_get_texture_res();
}

i32 config_get_texture_res_y() {
	return context_raw->project_aspect_ratio == 1 ? math_floor(config_get_texture_res() / 2.0) : config_get_texture_res();
}

i32 config_get_texture_res_pos(i32 i) {
	return i == 128     ? TEXTURE_RES_RES128
	       : i == 256   ? TEXTURE_RES_RES256
	       : i == 512   ? TEXTURE_RES_RES512
	       : i == 1024  ? TEXTURE_RES_RES1024
	       : i == 2048  ? TEXTURE_RES_RES2048
	       : i == 4096  ? TEXTURE_RES_RES4096
	       : i == 8192  ? TEXTURE_RES_RES8192
	       : i == 16384 ? TEXTURE_RES_RES16384
	                    : 0;
}

void config_load_theme(char *theme, bool tag_redraw) {
	gc_unroot(base_theme);
	base_theme = ui_theme_create();
	gc_root(base_theme);

	if (!string_equals(theme, "default.json")) {
		buffer_t   *b      = data_get_blob(string("themes/%s", theme));
		ui_theme_t *parsed = json_parse(sys_buffer_to_string(b));
		gc_unroot(base_theme);
		base_theme = parsed;
		gc_root(base_theme);
	}

	base_theme->FILL_WINDOW_BG = true;

	if (tag_redraw) {
		ui->ops->theme = base_theme;
		base_redraw_ui();
	}

	if (config_raw->touch_ui) {
		// Enlarge elements
		base_theme->FULL_TABS         = true;
		base_theme->ELEMENT_H         = 24 + 6;
		base_theme->BUTTON_H          = 22 + 6;
		base_theme->FONT_SIZE         = 13 + 2;
		base_theme->ARROW_SIZE        = 5 + 2;
		base_theme->CHECK_SIZE        = 15 + 4;
		base_theme->CHECK_SELECT_SIZE = 8 + 2;
	}
	else {
		base_theme->FULL_TABS = false;
	}
}

void config_enable_plugin(char *f) {
	if (string_array_index_of(config_raw->plugins, f) == -1) {
		any_array_push(config_raw->plugins, f);
		plugin_start(f);
	}
}

void config_disable_plugin(char *f) {
	if (string_array_index_of(config_raw->plugins, f) > -1) {
		string_array_remove(config_raw->plugins, f);
		plugin_stop(f);
	}
}

#ifdef IRON_IOS
bool config_is_iphone() {
	return sys_display_ppi() > 330;
}
#endif
