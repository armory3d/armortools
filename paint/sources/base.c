
#include "global.h"

void base_on_shutdown() {
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	project_save(false);
#endif
	config_save();
}

void base_on_background() {
	// Release keys after alt-tab / win-tab
	_key_up(KEY_CODE_ALT, NULL);
	_key_up(KEY_CODE_WIN, NULL);
}

void base_on_pause() {}

void base_on_resume() {}

void base_on_foreground() {
	context_raw->foreground_event = true;
	context_raw->last_paint_x     = -1;
	context_raw->last_paint_y     = -1;
}

void base_on_drop_files(char *drop_path) {
	drop_path = string_copy(trim_end(drop_path));
	any_array_push(base_drop_paths, drop_path);
}

void base_init() {
	base_last_window_width  = iron_window_width();
	base_last_window_height = iron_window_height();

	sys_notify_on_drop_files(&base_on_drop_files);
	sys_notify_on_app_state(&base_on_foreground, &base_on_resume, &base_on_pause, &base_on_background, &base_on_shutdown);
	iron_set_save_and_quit_callback(base_save_and_quit_callback);

	gc_unroot(base_font);
	base_font = data_get_font("font.ttf");
	gc_root(base_font);
	gc_unroot(base_color_wheel);
	base_color_wheel = data_get_image("color_wheel.k");
	gc_root(base_color_wheel);
	gc_unroot(base_color_wheel_gradient);
	base_color_wheel_gradient = data_get_image("color_wheel_gradient.k");
	gc_root(base_color_wheel_gradient);
	config_load_theme(config_raw->theme, false);
	base_default_element_w = base_theme->ELEMENT_W;
	base_default_element_h = base_theme->ELEMENT_H;
	base_default_font_size = base_theme->FONT_SIZE;
	translator_load_translations(config_raw->locale);
	gc_unroot(ui_files_filename);
	ui_files_filename = string_copy(tr("untitled"));
	gc_root(ui_files_filename);
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	sys_title_set(tr("untitled"));
#endif

	// Baked font for fast startup
	if (string_equals(config_raw->locale, "en")) {
		draw_font_13(base_font);
	}
	else {
		draw_font_init(base_font);
	}

	gc_unroot(ui_nodes_enum_texts);
	ui_nodes_enum_texts = base_enum_texts;
	gc_root(ui_nodes_enum_texts);

	// Init plugins
	if (config_raw->plugins != NULL) {
		for (i32 i = 0; i < config_raw->plugins->length; ++i) {
			char *plugin = config_raw->plugins->buffer[i];
			plugin_start(plugin);
		}
	}
	args_parse();
	camera_init();
	ui_base_init();
	ui_viewnodes_init();
	ui_view2d_init();

	sys_notify_on_update(base_update, NULL);
	sys_notify_on_update(ui_view2d_update, NULL);
	sys_notify_on_update(ui_nodes_update, NULL);
	sys_notify_on_update(ui_base_update, NULL);
	sys_notify_on_update(camera_update, NULL);
	sys_notify_on_render(ui_view2d_render, NULL);
	sys_notify_on_render(ui_base_render_cursor, NULL);
	sys_notify_on_render(ui_nodes_render, NULL);
	sys_notify_on_render(ui_base_render, NULL);
	sys_notify_on_render(base_render, NULL);

	base_appx = ui_toolbar_w(true);
	base_appy = 0;
	if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
		base_appy = ui_header_h * 2;
	}
	scene_camera->data->fov = math_floor(scene_camera->data->fov * 100) / (float)100;
	camera_object_build_proj(scene_camera, -1.0);

	args_run();

	if (config_raw->workspace != WORKSPACE_PAINT_3D) {
		base_update_workspace();
	}
	if (config_raw->workflow != WORKFLOW_PBR) {
		base_update_workflow();
	}

	bool has_projects = config_raw->recent_projects->length > 0;
	if (config_raw->splash_screen && has_projects) {
		box_projects_show();
	}
}

void base_save_and_quit_callback(bool save) {
	base_save_window_rect();
	if (save) {
		project_save(true);
	}
	else {
		iron_stop();
	}
}

i32 base_w() {
	// Drawing material preview
	if (context_raw->material_preview) {
		return util_render_material_preview_size;
	}

	// Drawing decal preview
	if (context_raw->decal_preview) {
		return util_render_decal_preview_size;
	}

	if (context_raw->paint2d_view) {
		return ui_view2d_ww;
	}

	// 3D view is hidden
	if (!base_view3d_show) {
		return 1;
	}

	i32 res = base_view3d_w();
	return res > 0 ? res : 1; // App was minimized, force render path resize
}

i32 base_view3d_w() {
	i32 res = 0;
	if (config_raw->layout == NULL) {
		i32 sidebarw = ui_sidebar_default_w;
		res          = iron_window_width() - sidebarw - ui_toolbar_default_w;
	}
	else if (ui_nodes_show || ui_view2d_show) {
		res = iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] - ui_toolbar_w(true);
	}
	else if (ui_base_show) {
		res = iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - ui_toolbar_w(true);
	}
	else { // Distract free
		res = iron_window_width();
	}
	if (context_raw->view_index > -1) {
		res = math_ceil(res / (float)2);
	}
	if (context_raw->paint2d_view) {
		res = ui_view2d_ww;
	}
	return res;
}

i32 base_h() {
	// Drawing material preview
	if (context_raw->material_preview) {
		return util_render_material_preview_size;
	}

	// Drawing decal preview
	if (context_raw->decal_preview) {
		return util_render_decal_preview_size;
	}

	i32 res = iron_window_height();

	if (config_raw->layout == NULL) {
		res -= ui_header_default_h * 2 + ui_statusbar_default_h;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
		res += ui_header_h;
#endif
	}
	else if (ui_base_show && res > 0) {
		i32 statush = config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H];
		res -= math_floor(ui_header_default_h * 2 * config_raw->window_scale) + statush;

		if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 0) {
			res += ui_header_h * 2;
		}
		if (!base_view3d_show) {
			res += ui_header_h * 4;
		}
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}

i32 base_x() {
	return context_raw->view_index == 1 ? base_appx + base_w() : base_appx;
}

i32 base_y() {
	return base_appy;
}

void base_on_resize() {
	if (iron_window_width() == 0 || iron_window_height() == 0) {
		return;
	}

	f32 ratio_w             = iron_window_width() / (float)base_last_window_width;
	base_last_window_width  = iron_window_width();
	f32 ratio_h             = iron_window_height() / (float)base_last_window_height;
	base_last_window_height = iron_window_height();

	config_raw->layout->buffer[LAYOUT_SIZE_NODES_W]    = math_floor(config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] * ratio_w);
	config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0] = math_floor(config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0] * ratio_h);
	config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H1] = iron_window_height() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0];

	base_resize();
	base_save_window_rect();
}

void base_save_window_rect() {
	config_raw->window_w = iron_window_width();
	config_raw->window_h = iron_window_height();
	config_raw->window_x = iron_window_x();
	config_raw->window_y = iron_window_y();
	config_save();
}

void base_resize() {
	if (iron_window_width() == 0 || iron_window_height() == 0) {
		return;
	}

	camera_object_t *cam = scene_camera;
	if (cam->data->ortho != NULL) {
		cam->data->ortho->buffer[2] = -2 * (sys_h() / (float)sys_w());
		cam->data->ortho->buffer[3] = 2 * (sys_h() / (float)sys_w());
	}
	camera_object_build_proj(cam, -1.0);
	render_path_base_taa_frame = 0;

	if (context_raw->camera_type == CAMERA_TYPE_ORTHOGRAPHIC) {
		viewport_update_camera_type(context_raw->camera_type);
	}

	context_raw->ddirty = 2;

	if (ui_base_show && base_view3d_show) {
		base_appx = ui_toolbar_w(true);
		base_appy = 0;
		if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
			base_appy = ui_header_h * 2;
		}
	}
	else {
		base_appx = -1;
		base_appy = 0;
	}

	ui_nodes_grid_redraw  = true;
	ui_view2d_grid_redraw = true;

	base_redraw_ui();
}

void base_update_import_asset_done() {
	// Asset was material
	if (project_materials->length > _base_material_count) {
		gc_unroot(base_drag_material);
		base_drag_material = context_raw->material;
		gc_root(base_drag_material);
		base_material_dropped();
	}
}

