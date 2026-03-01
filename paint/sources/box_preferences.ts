
let box_preferences_htab: ui_handle_t      = ui_handle_create();
let box_preferences_files_plugin: string[] = null;
let box_preferences_files_keymap: string[] = null;
let box_preferences_h_theme: ui_handle_t;
let box_preferences_h_preset: ui_handle_t;
let box_preferences_locales: string[] = null;
let box_preferences_themes: string[]  = null;
let _box_preferences_f: string;
let _box_preferences_h: ui_handle_t;
let _box_preferences_i: i32;

function box_preferences_interface_tab() {
	if (box_preferences_locales == null) {
		box_preferences_locales = translator_get_supported_locales();
	}

	let h_locale: ui_handle_t = ui_handle(__ID__);
	h_locale.i                = array_index_of(box_preferences_locales, config_raw.locale);
	ui_combo(h_locale, box_preferences_locales, tr("Language"), true);
	if (h_locale.changed) {
		let locale_code: string = box_preferences_locales[h_locale.i];
		config_raw.locale       = locale_code;
		translator_load_translations(locale_code);
		base_redraw_ui();
	}

	let h_scale: ui_handle_t = ui_handle(__ID__);
	if (h_scale.init) {
		h_scale.f = config_raw.window_scale;
	}
	ui_slider(h_scale, tr("UI Scale"), 1.0, 4.0, true, 10);
	if (context_raw.hscale_was_changed && !ui.input_down) {
		context_raw.hscale_was_changed = false;
		if (h_scale.f == 0.0) {
			h_scale.f = 1.0;
		}
		config_raw.window_scale = h_scale.f;
		box_preferences_set_scale();
	}
	if (h_scale.changed) {
		context_raw.hscale_was_changed = true;
	}

	let h_node_previews: ui_handle_t = ui_handle(__ID__);
	h_node_previews.b                = config_raw.node_previews;
	config_raw.node_previews         = ui_check(h_node_previews, tr("Node Previews"));
	if (h_node_previews.changed) {
		for (let i: i32 = 0; i < project_materials.length; ++i) {
			let c: ui_node_canvas_t = project_materials[i].canvas;
			for (let j: i32 = 0; j < c.nodes.length; ++j) {
				let n: ui_node_t = c.nodes[j];
				if (config_raw.node_previews) {
					n.flags |= ui_node_flag_t.PREVIEW;
				}
				else {
					n.flags &= ~ui_node_flag_t.PREVIEW;
				}
			}
		}
		ui_nodes_hwnd.redraws = 2;
	}
	if (ui.is_hovered) {
		ui_tooltip(tr("Show node preview on each node by default"));
	}

	let h_wrap_mouse: ui_handle_t = ui_handle(__ID__);
	h_wrap_mouse.b                = config_raw.wrap_mouse;
	config_raw.wrap_mouse         = ui_check(h_wrap_mouse, tr("Wrap Mouse"));
	if (ui.is_hovered) {
		ui_tooltip(tr("Wrap mouse around view boundaries during camera control"));
	}

	ui.changed                          = false;
	let h_show_asset_names: ui_handle_t = ui_handle(__ID__);
	h_show_asset_names.b                = config_raw.show_asset_names;
	config_raw.show_asset_names         = ui_check(h_show_asset_names, tr("Show Asset Names"));
	if (ui.changed) {
		base_redraw_ui();
	}

	ui.changed                  = false;
	let h_touch_ui: ui_handle_t = ui_handle(__ID__);
	h_touch_ui.b                = config_raw.touch_ui;
	config_raw.touch_ui         = ui_check(h_touch_ui, tr("Touch UI"));
	if (ui.changed) {
		ui_touch_control = config_raw.touch_ui;
		config_load_theme(config_raw.theme);
		box_preferences_set_scale();
		base_redraw_ui();
		context_raw.hscale_was_changed = true;
	}

	let h_splash_screen: ui_handle_t = ui_handle(__ID__);
	h_splash_screen.b                = config_raw.splash_screen;
	config_raw.splash_screen         = ui_check(h_splash_screen, tr("Splash Screen"));

	let h_grid_snap: ui_handle_t = ui_handle(__ID__);
	h_grid_snap.b                = config_raw.grid_snap;
	config_raw.grid_snap         = ui_check(h_grid_snap, tr("Grid Snap"));
	ui_nodes_grid_snap           = config_raw.grid_snap;

	let h_experimental: ui_handle_t = ui_handle(__ID__);
	h_experimental.b                = config_raw.experimental;
	config_raw.experimental         = ui_check(h_experimental, tr("Experimental Features"));

	ui_end_element();

	ui_row2();
	if (ui_icon_button(tr("Restore"), icon_t.REPLAY) && !ui_menu_show) {
		ui_menu_draw(function() {
			if (ui_menu_button(tr("Confirm"), "", icon_t.CHECK)) {
				sys_notify_on_next_frame(function(_: any) {
					ui.ops.theme.ELEMENT_H = base_default_element_h;
					config_restore();
					box_preferences_set_scale();
					if (config_raw.plugins != null) {
						for (let i: i32 = 0; i < config_raw.plugins.length; ++i) {
							let f: string = config_raw.plugins[i];
							plugin_stop(f);
						}
					}
					box_preferences_files_plugin = null;
					box_preferences_files_keymap = null;
					make_material_parse_mesh_material();
					make_material_parse_paint_material();
					ui_base_set_viewport_col(ui.ops.theme.VIEWPORT_COL);
				});
			}
			if (ui_menu_button(tr("Import..."), "", icon_t.IMPORT)) {
				ui_files_show("json", false, false, function(path: string) {
					let b: buffer_t   = data_get_blob(path);
					let raw: config_t = json_parse(sys_buffer_to_string(b));
					sys_notify_on_next_frame(function(raw: config_t) {
						ui.ops.theme.ELEMENT_H = base_default_element_h;
						config_import_from(raw);
						box_preferences_set_scale();
						make_material_parse_mesh_material();
						make_material_parse_paint_material();
					}, raw);
				});
			}
		});
	}
	if (ui_button(tr("Reset Layout")) && !ui_menu_show) {
		ui_menu_draw(function() {
			if (ui_menu_button(tr("Confirm"), "", icon_t.CHECK)) {
				config_init_layout();
				config_save();
			}
		});
	}
}

