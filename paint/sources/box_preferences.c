void box_preferences_interface_tab() {
	if (box_preferences_locales == null) {
		gc_unroot(box_preferences_locales);
		box_preferences_locales = translator_get_supported_locales();
		gc_root(box_preferences_locales);
	}

	ui_handle_t *h_locale = ui_handle(__ID__);
	h_locale->i           = char_ptr_array_index_of(box_preferences_locales, config_raw->locale);
	ui_combo(h_locale, box_preferences_locales, tr("Language", null), true, UI_ALIGN_LEFT, true);
	if (h_locale->changed) {
		string_t *locale_code = box_preferences_locales->buffer[h_locale->i];
		config_raw->locale    = string_copy(locale_code);
		translator_load_translations(locale_code);
		base_redraw_ui();
	}

	ui_handle_t *h_scale = ui_handle(__ID__);
	if (h_scale->init) {
		h_scale->f = config_raw->window_scale;
	}
	ui_slider(h_scale, tr("UI Scale", null), 1.0, 4.0, true, 10, true, UI_ALIGN_RIGHT, true);
	if (context_raw->hscale_was_changed && !ui->input_down) {
		context_raw->hscale_was_changed = false;
		if (h_scale->f == 0.0) {
			h_scale->f = 1.0;
		}
		config_raw->window_scale = h_scale->f;
		box_preferences_set_scale();
	}
	if (h_scale->changed) {
		context_raw->hscale_was_changed = true;
	}

	ui_handle_t *h_node_previews = ui_handle(__ID__);
	h_node_previews->b           = config_raw->node_previews;
	config_raw->node_previews    = ui_check(h_node_previews, tr("Node Previews", null), "");
	if (h_node_previews->changed) {
		for (i32 i = 0; i < project_materials->length; ++i) {
			ui_node_canvas_t *c = project_materials->buffer[i]->canvas;
			for (i32 j = 0; j < c->nodes->length; ++j) {
				ui_node_t *n = c->nodes->buffer[j];
				if (config_raw->node_previews) {
					n->flags |= UI_NODE_FLAG_PREVIEW;
				}
				else {
					n->flags &= ~UI_NODE_FLAG_PREVIEW;
				}
			}
		}
		ui_nodes_hwnd->redraws = 2;
	}
	if (ui->is_hovered) {
		ui_tooltip(tr("Show node preview on each node by default", null));
	}

	ui_handle_t *h_wrap_mouse = ui_handle(__ID__);
	h_wrap_mouse->b           = config_raw->wrap_mouse;
	config_raw->wrap_mouse    = ui_check(h_wrap_mouse, tr("Wrap Mouse", null), "");
	if (ui->is_hovered) {
		ui_tooltip(tr("Wrap mouse around view boundaries during camera control", null));
	}

	ui->changed                     = false;
	ui_handle_t *h_show_asset_names = ui_handle(__ID__);
	h_show_asset_names->b           = config_raw->show_asset_names;
	config_raw->show_asset_names    = ui_check(h_show_asset_names, tr("Show Asset Names", null), "");
	if (ui->changed) {
		base_redraw_ui();
	}

	ui->changed             = false;
	ui_handle_t *h_touch_ui = ui_handle(__ID__);
	h_touch_ui->b           = config_raw->touch_ui;
	config_raw->touch_ui    = ui_check(h_touch_ui, tr("Touch UI", null), "");
	if (ui->changed) {
		ui_touch_control = config_raw->touch_ui;
		config_load_theme(config_raw->theme, true);
		box_preferences_set_scale();
		base_redraw_ui();
		context_raw->hscale_was_changed = true;
	}

	ui_handle_t *h_splash_screen = ui_handle(__ID__);
	h_splash_screen->b           = config_raw->splash_screen;
	config_raw->splash_screen    = ui_check(h_splash_screen, tr("Splash Screen", null), "");

	ui_handle_t *h_grid_snap     = ui_handle(__ID__);
	h_grid_snap->b               = config_raw->grid_snap;
	config_raw->grid_snap        = ui_check(h_grid_snap, tr("Grid Snap", null), "");
	ui_nodes_grid_snap           = config_raw->grid_snap;

	ui_handle_t *h_experimental  = ui_handle(__ID__);
	h_experimental->b            = config_raw->experimental;
	config_raw->experimental     = ui_check(h_experimental, tr("Experimental Features", null), "");

	ui_end_element();

	ui_row2();
	if (ui_icon_button(tr("Restore", null), ICON_REPLAY, UI_ALIGN_CENTER) && !ui_menu_show) {
		ui_menu_draw(&box_preferences_interface_tab_170050, -1, -1);
	}
	if (ui_button(tr("Reset Layout", null), UI_ALIGN_CENTER, "") && !ui_menu_show) {
		ui_menu_draw(&box_preferences_interface_tab_170267, -1, -1);
	}
}

void box_preferences_interface_tab_170267() {
	if (ui_menu_button(tr("Confirm", null), "", ICON_CHECK)) {
		config_init_layout();
		config_save();
	}
}

void box_preferences_interface_tab_170210(config_t *raw) {
	ui->ops->theme->ELEMENT_H = base_default_element_h;
	config_import_from(raw);
	box_preferences_set_scale();
	make_material_parse_mesh_material();
	make_material_parse_paint_material(true);
}

void box_preferences_interface_tab_170178(string_t *path) {
	buffer_t *b   = data_get_blob(path);
	config_t *raw = json_parse(sys_buffer_to_string(b));
	sys_notify_on_next_frame(&box_preferences_interface_tab_170210, raw);
}