void base_update(void *_) {
	if (mouse_movement_x != 0 || mouse_movement_y != 0) {
		iron_mouse_set_cursor(IRON_CURSOR_ARROW);
	}

	bool has_drag = base_drag_asset != NULL || base_drag_material != NULL || base_drag_layer != NULL || base_drag_file != NULL || base_drag_swatch != NULL;

	if (config_raw->touch_ui) {
		// Touch and hold to activate dragging
		if (base_drag_start < 0.2) {
			if (has_drag && mouse_down("left")) {
				base_drag_start += sys_real_delta();
			}
			else {
				base_drag_start = 0;
			}
			has_drag = false;
		}
		if (mouse_released("left")) {
			base_drag_start = 0;
		}
		bool moved = math_abs(mouse_movement_x) > 1 && math_abs(mouse_movement_y) > 1;
		if ((mouse_released("left") || moved) && !has_drag) {
			gc_unroot(base_drag_asset);
			base_drag_asset = NULL;
			gc_unroot(base_drag_swatch);
			base_drag_swatch = NULL;
			gc_unroot(base_drag_file);
			base_drag_file = NULL;
			gc_unroot(base_drag_file_icon);
			base_drag_file_icon = NULL;
			base_is_dragging    = false;
			gc_unroot(base_drag_material);
			base_drag_material = NULL;
			gc_unroot(base_drag_layer);
			base_drag_layer = NULL;
		}
		// Disable touch scrolling while dragging is active
		ui_touch_control = !base_is_dragging;
	}

	if (has_drag && (mouse_movement_x != 0 || mouse_movement_y != 0)) {
		base_is_dragging = true;
	}
	if (mouse_released("left") && has_drag) {
		if (base_drag_asset != NULL) {

			// Create image texture
			if (context_in_nodes()) {
				ui_nodes_accept_asset_drop(array_index_of(project_assets, base_drag_asset));
			}
			else if (context_in_3d_view()) {
				if (ends_with(to_lower_case(base_drag_asset->file), ".hdr")) {
					gpu_texture_t *image = project_get_image(base_drag_asset);
					import_envmap_run(base_drag_asset->file, image);
				}
			}
			// Create mask
			else if (context_in_layers() || context_in_2d_view(VIEW_2D_TYPE_LAYER)) {
				layers_create_image_mask(base_drag_asset);
			}
			gc_unroot(base_drag_asset);
			base_drag_asset = NULL;
		}
		else if (base_drag_swatch != NULL) {
			// Create RGB node
			if (context_in_nodes()) {
				ui_nodes_accept_swatch_drop(base_drag_swatch);
			}
			else if (context_in_swatches()) {
				tab_swatches_accept_swatch_drop(base_drag_swatch);
			}
			else if (context_in_materials()) {
				tab_materials_accept_swatch_drop(base_drag_swatch);
			}
			else if (context_in_3d_view()) {
				i32 color = base_drag_swatch->base;
				color     = color_set_ab(color, base_drag_swatch->opacity * 255);
				layers_create_color_layer(color, base_drag_swatch->occlusion, base_drag_swatch->roughness, base_drag_swatch->metallic, -1);
			}
			else if (context_in_layers() && tab_layers_can_drop_new_layer(context_raw->drag_dest)) {
				i32 color = base_drag_swatch->base;
				color     = color_set_ab(color, base_drag_swatch->opacity * 255);
				layers_create_color_layer(color, base_drag_swatch->occlusion, base_drag_swatch->roughness, base_drag_swatch->metallic, context_raw->drag_dest);
			}

			gc_unroot(base_drag_swatch);
			base_drag_swatch = NULL;
		}
		else if (base_drag_file != NULL) {
			if (!context_in_browser()) {
				base_drop_x = mouse_x;
				base_drop_y = mouse_y;

				_base_material_count = project_materials->length;
				import_asset_run(base_drag_file, base_drop_x, base_drop_y, true, true, &base_update_import_asset_done);
			}

			gc_unroot(base_drag_file);
			base_drag_file = NULL;
			gc_unroot(base_drag_file_icon);
			base_drag_file_icon = NULL;
		}
		else if (base_drag_material != NULL) {
			base_material_dropped();
		}
		else if (base_drag_layer != NULL) {
			if (context_in_nodes()) {
				ui_nodes_accept_layer_drop(array_index_of(project_layers, base_drag_layer));
			}
			else if (context_in_layers() && base_is_dragging) {
				slot_layer_move(base_drag_layer, context_raw->drag_dest);
				make_material_parse_mesh_material();
			}
			gc_unroot(base_drag_layer);
			base_drag_layer = NULL;
		}

		iron_mouse_set_cursor(IRON_CURSOR_ARROW);
		base_is_dragging = false;
	}
	if (context_raw->color_picker_callback != NULL && (mouse_released("left") || mouse_released("right"))) {
		context_raw->color_picker_callback = NULL;
		context_select_tool(context_raw->color_picker_previous_tool);
	}

	base_handle_drop_paths();

	if (context_raw->ddirty < 0) {
		context_raw->ddirty = 0;
	}

	if (context_raw->tool == TOOL_TYPE_GIZMO) {
		if (keyboard_down("control") && keyboard_started("d")) {
			sim_duplicate();
		}
		if (keyboard_started("delete")) {
			sim_delete();
		}
	}

	compass_update();
}

void base_material_dropped() {
	// Material drag and dropped onto viewport or layers tab
	if (context_in_3d_view()) {
		uv_type_t uv_type   = keyboard_down("control") ? UV_TYPE_PROJECT : UV_TYPE_UVMAP;
		mat4_t    decal_mat = uv_type == UV_TYPE_PROJECT ? util_render_get_decal_mat() : mat4_nan();
		layers_create_fill_layer(uv_type, decal_mat, -1);
	}
	if (context_in_layers() && tab_layers_can_drop_new_layer(context_raw->drag_dest)) {
		uv_type_t uv_type   = keyboard_down("control") ? UV_TYPE_PROJECT : UV_TYPE_UVMAP;
		mat4_t    decal_mat = uv_type == UV_TYPE_PROJECT ? util_render_get_decal_mat() : mat4_nan();
		layers_create_fill_layer(uv_type, decal_mat, context_raw->drag_dest);
	}
	else if (context_in_nodes()) {
		ui_nodes_accept_material_drop(array_index_of(project_materials, base_drag_material));
	}
	gc_unroot(base_drag_material);
	base_drag_material = NULL;
}

void base_handle_drop_paths() {
	if (base_drop_paths->length > 0) {
		bool wait = false;
#if defined(IRON_LINUX) || defined(IRON_MACOS)
		wait = !mouse_moved; // Mouse coords not updated during drag
#endif
		if (!wait) {
			base_drop_x     = mouse_x;
			base_drop_y     = mouse_y;
			char *drop_path = array_shift(base_drop_paths);
			import_asset_run(drop_path, base_drop_x, base_drop_y, true, true, NULL);
		}
	}
}

rect_t *base_get_drag_background() {
	gpu_texture_t *icons = resource_get("icons.k");
	if (base_drag_layer != NULL && !slot_layer_is_group(base_drag_layer) && base_drag_layer->fill_layer == NULL) {
		return resource_tile50(icons, ICON_CHECKER);
	}
	return NULL;
}

gpu_texture_t *base_get_drag_image() {
	base_drag_tint = 0xffffffff;
	base_drag_size = -1;
	gc_unroot(base_drag_rect);
	base_drag_rect = NULL;
	if (base_drag_asset != NULL) {
		return project_get_image(base_drag_asset);
	}
	if (base_drag_swatch != NULL) {
		base_drag_tint = base_drag_swatch->base;
		base_drag_size = 26;
		return tab_swatches_empty_get();
	}
	if (base_drag_file != NULL) {
		if (base_drag_file_icon != NULL) {
			return base_drag_file_icon;
		}
		gpu_texture_t *icons = resource_get("icons.k");
		gc_unroot(base_drag_rect);
		base_drag_rect = string_index_of(base_drag_file, ".") > 0 ? resource_tile50(icons, ICON_FILE) : resource_tile50(icons, ICON_FOLDER_FULL);
		gc_root(base_drag_rect);
		base_drag_tint = ui->ops->theme->HIGHLIGHT_COL;
		return icons;
	}

	if (base_drag_material != NULL) {
		return base_drag_material->image_icon;
	}
	if (base_drag_layer != NULL && slot_layer_is_group(base_drag_layer)) {
		gpu_texture_t *icons         = resource_get("icons.k");
		rect_t        *folder_closed = resource_tile50(icons, ICON_FOLDER_FULL);
		rect_t        *folder_open   = resource_tile50(icons, ICON_FOLDER_OPEN);
		gc_unroot(base_drag_rect);
		base_drag_rect = base_drag_layer->show_panel ? folder_open : folder_closed;
		gc_root(base_drag_rect);
		base_drag_tint = ui->ops->theme->LABEL_COL - 0x00202020;
		return icons;
	}
	if (base_drag_layer != NULL && slot_layer_is_mask(base_drag_layer) && base_drag_layer->fill_layer == NULL) {
		tab_layers_make_mask_preview_rgba32(base_drag_layer);
		return context_raw->mask_preview_rgba32;
	}
	if (base_drag_layer != NULL) {
		return base_drag_layer->fill_layer != NULL ? base_drag_layer->fill_layer->image_icon : base_drag_layer->texpaint_preview;
	}
	return NULL;
}

void base_render(void *_) {
	if (context_raw->frame == 2) {
		util_render_make_material_preview();
		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;

		base_init_undo_layers();
	}

	if (context_raw->tool == TOOL_TYPE_GIZMO) {
		sim_init();
		sim_update();
	}

	if (context_raw->frame == 2) {
		make_material_parse_mesh_material();
		make_material_parse_paint_material(true);
		context_raw->ddirty = 0;

		// Mouse coords not updated during drag
		context_raw->camera_controls = config_raw->camera_controls;
	}
	else if (context_raw->frame == 3) {
		context_raw->ddirty = 3;
	}

	context_raw->frame++;

	if (base_is_dragging) {
		iron_mouse_set_cursor(IRON_CURSOR_HAND);
		gpu_texture_t *img          = base_get_drag_image();
		f32            scale_factor = UI_SCALE();
		f32            size         = (base_drag_size == -1 ? 50 : base_drag_size) * scale_factor;
		f32            ratio        = size / (float)img->width;
		f32            h            = img->height * ratio;
		i32            inv          = 0;

		draw_begin(NULL, false, 0);
		draw_set_color(base_drag_tint);

		rect_t *bg_rect = base_get_drag_background();
		if (bg_rect != NULL) {
			draw_scaled_sub_image(resource_get("icons.k"), bg_rect->x, bg_rect->y, bg_rect->w, bg_rect->h, mouse_x + base_drag_off_x,
			                      mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		}

		base_drag_rect == NULL ? draw_scaled_image(img, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2)
		                       : draw_scaled_sub_image(img, base_drag_rect->x, base_drag_rect->y, base_drag_rect->w, base_drag_rect->h,
		                                               mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		draw_set_color(0xffffffff);
		draw_end();
	}

	bool using_menu = ui_menu_show && mouse_y > ui_header_h;
	base_ui_enabled = !ui_box_show && !using_menu && ui->combo_selected_handle == NULL;
	if (ui_box_show) {
		ui_box_render();
	}
	if (ui_menu_show) {
		ui_menu_render();
	}

	// Save last pos for continuos paint
	context_raw->last_paint_vec_x = context_raw->paint_vec.x;
	context_raw->last_paint_vec_y = context_raw->paint_vec.y;

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	// No mouse move events for touch, re-init last paint position on touch start
	if (!mouse_down("left")) {
		context_raw->last_paint_x = -1;
		context_raw->last_paint_y = -1;
	}
#endif
}

string_t_array_t *base_enum_texts(char *node_type) {
	if (string_equals(node_type, "TEX_IMAGE")) {
		if (project_asset_names->length > 0) {
			return project_asset_names;
		}
		else {
			string_t_array_t *empty = any_array_create_from_raw(
			    (void *[]){
			        "",
			    },
			    1);
			return empty;
		}
	}

	if (string_equals(node_type, "LAYER") || string_equals(node_type, "LAYER_MASK")) {
		string_t_array_t *layer_names = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < project_layers->length; ++i) {
			slot_layer_t *l = project_layers->buffer[i];
			any_array_push(layer_names, l->name);
		}
		return layer_names;
	}

	if (string_equals(node_type, "MATERIAL")) {
		string_t_array_t *material_names = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < project_materials->length; ++i) {
			slot_material_t *m = project_materials->buffer[i];
			any_array_push(material_names, m->canvas->name);
		}
		return material_names;
	}

	if (string_equals(node_type, "image_texture_node")) {
		if (project_asset_names->length > 0) {
			return project_asset_names;
		}
		else {
			string_t_array_t *empty = any_array_create_from_raw(
			    (void *[]){
			        "",
			    },
			    1);
			return empty;
		}
	}

	return NULL;
}