function box_preferences_theme_tab() {
	if (box_preferences_themes == null) {
		box_preferences_fetch_themes();
	}

	ui_begin_sticky();
	ui_row4();

	box_preferences_h_theme   = ui_handle(__ID__);
	box_preferences_h_theme.i = box_preferences_get_theme_index();
	ui_combo(box_preferences_h_theme, box_preferences_themes, tr("Theme"));
	if (box_preferences_h_theme.changed) {
		config_raw.theme = box_preferences_themes[box_preferences_h_theme.i] + ".json";
		config_load_theme(config_raw.theme);
	}

	if (ui_icon_button(tr("New"), icon_t.PLUS)) {
		ui_box_show_custom(function() {
			ui_row2();
			let h: ui_handle_t = ui_handle(__ID__);
			if (h.init) {
				h.text = "new_theme";
			}
			let theme_name: string = ui_text_input(h, tr("Name"));
			if (ui_icon_button(tr("OK"), icon_t.CHECK) || ui.is_return_down) {
				let template: string = box_preferences_theme_to_json(base_theme);
				if (!ends_with(theme_name, ".json")) {
					theme_name += ".json";
				}
				let path: string = path_data() + PATH_SEP + "themes" + PATH_SEP + theme_name;
				iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
				box_preferences_fetch_themes(); // Refresh file list
				config_raw.theme          = theme_name;
				box_preferences_h_theme.i = box_preferences_get_theme_index();
				ui_box_hide();
				box_preferences_htab.i = 1; // Themes
				box_preferences_show();
			}
		}, 400, 200, null, true, tr("New Theme"));
	}

	if (ui_icon_button(tr("Import"), icon_t.IMPORT)) {
		ui_files_show("json", false, false, function(path: string) {
			import_theme_run(path);
		});
	}

	if (ui_icon_button(tr("Export"), icon_t.EXPORT)) {
		ui_files_show("json", true, false, function(path: string) {
			path += PATH_SEP;
			path += ui_files_filename;
			if (!ends_with(path, ".json")) {
				path += ".json";
			}
			iron_file_save_bytes(path, sys_string_to_buffer(box_preferences_theme_to_json(base_theme)), 0);
		});
	}

	ui_end_sticky();

	// Theme fields
	let h_list: ui_handle_t = ui_handle(__ID__);
	let u32_theme: u32_ptr  = base_theme;
	ui.input_enabled        = !ui_menu_show;
	for (let i: i32 = 0; i < ui_theme_keys_count; ++i) {
		let key: string    = ARRAY_ACCESS(ui_theme_keys, i);
		let h: ui_handle_t = ui_nest(h_list, i);
		let val: u32       = DEREFERENCE(u32_theme + i);
		let is_hex: bool   = ends_with(key, "_COL");

		if (is_hex) {
			let row: f32[] = [ 1 / 8, 7 / 8 ];
			ui_row(row);
			ui_text("", 0, val);
			if (ui.is_hovered && ui.input_released) {
				h.color            = val;
				_box_preferences_h = h;
				_box_preferences_i = i;
				ui_menu_draw(function() {
					ui.changed                                  = false;
					let color: i32                              = ui_color_wheel(_box_preferences_h, false, -1, 11 * ui.ops.theme.ELEMENT_H * UI_SCALE(), true);
					let u32_theme: u32_ptr                      = base_theme;
					DEREFERENCE(u32_theme + _box_preferences_i) = color;
					if (ui.changed) {
						ui_menu_keep_open = true;
					}
				});
			}

			if (key == "VIEWPORT_COL" && ui_base_viewport_col != val) {
				ui_base_set_viewport_col(val);
			}
		}

		ui.changed = false;

		if (key == "FILL_WINDOW_BG" || key == "FILL_BUTTON_BG" || key == "FULL_TABS" || key == "ROUND_CORNERS" || key == "SHADOWS") {
			h.b                        = val > 0;
			let b: bool                = ui_check(h, key);
			DEREFERENCE(u32_theme + i) = b;
		}
		else if (key == "LINK_STYLE") {
			let styles: string[]       = [ tr("Straight"), tr("Curved") ];
			h.i                        = val;
			let pos: i32               = ui_combo(h, styles, key, true);
			DEREFERENCE(u32_theme + i) = pos;
		}
		else {
			h.text = is_hex ? i32_to_string_hex(val) : i32_to_string(val);
			ui_text_input(h, key);
			if (is_hex) {
				DEREFERENCE(u32_theme + i) = parse_int_hex(h.text);
			}
			else {
				DEREFERENCE(u32_theme + i) = parse_int(h.text);
			}
		}

		if (ui.changed) {
			ui.elements_baked = false;
		}
	}
	ui.input_enabled = true;
}