void box_preferences_interface_tab_170071(any _) {
	ui->ops->theme->ELEMENT_H = base_default_element_h;
	config_restore();
	box_preferences_set_scale();
	if (config_raw->plugins != null) {
		for (i32 i = 0; i < config_raw->plugins->length; ++i) {
			string_t *f = config_raw->plugins->buffer[i];
			plugin_stop(f);
		}
	}
	gc_unroot(box_preferences_files_plugin);
	box_preferences_files_plugin = null;
	gc_unroot(box_preferences_files_keymap);
	box_preferences_files_keymap = null;
	make_material_parse_mesh_material();
	make_material_parse_paint_material(true);
	ui_base_set_viewport_col(ui->ops->theme->VIEWPORT_COL);
}

void box_preferences_interface_tab_170050() {
	if (ui_menu_button(tr("Confirm", null), "", ICON_CHECK)) {
		sys_notify_on_next_frame(&box_preferences_interface_tab_170071, null);
	}
	if (ui_menu_button(tr("Import...", null), "", ICON_IMPORT)) {
		ui_files_show("json", false, false, &box_preferences_interface_tab_170178);
	}
}

void box_preferences_theme_tab() {
	if (box_preferences_themes == null) {
		box_preferences_fetch_themes();
	}

	ui_begin_sticky();
	ui_row4();

	gc_unroot(box_preferences_h_theme);
	box_preferences_h_theme = ui_handle(__ID__);
	gc_root(box_preferences_h_theme);
	box_preferences_h_theme->i = box_preferences_get_theme_index();
	ui_combo(box_preferences_h_theme, box_preferences_themes, tr("Theme", null), false, UI_ALIGN_LEFT, true);
	if (box_preferences_h_theme->changed) {
		config_raw->theme = string_join(box_preferences_themes->buffer[box_preferences_h_theme->i], ".json");
		config_load_theme(config_raw->theme, true);
	}

	if (ui_icon_button(tr("New", null), ICON_PLUS, UI_ALIGN_CENTER)) {
		ui_box_show_custom(&box_preferences_theme_tab_170385, 400, 200, null, true, tr("New Theme", null));
	}

	if (ui_icon_button(tr("Import", null), ICON_IMPORT, UI_ALIGN_CENTER)) {
		ui_files_show("json", false, false, &box_preferences_theme_tab_170563);
	}

	if (ui_icon_button(tr("Export", null), ICON_EXPORT, UI_ALIGN_CENTER)) {
		ui_files_show("json", true, false, &box_preferences_theme_tab_170600);
	}

	ui_end_sticky();

	// Theme fields
	ui_handle_t *h_list    = ui_handle(__ID__);
	u32_ptr      u32_theme = base_theme;
	ui->input_enabled      = !ui_menu_show;
	for (i32 i = 0; i < ui_theme_keys_count; ++i) {
		string_t    *key    = ARRAY_ACCESS(ui_theme_keys, i);
		ui_handle_t *h      = ui_nest(h_list, i);
		u32          val    = DEREFERENCE(u32_theme + i);
		bool         is_hex = ends_with(key, "_COL");

		if (is_hex) {
			f32_array_t *row = f32_array_create_from_raw(
			    (f32[]){
			        1 / (float)8,
			        7 / (float)8,
			    },
			    2);
			ui_row(row);
			ui_text("", 0, val);
			if (ui->is_hovered && ui->input_released) {
				h->color = val;
				gc_unroot(_box_preferences_h);
				_box_preferences_h = h;
				gc_root(_box_preferences_h);
				_box_preferences_i = i;
				ui_menu_draw(&box_preferences_theme_tab_170797, -1, -1);
			}

			if (string_equals(key, "VIEWPORT_COL") && ui_base_viewport_col != val) {
				ui_base_set_viewport_col(val);
			}
		}

		ui->changed = false;

		if (string_equals(key, "FILL_WINDOW_BG") || string_equals(key, "FILL_BUTTON_BG") || string_equals(key, "FULL_TABS") ||
		    string_equals(key, "ROUND_CORNERS") || string_equals(key, "SHADOWS")) {
			h->b                       = val > 0;
			bool b                     = ui_check(h, key, "");
			DEREFERENCE(u32_theme + i) = b;
		}
		else if (string_equals(key, "LINK_STYLE")) {
			string_t_array_t *styles = any_array_create_from_raw(
			    (any[]){
			        tr("Straight", null),
			        tr("Curved", null),
			    },
			    2);
			h->i                       = val;
			i32 pos                    = ui_combo(h, styles, key, true, UI_ALIGN_LEFT, true);
			DEREFERENCE(u32_theme + i) = pos;
		}
		else {
			h->text = is_hex ? i32_to_string_hex(val) : i32_to_string(val);
			ui_text_input(h, key, UI_ALIGN_LEFT, true, false);
			if (is_hex) {
				DEREFERENCE(u32_theme + i) = parse_int_hex(h->text);
			}
			else {
				DEREFERENCE(u32_theme + i) = parse_int(h->text);
			}
		}
		if (ui->changed) {
			ui->elements_baked = false;
		}
	}
	ui->input_enabled = true;
}

void box_preferences_theme_tab_170797() {
	ui->changed                                 = false;
	i32     color                               = ui_color_wheel(_box_preferences_h, false, -1, 11 * ui->ops->theme->ELEMENT_H * UI_SCALE(), true, null, null);
	u32_ptr u32_theme                           = base_theme;
	DEREFERENCE(u32_theme + _box_preferences_i) = color;
	if (ui->changed) {
		ui_menu_keep_open = true;
	}
}

