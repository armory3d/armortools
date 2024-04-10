
let box_preferences_htab: zui_handle_t = zui_handle_create();
let box_preferences_files_plugin: string[] = null;
let box_preferences_files_keymap: string[] = null;
let box_preferences_theme_handle: zui_handle_t;
let box_preferences_preset_handle: zui_handle_t;
let box_preferences_locales: string[] = null;
let box_preferences_themes: string[] = null;
let box_preferences_world_color: i32 = 0xff080808;

function box_preferences_show() {
	ui_box_show_custom(function (ui: zui_t) {
		if (zui_tab(box_preferences_htab, tr("Interface"), true)) {

			if (box_preferences_locales == null) {
				box_preferences_locales = translator_get_supported_locales();
			}

			let locale_handle: zui_handle_t = zui_handle(__ID__, { position: array_index_of(box_preferences_locales, config_raw.locale) });
			zui_combo(locale_handle, box_preferences_locales, tr("Language"), true);
			if (locale_handle.changed) {
				let locale_code: string = box_preferences_locales[locale_handle.position];
				config_raw.locale = locale_code;
				translator_load_translations(locale_code);
				ui_base_tag_ui_redraw();
			}

			let hscale: zui_handle_t = zui_handle(__ID__, { value: config_raw.window_scale });
			zui_slider(hscale, tr("UI Scale"), 1.0, 4.0, true, 10);
			if (context_raw.hscale_was_changed && !ui.input_down) {
				context_raw.hscale_was_changed = false;
				if (hscale.value == null) {
					hscale.value = 1.0;
				}
				config_raw.window_scale = hscale.value;
				box_preferences_set_scale();
			}
			if (hscale.changed) {
				context_raw.hscale_was_changed = true;
			}

			let hspeed: zui_handle_t = zui_handle(__ID__, { value: config_raw.camera_zoom_speed });
			config_raw.camera_zoom_speed = zui_slider(hspeed, tr("Camera Zoom Speed"), 0.1, 4.0, true);

			hspeed = zui_handle(__ID__, { value: config_raw.camera_rotation_speed });
			config_raw.camera_rotation_speed = zui_slider(hspeed, tr("Camera Rotation Speed"), 0.1, 4.0, true);

			hspeed = zui_handle(__ID__, { value: config_raw.camera_pan_speed });
			config_raw.camera_pan_speed = zui_slider(hspeed, tr("Camera Pan Speed"), 0.1, 4.0, true);

			let zoom_direction_handle: zui_handle_t = zui_handle(__ID__, { position: config_raw.zoom_direction });
			zui_combo(zoom_direction_handle, [tr("Vertical"), tr("Vertical Inverted"), tr("Horizontal"), tr("Horizontal Inverted"), tr("Vertical and Horizontal"), tr("Vertical and Horizontal Inverted")], tr("Direction to Zoom"), true);
			if (zoom_direction_handle.changed) {
				config_raw.zoom_direction = zoom_direction_handle.position;
			}

			config_raw.wrap_mouse = zui_check(zui_handle(__ID__, { selected: config_raw.wrap_mouse }), tr("Wrap Mouse"));
			if (ui.is_hovered) {
				zui_tooltip(tr("Wrap mouse around view boundaries during camera control"));
			}

			config_raw.node_preview = zui_check(zui_handle(__ID__, { selected: config_raw.node_preview }), tr("Show Node Preview"));

			ui.changed = false;
			config_raw.show_asset_names = zui_check(zui_handle(__ID__, { selected: config_raw.show_asset_names }), tr("Show Asset Names"));
			if (ui.changed) {
				ui_base_tag_ui_redraw();
			}

			///if !(krom_android || krom_ios)
			ui.changed = false;
			config_raw.touch_ui = zui_check(zui_handle(__ID__, { selected: config_raw.touch_ui }), tr("Touch UI"));
			if (ui.changed) {
				zui_set_touch_scroll(config_raw.touch_ui);
				zui_set_touch_hold(config_raw.touch_ui);
				zui_set_touch_tooltip(config_raw.touch_ui);
				config_load_theme(config_raw.theme);
				box_preferences_set_scale();
				ui_base_tag_ui_redraw();
			}
			///end

			config_raw.splash_screen = zui_check(zui_handle(__ID__, { selected: config_raw.splash_screen }), tr("Splash Screen"));

			// Zui.text("Node Editor");
			// let grid_snap: bool = Zui.check(Zui.handle("boxpreferences_11", { selected: false }), "Grid Snap");

			zui_end_element();
			zui_row([0.5, 0.5]);
			if (zui_button(tr("Restore")) && !ui_menu_show) {
				ui_menu_draw(function (ui: zui_t) {
					if (ui_menu_button(ui, tr("Confirm"))) {
						app_notify_on_init(function () {
							ui.ops.theme.ELEMENT_H = base_default_element_h;
							config_restore();
							box_preferences_set_scale();
							if (box_preferences_files_plugin != null) {
								for (let i: i32 = 0; i < box_preferences_files_plugin.length; ++i) {
									let f: string = box_preferences_files_plugin[i];
									plugin_stop(f);
								}
							}
							box_preferences_files_plugin = null;
							box_preferences_files_keymap = null;
							make_material_parse_mesh_material();
							make_material_parse_paint_material();
						});
					}
					if (ui_menu_button(ui, tr("Import..."))) {
						ui_files_show("json", false, false, function (path: string) {
							let b: buffer_t = data_get_blob(path);
							let raw: config_t = json_parse(sys_buffer_to_string(b));
							app_notify_on_init(function (raw: config_t) {
								ui.ops.theme.ELEMENT_H = base_default_element_h;
								config_import_from(raw);
								box_preferences_set_scale();
								make_material_parse_mesh_material();
								make_material_parse_paint_material();
							}, raw);
						});
					}
				}, 2);
			}
			if (zui_button(tr("Reset Layout")) && !ui_menu_show) {
				ui_menu_draw(function (ui: zui_t) {
					if (ui_menu_button(ui, tr("Confirm"))) {
						base_init_layout();
						config_save();
					}
				}, 1);
			}
		}

		if (zui_tab(box_preferences_htab, tr("Theme"), true)) {

			if (box_preferences_themes == null) {
				box_preferences_fetch_themes();
			}
			box_preferences_theme_handle = zui_handle(__ID__, { position: box_preferences_get_theme_index() });

			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

			zui_combo(box_preferences_theme_handle, box_preferences_themes, tr("Theme"));
			if (box_preferences_theme_handle.changed) {
				config_raw.theme = box_preferences_themes[box_preferences_theme_handle.position] + ".json";
				config_load_theme(config_raw.theme);
			}

			if (zui_button(tr("New"))) {
				ui_box_show_custom(function (ui: zui_t) {
					if (zui_tab(zui_handle(__ID__), tr("New Theme"))) {
						zui_row([0.5, 0.5]);
						let theme_name: string = zui_text_input(zui_handle(__ID__, { text: "new_theme" }), tr("Name"));
						if (zui_button(tr("OK")) || ui.is_return_down) {
							let template: string = json_stringify(base_theme);
							if (!ends_with(theme_name, ".json")) {
								theme_name += ".json";
							}
							let path: string = path_data() + path_sep + "themes" + path_sep + theme_name;
							krom_file_save_bytes(path, sys_string_to_buffer(template));
							box_preferences_fetch_themes(); // Refresh file list
							config_raw.theme = theme_name;
							box_preferences_theme_handle.position = box_preferences_get_theme_index();
							ui_box_hide();
							box_preferences_htab.position = 1; // Themes
							box_preferences_show();
						}
					}
				});
			}

			if (zui_button(tr("Import"))) {
				ui_files_show("json", false, false, function (path: string) {
					import_theme_run(path);
				});
			}

			if (zui_button(tr("Export"))) {
				ui_files_show("json", true, false, function (path: string) {
					path += path_sep + ui_files_filename;
					if (!ends_with(path, ".json")) {
						path += ".json";
					}
					krom_file_save_bytes(path, sys_string_to_buffer(json_stringify(base_theme)));
				});
			}

			zui_end_sticky();

			let i: i32 = 0;
			let theme: any = base_theme;
			let hlist: zui_handle_t = zui_handle(__ID__);

			// Viewport color
			let h: zui_handle_t = zui_nest(hlist, i++, { color: box_preferences_world_color });
			zui_row([1 / 8, 7 / 8]);
			zui_text("", 0, h.color);
			if (ui.is_hovered && ui.input_released) {
				ui_menu_draw(function (ui: zui_t) {
					ui.changed = false;
					zui_color_wheel(h, false, null, 11 * ui.ops.theme.ELEMENT_H * zui_SCALE(ui), true);
					if (ui.changed) {
						ui_menu_keep_open = true;
					}
				}, 11);
			}
			let val: i32 = h.color;
			if (val < 0) {
				val += 4294967296;
			}
			h.text = i32_to_string_hex(val);
			zui_text_input(h, "VIEWPORT_COL");
			h.color = parse_int_hex(h.text);

			if (box_preferences_world_color != h.color) {
				box_preferences_world_color = h.color;
				let b: u8_array_t = u8_array_create(4);
				b[0] = color_get_rb(box_preferences_world_color);
				b[1] = color_get_gb(box_preferences_world_color);
				b[2] = color_get_bb(box_preferences_world_color);
				b[3] = 255;
				context_raw.empty_envmap = image_from_bytes(b.buffer, 1, 1);
				context_raw.ddirty = 2;
				if (!context_raw.show_envmap) {
					scene_world._.envmap = context_raw.empty_envmap;
				}
			}

			// Theme fields
			for (let i: i32 = 0; i < zui_theme_keys.length; ++i) {
				let key: string = zui_theme_keys[i];

				let h: zui_handle_t = zui_nest(hlist, i);
				let val: any = theme[key];

				let is_hex: bool = ends_with(key, "_COL");
				if (is_hex && val < 0) {
					val += 4294967296;
				}

				if (is_hex) {
					zui_row([1 / 8, 7 / 8]);
					zui_text("", 0, val);
					if (ui.is_hovered && ui.input_released) {
						h.color = theme[key];
						ui_menu_draw(function (ui: zui_t) {
							ui.changed = false;
							let color: i32 = zui_color_wheel(h, false, null, 11 * ui.ops.theme.ELEMENT_H * zui_SCALE(ui), true);
							let theme: any = base_theme;
							theme[key] = color;
							if (ui.changed) {
								ui_menu_keep_open = true;
							}
						}, 11);
					}
				}

				ui.changed = false;

				if (key == "FILL_WINDOW_BG" ||
					key == "FILL_BUTTON_BG" ||
					key == "FILL_ACCENT_BG" ||
					key == "FULL_TABS" ||
					key == "ROUND_CORNERS") {
					h.selected = val;
					let b: bool = zui_check(h, key);
					theme[key] = b;
				}
				else if (key == "LINK_STYLE") {
					let styles: string[] = [tr("Straight"), tr("Curved")];
					h.position = val;
					let i: i32 = zui_combo(h, styles, key, true);
					theme[key] = i;
				}
				else {
					h.text = is_hex ? i32_to_string_hex(val) : i32_to_string(val);
					zui_text_input(h, key);
					if (is_hex) {
						theme[key] = parse_int_hex(h.text);
					}
					else {
						theme[key] = parse_int(h.text);
					}
				}

				if (ui.changed) {
					for (let i: i32 = 0; i < base_get_uis().length; ++i) {
						let ui: zui_t = base_get_uis()[i];
						ui.elements_baked = false;
					}
				}
			}
		}

		if (zui_tab(box_preferences_htab, tr("Usage"), true)) {
			context_raw.undo_handle = zui_handle(__ID__, { value: config_raw.undo_steps });
			config_raw.undo_steps = math_floor(zui_slider(context_raw.undo_handle, tr("Undo Steps"), 1, 64, false, 1));
			if (config_raw.undo_steps < 1) {
				config_raw.undo_steps = math_floor(context_raw.undo_handle.value = 1);
			}
			if (context_raw.undo_handle.changed) {
				let current: image_t = _g2_current;
				g2_end();

				///if (is_paint || is_sculpt)
				while (history_undo_layers.length < config_raw.undo_steps) {
					let l: slot_layer_t = slot_layer_create("_undo" + history_undo_layers.length);
					array_push(history_undo_layers, l);
				}
				while (history_undo_layers.length > config_raw.undo_steps) {
					let l: slot_layer_t = history_undo_layers.pop();
					slot_layer_unload(l);
				}
				///end

				history_reset();
				g2_begin(current);
			}

			///if is_paint
			config_raw.dilate_radius = math_floor(zui_slider(zui_handle(__ID__, { value: config_raw.dilate_radius }), tr("Dilate Radius"), 0.0, 16.0, true, 1));
			if (ui.is_hovered) {
				zui_tooltip(tr("Dilate painted textures to prevent seams"));
			}

			let dilate_handle: zui_handle_t = zui_handle(__ID__, { position: config_raw.dilate });
			zui_combo(dilate_handle, [tr("Instant"), tr("Delayed")], tr("Dilate"), true);
			if (dilate_handle.changed) {
				config_raw.dilate = dilate_handle.position;
			}
			///end

			///if is_lab
			let workspace_handle: zui_handle_t = zui_handle(__ID__, { position: config_raw.workspace });
			zui_combo(workspace_handle, [tr("3D View"), tr("2D View")], tr("Default Workspace"), true);
			if (workspace_handle.changed) {
				config_raw.workspace = workspace_handle.position;
			}
			///end

			let camera_controls_handle: zui_handle_t = zui_handle(__ID__, { position: config_raw.camera_controls });
			zui_combo(camera_controls_handle, [tr("Orbit"), tr("Rotate"), tr("Fly")], tr("Default Camera Controls"), true);
			if (camera_controls_handle.changed) {
				config_raw.camera_controls = camera_controls_handle.position;
			}

			let layer_res_handle: zui_handle_t = zui_handle(__ID__, { position: config_raw.layer_res });

			///if is_paint
			///if (krom_android || krom_ios)
			zui_combo(layer_res_handle, ["128", "256", "512", "1K", "2K", "4K"], tr("Default Layer Resolution"), true);
			///else
			zui_combo(layer_res_handle, ["128", "256", "512", "1K", "2K", "4K", "8K"], tr("Default Layer Resolution"), true);
			///end
			///end

			///if is_lab
			///if (krom_android || krom_ios)
			zui_combo(layer_res_handle, ["2K", "4K"], tr("Default Layer Resolution"), true);
			///else
			zui_combo(layer_res_handle, ["2K", "4K", "8K", "16K"], tr("Default Layer Resolution"), true);
			///end
			///end

			if (layer_res_handle.changed) {
				config_raw.layer_res = layer_res_handle.position;
			}

			let server_handle: zui_handle_t = zui_handle(__ID__, { text: config_raw.server });
			config_raw.server = zui_text_input(server_handle, tr("Cloud Server"));

			///if (is_paint || is_sculpt)
			let material_live_handle: zui_handle_t = zui_handle(__ID__, {selected: config_raw.material_live });
			config_raw.material_live = zui_check(material_live_handle, tr("Live Material Preview"));
			if (ui.is_hovered) {
				zui_tooltip(tr("Instantly update material preview on node change"));
			}

			let brush_live_handle: zui_handle_t = zui_handle(__ID__, { selected: config_raw.brush_live });
			config_raw.brush_live = zui_check(brush_live_handle, tr("Live Brush Preview"));
			if (ui.is_hovered) {
				zui_tooltip(tr("Draw live brush preview in viewport"));
			}
			if (brush_live_handle.changed) {
				context_raw.ddirty = 2;
			}

			let brush_3d_handle: zui_handle_t = zui_handle(__ID__, { selected: config_raw.brush_3d });
			config_raw.brush_3d = zui_check(brush_3d_handle, tr("3D Cursor"));
			if (brush_3d_handle.changed) {
				make_material_parse_paint_material();
			}

			ui.enabled = config_raw.brush_3d;
			let brush_depth_reject_handle: zui_handle_t = zui_handle(__ID__, { selected: config_raw.brush_depth_reject });
			config_raw.brush_depth_reject = zui_check(brush_depth_reject_handle, tr("Depth Reject"));
			if (brush_depth_reject_handle.changed) {
				make_material_parse_paint_material();
			}

			zui_row([0.5, 0.5]);

			let brush_angle_reject_handle: zui_handle_t = zui_handle(__ID__, { selected: config_raw.brush_angle_reject });
			config_raw.brush_angle_reject = zui_check(brush_angle_reject_handle, tr("Angle Reject"));
			if (brush_angle_reject_handle.changed) {
				make_material_parse_paint_material();
			}

			if (!config_raw.brush_angle_reject) ui.enabled = false;
			let angle_dot_handle: zui_handle_t = zui_handle(__ID__, { value: context_raw.brush_angle_reject_dot });
			context_raw.brush_angle_reject_dot = zui_slider(angle_dot_handle, tr("Angle"), 0.0, 1.0, true);
			if (angle_dot_handle.changed) {
				make_material_parse_paint_material();
			}
			ui.enabled = true;
			///end

			///if is_lab
			config_raw.gpu_inference = zui_check(zui_handle(__ID__, { selected: config_raw.gpu_inference }), tr("Use GPU"));
			if (ui.is_hovered) {
				zui_tooltip(tr("Use GPU to accelerate node graph processing"));
			}
			///end
		}

		let pen_name: string;
		///if krom_ios
		pen_name = tr("Pencil");
		///else
		pen_name = tr("Pen");
		///end

		if (zui_tab(box_preferences_htab, pen_name, true)) {
			zui_text(tr("Pressure controls"));
			config_raw.pressure_radius = zui_check(zui_handle(__ID__, { selected: config_raw.pressure_radius }), tr("Brush Radius"));
			config_raw.pressure_sensitivity = zui_slider(zui_handle(__ID__, { value: config_raw.pressure_sensitivity }), tr("Sensitivity"), 0.0, 10.0, true);
			///if (is_paint || is_sculpt)
			config_raw.pressure_hardness = zui_check(zui_handle(__ID__, { selected: config_raw.pressure_hardness }), tr("Brush Hardness"));
			config_raw.pressure_opacity = zui_check(zui_handle(__ID__, { selected: config_raw.pressure_opacity }), tr("Brush Opacity"));
			config_raw.pressure_angle = zui_check(zui_handle(__ID__, { selected: config_raw.pressure_angle }), tr("Brush Angle"));
			///end

			zui_end_element();
			zui_row([0.5]);
			if (zui_button(tr("Help"))) {
				///if (is_paint || is_sculpt)
				file_load_url("https://github.com/armory3d/armorpaint_docs///pen");
				///end
				///if is_lab
				file_load_url("https://github.com/armory3d/armorlab_docs///pen");
				///end
			}
		}

		context_raw.hssao = zui_handle(__ID__, { selected: config_raw.rp_ssao });
		context_raw.hssr = zui_handle(__ID__, { selected: config_raw.rp_ssr });
		context_raw.hbloom = zui_handle(__ID__, { selected: config_raw.rp_bloom });
		context_raw.hsupersample = zui_handle(__ID__, { position: config_get_super_sample_quality(config_raw.rp_supersample) });
		context_raw.hvxao = zui_handle(__ID__, { selected: config_raw.rp_gi });
		if (zui_tab(box_preferences_htab, tr("Viewport"), true)) {
			///if (krom_direct3d12 || krom_vulkan || krom_metal)

			let hpathtrace_mode: zui_handle_t = zui_handle(__ID__, { position: context_raw.pathtrace_mode });
			context_raw.pathtrace_mode = zui_combo(hpathtrace_mode, [tr("Core"), tr("Full")], tr("Path Tracer"), true);
			if (hpathtrace_mode.changed) {
				render_path_raytrace_ready = false;
			}

			///end

			let hrender_mode: zui_handle_t = zui_handle(__ID__, { position: context_raw.render_mode });
			context_raw.render_mode = zui_combo(hrender_mode, [tr("Full"), tr("Mobile")], tr("Renderer"), true);
			if (hrender_mode.changed) {
				context_set_render_path();
			}

			zui_combo(context_raw.hsupersample, ["0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x"], tr("Super Sample"), true);
			if (context_raw.hsupersample.changed) {
				config_apply();
			}

			if (context_raw.render_mode == render_mode_t.DEFERRED) {
				///if arm_voxels
				zui_check(context_raw.hvxao, tr("Voxel AO"));
				if (ui.is_hovered) {
					zui_tooltip(tr("Cone-traced AO and shadows"));
				}
				if (context_raw.hvxao.changed) {
					config_apply();
				}

				ui.enabled = context_raw.hvxao.selected;
				let h: zui_handle_t = zui_handle(__ID__, { value: context_raw.vxao_offset });
				context_raw.vxao_offset = zui_slider(h, tr("Cone Offset"), 1.0, 4.0, true);
				if (h.changed) {
					context_raw.ddirty = 2;
				}
				h = zui_handle(__ID__, { value: context_raw.vxao_aperture });
				context_raw.vxao_aperture = zui_slider(h, tr("Aperture"), 1.0, 4.0, true);
				if (h.changed) {
					context_raw.ddirty = 2;
				}
				ui.enabled = true;
				///end

				zui_check(context_raw.hssao, tr("SSAO"));
				if (context_raw.hssao.changed) {
					config_apply();
				}
				zui_check(context_raw.hssr, tr("SSR"));
				if (context_raw.hssr.changed) {
					config_apply();
				}
				zui_check(context_raw.hbloom, tr("Bloom"));
				if (context_raw.hbloom.changed) {
					config_apply();
				}
			}

			let h: zui_handle_t = zui_handle(__ID__, { value: config_raw.rp_vignette });
			config_raw.rp_vignette = zui_slider(h, tr("Vignette"), 0.0, 1.0, true);
			if (h.changed) {
				context_raw.ddirty = 2;
			}

			h = zui_handle(__ID__, { value: config_raw.rp_grain });
			config_raw.rp_grain = zui_slider(h, tr("Noise Grain"), 0.0, 1.0, true);
			if (h.changed) {
				context_raw.ddirty = 2;
			}

			// let h: zui_handle_t = Zui.handle("boxpreferences_46", { value: raw.autoExposureStrength });
			// raw.autoExposureStrength = Zui.slider(h, "Auto Exposure", 0.0, 2.0, true);
			// if (h.changed) raw.ddirty = 2;

			let cam: camera_object_t = scene_camera;
			let cam_raw: camera_data_t = cam.data;
			let near_handle: zui_handle_t = zui_handle(__ID__);
			let far_handle: zui_handle_t = zui_handle(__ID__);
			near_handle.value = math_floor(cam_raw.near_plane * 1000) / 1000;
			far_handle.value = math_floor(cam_raw.far_plane * 100) / 100;
			cam_raw.near_plane = zui_slider(near_handle, tr("Clip Start"), 0.001, 1.0, true);
			cam_raw.far_plane = zui_slider(far_handle, tr("Clip End"), 50.0, 100.0, true);
			if (near_handle.changed || far_handle.changed) {
				camera_object_build_proj(cam);
			}

			let disp_handle: zui_handle_t = zui_handle(__ID__, { value: config_raw.displace_strength });
			config_raw.displace_strength = zui_slider(disp_handle, tr("Displacement Strength"), 0.0, 10.0, true);
			if (disp_handle.changed) {
				context_raw.ddirty = 2;
				make_material_parse_mesh_material();
			}
		}
		if (zui_tab(box_preferences_htab, tr("Keymap"), true)) {

			if (box_preferences_files_keymap == null) {
				box_preferences_fetch_keymaps();
			}

			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

			box_preferences_preset_handle = zui_handle(__ID__, { position: box_preferences_get_preset_index() });
			zui_combo(box_preferences_preset_handle, box_preferences_files_keymap, tr("Preset"));
			if (box_preferences_preset_handle.changed) {
				config_raw.keymap = box_preferences_files_keymap[box_preferences_preset_handle.position] + ".json";
				config_apply();
				config_load_keymap();
			}

			if (zui_button(tr("New"))) {
				ui_box_show_custom(function (ui: zui_t) {
					if (zui_tab(zui_handle(__ID__), tr("New Keymap"))) {
						zui_row([0.5, 0.5]);
						let keymap_name: string = zui_text_input(zui_handle(__ID__, { text: "new_keymap" }), tr("Name"));
						if (zui_button(tr("OK")) || ui.is_return_down) {
							let template: string = json_stringify(base_get_default_keymap());
							if (!ends_with(keymap_name, ".json")) {
								keymap_name += ".json";
							}
							let path: string = path_data() + path_sep + "keymap_presets" + path_sep + keymap_name;
							krom_file_save_bytes(path, sys_string_to_buffer(template));
							box_preferences_fetch_keymaps(); // Refresh file list
							config_raw.keymap = keymap_name;
							box_preferences_preset_handle.position = box_preferences_get_preset_index();
							ui_box_hide();
							box_preferences_htab.position = 5; // Keymap
							box_preferences_show();
						}
					}
				});
			}

			if (zui_button(tr("Import"))) {
				ui_files_show("json", false, false, function (path: string) {
					import_keymap_run(path);
				});
			}
			if (zui_button(tr("Export"))) {
				ui_files_show("json", true, false, function (dest: string) {
					if (!ends_with(ui_files_filename, ".json")) {
						ui_files_filename += ".json";
					}
					let path: string = path_data() + path_sep + "keymap_presets" + path_sep + config_raw.keymap;
					file_copy(path, dest + path_sep + ui_files_filename);
				});
			}

			zui_end_sticky();

			zui_separator(8, false);

			let index: i32 = 0;
			ui.changed = false;
			let keys: string[] = map_keys_to_array(config_keymap);
			for (let i: i32 = 0; i < keys.length; ++i) {
				let key: string = keys[i];
				let h: zui_handle_t = zui_nest(zui_handle(__ID__), index++);
				h.text = map_get(config_keymap, key);
				let text: string = zui_text_input(h, key, zui_align_t.LEFT);
				map_set(config_keymap, key, text);
			}
			if (ui.changed) {
				config_apply();
				config_save_keymap();
			}
		}
		if (zui_tab(box_preferences_htab, tr("Plugins"), true)) {
			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4]);
			if (zui_button(tr("New"))) {
				ui_box_show_custom(function (ui: zui_t) {
					if (zui_tab(zui_handle(__ID__), tr("New Plugin"))) {
						zui_row([0.5, 0.5]);
						let plugin_name: string = zui_text_input(zui_handle(__ID__, { text: "new_plugin" }), tr("Name"));
						if (zui_button(tr("OK")) || ui.is_return_down) {
							let template: string =
"let plugin = create();\
let h1 = zui_handle_create();\
plugin.draw_ui = function (ui) {\
	if (zui_panel(h1, \"New Plugin\")) {\
		if (zui_button(\"Button\")) {\
			console.error(\"Hello\");\
		}\
	}\
}\
";
							if (!ends_with(plugin_name, ".js")) {
								plugin_name += ".js";
							}
							let path: string = path_data() + path_sep + "plugins" + path_sep + plugin_name;
							krom_file_save_bytes(path, sys_string_to_buffer(template));
							box_preferences_files_plugin = null; // Refresh file list
							ui_box_hide();
							box_preferences_htab.position = 6; // Plugins
							box_preferences_show();
						}
					}
				});
			}
			if (zui_button(tr("Import"))) {
				ui_files_show("js,zip", false, false, function (path: string) {
					import_plugin_run(path);
				});
			}
			zui_end_sticky();

			if (box_preferences_files_plugin == null) {
				box_preferences_fetch_plugins();
			}

			if (config_raw.plugins == null) {
				config_raw.plugins = [];
			}
			let h: zui_handle_t = zui_handle(__ID__, { selected: false });
			for (let i: i32 = 0; i < box_preferences_files_plugin.length; ++i) {
				let f: string = box_preferences_files_plugin[i];
				let is_js: bool = ends_with(f, ".js");
				if (!is_js) {
					continue;
				}
				let enabled: bool = array_index_of(config_raw.plugins, f) >= 0;
				h.selected = enabled;
				let tag: string = is_js ? string_split(f, ".")[0] : f;
				zui_check(h, tag);
				if (h.changed && h.selected != enabled) {
					h.selected ? config_enable_plugin(f) : config_disable_plugin(f);
					base_redraw_ui();
				}
				if (ui.is_hovered && ui.input_released_r) {
					ui_menu_draw(function (ui: zui_t) {
						let path: string = path_data() + path_sep + "plugins" + path_sep + f;
						if (ui_menu_button(ui, tr("Edit in Text Editor"))) {
							file_start(path);
						}
						if (ui_menu_button(ui, tr("Edit in Script Tab"))) {
							let blob: buffer_t = data_get_blob("plugins/" + f);
							tab_script_hscript.text = sys_buffer_to_string(blob);
							data_delete_blob("plugins/" + f);
							console_info(tr("Script opened"));
						}
						if (ui_menu_button(ui, tr("Export"))) {
							ui_files_show("js", true, false, function (dest: string) {
								if (!ends_with(ui_files_filename, ".js")) {
									ui_files_filename += ".js";
								}
								let path: string = path_data() + path_sep + "plugins" + path_sep + f;
								file_copy(path, dest + path_sep + ui_files_filename);
							});
						}
						if (ui_menu_button(ui, tr("Delete"))) {
							if (array_index_of(config_raw.plugins, f) >= 0) {
								array_remove(config_raw.plugins, f);
								plugin_stop(f);
							}
							array_remove(box_preferences_files_plugin, f);
							file_delete(path);
						}
					}, 4);
				}
			}
		}

	}, 620, config_raw.touch_ui ? 480 : 420, function () {
		config_save();
	});
}