function box_preferences_usage_tab() {
	let h_undo: ui_handle_t = ui_handle(__ID__);
	h_undo.f                = config_raw.undo_steps;
	config_raw.undo_steps   = ui_slider(h_undo, tr("Undo Steps"), 1, 64, false, 1);
	if (config_raw.undo_steps < 1) {
		config_raw.undo_steps = h_undo.f = 1;
	}
	if (h_undo.changed) {
		let current: gpu_texture_t = _draw_current;
		draw_end();

		if (history_undo_layers != null) {
			while (history_undo_layers.length < config_raw.undo_steps) {
				let len: i32        = history_undo_layers.length;
				let l: slot_layer_t = slot_layer_create("_undo" + len);
				array_push(history_undo_layers, l);
			}
			while (history_undo_layers.length > config_raw.undo_steps) {
				let l: slot_layer_t = array_pop(history_undo_layers);
				slot_layer_unload(l);
			}
		}

		history_reset();
		draw_begin(current);
	}

	let h_dilate_radius: ui_handle_t = ui_handle(__ID__);
	h_dilate_radius.f                = config_raw.dilate_radius;
	config_raw.dilate_radius         = ui_slider(h_dilate_radius, tr("Dilate Radius"), 0.0, 16.0, true, 1);
	if (ui.is_hovered) {
		ui_tooltip(tr("Dilate painted textures to prevent seams"));
	}

	let h_camera_controls: ui_handle_t  = ui_handle(__ID__);
	h_camera_controls.i                 = config_raw.camera_controls;
	let camera_controls_combo: string[] = [ tr("Orbit"), tr("Rotate"), tr("Fly") ];
	config_raw.camera_controls          = ui_combo(h_camera_controls, camera_controls_combo, tr("Default Camera Controls"), true);

	let h_fov: ui_handle_t = ui_handle(__ID__);
	h_fov.f                = config_raw.camera_fov;
	config_raw.camera_fov  = ui_slider(h_fov, tr("Default Camera FoV"), 0.3, 1.4, true);

	let h_speed: ui_handle_t     = ui_handle(__ID__);
	h_speed.f                    = config_raw.camera_zoom_speed;
	config_raw.camera_zoom_speed = ui_slider(h_speed, tr("Camera Zoom Speed"), 0.1, 4.0, true);

	h_speed                          = ui_handle(__ID__);
	h_speed.f                        = config_raw.camera_rotation_speed;
	config_raw.camera_rotation_speed = ui_slider(h_speed, tr("Camera Rotation Speed"), 0.1, 4.0, true);

	h_speed                     = ui_handle(__ID__);
	h_speed.f                   = config_raw.camera_pan_speed;
	config_raw.camera_pan_speed = ui_slider(h_speed, tr("Camera Pan Speed"), 0.1, 4.0, true);

	let h_upside_down: ui_handle_t = ui_handle(__ID__);
	h_upside_down.b                = config_raw.camera_upside_down;
	config_raw.camera_upside_down  = ui_check(h_upside_down, tr("Allow Upside Down Camera"));

	let h_zoom_direction: ui_handle_t  = ui_handle(__ID__);
	h_zoom_direction.i                 = config_raw.zoom_direction;
	let zoom_direction_combo: string[] = [
		tr("Vertical"), tr("Vertical Inverted"), tr("Horizontal"), tr("Horizontal Inverted"), tr("Vertical and Horizontal"),
		tr("Vertical and Horizontal Inverted")
	];
	config_raw.zoom_direction = ui_combo(h_zoom_direction, zoom_direction_combo, tr("Direction to Zoom"), true);

	let h_layer_res: ui_handle_t = ui_handle(__ID__);
	h_layer_res.i                = config_raw.layer_res;
	/// if (arm_android || arm_ios)
	let res_combo: string[] = [ "128", "256", "512", "1024", "2048", "4096" ];
	/// else
	let res_combo: string[] = [ "128", "256", "512", "1024", "2048", "4096", "8192", "16384" ];
	/// end
	config_raw.layer_res = ui_combo(h_layer_res, res_combo, tr("Default Layer Resolution"), true);

	let h_scene_atlas_res: ui_handle_t = ui_handle(__ID__);
	h_scene_atlas_res.i                = config_raw.scene_atlas_res;
	config_raw.scene_atlas_res         = ui_combo(h_scene_atlas_res, res_combo, tr("Scene Atlas Resolution"), true);

	let h_server: ui_handle_t = ui_handle(__ID__);
	h_server.text             = config_raw.server;
	config_raw.server         = ui_text_input(h_server, tr("Cloud Server"));

	let h_material_live: ui_handle_t = ui_handle(__ID__);
	h_material_live.b                = config_raw.material_live;
	config_raw.material_live         = ui_check(h_material_live, tr("Live Material Preview"));
	if (ui.is_hovered) {
		ui_tooltip(tr("Instantly update material preview on node change"));
	}

	let h_brush_live: ui_handle_t = ui_handle(__ID__);
	h_brush_live.b                = config_raw.brush_live;
	config_raw.brush_live         = ui_check(h_brush_live, tr("Live Brush Preview"));
	if (ui.is_hovered) {
		ui_tooltip(tr("Draw live brush preview in viewport"));
	}
	if (h_brush_live.changed) {
		context_raw.ddirty = 2;
	}

	let h_brush_depth_reject: ui_handle_t = ui_handle(__ID__);
	h_brush_depth_reject.b                = config_raw.brush_depth_reject;
	config_raw.brush_depth_reject         = ui_check(h_brush_depth_reject, tr("Depth Reject"));
	if (h_brush_depth_reject.changed) {
		make_material_parse_paint_material();
	}

	ui_row2();

	let h_brush_angle_reject: ui_handle_t = ui_handle(__ID__);
	h_brush_angle_reject.b                = config_raw.brush_angle_reject;
	config_raw.brush_angle_reject         = ui_check(h_brush_angle_reject, tr("Angle Reject"));
	if (h_brush_angle_reject.changed) {
		make_material_parse_paint_material();
	}

	if (!config_raw.brush_angle_reject) {
		ui.enabled = false;
	}
	let h_angle_dot: ui_handle_t       = ui_handle(__ID__);
	h_angle_dot.f                      = context_raw.brush_angle_reject_dot;
	context_raw.brush_angle_reject_dot = ui_slider(h_angle_dot, tr("Angle"), 0.0, 1.0, true);
	if (h_angle_dot.changed) {
		make_material_parse_paint_material();
	}
	ui.enabled = true;

	let h_alpha_discard: ui_handle_t = ui_handle(__ID__);
	h_alpha_discard.f                = config_raw.brush_alpha_discard;
	config_raw.brush_alpha_discard   = ui_slider(h_alpha_discard, tr("Alpha Discard"), 0.0, 1.0, true);
	if (h_alpha_discard.changed) {
		make_material_parse_paint_material();
	}
}