i32 base_get_asset_index(char *file_name) {
	i32 i = string_array_index_of(project_asset_names, file_name);
	return i >= 0 ? i : 0;
}

void base_toggle_fullscreen() {
	if (iron_window_get_mode() == IRON_WINDOW_MODE_WINDOW) {
		config_raw->window_w = iron_window_width();
		config_raw->window_h = iron_window_height();
		config_raw->window_x = iron_window_x();
		config_raw->window_y = iron_window_y();
		iron_window_change_mode(IRON_WINDOW_MODE_FULLSCREEN);
	}
	else {
		iron_window_change_mode(IRON_WINDOW_MODE_WINDOW);
		iron_window_resize(config_raw->window_w, config_raw->window_h);
		iron_window_move(config_raw->window_x, config_raw->window_y);
	}
	context_raw->ddirty = 3;
}

bool base_is_decal_layer() {
	bool is_painting = context_raw->tool != TOOL_TYPE_MATERIAL && context_raw->tool != TOOL_TYPE_BAKE;
	return is_painting && context_raw->layer->fill_layer != NULL && context_raw->layer->uv_type == UV_TYPE_PROJECT;
}

void base_redraw_status() {
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
}

void base_redraw_console() {
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
}

void base_init_undo_layers() {
	if (history_undo_layers == NULL) {
		gc_unroot(history_undo_layers);
		history_undo_layers = any_array_create_from_raw((void *[]){}, 0);
		gc_root(history_undo_layers);
		for (i32 i = 0; i < config_raw->undo_steps; ++i) {
			i32           len = history_undo_layers->length;
			char         *ext = string("_undo%s", i32_to_string(len));
			slot_layer_t *l   = slot_layer_create(ext, LAYER_SLOT_TYPE_LAYER, NULL);
			any_array_push(history_undo_layers, l);
		}
	}
}

ui_handle_t_array_t *ui_base_init_hwnds() {
	ui_handle_t_array_t *hwnds = any_array_create_from_raw(
	    (void *[]){
	        ui_handle_create(),
	        ui_handle_create(),
	        ui_handle_create(),
	    },
	    3);
	return hwnds;
}

ui_handle_t_array_t *ui_base_init_htabs() {
	ui_handle_t_array_t *htabs = any_array_create_from_raw(
	    (void *[]){
	        ui_handle_create(),
	        ui_handle_create(),
	        ui_handle_create(),
	    },
	    3);
	return htabs;
}

tab_draw_t *_draw_callback_create(void (*f)(ui_handle_t *)) {
	tab_draw_t *cb = GC_ALLOC_INIT(tab_draw_t, {.f = f});
	return cb;
}

tab_draw_array_t_array_t *ui_base_init_hwnd_tabs() {
	tab_draw_array_t *a0 = any_array_create_from_raw(
	    (void *[]){
	        _draw_callback_create(tab_layers_draw),
	        _draw_callback_create(tab_history_draw),
	        _draw_callback_create(tab_scripts_draw),
	    },
	    3);
	tab_draw_array_t *a1 = any_array_create_from_raw(
	    (void *[]){
	        _draw_callback_create(tab_materials_draw),
	        _draw_callback_create(tab_brushes_draw),
	        _draw_callback_create(tab_plugins_draw),
	    },
	    3);
	tab_draw_array_t *a2 = any_array_create_from_raw(
	    (void *[]){
	        _draw_callback_create(tab_browser_draw),
	        _draw_callback_create(tab_meshes_draw),
	        _draw_callback_create(tab_textures_draw),
	        _draw_callback_create(tab_fonts_draw),
	        _draw_callback_create(tab_swatches_draw),
	        _draw_callback_create(tab_console_draw),
	        _draw_callback_create(ui_statusbar_draw_version_tab),
	    },
	    7);

#ifdef IRON_IOS
	if (config_is_iphone()) {
		array_splice(a2, 4, 1); // Swatches
	}
#endif

	tab_draw_array_t_array_t *r = any_array_create_from_raw((void *[]){}, 0);
	any_array_push(r, a0);
	any_array_push(r, a1);
	any_array_push(r, a2);
	return r;
}

void ui_base_init_on_next_frame(void *_) {
	layers_init();
}

void ui_base_init() {
	ui_toolbar_init();
	context_raw->text_tool_text = string_copy(tr("Text"));
	ui_header_init();
	ui_statusbar_init();
	ui_menubar_init();

	ui_header_h  = math_floor(ui_header_default_h * config_raw->window_scale);
	ui_menubar_w = math_floor(ui_menubar_default_w * config_raw->window_scale);

	if (context_raw->empty_envmap == NULL) {
		ui_base_make_empty_envmap(base_theme->VIEWPORT_COL);
	}
	if (context_raw->preview_envmap == NULL) {
		u8_array_t *b               = u8_array_create(4);
		b->buffer[0]                = 0;
		b->buffer[1]                = 0;
		b->buffer[2]                = 0;
		b->buffer[3]                = 255;
		context_raw->preview_envmap = gpu_create_texture_from_bytes(b, 1, 1, GPU_TEXTURE_FORMAT_RGBA32);
	}

	if (context_raw->saved_envmap == NULL) {
		// raw.saved_envmap = scene_world._envmap;
		context_raw->default_irradiance       = scene_world->_->irradiance;
		context_raw->default_radiance         = scene_world->_->radiance;
		context_raw->default_radiance_mipmaps = scene_world->_->radiance_mipmaps;
	}
	scene_world->_->envmap = context_raw->show_envmap ? context_raw->saved_envmap : context_raw->empty_envmap;
	context_raw->ddirty    = 1;

	string_t_array_t *resources = any_array_create_from_raw(
	    (void *[]){
	        "cursor.k",
	        "icons.k",
	        "icons05x.k",
	    },
	    3);
	resource_load(resources);

	f32           scale = config_raw->window_scale;
	ui_options_t *ops   = GC_ALLOC_INIT(
        ui_options_t,
        {.theme = base_theme, .font = base_font, .scale_factor = scale, .color_wheel = base_color_wheel, .black_white_gradient = base_color_wheel_gradient});
	gc_unroot(ui);
	ui = ui_create(ops);
	gc_root(ui);
	gc_unroot(ui_on_border_hover);
	ui_on_border_hover = ui_base_on_border_hover;
	gc_root(ui_on_border_hover);
	gc_unroot(ui_on_tab_drop);
	ui_on_tab_drop = ui_base_on_tab_drop;
	gc_root(ui_on_tab_drop);
	if (UI_SCALE() > 1) {
		ui_base_set_icon_scale();
	}

	context_raw->gizmo             = scene_get_child(".Gizmo");
	context_raw->gizmo_translate_x = object_get_child(context_raw->gizmo, ".TranslateX");
	context_raw->gizmo_translate_y = object_get_child(context_raw->gizmo, ".TranslateY");
	context_raw->gizmo_translate_z = object_get_child(context_raw->gizmo, ".TranslateZ");
	context_raw->gizmo_scale_x     = object_get_child(context_raw->gizmo, ".ScaleX");
	context_raw->gizmo_scale_y     = object_get_child(context_raw->gizmo, ".ScaleY");
	context_raw->gizmo_scale_z     = object_get_child(context_raw->gizmo, ".ScaleZ");
	context_raw->gizmo_rotate_x    = object_get_child(context_raw->gizmo, ".RotateX");
	context_raw->gizmo_rotate_y    = object_get_child(context_raw->gizmo, ".RotateY");
	context_raw->gizmo_rotate_z    = object_get_child(context_raw->gizmo, ".RotateZ");

	project_new(false);

	if (string_equals(project_filepath, "")) {
		sys_notify_on_next_frame(&ui_base_init_on_next_frame, NULL);
	}

	context_raw->project_objects = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < scene_meshes->length; ++i) {
		mesh_object_t *m = scene_meshes->buffer[i];
		any_array_push(context_raw->project_objects, m);
	}

	operator_register("view_top", ui_base_view_top);
}

void ui_base_menu_draw_viewport_mode() {
	ui_handle_t *mode_handle = ui_handle(__ID__);
	mode_handle->i           = context_raw->viewport_mode;
	ui_text(tr("Viewport Mode"), UI_ALIGN_RIGHT, 0x00000000);

	string_t_array_t *modes = any_array_create_from_raw(
	    (void *[]){
	        tr("Lit"),
	        tr("Base Color"),
	        tr("Normal"),
	        tr("Occlusion"),
	        tr("Roughness"),
	        tr("Metallic"),
	        tr("Opacity"),
	        tr("Height"),
	        tr("Emission"),
	        tr("Subsurface"),
	        tr("TexCoord"),
	        tr("Object Normal"),
	        tr("Material ID"),
	        tr("Object ID"),
	        tr("Mask"),
	    },
	    15);
	string_t_array_t *shortcuts = any_array_create_from_raw(
	    (void *[]){
	        "l",
	        "b",
	        "n",
	        "o",
	        "r",
	        "m",
	        "a",
	        "h",
	        "e",
	        "s",
	        "t",
	        "1",
	        "2",
	        "3",
	        "4",
	    },
	    15);
	if (gpu_raytrace_supported()) {
		any_array_push(modes, tr("Path Traced"));
		any_array_push(shortcuts, "p");
	}
	for (i32 i = 0; i < modes->length; ++i) {
		ui_radio(mode_handle, i, modes->buffer[i], shortcuts->buffer[i]);
	}

	i32 index = string_array_index_of(shortcuts, keyboard_key_code(ui->key_code));
	if (ui->is_key_pressed && index != -1) {
		mode_handle->i = index;
		ui->changed    = true;
		context_set_viewport_mode(mode_handle->i);
	}
	else if (mode_handle->changed) {
		context_set_viewport_mode(mode_handle->i);
		ui->changed = true;
	}
}

void ui_base_update_51603(void *_) {
	export_texture_run(context_raw->texture_export_path, false);
}