function box_preferences_fetch_themes() {
	box_preferences_themes = file_read_directory(path_data() + path_sep + "themes");
	for (let i: i32 = 0; i < box_preferences_themes.length; ++i) {
		box_preferences_themes[i] = substring(box_preferences_themes[i], 0, box_preferences_themes[i].length - 5); // Strip .json
	}
	box_preferences_themes.unshift("default");
}

function box_preferences_fetch_keymaps() {
	box_preferences_files_keymap = file_read_directory(path_data() + path_sep + "keymap_presets");
	for (let i: i32 = 0; i < box_preferences_files_keymap.length; ++i) {
		box_preferences_files_keymap[i] = substring(box_preferences_files_keymap[i], 0, box_preferences_files_keymap[i].length - 5); // Strip .json
	}
	box_preferences_files_keymap.unshift("default");
}

function box_preferences_fetch_plugins() {
	box_preferences_files_plugin = file_read_directory(path_data() + path_sep + "plugins");
}

function box_preferences_get_theme_index(): i32 {
	return array_index_of(box_preferences_themes, substring(config_raw.theme, 0, config_raw.theme.length - 5)); // Strip .json
}

function box_preferences_get_preset_index(): i32 {
	return array_index_of(box_preferences_files_keymap, substring(config_raw.keymap, 0, config_raw.keymap.length - 5)); // Strip .json
}

function box_preferences_set_scale() {
	let scale: f32 = config_raw.window_scale;
	zui_set_scale(ui_base_ui, scale);
	ui_header_h = math_floor(ui_header_default_h * scale);
	config_raw.layout[layout_size_t.STATUS_H] = math_floor(ui_status_default_status_h * scale);
	ui_menubar_w = math_floor(ui_menubar_default_w * scale);
	ui_base_set_icon_scale();
	zui_set_scale(ui_nodes_ui, scale);
	zui_set_scale(ui_view2d_ui, scale);
	zui_set_scale(base_ui_box, scale);
	zui_set_scale(base_ui_menu, scale);
	base_resize();
	///if (is_paint || is_sculpt)
	config_raw.layout[layout_size_t.SIDEBAR_W] = math_floor(ui_base_default_sidebar_w * scale);
	ui_toolbar_w = math_floor(ui_toolbar_default_w * scale);
	///end
}