function box_preferences_pen_tab() {
	ui_text(tr("Pressure controls"));
	let h_pressure_radius: ui_handle_t = ui_handle(__ID__);
	h_pressure_radius.b                = config_raw.pressure_radius;
	config_raw.pressure_radius         = ui_check(h_pressure_radius, tr("Brush Radius"));

	let h_pressure_sensitivity: ui_handle_t = ui_handle(__ID__);
	h_pressure_sensitivity.f                = config_raw.pressure_sensitivity;
	config_raw.pressure_sensitivity         = ui_slider(h_pressure_sensitivity, tr("Sensitivity"), 0.0, 10.0, true);

	let h_pressure_hardness: ui_handle_t = ui_handle(__ID__);
	h_pressure_hardness.b                = config_raw.pressure_hardness;
	config_raw.pressure_hardness         = ui_check(h_pressure_hardness, tr("Brush Hardness"));

	let h_pressure_opacity: ui_handle_t = ui_handle(__ID__);
	h_pressure_opacity.b                = config_raw.pressure_opacity;
	config_raw.pressure_opacity         = ui_check(h_pressure_opacity, tr("Brush Opacity"));

	let h_pressure_angle: ui_handle_t = ui_handle(__ID__);
	h_pressure_angle.b                = config_raw.pressure_angle;
	config_raw.pressure_angle         = ui_check(h_pressure_angle, tr("Brush Angle"));

	ui_end_element();
	let row: f32[] = [ 1 / 4 ];
	ui_row(row);
	if (ui_icon_button(tr("Help"), icon_t.LINK)) {
		let url: string  = "https://github.com/armory3d/";
		let name: string = to_lower_case(manifest_title);
		url += name;
		url += "_docs#pen";
		iron_load_url(url);
	}
}