void ui_base_update(void *_) {
	ui_base_update_ui();
	operator_update();

	string_t_array_t *keys = map_keys(plugin_map);
	for (i32 i = 0; i < keys->length; ++i) {
		plugin_t *p = any_map_get(plugin_map, keys->buffer[i]);
		if (p->on_update != NULL) {
			js_call(p->on_update);
		}
	}

	if (!base_ui_enabled) {
		return;
	}

	if (!ui->is_typing) {
		if (operator_shortcut(any_map_get(config_keymap, "toggle_node_editor"), SHORTCUT_TYPE_STARTED)) {
			ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL ? ui_base_show_material_nodes() : ui_base_show_brush_nodes();
		}
		else if (operator_shortcut(any_map_get(config_keymap, "toggle_browser"), SHORTCUT_TYPE_STARTED)) {
			ui_base_toggle_browser();
		}
		else if (operator_shortcut(any_map_get(config_keymap, "toggle_2d_view"), SHORTCUT_TYPE_STARTED)) {
			ui_base_show_2d_view(VIEW_2D_TYPE_LAYER);
		}
	}

	if (operator_shortcut(any_map_get(config_keymap, "file_save_as"), SHORTCUT_TYPE_STARTED)) {
		project_save_as(false);
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_save"), SHORTCUT_TYPE_STARTED)) {
		project_save(false);
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_open"), SHORTCUT_TYPE_STARTED)) {
		project_open();
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_open_recent"), SHORTCUT_TYPE_STARTED)) {
		box_projects_show();
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_reimport_mesh"), SHORTCUT_TYPE_STARTED)) {
		project_reimport_mesh();
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_reimport_textures"), SHORTCUT_TYPE_STARTED)) {
		project_reimport_textures();
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_new"), SHORTCUT_TYPE_STARTED)) {
		project_new_box();
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_export_textures"), SHORTCUT_TYPE_STARTED)) {
		if (string_equals(context_raw->texture_export_path, "")) { // First export, ask for path
			context_raw->layers_export = EXPORT_MODE_VISIBLE;
			box_export_show_textures();
		}
		else {
			sys_notify_on_next_frame(&ui_base_update_51603, NULL);
		}
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_export_textures_as"), SHORTCUT_TYPE_STARTED)) {
		context_raw->layers_export = EXPORT_MODE_VISIBLE;
		box_export_show_textures();
	}
	else if (operator_shortcut(any_map_get(config_keymap, "file_import_assets"), SHORTCUT_TYPE_STARTED)) {
		project_import_asset(NULL, true);
	}
	else if (operator_shortcut(any_map_get(config_keymap, "edit_prefs"), SHORTCUT_TYPE_STARTED)) {
		box_preferences_show();
	}
	if (keyboard_started(any_map_get(config_keymap, "view_distract_free")) || (keyboard_started("escape") && !ui_base_show && !ui_box_show)) {
		ui_base_toggle_distract_free();
	}

#ifdef IRON_LINUX
	if (operator_shortcut("alt+enter", SHORTCUT_TYPE_STARTED)) {
		base_toggle_fullscreen();
	}
#endif

	bool decal_mask = context_is_decal_mask();

	if (context_raw->brush_locked && mouse_moved) {
		if (operator_shortcut(any_map_get(config_keymap, "brush_radius"), SHORTCUT_TYPE_DOWN) ||
		    operator_shortcut(any_map_get(config_keymap, "brush_opacity"), SHORTCUT_TYPE_DOWN) ||
		    operator_shortcut(any_map_get(config_keymap, "brush_angle"), SHORTCUT_TYPE_DOWN) ||
		    (decal_mask &&
		     operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "brush_radius")), SHORTCUT_TYPE_DOWN))) {
			if (context_raw->brush_locked) {
				if (operator_shortcut(any_map_get(config_keymap, "brush_opacity"), SHORTCUT_TYPE_DOWN)) {
					context_raw->brush_opacity += mouse_movement_x / (float)500;
					context_raw->brush_opacity           = math_max(0.0, math_min(1.0, context_raw->brush_opacity));
					context_raw->brush_opacity           = math_round(context_raw->brush_opacity * 100) / (float)100;
					context_raw->brush_opacity_handle->f = context_raw->brush_opacity;
				}
				else if (operator_shortcut(any_map_get(config_keymap, "brush_angle"), SHORTCUT_TYPE_DOWN)) {
					context_raw->brush_angle += mouse_movement_x / (float)5;
					i32 i                    = math_floor(context_raw->brush_angle);
					context_raw->brush_angle = i % 360;
					if (context_raw->brush_angle < 0)
						context_raw->brush_angle += 360;
					context_raw->brush_angle_handle->f = context_raw->brush_angle;
					make_material_parse_paint_material(true);
				}
				else if (decal_mask && operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "brush_radius")),
				                                         SHORTCUT_TYPE_DOWN)) {
					context_raw->brush_decal_mask_radius += mouse_movement_x / (float)150;
					context_raw->brush_decal_mask_radius           = math_max(0.01, math_min(4.0, context_raw->brush_decal_mask_radius));
					context_raw->brush_decal_mask_radius           = math_round(context_raw->brush_decal_mask_radius * 100) / (float)100;
					context_raw->brush_decal_mask_radius_handle->f = context_raw->brush_decal_mask_radius;
				}
				else {
					context_raw->brush_radius += mouse_movement_x / (float)150;
					context_raw->brush_radius           = math_max(0.01, math_min(4.0, context_raw->brush_radius));
					context_raw->brush_radius           = math_round(context_raw->brush_radius * 100) / (float)100;
					context_raw->brush_radius_handle->f = context_raw->brush_radius;
				}
				ui_header_handle->redraws = 2;
			}
		}
	}

	bool is_typing = ui->is_typing;
	if (!is_typing) {
		if (operator_shortcut(any_map_get(config_keymap, "select_material"), SHORTCUT_TYPE_DOWN)) {
			ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
			for (i32 i = 1; i < 10; ++i) {
				if (keyboard_started(i32_to_string(i))) {
					context_select_material(i - 1);
				}
			}
		}
		else if (operator_shortcut(any_map_get(config_keymap, "select_layer"), SHORTCUT_TYPE_DOWN)) {
			ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
			for (i32 i = 1; i < 10; ++i) {
				if (keyboard_started(i32_to_string(i))) {
					context_select_layer(i - 1);
				}
			}
		}
	}

	// Viewport shortcuts
	if (context_in_paint_area() && !is_typing) {

		if (!mouse_down("right")) { // Fly mode off
			if (operator_shortcut(any_map_get(config_keymap, "tool_brush"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_BRUSH);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_eraser"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_ERASER);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_fill"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_FILL);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_colorid"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_COLORID);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_decal"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_DECAL);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_text"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_TEXT);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_clone"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_CLONE);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_blur"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_BLUR);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_smudge"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_SMUDGE);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_particle"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_PARTICLE);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_picker"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_PICKER);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_bake"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_BAKE);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_gizmo"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_GIZMO);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "tool_material"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(TOOL_TYPE_MATERIAL);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "swap_brush_eraser"), SHORTCUT_TYPE_STARTED)) {
				context_select_tool(context_raw->tool == TOOL_TYPE_BRUSH ? TOOL_TYPE_ERASER : TOOL_TYPE_BRUSH);
			}
		}

		// Radius
		if (context_raw->tool == TOOL_TYPE_BRUSH || context_raw->tool == TOOL_TYPE_ERASER || context_raw->tool == TOOL_TYPE_DECAL ||
		    context_raw->tool == TOOL_TYPE_TEXT || context_raw->tool == TOOL_TYPE_CLONE || context_raw->tool == TOOL_TYPE_BLUR ||
		    context_raw->tool == TOOL_TYPE_SMUDGE || context_raw->tool == TOOL_TYPE_PARTICLE) {
			if (operator_shortcut(any_map_get(config_keymap, "brush_radius"), SHORTCUT_TYPE_STARTED) ||
			    operator_shortcut(any_map_get(config_keymap, "brush_opacity"), SHORTCUT_TYPE_STARTED) ||
			    operator_shortcut(any_map_get(config_keymap, "brush_angle"), SHORTCUT_TYPE_STARTED) ||
			    (decal_mask && operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "brush_radius")),
			                                     SHORTCUT_TYPE_STARTED))) {
				context_raw->brush_locked = true;
				if (!pen_connected) {
					iron_mouse_lock();
				}
			}
			else if (operator_shortcut(any_map_get(config_keymap, "brush_radius_decrease"), SHORTCUT_TYPE_REPEAT)) {
				context_raw->brush_radius -= ui_base_get_radius_increment();
				context_raw->brush_radius           = math_max(math_round(context_raw->brush_radius * 100) / (float)100, 0.01);
				context_raw->brush_radius_handle->f = context_raw->brush_radius;
				ui_header_handle->redraws           = 2;
			}
			else if (operator_shortcut(any_map_get(config_keymap, "brush_radius_increase"), SHORTCUT_TYPE_REPEAT)) {
				context_raw->brush_radius += ui_base_get_radius_increment();
				context_raw->brush_radius           = math_round(context_raw->brush_radius * 100) / (float)100;
				context_raw->brush_radius_handle->f = context_raw->brush_radius;
				ui_header_handle->redraws           = 2;
			}
			else if (decal_mask) {
				if (operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "brush_radius_decrease")),
				                      SHORTCUT_TYPE_REPEAT)) {
					context_raw->brush_decal_mask_radius -= ui_base_get_radius_increment();
					context_raw->brush_decal_mask_radius           = math_max(math_round(context_raw->brush_decal_mask_radius * 100) / (float)100, 0.01);
					context_raw->brush_decal_mask_radius_handle->f = context_raw->brush_decal_mask_radius;
					ui_header_handle->redraws                      = 2;
				}
				else if (operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "brush_radius_increase")),
				                           SHORTCUT_TYPE_REPEAT)) {
					context_raw->brush_decal_mask_radius += ui_base_get_radius_increment();
					context_raw->brush_decal_mask_radius           = math_round(context_raw->brush_decal_mask_radius * 100) / (float)100;
					context_raw->brush_decal_mask_radius_handle->f = context_raw->brush_decal_mask_radius;
					ui_header_handle->redraws                      = 2;
				}
			}
		}
		if (decal_mask && (operator_shortcut(any_map_get(config_keymap, "decal_mask"), SHORTCUT_TYPE_STARTED) ||
		                   operator_shortcut(any_map_get(config_keymap, "decal_mask"), SHORTCUT_TYPE_RELEASED))) {
			ui_header_handle->redraws = 2;
		}

		// Viewpoint
		if (mouse_view_x() < sys_w()) {
			if (operator_shortcut(any_map_get(config_keymap, "view_reset"), SHORTCUT_TYPE_STARTED)) {
				viewport_reset();
				viewport_scale_to_bounds(2.0);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_back"), SHORTCUT_TYPE_STARTED)) {
				viewport_set_view(0, 1, 0, math_pi() / (float)2, 0, math_pi());
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_front"), SHORTCUT_TYPE_STARTED)) {
				viewport_set_view(0, -1, 0, math_pi() / (float)2, 0, 0);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_left"), SHORTCUT_TYPE_STARTED)) {
				viewport_set_view(-1, 0, 0, math_pi() / (float)2, 0, -math_pi() / (float)2);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_right"), SHORTCUT_TYPE_STARTED)) {
				viewport_set_view(1, 0, 0, math_pi() / (float)2, 0, math_pi() / (float)2);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_bottom"), SHORTCUT_TYPE_STARTED)) {
				viewport_set_view(0, 0, -1, math_pi(), 0, math_pi());
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_camera_type"), SHORTCUT_TYPE_STARTED)) {
				context_raw->camera_type   = context_raw->camera_type == CAMERA_TYPE_PERSPECTIVE ? CAMERA_TYPE_ORTHOGRAPHIC : CAMERA_TYPE_PERSPECTIVE;
				context_raw->cam_handle->i = context_raw->camera_type;
				viewport_update_camera_type(context_raw->camera_type);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_orbit_left"), SHORTCUT_TYPE_REPEAT)) {
				viewport_orbit(-math_pi() / (float)12, 0);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_orbit_right"), SHORTCUT_TYPE_REPEAT)) {
				viewport_orbit(math_pi() / (float)12, 0);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_orbit_up"), SHORTCUT_TYPE_REPEAT)) {
				viewport_orbit(0, -math_pi() / (float)12);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_orbit_down"), SHORTCUT_TYPE_REPEAT)) {
				viewport_orbit(0, math_pi() / (float)12);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_orbit_opposite"), SHORTCUT_TYPE_STARTED)) {
				viewport_orbit_opposite();
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_zoom_in"), SHORTCUT_TYPE_REPEAT)) {
				viewport_zoom(0.2);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "view_zoom_out"), SHORTCUT_TYPE_REPEAT)) {
				viewport_zoom(-0.2);
			}
			else if (operator_shortcut(any_map_get(config_keymap, "viewport_mode"), SHORTCUT_TYPE_STARTED)) {
				ui->is_key_pressed = false;
				ui_menu_draw(&ui_base_menu_draw_viewport_mode, -1, -1);
			}
		}
		if (operator_shortcut(any_map_get(config_keymap, "operator_search"), SHORTCUT_TYPE_STARTED)) {
			ui_base_operator_search();
		}
	}

	if (context_raw->brush_locked) {
		bool b = context_raw->brush_locked && !operator_shortcut(any_map_get(config_keymap, "brush_radius"), SHORTCUT_TYPE_DOWN) &&
		         !operator_shortcut(any_map_get(config_keymap, "brush_opacity"), SHORTCUT_TYPE_DOWN) &&
		         !operator_shortcut(any_map_get(config_keymap, "brush_angle"), SHORTCUT_TYPE_DOWN) &&
		         !(decal_mask && operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "brush_radius")),
		                                           SHORTCUT_TYPE_DOWN));
		if (b) {
			iron_mouse_unlock();
			context_raw->last_paint_x = -1;
			context_raw->last_paint_y = -1;
			context_raw->brush_locked = false;
		}
	}

	// Resizing
	if (ui_base_border_handle != NULL) {
		if (ui_base_border_handle == ui_nodes_hwnd || ui_base_border_handle == ui_view2d_hwnd) {
			if (ui_base_border_started == BORDER_SIDE_LEFT) {
				config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] -= math_floor(mouse_movement_x);
				if (config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] < 32) {
					config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] = 32;
				}
				else if (config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] > iron_window_width() * 0.7) {
					config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] = math_floor(iron_window_width() * 0.7);
				}
			}
			else { // UINodes / UIView2D ratio
				config_raw->layout->buffer[LAYOUT_SIZE_NODES_H] -= math_floor(mouse_movement_y);
				if (config_raw->layout->buffer[LAYOUT_SIZE_NODES_H] < 32) {
					config_raw->layout->buffer[LAYOUT_SIZE_NODES_H] = 32;
				}
				else if (config_raw->layout->buffer[LAYOUT_SIZE_NODES_H] > sys_h() * 0.95) {
					config_raw->layout->buffer[LAYOUT_SIZE_NODES_H] = math_floor(sys_h() * 0.95);
				}
			}
		}
		else if (ui_base_border_handle == ui_base_hwnds->buffer[TAB_AREA_STATUS]) {
			i32 my = math_floor(mouse_movement_y);
			if (config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] - my >= ui_statusbar_default_h * config_raw->window_scale &&
			    config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] - my < iron_window_height() * 0.7) {
				config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] -= my;
			}
		}
		else { // Sidebar
			if (ui_base_border_started == BORDER_SIDE_LEFT) {
				config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] -= math_floor(mouse_movement_x);
				if (config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] < ui_sidebar_w_mini) {
					config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] = ui_sidebar_w_mini;
				}
				else if (config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] > iron_window_width() - ui_sidebar_w_mini * 2) {
					config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] = iron_window_width() - ui_sidebar_w_mini * 2;
				}

				if (ui_nodes_show || ui_base_show) {
					// Scale down node view if viewport is already at minimal size
					if (config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] + config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] + ui_sidebar_w_mini >
					    iron_window_width()) {
						config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] =
						    iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - ui_sidebar_w_mini;
					}
				}
			}
			else {
				i32 my = math_floor(mouse_movement_y);
				if (ui_base_border_handle == ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1] && ui_base_border_started == BORDER_SIDE_TOP) {
					if (config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0] + my > 32 && config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H1] - my > 32) {
						config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0] += my;
						config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H1] -= my;
					}
				}
			}
		}
		base_resize();
	}

	if (!mouse_down("left")) {
		gc_unroot(ui_base_border_handle);
		ui_base_border_handle = NULL;
		base_is_resizing      = false;
	}

	if (context_raw->tool == TOOL_TYPE_PARTICLE && context_in_paint_area() && !context_raw->paint2d) {
		util_particle_init_physics();
		physics_world_t *world = physics_world_active;
		physics_world_update(world);
		context_raw->ddirty = 2;
		context_raw->rdirty = 2;
		if (mouse_started("left")) {
			if (context_raw->particle_timer != NULL) {
				tween_stop(context_raw->particle_timer);
				tween_anim_t *timer = context_raw->particle_timer;
				timer->done(timer->done_data);
				context_raw->particle_timer = NULL;
			}
			history_push_undo           = true;
			context_raw->particle_hit_x = context_raw->particle_hit_y = context_raw->particle_hit_z = 0;
			object_t      *o                                                                        = scene_spawn_object(".Sphere", NULL, true);
			mesh_object_t *mo                                                                       = o->ext;
			mo->base->name                                                                          = ".Bullet";
			mo->base->visible                                                                       = true;

			camera_object_t *camera    = scene_camera;
			transform_t     *ct        = camera->base->transform;
			mo->base->transform->loc   = vec4_create(transform_world_x(ct), transform_world_y(ct), transform_world_z(ct), 1.0);
			mo->base->transform->scale = vec4_create(context_raw->brush_radius * 0.2, context_raw->brush_radius * 0.2, context_raw->brush_radius * 0.2, 1.0);
			transform_build_matrix(mo->base->transform);

			physics_body_t *body = physics_body_create();
			body->shape          = PHYSICS_SHAPE_SPHERE;
			body->mass           = 1.0;
			physics_body_init(body, mo->base);

			ray_t *ray = raycast_get_ray(mouse_view_x(), mouse_view_y(), camera);
			physics_body_apply_impulse(body, vec4_mult(ray->dir, 0.15));

			context_raw->particle_timer = tween_timer(5, &mesh_object_remove, mo);
		}

#ifdef arm_physics

		physics_pair_t_array_t *pairs = physics_world_get_contact_pairs(world, context_raw->paint_body);
		if (pairs != NULL) {
			for (i32 i = 0; i < pairs->length; ++i) {
				physics_pair_t *p                = pairs->buffer[i];
				context_raw->last_particle_hit_x = context_raw->particle_hit_x != 0 ? context_raw->particle_hit_x : p->pos_a_x;
				context_raw->last_particle_hit_y = context_raw->particle_hit_y != 0 ? context_raw->particle_hit_y : p->pos_a_y;
				context_raw->last_particle_hit_z = context_raw->particle_hit_z != 0 ? context_raw->particle_hit_z : p->pos_a_z;
				context_raw->particle_hit_x      = p->pos_a_x;
				context_raw->particle_hit_y      = p->pos_a_y;
				context_raw->particle_hit_z      = p->pos_a_z;
				context_raw->pdirty              = 1;
				break; // 1 pair for now
			}
		}

#endif
	}
}

