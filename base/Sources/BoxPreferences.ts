
class BoxPreferences {

	static htab : zui_handle_t= zui_handle_create();
	static files_plugin: string[] = null;
	static files_keymap: string[] = null;
	static theme_handle: zui_handle_t;
	static preset_handle: zui_handle_t;
	static locales: string[] = null;
	static themes: string[] = null;
	static world_color: i32 = 0xff080808;

	static show = () => {

		UIBox.show_custom((ui: zui_t) => {
			if (zui_tab(BoxPreferences.htab, tr("Interface"), true)) {

				if (BoxPreferences.locales == null) {
					BoxPreferences.locales = translator_get_supported_locales();
				}

				let locale_handle: zui_handle_t = zui_handle("boxpreferences_0", { position: BoxPreferences.locales.indexOf(config_raw.locale) });
				zui_combo(locale_handle, BoxPreferences.locales, tr("Language"), true);
				if (locale_handle.changed) {
					let locale_code: string = BoxPreferences.locales[locale_handle.position];
					config_raw.locale = locale_code;
					translator_load_translations(locale_code);
					UIBase.tag_ui_redraw();
				}

				let hscale: zui_handle_t = zui_handle("boxpreferences_1", { value: config_raw.window_scale });
				zui_slider(hscale, tr("UI Scale"), 1.0, 4.0, true, 10);
				if (context_raw.hscale_was_changed && !ui.input_down) {
					context_raw.hscale_was_changed = false;
					if (hscale.value == null || isNaN(hscale.value)) hscale.value = 1.0;
					config_raw.window_scale = hscale.value;
					BoxPreferences.set_scale();
				}
				if (hscale.changed) context_raw.hscale_was_changed = true;

				let hspeed: zui_handle_t = zui_handle("boxpreferences_2", { value: config_raw.camera_zoom_speed });
				config_raw.camera_zoom_speed = zui_slider(hspeed, tr("Camera Zoom Speed"), 0.1, 4.0, true);

				hspeed = zui_handle("boxpreferences_3", { value: config_raw.camera_rotation_speed });
				config_raw.camera_rotation_speed = zui_slider(hspeed, tr("Camera Rotation Speed"), 0.1, 4.0, true);

				hspeed = zui_handle("boxpreferences_4", { value: config_raw.camera_pan_speed });
				config_raw.camera_pan_speed = zui_slider(hspeed, tr("Camera Pan Speed"), 0.1, 4.0, true);

				let zoom_direction_handle: zui_handle_t = zui_handle("boxpreferences_5", { position: config_raw.zoom_direction });
				zui_combo(zoom_direction_handle, [tr("Vertical"), tr("Vertical Inverted"), tr("Horizontal"), tr("Horizontal Inverted"), tr("Vertical and Horizontal"), tr("Vertical and Horizontal Inverted")], tr("Direction to Zoom"), true);
				if (zoom_direction_handle.changed) {
					config_raw.zoom_direction = zoom_direction_handle.position;
				}

				config_raw.wrap_mouse = zui_check(zui_handle("boxpreferences_6", { selected: config_raw.wrap_mouse }), tr("Wrap Mouse"));
				if (ui.is_hovered) zui_tooltip(tr("Wrap mouse around view boundaries during camera control"));

				config_raw.node_preview = zui_check(zui_handle("boxpreferences_7", { selected: config_raw.node_preview }), tr("Show Node Preview"));

				ui.changed = false;
				config_raw.show_asset_names = zui_check(zui_handle("boxpreferences_8", { selected: config_raw.show_asset_names }), tr("Show Asset Names"));
				if (ui.changed) {
					UIBase.tag_ui_redraw();
				}

				///if !(krom_android || krom_ios)
				ui.changed = false;
				config_raw.touch_ui = zui_check(zui_handle("boxpreferences_9", { selected: config_raw.touch_ui }), tr("Touch UI"));
				if (ui.changed) {
					zui_set_touch_scroll(config_raw.touch_ui);
					zui_set_touch_hold(config_raw.touch_ui);
					zui_set_touch_tooltip(config_raw.touch_ui);
					config_load_theme(config_raw.theme);
					BoxPreferences.set_scale();
					UIBase.tag_ui_redraw();
				}
				///end

				config_raw.splash_screen = zui_check(zui_handle("boxpreferences_10", { selected: config_raw.splash_screen }), tr("Splash Screen"));

				// Zui.text("Node Editor");
				// let grid_snap: bool = Zui.check(Zui.handle("boxpreferences_11", { selected: false }), "Grid Snap");

				zui_end_element();
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Restore")) && !UIMenu.show) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Confirm"))) {
							app_notify_on_init(() => {
								ui.t.ELEMENT_H = base_default_element_h;
								config_restore();
								BoxPreferences.set_scale();
								if (BoxPreferences.files_plugin != null) for (let f of BoxPreferences.files_plugin) plugin_stop(f);
								BoxPreferences.files_plugin = null;
								BoxPreferences.files_keymap = null;
								MakeMaterial.parse_mesh_material();
								MakeMaterial.parse_paint_material();
							});
						}
						if (UIMenu.menu_button(ui, tr("Import..."))) {
							UIFiles.show("json", false, false, (path: string) => {
								let b: ArrayBuffer = data_get_blob(path);
								let raw: config_t = JSON.parse(sys_buffer_to_string(b));
								app_notify_on_init(() => {
									ui.t.ELEMENT_H = base_default_element_h;
									config_import_from(raw);
									BoxPreferences.set_scale();
									MakeMaterial.parse_mesh_material();
									MakeMaterial.parse_paint_material();
								});
							});
						}
					}, 2);
				}
				if (zui_button(tr("Reset Layout")) && !UIMenu.show) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Confirm"))) {
							base_init_layout();
							config_save();
						}
					}, 1);
				}
			}

			if (zui_tab(BoxPreferences.htab, tr("Theme"), true)) {

				if (BoxPreferences.themes == null) {
					BoxPreferences.fetch_themes();
				}
				BoxPreferences.theme_handle = zui_handle("boxpreferences_12", { position: BoxPreferences.get_theme_index() });

				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

				zui_combo(BoxPreferences.theme_handle, BoxPreferences.themes, tr("Theme"));
				if (BoxPreferences.theme_handle.changed) {
					config_raw.theme = BoxPreferences.themes[BoxPreferences.theme_handle.position] + ".json";
					config_load_theme(config_raw.theme);
				}

				if (zui_button(tr("New"))) {
					UIBox.show_custom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_13"), tr("New Theme"))) {
							zui_row([0.5, 0.5]);
							let theme_name: string = zui_text_input(zui_handle("boxpreferences_14", { text: "new_theme" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template: string = JSON.stringify(base_theme);
								if (!theme_name.endsWith(".json")) theme_name += ".json";
								let path: string = path_data() + path_sep + "themes" + path_sep + theme_name;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.fetch_themes(); // Refresh file list
								config_raw.theme = theme_name;
								BoxPreferences.theme_handle.position = BoxPreferences.get_theme_index();
								UIBox.hide();
								BoxPreferences.htab.position = 1; // Themes
								BoxPreferences.show();
							}
						}
					});
				}

				if (zui_button(tr("Import"))) {
					UIFiles.show("json", false, false, (path: string) => {
						ImportTheme.run(path);
					});
				}

				if (zui_button(tr("Export"))) {
					UIFiles.show("json", true, false, (path: string) => {
						path += path_sep + UIFiles.filename;
						if (!path.endsWith(".json")) path += ".json";
						krom_file_save_bytes(path, sys_string_to_buffer(JSON.stringify(base_theme)));
					});
				}

				zui_end_sticky();

				let i: i32 = 0;
				let theme: any = base_theme;
				let hlist: zui_handle_t = zui_handle("boxpreferences_15");

				// Viewport color
				let h: zui_handle_t = zui_nest(hlist, i++, { color: BoxPreferences.world_color });
				zui_row([1 / 8, 7 / 8]);
				zui_text("", 0, h.color);
				if (ui.is_hovered && ui.input_released) {
					UIMenu.draw((ui) => {
						ui.changed = false;
						zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true);
						if (ui.changed) UIMenu.keep_open = true;
					}, 11);
				}
				let val: i32 = h.color;
				if (val < 0) val += 4294967296;
				h.text = val.toString(16);
				zui_text_input(h, "VIEWPORT_COL");
				h.color = parseInt(h.text, 16);

				if (BoxPreferences.world_color != h.color) {
					BoxPreferences.world_color = h.color;
					let b: Uint8Array = new Uint8Array(4);
					b[0] = color_get_rb(BoxPreferences.world_color);
					b[1] = color_get_gb(BoxPreferences.world_color);
					b[2] = color_get_bb(BoxPreferences.world_color);
					b[3] = 255;
					context_raw.empty_envmap = image_from_bytes(b.buffer, 1, 1);
					context_raw.ddirty = 2;
					if (!context_raw.show_envmap) {
						scene_world._.envmap = context_raw.empty_envmap;
					}
				}

				// Theme fields
				for (let key of Object.getOwnPropertyNames(theme_t.prototype)) {
					if (key == "constructor") continue;

					let h: zui_handle_t = zui_nest(hlist, i++);
					let val: any = theme[key];

					let isHex: bool = key.endsWith("_COL");
					if (isHex && val < 0) val += 4294967296;

					if (isHex) {
						zui_row([1 / 8, 7 / 8]);
						zui_text("", 0, val);
						if (ui.is_hovered && ui.input_released) {
							h.color = theme[key];
							UIMenu.draw((ui) => {
								ui.changed = false;
								let color: i32 = zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true);
								theme[key] = color;
								if (ui.changed) UIMenu.keep_open = true;
							}, 11);
						}
					}

					ui.changed = false;

					if (typeof val == "boolean") {
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
						h.text = isHex ? val.toString(16) : val.toString();
						zui_text_input(h, key);
						if (isHex) theme[key] = parseInt(h.text, 16);
						else theme[key] = parseInt(h.text);
					}

					if (ui.changed) {
						for (let ui of base_get_uis()) {
							ui.elements_baked = false;
						}
					}
				}
			}

			if (zui_tab(BoxPreferences.htab, tr("Usage"), true)) {
				context_raw.undo_handle = zui_handle("boxpreferences_16", { value: config_raw.undo_steps });
				config_raw.undo_steps = Math.floor(zui_slider(context_raw.undo_handle, tr("Undo Steps"), 1, 64, false, 1));
				if (config_raw.undo_steps < 1) {
					config_raw.undo_steps = Math.floor(context_raw.undo_handle.value = 1);
				}
				if (context_raw.undo_handle.changed) {
					let current: image_t = _g2_current;
					g2_end();

					///if (is_paint || is_sculpt)
					while (history_undo_layers.length < config_raw.undo_steps) {
						let l: SlotLayerRaw = SlotLayer.create("_undo" + history_undo_layers.length);
						history_undo_layers.push(l);
					}
					while (history_undo_layers.length > config_raw.undo_steps) {
						let l: SlotLayerRaw = history_undo_layers.pop();
						SlotLayer.unload(l);
					}
					///end

					history_reset();
					g2_begin(current);
				}

				///if is_paint
				config_raw.dilate_radius = Math.floor(zui_slider(zui_handle("boxpreferences_17", { value: config_raw.dilate_radius }), tr("Dilate Radius"), 0.0, 16.0, true, 1));
				if (ui.is_hovered) zui_tooltip(tr("Dilate painted textures to prevent seams"));

				let dilate_handle: zui_handle_t = zui_handle("boxpreferences_18", { position: config_raw.dilate });
				zui_combo(dilate_handle, [tr("Instant"), tr("Delayed")], tr("Dilate"), true);
				if (dilate_handle.changed) {
					config_raw.dilate = dilate_handle.position;
				}
				///end

				///if is_lab
				let workspace_handle: zui_handle_t = zui_handle("boxpreferences_19", { position: config_raw.workspace });
				zui_combo(workspace_handle, [tr("3D View"), tr("2D View")], tr("Default Workspace"), true);
				if (workspace_handle.changed) {
					config_raw.workspace = workspace_handle.position;
				}
				///end

				let camera_controls_handle: zui_handle_t = zui_handle("boxpreferences_20", { position: config_raw.camera_controls });
				zui_combo(camera_controls_handle, [tr("Orbit"), tr("Rotate"), tr("Fly")], tr("Default Camera Controls"), true);
				if (camera_controls_handle.changed) {
					config_raw.camera_controls = camera_controls_handle.position;
				}

				let layer_res_handle: zui_handle_t = zui_handle("boxpreferences_21", { position: config_raw.layer_res });

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

				let server_handle: zui_handle_t = zui_handle("boxpreferences_22", { text: config_raw.server });
				config_raw.server = zui_text_input(server_handle, tr("Cloud Server"));

				///if (is_paint || is_sculpt)
				let material_live_handle: zui_handle_t = zui_handle("boxpreferences_23", {selected: config_raw.material_live });
				config_raw.material_live = zui_check(material_live_handle, tr("Live Material Preview"));
				if (ui.is_hovered) zui_tooltip(tr("Instantly update material preview on node change"));

				let brush_live_handle: zui_handle_t = zui_handle("boxpreferences_24", { selected: config_raw.brush_live });
				config_raw.brush_live = zui_check(brush_live_handle, tr("Live Brush Preview"));
				if (ui.is_hovered) zui_tooltip(tr("Draw live brush preview in viewport"));
				if (brush_live_handle.changed) context_raw.ddirty = 2;

				let brush_3d_handle: zui_handle_t = zui_handle("boxpreferences_25", { selected: config_raw.brush_3d });
				config_raw.brush_3d = zui_check(brush_3d_handle, tr("3D Cursor"));
				if (brush_3d_handle.changed) MakeMaterial.parse_paint_material();

				ui.enabled = config_raw.brush_3d;
				let brush_depth_reject_handle: zui_handle_t = zui_handle("boxpreferences_26", { selected: config_raw.brush_depth_reject });
				config_raw.brush_depth_reject = zui_check(brush_depth_reject_handle, tr("Depth Reject"));
				if (brush_depth_reject_handle.changed) MakeMaterial.parse_paint_material();

				zui_row([0.5, 0.5]);

				let brush_angle_reject_handle: zui_handle_t = zui_handle("boxpreferences_27", { selected: config_raw.brush_angle_reject });
				config_raw.brush_angle_reject = zui_check(brush_angle_reject_handle, tr("Angle Reject"));
				if (brush_angle_reject_handle.changed) MakeMaterial.parse_paint_material();

				if (!config_raw.brush_angle_reject) ui.enabled = false;
				let angle_dot_handle: zui_handle_t = zui_handle("boxpreferences_28", { value: context_raw.brush_angle_reject_dot });
				context_raw.brush_angle_reject_dot = zui_slider(angle_dot_handle, tr("Angle"), 0.0, 1.0, true);
				if (angle_dot_handle.changed) {
					MakeMaterial.parse_paint_material();
				}
				ui.enabled = true;
				///end

				///if is_lab
				config_raw.gpu_inference = zui_check(zui_handle("boxpreferences_29", { selected: config_raw.gpu_inference }), tr("Use GPU"));
				if (ui.is_hovered) zui_tooltip(tr("Use GPU to accelerate node graph processing"));
				///end
			}

			let pen_name: string;
			///if krom_ios
			pen_name = tr("Pencil");
			///else
			pen_name = tr("Pen");
			///end

			if (zui_tab(BoxPreferences.htab, pen_name, true)) {
				zui_text(tr("Pressure controls"));
				config_raw.pressure_radius = zui_check(zui_handle("boxpreferences_30", { selected: config_raw.pressure_radius }), tr("Brush Radius"));
				config_raw.pressure_sensitivity = zui_slider(zui_handle("boxpreferences_31", { value: config_raw.pressure_sensitivity }), tr("Sensitivity"), 0.0, 10.0, true);
				///if (is_paint || is_sculpt)
				config_raw.pressure_hardness = zui_check(zui_handle("boxpreferences_32", { selected: config_raw.pressure_hardness }), tr("Brush Hardness"));
				config_raw.pressure_opacity = zui_check(zui_handle("boxpreferences_33", { selected: config_raw.pressure_opacity }), tr("Brush Opacity"));
				config_raw.pressure_angle = zui_check(zui_handle("boxpreferences_34", { selected: config_raw.pressure_angle }), tr("Brush Angle"));
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

			context_raw.hssao = zui_handle("boxpreferences_35", { selected: config_raw.rp_ssao });
			context_raw.hssr = zui_handle("boxpreferences_36", { selected: config_raw.rp_ssr });
			context_raw.hbloom = zui_handle("boxpreferences_37", { selected: config_raw.rp_bloom });
			context_raw.hsupersample = zui_handle("boxpreferences_38", { position: config_get_super_sample_quality(config_raw.rp_supersample) });
			context_raw.hvxao = zui_handle("boxpreferences_39", { selected: config_raw.rp_gi });
			if (zui_tab(BoxPreferences.htab, tr("Viewport"), true)) {
				///if (krom_direct3d12 || krom_vulkan || krom_metal)

				let hpathtrace_mode: zui_handle_t = zui_handle("boxpreferences_40", { position: context_raw.pathtrace_mode });
				context_raw.pathtrace_mode = zui_combo(hpathtrace_mode, [tr("Core"), tr("Full")], tr("Path Tracer"), true);
				if (hpathtrace_mode.changed) {
					RenderPathRaytrace.ready = false;
				}

				///end

				let hrender_mode: zui_handle_t = zui_handle("boxpreferences_41", { position: context_raw.render_mode });
				context_raw.render_mode = zui_combo(hrender_mode, [tr("Full"), tr("Mobile")], tr("Renderer"), true);
				if (hrender_mode.changed) {
					context_set_render_path();
				}

				zui_combo(context_raw.hsupersample, ["0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x"], tr("Super Sample"), true);
				if (context_raw.hsupersample.changed) config_apply();

				if (context_raw.render_mode == render_mode_t.DEFERRED) {
					///if arm_voxels
					zui_check(context_raw.hvxao, tr("Voxel AO"));
					if (ui.is_hovered) zui_tooltip(tr("Cone-traced AO and shadows"));
					if (context_raw.hvxao.changed) {
						config_apply();
					}

					ui.enabled = context_raw.hvxao.selected;
					let h: zui_handle_t = zui_handle("boxpreferences_42", { value: context_raw.vxao_offset });
					context_raw.vxao_offset = zui_slider(h, tr("Cone Offset"), 1.0, 4.0, true);
					if (h.changed) context_raw.ddirty = 2;
					h = zui_handle("boxpreferences_43", { value: context_raw.vxao_aperture });
					context_raw.vxao_aperture = zui_slider(h, tr("Aperture"), 1.0, 4.0, true);
					if (h.changed) context_raw.ddirty = 2;
					ui.enabled = true;
					///end

					zui_check(context_raw.hssao, tr("SSAO"));
					if (context_raw.hssao.changed) config_apply();
					zui_check(context_raw.hssr, tr("SSR"));
					if (context_raw.hssr.changed) config_apply();
					zui_check(context_raw.hbloom, tr("Bloom"));
					if (context_raw.hbloom.changed) config_apply();
				}

				let h: zui_handle_t = zui_handle("boxpreferences_44", { value: config_raw.rp_vignette });
				config_raw.rp_vignette = zui_slider(h, tr("Vignette"), 0.0, 1.0, true);
				if (h.changed) context_raw.ddirty = 2;

				h = zui_handle("boxpreferences_45", { value: config_raw.rp_grain });
				config_raw.rp_grain = zui_slider(h, tr("Noise Grain"), 0.0, 1.0, true);
				if (h.changed) context_raw.ddirty = 2;

				// let h: zui_handle_t = Zui.handle("boxpreferences_46", { value: raw.autoExposureStrength });
				// raw.autoExposureStrength = Zui.slider(h, "Auto Exposure", 0.0, 2.0, true);
				// if (h.changed) raw.ddirty = 2;

				let cam: camera_object_t = scene_camera;
				let cam_raw: camera_data_t = cam.data;
				let near_handle: zui_handle_t = zui_handle("boxpreferences_47");
				let far_handle: zui_handle_t = zui_handle("boxpreferences_48");
				near_handle.value = Math.floor(cam_raw.near_plane * 1000) / 1000;
				far_handle.value = Math.floor(cam_raw.far_plane * 100) / 100;
				cam_raw.near_plane = zui_slider(near_handle, tr("Clip Start"), 0.001, 1.0, true);
				cam_raw.far_plane = zui_slider(far_handle, tr("Clip End"), 50.0, 100.0, true);
				if (near_handle.changed || far_handle.changed) {
					camera_object_build_proj(cam);
				}

				let disp_handle: zui_handle_t = zui_handle("boxpreferences_49", { value: config_raw.displace_strength });
				config_raw.displace_strength = zui_slider(disp_handle, tr("Displacement Strength"), 0.0, 10.0, true);
				if (disp_handle.changed) {
					context_raw.ddirty = 2;
					MakeMaterial.parse_mesh_material();
				}
			}
			if (zui_tab(BoxPreferences.htab, tr("Keymap"), true)) {

				if (BoxPreferences.files_keymap == null) {
					BoxPreferences.fetch_keymaps();
				}

				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

				BoxPreferences.preset_handle = zui_handle("boxpreferences_50", { position: BoxPreferences.get_preset_index() });
				zui_combo(BoxPreferences.preset_handle, BoxPreferences.files_keymap, tr("Preset"));
				if (BoxPreferences.preset_handle.changed) {
					config_raw.keymap = BoxPreferences.files_keymap[BoxPreferences.preset_handle.position] + ".json";
					config_apply();
					config_load_keymap();
				}

				if (zui_button(tr("New"))) {
					UIBox.show_custom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_51"), tr("New Keymap"))) {
							zui_row([0.5, 0.5]);
							let keymap_name: string = zui_text_input(zui_handle("boxpreferences_52", { text: "new_keymap" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template: string = JSON.stringify(base_default_keymap);
								if (!keymap_name.endsWith(".json")) keymap_name += ".json";
								let path: string = path_data() + path_sep + "keymap_presets" + path_sep + keymap_name;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.fetch_keymaps(); // Refresh file list
								config_raw.keymap = keymap_name;
								BoxPreferences.preset_handle.position = BoxPreferences.get_preset_index();
								UIBox.hide();
								BoxPreferences.htab.position = 5; // Keymap
								BoxPreferences.show();
							}
						}
					});
				}

				if (zui_button(tr("Import"))) {
					UIFiles.show("json", false, false, (path: string) => {
						ImportKeymap.run(path);
					});
				}
				if (zui_button(tr("Export"))) {
					UIFiles.show("json", true, false, (dest: string) => {
						if (!UIFiles.filename.endsWith(".json")) UIFiles.filename += ".json";
						let path: string = path_data() + path_sep + "keymap_presets" + path_sep + config_raw.keymap;
						file_copy(path, dest + path_sep + UIFiles.filename);
					});
				}

				zui_end_sticky();

				zui_separator(8, false);

				let i: i32 = 0;
				ui.changed = false;
				for (let key in config_keymap) {
					let h: zui_handle_t = zui_nest(zui_handle("boxpreferences_53"), i++);
					h.text = config_keymap[key];
					let text: string = zui_text_input(h, key, zui_align_t.LEFT);
					config_keymap[key] = text;
				}
				if (ui.changed) {
					config_apply();
					config_save_keymap();
				}
			}
			if (zui_tab(BoxPreferences.htab, tr("Plugins"), true)) {
				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4]);
				if (zui_button(tr("New"))) {
					UIBox.show_custom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_54"), tr("New Plugin"))) {
							zui_row([0.5, 0.5]);
							let plugin_name: string = zui_text_input(zui_handle("boxpreferences_55", { text: "new_plugin" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template: string =
`let plugin = create();
let h1 = new Handle();
plugin.drawUI = (ui) { =>
	if (Zui.panel(h1, 'New Plugin')) {
		if (Zui.button('Button')) {
			console.error('Hello');
		}
	}
}
`;
								if (!plugin_name.endsWith(".js")) plugin_name += ".js";
								let path: string = path_data() + path_sep + "plugins" + path_sep + plugin_name;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.files_plugin = null; // Refresh file list
								UIBox.hide();
								BoxPreferences.htab.position = 6; // Plugins
								BoxPreferences.show();
							}
						}
					});
				}
				if (zui_button(tr("Import"))) {
					UIFiles.show("js,zip", false, false, (path: string) => {
						ImportPlugin.run(path);
					});
				}
				zui_end_sticky();

				if (BoxPreferences.files_plugin == null) {
					BoxPreferences.fetch_plugins();
				}

				if (config_raw.plugins == null) config_raw.plugins = [];
				let h: zui_handle_t = zui_handle("boxpreferences_56", { selected: false });
				for (let f of BoxPreferences.files_plugin) {
					let is_js: bool = f.endsWith(".js");
					if (!is_js) continue;
					let enabled: bool = config_raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					let tag: string = is_js ? f.split(".")[0] : f;
					zui_check(h, tag);
					if (h.changed && h.selected != enabled) {
						h.selected ? config_enable_plugin(f) : config_disable_plugin(f);
						base_redraw_ui();
					}
					if (ui.is_hovered && ui.input_released_r) {
						UIMenu.draw((ui: zui_t) => {
							let path: string = path_data() + path_sep + "plugins" + path_sep + f;
							if (UIMenu.menu_button(ui, tr("Edit in Text Editor"))) {
								file_start(path);
							}
							if (UIMenu.menu_button(ui, tr("Edit in Script Tab"))) {
								let blob: ArrayBuffer = data_get_blob("plugins/" + f);
								TabScript.hscript.text = sys_buffer_to_string(blob);
								data_delete_blob("plugins/" + f);
								console_info(tr("Script opened"));
							}
							if (UIMenu.menu_button(ui, tr("Export"))) {
								UIFiles.show("js", true, false, (dest: string) => {
									if (!UIFiles.filename.endsWith(".js")) UIFiles.filename += ".js";
									file_copy(path, dest + path_sep + UIFiles.filename);
								});
							}
							if (UIMenu.menu_button(ui, tr("Delete"))) {
								if (config_raw.plugins.indexOf(f) >= 0) {
									array_remove(config_raw.plugins, f);
									plugin_stop(f);
								}
								array_remove(BoxPreferences.files_plugin, f);
								file_delete(path);
							}
						}, 4);
					}
				}
			}

		}, 620, config_raw.touch_ui ? 480 : 420, () => { config_save(); });
	}

	static fetch_themes = () => {
		BoxPreferences.themes = file_read_directory(path_data() + path_sep + "themes");
		for (let i: i32 = 0; i < BoxPreferences.themes.length; ++i) BoxPreferences.themes[i] = BoxPreferences.themes[i].substr(0, BoxPreferences.themes[i].length - 5); // Strip .json
		BoxPreferences.themes.unshift("default");
	}

	static fetch_keymaps = () => {
		BoxPreferences.files_keymap = file_read_directory(path_data() + path_sep + "keymap_presets");
		for (let i: i32 = 0; i < BoxPreferences.files_keymap.length; ++i) {
			BoxPreferences.files_keymap[i] = BoxPreferences.files_keymap[i].substr(0, BoxPreferences.files_keymap[i].length - 5); // Strip .json
		}
		BoxPreferences.files_keymap.unshift("default");
	}

	static fetch_plugins = () => {
		BoxPreferences.files_plugin = file_read_directory(path_data() + path_sep + "plugins");
	}

	static get_theme_index = (): i32 => {
		return BoxPreferences.themes.indexOf(config_raw.theme.substr(0, config_raw.theme.length - 5)); // Strip .json
	}

	static get_preset_index = (): i32 => {
		return BoxPreferences.files_keymap.indexOf(config_raw.keymap.substr(0, config_raw.keymap.length - 5)); // Strip .json
	}

	static set_scale = () => {
		let scale: f32 = config_raw.window_scale;
		zui_set_scale(UIBase.ui, scale);
		UIHeader.headerh = Math.floor(UIHeader.default_header_h * scale);
		config_raw.layout[layout_size_t.STATUS_H] = Math.floor(UIStatus.default_status_h * scale);
		UIMenubar.menubarw = Math.floor(UIMenubar.default_menubar_w * scale);
		UIBase.set_icon_scale();
		zui_set_scale(UINodes.ui, scale);
		zui_set_scale(UIView2D.ui, scale);
		zui_set_scale(base_ui_box, scale);
		zui_set_scale(base_ui_menu, scale);
		base_resize();
		///if (is_paint || is_sculpt)
		config_raw.layout[layout_size_t.SIDEBAR_W] = Math.floor(UIBase.default_sidebar_w * scale);
		UIToolbar.toolbar_w = Math.floor(UIToolbar.default_toolbar_w * scale);
		///end
	}
}