function box_preferences_viewport_tab() {
	let h_mode: ui_handle_t  = ui_handle(__ID__);
	h_mode.i                 = config_raw.viewport_mode;
	let mode_combo: string[] = [ tr("Lit"), tr("Path Traced") ];
	ui_combo(h_mode, mode_combo, tr("Default Mode"), true);
	if (h_mode.changed) {
		config_raw.viewport_mode = h_mode.i;
	}

	let h_pathtrace_mode: ui_handle_t  = ui_handle(__ID__);
	h_pathtrace_mode.i                 = config_raw.pathtrace_mode;
	let pathtrace_mode_combo: string[] = [ tr("Fast"), tr("Quality") ];
	config_raw.pathtrace_mode          = ui_combo(h_pathtrace_mode, pathtrace_mode_combo, tr("Path Tracer"), true);
	if (h_pathtrace_mode.changed) {
		render_path_raytrace_ready       = false;
		render_path_raytrace_init_shader = true;
		context_raw.ddirty               = 2;
	}

	let h_render_mode: ui_handle_t  = ui_handle(__ID__);
	h_render_mode.i                 = config_raw.render_mode;
	let render_mode_combo: string[] = [ tr("Desktop"), tr("Mobile") ];
	config_raw.render_mode          = ui_combo(h_render_mode, render_mode_combo, tr("Renderer"), true);
	if (h_render_mode.changed) {
		context_set_render_path();
	}

	let h_supersample: ui_handle_t  = ui_handle(__ID__);
	h_supersample.i                 = config_get_super_sample_quality(config_raw.rp_supersample);
	let supersample_combo: string[] = [ "0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x" ];
	ui_combo(h_supersample, supersample_combo, tr("Super Sample"), true);
	if (h_supersample.changed) {
		config_raw.rp_supersample = config_get_super_sample_size(h_supersample.i);
		config_apply();
	}

	if (config_raw.render_mode == render_mode_t.DEFERRED) {
		let h_ssao: ui_handle_t = ui_handle(__ID__);
		h_ssao.b                = config_raw.rp_ssao;
		config_raw.rp_ssao      = ui_check(h_ssao, tr("SSAO"));
		if (h_ssao.changed) {
			config_apply();
		}

		let h_bloom: ui_handle_t = ui_handle(__ID__);
		h_bloom.b                = config_raw.rp_bloom;
		config_raw.rp_bloom      = ui_check(h_bloom, tr("Bloom"));
		if (h_bloom.changed) {
			config_apply();
		}
	}

	let h_vignette: ui_handle_t = ui_handle(__ID__);
	h_vignette.f                = config_raw.rp_vignette;
	config_raw.rp_vignette      = ui_slider(h_vignette, tr("Vignette"), 0.0, 1.0, true);
	if (h_vignette.changed) {
		context_raw.ddirty = 2;
	}

	let h_noise_grain: ui_handle_t = ui_handle(__ID__);
	h_noise_grain.f                = config_raw.rp_grain;
	config_raw.rp_grain            = ui_slider(h_noise_grain, tr("Noise Grain"), 0.0, 1.0, true);
	if (h_noise_grain.changed) {
		context_raw.ddirty = 2;
	}

	let cam: camera_object_t   = scene_camera;
	let cam_raw: camera_data_t = cam.data;
	let h_near: ui_handle_t    = ui_handle(__ID__);
	let h_far: ui_handle_t     = ui_handle(__ID__);
	h_near.f                   = math_floor(cam_raw.near_plane * 1000) / 1000;
	h_far.f                    = math_floor(cam_raw.far_plane * 100) / 100;
	cam_raw.near_plane         = ui_slider(h_near, tr("Clip Start"), 0.001, 1.0, true);
	cam_raw.far_plane          = ui_slider(h_far, tr("Clip End"), 50.0, 100.0, true);
	if (h_near.changed || h_far.changed) {
		camera_object_build_proj(cam);
	}

	let h_disp: ui_handle_t      = ui_handle(__ID__);
	h_disp.f                     = config_raw.displace_strength;
	config_raw.displace_strength = ui_slider(h_disp, tr("Displacement Strength"), 0.0, 10.0, true);
	if (h_disp.changed) {
		context_raw.ddirty = 2;
		make_material_parse_mesh_material();
	}
}