void box_preferences_theme_tab_170600(string_t *path) {
	path = string_join(path, PATH_SEP);
	path = string_join(path, ui_files_filename);
	if (!ends_with(path, ".json")) {
		path = string_join(path, ".json");
	}
	iron_file_save_bytes(path, sys_string_to_buffer(box_preferences_theme_to_json(base_theme)), 0);
}

void box_preferences_theme_tab_170563(string_t *path) {
	import_theme_run(path);
}

void box_preferences_theme_tab_170385() {
	ui_row2();
	ui_handle_t *h = ui_handle(__ID__);
	if (h->init) {
		h->text = "new_theme";
	}
	string_t *theme_name = ui_text_input(h, tr("Name", null), UI_ALIGN_LEFT, true, false);
	if (ui_icon_button(tr("OK", null), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {
		string_t *template = box_preferences_theme_to_json(base_theme);
		if (!ends_with(theme_name, ".json")) {
			theme_name = string_join(theme_name, ".json");
		}
		string_t *path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "themes"), PATH_SEP), theme_name);
		iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
		box_preferences_fetch_themes(); // Refresh file list
		config_raw->theme          = string_copy(theme_name);
		box_preferences_h_theme->i = box_preferences_get_theme_index();
		ui_box_hide();
		box_preferences_htab->i = 1; // Themes
		box_preferences_show();
	}
}

