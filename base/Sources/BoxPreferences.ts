
class BoxPreferences {

	static htab = zui_handle_create();
	static filesPlugin: string[] = null;
	static filesKeymap: string[] = null;
	static themeHandle: zui_handle_t;
	static presetHandle: zui_handle_t;
	static locales: string[] = null;
	static themes: string[] = null;
	static worldColor = 0xff080808;

	static show = () => {

		UIBox.showCustom((ui: zui_t) => {
			if (zui_tab(BoxPreferences.htab, tr("Interface"), true)) {

				if (BoxPreferences.locales == null) {
					BoxPreferences.locales = Translator.getSupportedLocales();
				}

				let localeHandle = zui_handle("boxpreferences_0", { position: BoxPreferences.locales.indexOf(Config.raw.locale) });
				zui_combo(localeHandle, BoxPreferences.locales, tr("Language"), true);
				if (localeHandle.changed) {
					let localeCode = BoxPreferences.locales[localeHandle.position];
					Config.raw.locale = localeCode;
					Translator.loadTranslations(localeCode);
					UIBase.tagUIRedraw();
				}

				let hscale = zui_handle("boxpreferences_1", { value: Config.raw.window_scale });
				zui_slider(hscale, tr("UI Scale"), 1.0, 4.0, true, 10);
				if (Context.raw.hscaleWasChanged && !ui.input_down) {
					Context.raw.hscaleWasChanged = false;
					if (hscale.value == null || isNaN(hscale.value)) hscale.value = 1.0;
					Config.raw.window_scale = hscale.value;
					BoxPreferences.setScale();
				}
				if (hscale.changed) Context.raw.hscaleWasChanged = true;

				let hspeed = zui_handle("boxpreferences_2", { value: Config.raw.camera_zoom_speed });
				Config.raw.camera_zoom_speed = zui_slider(hspeed, tr("Camera Zoom Speed"), 0.1, 4.0, true);

				hspeed = zui_handle("boxpreferences_3", { value: Config.raw.camera_rotation_speed });
				Config.raw.camera_rotation_speed = zui_slider(hspeed, tr("Camera Rotation Speed"), 0.1, 4.0, true);

				hspeed = zui_handle("boxpreferences_4", { value: Config.raw.camera_pan_speed });
				Config.raw.camera_pan_speed = zui_slider(hspeed, tr("Camera Pan Speed"), 0.1, 4.0, true);

				let zoomDirectionHandle = zui_handle("boxpreferences_5", { position: Config.raw.zoom_direction });
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
					UIBase.tagUIRedraw();
				}

				///if !(krom_android || krom_ios)
				ui.changed = false;
				Config.raw.touch_ui = zui_check(zui_handle("boxpreferences_9", { selected: Config.raw.touch_ui }), tr("Touch UI"));
				if (ui.changed) {
					zui_set_touch_scroll(Config.raw.touch_ui);
					zui_set_touch_hold(Config.raw.touch_ui);
					zui_set_touch_tooltip(Config.raw.touch_ui);
					Config.loadTheme(Config.raw.theme);
					BoxPreferences.setScale();
					UIBase.tagUIRedraw();
				}
				///end

				Config.raw.splash_screen = zui_check(zui_handle("boxpreferences_10", { selected: Config.raw.splash_screen }), tr("Splash Screen"));

				// Zui.text("Node Editor");
				// let gridSnap = Zui.check(Zui.handle("boxpreferences_11", { selected: false }), "Grid Snap");

				zui_end_element();
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Restore")) && !UIMenu.show) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menuButton(ui, tr("Confirm"))) {
							app_notify_on_init(() => {
								ui.t.ELEMENT_H = Base.defaultElementH;
								Config.restore();
								BoxPreferences.setScale();
								if (BoxPreferences.filesPlugin != null) for (let f of BoxPreferences.filesPlugin) Plugin.stop(f);
								BoxPreferences.filesPlugin = null;
								BoxPreferences.filesKeymap = null;
								MakeMaterial.parseMeshMaterial();
								MakeMaterial.parsePaintMaterial();
							});
						}
						if (UIMenu.menuButton(ui, tr("Import..."))) {
							UIFiles.show("json", false, false, (path: string) => {
								data_get_blob(path, (b: ArrayBuffer) => {
									let raw = JSON.parse(sys_buffer_to_string(b));
									app_notify_on_init(() => {
										ui.t.ELEMENT_H = Base.defaultElementH;
										Config.importFrom(raw);
										BoxPreferences.setScale();
										MakeMaterial.parseMeshMaterial();
										MakeMaterial.parsePaintMaterial();
									});
								});
							});
						}
					}, 2);
				}
				if (zui_button(tr("Reset Layout")) && !UIMenu.show) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menuButton(ui, tr("Confirm"))) {
							Base.initLayout();
							Config.save();
						}
					}, 1);
				}
			}

			if (zui_tab(BoxPreferences.htab, tr("Theme"), true)) {

				if (BoxPreferences.themes == null) {
					BoxPreferences.fetchThemes();
				}
				BoxPreferences.themeHandle = zui_handle("boxpreferences_12", { position: BoxPreferences.getThemeIndex() });

				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

				zui_combo(BoxPreferences.themeHandle, BoxPreferences.themes, tr("Theme"));
				if (BoxPreferences.themeHandle.changed) {
					Config.raw.theme = BoxPreferences.themes[BoxPreferences.themeHandle.position] + ".json";
					Config.loadTheme(Config.raw.theme);
				}

				if (zui_button(tr("New"))) {
					UIBox.showCustom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_13"), tr("New Theme"))) {
							zui_row([0.5, 0.5]);
							let themeName = zui_text_input(zui_handle("boxpreferences_14", { text: "new_theme" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template = JSON.stringify(Base.theme);
								if (!themeName.endsWith(".json")) themeName += ".json";
								let path = Path.data() + Path.sep + "themes" + Path.sep + themeName;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.fetchThemes(); // Refresh file list
								Config.raw.theme = themeName;
								BoxPreferences.themeHandle.position = BoxPreferences.getThemeIndex();
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

				let i = 0;
				let theme: any = Base.theme;
				let hlist = zui_handle("boxpreferences_15");

				// Viewport color
				let h = zui_nest(hlist, i++, { color: BoxPreferences.worldColor });
				zui_row([1 / 8, 7 / 8]);
				zui_text("", 0, h.color);
				if (ui.is_hovered && ui.input_released) {
					UIMenu.draw((ui) => {
						ui.changed = false;
						zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true);
						if (ui.changed) UIMenu.keepOpen = true;
					}, 11);
				}
				let val = h.color;
				if (val < 0) val += 4294967296;
				h.text = val.toString(16);
				zui_text_input(h, "VIEWPORT_COL");
				h.color = parseInt(h.text, 16);

				if (BoxPreferences.worldColor != h.color) {
					BoxPreferences.worldColor = h.color;
					let b = new Uint8Array(4);
					b[0] = color_get_rb(BoxPreferences.worldColor);
					b[1] = color_get_gb(BoxPreferences.worldColor);
					b[2] = color_get_bb(BoxPreferences.worldColor);
					b[3] = 255;
					Context.raw.emptyEnvmap = image_from_bytes(b.buffer, 1, 1);
					Context.raw.ddirty = 2;
					if (!Context.raw.showEnvmap) {
						scene_world._envmap = Context.raw.emptyEnvmap;
					}
				}

				// Theme fields
				for (let key of Object.getOwnPropertyNames(theme_t.prototype)) {
					if (key == "constructor") continue;

					let h = zui_nest(hlist, i++);
					let val: any = theme[key];

					let isHex = key.endsWith("_COL");
					if (isHex && val < 0) val += 4294967296;

					if (isHex) {
						zui_row([1 / 8, 7 / 8]);
						zui_text("", 0, val);
						if (ui.is_hovered && ui.input_released) {
							h.color = theme[key];
							UIMenu.draw((ui) => {
								ui.changed = false;
								let color = zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true);
								theme[key] = color;
								if (ui.changed) UIMenu.keepOpen = true;
							}, 11);
						}
					}

					ui.changed = false;

					if (typeof val == "boolean") {
						h.selected = val;
						let b = zui_check(h, key);
						theme[key] = b;
					}
					else if (key == "LINK_STYLE") {
						let styles = [tr("Straight"), tr("Curved")];
						h.position = val;
						let i = zui_combo(h, styles, key, true);
						theme[key] = i;
					}
					else {
						h.text = isHex ? val.toString(16) : val.toString();
						let res = zui_text_input(h, key);
						if (isHex) theme[key] = parseInt(h.text, 16);
						else theme[key] = parseInt(h.text);
					}

					if (ui.changed) {
						for (let ui of Base.getUIs()) {
							ui.elements_baked = false;
						}
					}
				}
			}

			if (zui_tab(BoxPreferences.htab, tr("Usage"), true)) {
				Context.raw.undoHandle = zui_handle("boxpreferences_16", { value: Config.raw.undo_steps });
				Config.raw.undo_steps = Math.floor(zui_slider(Context.raw.undoHandle, tr("Undo Steps"), 1, 64, false, 1));
				if (Config.raw.undo_steps < 1) {
					Config.raw.undo_steps = Math.floor(Context.raw.undoHandle.value = 1);
				}
				if (Context.raw.undoHandle.changed) {
					let current = _g2_current;
					g2_end();

					///if (is_paint || is_sculpt)
					while (History.undoLayers.length < Config.raw.undo_steps) {
						let l = SlotLayer.create("_undo" + History.undoLayers.length);
						History.undoLayers.push(l);
					}
					while (History.undoLayers.length > Config.raw.undo_steps) {
						let l = History.undoLayers.pop();
						SlotLayer.unload(l);
					}
					///end

					History.reset();
					g2_begin(current, false);
				}

				///if is_paint
				Config.raw.dilate_radius = Math.floor(zui_slider(zui_handle("boxpreferences_17", { value: Config.raw.dilate_radius }), tr("Dilate Radius"), 0.0, 16.0, true, 1));
				if (ui.is_hovered) zui_tooltip(tr("Dilate painted textures to prevent seams"));

				let dilateHandle = zui_handle("boxpreferences_18", { position: Config.raw.dilate });
				zui_combo(dilateHandle, [tr("Instant"), tr("Delayed")], tr("Dilate"), true);
				if (dilateHandle.changed) {
					Config.raw.dilate = dilateHandle.position;
				}
				///end

				///if is_lab
				let workspaceHandle = zui_handle("boxpreferences_19", { position: Config.raw.workspace });
				zui_combo(workspaceHandle, [tr("3D View"), tr("2D View")], tr("Default Workspace"), true);
				if (workspaceHandle.changed) {
					Config.raw.workspace = workspaceHandle.position;
				}
				///end

				let cameraControlsHandle = zui_handle("boxpreferences_20", { position: Config.raw.camera_controls });
				zui_combo(cameraControlsHandle, [tr("Orbit"), tr("Rotate"), tr("Fly")], tr("Default Camera Controls"), true);
				if (cameraControlsHandle.changed) {
					Config.raw.camera_controls = cameraControlsHandle.position;
				}

				let layerResHandle = zui_handle("boxpreferences_21", { position: Config.raw.layer_res });

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

				let serverHandle = zui_handle("boxpreferences_22", { text: Config.raw.server });
				Config.raw.server = zui_text_input(serverHandle, tr("Cloud Server"));

				///if (is_paint || is_sculpt)
				let materialLiveHandle = zui_handle("boxpreferences_23", {selected: Config.raw.material_live });
				Config.raw.material_live = zui_check(materialLiveHandle, tr("Live Material Preview"));
				if (ui.is_hovered) zui_tooltip(tr("Instantly update material preview on node change"));

				let brushLiveHandle = zui_handle("boxpreferences_24", { selected: Config.raw.brush_live });
				Config.raw.brush_live = zui_check(brushLiveHandle, tr("Live Brush Preview"));
				if (ui.is_hovered) zui_tooltip(tr("Draw live brush preview in viewport"));
				if (brushLiveHandle.changed) Context.raw.ddirty = 2;

				let brush3dHandle = zui_handle("boxpreferences_25", { selected: Config.raw.brush_3d });
				Config.raw.brush_3d = zui_check(brush3dHandle, tr("3D Cursor"));
				if (brush3dHandle.changed) MakeMaterial.parsePaintMaterial();

				ui.enabled = Config.raw.brush_3d;
				let brushDepthRejectHandle = zui_handle("boxpreferences_26", { selected: Config.raw.brush_depth_reject });
				Config.raw.brush_depth_reject = zui_check(brushDepthRejectHandle, tr("Depth Reject"));
				if (brushDepthRejectHandle.changed) MakeMaterial.parsePaintMaterial();

				zui_row([0.5, 0.5]);

				let brushAngleRejectHandle = zui_handle("boxpreferences_27", { selected: Config.raw.brush_angle_reject });
				Config.raw.brush_angle_reject = zui_check(brushAngleRejectHandle, tr("Angle Reject"));
				if (brushAngleRejectHandle.changed) MakeMaterial.parsePaintMaterial();

				if (!Config.raw.brush_angle_reject) ui.enabled = false;
				let angleDotHandle = zui_handle("boxpreferences_28", { value: Context.raw.brushAngleRejectDot });
				Context.raw.brushAngleRejectDot = zui_slider(angleDotHandle, tr("Angle"), 0.0, 1.0, true);
				if (angleDotHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
				ui.enabled = true;
				///end

				///if is_lab
				Config.raw.gpu_inference = zui_check(zui_handle("boxpreferences_29", { selected: Config.raw.gpu_inference }), tr("Use GPU"));
				if (ui.is_hovered) zui_tooltip(tr("Use GPU to accelerate node graph processing"));
				///end
			}

			let penName;
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
					File.loadUrl("https://github.com/armory3d/armorpaint_docs///pen");
					///end
					///if is_lab
					File.loadUrl("https://github.com/armory3d/armorlab_docs///pen");
					///end
				}
			}

			Context.raw.hssao = zui_handle("boxpreferences_35", { selected: Config.raw.rp_ssao });
			Context.raw.hssr = zui_handle("boxpreferences_36", { selected: Config.raw.rp_ssr });
			Context.raw.hbloom = zui_handle("boxpreferences_37", { selected: Config.raw.rp_bloom });
			Context.raw.hsupersample = zui_handle("boxpreferences_38", { position: Config.getSuperSampleQuality(Config.raw.rp_supersample) });
			Context.raw.hvxao = zui_handle("boxpreferences_39", { selected: Config.raw.rp_gi });
			if (zui_tab(BoxPreferences.htab, tr("Viewport"), true)) {
				///if (krom_direct3d12 || krom_vulkan || krom_metal)

				let hpathtracemode = zui_handle("boxpreferences_40", { position: Context.raw.pathTraceMode });
				Context.raw.pathTraceMode = zui_combo(hpathtracemode, [tr("Core"), tr("Full")], tr("Path Tracer"), true);
				if (hpathtracemode.changed) {
					RenderPathRaytrace.ready = false;
				}

				///end

				let hrendermode = zui_handle("boxpreferences_41", { position: Context.raw.renderMode });
				Context.raw.renderMode = zui_combo(hrendermode, [tr("Full"), tr("Mobile")], tr("Renderer"), true);
				if (hrendermode.changed) {
					Context.setRenderPath();
				}

				zui_combo(Context.raw.hsupersample, ["0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x"], tr("Super Sample"), true);
				if (Context.raw.hsupersample.changed) Config.applyConfig();

				if (Context.raw.renderMode == RenderMode.RenderDeferred) {
					///if arm_voxels
					zui_check(Context.raw.hvxao, tr("Voxel AO"));
					if (ui.is_hovered) zui_tooltip(tr("Cone-traced AO and shadows"));
					if (Context.raw.hvxao.changed) {
						Config.applyConfig();
					}

					ui.enabled = Context.raw.hvxao.selected;
					let h = zui_handle("boxpreferences_42", { value: Context.raw.vxaoOffset });
					Context.raw.vxaoOffset = zui_slider(h, tr("Cone Offset"), 1.0, 4.0, true);
					if (h.changed) Context.raw.ddirty = 2;
					h = zui_handle("boxpreferences_43", { value: Context.raw.vxaoAperture });
					Context.raw.vxaoAperture = zui_slider(h, tr("Aperture"), 1.0, 4.0, true);
					if (h.changed) Context.raw.ddirty = 2;
					ui.enabled = true;
					///end

					zui_check(Context.raw.hssao, tr("SSAO"));
					if (Context.raw.hssao.changed) Config.applyConfig();
					zui_check(Context.raw.hssr, tr("SSR"));
					if (Context.raw.hssr.changed) Config.applyConfig();
					zui_check(Context.raw.hbloom, tr("Bloom"));
					if (Context.raw.hbloom.changed) Config.applyConfig();
				}

				let h = zui_handle("boxpreferences_44", { value: Config.raw.rp_vignette });
				Config.raw.rp_vignette = zui_slider(h, tr("Vignette"), 0.0, 1.0, true);
				if (h.changed) Context.raw.ddirty = 2;

				h = zui_handle("boxpreferences_45", { value: Config.raw.rp_grain });
				Config.raw.rp_grain = zui_slider(h, tr("Noise Grain"), 0.0, 1.0, true);
				if (h.changed) Context.raw.ddirty = 2;

				// let h = Zui.handle("boxpreferences_46", { value: Context.raw.autoExposureStrength });
				// Context.raw.autoExposureStrength = Zui.slider(h, "Auto Exposure", 0.0, 2.0, true);
				// if (h.changed) Context.raw.ddirty = 2;

				let cam = scene_camera;
				let camRaw = cam.data;
				let near_handle = zui_handle("boxpreferences_47");
				let far_handle = zui_handle("boxpreferences_48");
				near_handle.value = Math.floor(camRaw.near_plane * 1000) / 1000;
				far_handle.value = Math.floor(camRaw.far_plane * 100) / 100;
				camRaw.near_plane = zui_slider(near_handle, tr("Clip Start"), 0.001, 1.0, true);
				camRaw.far_plane = zui_slider(far_handle, tr("Clip End"), 50.0, 100.0, true);
				if (near_handle.changed || far_handle.changed) {
					camera_object_build_proj(cam);
				}

				let dispHandle = zui_handle("boxpreferences_49", { value: Config.raw.displace_strength });
				Config.raw.displace_strength = zui_slider(dispHandle, tr("Displacement Strength"), 0.0, 10.0, true);
				if (dispHandle.changed) {
					Context.raw.ddirty = 2;
					MakeMaterial.parseMeshMaterial();
				}
			}
			if (zui_tab(BoxPreferences.htab, tr("Keymap"), true)) {

				if (BoxPreferences.filesKeymap == null) {
					BoxPreferences.fetchKeymaps();
				}

				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

				BoxPreferences.presetHandle = zui_handle("boxpreferences_50", { position: BoxPreferences.getPresetIndex() });
				zui_combo(BoxPreferences.presetHandle, BoxPreferences.filesKeymap, tr("Preset"));
				if (BoxPreferences.presetHandle.changed) {
					Config.raw.keymap = BoxPreferences.filesKeymap[BoxPreferences.presetHandle.position] + ".json";
					Config.applyConfig();
					Config.loadKeymap();
				}

				if (zui_button(tr("New"))) {
					UIBox.showCustom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_51"), tr("New Keymap"))) {
							zui_row([0.5, 0.5]);
							let keymapName = zui_text_input(zui_handle("boxpreferences_52", { text: "new_keymap" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template = JSON.stringify(Base.defaultKeymap);
								if (!keymapName.endsWith(".json")) keymapName += ".json";
								let path = Path.data() + Path.sep + "keymap_presets" + Path.sep + keymapName;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.fetchKeymaps(); // Refresh file list
								Config.raw.keymap = keymapName;
								BoxPreferences.presetHandle.position = BoxPreferences.getPresetIndex();
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
						let path = Path.data() + Path.sep + "keymap_presets" + Path.sep + Config.raw.keymap;
						File.copy(path, dest + Path.sep + UIFiles.filename);
					});
				}

				zui_end_sticky();

				zui_separator(8, false);

				let i = 0;
				ui.changed = false;
				for (let key in Config.keymap) {
					let h = zui_nest(zui_handle("boxpreferences_53"), i++);
					h.text = Config.keymap[key];
					let text = zui_text_input(h, key, Align.Left);
					Config.keymap[key] = text;
				}
				if (ui.changed) {
					Config.applyConfig();
					Config.saveKeymap();
				}
			}
			if (zui_tab(BoxPreferences.htab, tr("Plugins"), true)) {
				zui_begin_sticky();
				zui_row([1 / 4, 1 / 4]);
				if (zui_button(tr("New"))) {
					UIBox.showCustom((ui: zui_t) => {
						if (zui_tab(zui_handle("boxpreferences_54"), tr("New Plugin"))) {
							zui_row([0.5, 0.5]);
							let pluginName = zui_text_input(zui_handle("boxpreferences_55", { text: "new_plugin" }), tr("Name"));
							if (zui_button(tr("OK")) || ui.is_return_down) {
								let template =
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
								let path = Path.data() + Path.sep + "plugins" + Path.sep + pluginName;
								krom_file_save_bytes(path, sys_string_to_buffer(template));
								BoxPreferences.filesPlugin = null; // Refresh file list
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

				if (BoxPreferences.filesPlugin == null) {
					BoxPreferences.fetchPlugins();
				}

				if (Config.raw.plugins == null) Config.raw.plugins = [];
				let h = zui_handle("boxpreferences_56", { selected: false });
				for (let f of BoxPreferences.filesPlugin) {
					let isJs = f.endsWith(".js");
					if (!isJs) continue;
					let enabled = Config.raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					let tag = isJs ? f.split(".")[0] : f;
					zui_check(h, tag);
					if (h.changed && h.selected != enabled) {
						h.selected ? Config.enablePlugin(f) : Config.disablePlugin(f);
						Base.redrawUI();
					}
					if (ui.is_hovered && ui.input_released_r) {
						UIMenu.draw((ui: zui_t) => {
							let path = Path.data() + Path.sep + "plugins" + Path.sep + f;
							if (UIMenu.menuButton(ui, tr("Edit in Text Editor"))) {
								File.start(path);
							}
							if (UIMenu.menuButton(ui, tr("Edit in Script Tab"))) {
								data_get_blob("plugins/" + f, (blob: ArrayBuffer) => {
									TabScript.hscript.text = sys_buffer_to_string(blob);
									data_delete_blob("plugins/" + f);
									Console.info(tr("Script opened"));
								});

							}
							if (UIMenu.menuButton(ui, tr("Export"))) {
								UIFiles.show("js", true, false, (dest: string) => {
									if (!UIFiles.filename.endsWith(".js")) UIFiles.filename += ".js";
									File.copy(path, dest + Path.sep + UIFiles.filename);
								});
							}
							if (UIMenu.menuButton(ui, tr("Delete"))) {
								if (Config.raw.plugins.indexOf(f) >= 0) {
									array_remove(Config.raw.plugins, f);
									Plugin.stop(f);
								}
								array_remove(BoxPreferences.filesPlugin, f);
								File.delete(path);
							}
						}, 4);
					}
				}
			}

		}, 620, Config.raw.touch_ui ? 480 : 420, () => { Config.save(); });
	}

	static fetchThemes = () => {
		BoxPreferences.themes = File.readDirectory(Path.data() + Path.sep + "themes");
		for (let i = 0; i < BoxPreferences.themes.length; ++i) BoxPreferences.themes[i] = BoxPreferences.themes[i].substr(0, BoxPreferences.themes[i].length - 5); // Strip .json
		BoxPreferences.themes.unshift("default");
	}

	static fetchKeymaps = () => {
		BoxPreferences.filesKeymap = File.readDirectory(Path.data() + Path.sep + "keymap_presets");
		for (let i = 0; i < BoxPreferences.filesKeymap.length; ++i) {
			BoxPreferences.filesKeymap[i] = BoxPreferences.filesKeymap[i].substr(0, BoxPreferences.filesKeymap[i].length - 5); // Strip .json
		}
		BoxPreferences.filesKeymap.unshift("default");
	}

	static fetchPlugins = () => {
		BoxPreferences.filesPlugin = File.readDirectory(Path.data() + Path.sep + "plugins");
	}

	static getThemeIndex = (): i32 => {
		return BoxPreferences.themes.indexOf(Config.raw.theme.substr(0, Config.raw.theme.length - 5)); // Strip .json
	}

	static getPresetIndex = (): i32 => {
		return BoxPreferences.filesKeymap.indexOf(Config.raw.keymap.substr(0, Config.raw.keymap.length - 5)); // Strip .json
	}

	static setScale = () => {
		let scale = Config.raw.window_scale;
		zui_set_scale(UIBase.ui, scale);
		UIHeader.headerh = Math.floor(UIHeader.defaultHeaderH * scale);
		Config.raw.layout[LayoutSize.LayoutStatusH] = Math.floor(UIStatus.defaultStatusH * scale);
		UIMenubar.menubarw = Math.floor(UIMenubar.defaultMenubarW * scale);
		UIBase.setIconScale();
		zui_set_scale(UINodes.ui, scale);
		zui_set_scale(UIView2D.ui, scale);
		zui_set_scale(Base.uiBox, scale);
		zui_set_scale(Base.uiMenu, scale);
		Base.resize();
		///if (is_paint || is_sculpt)
		Config.raw.layout[LayoutSize.LayoutSidebarW] = Math.floor(UIBase.defaultSidebarW * scale);
		UIToolbar.toolbarw = Math.floor(UIToolbar.defaultToolbarW * scale);
		///end
	}
}