function box_preferences_keymap_tab() {
	if (box_preferences_files_keymap == null) {
		box_preferences_fetch_keymaps();
	}

	ui_begin_sticky();
	ui_row4();

	box_preferences_h_preset   = ui_handle(__ID__);
	box_preferences_h_preset.i = box_preferences_get_preset_index();
	ui_combo(box_preferences_h_preset, box_preferences_files_keymap, tr("Preset"));
	if (box_preferences_h_preset.changed) {
		config_raw.keymap = box_preferences_files_keymap[box_preferences_h_preset.i] + ".json";
		config_apply();
		keymap_load();
	}

	if (ui_icon_button(tr("New"), icon_t.PLUS)) {
		ui_box_show_custom(function() {
			ui_row2();
			let h: ui_handle_t = ui_handle(__ID__);
			if (h.init) {
				h.text = "new_keymap";
			}
			let keymap_name: string = ui_text_input(h, tr("Name"));
			if (ui_icon_button(tr("OK"), icon_t.CHECK) || ui.is_return_down) {
				let template: string = keymap_to_json(keymap_get_default());
				if (!ends_with(keymap_name, ".json")) {
					keymap_name += ".json";
				}
				let path: string = path_data() + PATH_SEP + "keymap_presets" + PATH_SEP + keymap_name;
				iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
				box_preferences_fetch_keymaps(); // Refresh file list
				config_raw.keymap          = keymap_name;
				box_preferences_h_preset.i = box_preferences_get_preset_index();
				ui_box_hide();
				box_preferences_htab.i = 5; // Keymap
				box_preferences_show();
			}
		}, 400, 200, null, true, tr("New Keymap"));
	}

	if (ui_icon_button(tr("Import"), icon_t.IMPORT)) {
		ui_files_show("json", false, false, function(path: string) {
			import_keymap_run(path);
		});
	}
	if (ui_icon_button(tr("Export"), icon_t.EXPORT)) {
		ui_files_show("json", true, false, function(dest: string) {
			if (!ends_with(ui_files_filename, ".json")) {
				ui_files_filename += ".json";
			}
			let path: string = path_data() + PATH_SEP + "keymap_presets" + PATH_SEP + config_raw.keymap;
			file_copy(path, dest + PATH_SEP + ui_files_filename);
		});
	}

	ui_end_sticky();

	ui_separator(8, false);

	let index: i32     = 0;
	ui.changed         = false;
	let keys: string[] = map_keys(config_keymap);
	array_sort(keys, null);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let key: string    = keys[i];
		let h: ui_handle_t = ui_nest(ui_handle(__ID__), index++);
		h.text             = map_get(config_keymap, key);
		let text: string   = ui_text_input(h, key, ui_align_t.LEFT);
		map_set(config_keymap, key, text);
	}
	if (ui.changed) {
		config_apply();
		keymap_save();
	}
}

function box_preferences_neural_tab() {
	ui_text(tr("All processing is done locally on device"));

	let h_inference: ui_handle_t = ui_handle(__ID__);
	h_inference.i                = config_raw.neural_backend;
	/// if arm_windows
	let inference_combo: string[] = [ "CPU", "Vulkan", "CUDA" ];
	/// else
	let inference_combo: string[] = [ "CPU", "Vulkan" ];
	/// end
	config_raw.neural_backend = ui_combo(h_inference, inference_combo, tr("Inference Backend"), true);
	if (ui.is_hovered) {
		ui_tooltip(tr("Backend for neural node processing"));
	}

	ui_text(tr("Models"));

	if (neural_node_models == null) {
		neural_node_models_init();
	}

	for (let i: i32 = 0; i < neural_node_models.length; ++i) {
		box_preferences_model_panel(neural_node_models[i]);
	}

	let row: f32[] = [ 1 / 4 ];
	ui_row(row);
	if (ui_icon_button(tr("Models Directory..."), icon_t.FOLDER)) {
		if (file_read_directory(neural_node_dir())[0] == "") {
			file_create_directory(neural_node_dir());
		}
		file_start(neural_node_dir());
	}
}

function box_preferences_model_exists(file_name: string): bool {
	return iron_file_exists(neural_node_dir() + PATH_SEP + file_name);
}

function box_preferences_file_name_from_url(url: string): string {
	return substring(url, string_last_index_of(url, "/") + 1, url.length);
}

function box_preferneces_model_url_from_name(name: string): string {
	if (neural_node_models == null) {
		neural_node_models_init();
	}

	for (let i: i32 = 0; i < neural_node_models.length; ++i) {
		if (neural_node_models[i].name == name) {
			return neural_node_models[i].urls[0];
		}
	}
	return null;
}