void ui_base_view_top() {
	bool is_typing = ui->is_typing;

	if (context_in_paint_area() && !is_typing) {
		if (mouse_view_x() < sys_w()) {
			viewport_set_view(0, 0, 1, 0, 0, 0);
		}
	}
}

void ui_base_operator_search_menu_draw() {
	ui_menu_h                  = UI_ELEMENT_H() * 8;
	ui_handle_t *search_handle = ui_handle(__ID__);
	char        *search        = ui_text_input(search_handle, "", UI_ALIGN_LEFT, true, true);
	ui->changed                = false;
	if (_ui_base_operator_search_first) {
		_ui_base_operator_search_first = false;
		search_handle->text            = "";
		ui_start_text_edit(search_handle, UI_ALIGN_LEFT); // Focus search bar
	}

	if (search_handle->changed) {
		ui_base_operator_search_offset = 0;
	}

	if (ui->is_key_pressed) { // Move selection
		if (ui->key_code == KEY_CODE_DOWN && ui_base_operator_search_offset < 6) {
			ui_base_operator_search_offset++;
		}
		if (ui->key_code == KEY_CODE_UP && ui_base_operator_search_offset > 0) {
			ui_base_operator_search_offset--;
		}
	}
	bool enter      = keyboard_down("enter");
	i32  count      = 0;
	i32  BUTTON_COL = ui->ops->theme->BUTTON_COL;

	string_t_array_t *keys = map_keys(config_keymap);
	for (i32 i = 0; i < keys->length; ++i) {
		char *n = keys->buffer[i];
		if (string_index_of(n, search) >= 0) {
			ui->ops->theme->BUTTON_COL = count == ui_base_operator_search_offset ? ui->ops->theme->HIGHLIGHT_COL : ui->ops->theme->SEPARATOR_COL;
			if (ui_button(n, UI_ALIGN_LEFT, any_map_get(config_keymap, n)) || (enter && count == ui_base_operator_search_offset)) {
				if (enter) {
					ui->changed = true;
					count       = 6; // Trigger break
				}
				operator_run(n);
			}
			if (++count > 6) {
				break;
			}
		}
	}

	if (enter && count == 0) { // Hide popup on enter when command is not found
		ui->changed         = true;
		search_handle->text = "";
	}
	ui->ops->theme->BUTTON_COL = BUTTON_COL;
}

void ui_base_operator_search() {
	_ui_base_operator_search_first = true;
	ui_menu_draw(&ui_base_operator_search_menu_draw, -1, -1);
}

void ui_base_toggle_distract_free() {
	ui_base_show = !ui_base_show;
	base_resize();
}

