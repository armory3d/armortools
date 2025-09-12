
let ui_menubar_default_w: i32 = 330;
let ui_menubar_workspace_handle: ui_handle_t = ui_handle_create();
let ui_menubar_menu_handle: ui_handle_t = ui_handle_create();
let ui_menubar_w: i32 = ui_menubar_default_w;
let ui_menubar_category: i32 = 0;

let _ui_menubar_saved_camera: mat4_t = mat4_nan();
let _ui_menubar_plane: mesh_object_t = null;

type update_info_t = {
	version: i32;
	version_name: string;
};

function ui_menubar_init() {
	ui_menubar_workspace_handle.layout = ui_layout_t.HORIZONTAL;
	ui_menubar_menu_handle.layout = ui_layout_t.HORIZONTAL;
}

function ui_menu_panel_x(): i32 {
	let panel_x: i32 = sys_x();
	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		let item_w: i32 = ui_toolbar_w();
		panel_x = sys_x() - item_w;
	}
	else {
		panel_x += 5 * UI_SCALE();
	}
	return panel_x;
}

function ui_menu_panel_y(): i32 {
	let panel_y: i32 = 0;
	if (config_raw.layout[layout_size_t.HEADER] == 1) {
	}
	else {
		panel_y += 5 * UI_SCALE();
	}
	return panel_y;
}

function ui_menubar_render_ui() {
	let item_w: i32 = ui_toolbar_w();
	let panel_x: i32 = ui_menu_panel_x();
	let panel_y: i32 = ui_menu_panel_y();

	if (ui_window(ui_menubar_menu_handle, panel_x, panel_y, ui_menubar_w, ui_header_h)) {
		ui._x += 1; // Prevent "File" button highlight on startup

		ui_begin_menu();

		if (config_raw.touch_ui) {
			ui._w = item_w;

			if (ui_menubar_icon_button(0, 2)) box_preferences_show();
			if (ui_menubar_icon_button(0, 3)) {
				// Save project icon in lit mode
				context_set_viewport_mode(viewport_mode_t.LIT);
				console_toast(tr("Saving project"));
				sys_notify_on_next_frame(function () {
					sys_notify_on_end_frame(function () {
						project_save();
						box_projects_show();
					});
				});
			}
			if (ui_menubar_icon_button(4, 2)) {
				project_import_asset();
			}
			if (ui_menubar_icon_button(5, 2)) {
				box_export_show_textures();
			}
			let size: i32 = math_floor(ui._w / UI_SCALE());
			if (ui_menu_show && ui_menubar_category == menubar_category_t.VIEWPORT) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(8, 2)) {
				ui_menubar_show_menu(menubar_category_t.VIEWPORT);
			}
			if (ui_menu_show && ui_menubar_category == menubar_category_t.MODE) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(9, 2)) {
				ui_menubar_show_menu(menubar_category_t.MODE);
			}
			if (ui_menu_show && ui_menubar_category == menubar_category_t.CAMERA) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(10, 2)) {
				ui_menubar_show_menu(menubar_category_t.CAMERA);
			}
			if (ui_menu_show && ui_menubar_category == menubar_category_t.HELP) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(11, 2)) {
				ui_menubar_show_menu(menubar_category_t.HELP);
			}
			ui.enabled = history_undos > 0;
			if (ui_menubar_icon_button(6, 2)) {
				history_undo();
			}
			ui.enabled = history_redos > 0;
			if (ui_menubar_icon_button(7, 2)) {
				history_redo();
			}
			ui.enabled = true;
		}
		else {
			let categories: string[] = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
			for (let i: i32 = 0; i < categories.length; ++i) {
				if (_ui_menu_button(categories[i]) || (ui_menu_show && ui_menu_commands == ui_menubar_draw_category_items && ui.is_hovered)) {
					ui_menubar_show_menu(i);
				}
			}
		}

		if (ui_menubar_w < ui._x + 10) {
			ui_menubar_w = math_floor(ui._x + 10);
			ui_toolbar_handle.redraws = 2;
		}

		ui_end_menu();
	}

	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		// Non-floating header
		ui_menubar_draw_tab_header();
	}
}

