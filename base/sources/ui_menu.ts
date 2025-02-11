
let ui_menu_show: bool = false;
let ui_menu_category: i32 = 0;
let ui_menu_x: i32 = 0;
let ui_menu_y: i32 = 0;
let ui_menu_h: i32 = 0;
let ui_menu_elements: i32 = 0; ////
let ui_menu_keep_open: bool = false;
let ui_menu_commands: (ui: ui_t)=>void = null;
let ui_menu_show_first: bool = true;
let ui_menu_hide_flag: bool = false;

let _ui_menu_render_msg: string;

type update_info_t = {
	version: i32;
	version_name: string;
};

function ui_menu_render() {
	let ui: ui_t = base_ui_menu;
	let menu_w: i32 = ui_menu_commands != null ?
		math_floor(base_default_element_w * ui_SCALE(base_ui_menu) * 2.3) :
		math_floor(ui_ELEMENT_W(ui) * 2.3);

	let _FILL_BUTTON_BG: i32 = ui.ops.theme.FILL_BUTTON_BG;
	ui.ops.theme.FILL_BUTTON_BG = false;
	let _ELEMENT_OFFSET: i32 = ui.ops.theme.ELEMENT_OFFSET;
	ui.ops.theme.ELEMENT_OFFSET = 0;
	let _ELEMENT_H: i32 = ui.ops.theme.ELEMENT_H;
	ui.ops.theme.ELEMENT_H = config_raw.touch_ui ? (28 + 2) : 28;

	// First draw out of screen, then align the menu based on menu height
	if (ui_menu_show_first) {
		ui_menu_x -= sys_width() * 2;
		ui_menu_y -= sys_height() * 2;
	}

	ui_begin_region(ui, ui_menu_x, ui_menu_y, menu_w);
	ui_menu_start(ui);

	if (ui_menu_commands != null) {
		ui_menu_commands(ui);
	}
	else {
		if (ui_menu_category == menu_category_t.FILE) {
			if (ui_menu_button(ui, tr("New Project..."), map_get(config_keymap, "file_new"))) {
				project_new_box();
			}
			if (ui_menu_button(ui, tr("Open..."), map_get(config_keymap, "file_open"))) {
				project_open();
			}
			if (ui_menu_button(ui, tr("Open Recent..."), map_get(config_keymap, "file_open_recent"))) {
				box_projects_show();
			}
			if (ui_menu_button(ui, tr("Save"), map_get(config_keymap, "file_save"))) {
				project_save();
			}
			if (ui_menu_button(ui, tr("Save As..."), map_get(config_keymap, "file_save_as"))) {
				project_save_as();
			}
			ui_menu_separator(ui);
			if (ui_menu_button(ui, tr("Import Texture..."), map_get(config_keymap, "file_import_assets"))) {
				project_import_asset(string_array_join(path_texture_formats, ","), false);
			}
			if (ui_menu_button(ui, tr("Import Envmap..."))) {
				ui_files_show("hdr", false, false, function (path: string) {
					if (!ends_with(path, ".hdr")) {
						console_error(tr("Error: .hdr file expected"));
						return;
					}
					import_asset_run(path);
				});
			}

			///if (is_paint || is_sculpt)
			if (ui_menu_button(ui, tr("Import Font..."))) {
				project_import_asset("ttf,ttc,otf");
			}
			if (ui_menu_button(ui, tr("Import Material..."))) {
				project_import_material();
			}
			if (ui_menu_button(ui, tr("Import Brush..."))) {
				project_import_brush();
			}
			///end

			///if (is_paint || is_lab)
			if (ui_menu_button(ui, tr("Import Swatches..."))) {
				project_import_swatches();
			}
			///end
			if (ui_menu_button(ui, tr("Import Mesh..."))) {
				project_import_mesh();
			}
			if (ui_menu_button(ui, tr("Reimport Mesh"), map_get(config_keymap, "file_reimport_mesh"))) {
				project_reimport_mesh();
			}
			if (ui_menu_button(ui, tr("Reimport Textures"), map_get(config_keymap, "file_reimport_textures"))) {
				project_reimport_textures();
			}
			ui_menu_separator(ui);
			///if (is_paint || is_lab)
			if (ui_menu_button(ui, tr("Export Textures..."), map_get(config_keymap, "file_export_textures_as"))) {
				context_raw.layers_export = export_mode_t.VISIBLE;
				box_export_show_textures();
			}
			if (ui_menu_button(ui, tr("Export Swatches..."))) {
				project_export_swatches();
			}
			///end
			if (ui_menu_button(ui, tr("Export Mesh..."))) {
				context_raw.export_mesh_index = 0; // All
				box_export_show_mesh();
			}

			///if is_paint
			if (ui_menu_button(ui, tr("Bake Material..."))) {
				box_export_show_bake_material();
			}
			///end

			ui_menu_separator(ui);
			if (ui_menu_button(ui, tr("Exit"))) {
				sys_stop();
			}
		}
		else if (ui_menu_category == menu_category_t.EDIT) {
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
			if (ui_menu_button(ui, tr("Undo {step}", vars_undo), map_get(config_keymap, "edit_undo"))) {
				history_undo();
			}

			ui.enabled = history_redos > 0;
			let vars_redo: map_t<string, string> = map_create();
			map_set(vars_redo, "step", step_redo);
			if (ui_menu_button(ui, tr("Redo {step}", vars_redo), map_get(config_keymap, "edit_redo"))) {
				history_redo();
			}
			ui.enabled = true;
			ui_menu_separator(ui);
			if (ui_menu_button(ui, tr("Preferences..."), map_get(config_keymap, "edit_prefs"))) {
				box_preferences_show();
			}
		}
		else if (ui_menu_category == menu_category_t.VIEWPORT) {
			if (ui_menu_button(ui, tr("Distract Free"), map_get(config_keymap, "view_distract_free"))) {
				ui_base_toggle_distract_free();
				ui_base_ui.is_hovered = false;
			}

			///if !(arm_android || arm_ios)
			if (ui_menu_button(ui, tr("Toggle Fullscreen"), "alt+enter")) {
				base_toggle_fullscreen();
			}
			///end

			ui.changed = false;

			let p: world_data_t = scene_world;
			let env_handle: ui_handle_t = ui_handle(__ID__);
			env_handle.value = p.strength;
			ui_menu_align(ui);
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
			ui_menu_align(ui);
			context_raw.envmap_angle = ui_slider(enva_handle, tr("Environment Angle"), 0.0, 360.0, true, 1) / 180.0 * math_pi();
			if (ui.is_hovered) {
				let vars: map_t<string, string> = map_create();
				map_set(vars, "shortcut", map_get(config_keymap, "rotate_envmap"));
				ui_tooltip(tr("{shortcut} and move mouse", vars));
			}
			if (enva_handle.changed) {
				context_raw.ddirty = 2;
			}

			let lhandle: ui_handle_t = ui_handle(__ID__);
			lhandle.value = uniforms_light_strength;
			lhandle.value = math_floor(lhandle.value * 100) / 100;
			ui_menu_align(ui);
			uniforms_light_strength = ui_slider(lhandle, tr("Light"), 0.0, 4.0, true);
			if (lhandle.changed) {
				context_raw.ddirty = 2;
			}

			let lahandle: ui_handle_t = ui_handle(__ID__);
			lahandle.value = context_raw.light_angle / math_pi() * 180;
			ui_menu_align(ui);
			let new_angle: f32 = ui_slider(lahandle, tr("Light Angle"), 0.0, 360.0, true, 1) / 180 * math_pi();
			if (ui.is_hovered) {
				let vars: map_t<string, string> = map_create();
				map_set(vars, "shortcut", map_get(config_keymap, "rotate_light"));
				ui_tooltip(tr("{shortcut} and move mouse", vars));
			}
			let ldiff: f32 = new_angle - context_raw.light_angle;
			if (math_abs(ldiff) > 0.005) {
				if (new_angle < 0) {
					new_angle += (math_floor(-new_angle / (2 * math_pi())) + 1) * 2 * math_pi();
				}
				else if (new_angle > 2 * math_pi()) {
					new_angle -= math_floor(new_angle / (2 * math_pi())) * 2 * math_pi();
				}
				context_raw.light_angle = new_angle;
				let m: mat4_t = mat4_rot_z(ldiff);
				uniforms_light_world = mat4_mult_mat(uniforms_light_world, m);
				context_raw.ddirty = 2;
			}

			let sxhandle: ui_handle_t = ui_handle(__ID__);
			sxhandle.value = uniforms_light_size_x;
			ui_menu_align(ui);
			uniforms_light_size_x = ui_slider(sxhandle, tr("Light Size"), 0.0, 4.0, true);
			if (sxhandle.changed) {
				context_raw.ddirty = 2;
			}

			///if (is_paint || is_sculpt)
			let split_view_handle: ui_handle_t = ui_handle(__ID__);
			if (split_view_handle.init) {
				split_view_handle.selected = context_raw.split_view;
			}
			context_raw.split_view = ui_check(split_view_handle, " " + tr("Split View"));
			if (split_view_handle.changed) {
				base_resize();
			}
			///end

			///if is_lab
			let brush_scale_handle: ui_handle_t = ui_handle(__ID__);
			if (brush_scale_handle.init) {
				brush_scale_handle.value = context_raw.brush_scale;
			}
			ui_menu_align(ui);
			context_raw.brush_scale = ui_slider(brush_scale_handle, tr("UV Scale"), 0.01, 5.0, true);
			if (brush_scale_handle.changed) {
				make_material_parse_mesh_material();
				render_path_raytrace_uv_scale = context_raw.brush_scale;
				render_path_raytrace_ready = false;
			}
			///end

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
				make_material_parse_paint_material();
				make_material_parse_mesh_material();
			}

			///if (is_paint || is_sculpt)
			context_raw.draw_wireframe = ui_check(context_raw.wireframe_handle, " " + tr("Wireframe"));
			if (context_raw.wireframe_handle.changed) {
				let current: image_t = _g2_current;
				g2_end();
				util_uv_cache_uv_map();
				g2_begin(current);
				make_material_parse_mesh_material();
			}
			///end

			///if is_paint
			context_raw.draw_texels = ui_check(context_raw.texels_handle, " " + tr("Texels"));
			if (context_raw.texels_handle.changed) {
				make_material_parse_mesh_material();
			}
			///end

			let compass_handle: ui_handle_t = ui_handle(__ID__);
			if (compass_handle.init) {
				compass_handle.selected = context_raw.show_compass;
			}
			context_raw.show_compass = ui_check(compass_handle, " " + tr("Compass"));
			if (compass_handle.changed) {
				context_raw.ddirty = 2;
			}

			context_raw.show_envmap = ui_check(context_raw.show_envmap_handle, " " + tr("Envmap"));
			if (context_raw.show_envmap_handle.changed) {
				context_load_envmap();
				context_raw.ddirty = 2;
			}

			context_raw.show_envmap_blur = ui_check(context_raw.show_envmap_blur_handle, " " + tr("Blur Envmap"));
			if (context_raw.show_envmap_blur_handle.changed) {
				context_raw.ddirty = 2;
			}

			context_update_envmap();

			if (ui.changed) {
				ui_menu_keep_open = true;
			}
		}
		else if (ui_menu_category == menu_category_t.MODE) {
			let mode_handle: ui_handle_t = ui_handle(__ID__);
			mode_handle.position = context_raw.viewport_mode;
			let modes: string[] = [
				tr("Lit"),
				tr("Base Color"),
				///if (is_paint || is_lab)
				tr("Normal"),
				tr("Occlusion"),
				tr("Roughness"),
				tr("Metallic"),
				tr("Opacity"),
				tr("Height"),
				///end
				///if (is_paint)
				tr("Emission"),
				tr("Subsurface"),
				///end
				///if (is_paint || is_sculpt)
				tr("TexCoord"),
				tr("Object Normal"),
				tr("Material ID"),
				tr("Object ID"),
				tr("Mask")
				///end
			];
			let shortcuts: string[] = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];

			if (iron_raytrace_supported()) {
				array_push(modes, tr("Path Traced"));
				array_push(shortcuts, "p");
			}

			for (let i: i32 = 0; i < modes.length; ++i) {
				let shortcut: string = config_raw.touch_ui ? "" : map_get(config_keymap, "viewport_mode") + ", " + shortcuts[i];
				ui_radio(mode_handle, i, modes[i], shortcut);
			}

			if (mode_handle.changed) {
				context_set_viewport_mode(mode_handle.position);
				// TODO: rotate mode is not supported for path tracing yet
				if (mode_handle.position == viewport_mode_t.PATH_TRACE && context_raw.camera_controls == camera_controls_t.ROTATE) {
					context_raw.camera_controls = camera_controls_t.ORBIT;
					viewport_reset();
				}
			}
		}
		else if (ui_menu_category == menu_category_t.CAMERA) {
			if (ui_menu_button(ui, tr("Reset"), map_get(config_keymap, "view_reset"))) {
				viewport_reset();
				viewport_scale_to_bounds();
			}
			ui_menu_separator(ui);
			if (ui_menu_button(ui, tr("Front"), map_get(config_keymap, "view_front"))) {
				viewport_set_view(0, -1, 0, math_pi() / 2, 0, 0);
			}
			if (ui_menu_button(ui, tr("Back"), map_get(config_keymap, "view_back"))) {
				viewport_set_view(0, 1, 0, math_pi() / 2, 0, math_pi());
			}
			if (ui_menu_button(ui, tr("Right"), map_get(config_keymap, "view_right"))) {
				viewport_set_view(1, 0, 0, math_pi() / 2, 0, math_pi() / 2);
			}
			if (ui_menu_button(ui, tr("Left"), map_get(config_keymap, "view_left"))) {
				viewport_set_view(-1, 0, 0, math_pi() / 2, 0, -math_pi() / 2);
			}
			if (ui_menu_button(ui, tr("Top"), map_get(config_keymap, "view_top"))) {
				viewport_set_view(0, 0, 1, 0, 0, 0);
			}
			if (ui_menu_button(ui, tr("Bottom"), map_get(config_keymap, "view_bottom"))) {
				viewport_set_view(0, 0, -1, math_pi(), 0, math_pi());
			}
			ui_menu_separator(ui);

			ui.changed = false;

			if (ui_menu_button(ui, tr("Orbit Left"), map_get(config_keymap, "view_orbit_left"))) {
				viewport_orbit(-math_pi() / 12, 0);
			}
			if (ui_menu_button(ui, tr("Orbit Right"), map_get(config_keymap, "view_orbit_right"))) {
				viewport_orbit(math_pi() / 12, 0);
			}
			if (ui_menu_button(ui, tr("Orbit Up"), map_get(config_keymap, "view_orbit_up"))) {
				viewport_orbit(0, -math_pi() / 12);
			}
			if (ui_menu_button(ui, tr("Orbit Down"), map_get(config_keymap, "view_orbit_down"))) {
				viewport_orbit(0, math_pi() / 12);
			}
			if (ui_menu_button(ui, tr("Orbit Opposite"), map_get(config_keymap, "view_orbit_opposite"))) {
				viewport_orbit_opposite();
			}
			if (ui_menu_button(ui, tr("Zoom In"), map_get(config_keymap, "view_zoom_in"))) {
				viewport_zoom(0.2);
			}
			if (ui_menu_button(ui, tr("Zoom Out"), map_get(config_keymap, "view_zoom_out"))) {
				viewport_zoom(-0.2);
			}

			let cam: camera_object_t = scene_camera;
			context_raw.fov_handle = ui_handle(__ID__);
			if (context_raw.fov_handle.init) {
				context_raw.fov_handle.value = math_floor(cam.data.fov * 100) / 100;
			}
			ui_menu_align(ui);
			cam.data.fov = ui_slider(context_raw.fov_handle, tr("FoV"), 0.3, 1.4, true);
			if (context_raw.fov_handle.changed) {
				viewport_update_camera_type(context_raw.camera_type);
			}

			ui_menu_align(ui);
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

			ui_menu_align(ui);
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
		else if (ui_menu_category == menu_category_t.HELP) {
			if (ui_menu_button(ui, tr("Manual"))) {
				file_load_url(manifest_url + "/manual");
			}
			if (ui_menu_button(ui, tr("How To"))) {
				file_load_url(manifest_url + "/howto");
			}
			if (ui_menu_button(ui, tr("What's New"))) {
				file_load_url(manifest_url + "/notes");
			}
			if (ui_menu_button(ui, tr("Issue Tracker"))) {
				file_load_url("https://github.com/armory3d/armortools/issues");
			}
			if (ui_menu_button(ui, tr("Report Bug"))) {
				///if (arm_macos || arm_ios) // Limited url length
				file_load_url("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + sys_system_id());
				///else
				file_load_url("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + sys_system_id() + "*%0A%0A**Issue description:**%0A%0A**Steps to reproduce:**%0A%0A");
				///end
			}
			if (ui_menu_button(ui, tr("Request Feature"))) {
				///if (arm_macos || arm_ios) // Limited url length
				file_load_url("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + sys_system_id());
				///else
				file_load_url("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + config_get_sha() + ",%20" + sys_system_id() + "*%0A%0A**Feature description:**%0A%0A");
				///end
			}
			ui_menu_separator(ui);

			if (ui_menu_button(ui, tr("Check for Updates..."))) {
				///if arm_android
				file_load_url(manifest_url_android);
				///elseif arm_ios
				file_load_url(manifest_url_ios);
				///else
				// Retrieve latest version number
				file_download_bytes("https://server.armorpaint.org/" + to_lower_case(manifest_title) + ".html", function (url: string, buffer: buffer_t) {
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

			if (ui_menu_button(ui, tr("About..."))) {

				let msg: string = manifest_title + ".org - v" + manifest_version + " (" + config_get_date() + ") - " + config_get_sha() + "\n";
				msg += sys_system_id() + " - " + strings_graphics_api();

				///if arm_windows
				let save: string;
				if (path_is_protected()) {
					save = iron_save_path();
				}
				else {
					save = path_data();
				}
				save += path_sep + "tmp.txt";
				iron_sys_command("wmic path win32_VideoController get name > \"" + save + "\"");
				let blob: buffer_t = iron_load_blob(save);
				let u8: u8_array_t = blob;
				let gpu_raw: string = "";
				for (let i: i32 = 0; i < math_floor(u8.length / 2); ++i) {
					let c: string = string_from_char_code(u8[i * 2]);
					gpu_raw += c;
				}

				let gpus: string[] = string_split(gpu_raw, "\n");
				array_splice(gpus, 1, gpus.length - 2);
				let gpu: string = "";
				for (let i: i32 = 0; i < gpus.length; ++i) {
					let g: string = gpus[i];
					gpu += trim_end(g) + ", ";
				}
				gpu = substring(gpu, 0, gpu.length - 2);
				msg += "\n" + gpu;
				///else
				// { lshw -C display }
				///end

				_ui_menu_render_msg = msg;

				ui_box_show_custom(function (ui: ui_t) {
					let tab_vertical: bool = config_raw.touch_ui;
					if (ui_tab(ui_handle(__ID__), tr("About"), tab_vertical)) {

						let img: image_t = data_get_image("badge.k");
						_ui_image(img);
						_ui_end_element();

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
						_ui_end_element();
						///end

						if (ui_button(tr("Contributors"))) {
							file_load_url("https://github.com/armory3d/armortools/graphs/contributors");
						}
						if (ui_button(tr("OK"))) {
							ui_box_hide();
						}
					}
				}, 400, 320);
			}
		}
	}

	ui_menu_hide_flag = ui.combo_selected_handle == null && !ui_menu_keep_open && !ui_menu_show_first && (ui.changed || ui.input_released || ui.input_released_r || ui.is_escape_down);
	ui_menu_keep_open = false;

	ui.ops.theme.FILL_BUTTON_BG = _FILL_BUTTON_BG;
	ui.ops.theme.ELEMENT_OFFSET = _ELEMENT_OFFSET;
	ui.ops.theme.ELEMENT_H = _ELEMENT_H;
	ui_menu_end(ui);
	ui_end_region();

	if (ui_menu_show_first) {
		ui_menu_show_first = false;
		ui_menu_h = ui._y - ui_menu_y;
		ui_menu_x += sys_width() * 2;
		ui_menu_y += sys_height() * 2;
		ui_menu_fit_to_screen();
		ui_menu_render(); // Render at correct position now
	}

	if (ui_menu_hide_flag) {
		ui_menu_hide();
		ui_menu_show_first = true;
		ui_menu_commands = null;
	}
}

function ui_menu_hide() {
	ui_menu_show = false;
	base_redraw_ui();
}

function ui_menu_draw(commands: (ui: ui_t)=>void = null, x: i32 = -1, y: i32 = -1) {
	ui_end_input();
	ui_menu_show = true;
	ui_menu_commands = commands;
	ui_menu_x = x > -1 ? x : math_floor(mouse_x + 1);
	ui_menu_y = y > -1 ? y : math_floor(mouse_y + 1);
	ui_menu_h = 0;
}

function ui_menu_fit_to_screen() {
	// Prevent the menu going out of screen
	let menu_w: f32 = base_default_element_w * ui_SCALE(base_ui_menu) * 2.3;
	if (ui_menu_x + menu_w > sys_width()) {
		if (ui_menu_x - menu_w > 0) {
			ui_menu_x = math_floor(ui_menu_x - menu_w);
		}
		else {
			ui_menu_x = math_floor(sys_width() - menu_w);
		}
	}
	if (ui_menu_y + ui_menu_h > sys_height()) {
		if (ui_menu_y - ui_menu_h > 0) {
			ui_menu_y = math_floor(ui_menu_y - ui_menu_h);
		}
		else {
			ui_menu_y = sys_height() - ui_menu_h;
		}
		ui_menu_x += 1; // Move out of mouse focus
	}
}

function ui_menu_separator(ui: ui_t) {
	ui._y++;
	if (config_raw.touch_ui) {
		ui_fill(0, 0, ui._w / ui_SCALE(ui), 1, ui.ops.theme.BUTTON_COL);
	}
	else {
		ui_fill(26, 0, ui._w / ui_SCALE(ui) - 26, 1, ui.ops.theme.BUTTON_COL);
	}
}

function ui_menu_button(ui: ui_t, text: string, label: string = ""): bool {
	if (config_raw.touch_ui) {
		label = "";
	}

	// let icons: image_t = icon > -1 ? get("icons.k") : null;
	// let r: rect_t = tile25(icons, icon, 8);
	// return ui_button(config_button_spacing + text, config_button_align, label, icons, r.x, r.y, r.w, r.h);

	return ui_button(config_button_spacing + text, config_button_align, label);
}

function ui_menu_align(ui: ui_t) {
	if (!config_raw.touch_ui) {
		let row: f32[] = [12 / 100, 88 / 100];
		ui_row(row);
		_ui_end_element();
	}
}

function ui_menu_start(ui: ui_t) {
	ui_draw_shadow(ui._x, ui._y, ui._w, ui_menu_h);

	g2_set_color(ui.ops.theme.SEPARATOR_COL);
	ui_draw_rect(true, ui._x, ui._y, ui._w, ui_menu_h);
	g2_set_color(0xffffffff);
}

function ui_menu_end(ui: ui_t) {
}