f32 ui_base_get_radius_increment() {
	return 0.1;
}

bool ui_base_hit_rect(f32 mx, f32 my, i32 x, i32 y, i32 w, i32 h) {
	return mx > x && mx < x + w && my > y && my < y + h;
}

rect_t *ui_base_get_brush_stencil_rect() {
	i32 w =
	    math_floor(context_raw->brush_stencil_image->width * (base_h() / (float)context_raw->brush_stencil_image->height) * context_raw->brush_stencil_scale);
	i32     h = math_floor(base_h() * context_raw->brush_stencil_scale);
	i32     x = math_floor(base_x() + context_raw->brush_stencil_x * base_w());
	i32     y = math_floor(base_y() + context_raw->brush_stencil_y * base_h());
	rect_t *r = GC_ALLOC_INIT(rect_t, {.w = w, .h = h, .x = x, .y = y});
	return r;
}

void ui_base_update_ui_on_next_frame(void *_) {
	layers_update_fill_layer(true);
	make_material_parse_paint_material(false);
}

void ui_base_update_ui() {
	if (console_message_timer > 0) {
		console_message_timer -= sys_delta();
		if (console_message_timer <= 0) {
			ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
		}
	}

	ui_sidebar_w_mini = math_floor(ui_sidebar_default_w_mini * UI_SCALE());

	if (!base_ui_enabled) {
		return;
	}

	// Same mapping for paint and rotate (predefined in touch keymap)
	if (context_in_3d_view()) {
		char *paint_key  = any_map_get(config_keymap, "action_paint");
		char *rotate_key = any_map_get(config_keymap, "action_rotate");
		if (mouse_started("left") && string_equals(paint_key, rotate_key)) {
			gc_unroot(ui_base_action_paint_remap);
			ui_base_action_paint_remap = string_copy(paint_key);
			gc_root(ui_base_action_paint_remap);
			util_render_pick_pos_nor_tex();
			bool is_mesh = math_abs(context_raw->posx_picked) < 50 && math_abs(context_raw->posy_picked) < 50 && math_abs(context_raw->posz_picked) < 50;
#ifdef IRON_ANDROID
			// Allow rotating with both pen and touch, because hovering a pen prevents touch input on android
			bool pen_only = false;
#else
			bool pen_only = context_raw->pen_painting_only;
#endif
			bool is_pen = pen_only && pen_down("tip");
			// Mesh picked - disable rotate
			// Pen painting only - rotate with touch, paint with pen
			if ((is_mesh && !pen_only) || is_pen) {
				any_map_set(config_keymap, "action_rotate", "");
				any_map_set(config_keymap, "action_paint", ui_base_action_paint_remap);
			}
			// World sphere picked - disable paint
			else {
				any_map_set(config_keymap, "action_paint", "");
				any_map_set(config_keymap, "action_rotate", ui_base_action_paint_remap);
			}
		}
		else if (!mouse_down("left") && !string_equals(ui_base_action_paint_remap, "")) {
			any_map_set(config_keymap, "action_rotate", ui_base_action_paint_remap);
			any_map_set(config_keymap, "action_paint", ui_base_action_paint_remap);
			gc_unroot(ui_base_action_paint_remap);
			ui_base_action_paint_remap = "";
			gc_root(ui_base_action_paint_remap);
		}
	}

	if (context_raw->brush_stencil_image != NULL && operator_shortcut(any_map_get(config_keymap, "stencil_transform"), SHORTCUT_TYPE_DOWN)) {
		rect_t *r = ui_base_get_brush_stencil_rect();
		if (mouse_started("left")) {
			context_raw->brush_stencil_scaling = ui_base_hit_rect(mouse_x, mouse_y, r->x - 8, r->y - 8, 16, 16) ||
			                                     ui_base_hit_rect(mouse_x, mouse_y, r->x - 8, r->h + r->y - 8, 16, 16) ||
			                                     ui_base_hit_rect(mouse_x, mouse_y, r->w + r->x - 8, r->y - 8, 16, 16) ||
			                                     ui_base_hit_rect(mouse_x, mouse_y, r->w + r->x - 8, r->h + r->y - 8, 16, 16);
			f32 cosa = math_cos(-context_raw->brush_stencil_angle);
			f32 sina = math_sin(-context_raw->brush_stencil_angle);
			f32 ox   = 0;
			f32 oy   = -r->h / (float)2;
			f32 x    = ox * cosa - oy * sina;
			f32 y    = ox * sina + oy * cosa;
			x += r->x + r->w / (float)2;
			y += r->y + r->h / (float)2;
			context_raw->brush_stencil_rotating = ui_base_hit_rect(mouse_x, mouse_y, math_floor(x - 16), math_floor(y - 16), 32, 32);
		}
		f32 _scale = context_raw->brush_stencil_scale;
		if (mouse_down("left")) {
			if (context_raw->brush_stencil_scaling) {
				i32 mult = mouse_x > r->x + r->w / (float)2 ? 1 : -1;
				context_raw->brush_stencil_scale += mouse_movement_x / (float)400 * mult;
			}
			else if (context_raw->brush_stencil_rotating) {
				f32 gizmo_x                      = r->x + r->w / (float)2;
				f32 gizmo_y                      = r->y + r->h / (float)2;
				context_raw->brush_stencil_angle = -math_atan2(mouse_y - gizmo_y, mouse_x - gizmo_x) - math_pi() / (float)2;
			}
			else {
				context_raw->brush_stencil_x += mouse_movement_x / (float)base_w();
				context_raw->brush_stencil_y += mouse_movement_y / (float)base_h();
			}
		}
		else {
			context_raw->brush_stencil_scaling = false;
		}
		if (mouse_wheel_delta != 0) {
			context_raw->brush_stencil_scale -= mouse_wheel_delta / (float)10;
		}
		// Center after scale
		f32 ratio = base_h() / (float)context_raw->brush_stencil_image->height;
		f32 old_w = _scale * context_raw->brush_stencil_image->width * ratio;
		f32 new_w = context_raw->brush_stencil_scale * context_raw->brush_stencil_image->width * ratio;
		f32 old_h = _scale * base_h();
		f32 new_h = context_raw->brush_stencil_scale * base_h();
		context_raw->brush_stencil_x += (old_w - new_w) / (float)base_w() / (float)2;
		context_raw->brush_stencil_y += (old_h - new_h) / (float)base_h() / (float)2;
	}
	bool set_clone_source =
	    context_raw->tool == TOOL_TYPE_CLONE &&
	    operator_shortcut(string("%s+%s", any_map_get(config_keymap, "set_clone_source"), any_map_get(config_keymap, "action_paint")), SHORTCUT_TYPE_DOWN);

	bool decal_mask = context_is_decal_mask_paint();

	bool down = operator_shortcut(any_map_get(config_keymap, "action_paint"), SHORTCUT_TYPE_DOWN) || decal_mask || set_clone_source ||
	            operator_shortcut(string("%s+%s", any_map_get(config_keymap, "brush_ruler"), any_map_get(config_keymap, "action_paint")), SHORTCUT_TYPE_DOWN) ||
	            (pen_down("tip") && !keyboard_down("alt"));

	if (config_raw->touch_ui) {
		if (pen_down("tip")) {
			context_raw->pen_painting_only = true;
		}
		else if (context_raw->pen_painting_only) {
			down = false;
		}
	}

	if (context_raw->tool == TOOL_TYPE_PARTICLE) {
		down = false;
	}

	if (down) {
		i32 mx = mouse_view_x();
		i32 my = mouse_view_y();
		i32 ww = sys_w();

		if (context_raw->paint2d) {
			mx -= sys_w();
			ww = ui_view2d_ww;
		}

		if (mx < ww && mx > sys_x() && my < sys_h() && my > sys_y()) {

			if (set_clone_source) {
				context_raw->clone_start_x = mx;
				context_raw->clone_start_y = my;
			}
			else {
				if (context_raw->brush_time == 0 && !base_is_dragging && !base_is_resizing && ui->combo_selected_handle == NULL) { // Paint started

					// Draw line
					if (operator_shortcut(string("%s+%s", any_map_get(config_keymap, "brush_ruler"), any_map_get(config_keymap, "action_paint")),
					                      SHORTCUT_TYPE_DOWN)) {
						context_raw->last_paint_vec_x = context_raw->last_paint_x;
						context_raw->last_paint_vec_y = context_raw->last_paint_y;
					}

					history_push_undo = true;

					if (context_raw->tool == TOOL_TYPE_CLONE && context_raw->clone_start_x >= 0.0) { // Clone delta
						context_raw->clone_delta_x = (context_raw->clone_start_x - mx) / (float)ww;
						context_raw->clone_delta_y = (context_raw->clone_start_y - my) / (float)sys_h();
						context_raw->clone_start_x = -1;
					}
					else if (context_raw->tool == TOOL_TYPE_FILL && context_raw->fill_type_handle->i == FILL_TYPE_UV_ISLAND) {
						util_uv_uvislandmap_cached = false;
					}
				}

				context_raw->brush_time += sys_delta();

				if (context_raw->run_brush != NULL) {
					context_raw->run_brush(context_raw->brush_output_node_inst, 0);
				}
			}
		}
	}
	else if (context_raw->brush_time > 0) { // Brush released
		context_raw->brush_time       = 0;
		context_raw->prev_paint_vec_x = -1;
		context_raw->prev_paint_vec_y = -1;
		// context_raw->ddirty              = 3; // Keep accumulated samples for D3D12
		context_raw->brush_blend_dirty = true; // Update brush mask

		context_raw->layer_preview_dirty = true; // Update layer preview

		// New color id picked, update fill layer
		if (context_raw->tool == TOOL_TYPE_COLORID && context_raw->layer->fill_layer != NULL) {
			sys_notify_on_next_frame(&ui_base_update_ui_on_next_frame, NULL);
		}
	}

	if (context_raw->layers_preview_dirty) {
		context_raw->layers_preview_dirty = false;
		context_raw->layer_preview_dirty  = false;
		context_raw->mask_preview_last    = NULL;
		// Update all layer previews
		for (i32 i = 0; i < project_layers->length; ++i) {
			slot_layer_t *l = project_layers->buffer[i];
			if (slot_layer_is_group(l)) {
				continue;
			}

			gpu_texture_t *target = l->texpaint_preview;
			if (target == NULL) {
				continue;
			}

			gpu_texture_t *source = l->texpaint;
			draw_begin(target, true, 0x00000000);
			// draw_set_pipeline(l.is_mask() ? pipes_copy8 : pipes_copy);
			draw_set_pipeline(pipes_copy); // texpaint_preview is always RGBA32 for now
			draw_scaled_image(source, 0, 0, target->width, target->height);
			draw_set_pipeline(NULL);
			draw_end();
		}
		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
	}
	if (context_raw->layer != NULL && context_raw->layer_preview_dirty && !slot_layer_is_group(context_raw->layer)) {
		context_raw->layer_preview_dirty = false;
		context_raw->mask_preview_last   = NULL;
		// Update layer preview
		slot_layer_t *l = context_raw->layer;

		gpu_texture_t *target = l->texpaint_preview;
		if (target != NULL) {

			gpu_texture_t *source = l->texpaint;
			draw_begin(target, true, 0x00000000);
			// draw_set_pipeline(raw.layer.is_mask() ? pipes_copy8 : pipes_copy);
			draw_set_pipeline(pipes_copy); // texpaint_preview is always RGBA32 for now
			draw_scaled_image(source, 0, 0, target->width, target->height);
			draw_set_pipeline(NULL);
			draw_end();
			ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
		}
	}
	bool undo_pressed = operator_shortcut(any_map_get(config_keymap, "edit_undo"), SHORTCUT_TYPE_STARTED);
	bool redo_pressed =
	    operator_shortcut(any_map_get(config_keymap, "edit_redo"), SHORTCUT_TYPE_STARTED) || (keyboard_down("control") && keyboard_started("y"));

	// Two-finger tap to undo, three-finger tap to redo
	if (context_in_3d_view() && config_raw->touch_ui) {
		if (mouse_started("middle")) {
			ui_base_redo_tap_time = sys_time();
		}
		else if (mouse_started("right")) {
			ui_base_undo_tap_time = sys_time();
		}
		else if (mouse_released("middle") && sys_time() - ui_base_redo_tap_time < 0.1) {
			ui_base_redo_tap_time = ui_base_undo_tap_time = 0;
			redo_pressed                                  = true;
		}
		else if (mouse_released("right") && sys_time() - ui_base_undo_tap_time < 0.1) {
			ui_base_redo_tap_time = ui_base_undo_tap_time = 0;
			undo_pressed                                  = true;
		}
	}

	if (undo_pressed) {
		history_undo();
	}
	else if (redo_pressed) {
		history_redo();
	}

	gizmo_update();
}

