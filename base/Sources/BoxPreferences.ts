
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
					BoxPreferences.locales = Translator.get_supported_locales();
				}

				let localeHandle: zui_handle_t = zui_handle("boxpreferences_0", { position: BoxPreferences.locales.indexOf(Config.raw.locale) });
				zui_combo(localeHandle, BoxPreferences.locales, tr("Language"), true);
				if (localeHandle.changed) {
					let localeCode: string = BoxPreferences.locales[localeHandle.position];
					Config.raw.locale = localeCode;
					Translator.load_translations(localeCode);
					UIBase.tag_ui_redraw();
				}

				let hscale: zui_handle_t = zui_handle("boxpreferences_1", { value: Config.raw.window_scale });
				zui_slider(hscale, tr("UI Scale"), 1.0, 4.0, true, 10);
				if (Context.raw.hscale_was_changed && !ui.input_down) {
					Context.raw.hscale_was_changed = false;
					if (hscale.value == null || isNaN(hscale.value)) hscale.value = 1.0;
					Config.raw.window_scale = hscale.value;
					BoxPreferences.set_scale();
				}
				if (hscale.changed) Context.raw.hscale_was_changed = true;

				let hspeed: zui_handle_t = zui_handle("boxpreferences_2", { value: Config.raw.camera_zoom_speed });
				Config.raw.camera_zoom_speed = zui_slider(hspeed, tr("Camera Zoom Speed"), 0.1, 4.0, true);

				hspeed = zui_handle("boxpreferences_3", { value: Config.raw.camera_rotation_speed });
				Config.raw.camera_rotation_speed = zui_slider(hspeed, tr("Camera Rotation Speed"), 0.1, 4.0, true);

				hspeed = zui_handle("boxpreferences_4", { value: Config.raw.camera_pan_speed });
				Config.raw.camera_pan_speed = zui_slider(hspeed, tr("Camera Pan Speed"), 0.1, 4.0, true);

				let zoomDirectionHandle: zui_handle_t = zui_handle("boxpreferences_5", { position: Config.raw.zoom_direction });
				zui_combo(zoomDirectionHandle, [tr("Vertical"), tr("Vertical Inverted"), tr("Horizontal"), tr("Horizontal Inverted"), tr("Vertical and Horizontal"), tr("Vertical and Horizontal Inverted")], tr("Direction to Zoom"), true);
				if (zoomDirectionHandle.changed) {
					Config.raw.zoom_direction = zoomDirectionHandle.position;
				}

				Config.raw.wrap_mouse = zui_check(zui_handle("boxpreferences_6", { selected: Config.raw.wrap_mouse }), tr("Wrap Mouse"));
				if (ui.is_hovered) zui_tooltip(tr("Wrap mouse around view boundaries during camera control"));

				Config.raw.node_preview = zui_check(zui_handle("boxpreferences_7", { selected: Config.raw.node_preview }), tr("Show Node Preview"));

				ui.changed = false;
				Config.raw.show_asset_names = zui_check(zui_handle("boxpreferences_8", { selected: Config.raw.show_asset_names }), tr("Show Asset Names"));
				if (ui.changed) {
					UIBase.tag_ui_redraw();
				}

				///if !(krom_android || krom_ios)
				ui.changed = false;
				Config.raw.touch_ui = zui_check(zui_handle("boxpreferences_9", { selected: Config.raw.touch_ui }), tr("Touch UI"));
				if (ui.changed) {
					zui_set_touch_scroll(Config.raw.touch_ui);
					zui_set_touch_hold(Config.raw.touch_ui);
					zui_set_touch_tooltip(Config.raw.touch_ui);
					Config.load_theme(Config.raw.theme);
					BoxPreferences.set_scale();
					UIBase.tag_ui_redraw();
				}
				///end

				Config.raw.splash_screen = zui_check(zui_handle("boxpreferences_10", { selected: Config.raw.splash_screen }), tr("Splash Screen"));

				// Zui.text("Node Editor");
				// let gridSnap: bool = Zui.check(Zui.handle("boxpreferences_11", { selected: false }), "Grid Snap");

				zui_end_element();
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Restore")) && !UIMenu.show) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Confirm"))) {
							app_notify_on_init(() => {
								ui.t.ELEMENT_H = Base.default_element_h;
								Config.restore();
								BoxPreferences.set_scale();
								if (BoxPreferences.files_plugin != null) for (let f of BoxPreferences.files_plugin) Plugin.stop(f);
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
									ui.t.ELEMENT_H = Base.default_element_h;
									Config.import_from(raw);
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
							Base.init_layout();
							Config.save();
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
					Config.raw.theme = BoxPreferences.themes[BoxPreferences.theme_handle.position] + ".json";
					Config.load_theme(Config.raw.theme);
				}

				if (zui_button(tr("New"))) {
					UIBox.show_custom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_13"), tr("New Theme"))) {
							zui_row([0.5, 0.5]);
							let themeName: string = zui_text_input(zui_handle("boxpreferences_14", { text: "new_theme" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template: string = JSON.stringify(Base.theme);
								if (!themeName.endsWith(".json")) themeName += ".json";
								let path: string = Path.data() + Path.sep + "themes" + Path.sep + themeName;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.fetch_themes(); // Refresh file list
								Config.raw.theme = themeName;
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
						path += Path.sep + UIFiles.filename;
						if (!path.endsWith(".json")) path += ".json";
						krom_file_save_bytes(path, sys_string_to_buffer(JSON.stringify(Base.theme)));
					});
				}

				zui_end_sticky();

				let i: i32 = 0;
				let theme: any = Base.theme;
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
					Context.raw.empty_envmap = image_from_bytes(b.buffer, 1, 1);
					Context.raw.ddirty = 2;
					if (!Context.raw.show_envmap) {
						scene_world._.envmap = Context.raw.empty_envmap;
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
						for (let ui of Base.get_uis()) {
							ui.elements_baked = false;
						}
					}
				}
			}

			if (zui_tab(BoxPreferences.htab, tr("Usage"), true)) {
				Context.raw.undo_handle = zui_handle("boxpreferences_16", { value: Config.raw.undo_steps });
				Config.raw.undo_steps = Math.floor(zui_slider(Context.raw.undo_handle, tr("Undo Steps"), 1, 64, false, 1));
				if (Config.raw.undo_steps < 1) {
					Config.raw.undo_steps = Math.floor(Context.raw.undo_handle.value = 1);
				}
				if (Context.raw.undo_handle.changed) {
					let current: image_t = _g2_current;
					g2_end();

					///if (is_paint || is_sculpt)
					while (History.undo_layers.length < Config.raw.undo_steps) {
						let l: SlotLayerRaw = SlotLayer.create("_undo" + History.undo_layers.length);
						History.undo_layers.push(l);
					}
					while (History.undo_layers.length > Config.raw.undo_steps) {
						let l: SlotLayerRaw = History.undo_layers.pop();
						SlotLayer.unload(l);
					}
					///end

					History.reset();
					g2_begin(current);
				}

				///if is_paint
				Config.raw.dilate_radius = Math.floor(zui_slider(zui_handle("boxpreferences_17", { value: Config.raw.dilate_radius }), tr("Dilate Radius"), 0.0, 16.0, true, 1));
				if (ui.is_hovered) zui_tooltip(tr("Dilate painted textures to prevent seams"));

				let dilateHandle: zui_handle_t = zui_handle("boxpreferences_18", { position: Config.raw.dilate });
				zui_combo(dilateHandle, [tr("Instant"), tr("Delayed")], tr("Dilate"), true);
				if (dilateHandle.changed) {
					Config.raw.dilate = dilateHandle.position;
				}
				///end

				///if is_lab
				let workspaceHandle: zui_handle_t = zui_handle("boxpreferences_19", { position: Config.raw.workspace });
				zui_combo(workspaceHandle, [tr("3D View"), tr("2D View")], tr("Default Workspace"), true);
				if (workspaceHandle.changed) {
					Config.raw.workspace = workspaceHandle.position;
				}
				///end

				let cameraControlsHandle: zui_handle_t = zui_handle("boxpreferences_20", { position: Config.raw.camera_controls });
				zui_combo(cameraControlsHandle, [tr("Orbit"), tr("Rotate"), tr("Fly")], tr("Default Camera Controls"), true);
				if (cameraControlsHandle.changed) {
					Config.raw.camera_controls = cameraControlsHandle.position;
				}

				let layerResHandle: zui_handle_t = zui_handle("boxpreferences_21", { position: Config.raw.layer_res });

				///if is_paint
				///if (krom_android || krom_ios)
				zui_combo(layerResHandle, ["128", "256", "512", "1K", "2K", "4K"], tr("Default Layer Resolution"), true);
				///else
				zui_combo(layerResHandle, ["128", "256", "512", "1K", "2K", "4K", "8K"], tr("Default Layer Resolution"), true);
				///end
				///end

				///if is_lab
				///if (krom_android || krom_ios)
				zui_combo(layerResHandle, ["2K", "4K"], tr("Default Layer Resolution"), true);
				///else
				zui_combo(layerResHandle, ["2K", "4K", "8K", "16K"], tr("Default Layer Resolution"), true);
				///end
				///end

				if (layerResHandle.changed) {
					Config.raw.layer_res = layerResHandle.position;
				}

				let serverHandle: zui_handle_t = zui_handle("boxpreferences_22", { text: Config.raw.server });
				Config.raw.server = zui_text_input(serverHandle, tr("Cloud Server"));

				///if (is_paint || is_sculpt)
				let materialLiveHandle: zui_handle_t = zui_handle("boxpreferences_23", {selected: Config.raw.material_live });
				Config.raw.material_live = zui_check(materialLiveHandle, tr("Live Material Preview"));
				if (ui.is_hovered) zui_tooltip(tr("Instantly update material preview on node change"));

				let brushLiveHandle: zui_handle_t = zui_handle("boxpreferences_24", { selected: Config.raw.brush_live });
				Config.raw.brush_live = zui_check(brushLiveHandle, tr("Live Brush Preview"));
				if (ui.is_hovered) zui_tooltip(tr("Draw live brush preview in viewport"));
				if (brushLiveHandle.changed) Context.raw.ddirty = 2;

				let brush3dHandle: zui_handle_t = zui_handle("boxpreferences_25", { selected: Config.raw.brush_3d });
				Config.raw.brush_3d = zui_check(brush3dHandle, tr("3D Cursor"));
				if (brush3dHandle.changed) MakeMaterial.parse_paint_material();

				ui.enabled = Config.raw.brush_3d;
				let brushDepthRejectHandle: zui_handle_t = zui_handle("boxpreferences_26", { selected: Config.raw.brush_depth_reject });
				Config.raw.brush_depth_reject = zui_check(brushDepthRejectHandle, tr("Depth Reject"));
				if (brushDepthRejectHandle.changed) MakeMaterial.parse_paint_material();

				zui_row([0.5, 0.5]);

				let brushAngleRejectHandle: zui_handle_t = zui_handle("boxpreferences_27", { selected: Config.raw.brush_angle_reject });
				Config.raw.brush_angle_reject = zui_check(brushAngleRejectHandle, tr("Angle Reject"));
				if (brushAngleRejectHandle.changed) MakeMaterial.parse_paint_material();

				if (!Config.raw.brush_angle_reject) ui.enabled = false;
				let angleDotHandle: zui_handle_t = zui_handle("boxpreferences_28", { value: Context.raw.brush_angle_reject_dot });
				Context.raw.brush_angle_reject_dot = zui_slider(angleDotHandle, tr("Angle"), 0.0, 1.0, true);
				if (angleDotHandle.changed) {
					MakeMaterial.parse_paint_material();
				}
				ui.enabled = true;
				///end

				///if is_lab
				Config.raw.gpu_inference = zui_check(zui_handle("boxpreferences_29", { selected: Config.raw.gpu_inference }), tr("Use GPU"));
				if (ui.is_hovered) zui_tooltip(tr("Use GPU to accelerate node graph processing"));
				///end
			}

			let penName: string;
			///if krom_ios
			penName = tr("Pencil");
			///else
			penName = tr("Pen");
			///end

			if (zui_tab(BoxPreferences.htab, penName, true)) {
				zui_text(tr("Pressure controls"));
				Config.raw.pressure_radius = zui_check(zui_handle("boxpreferences_30", { selected: Config.raw.pressure_radius }), tr("Brush Radius"));
				Config.raw.pressure_sensitivity = zui_slider(zui_handle("boxpreferences_31", { value: Config.raw.pressure_sensitivity }), tr("Sensitivity"), 0.0, 10.0, true);
				///if (is_paint || is_sculpt)
				Config.raw.pressure_hardness = zui_check(zui_handle("boxpreferences_32", { selected: Config.raw.pressure_hardness }), tr("Brush Hardness"));
				Config.raw.pressure_opacity = zui_check(zui_handle("boxpreferences_33", { selected: Config.raw.pressure_opacity }), tr("Brush Opacity"));
				Config.raw.pressure_angle = zui_check(zui_handle("boxpreferences_34", { selected: Config.raw.pressure_angle }), tr("Brush Angle"));
				///end

				zui_end_element();
				zui_row([0.5]);
				if (zui_button(tr("Help"))) {
					///if (is_paint || is_sculpt)
					File.load_url("https://github.com/armory3d/armorpaint_docs///pen");
					///end
					///if is_lab
					File.load_url("https://github.com/armory3d/armorlab_docs///pen");
					///end
				}
			}

			Context.raw.hssao = zui_handle("boxpreferences_35", { selected: Config.raw.rp_ssao });
			Context.raw.hssr = zui_handle("boxpreferences_36", { selected: Config.raw.rp_ssr });
			Context.raw.hbloom = zui_handle("boxpreferences_37", { selected: Config.raw.rp_bloom });
			Context.raw.hsupersample = zui_handle("boxpreferences_38", { position: Config.get_super_sample_quality(Config.raw.rp_supersample) });
			Context.raw.hvxao = zui_handle("boxpreferences_39", { selected: Config.raw.rp_gi });
			if (zui_tab(BoxPreferences.htab, tr("Viewport"), true)) {
				///if (krom_direct3d12 || krom_vulkan || krom_metal)

				let hpathtracemode: zui_handle_t = zui_handle("boxpreferences_40", { position: Context.raw.pathtrace_mode });
				Context.raw.pathtrace_mode = zui_combo(hpathtracemode, [tr("Core"), tr("Full")], tr("Path Tracer"), true);
				if (hpathtracemode.changed) {
					RenderPathRaytrace.ready = false;
				}

				///end

				let hrendermode: zui_handle_t = zui_handle("boxpreferences_41", { position: Context.raw.render_mode });
				Context.raw.render_mode = zui_combo(hrendermode, [tr("Full"), tr("Mobile")], tr("Renderer"), true);
				if (hrendermode.changed) {
					Context.set_render_path();
				}

				zui_combo(Context.raw.hsupersample, ["0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x"], tr("Super Sample"), true);
				if (Context.raw.hsupersample.changed) Config.apply_config();

				if (Context.raw.render_mode == render_mode_t.DEFERRED) {
					///if arm_voxels
					zui_check(Context.raw.hvxao, tr("Voxel AO"));
					if (ui.is_hovered) zui_tooltip(tr("Cone-traced AO and shadows"));
					if (Context.raw.hvxao.changed) {
						Config.apply_config();
					}

					ui.enabled = Context.raw.hvxao.selected;
					let h: zui_handle_t = zui_handle("boxpreferences_42", { value: Context.raw.vxao_offset });
					Context.raw.vxao_offset = zui_slider(h, tr("Cone Offset"), 1.0, 4.0, true);
					if (h.changed) Context.raw.ddirty = 2;
					h = zui_handle("boxpreferences_43", { value: Context.raw.vxao_aperture });
					Context.raw.vxao_aperture = zui_slider(h, tr("Aperture"), 1.0, 4.0, true);
					if (h.changed) Context.raw.ddirty = 2;
					ui.enabled = true;
					///end

					zui_check(Context.raw.hssao, tr("SSAO"));
					if (Context.raw.hssao.changed) Config.apply_config();
					zui_check(Context.raw.hssr, tr("SSR"));
					if (Context.raw.hssr.changed) Config.apply_config();
					zui_check(Context.raw.hbloom, tr("Bloom"));
					if (Context.raw.hbloom.changed) Config.apply_config();
				}

				let h: zui_handle_t = zui_handle("boxpreferences_44", { value: Config.raw.rp_vignette });
				Config.raw.rp_vignette = zui_slider(h, tr("Vignette"), 0.0, 1.0, true);
				if (h.changed) Context.raw.ddirty = 2;

				h = zui_handle("boxpreferences_45", { value: Config.raw.rp_grain });
				Config.raw.rp_grain = zui_slider(h, tr("Noise Grain"), 0.0, 1.0, true);
				if (h.changed) Context.raw.ddirty = 2;

				// let h: zui_handle_t = Zui.handle("boxpreferences_46", { value: Context.raw.autoExposureStrength });
				// Context.raw.autoExposureStrength = Zui.slider(h, "Auto Exposure", 0.0, 2.0, true);
				// if (h.changed) Context.raw.ddirty = 2;

				let cam: camera_object_t = scene_camera;
				let camRaw: camera_data_t = cam.data;
				let near_handle: zui_handle_t = zui_handle("boxpreferences_47");
				let far_handle: zui_handle_t = zui_handle("boxpreferences_48");
				near_handle.value = Math.floor(camRaw.near_plane * 1000) / 1000;
				far_handle.value = Math.floor(camRaw.far_plane * 100) / 100;
				camRaw.near_plane = zui_slider(near_handle, tr("Clip Start"), 0.001, 1.0, true);
				camRaw.far_plane = zui_slider(far_handle, tr("Clip End"), 50.0, 100.0, true);
				if (near_handle.changed || far_handle.changed) {
					camera_object_build_proj(cam);
				}

				let dispHandle: zui_handle_t = zui_handle("boxpreferences_49", { value: Config.raw.displace_strength });
				Config.raw.displace_strength = zui_slider(dispHandle, tr("Displacement Strength"), 0.0, 10.0, true);
				if (dispHandle.changed) {
					Context.raw.ddirty = 2;
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
					Config.raw.keymap = BoxPreferences.files_keymap[BoxPreferences.preset_handle.position] + ".json";
					Config.apply_config();
					Config.load_keymap();
				}

				if (zui_button(tr("New"))) {
					UIBox.show_custom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_51"), tr("New Keymap"))) {
							zui_row([0.5, 0.5]);
							let keymapName: string = zui_text_input(zui_handle("boxpreferences_52", { text: "new_keymap" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template: string = JSON.stringify(Base.default_keymap);
								if (!keymapName.endsWith(".json")) keymapName += ".json";
								let path: string = Path.data() + Path.sep + "keymap_presets" + Path.sep + keymapName;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.fetch_keymaps(); // Refresh file list
								Config.raw.keymap = keymapName;
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
						let path: string = Path.data() + Path.sep + "keymap_presets" + Path.sep + Config.raw.keymap;
						File.copy(path, dest + Path.sep + UIFiles.filename);
					});
				}

				zui_end_sticky();

				zui_separator(8, false);

				let i: i32 = 0;
				ui.changed = false;
				for (let key in Config.keymap) {
					let h: zui_handle_t = zui_nest(zui_handle("boxpreferences_53"), i++);
					h.text = Config.keymap[key];
					let text: string = zui_text_input(h, key, zui_align_t.LEFT);
					Config.keymap[key] = text;
				}
				if (ui.changed) {
					Config.apply_config();
					Config.save_keymap();
				}
			}
			if (zui_tab(BoxPreferences.htab, tr("Plugins"), true)) {
				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4]);
				if (zui_button(tr("New"))) {
					UIBox.show_custom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_54"), tr("New Plugin"))) {
							zui_row([0.5, 0.5]);
							let pluginName: string = zui_text_input(zui_handle("boxpreferences_55", { text: "new_plugin" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template: string =
`let plugin = Plugin.create();
let h1 = new Handle();
plugin.drawUI = (ui) { =>
	if (Zui.panel(h1, 'New Plugin')) {
		if (Zui.button('Button')) {
			console.error('Hello');
		}
	}
}
`;
								if (!pluginName.endsWith(".js")) pluginName += ".js";
								let path: string = Path.data() + Path.sep + "plugins" + Path.sep + pluginName;
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

				if (Config.raw.plugins == null) Config.raw.plugins = [];
				let h: zui_handle_t = zui_handle("boxpreferences_56", { selected: false });
				for (let f of BoxPreferences.files_plugin) {
					let isJs: bool = f.endsWith(".js");
					if (!isJs) continue;
					let enabled: bool = Config.raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					let tag: string = isJs ? f.split(".")[0] : f;
					zui_check(h, tag);
					if (h.changed && h.selected != enabled) {
						h.selected ? Config.enable_plugin(f) : Config.disable_plugin(f);
						Base.redraw_ui();
					}
					if (ui.is_hovered && ui.input_released_r) {
						UIMenu.draw((ui: zui_t) => {
							let path: string = Path.data() + Path.sep + "plugins" + Path.sep + f;
							if (UIMenu.menu_button(ui, tr("Edit in Text Editor"))) {
								File.start(path);
							}
							if (UIMenu.menu_button(ui, tr("Edit in Script Tab"))) {
								let blob: ArrayBuffer = data_get_blob("plugins/" + f);
								TabScript.hscript.text = sys_buffer_to_string(blob);
								data_delete_blob("plugins/" + f);
								Console.info(tr("Script opened"));
							}
							if (UIMenu.menu_button(ui, tr("Export"))) {
								UIFiles.show("js", true, false, (dest: string) => {
									if (!UIFiles.filename.endsWith(".js")) UIFiles.filename += ".js";
									File.copy(path, dest + Path.sep + UIFiles.filename);
								});
							}
							if (UIMenu.menu_button(ui, tr("Delete"))) {
								if (Config.raw.plugins.indexOf(f) >= 0) {
									array_remove(Config.raw.plugins, f);
									Plugin.stop(f);
								}
								array_remove(BoxPreferences.files_plugin, f);
								File.delete(path);
							}
						}, 4);
					}
				}
			}

		}, 620, Config.raw.touch_ui ? 480 : 420, () => { Config.save(); });
	}

	static fetch_themes = () => {
		BoxPreferences.themes = File.read_directory(Path.data() + Path.sep + "themes");
		for (let i: i32 = 0; i < BoxPreferences.themes.length; ++i) BoxPreferences.themes[i] = BoxPreferences.themes[i].substr(0, BoxPreferences.themes[i].length - 5); // Strip .json
		BoxPreferences.themes.unshift("default");
	}

	static fetch_keymaps = () => {
		BoxPreferences.files_keymap = File.read_directory(Path.data() + Path.sep + "keymap_presets");
		for (let i: i32 = 0; i < BoxPreferences.files_keymap.length; ++i) {
			BoxPreferences.files_keymap[i] = BoxPreferences.files_keymap[i].substr(0, BoxPreferences.files_keymap[i].length - 5); // Strip .json
		}
		BoxPreferences.files_keymap.unshift("default");
	}

	static fetch_plugins = () => {
		BoxPreferences.files_plugin = File.read_directory(Path.data() + Path.sep + "plugins");
	}

	static get_theme_index = (): i32 => {
		return BoxPreferences.themes.indexOf(Config.raw.theme.substr(0, Config.raw.theme.length - 5)); // Strip .json
	}

	static get_preset_index = (): i32 => {
		return BoxPreferences.files_keymap.indexOf(Config.raw.keymap.substr(0, Config.raw.keymap.length - 5)); // Strip .json
	}

	static set_scale = () => {
		let scale: f32 = Config.raw.window_scale;
		zui_set_scale(UIBase.ui, scale);
		UIHeader.headerh = Math.floor(UIHeader.default_header_h * scale);
		Config.raw.layout[layout_size_t.STATUS_H] = Math.floor(UIStatus.default_status_h * scale);
		UIMenubar.menubarw = Math.floor(UIMenubar.default_menubar_w * scale);
		UIBase.set_icon_scale();
		zui_set_scale(UINodes.ui, scale);
		zui_set_scale(UIView2D.ui, scale);
		zui_set_scale(Base.ui_box, scale);
		zui_set_scale(Base.ui_menu, scale);
		Base.resize();
		///if (is_paint || is_sculpt)
		Config.raw.layout[layout_size_t.SIDEBAR_W] = Math.floor(UIBase.default_sidebar_w * scale);
		UIToolbar.toolbar_w = Math.floor(UIToolbar.default_toolbar_w * scale);
		///end
	}
}
