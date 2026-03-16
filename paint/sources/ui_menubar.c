
#include "global.h"

void ui_menubar_init() {
	ui_menubar_hwnd->layout        = UI_LAYOUT_HORIZONTAL;
	ui_menubar_menu_handle->layout = UI_LAYOUT_HORIZONTAL;
}

i32 ui_menu_panel_x() {
	if (!base_view3d_show && ui_view2d_show) {
		return 0;
	}

	i32 panel_x = base_x();
	if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
		i32 item_w = ui_toolbar_w(false);
		panel_x    = base_x() - item_w;
	}
	else {
		panel_x += 5 * UI_SCALE();
	}
	return panel_x;
}

i32 ui_menu_top_y() {
#ifdef IRON_IOS
	if (config_is_iphone()) {
		return UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
	}
#endif

	if (config_raw->touch_ui) {
		return 0;
	}

	return UI_ELEMENT_H();
}

i32 ui_menu_panel_y() {
#ifdef IRON_IOS
	if (config_is_iphone()) {
		return ui_header_h;
	}
#endif

	i32 panel_y = 0;
	if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 0) { // Floating
		panel_y += 5 * UI_SCALE();
	}
	return panel_y;
}

void ui_menubar_render_ui_save_project_on_next_frame(void *_) {
	project_save(false);
	box_projects_show();
}

void ui_menubar_render_ui_save_project(void *_) {
	sys_notify_on_end_frame(&ui_menubar_render_ui_save_project_on_next_frame, NULL);
}

void ui_menubar_render_ui() {
	if (config_raw->touch_ui && !base_view3d_show) {
		return;
	}

	i32 item_w  = ui_toolbar_w(false);
	i32 panel_x = ui_menu_panel_x();
	i32 panel_y = ui_menu_panel_y();

	if (ui_window(ui_menubar_menu_handle, panel_x, panel_y, ui_menubar_w, ui_header_h, false)) {

		if (!base_view3d_show) {
			ui->_x += 1;
		}

		ui_begin_menu();

		if (config_raw->touch_ui) {
			ui->_w = item_w;

			if (ui_menubar_icon_button(ICON_MENU)) {
				box_preferences_show();
			}
			if (ui_menubar_icon_button(ICON_PROJECTS)) {
				// Save project icon in lit mode
				context_set_viewport_mode(VIEWPORT_MODE_LIT);
				console_toast(tr("Saving project"));
				sys_notify_on_next_frame(&ui_menubar_render_ui_save_project, NULL);
			}
			if (ui_menubar_icon_button(ICON_IMPORT)) {
				project_import_asset(NULL, true);
			}
			if (ui_menubar_icon_button(ICON_EXPORT)) {
				box_export_show_textures();
			}
			i32 size = math_floor(ui->_w / (float)UI_SCALE());
			if (ui_menu_show && ui_menubar_category == MENUBAR_CATEGORY_VIEWPORT) {
				ui_fill(0, -6, size, size - 4, ui->ops->theme->HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ICON_IMAGE)) {
				ui_menubar_show_menu(MENUBAR_CATEGORY_VIEWPORT);
			}
			if (ui_menu_show && ui_menubar_category == MENUBAR_CATEGORY_MODE) {
				ui_fill(0, -6, size, size - 4, ui->ops->theme->HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ICON_SUN)) {
				ui_menubar_show_menu(MENUBAR_CATEGORY_MODE);
			}
			if (ui_menu_show && ui_menubar_category == MENUBAR_CATEGORY_CAMERA) {
				ui_fill(0, -6, size, size - 4, ui->ops->theme->HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ICON_CAMERA)) {
				ui_menubar_show_menu(MENUBAR_CATEGORY_CAMERA);
			}
			// if (ui_menubar_icon_button(ICON_WINDOW)) {
				// ui_menubar_show_menu(MENUBAR_CATEGORY_WORKSPACE);
			// }
			if (ui_menu_show && ui_menubar_category == MENUBAR_CATEGORY_HELP) {
				ui_fill(0, -6, size, size - 4, ui->ops->theme->HIGHLIGHT_COL);
			}

			bool full = true;
#ifdef IRON_IOS
			if (config_is_iphone()) {
				full = false;
			}
#endif

			if (full && ui_menubar_icon_button(ICON_HELP)) {
				ui_menubar_show_menu(MENUBAR_CATEGORY_HELP);
			}

			ui->enabled = history_undos > 0;
			if (ui_menubar_icon_button(ICON_UNDO)) {
				history_undo();
			}
			ui->enabled = history_redos > 0;
			if (full && ui_menubar_icon_button(ICON_REDO)) {
				history_redo();
			}
			ui->enabled = true;
		}
		else {
			string_t_array_t *categories = any_array_create_from_raw(
			    (void *[]){
			        tr("File"),
			        tr("Edit"),
			        tr("Viewport"),
			        tr("Mode"),
			        tr("Camera"),
			        tr("Workspace"),
			        tr("Help"),
			    },
			    7);

			for (i32 i = 0; i < categories->length; ++i) {
				if (ui_menubar_button(categories->buffer[i]) || (ui_menu_show && ui_menu_commands == ui_menubar_draw_category_items && ui->is_hovered)) {
					ui_menubar_show_menu(i);
				}
			}
		}

		// Store real menubar w
		if (ui_menubar_w < ui->_x + 10) {
			ui_menubar_w               = math_floor(ui->_x + 10);
			ui_toolbar_handle->redraws = 2;
		}
		// Crop menubar if sidebar + nodes are overlapping
		i32 nodesw = (ui_nodes_show || ui_view2d_show) ? config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] : 0;
		if (ui_menubar_w > iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - nodesw) {
			ui_menubar_w               = iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - nodesw;
			ui_toolbar_handle->redraws = 2;
		}

		ui_end_menu();
	}

	if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
		// Non-floating header
		ui_menubar_draw_tab_header();
	}
}