void box_preferences_usage_tab() {
	ui_handle_t *h_undo    = ui_handle(__ID__);
	h_undo->f              = config_raw->undo_steps;
	config_raw->undo_steps = ui_slider(h_undo, tr("Undo Steps", null), 1, 64, false, 1, true, UI_ALIGN_RIGHT, true);
	if (config_raw->undo_steps < 1) {
		config_raw->undo_steps = h_undo->f = 1;
	}
	if (h_undo->changed) {
		gpu_texture_t *current = _draw_current;
		draw_end();

		if (history_undo_layers != null) {
			while (history_undo_layers->length < config_raw->undo_steps) {
				i32           len = history_undo_layers->length;
				slot_layer_t *l   = slot_layer_create(string_join("_undo", i32_to_string(len)), LAYER_SLOT_TYPE_LAYER, null);
				any_array_push(history_undo_layers, l);
			}
			while (history_undo_layers->length > config_raw->undo_steps) {
				slot_layer_t *l = array_pop(history_undo_layers);
				slot_layer_unload(l);
			}
		}

		history_reset();
		draw_begin(current, false, 0);
	}

	ui_handle_t *h_dilate_radius = ui_handle(__ID__);
	h_dilate_radius->f           = config_raw->dilate_radius;
	config_raw->dilate_radius    = ui_slider(h_dilate_radius, tr("Dilate Radius", null), 0.0, 16.0, true, 1, true, UI_ALIGN_RIGHT, true);
	if (ui->is_hovered) {
		ui_tooltip(tr("Dilate painted textures to prevent seams", null));
	}

	ui_handle_t *h_camera_controls          = ui_handle(__ID__);
	h_camera_controls->i                    = config_raw->camera_controls;
	string_t_array_t *camera_controls_combo = any_array_create_from_raw(
	    (any[]){
	        tr("Orbit", null),
	        tr("Rotate", null),
	        tr("Fly", null),
	    },
	    3);
	config_raw->camera_controls            = ui_combo(h_camera_controls, camera_controls_combo, tr("Default Camera Controls", null), true, UI_ALIGN_LEFT, true);

	ui_handle_t *h_fov                     = ui_handle(__ID__);
	h_fov->f                               = config_raw->camera_fov;
	config_raw->camera_fov                 = ui_slider(h_fov, tr("Default Camera FoV", null), 0.3, 1.4, true, 100.0, true, UI_ALIGN_RIGHT, true);

	ui_handle_t *h_speed                   = ui_handle(__ID__);
	h_speed->f                             = config_raw->camera_zoom_speed;
	config_raw->camera_zoom_speed          = ui_slider(h_speed, tr("Camera Zoom Speed", null), 0.1, 4.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

	h_speed                                = ui_handle(__ID__);
	h_speed->f                             = config_raw->camera_rotation_speed;
	config_raw->camera_rotation_speed      = ui_slider(h_speed, tr("Camera Rotation Speed", null), 0.1, 4.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

	h_speed                                = ui_handle(__ID__);
	h_speed->f                             = config_raw->camera_pan_speed;
	config_raw->camera_pan_speed           = ui_slider(h_speed, tr("Camera Pan Speed", null), 0.1, 4.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

	ui_handle_t *h_upside_down             = ui_handle(__ID__);
	h_upside_down->b                       = config_raw->camera_upside_down;
	config_raw->camera_upside_down         = ui_check(h_upside_down, tr("Allow Upside Down Camera", null), "");

	ui_handle_t *h_zoom_direction          = ui_handle(__ID__);
	h_zoom_direction->i                    = config_raw->zoom_direction;
	string_t_array_t *zoom_direction_combo = any_array_create_from_raw(
	    (any[]){
	        tr("Vertical", null),
	        tr("Vertical Inverted", null),
	        tr("Horizontal", null),
	        tr("Horizontal Inverted", null),
	        tr("Vertical and Horizontal", null),
	        tr("Vertical and Horizontal Inverted", null),
	    },
	    6);
	config_raw->zoom_direction  = ui_combo(h_zoom_direction, zoom_direction_combo, tr("Direction to Zoom", null), true, UI_ALIGN_LEFT, true);

	ui_handle_t *h_layer_res    = ui_handle(__ID__);
	h_layer_res->i              = config_raw->layer_res;
	#if defined(IRON_ANDROID) || defined(IRON_IOS)
	string_t_array_t *res_combo = any_array_create_from_raw(
	    (any[]){
	        "128",
	        "256",
	        "512",
	        "1024",
	        "2048",
	        "4096",
	    },
	    6);
	#else
	string_t_array_t *res_combo = any_array_create_from_raw(
	    (any[]){
	        "128",
	        "256",
	        "512",
	        "1024",
	        "2048",
	        "4096",
	        "8192",
	        "16384",
	    },
	    8);
	#endif
	config_raw->layer_res          = ui_combo(h_layer_res, res_combo, tr("Default Layer Resolution", null), true, UI_ALIGN_LEFT, true);

	ui_handle_t *h_scene_atlas_res = ui_handle(__ID__);
	h_scene_atlas_res->i           = config_raw->scene_atlas_res;
	config_raw->scene_atlas_res    = ui_combo(h_scene_atlas_res, res_combo, tr("Scene Atlas Resolution", null), true, UI_ALIGN_LEFT, true);

	ui_handle_t *h_server          = ui_handle(__ID__);
	h_server->text                 = string_copy(config_raw->server);
	config_raw->server             = string_copy(ui_text_input(h_server, tr("Cloud Server", null), UI_ALIGN_LEFT, true, false));

	ui_handle_t *h_material_live   = ui_handle(__ID__);
	h_material_live->b             = config_raw->material_live;
	config_raw->material_live      = ui_check(h_material_live, tr("Live Material Preview", null), "");
	if (ui->is_hovered) {
		ui_tooltip(tr("Instantly update material preview on node change", null));
	}

	ui_handle_t *h_brush_live = ui_handle(__ID__);
	h_brush_live->b           = config_raw->brush_live;
	config_raw->brush_live    = ui_check(h_brush_live, tr("Live Brush Preview", null), "");
	if (ui->is_hovered) {
		ui_tooltip(tr("Draw live brush preview in viewport", null));
	}
	if (h_brush_live->changed) {
		context_raw->ddirty = 2;
	}

	ui_handle_t *h_brush_depth_reject = ui_handle(__ID__);
	h_brush_depth_reject->b           = config_raw->brush_depth_reject;
	config_raw->brush_depth_reject    = ui_check(h_brush_depth_reject, tr("Depth Reject", null), "");
	if (h_brush_depth_reject->changed) {
		make_material_parse_paint_material(true);
	}

	ui_row2();

	ui_handle_t *h_brush_angle_reject = ui_handle(__ID__);
	h_brush_angle_reject->b           = config_raw->brush_angle_reject;
	config_raw->brush_angle_reject    = ui_check(h_brush_angle_reject, tr("Angle Reject", null), "");
	if (h_brush_angle_reject->changed) {
		make_material_parse_paint_material(true);
	}

	if (!config_raw->brush_angle_reject) {
		ui->enabled = false;
	}

	ui_handle_t *h_angle_dot            = ui_handle(__ID__);
	h_angle_dot->f                      = context_raw->brush_angle_reject_dot;
	context_raw->brush_angle_reject_dot = ui_slider(h_angle_dot, tr("Angle", null), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (h_angle_dot->changed) {
		make_material_parse_paint_material(true);
	}

	ui->enabled                     = true;

	ui_handle_t *h_alpha_discard    = ui_handle(__ID__);
	h_alpha_discard->f              = config_raw->brush_alpha_discard;
	config_raw->brush_alpha_discard = ui_slider(h_alpha_discard, tr("Alpha Discard", null), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (h_alpha_discard->changed) {
		make_material_parse_paint_material(true);
	}
}

void box_preferences_pen_tab() {
	ui_text(tr("Pressure controls", null), UI_ALIGN_LEFT, 0x00000000);
	ui_handle_t *h_pressure_radius      = ui_handle(__ID__);
	h_pressure_radius->b                = config_raw->pressure_radius;
	config_raw->pressure_radius         = ui_check(h_pressure_radius, tr("Brush Radius", null), "");

	ui_handle_t *h_pressure_sensitivity = ui_handle(__ID__);
	h_pressure_sensitivity->f           = config_raw->pressure_sensitivity;
	config_raw->pressure_sensitivity    = ui_slider(h_pressure_sensitivity, tr("Sensitivity", null), 0.0, 10.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

	ui_handle_t *h_pressure_hardness    = ui_handle(__ID__);
	h_pressure_hardness->b              = config_raw->pressure_hardness;
	config_raw->pressure_hardness       = ui_check(h_pressure_hardness, tr("Brush Hardness", null), "");

	ui_handle_t *h_pressure_opacity     = ui_handle(__ID__);
	h_pressure_opacity->b               = config_raw->pressure_opacity;
	config_raw->pressure_opacity        = ui_check(h_pressure_opacity, tr("Brush Opacity", null), "");

	ui_handle_t *h_pressure_angle       = ui_handle(__ID__);
	h_pressure_angle->b                 = config_raw->pressure_angle;
	config_raw->pressure_angle          = ui_check(h_pressure_angle, tr("Brush Angle", null), "");

	ui_end_element();
	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        1 / (float)4,
	    },
	    1);
	ui_row(row);
	if (ui_icon_button(tr("Help", null), ICON_LINK, UI_ALIGN_CENTER)) {
		string_t *url  = "https://github.com/armory3d/";
		string_t *name = to_lower_case(manifest_title);
		url            = string_join(url, name);
		url            = string_join(url, "_docs#pen");
		iron_load_url(url);
	}
}

void box_preferences_viewport_tab() {
	ui_handle_t *h_mode          = ui_handle(__ID__);
	h_mode->i                    = config_raw->viewport_mode;
	string_t_array_t *mode_combo = any_array_create_from_raw(
	    (any[]){
	        tr("Lit", null),
	        tr("Path Traced", null),
	    },
	    2);
	ui_combo(h_mode, mode_combo, tr("Default Mode", null), true, UI_ALIGN_LEFT, true);
	if (h_mode->changed) {
		config_raw->viewport_mode = h_mode->i;
	}

	ui_handle_t *h_pathtrace_mode          = ui_handle(__ID__);
	h_pathtrace_mode->i                    = config_raw->pathtrace_mode;
	string_t_array_t *pathtrace_mode_combo = any_array_create_from_raw(
	    (any[]){
	        tr("Fast", null),
	        tr("Quality", null),
	    },
	    2);
	config_raw->pathtrace_mode = ui_combo(h_pathtrace_mode, pathtrace_mode_combo, tr("Path Tracer", null), true, UI_ALIGN_LEFT, true);
	if (h_pathtrace_mode->changed) {
		render_path_raytrace_ready       = false;
		render_path_raytrace_init_shader = true;
		context_raw->ddirty              = 2;
	}

	ui_handle_t *h_render_mode          = ui_handle(__ID__);
	h_render_mode->i                    = config_raw->render_mode;
	string_t_array_t *render_mode_combo = any_array_create_from_raw(
	    (any[]){
	        tr("Desktop", null),
	        tr("Mobile", null),
	    },
	    2);
	config_raw->render_mode = ui_combo(h_render_mode, render_mode_combo, tr("Renderer", null), true, UI_ALIGN_LEFT, true);
	if (h_render_mode->changed) {
		context_set_render_path();
	}

	ui_handle_t *h_supersample          = ui_handle(__ID__);
	h_supersample->i                    = config_get_super_sample_quality(config_raw->rp_supersample);
	string_t_array_t *supersample_combo = any_array_create_from_raw(
	    (any[]){
	        "0.25x",
	        "0.5x",
	        "1.0x",
	        "1.5x",
	        "2.0x",
	        "4.0x",
	    },
	    6);
	ui_combo(h_supersample, supersample_combo, tr("Super Sample", null), true, UI_ALIGN_LEFT, true);
	if (h_supersample->changed) {
		config_raw->rp_supersample = config_get_super_sample_size(h_supersample->i);
		config_apply();
	}

	if (config_raw->render_mode == RENDER_MODE_DEFERRED) {
		ui_handle_t *h_ssao = ui_handle(__ID__);
		h_ssao->b           = config_raw->rp_ssao;
		config_raw->rp_ssao = ui_check(h_ssao, tr("SSAO", null), "");
		if (h_ssao->changed) {
			config_apply();
		}

		ui_handle_t *h_bloom = ui_handle(__ID__);
		h_bloom->b           = config_raw->rp_bloom;
		config_raw->rp_bloom = ui_check(h_bloom, tr("Bloom", null), "");
		if (h_bloom->changed) {
			config_apply();
		}
	}

	ui_handle_t *h_vignette = ui_handle(__ID__);
	h_vignette->f           = config_raw->rp_vignette;
	config_raw->rp_vignette = ui_slider(h_vignette, tr("Vignette", null), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (h_vignette->changed) {
		context_raw->ddirty = 2;
	}

	ui_handle_t *h_noise_grain = ui_handle(__ID__);
	h_noise_grain->f           = config_raw->rp_grain;
	config_raw->rp_grain       = ui_slider(h_noise_grain, tr("Noise Grain", null), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (h_noise_grain->changed) {
		context_raw->ddirty = 2;
	}

	camera_object_t *cam     = scene_camera;
	camera_data_t   *cam_raw = cam->data;
	ui_handle_t     *h_near  = ui_handle(__ID__);
	ui_handle_t     *h_far   = ui_handle(__ID__);
	h_near->f                = math_floor(cam_raw->near_plane * 1000) / (float)1000;
	h_far->f                 = math_floor(cam_raw->far_plane * 100) / (float)100;
	cam_raw->near_plane      = ui_slider(h_near, tr("Clip Start", null), 0.001, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	cam_raw->far_plane       = ui_slider(h_far, tr("Clip End", null), 50.0, 100.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (h_near->changed || h_far->changed) {
		camera_object_build_proj(cam, -1.0);
	}

	ui_handle_t *h_disp           = ui_handle(__ID__);
	h_disp->f                     = config_raw->displace_strength;
	config_raw->displace_strength = ui_slider(h_disp, tr("Displacement Strength", null), 0.0, 10.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
	if (h_disp->changed) {
		context_raw->ddirty = 2;
		make_material_parse_mesh_material();
	}
}

void box_preferences_keymap_tab() {
	if (box_preferences_files_keymap == null) {
		box_preferences_fetch_keymaps();
	}

	ui_begin_sticky();
	ui_row4();

	gc_unroot(box_preferences_h_preset);
	box_preferences_h_preset = ui_handle(__ID__);
	gc_root(box_preferences_h_preset);
	box_preferences_h_preset->i = box_preferences_get_preset_index();
	ui_combo(box_preferences_h_preset, box_preferences_files_keymap, tr("Preset", null), false, UI_ALIGN_LEFT, true);
	if (box_preferences_h_preset->changed) {
		config_raw->keymap = string_join(box_preferences_files_keymap->buffer[box_preferences_h_preset->i], ".json");
		config_apply();
		keymap_load();
	}

	if (ui_icon_button(tr("New", null), ICON_PLUS, UI_ALIGN_CENTER)) {
		ui_box_show_custom(&box_preferences_keymap_tab_172800, 400, 200, null, true, tr("New Keymap", null));
	}

	if (ui_icon_button(tr("Import", null), ICON_IMPORT, UI_ALIGN_CENTER)) {
		ui_files_show("json", false, false, &box_preferences_keymap_tab_172980);
	}

	if (ui_icon_button(tr("Export", null), ICON_EXPORT, UI_ALIGN_CENTER)) {
		ui_files_show("json", true, false, &box_preferences_keymap_tab_173017);
	}

	ui_end_sticky();

	ui_separator(8, false);

	i32 index              = 0;
	ui->changed            = false;
	string_t_array_t *keys = map_keys(config_keymap);
	array_sort(keys, null);
	for (i32 i = 0; i < keys->length; ++i) {
		string_t    *key = keys->buffer[i];
		ui_handle_t *h   = ui_nest(ui_handle(__ID__), index++);
		h->text          = string_copy(any_map_get(config_keymap, key));
		string_t *text   = ui_text_input(h, key, UI_ALIGN_LEFT, true, false);
		any_map_set(config_keymap, key, text);
	}
	if (ui->changed) {
		config_apply();
		keymap_save();
	}
}

void box_preferences_keymap_tab_173017(string_t *dest) {
	if (!ends_with(ui_files_filename, ".json")) {
		gc_unroot(ui_files_filename);
		ui_files_filename = string_join(ui_files_filename, ".json");
		gc_root(ui_files_filename);
	}
	string_t *path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "keymap_presets"), PATH_SEP), config_raw->keymap);
	file_copy(path, string_join(string_join(dest, PATH_SEP), ui_files_filename));
}

void box_preferences_keymap_tab_172980(string_t *path) {
	import_keymap_run(path);
}

void box_preferences_keymap_tab_172800() {
	ui_row2();
	ui_handle_t *h = ui_handle(__ID__);
	if (h->init) {
		h->text = "new_keymap";
	}
	string_t *keymap_name = ui_text_input(h, tr("Name", null), UI_ALIGN_LEFT, true, false);
	if (ui_icon_button(tr("OK", null), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {
		string_t *template = keymap_to_json(keymap_get_default());
		if (!ends_with(keymap_name, ".json")) {
			keymap_name = string_join(keymap_name, ".json");
		}
		string_t *path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "keymap_presets"), PATH_SEP), keymap_name);
		iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
		box_preferences_fetch_keymaps(); // Refresh file list
		config_raw->keymap          = string_copy(keymap_name);
		box_preferences_h_preset->i = box_preferences_get_preset_index();
		ui_box_hide();
		box_preferences_htab->i = 5; // Keymap
		box_preferences_show();
	}
}

void box_preferences_neural_tab() {
	ui_text(tr("All processing is done locally on device", null), UI_ALIGN_LEFT, 0x00000000);
	ui_handle_t *h_inference          = ui_handle(__ID__);
	h_inference->i                    = config_raw->neural_backend;
	#ifdef IRON_WINDOWS
	string_t_array_t *inference_combo = any_array_create_from_raw(
	    (any[]){
	        "CPU",
	        "Vulkan",
	        "CUDA",
	    },
	    3);
	#else
	string_t_array_t *inference_combo = any_array_create_from_raw(
	    (any[]){
	        "CPU",
	        "Vulkan",
	    },
	    2);
	#endif
	config_raw->neural_backend = ui_combo(h_inference, inference_combo, tr("Inference Backend", null), true, UI_ALIGN_LEFT, true);
	if (ui->is_hovered) {
		ui_tooltip(tr("Backend for neural node processing", null));
	}

	ui_text(tr("Models", null), UI_ALIGN_LEFT, 0x00000000);

	if (neural_node_models == null) {
		neural_node_models_init();
	}

	for (i32 i = 0; i < neural_node_models->length; ++i) {
		box_preferences_model_panel(neural_node_models->buffer[i]);
	}

	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        1 / (float)4,
	    },
	    1);
	ui_row(row);
	if (ui_icon_button(tr("Models Directory...", null), ICON_FOLDER, UI_ALIGN_CENTER)) {
		if (string_equals(file_read_directory(neural_node_dir())->buffer[0], "")) {
			iron_create_directory(neural_node_dir());
		}
		file_start(neural_node_dir());
	}
}

bool box_preferences_model_exists(string_t *file_name) {
	return iron_file_exists(string_join(string_join(neural_node_dir(), PATH_SEP), file_name));
}

string_t *box_preferences_file_name_from_url(string_t *url) {
	return substring(url, string_last_index_of(url, "/") + 1, string_length(url));
}

string_t *box_preferneces_model_url_from_name(string_t *name) {
	if (neural_node_models == null) {
		neural_node_models_init();
	}

	for (i32 i = 0; i < neural_node_models->length; ++i) {
		if (string_equals(neural_node_models->buffer[i]->name, name)) {
			return neural_node_models->buffer[i]->urls->buffer[0];
		}
	}
	return null;
}

void box_preferences_model_panel(neural_node_model_t *m) {
	if (ui_panel(ui_handle(m->name), m->name, false, true, false)) {
		if (ui_text(string_join(string_join(string_join(string_join(string_join(tr("source", null), ": "), m->web), " ("), m->license), ")"), UI_ALIGN_LEFT,
		            0x00000000) == UI_STATE_RELEASED) {
			iron_load_url(m->web);
		}
		ui_text(string_join(string_join(tr("gpu memory", null), ": "), m->memory), UI_ALIGN_LEFT, 0x00000000);
		ui_text(string_join(string_join(tr("nodes", null), ": "), m->nodes), UI_ALIGN_LEFT, 0x00000000);

		string_t *url       = m->urls->buffer[0];
		string_t *file_name = box_preferences_file_name_from_url(url);
		bool      found     = box_preferences_model_exists(file_name);

		if (neural_node_downloading > 0) {
			ui->enabled                   = false;

			u64       u                   = iron_net_bytes_downloaded;
			i32       i                   = (u / (float)1000000000) * 100;
			f32       f                   = i / (float)100;
			string_t *downloaded          = string_join(f32_to_string(f), "GB");
			ui_box_hwnd->redraws          = 2;
			box_preferences_htab->redraws = 2;
			iron_delay_idle_sleep();

			i32 _BUTTON_COL            = ui->ops->theme->BUTTON_COL;
			ui->ops->theme->BUTTON_COL = ui->ops->theme->HIGHLIGHT_COL;

			ui_handle_t *h             = ui_handle(__ID__);
			h->f                       = f / (float)parse_float(m->size);
			ui_slider(h, string_join(string_join(downloaded, " / "), m->size), 0.0, 1.0, true, 100, false, UI_ALIGN_CENTER, true);

			ui->ops->theme->BUTTON_COL = _BUTTON_COL;

			ui->enabled                = true;
		}
		else if (!found && ui_icon_button(string_join(string_join(string_join(tr("Download", null), " ("), m->size), ")"), ICON_ARROW_DOWN, UI_ALIGN_CENTER)) {
			neural_node_download_models(m->urls);
			console_info(tr("Downloading", null));
		}
		else if (found && ui_icon_button(string_join(string_join(string_join(tr("Remove", null), " ("), m->size), ")"), ICON_DELETE, UI_ALIGN_CENTER)) {
			for (i32 i = 0; i < m->urls->length; ++i) {
				string_t *url       = m->urls->buffer[i];
				string_t *file_name = box_preferences_file_name_from_url(url);
				iron_delete_file(string_join(string_join(neural_node_dir(), PATH_SEP), file_name));
			}
		}
	}
}

void box_preferences_plugins_tab() {
	ui_begin_sticky();
	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        1 / (float)4,
	        1 / (float)4,
	    },
	    2);
	ui_row(row);
	if (ui_icon_button(tr("New", null), ICON_PLUS, UI_ALIGN_CENTER)) {
		ui_box_show_custom(&box_preferences_plugins_tab_173894, 400, 200, null, true, tr("New Plugin", null));
	}
	if (ui_icon_button(tr("Import", null), ICON_IMPORT, UI_ALIGN_CENTER)) {
		ui_files_show("js,zip", false, false, &box_preferences_plugins_tab_174059);
	}
	ui_end_sticky();

	if (box_preferences_files_plugin == null) {
		box_preferences_fetch_plugins();
	}

	if (config_raw->plugins == null) {
		config_raw->plugins = any_array_create_from_raw((any[]){}, 0);
	}
	ui_handle_t *h = ui_handle(__ID__);
	if (h->init) {
		h->b = false;
	}
	for (i32 i = 0; i < box_preferences_files_plugin->length; ++i) {
		string_t *f     = box_preferences_files_plugin->buffer[i];
		bool      is_js = ends_with(f, ".js");
		if (!is_js) {
			continue;
		}
		bool enabled  = char_ptr_array_index_of(config_raw->plugins, f) >= 0;
		h->b          = enabled;
		string_t *tag = is_js ? string_split(f, ".")->buffer[0] : f;
		ui_check(h, tag, "");
		if (h->changed && h->b != enabled) {
			h->b ? config_enable_plugin(f) : config_disable_plugin(f);
			base_redraw_ui();
		}
		if (ui->is_hovered && ui->input_released_r) {
			gc_unroot(_box_preferences_f);
			_box_preferences_f = string_copy(f);
			gc_root(_box_preferences_f);
			ui_menu_draw(&box_preferences_plugins_tab_174255, -1, -1);
		}
	}
}

void box_preferences_plugins_tab_174362(string_t *dest) {
	if (!ends_with(ui_files_filename, ".js")) {
		gc_unroot(ui_files_filename);
		ui_files_filename = string_join(ui_files_filename, ".js");
		gc_root(ui_files_filename);
	}
	string_t *path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "plugins"), PATH_SEP), _box_preferences_f);
	file_copy(path, string_join(string_join(dest, PATH_SEP), ui_files_filename));
}