void ui_base_render(void *_) {
	if (!ui_base_show && config_raw->touch_ui) {
		ui->input_enabled = true;
		ui_begin(ui);
		if (ui_window(ui_handle(__ID__), 0, 0, 150, math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET() + 1), false)) {
			if (ui_button(tr("Close"), UI_ALIGN_CENTER, "")) {
				ui_base_toggle_distract_free();
			}
		}
		ui_end();
	}

	if (!ui_base_show) {
		return;
	}

	ui->input_enabled = base_ui_enabled;

	// Remember last tab positions
	for (i32 i = 0; i < ui_base_htabs->length; ++i) {
		if (ui_base_htabs->buffer[i]->changed) {
			config_raw->layout_tabs->buffer[i] = ui_base_htabs->buffer[i]->i;
			config_save();
		}
	}

	// Set tab positions
	for (i32 i = 0; i < ui_base_htabs->length; ++i) {
		ui_base_htabs->buffer[i]->i = config_raw->layout_tabs->buffer[i];
	}

	// Nothing to display in the main area
	if (!base_view3d_show && !ui_nodes_show && !ui_view2d_show) {
		draw_begin(NULL, true, base_theme->SEPARATOR_COL);
		gpu_texture_t *img = data_get_image("badge_bw.k");
		draw_set_color(0x22ffffff);
		draw_image(img, base_view3d_w() / (float)2 - img->width / (float)2, base_h() / (float)2 - img->height / (float)2);
		draw_end();
	}
	ui_begin(ui);
	ui_toolbar_render_ui();
	ui_menubar_render_ui();
	ui_header_render_ui();
	ui_statusbar_render_ui();
	ui_sidebar_render_ui();
	ui_end();
	ui->input_enabled = true;
}

void ui_base_render_cursor(void *_) {
	if (!base_ui_enabled) {
		return;
	}

	if (context_raw->tool == TOOL_TYPE_MATERIAL || context_raw->tool == TOOL_TYPE_BAKE) {
		return;
	}

	draw_begin(NULL, false, 0);
	draw_set_color(0xffffffff);

	context_raw->view_index = context_raw->view_index_last;
	i32 mx                  = base_x() + context_raw->paint_vec.x * base_w();
	i32 my                  = base_y() + context_raw->paint_vec.y * base_h();
	context_raw->view_index = -1;

	if (context_raw->brush_stencil_image != NULL && context_raw->tool != TOOL_TYPE_PICKER && context_raw->tool != TOOL_TYPE_COLORID) {
		rect_t *r = ui_base_get_brush_stencil_rect();
		if (!operator_shortcut(any_map_get(config_keymap, "stencil_hide"), SHORTCUT_TYPE_DOWN)) {
			draw_set_color(0x88ffffff);
			f32 angle = context_raw->brush_stencil_angle;
			draw_set_transform(mat3_multmat(mat3_multmat(mat3_translation(0.5, 0.5), mat3_rotation(-angle)), mat3_translation(-0.5, -0.5)));
			draw_scaled_image(context_raw->brush_stencil_image, r->x, r->y, r->w, r->h);
			draw_set_transform(mat3_nan());
			draw_set_color(0xffffffff);
		}
		bool transform = operator_shortcut(any_map_get(config_keymap, "stencil_transform"), SHORTCUT_TYPE_DOWN);
		if (transform) {
			// Outline
			draw_rect(r->x, r->y, r->w, r->h, 1.0);
			// Scale
			draw_rect(r->x - 8, r->y - 8, 16, 16, 1.0);
			draw_rect(r->x - 8 + r->w, r->y - 8, 16, 16, 1.0);
			draw_rect(r->x - 8, r->y - 8 + r->h, 16, 16, 1.0);
			draw_rect(r->x - 8 + r->w, r->y - 8 + r->h, 16, 16, 1.0);
			// Rotate
			f32 cosa = math_cos(-context_raw->brush_stencil_angle);
			f32 sina = math_sin(-context_raw->brush_stencil_angle);
			f32 ox   = 0;
			f32 oy   = -r->h / (float)2;
			f32 x    = ox * cosa - oy * sina;
			f32 y    = ox * sina + oy * cosa;
			x += r->x + r->w / (float)2;
			y += r->y + r->h / (float)2;
			draw_filled_circle(x, y, 8, 0);
		}
	}

	// Show picked material next to cursor
	if (context_raw->tool == TOOL_TYPE_PICKER && context_raw->picker_select_material && context_raw->color_picker_callback == NULL) {
		gpu_texture_t *img = context_raw->material->image_icon;
		draw_image(img, mx + 10, my + 10);
	}
	if (context_raw->tool == TOOL_TYPE_PICKER && context_raw->color_picker_callback != NULL) {
		gpu_texture_t *img  = resource_get("icons.k");
		rect_t        *rect = resource_tile50(img, TOOL_TYPE_PICKER);
		draw_sub_image(img, mx + 10, my + 10, rect->x, rect->y, rect->w, rect->h);
	}

	gpu_texture_t *cursor_img = resource_get("cursor.k");
	i32            psize      = math_floor(182 * (context_raw->brush_radius * context_raw->brush_nodes_radius) * UI_SCALE());

	// Clone source cursor
	if (context_raw->tool == TOOL_TYPE_CLONE && !keyboard_down("alt") && (mouse_down("left") || pen_down("tip"))) {
		draw_set_color(0x66ffffff);
		draw_scaled_image(cursor_img, mx + context_raw->clone_delta_x * sys_w() - psize / (float)2,
		                  my + context_raw->clone_delta_y * sys_h() - psize / (float)2, psize, psize);
		draw_set_color(0xffffffff);
	}

	bool decal = context_is_decal();

	if (context_in_2d_view(VIEW_2D_TYPE_LAYER) || decal) {
		bool decal_mask = context_is_decal_mask();
		if (decal && !context_in_nodes()) {
			f32 decal_alpha = 0.5;
			if (!decal_mask) {
				context_raw->decal_x = context_raw->paint_vec.x;
				context_raw->decal_y = context_raw->paint_vec.y;
				decal_alpha          = context_raw->brush_opacity;
			}

			if (!config_raw->brush_live) {
				i32 psizex = math_floor(256 * UI_SCALE() * (context_raw->brush_radius * context_raw->brush_nodes_radius * context_raw->brush_scale_x));
				i32 psizey = math_floor(256 * UI_SCALE() * (context_raw->brush_radius * context_raw->brush_nodes_radius));

				context_raw->view_index = context_raw->view_index_last;
				f32 decalx              = base_x() + context_raw->decal_x * base_w() - psizex / (float)2;
				f32 decaly              = base_y() + context_raw->decal_y * base_h() - psizey / (float)2;
				context_raw->view_index = -1;

				draw_set_color(color_from_floats(1, 1, 1, decal_alpha));
				f32 angle = (context_raw->brush_angle + context_raw->brush_nodes_angle) * (math_pi() / (float)180);
				draw_set_transform(mat3_multmat(mat3_multmat(mat3_translation(0.5, 0.5), mat3_rotation(angle)), mat3_translation(-0.5, -0.5)));
				draw_scaled_image(context_raw->decal_image, decalx, decaly, psizex, psizey);
				draw_set_transform(mat3_nan());
				draw_set_color(0xffffffff);
			}
		}
		if (context_raw->tool == TOOL_TYPE_BRUSH || context_raw->tool == TOOL_TYPE_ERASER || context_raw->tool == TOOL_TYPE_CLONE ||
		    context_raw->tool == TOOL_TYPE_BLUR || context_raw->tool == TOOL_TYPE_SMUDGE || context_raw->tool == TOOL_TYPE_PARTICLE ||
		    (decal_mask && context_in_2d_view(VIEW_2D_TYPE_LAYER))) {
			if (decal_mask) {
				psize = math_floor(cursor_img->width * (context_raw->brush_decal_mask_radius * context_raw->brush_nodes_radius) * UI_SCALE());
			}
			if (context_in_2d_view(VIEW_2D_TYPE_LAYER)) {
				psize = math_floor(psize * ui_view2d_pan_scale);
			}
			draw_scaled_image(cursor_img, mx - psize / (float)2, my - psize / (float)2, psize, psize);
		}
	}

	if (context_raw->brush_lazy_radius > 0 && !context_raw->brush_locked &&
	    (context_raw->tool == TOOL_TYPE_BRUSH || context_raw->tool == TOOL_TYPE_ERASER || context_raw->tool == TOOL_TYPE_DECAL ||
	     context_raw->tool == TOOL_TYPE_TEXT || context_raw->tool == TOOL_TYPE_CLONE || context_raw->tool == TOOL_TYPE_BLUR ||
	     context_raw->tool == TOOL_TYPE_SMUDGE || context_raw->tool == TOOL_TYPE_PARTICLE)) {
		draw_filled_rect(mx - 1, my - 1, 2, 2);
		mx         = context_raw->brush_lazy_x * base_w() + base_x();
		my         = context_raw->brush_lazy_y * base_h() + base_y();
		f32 radius = context_raw->brush_lazy_radius * 180;
		draw_set_color(0xff666666);
		draw_scaled_image(cursor_img, mx - radius / (float)2, my - radius / (float)2, radius, radius);
		draw_set_color(0xffffffff);
	}
	draw_end();
}