function box_preferences_model_panel(m: neural_node_model_t) {
	if (ui_panel(ui_handle(m.name), m.name)) {
		if (ui_text(tr("source") + ": " + m.web + " (" + m.license + ")") == UI_STATE_RELEASED) {
			iron_load_url(m.web);
		}
		ui_text(tr("gpu memory") + ": " + m.memory);
		ui_text(tr("nodes") + ": " + m.nodes);

		let url: string       = m.urls[0];
		let file_name: string = box_preferences_file_name_from_url(url);
		let found: bool       = box_preferences_model_exists(file_name);

		if (neural_node_downloading > 0) {
			ui.enabled = false;

			let u: u64                   = iron_net_bytes_downloaded;
			let i: i32                   = (u / 1000000000) * 100;
			let f: f32                   = i / 100;
			let downloaded: string       = f + "GB";
			ui_box_hwnd.redraws          = 2;
			box_preferences_htab.redraws = 2;
			iron_delay_idle_sleep();

			let _BUTTON_COL: i32    = ui.ops.theme.BUTTON_COL;
			ui.ops.theme.BUTTON_COL = ui.ops.theme.HIGHLIGHT_COL;

			let h: ui_handle_t = ui_handle(__ID__);
			h.f                = f / parse_float(m.size);
			ui_slider(h, downloaded + " / " + m.size, 0.0, 1.0, true, 100, false, ui_align_t.CENTER);

			ui.ops.theme.BUTTON_COL = _BUTTON_COL;

			ui.enabled = true;
		}
		else if (!found && ui_icon_button(tr("Download") + " (" + m.size + ")", icon_t.ARROW_DOWN)) {
			neural_node_download_models(m.urls);
			console_info(tr("Downloading"));
		}
		else if (found && ui_icon_button(tr("Remove") + " (" + m.size + ")", icon_t.DELETE)) {
			for (let i: i32 = 0; i < m.urls.length; ++i) {
				let url: string       = m.urls[i];
				let file_name: string = box_preferences_file_name_from_url(url);
				iron_delete_file(neural_node_dir() + PATH_SEP + file_name);
			}
		}
	}
}

function box_preferences_plugins_tab() {
	ui_begin_sticky();
	let row: f32[] = [ 1 / 4, 1 / 4 ];
	ui_row(row);
	if (ui_icon_button(tr("New"), icon_t.PLUS)) {
		ui_box_show_custom(function() {
			ui_row2();
			let h: ui_handle_t = ui_handle(__ID__);
			if (h.init) {
				h.text = "new_plugin";
			}
			let plugin_name: string = ui_text_input(h, tr("Name"));
			if (ui_icon_button(tr("OK"), icon_t.CHECK) || ui.is_return_down) {
				let template: string = "let plugin = plugin_create();\
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
					plugin_name += ".js";
				}
				let path: string = path_data() + PATH_SEP + "plugins" + PATH_SEP + plugin_name;
				iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
				box_preferences_files_plugin = null; // Refresh file list
				ui_box_hide();
				box_preferences_htab.i = 6; // Plugins
				box_preferences_show();
			}
		}, 400, 200, null, true, tr("New Plugin"));
	}
	if (ui_icon_button(tr("Import"), icon_t.IMPORT)) {
		ui_files_show("js,zip", false, false, function(path: string) {
			import_plugin_run(path);
		});
	}
	ui_end_sticky();

	if (box_preferences_files_plugin == null) {
		box_preferences_fetch_plugins();
	}

	if (config_raw.plugins == null) {
		config_raw.plugins = [];
	}
	let h: ui_handle_t = ui_handle(__ID__);
	if (h.init) {
		h.b = false;
	}
	for (let i: i32 = 0; i < box_preferences_files_plugin.length; ++i) {
		let f: string   = box_preferences_files_plugin[i];
		let is_js: bool = ends_with(f, ".js");
		if (!is_js) {
			continue;
		}
		let enabled: bool = array_index_of(config_raw.plugins, f) >= 0;
		h.b               = enabled;
		let tag: string   = is_js ? string_split(f, ".")[0] : f;
		ui_check(h, tag);
		if (h.changed && h.b != enabled) {
			h.b ? config_enable_plugin(f) : config_disable_plugin(f);
			base_redraw_ui();
		}
		if (ui.is_hovered && ui.input_released_r) {
			_box_preferences_f = f;
			ui_menu_draw(function() {
				let path: string = path_data() + PATH_SEP + "plugins" + PATH_SEP + _box_preferences_f;
				if (ui_menu_button(tr("Edit in Text Editor"))) {
					file_start(path);
				}
				if (ui_menu_button(tr("Edit in Script Tab"))) {
					let blob: buffer_t       = data_get_blob("plugins/" + _box_preferences_f);
					tab_scripts_hscript.text = sys_buffer_to_string(blob);
					data_delete_blob("plugins/" + _box_preferences_f);
					console_info(tr("Script opened"));
				}
				if (ui_menu_button(tr("Export"), "", icon_t.EXPORT)) {
					ui_files_show("js", true, false, function(dest: string) {
						if (!ends_with(ui_files_filename, ".js")) {
							ui_files_filename += ".js";
						}
						let path: string = path_data() + PATH_SEP + "plugins" + PATH_SEP + _box_preferences_f;
						file_copy(path, dest + PATH_SEP + ui_files_filename);
					});
				}
				if (ui_menu_button(tr("Delete"), "", icon_t.DELETE)) {
					if (array_index_of(config_raw.plugins, _box_preferences_f) >= 0) {
						array_remove(config_raw.plugins, _box_preferences_f);
						plugin_stop(_box_preferences_f);
					}
					array_remove(box_preferences_files_plugin, _box_preferences_f);
					iron_delete_file(path);
				}
			});
		}
	}
}