void box_preferences_plugins_tab_174255() {
	string_t *path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "plugins"), PATH_SEP), _box_preferences_f);
	if (ui_menu_button(tr("Edit in Text Editor", null), "", ICON_NONE)) {
		file_start(path);
	}
	if (ui_menu_button(tr("Edit in Script Tab", null), "", ICON_NONE)) {
		buffer_t *blob            = data_get_blob(string_join("plugins/", _box_preferences_f));
		tab_scripts_hscript->text = string_copy(sys_buffer_to_string(blob));
		data_delete_blob(string_join("plugins/", _box_preferences_f));
		console_info(tr("Script opened", null));
	}
	if (ui_menu_button(tr("Export", null), "", ICON_EXPORT)) {
		ui_files_show("js", true, false, &box_preferences_plugins_tab_174362);
	}
	if (ui_menu_button(tr("Delete", null), "", ICON_DELETE)) {
		if (char_ptr_array_index_of(config_raw->plugins, _box_preferences_f) >= 0) {
			char_ptr_array_remove(config_raw->plugins, _box_preferences_f);
			plugin_stop(_box_preferences_f);
		}
		char_ptr_array_remove(box_preferences_files_plugin, _box_preferences_f);
		iron_delete_file(path);
	}
}

void box_preferences_plugins_tab_174059(string_t *path) {
	import_plugin_run(path);
}