void ui_base_show_material_nodes() {
	// Clear input state as ui receives input events even when not drawn
	ui_end_input();

	ui_nodes_show        = !ui_nodes_show || ui_nodes_canvas_type != CANVAS_TYPE_MATERIAL;
	ui_nodes_canvas_type = CANVAS_TYPE_MATERIAL;

	if (config_raw->touch_ui && ui_view2d_show && base_view3d_show) {
		ui_view2d_show = false;
	}

	if (config_raw->touch_ui && ui_nodes_show && iron_window_width() < iron_window_height()) {
		ui_view2d_show   = false;
		base_view3d_show = false;
	}

	if (config_raw->touch_ui && !ui_nodes_show && iron_window_width() < iron_window_height()) {
		base_view3d_show = true;
	}

	base_resize();
}

void ui_base_show_brush_nodes() {
	// Clear input state as ui receives input events even when not drawn
	ui_end_input();
	ui_nodes_show        = !ui_nodes_show || ui_nodes_canvas_type != CANVAS_TYPE_BRUSH;
	ui_nodes_canvas_type = CANVAS_TYPE_BRUSH;

	if (config_raw->touch_ui && ui_view2d_show && base_view3d_show) {
		ui_view2d_show = false;
	}

	if (config_raw->touch_ui && ui_nodes_show && iron_window_width() < iron_window_height()) {
		ui_view2d_show   = false;
		base_view3d_show = false;
	}

	if (config_raw->touch_ui && !ui_nodes_show && iron_window_width() < iron_window_height()) {
		base_view3d_show = true;
	}

	base_resize();
}

void ui_base_show_2d_view(view_2d_type_t type) {
	// Clear input state as ui receives input events even when not drawn
	ui_end_input();
	if (ui_view2d_type != type) {
		ui_view2d_show = true;
	}
	else {
		ui_view2d_show = !ui_view2d_show;
	}
	ui_view2d_type          = type;
	ui_view2d_hwnd->redraws = 2;

	if (config_raw->touch_ui && ui_nodes_show && base_view3d_show) {
		ui_nodes_show = false;
	}

	if (config_raw->touch_ui && ui_view2d_show && iron_window_width() < iron_window_height()) {
		ui_nodes_show    = false;
		base_view3d_show = false;
	}

	if (config_raw->touch_ui && !ui_view2d_show && iron_window_width() < iron_window_height()) {
		base_view3d_show = true;
	}

	base_resize();
}

void ui_base_show_3d_view() {
	if (!base_view3d_show) {
		if (config_raw->touch_ui && ui_nodes_show && ui_view2d_show) {
			ui_view2d_show = false;
		}
		if (config_raw->touch_ui && (ui_nodes_show || ui_view2d_show) && iron_window_width() < iron_window_height()) {
			ui_nodes_show  = false;
			ui_view2d_show = false;
		}
	}

	base_view3d_show = !base_view3d_show;
	base_resize();
}

void ui_base_toggle_browser() {
	bool minimized                                   = config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] <= (ui_statusbar_default_h * config_raw->window_scale);
	config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] = minimized ? 240 : ui_statusbar_default_h;
	config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] = math_floor(config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] * config_raw->window_scale);
	base_resize();
}

void ui_base_set_icon_scale() {
	if (UI_SCALE() > 1) {
		string_t_array_t *res = any_array_create_from_raw(
		    (void *[]){
		        "icons.k",
		        "icons05x.k",
		        "icons2x.k",
		    },
		    3);
		resource_load(res);
		any_map_set(resource_bundled, "icons05x.k", resource_get("icons.k"));
		any_map_set(resource_bundled, "icons.k", resource_get("icons2x.k"));
	}
	else {
		string_t_array_t *res = any_array_create_from_raw(
		    (void *[]){
		        "icons.k",
		        "icons05x.k",
		    },
		    2);
		resource_load(res);
	}
}

void ui_base_on_border_hover(ui_handle_t *handle, i32 side) {
	if (!base_ui_enabled) {
		return;
	}

	if (handle != ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0] && handle != ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1] &&
	    handle != ui_base_hwnds->buffer[TAB_AREA_STATUS] && handle != ui_nodes_hwnd && handle != ui_view2d_hwnd) {
		return; // Scalable handles
	}
	if (handle == ui_view2d_hwnd && side != BORDER_SIDE_LEFT) {
		return;
	}
	if (handle == ui_nodes_hwnd && side == BORDER_SIDE_TOP && !ui_view2d_show) {
		return;
	}
	if (handle == ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0] && side == BORDER_SIDE_TOP) {
		return;
	}

	if (handle == ui_nodes_hwnd && side != BORDER_SIDE_LEFT && side != BORDER_SIDE_TOP) {
		return;
	}
	if (handle == ui_base_hwnds->buffer[TAB_AREA_STATUS] && side != BORDER_SIDE_TOP) {
		return;
	}
	if (side == BORDER_SIDE_RIGHT) {
		return; // UI is snapped to the right side
	}

	side == BORDER_SIDE_LEFT || side == BORDER_SIDE_RIGHT ? iron_mouse_set_cursor(IRON_CURSOR_SIZEWE) : iron_mouse_set_cursor(IRON_CURSOR_SIZENS);

	if (ui->input_started) {
		ui_base_border_started = side;
		gc_unroot(ui_base_border_handle);
		ui_base_border_handle = handle;
		gc_root(ui_base_border_handle);
		base_is_resizing = true;
	}
}

void ui_base_on_tab_drop(ui_handle_t *to, i32 to_position, ui_handle_t *from, i32 from_position) {
	i32 i = -1;
	i32 j = -1;
	for (i32 k = 0; k < ui_base_htabs->length; ++k) {
		if (ui_base_htabs->buffer[k] == to) {
			i = k;
		}
		if (ui_base_htabs->buffer[k] == from) {
			j = k;
		}
	}
	if (i == j && to_position == from_position) {
		return;
	}
	if (i > -1 && j > -1) {
		tab_draw_t_array_t *tabsi = ui_base_hwnd_tabs->buffer[i];
		tab_draw_t_array_t *tabsj = ui_base_hwnd_tabs->buffer[j];
		if (tabsj->length == 1) {
			return; // Keep at least one tab in place
		}
		tab_draw_t *element = tabsj->buffer[from_position];
		array_splice(tabsj, from_position, 1);
		array_insert(tabsi, to_position, element);
		ui_base_hwnds->buffer[i]->redraws = 2;
		ui_base_hwnds->buffer[j]->redraws = 2;
	}
}

void base_redraw_ui() {
	ui_header_handle->redraws                       = 2;
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
	ui_menubar_menu_handle->redraws                 = 2;
	ui_menubar_hwnd->redraws                        = 2;
	ui_nodes_hwnd->redraws                          = 2;
	ui_box_hwnd->redraws                            = 2;
	ui_view2d_hwnd->redraws                         = 2;
	// Redraw viewport
	if (context_raw->ddirty < 0) {
		context_raw->ddirty = 0;
	}
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
	ui_toolbar_handle->redraws                        = 2;
	if (context_raw->split_view) {
		context_raw->ddirty = 1;
	}
}

void ui_base_make_empty_envmap(i32 col) {
	ui_base_viewport_col      = col;
	u8_array_t *b             = u8_array_create(4);
	b->buffer[0]              = color_get_rb(col);
	b->buffer[1]              = color_get_gb(col);
	b->buffer[2]              = color_get_bb(col);
	b->buffer[3]              = 255;
	context_raw->empty_envmap = gpu_create_texture_from_bytes(b, 1, 1, GPU_TEXTURE_FORMAT_RGBA32);
}

void ui_base_set_viewport_col(i32 col) {
	ui_base_make_empty_envmap(col);
	context_raw->ddirty = 2;
	if (!context_raw->show_envmap) {
		scene_world->_->envmap = context_raw->empty_envmap;
	}
}

void base_update_workspace() {
	config_init_layout();

	if (config_raw->workspace == WORKSPACE_PAINT_3D || config_raw->workspace == WORKSPACE_SCULPT) {
		base_view3d_show  = true;
		ui_menubar_tab->i = 0;
		ui_view2d_show    = false;
		ui_nodes_show     = false;
	}
	else if (config_raw->workspace == WORKSPACE_PAINT_2D) {
		base_view3d_show  = false;
		ui_menubar_tab->i = -1;
		ui_view2d_show    = true;
		ui_nodes_show     = false;
	}
	else if (config_raw->workspace == WORKSPACE_NODES) {
		base_view3d_show  = false;
		ui_menubar_tab->i = -1;
		ui_view2d_show    = false;
		ui_nodes_show     = true;

		ui_sidebar_show(false);
	}
	else if (config_raw->workspace == WORKSPACE_SCRIPT) {
		base_view3d_show  = true;
		ui_menubar_tab->i = 0;
		ui_view2d_show    = false;
		ui_nodes_show     = false;

		ui_base_htabs->buffer[TAB_AREA_STATUS]->i          = 5; // Console
		config_raw->layout_tabs->buffer[TAB_AREA_STATUS]   = 5;
		ui_base_htabs->buffer[TAB_AREA_SIDEBAR0]->i        = 2; // Script
		config_raw->layout_tabs->buffer[TAB_AREA_SIDEBAR0] = 2;

		config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H]   = iron_window_height() * 0.3;
		config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W]  = iron_window_width() * 0.52;
		float h = UI_ELEMENT_H() + UI_ELEMENT_OFFSET() + 2;
		config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0] = iron_window_height() - h;
		config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H1] = h;
	}

	base_resize();
}

void base_update_workflow() {
	// Update Material Output nodes
	for (i32 i = 0; i < project_materials->length; ++i) {
		ui_node_array_t *nodes = project_materials->buffer[i]->canvas->nodes;
		for (i32 j = 0; j < nodes->length; ++j) {
			if (string_equals(nodes->buffer[j]->type, "OUTPUT_MATERIAL_PBR")) {
				nodes->buffer[j]->inputs->length = config_raw->workflow == WORKFLOW_BASE ? 2 : 9;
			}
		}
	}
}