void ui_menubar_draw_tab_header() {
	i32 item_w  = ui_toolbar_w(false);
	i32 nodesw  = (ui_nodes_show || ui_view2d_show) ? config_raw->layout->buffer[LAYOUT_SIZE_NODES_W] : 0;
	i32 ww      = iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - ui_menubar_w - nodesw;
	i32 panel_x = (base_x() - item_w) + ui_menubar_w;

	if (!base_view3d_show) {
		panel_x += 1;
	}

	if (!base_view3d_show && ui_view2d_show) {
		panel_x = ui_menubar_w;
		ww -= 100 * UI_SCALE();
	}

	if (ui_window(ui_menubar_hwnd, panel_x, 0, ww, ui_header_h, false)) {
		if (config_raw->touch_ui) {
			ui_fill(0, 0, ui->_window_w, ui->_window_h + 4, ui->ops->theme->SEPARATOR_COL);
		}
		else {
			bool a = ui_tab(ui_menubar_tab, tr("3D View"), false, -1, false);
			bool b = ui_tab(ui_menubar_tab, base_view3d_show ? ">" : "<", false, -2, false);
			if ((a && !base_view3d_show) || b) {
				base_view3d_show  = !base_view3d_show;
				ui_menubar_tab->i = base_view3d_show ? 0 : -1;
				base_resize();
			}
		}
	}
}

void ui_menubar_draw_category_items_about_box() {
	gpu_texture_t *img = data_get_image("badge.k");
	ui_image(img, 0xffffffff, -1.0);
	ui_end_element();

	ui_handle_t *h = ui_handle(__ID__);
	if (h->init) {
		h->text = string_copy(_ui_menu_render_msg);
	}
	ui_text_area(h, UI_ALIGN_LEFT, false, "", false);

	ui_row3();

#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	if (ui_icon_button(tr("Copy"), ICON_COPY, UI_ALIGN_CENTER)) {
		iron_copy_to_clipboard(_ui_menu_render_msg);
	}
#else
	ui_end_element();
#endif

	if (ui_icon_button(tr("Contributors"), ICON_LINK, UI_ALIGN_CENTER)) {
		iron_load_url("https://github.com/armory3d/armortools/graphs/contributors");
	}
	if (ui_icon_button(tr("OK"), ICON_CHECK, UI_ALIGN_CENTER)) {
		ui_box_hide();
	}
}

void ui_menubar_draw_category_items_on_check_for_updates_downloaded(char *url, buffer_t *buffer) {
	if (buffer != NULL) {
		// Compare versions
		update_info_t *update         = json_parse(sys_buffer_to_string(buffer));
		i32            update_version = math_floor(update->version);
		if (update_version > 0) {
			char *date   = config_get_date(); // 2019 -> 19
			date         = string_copy(substring(date, 2, string_length(date)));
			i32 date_int = parse_int(string_replace_all(date, "-", ""));
			if (update_version > date_int) {
				any_map_t *vars = any_map_create();
				any_map_set(vars, "url", manifest_url);
				ui_box_show_message(tr("Update"), vtr("Update is available!\nPlease visit {url}.", vars), false);
			}
			else {
				ui_box_show_message(tr("Update"), tr("You are up to date!"), false);
			}
		}
	}
	else {
		any_map_t *vars = any_map_create();
		any_map_set(vars, "url", manifest_url);
		ui_box_show_message(tr("Update"), vtr("Unable to check for updates.\nPlease visit {url}.", vars), false);
	}
}

void ui_menubar_draw_category_items_capture_screenshot(void *_) {
	viewport_capture_screenshot();
}