function box_preferences_show() {
	ui_box_show_custom(
	    function() {
		    if (ui_tab(box_preferences_htab, tr("Interface"), true)) {
			    box_preferences_interface_tab();
		    }
		    if (ui_tab(box_preferences_htab, tr("Theme"), true)) {
			    box_preferences_theme_tab();
		    }
		    if (ui_tab(box_preferences_htab, tr("Usage"), true)) {
			    box_preferences_usage_tab();
		    }

		    /// if arm_ios
		    let pen_name: string = tr("Pencil");
		    /// else
		    let pen_name: string = tr("Pen");
		    /// end
		    if (ui_tab(box_preferences_htab, pen_name, true)) {
			    box_preferences_pen_tab();
		    }

		    if (ui_tab(box_preferences_htab, tr("Viewport"), true)) {
			    box_preferences_viewport_tab();
		    }
		    if (ui_tab(box_preferences_htab, tr("Keymap"), true)) {
			    box_preferences_keymap_tab();
		    }
		    /// if (arm_windows || arm_linux || arm_macos)
		    if (ui_tab(box_preferences_htab, tr("Neural"), true)) {
			    box_preferences_neural_tab();
		    }
		    /// end
		    if (ui_tab(box_preferences_htab, tr("Plugins"), true)) {
			    box_preferences_plugins_tab();
		    }
	    },
	    720, 520,
	    function() {
		    config_save();
	    },
	    true, tr("Preferences"));
}

function box_preferences_fetch_themes() {
	box_preferences_themes = file_read_directory(path_data() + PATH_SEP + "themes");
	for (let i: i32 = 0; i < box_preferences_themes.length; ++i) {
		let s: string             = box_preferences_themes[i];
		box_preferences_themes[i] = substring(box_preferences_themes[i], 0, s.length - 5); // Strip .json
	}
	array_insert(box_preferences_themes, 0, "default");
}

function box_preferences_fetch_keymaps() {
	box_preferences_files_keymap = file_read_directory(path_data() + PATH_SEP + "keymap_presets");
	for (let i: i32 = 0; i < box_preferences_files_keymap.length; ++i) {
		let s: string                   = box_preferences_files_keymap[i];
		box_preferences_files_keymap[i] = substring(box_preferences_files_keymap[i], 0, s.length - 5); // Strip .json
	}
	array_insert(box_preferences_files_keymap, 0, "default");
}

function box_preferences_fetch_plugins() {
	box_preferences_files_plugin = file_read_directory(path_data() + PATH_SEP + "plugins");
}

function box_preferences_get_theme_index(): i32 {
	return array_index_of(box_preferences_themes, substring(config_raw.theme, 0, config_raw.theme.length - 5)); // Strip .json
}

function box_preferences_get_preset_index(): i32 {
	return array_index_of(box_preferences_files_keymap, substring(config_raw.keymap, 0, config_raw.keymap.length - 5)); // Strip .json
}

function box_preferences_set_scale() {
	let scale: f32 = config_raw.window_scale;
	ui_set_scale(scale);
	ui_header_h                               = math_floor(ui_header_default_h * scale);
	config_raw.layout[layout_size_t.STATUS_H] = math_floor(ui_statusbar_default_h * scale);
	ui_menubar_w                              = math_floor(ui_menubar_default_w * scale);
	ui_base_set_icon_scale();
	base_resize();
	config_raw.layout[layout_size_t.SIDEBAR_W] = math_floor(ui_sidebar_default_w * scale);
}

function box_preferences_theme_to_json(theme: ui_theme_t): string {
	json_encode_begin();
	let u32_theme: u32_ptr = theme;
	for (let i: i32 = 0; i < ui_theme_keys_count; ++i) {
		let key: string = ARRAY_ACCESS(ui_theme_keys, i);
		let val: u32    = DEREFERENCE(u32_theme + i);
		json_encode_i32(key, val);
	}
	return json_encode_end();
}