function ui_menubar_draw_tab_header() {
	let item_w: i32 = ui_toolbar_w();
	let panel_x: i32 = sys_x();

	let nodesw: i32 = (ui_nodes_show || ui_view2d_show) ? config_raw.layout[layout_size_t.NODES_W] : 0;
	let ww: i32 = iron_window_width() - config_raw.layout[layout_size_t.SIDEBAR_W] - ui_menubar_w - nodesw;
	panel_x = (sys_x() - item_w) + ui_menubar_w;

	if (ui_window(ui_menubar_workspace_handle, panel_x, 0, ww, ui_header_h)) {

		if (!config_raw.touch_ui) {
			ui_tab(ui_header_worktab, tr("3D View"));
		}
		else {
			ui_fill(0, 0, ui._window_w, ui._window_h + 4, ui.ops.theme.SEPARATOR_COL);
		}
	}
}

function ui_menubar_draw_category_items() {
	if (ui_menubar_category == menubar_category_t.FILE) {
		if (ui_menu_button(tr("New Project..."), map_get(config_keymap, "file_new"))) {
			project_new_box();
		}
		if (ui_menu_button(tr("Open..."), map_get(config_keymap, "file_open"))) {
			project_open();
		}
		if (ui_menu_button(tr("Open Recent..."), map_get(config_keymap, "file_open_recent"))) {
			box_projects_show();
		}
		if (ui_menu_button(tr("Save"), map_get(config_keymap, "file_save"))) {
			project_save();
		}
		if (ui_menu_button(tr("Save As..."), map_get(config_keymap, "file_save_as"))) {
			project_save_as();
		}
		ui_menu_separator();
		if (ui_menu_button(tr("Import Texture..."), map_get(config_keymap, "file_import_assets"))) {
			project_import_asset(string_array_join(path_texture_formats, ","), false);
		}
		if (ui_menu_button(tr("Import Envmap..."))) {
			ui_files_show("hdr", false, false, function (path: string) {
				if (!ends_with(path, ".hdr")) {
					console_error(tr("Error: .hdr file expected"));
					return;
				}
				import_asset_run(path);
			});
		}

		if (ui_menu_button(tr("Import Font..."))) {
			project_import_asset("ttf,ttc,otf");
		}
		if (ui_menu_button(tr("Import Material..."))) {
			project_import_material();
		}
		if (ui_menu_button(tr("Import Brush..."))) {
			project_import_brush();
		}

		if (ui_menu_button(tr("Import Swatches..."))) {
			project_import_swatches();
		}
		if (ui_menu_button(tr("Import Mesh..."))) {
			project_import_mesh();
		}
		if (ui_menu_button(tr("Reimport Mesh"), map_get(config_keymap, "file_reimport_mesh"))) {
			project_reimport_mesh();
		}
		if (ui_menu_button(tr("Reimport Textures"), map_get(config_keymap, "file_reimport_textures"))) {
			project_reimport_textures();
		}
		ui_menu_separator();
		if (ui_menu_button(tr("Export Textures..."), map_get(config_keymap, "file_export_textures_as"))) {
			context_raw.layers_export = export_mode_t.VISIBLE;
			box_export_show_textures();
		}
		if (ui_menu_button(tr("Export Swatches..."))) {
			project_export_swatches();
		}
		if (ui_menu_button(tr("Export Mesh..."))) {
			context_raw.export_mesh_index = 0; // All
			box_export_show_mesh();
		}
		if (ui_menu_button(tr("Bake Material..."))) {
			box_export_show_bake_material();
		}

		ui_menu_separator();
		if (ui_menu_button(tr("Exit"))) {
			iron_stop();
		}
	}
	else if (ui_menubar_category == menubar_category_t.EDIT) {
		let step_undo: string = "";
		let step_redo: string = "";
		if (history_undos > 0) {
			step_undo = history_steps[history_steps.length - 1 - history_redos].name;
		}
		if (history_redos > 0) {
			step_redo = history_steps[history_steps.length - history_redos].name;
		}

		ui.enabled = history_undos > 0;
		let vars_undo: map_t<string, string> = map_create();
		map_set(vars_undo, "step", step_undo);
		if (ui_menu_button(tr("Undo {step}", vars_undo), map_get(config_keymap, "edit_undo"))) {
			history_undo();
		}

		ui.enabled = history_redos > 0;
		let vars_redo: map_t<string, string> = map_create();
		map_set(vars_redo, "step", step_redo);
		if (ui_menu_button(tr("Redo {step}", vars_redo), map_get(config_keymap, "edit_redo"))) {
			history_redo();
		}
		ui.enabled = true;
		ui_menu_separator();
		if (ui_menu_button(tr("Preferences..."), map_get(config_keymap, "edit_prefs"))) {
			box_preferences_show();
		}
	}
	else if (ui_menubar_category == menubar_category_t.VIEWPORT) {
		if (ui_menu_button(tr("Distract Free"), map_get(config_keymap, "view_distract_free"))) {
			ui_base_toggle_distract_free();
			ui.is_hovered = false;
		}

		///if !(arm_android || arm_ios)
		if (ui_menu_button(tr("Toggle Fullscreen"), "alt+enter")) {
			base_toggle_fullscreen();
		}
		///end

		ui.changed = false;

		let p: world_data_t = scene_world;
		let env_handle: ui_handle_t = ui_handle(__ID__);
		env_handle.value = p.strength;
		ui_menu_align();
		p.strength = ui_slider(env_handle, tr("Environment"), 0.0, 8.0, true);
		if (env_handle.changed) {
			context_raw.ddirty = 2;
		}

		let enva_handle: ui_handle_t = ui_handle(__ID__);
		enva_handle.value = context_raw.envmap_angle / math_pi() * 180.0;
		if (enva_handle.value < 0) {
			enva_handle.value += (math_floor(-enva_handle.value / 360) + 1) * 360;
		}
		else if (enva_handle.value > 360) {
			enva_handle.value -= math_floor(enva_handle.value / 360) * 360;
		}
		ui_menu_align();
		context_raw.envmap_angle = ui_slider(enva_handle, tr("Environment Angle"), 0.0, 360.0, true, 1) / 180.0 * math_pi();
		if (ui.is_hovered) {
			let vars: map_t<string, string> = map_create();
			map_set(vars, "shortcut", map_get(config_keymap, "rotate_envmap"));
			ui_tooltip(tr("{shortcut} and move mouse", vars));
		}
		if (enva_handle.changed) {
			context_raw.ddirty = 2;
		}

		let split_view_handle: ui_handle_t = ui_handle(__ID__);
		if (split_view_handle.init) {
			split_view_handle.selected = context_raw.split_view;
		}
		context_raw.split_view = ui_check(split_view_handle, " " + tr("Split View"));
		if (split_view_handle.changed) {
			base_resize();
		}

		let cull_handle: ui_handle_t = ui_handle(__ID__);
		if (cull_handle.init) {
			cull_handle.selected = context_raw.cull_backfaces;
		}
		context_raw.cull_backfaces = ui_check(cull_handle, " " + tr("Cull Backfaces"));
		if (cull_handle.changed) {
			make_material_parse_mesh_material();
		}

		let filter_handle: ui_handle_t = ui_handle(__ID__);
		if (filter_handle.init) {
			filter_handle.selected = context_raw.texture_filter;
		}
		context_raw.texture_filter = ui_check(filter_handle, " " + tr("Filter Textures"));
		if (filter_handle.changed) {
			gpu_use_linear_sampling(context_raw.texture_filter);
		}

		context_raw.draw_wireframe = ui_check(context_raw.wireframe_handle, " " + tr("Wireframe"));
		if (context_raw.wireframe_handle.changed) {
			let current: gpu_texture_t = _draw_current;
			draw_end();
			util_uv_cache_uv_map();
			draw_begin(current);
			make_material_parse_mesh_material();
		}

		context_raw.draw_texels = ui_check(context_raw.texels_handle, " " + tr("Texels"));
		if (context_raw.texels_handle.changed) {
			make_material_parse_mesh_material();
		}

		let compass_handle: ui_handle_t = ui_handle(__ID__);
		if (compass_handle.init) {
			compass_handle.selected = context_raw.show_compass;
		}
		context_raw.show_compass = ui_check(compass_handle, " " + tr("Compass"));
		if (compass_handle.changed) {
			context_raw.ddirty = 2;
		}

		context_raw.show_envmap_handle.selected = context_raw.show_envmap;
		context_raw.show_envmap = ui_check(context_raw.show_envmap_handle, " " + tr("Envmap"));
		if (context_raw.show_envmap_handle.changed) {
			context_load_envmap();
			context_raw.ddirty = 2;
		}

		context_raw.show_envmap_blur = ui_check(context_raw.show_envmap_blur_handle, " " + tr("Blur Envmap"));
		if (context_raw.show_envmap_blur_handle.changed) {
			context_raw.ddirty = 2;
		}

		if (ui_menu_button(tr("Reset Envmap"))) {
			project_set_default_envmap();
		}

		context_update_envmap();

		if (ui.changed) {
			ui_menu_keep_open = true;
		}
	}
	else if (ui_menubar_category == menubar_category_t.MODE) {
		let mode_handle: ui_handle_t = ui_handle(__ID__);
		mode_handle.position = context_raw.viewport_mode;
		let modes: string[] = [
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
			tr("Mask")
		];
		let shortcuts: string[] = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];

		if (gpu_raytrace_supported()) {
			array_push(modes, tr("Path Traced"));
			array_push(shortcuts, "p");
		}

		for (let i: i32 = 0; i < modes.length; ++i) {
			let shortcut: string = config_raw.touch_ui ? "" : map_get(config_keymap, "viewport_mode") + ", " + shortcuts[i];
			ui_radio(mode_handle, i, modes[i], shortcut);
		}

		if (mode_handle.changed) {
			context_set_viewport_mode(mode_handle.position);
			ui_menu_keep_open = true;
		}
	}
	else if (ui_menubar_category == menubar_category_t.CAMERA) {
		if (ui_menu_button(tr("Reset"), map_get(config_keymap, "view_reset"))) {
			viewport_reset();
			viewport_scale_to_bounds();
		}
		ui_menu_separator();
		if (ui_menu_button(tr("Front"), map_get(config_keymap, "view_front"))) {
			viewport_set_view(0, -1, 0, math_pi() / 2, 0, 0);
		}
		if (ui_menu_button(tr("Back"), map_get(config_keymap, "view_back"))) {
			viewport_set_view(0, 1, 0, math_pi() / 2, 0, math_pi());
		}
		if (ui_menu_button(tr("Right"), map_get(config_keymap, "view_right"))) {
			viewport_set_view(1, 0, 0, math_pi() / 2, 0, math_pi() / 2);
		}
		if (ui_menu_button(tr("Left"), map_get(config_keymap, "view_left"))) {
			viewport_set_view(-1, 0, 0, math_pi() / 2, 0, -math_pi() / 2);
		}
		if (ui_menu_button(tr("Top"), map_get(config_keymap, "view_top"))) {
			viewport_set_view(0, 0, 1, 0, 0, 0);
		}
		if (ui_menu_button(tr("Bottom"), map_get(config_keymap, "view_bottom"))) {
			viewport_set_view(0, 0, -1, math_pi(), 0, math_pi());
		}
		ui_menu_separator();

		ui.changed = false;

		if (ui_menu_button(tr("Orbit Left"), map_get(config_keymap, "view_orbit_left"))) {
			viewport_orbit(-math_pi() / 12, 0);
		}
		if (ui_menu_button(tr("Orbit Right"), map_get(config_keymap, "view_orbit_right"))) {
			viewport_orbit(math_pi() / 12, 0);
		}
		if (ui_menu_button(tr("Orbit Up"), map_get(config_keymap, "view_orbit_up"))) {
			viewport_orbit(0, -math_pi() / 12);
		}
		if (ui_menu_button(tr("Orbit Down"), map_get(config_keymap, "view_orbit_down"))) {
			viewport_orbit(0, math_pi() / 12);
		}
		if (ui_menu_button(tr("Orbit Opposite"), map_get(config_keymap, "view_orbit_opposite"))) {
			viewport_orbit_opposite();
		}
		if (ui_menu_button(tr("Zoom In"), map_get(config_keymap, "view_zoom_in"))) {
			viewport_zoom(0.2);
		}
		if (ui_menu_button(tr("Zoom Out"), map_get(config_keymap, "view_zoom_out"))) {
			viewport_zoom(-0.2);
		}

		let cam: camera_object_t = scene_camera;
		context_raw.fov_handle = ui_handle(__ID__);
		if (context_raw.fov_handle.init) {
			context_raw.fov_handle.value = math_floor(cam.data.fov * 100) / 100;
		}
		ui_menu_align();
		cam.data.fov = ui_slider(context_raw.fov_handle, tr("FoV"), 0.3, 1.4, true);
		if (context_raw.fov_handle.changed) {
			viewport_update_camera_type(context_raw.camera_type);
		}

		ui_menu_align();
		let camera_controls_handle: ui_handle_t = ui_handle(__ID__);
		camera_controls_handle.position = context_raw.camera_controls;
		let camera_controls_items: string[] = [tr("Orbit"), tr("Rotate"), tr("Fly")];
		context_raw.camera_controls = ui_inline_radio(camera_controls_handle, camera_controls_items, ui_align_t.LEFT);

		let vars: map_t<string, string> = map_create();
		map_set(vars, "rotate_shortcut", map_get(config_keymap, "action_rotate"));
		map_set(vars, "zoom_shortcut", map_get(config_keymap, "action_zoom"));
		map_set(vars, "pan_shortcut", map_get(config_keymap, "action_pan"));
		let orbit_and_rotate_tooltip: string = tr("Orbit and Rotate mode:\n{rotate_shortcut} or move right mouse button to rotate.\n{zoom_shortcut} or scroll to zoom.\n{pan_shortcut} or move middle mouse to pan.", vars);
		let fly_tooltip: string = tr("Fly mode:\nHold the right mouse button and one of the following commands:\nmove mouse to rotate.\nw, up or scroll up to move forward.\ns, down or scroll down to move backward.\na or left to move left.\nd or right to move right.\ne to move up.\nq to move down.\nHold shift to move faster or alt to move slower.");
		if (ui.is_hovered) {
			ui_tooltip(orbit_and_rotate_tooltip + "\n\n" + fly_tooltip);
		}

		ui_menu_align();
		let camera_type_items: string[] = [tr("Perspective"), tr("Orthographic")];
		context_raw.camera_type = ui_inline_radio(context_raw.cam_handle, camera_type_items, ui_align_t.LEFT);
		if (ui.is_hovered) {
			ui_tooltip(tr("Camera Type") + " (" + map_get(config_keymap, "view_camera_type") + ")");
		}
		if (context_raw.cam_handle.changed) {
			viewport_update_camera_type(context_raw.camera_type);
		}

		if (ui.changed) {
			ui_menu_keep_open = true;
		}
	}
	else if (ui_menubar_category == menubar_category_t.HELP) {
		if (ui_menu_button(tr("Manual"))) {
			iron_load_url(manifest_url + "/manual");
		}
		if (ui_menu_button(tr("How To"))) {
			iron_load_url(manifest_url + "/howto");
		}
		if (ui_menu_button(tr("What's New"))) {
			iron_load_url(manifest_url + "/notes");
		}
		if (ui_menu_button(tr("Issue Tracker"))) {
			iron_load_url("https://github.com/armory3d/armortools/issues");
		}
		if (ui_menu_button(tr("Report Bug"))) {
			///if (arm_macos || arm_ios) // Limited url length
			iron_load_url("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + iron_system_id());
			///else
			iron_load_url("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + iron_system_id() + "*%0A%0A**Issue description:**%0A%0A**Steps to reproduce:**%0A%0A");
			///end
		}
		if (ui_menu_button(tr("Request Feature"))) {
			///if (arm_macos || arm_ios) // Limited url length
			iron_load_url("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + iron_system_id());
			///else
			iron_load_url("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + iron_system_id() + "*%0A%0A**Feature description:**%0A%0A");
			///end
		}
		ui_menu_separator();

		if (ui_menu_button(tr("Check for Updates..."))) {
			///if arm_android
			iron_load_url(manifest_url_android);
			///elseif arm_ios
			iron_load_url(manifest_url_ios);
			///else
			// Retrieve latest version number
			iron_file_download("https://server.armorpaint.org/" + to_lower_case(manifest_title) + ".html", function (url: string, buffer: buffer_t) {
				if (buffer != null)  {
					// Compare versions
					let update: update_info_t = json_parse(sys_buffer_to_string(buffer));
					let update_version: i32 = math_floor(update.version);
					if (update_version > 0) {
						let date: string = config_get_date(); // 2019 -> 19
						date = substring(date, 2, date.length);
						let date_int: i32 = parse_int(string_replace_all(date, "-", ""));
						if (update_version > date_int) {
							let vars: map_t<string, string> = map_create();
							map_set(vars, "url", manifest_url);
							ui_box_show_message(tr("Update"), tr("Update is available!\nPlease visit {url}.", vars));
						}
						else {
							ui_box_show_message(tr("Update"), tr("You are up to date!"));
						}
					}
				}
				else {
					let vars: map_t<string, string> = map_create();
					map_set(vars, "url", manifest_url);
					ui_box_show_message(tr("Update"), tr("Unable to check for updates.\nPlease visit {url}.", vars));
				}
			});
			///end
		}

		if (ui_menu_button(tr("About..."))) {

			let msg: string = manifest_title + ".org - v" + manifest_version + " (" + config_get_date() + ") - " + config_get_sha() + "\n";
			msg += iron_system_id() + " - " + strings_graphics_api();

			let gpu: string = gpu_device_name();
			msg += "\n" + gpu;

			_ui_menu_render_msg = msg;

			ui_box_show_custom(function () {
				let tab_vertical: bool = config_raw.touch_ui;
				if (ui_tab(ui_handle(__ID__), tr("About"), tab_vertical)) {

					let img: gpu_texture_t = data_get_image("badge.k");
					ui_image(img);
					ui_end_element();

					let h: ui_handle_t = ui_handle(__ID__);
					if (h.init) {
						h.text = _ui_menu_render_msg;
					}
					ui_text_area(h, ui_align_t.LEFT, false);

					ui_row3();

					///if (arm_windows || arm_linux || arm_macos)
					if (ui_button(tr("Copy"))) {
						iron_copy_to_clipboard(_ui_menu_render_msg);
					}
					///else
					ui_end_element();
					///end

					if (ui_button(tr("Contributors"))) {
						iron_load_url("https://github.com/armory3d/armortools/graphs/contributors");
					}
					if (ui_button(tr("OK"))) {
						ui_box_hide();
					}
				}
			}, 400, 320);
		}
	}
}

function ui_menubar_show_menu(category: i32) {
	if (ui_menu_show && ui_menubar_category == category) {
		return;
	}

	ui_menu_show = true;
	ui_menu_show_first = true;
	ui_menu_commands = ui_menubar_draw_category_items;
	ui_menubar_category = category;

	let panel_x: i32 = ui_menu_panel_x();
	let panel_y: i32 = ui_menu_panel_y();
	ui_menu_x = math_floor(ui._x - ui._w) + panel_x;
	ui_menu_y = math_floor(ui_MENUBAR_H(ui)) + panel_y;
	if (config_raw.touch_ui) {
		let menu_w: i32 = math_floor(base_default_element_w * UI_SCALE() * 2.0);
		ui_menu_x -= math_floor((menu_w - ui._w) / 2) + math_floor(ui_header_h / 2);
		ui_menu_x += math_floor(2 * UI_SCALE());
		ui_menu_y -= math_floor(2 * UI_SCALE());
		ui_menu_keep_open = true;
	}
}

function ui_menubar_icon_button(i: i32, j: i32): bool {
	let col: u32 = ui.ops.theme.WINDOW_BG_COL;
	let light: bool = col > 0xff666666 ;
	let icon_accent: i32 = light ? 0xff666666 : 0xffaaaaaa;
	let img: gpu_texture_t = resource_get("icons.k");
	let rect: rect_t = resource_tile50(img, i, j);
	return ui_sub_image(img, icon_accent, -1.0, rect.x, rect.y, rect.w, rect.h) == ui_state_t.RELEASED;
}