void ui_menubar_draw_category_items_import_envmap(char *path) {
	if (!ends_with(path, ".hdr")) {
		console_error(tr("Error: .hdr file expected"));
		return;
	}
	import_asset_run(path, -1.0, -1.0, true, true, NULL);
}

viewport_mode_t ui_menubar_string_to_viewport_mode(char *s) {
	if (string_equals(s, tr("Lit")))
		return VIEWPORT_MODE_LIT;
	if (string_equals(s, tr("Base Color")))
		return VIEWPORT_MODE_BASE_COLOR;
	if (string_equals(s, tr("Normal")))
		return VIEWPORT_MODE_NORMAL_MAP;
	if (string_equals(s, tr("Occlusion")))
		return VIEWPORT_MODE_OCCLUSION;
	if (string_equals(s, tr("Roughness")))
		return VIEWPORT_MODE_ROUGHNESS;
	if (string_equals(s, tr("Metallic")))
		return VIEWPORT_MODE_METALLIC;
	if (string_equals(s, tr("Opacity")))
		return VIEWPORT_MODE_OPACITY;
	if (string_equals(s, tr("Height")))
		return VIEWPORT_MODE_HEIGHT;
	if (string_equals(s, tr("Emission")))
		return VIEWPORT_MODE_EMISSION;
	if (string_equals(s, tr("Subsurface")))
		return VIEWPORT_MODE_SUBSURFACE;
	if (string_equals(s, tr("TexCoord")))
		return VIEWPORT_MODE_TEXCOORD;
	if (string_equals(s, tr("Object Normal")))
		return VIEWPORT_MODE_OBJECT_NORMAL;
	if (string_equals(s, tr("Material ID")))
		return VIEWPORT_MODE_MATERIAL_ID;
	if (string_equals(s, tr("Object ID")))
		return VIEWPORT_MODE_OBJECT_ID;
	if (string_equals(s, tr("Mask")))
		return VIEWPORT_MODE_MASK;
	if (string_equals(s, tr("Path Traced")))
		return VIEWPORT_MODE_PATH_TRACE;
	return VIEWPORT_MODE_LIT;
}

char *ui_menubar_viewport_mode_to_string(viewport_mode_t m) {
	if (m == VIEWPORT_MODE_LIT)
		return "Lit";
	if (m == VIEWPORT_MODE_BASE_COLOR)
		return "Base Color";
	if (m == VIEWPORT_MODE_NORMAL_MAP)
		return "Normal";
	if (m == VIEWPORT_MODE_OCCLUSION)
		return "Occlusion";
	if (m == VIEWPORT_MODE_ROUGHNESS)
		return "Roughness";
	if (m == VIEWPORT_MODE_METALLIC)
		return "Metallic";
	if (m == VIEWPORT_MODE_OPACITY)
		return "Opacity";
	if (m == VIEWPORT_MODE_HEIGHT)
		return "Height";
	if (m == VIEWPORT_MODE_EMISSION)
		return "Emission";
	if (m == VIEWPORT_MODE_SUBSURFACE)
		return "Subsurface";
	if (m == VIEWPORT_MODE_TEXCOORD)
		return "TexCoord";
	if (m == VIEWPORT_MODE_OBJECT_NORMAL)
		return "Object Normal";
	if (m == VIEWPORT_MODE_MATERIAL_ID)
		return "Material ID";
	if (m == VIEWPORT_MODE_OBJECT_ID)
		return "Object ID";
	if (m == VIEWPORT_MODE_MASK)
		return "Mask";
	if (m == VIEWPORT_MODE_PATH_TRACE)
		return "Path Traced";
	return "Lit";
}