void box_preferences_plugins_tab_173894() {
	ui_row2();
	ui_handle_t *h = ui_handle(__ID__);
	if (h->init) {
		h->text = "new_plugin";
	}
	string_t *plugin_name = ui_text_input(h, tr("Name", null), UI_ALIGN_LEFT, true, false);
	if (ui_icon_button(tr("OK", null), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {
		string_t *template = "let plugin = plugin_create();\
let h1 = ui_handle_create();\
plugin_notify_on_ui(plugin, function() {\
	if (ui_panel(h1, \"New Plugin\")) {\
		if (ui_button(\"Button\")) {\
			console_info(\"Hello\");\
		}\
	}\
});\
";
		if (!ends_with(plugin_name, ".js")) {
			plugin_name = string_join(plugin_name, ".js");
		}
		string_t *path = string_join(string_join(string_join(string_join(path_data(), PATH_SEP), "plugins"), PATH_SEP), plugin_name);
		iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
		gc_unroot(box_preferences_files_plugin);
		box_preferences_files_plugin = null; // Refresh file list
		ui_box_hide();
		box_preferences_htab->i = 6; // Plugins
		box_preferences_show();
	}
}

void box_preferences_show() {
	ui_box_show_custom(&box_preferences_show_174484, 720, 520, &box_preferences_show_174671, true, tr("Preferences", null));
}

void box_preferences_show_174671() {
	config_save();
}

void box_preferences_show_174484() {
	if (ui_tab(box_preferences_htab, tr("Interface", null), true, -1, false)) {
		box_preferences_interface_tab();
	}
	if (ui_tab(box_preferences_htab, tr("Theme", null), true, -1, false)) {
		box_preferences_theme_tab();
	}
	if (ui_tab(box_preferences_htab, tr("Usage", null), true, -1, false)) {
		box_preferences_usage_tab();
	}

	#ifdef IRON_IOS
	string_t *pen_name = tr("Pencil", null);
	#else
	string_t *pen_name = tr("Pen", null);
	#endif
	if (ui_tab(box_preferences_htab, pen_name, true, -1, false)) {
		box_preferences_pen_tab();
	}

	if (ui_tab(box_preferences_htab, tr("Viewport", null), true, -1, false)) {
		box_preferences_viewport_tab();
	}
	if (ui_tab(box_preferences_htab, tr("Keymap", null), true, -1, false)) {
		box_preferences_keymap_tab();
	}
	#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	if (ui_tab(box_preferences_htab, tr("Neural", null), true, -1, false)) {
		box_preferences_neural_tab();
	}
	#endif
	if (ui_tab(box_preferences_htab, tr("Plugins", null), true, -1, false)) {
		box_preferences_plugins_tab();
	}
}

void box_preferences_fetch_themes() {
	gc_unroot(box_preferences_themes);
	box_preferences_themes = file_read_directory(string_join(string_join(path_data(), PATH_SEP), "themes"));
	gc_root(box_preferences_themes);
	for (i32 i = 0; i < box_preferences_themes->length; ++i) {
		string_t *s                       = box_preferences_themes->buffer[i];
		box_preferences_themes->buffer[i] = substring(box_preferences_themes->buffer[i], 0, string_length(s) - 5); // Strip .json
	}
	array_insert(box_preferences_themes, 0, "default");
}

void box_preferences_fetch_keymaps() {
	gc_unroot(box_preferences_files_keymap);
	box_preferences_files_keymap = file_read_directory(string_join(string_join(path_data(), PATH_SEP), "keymap_presets"));
	gc_root(box_preferences_files_keymap);
	for (i32 i = 0; i < box_preferences_files_keymap->length; ++i) {
		string_t *s                             = box_preferences_files_keymap->buffer[i];
		box_preferences_files_keymap->buffer[i] = substring(box_preferences_files_keymap->buffer[i], 0, string_length(s) - 5); // Strip .json
	}
	array_insert(box_preferences_files_keymap, 0, "default");
}

void box_preferences_fetch_plugins() {
	gc_unroot(box_preferences_files_plugin);
	box_preferences_files_plugin = file_read_directory(string_join(string_join(path_data(), PATH_SEP), "plugins"));
	gc_root(box_preferences_files_plugin);
}

i32 box_preferences_get_theme_index() {
	return char_ptr_array_index_of(box_preferences_themes, substring(config_raw->theme, 0, string_length(config_raw->theme) - 5)); // Strip .json
}

i32 box_preferences_get_preset_index() {
	return char_ptr_array_index_of(box_preferences_files_keymap, substring(config_raw->keymap, 0, string_length(config_raw->keymap) - 5)); // Strip .json
}

void box_preferences_set_scale() {
	f32 scale = config_raw->window_scale;
	ui_set_scale(scale);
	ui_header_h                                      = math_floor(ui_header_default_h * scale);
	config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] = math_floor(ui_statusbar_default_h * scale);
	ui_menubar_w                                     = math_floor(ui_menubar_default_w * scale);
	ui_base_set_icon_scale();
	base_resize();
	config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] = math_floor(ui_sidebar_default_w * scale);
}

string_t *box_preferences_theme_to_json(ui_theme_t *theme) {
	json_encode_begin();
	u32_ptr u32_theme = theme;
	for (i32 i = 0; i < ui_theme_keys_count; ++i) {
		string_t *key = ARRAY_ACCESS(ui_theme_keys, i);
		u32       val = DEREFERENCE(u32_theme + i);
		json_encode_i32(key, val);
	}
	return json_encode_end();
}