void ui_menubar_draw_category_items() {
	if (ui_menubar_category == MENUBAR_CATEGORY_FILE) {
		if (ui_menu_button(tr("New Project..."), any_map_get(config_keymap, "file_new"), ICON_FILE_NEW)) {
			project_new_box();
		}
		if (ui_menu_button(tr("Open..."), any_map_get(config_keymap, "file_open"), ICON_FOLDER_OPEN)) {
			project_open();
		}
		if (ui_menu_button(tr("Open Recent..."), any_map_get(config_keymap, "file_open_recent"), ICON_REPLAY)) {
			box_projects_show();
		}
		if (ui_menu_button(tr("Save"), any_map_get(config_keymap, "file_save"), ICON_SAVE)) {
			project_save(false);
		}
		if (ui_menu_button(tr("Save As..."), any_map_get(config_keymap, "file_save_as"), ICON_SAVE_AS)) {
			project_save_as(false);
		}
		ui_menu_separator();
		if (ui_menu_sub_button(ui_handle(__ID__), tr("Import"))) {
			ui_menu_sub_begin(7);
			if (ui_menu_button(tr("Texture..."), any_map_get(config_keymap, "file_import_assets"), ICON_IMAGE)) {
				project_import_asset(string_array_join(path_texture_formats(), ","), false);
			}
			if (ui_menu_button(tr("Envmap..."), "", ICON_LANDSCAPE)) {
				ui_files_show("hdr", false, false, &ui_menubar_draw_category_items_import_envmap);
			}
			if (ui_menu_button(tr("Font..."), "", ICON_FONT)) {
				project_import_asset("ttf,ttc,otf", true);
			}
			if (ui_menu_button(tr("Material..."), "", ICON_SPHERE)) {
				project_import_material();
			}
			if (ui_menu_button(tr("Brush..."), "", ICON_PAINT)) {
				project_import_brush();
			}
			if (ui_menu_button(tr("Swatches..."), "", ICON_PALETTE)) {
				project_import_swatches(false);
			}
			if (ui_menu_button(tr("Mesh..."), "", ICON_CUBE)) {
				project_import_mesh(true, NULL);
			}
			ui_menu_sub_end();
		}
		if (ui_menu_button(tr("Reimport Mesh"), any_map_get(config_keymap, "file_reimport_mesh"), ICON_SYNC)) {
			project_reimport_mesh();
		}
		if (ui_menu_button(tr("Reimport Textures"), any_map_get(config_keymap, "file_reimport_textures"), ICON_SYNC)) {
			project_reimport_textures();
		}
		ui_menu_separator();
		if (ui_menu_sub_button(ui_handle(__ID__), tr("Export"))) {
			ui_menu_sub_begin(3);
			if (ui_menu_button(tr("Textures..."), any_map_get(config_keymap, "file_export_textures_as"), ICON_IMAGE)) {
				context_raw->layers_export = EXPORT_MODE_VISIBLE;
				box_export_show_textures();
			}
			if (ui_menu_button(tr("Swatches..."), "", ICON_PALETTE)) {
				project_export_swatches();
			}
			if (ui_menu_button(tr("Mesh..."), "", ICON_CUBE)) {
				context_raw->export_mesh_index = 0; // All
				box_export_show_mesh();
			}
			ui_menu_sub_end();
		}
		if (ui_menu_button(tr("Bake Material..."), "", ICON_BAKE)) {
			box_export_show_bake_material();
		}
		ui_menu_separator();
		if (ui_menu_button(tr("Exit"), "", ICON_EXIT)) {
			iron_stop();
		}
	}
	else if (ui_menubar_category == MENUBAR_CATEGORY_EDIT) {
		char *step_undo = "";
		char *step_redo = "";
		if (history_undos > 0) {
			step_undo = string_copy(history_steps->buffer[history_steps->length - 1 - history_redos]->name);
		}
		if (history_redos > 0) {
			step_redo = string_copy(history_steps->buffer[history_steps->length - history_redos]->name);
		}

		ui->enabled          = history_undos > 0;
		any_map_t *vars_undo = any_map_create();
		any_map_set(vars_undo, "step", step_undo);
		if (ui_menu_button(vtr("Undo {step}", vars_undo), any_map_get(config_keymap, "edit_undo"), ICON_UNDO)) {
			history_undo();
		}

		ui->enabled          = history_redos > 0;
		any_map_t *vars_redo = any_map_create();
		any_map_set(vars_redo, "step", step_redo);
		if (ui_menu_button(vtr("Redo {step}", vars_redo), any_map_get(config_keymap, "edit_redo"), ICON_REDO)) {
			history_redo();
		}
		ui->enabled = true;
		ui_menu_separator();
		if (ui_menu_button(tr("Preferences..."), any_map_get(config_keymap, "edit_prefs"), ICON_COG)) {
			box_preferences_show();
		}
	}
	else if (ui_menubar_category == MENUBAR_CATEGORY_VIEWPORT) {
		if (ui_menu_button(tr("Distract Free"), any_map_get(config_keymap, "view_distract_free"), ICON_NONE)) {
			ui_base_toggle_distract_free();
			ui->is_hovered = false;
		}

#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS) || defined(IRON_WASM)
		if (ui_menu_button(tr("Toggle Fullscreen"), "alt+enter", ICON_FULLSCREEN)) {
			base_toggle_fullscreen();
		}
#endif

		ui->changed = false;

		world_data_t *p          = scene_world;
		ui_handle_t  *env_handle = ui_handle(__ID__);
		env_handle->f            = p->strength;
		ui_menu_align();
		p->strength = ui_slider(env_handle, tr("Environment"), 0.0, 6.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		if (env_handle->changed) {
			context_raw->ddirty = 2;
		}

		ui_handle_t *enva_handle = ui_handle(__ID__);
		enva_handle->f           = context_raw->envmap_angle / (float)math_pi() * 180.0;
		if (enva_handle->f < 0) {
			enva_handle->f += (math_floor(-enva_handle->f / 360.0) + 1) * 360;
		}
		else if (enva_handle->f > 360) {
			enva_handle->f -= math_floor(enva_handle->f / 360.0) * 360;
		}
		ui_menu_align();
		context_raw->envmap_angle =
		    ui_slider(enva_handle, tr("Environment Angle"), 0.0, 360.0, true, 1, true, UI_ALIGN_RIGHT, true) / 180.0 * math_pi();
		if (ui->is_hovered) {
			any_map_t *vars = any_map_create();
			any_map_set(vars, "shortcut", any_map_get(config_keymap, "rotate_envmap"));
			ui_tooltip(vtr("{shortcut} and move mouse", vars));
		}
		if (enva_handle->changed) {
			context_raw->ddirty = 2;
		}

		ui_handle_t *split_view_handle = ui_handle(__ID__);
		if (split_view_handle->init) {
			split_view_handle->b = context_raw->split_view;
		}
		context_raw->split_view = ui_check(split_view_handle, string(" %s", tr("Split View")), "");
		if (split_view_handle->changed) {
			base_resize();
		}

		ui_handle_t *cull_handle = ui_handle(__ID__);
		if (cull_handle->init) {
			cull_handle->b = context_raw->cull_backfaces;
		}
		context_raw->cull_backfaces = ui_check(cull_handle, string(" %s", tr("Cull Backfaces")), "");
		if (cull_handle->changed) {
			make_material_parse_mesh_material();
		}

		ui_handle_t *filter_handle = ui_handle(__ID__);
		if (filter_handle->init) {
			filter_handle->b = context_raw->texture_filter;
		}
		context_raw->texture_filter = ui_check(filter_handle, string(" %s", tr("Filter Textures")), "");
		if (filter_handle->changed) {
			gpu_use_linear_sampling(context_raw->texture_filter);
		}

		context_raw->draw_wireframe = ui_check(context_raw->wireframe_handle, string(" %s", tr("Wireframe")), "");
		if (context_raw->wireframe_handle->changed) {
			gpu_texture_t *current = _draw_current;
			draw_end();
			util_uv_cache_uv_map();
			draw_begin(current, false, 0);
			make_material_parse_mesh_material();
		}

		context_raw->draw_texels = ui_check(context_raw->texels_handle, string(" %s", tr("Texels")), "");
		if (context_raw->texels_handle->changed) {
			make_material_parse_mesh_material();
		}

		ui_handle_t *compass_handle = ui_handle(__ID__);
		if (compass_handle->init) {
			compass_handle->b = context_raw->show_compass;
		}
		context_raw->show_compass = ui_check(compass_handle, string(" %s", tr("Compass")), "");
		if (compass_handle->changed) {
			context_raw->ddirty = 2;
		}

		context_raw->show_envmap_handle->b = context_raw->show_envmap;
		context_raw->show_envmap           = ui_check(context_raw->show_envmap_handle, string(" %s", tr("Envmap")), "");
		if (context_raw->show_envmap_handle->changed) {
			context_load_envmap();
			context_raw->ddirty = 2;
		}

		context_raw->show_envmap_blur_handle->b = context_raw->show_envmap_blur;
		context_raw->show_envmap_blur           = ui_check(context_raw->show_envmap_blur_handle, string(" %s", tr("Blur Envmap")), "");
		if (context_raw->show_envmap_blur_handle->changed) {
			context_raw->ddirty = 2;
		}

		if (ui_menu_button(tr("Reset Envmap"), "", ICON_NONE)) {
			project_set_default_envmap();
		}

		if (ui_menu_button(tr("Capture Screenshot"), "", ICON_PHOTO)) {
			sys_notify_on_next_frame(&ui_menubar_draw_category_items_capture_screenshot, NULL);
			ui->changed = false; // Close menu
		}

		context_update_envmap();

		if (ui->changed) {
			ui_menu_keep_open = true;
		}
	}
	else if (ui_menubar_category == MENUBAR_CATEGORY_MODE) {
		ui_handle_t *mode_handle = ui_handle(__ID__);
		mode_handle->i           = context_raw->viewport_mode;
		string_t_array_t *modes  = any_array_create_from_raw(
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

		if (config_raw->workflow == WORKFLOW_BASE) {
			array_splice(modes, 9, 1);
			array_splice(modes, 8, 1);
			array_splice(modes, 7, 1);
			array_splice(modes, 5, 1);
			array_splice(modes, 4, 1);
			array_splice(modes, 3, 1);
			array_splice(modes, 2, 1);
			array_splice(shortcuts, 9, 1);
			array_splice(shortcuts, 8, 1);
			array_splice(shortcuts, 7, 1);
			array_splice(shortcuts, 5, 1);
			array_splice(shortcuts, 4, 1);
			array_splice(shortcuts, 3, 1);
			array_splice(shortcuts, 2, 1);
			mode_handle->i = string_array_index_of(modes, ui_menubar_viewport_mode_to_string(mode_handle->i));
		}

		for (i32 i = 0; i < modes->length; ++i) {
			char *shortcut = config_raw->touch_ui ? "" : string("%s, %s", any_map_get(config_keymap, "viewport_mode"), shortcuts->buffer[i]);
			ui_radio(mode_handle, i, modes->buffer[i], shortcut);
		}

		if (mode_handle->changed) {
			viewport_mode_t m = ui_menubar_string_to_viewport_mode(modes->buffer[mode_handle->i]);
			context_set_viewport_mode(m);
			ui_menu_keep_open = true;
		}
	}
	else if (ui_menubar_category == MENUBAR_CATEGORY_CAMERA) {
		if (ui_menu_button(tr("Reset"), any_map_get(config_keymap, "view_reset"), ICON_NONE)) {
			viewport_reset();
			viewport_scale_to_bounds(2.0);
		}
		ui_menu_separator();

		bool full = true;

#ifdef IRON_IOS
		if (config_is_iphone()) {
			full = false;
		}
#endif

		if (full) {

			if (ui_menu_sub_button(ui_handle(__ID__), tr("View"))) {
				ui_menu_sub_begin(6);
				if (ui_menu_button(tr("Front"), any_map_get(config_keymap, "view_front"), ICON_NONE)) {
					viewport_set_view(0, -1, 0, math_pi() / 2.0, 0, 0);
				}
				if (ui_menu_button(tr("Back"), any_map_get(config_keymap, "view_back"), ICON_NONE)) {
					viewport_set_view(0, 1, 0, math_pi() / 2.0, 0, math_pi());
				}
				if (ui_menu_button(tr("Right"), any_map_get(config_keymap, "view_right"), ICON_NONE)) {
					viewport_set_view(1, 0, 0, math_pi() / 2.0, 0, math_pi() / 2.0);
				}
				if (ui_menu_button(tr("Left"), any_map_get(config_keymap, "view_left"), ICON_NONE)) {
					viewport_set_view(-1, 0, 0, math_pi() / 2.0, 0, -math_pi() / 2.0);
				}
				if (ui_menu_button(tr("Top"), any_map_get(config_keymap, "view_top"), ICON_NONE)) {
					viewport_set_view(0, 0, 1, 0, 0, 0);
				}
				if (ui_menu_button(tr("Bottom"), any_map_get(config_keymap, "view_bottom"), ICON_NONE)) {
					viewport_set_view(0, 0, -1, math_pi(), 0, math_pi());
				}
				ui_menu_sub_end();
			}

			ui->changed = false;

			if (ui_menu_sub_button(ui_handle(__ID__), tr("Orbit"))) {
				ui_menu_sub_begin(5);
				if (ui_menu_button(tr("Left"), any_map_get(config_keymap, "view_orbit_left"), ICON_ARROW_LEFT)) {
					viewport_orbit(-math_pi() / 12.0, 0);
				}
				if (ui_menu_button(tr("Right"), any_map_get(config_keymap, "view_orbit_right"), ICON_ARROW_RIGHT)) {
					viewport_orbit(math_pi() / 12.0, 0);
				}
				if (ui_menu_button(tr("Up"), any_map_get(config_keymap, "view_orbit_up"), ICON_ARROW_UP)) {
					viewport_orbit(0, -math_pi() / 12.0);
				}
				if (ui_menu_button(tr("Down"), any_map_get(config_keymap, "view_orbit_down"), ICON_ARROW_DOWN)) {
					viewport_orbit(0, math_pi() / 12.0);
				}
				if (ui_menu_button(tr("Opposite"), any_map_get(config_keymap, "view_orbit_opposite"), ICON_NONE)) {
					viewport_orbit_opposite();
				}
				ui_menu_sub_end();
			}
		}

		if (ui_menu_button(tr("Zoom In"), any_map_get(config_keymap, "view_zoom_in"), ICON_ZOOM_IN)) {
			viewport_zoom(0.2);
		}
		if (ui_menu_button(tr("Zoom Out"), any_map_get(config_keymap, "view_zoom_out"), ICON_ZOOM_OUT)) {
			viewport_zoom(-0.2);
		}

		camera_object_t *cam       = scene_camera;
		context_raw->fov_handle->f = math_floor(cam->data->fov * 100) / 100.0;
		ui_menu_align();
		cam->data->fov = ui_slider(context_raw->fov_handle, tr("FoV"), 0.3, 1.4, true, 100.0, true, UI_ALIGN_RIGHT, true);
		if (context_raw->fov_handle->changed) {
			viewport_update_camera_type(context_raw->camera_type);
		}

		ui_menu_separator();
		ui_menu_align();
		ui_menu_label(tr("Pivot"), any_map_get(config_keymap, "view_pivot_center"));
		ui_menu_align();
		ui_handle_t      *h                  = ui_handle(__ID__);
		string_t_array_t *pivot_center_items = any_array_create_from_raw(
		    (void *[]){
		        tr("Camera Center"), // tr("Paint Stroke")
		    },
		    1);
		ui_inline_radio(h, pivot_center_items, UI_ALIGN_LEFT);

		ui_menu_separator();
		ui_menu_align();
		ui_menu_label(tr("Mode"), NULL);
		ui_menu_align();
		ui_handle_t *camera_controls_handle     = ui_handle(__ID__);
		camera_controls_handle->i               = context_raw->camera_controls;
		string_t_array_t *camera_controls_items = any_array_create_from_raw(
		    (void *[]){
		        tr("Orbit"),
		        tr("Rotate"),
		        tr("Fly"),
		    },
		    3);
		context_raw->camera_controls = ui_inline_radio(camera_controls_handle, camera_controls_items, UI_ALIGN_LEFT);

		any_map_t *vars = any_map_create();
		any_map_set(vars, "rotate_shortcut", any_map_get(config_keymap, "action_rotate"));
		any_map_set(vars, "zoom_shortcut", any_map_get(config_keymap, "action_zoom"));
		any_map_set(vars, "pan_shortcut", any_map_get(config_keymap, "action_pan"));
		char *orbit_and_rotate_tooltip = vtr("Orbit and Rotate mode:\n{rotate_shortcut} or move right mouse button to rotate.\n{zoom_shortcut} or scroll to "
		                                    "zoom.\n{pan_shortcut} or move middle mouse to pan.",
		                                    vars);
		char *fly_tooltip = tr("Fly mode:\nHold the right mouse button and one of the following commands:\nmove mouse to rotate.\nw, up or scroll up to "
		                       "move forward.\ns, down or scroll down to move backward.\na or left to move left.\nd or right to move right.\ne to move "
		                       "up.\nq to move down.\nHold shift to move faster or alt to move slower.");
		if (ui->is_hovered) {
			ui_tooltip(string("%s\n\n%s", orbit_and_rotate_tooltip, fly_tooltip));
		}

		ui_menu_separator();
		ui_menu_align();
		ui_menu_label(tr("Type"), any_map_get(config_keymap, "view_camera_type"));
		ui_menu_align();
		string_t_array_t *camera_type_items = any_array_create_from_raw(
		    (void *[]){
		        tr("Perspective"),
		        tr("Orthographic"),
		    },
		    2);
		context_raw->camera_type = ui_inline_radio(context_raw->cam_handle, camera_type_items, UI_ALIGN_LEFT);
		if (context_raw->cam_handle->changed) {
			viewport_update_camera_type(context_raw->camera_type);
		}

		if (ui->changed) {
			ui_menu_keep_open = true;
		}
	}
	else if (ui_menubar_category == MENUBAR_CATEGORY_HELP) {
		if (ui_menu_button(tr("Manual"), "", ICON_HELP)) {
			iron_load_url(string("%s/manual", manifest_url));
		}
		if (ui_menu_button(tr("How To"), "", ICON_HELP)) {
			iron_load_url(string("%s/howto", manifest_url));
		}
		if (ui_menu_button(tr("What's New"), "", ICON_LINK)) {
			iron_load_url(string("%s/notes", manifest_url));
		}
		if (ui_menu_button(tr("Issue Tracker"), "", ICON_LINK)) {
			iron_load_url("https://github.com/armory3d/armortools/issues");
		}
		if (ui_menu_button(tr("Report Bug"), "", ICON_LINK)) {
#if defined(IRON_MACOS) || defined(IRON_IOS) // Limited url length
			iron_load_url(string("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*%s%%20%s-%s,%%20%s", manifest_title,
			                     manifest_version, config_get_sha(), iron_system_id()));
#else
			iron_load_url(string("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*%s%%20%s-%s,%%20%s*%%0A%%0A**Issue "
			                     "description:**%%0A%%0A**Steps to reproduce:**%%0A%%0A",
			                     manifest_title, manifest_version, config_get_sha(), iron_system_id()));
#endif
		}
		if (ui_menu_button(tr("Request Feature"), "", ICON_LINK)) {
#if defined(IRON_MACOS) || defined(IRON_IOS) // Limited url length
			iron_load_url(
			    string("https://github.com/armory3d/armortools/issues/new?labels=feature%%20request&template=feature_request.md&body=*%s%%20%s-%s,%%20%s",
			           manifest_title, manifest_version, config_get_sha(), iron_system_id()));
#else
			iron_load_url(string("https://github.com/armory3d/armortools/issues/"
			                     "new?labels=feature%%20request&template=feature_request.md&body=*%s%%20%s-%s,%%20%s*%%0A%%0A**Feature description:**%%0A%%0A",
			                     manifest_title, manifest_version, config_get_sha(), iron_system_id()));
#endif
		}
		ui_menu_separator();

		if (ui_menu_button(tr("Check for Updates..."), "", ICON_NONE)) {
#ifdef IRON_ANDROID
			iron_load_url(manifest_url_android);
#elif defined(IRON_IOS)
			iron_load_url(manifest_url_ios);
#else
			// Retrieve latest version number
			iron_file_download("https://check-for-updates.armory3d.workers.dev", &ui_menubar_draw_category_items_on_check_for_updates_downloaded, 0, NULL);
#endif
		}

		if (ui_menu_button(tr("About..."), "", ICON_INFO)) {

			char *msg = string("%s.org - v%s (%s) - %s\n", manifest_title, manifest_version, config_get_date(), config_get_sha());
			msg       = string("%s%s - %s", msg, iron_system_id(), strings_graphics_api());

			char *gpu = gpu_device_name();
			msg       = string("%s\n%s", msg, gpu);

			gc_unroot(_ui_menu_render_msg);
			_ui_menu_render_msg = string_copy(msg);
			gc_root(_ui_menu_render_msg);
			ui_box_show_custom(&ui_menubar_draw_category_items_about_box, 400, 320, NULL, true, tr("About"));
		}
	}
	else if (ui_menubar_category == MENUBAR_CATEGORY_WORKSPACE) {
		ui_handle_t *workspace_handle = ui_handle(__ID__);
		workspace_handle->i           = config_raw->workspace;
		string_t_array_t *modes       = any_array_create_from_raw(
            (void *[]){
                tr("Paint 3D"),
                tr("Paint 2D"),
                tr("Nodes"),
                tr("Script"),
            },
            4);

		if (config_raw->experimental) {
			any_array_push(modes, tr("Sculpt"));
			any_array_push(modes, tr("Player"));
		}

		for (i32 i = 0; i < modes->length; ++i) {
			ui_radio(workspace_handle, i, modes->buffer[i], "");
		}

		if (workspace_handle->changed) {
			config_raw->workspace = workspace_handle->i;
			config_save();
			base_update_workspace();
		}

		ui_menu_separator();
		ui_menu_align();
		ui_menu_label(tr("Workflow"), NULL);
		ui_menu_align();
		ui_handle_t *workflow_handle     = ui_handle(__ID__);
		workflow_handle->i               = config_raw->workflow;
		string_t_array_t *workflow_items = any_array_create_from_raw(
		    (void *[]){
		        tr("PBR"),
		        tr("Base"),
		    },
		    2);
		config_raw->workflow = ui_inline_radio(workflow_handle, workflow_items, UI_ALIGN_LEFT);
		if (workflow_handle->changed) {
			config_raw->workflow = workflow_handle->i;
			config_save();
			base_update_workflow();
		}
	}
}

void ui_menubar_show_menu(i32 category) {
	if (ui_menu_show && ui_menubar_category == category) {
		return;
	}

	ui_menu_show       = true;
	ui_menu_show_first = true;
	gc_unroot(ui_menu_commands);
	ui_menu_commands = ui_menubar_draw_category_items;
	gc_root(ui_menu_commands);
	ui_menubar_category = category;

	i32 panel_x = ui_menu_panel_x();
	i32 panel_y = ui_menu_panel_y();
	ui_menu_x   = math_floor(ui->_x - ui->_w) + panel_x;
	ui_menu_y   = math_floor(ui_MENUBAR_H(ui)) + panel_y + 2;
	if (config_raw->touch_ui) {
		i32 menu_w = math_floor(base_default_element_w * UI_SCALE() * 2.0);
		ui_menu_x -= math_floor((menu_w - ui->_w) / 2.0) + math_floor(ui_header_h / 2.0);
		ui_menu_x += math_floor(2 * UI_SCALE());
		ui_menu_y -= math_floor(2 * UI_SCALE());
		ui_menu_keep_open = true;
	}
}

bool ui_menubar_icon_button(i32 i) {
	u32            col         = ui->ops->theme->WINDOW_BG_COL;
	bool           light       = col > 0xff666666;
	i32            icon_accent = light ? 0xff666666 : 0xffaaaaaa;
	gpu_texture_t *img         = resource_get("icons.k");
	rect_t        *rect        = resource_tile50(img, i);
	return ui_sub_image(img, icon_accent, -1.0, rect->x, rect->y, rect->w, rect->h) == UI_STATE_RELEASED;
}
