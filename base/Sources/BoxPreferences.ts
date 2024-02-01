
class BoxPreferences {

	static htab = new Handle();
	static filesPlugin: string[] = null;
	static filesKeymap: string[] = null;
	static themeHandle: Handle;
	static presetHandle: Handle;
	static locales: string[] = null;
	static themes: string[] = null;
	static worldColor = 0xff080808;

	static show = () => {

		UIBox.showCustom((ui: Zui) => {
			if (ui.tab(BoxPreferences.htab, tr("Interface"), true)) {

				if (BoxPreferences.locales == null) {
					BoxPreferences.locales = Translator.getSupportedLocales();
				}

				let localeHandle = Zui.handle("boxpreferences_0", { position: BoxPreferences.locales.indexOf(Config.raw.locale) });
				ui.combo(localeHandle, BoxPreferences.locales, tr("Language"), true);
				if (localeHandle.changed) {
					let localeCode = BoxPreferences.locales[localeHandle.position];
					Config.raw.locale = localeCode;
					Translator.loadTranslations(localeCode);
					UIBase.tagUIRedraw();
				}

				let hscale = Zui.handle("boxpreferences_1", { value: Config.raw.window_scale });
				ui.slider(hscale, tr("UI Scale"), 1.0, 4.0, true, 10);
				if (Context.raw.hscaleWasChanged && !ui.inputDown) {
					Context.raw.hscaleWasChanged = false;
					if (hscale.value == null || isNaN(hscale.value)) hscale.value = 1.0;
					Config.raw.window_scale = hscale.value;
					BoxPreferences.setScale();
				}
				if (hscale.changed) Context.raw.hscaleWasChanged = true;

				let hspeed = Zui.handle("boxpreferences_2", { value: Config.raw.camera_zoom_speed });
				Config.raw.camera_zoom_speed = ui.slider(hspeed, tr("Camera Zoom Speed"), 0.1, 4.0, true);

				hspeed = Zui.handle("boxpreferences_3", { value: Config.raw.camera_rotation_speed });
				Config.raw.camera_rotation_speed = ui.slider(hspeed, tr("Camera Rotation Speed"), 0.1, 4.0, true);

				hspeed = Zui.handle("boxpreferences_4", { value: Config.raw.camera_pan_speed });
				Config.raw.camera_pan_speed = ui.slider(hspeed, tr("Camera Pan Speed"), 0.1, 4.0, true);

				let zoomDirectionHandle = Zui.handle("boxpreferences_5", { position: Config.raw.zoom_direction });
				ui.combo(zoomDirectionHandle, [tr("Vertical"), tr("Vertical Inverted"), tr("Horizontal"), tr("Horizontal Inverted"), tr("Vertical and Horizontal"), tr("Vertical and Horizontal Inverted")], tr("Direction to Zoom"), true);
				if (zoomDirectionHandle.changed) {
					Config.raw.zoom_direction = zoomDirectionHandle.position;
				}

				Config.raw.wrap_mouse = ui.check(Zui.handle("boxpreferences_6", { selected: Config.raw.wrap_mouse }), tr("Wrap Mouse"));
				if (ui.isHovered) ui.tooltip(tr("Wrap mouse around view boundaries during camera control"));

				Config.raw.node_preview = ui.check(Zui.handle("boxpreferences_7", { selected: Config.raw.node_preview }), tr("Show Node Preview"));

				ui.changed = false;
				Config.raw.show_asset_names = ui.check(Zui.handle("boxpreferences_8", { selected: Config.raw.show_asset_names }), tr("Show Asset Names"));
				if (ui.changed) {
					UIBase.tagUIRedraw();
				}

				///if !(krom_android || krom_ios)
				ui.changed = false;
				Config.raw.touch_ui = ui.check(Zui.handle("boxpreferences_9", { selected: Config.raw.touch_ui }), tr("Touch UI"));
				if (ui.changed) {
					Zui.touchScroll = Zui.touchHold = Zui.touchTooltip = Config.raw.touch_ui;
					Config.loadTheme(Config.raw.theme);
					BoxPreferences.setScale();
					UIBase.tagUIRedraw();
				}
				///end

				Config.raw.splash_screen = ui.check(Zui.handle("boxpreferences_10", { selected: Config.raw.splash_screen }), tr("Splash Screen"));

				// ui.text("Node Editor");
				// let gridSnap = ui.check(Zui.handle("boxpreferences_11", { selected: false }), "Grid Snap");

				ui.endElement();
				ui.row([0.5, 0.5]);
				if (ui.button(tr("Restore")) && !UIMenu.show) {
					UIMenu.draw((ui: Zui) => {
						if (UIMenu.menuButton(ui, tr("Confirm"))) {
							App.notifyOnInit(() => {
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
								Data.getBlob(path, (b: ArrayBuffer) => {
									let raw = JSON.parse(System.bufferToString(b));
									App.notifyOnInit(() => {
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
				if (ui.button(tr("Reset Layout")) && !UIMenu.show) {
					UIMenu.draw((ui: Zui) => {
						if (UIMenu.menuButton(ui, tr("Confirm"))) {
							Base.initLayout();
							Config.save();
						}
					}, 1);
				}
			}

			if (ui.tab(BoxPreferences.htab, tr("Theme"), true)) {

				if (BoxPreferences.themes == null) {
					BoxPreferences.fetchThemes();
				}
				BoxPreferences.themeHandle = Zui.handle("boxpreferences_12", { position: BoxPreferences.getThemeIndex() });

				ui.beginSticky();
				ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

				ui.combo(BoxPreferences.themeHandle, BoxPreferences.themes, tr("Theme"));
				if (BoxPreferences.themeHandle.changed) {
					Config.raw.theme = BoxPreferences.themes[BoxPreferences.themeHandle.position] + ".json";
					Config.loadTheme(Config.raw.theme);
				}

				if (ui.button(tr("New"))) {
					UIBox.showCustom((ui: Zui) => {
						if (ui.tab(Zui.handle("boxpreferences_13"), tr("New Theme"))) {
							ui.row([0.5, 0.5]);
							let themeName = ui.textInput(Zui.handle("boxpreferences_14", { text: "new_theme" }), tr("Name"));
							if (ui.button(tr("OK")) || ui.isReturnDown) {
								let template = JSON.stringify(Base.theme);
								if (!themeName.endsWith(".json")) themeName += ".json";
								let path = Path.data() + Path.sep + "themes" + Path.sep + themeName;
								Krom.fileSaveBytes(path, System.stringToBuffer(template));
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

				if (ui.button(tr("Import"))) {
					UIFiles.show("json", false, false, (path: string) => {
						ImportTheme.run(path);
					});
				}

				if (ui.button(tr("Export"))) {
					UIFiles.show("json", true, false, (path: string) => {
						path += Path.sep + UIFiles.filename;
						if (!path.endsWith(".json")) path += ".json";
						Krom.fileSaveBytes(path, System.stringToBuffer(JSON.stringify(Base.theme)));
					});
				}

				ui.endSticky();

				let i = 0;
				let theme: any = Base.theme;
				let hlist = Zui.handle("boxpreferences_15");

				// Viewport color
				let h = hlist.nest(i++, { color: BoxPreferences.worldColor });
				ui.row([1 / 8, 7 / 8]);
				ui.text("", 0, h.color);
				if (ui.isHovered && ui.inputReleased) {
					UIMenu.draw((ui) => {
						ui.changed = false;
						ui.colorWheel(h, false, null, 11 * ui.t.ELEMENT_H * ui.SCALE(), true);
						if (ui.changed) UIMenu.keepOpen = true;
					}, 11);
				}
				let val = h.color;
				if (val < 0) val += 4294967296;
				h.text = val.toString(16);
				ui.textInput(h, "VIEWPORT_COL");
				h.color = parseInt(h.text, 16);

				if (BoxPreferences.worldColor != h.color) {
					BoxPreferences.worldColor = h.color;
					let b = new Uint8Array(4);
					b[0] = color_get_rb(BoxPreferences.worldColor);
					b[1] = color_get_gb(BoxPreferences.worldColor);
					b[2] = color_get_bb(BoxPreferences.worldColor);
					b[3] = 255;
					Context.raw.emptyEnvmap = Image.fromBytes(b.buffer, 1, 1);
					Context.raw.ddirty = 2;
					if (!Context.raw.showEnvmap) {
						Scene.world._envmap = Context.raw.emptyEnvmap;
					}
				}

				// Theme fields
				for (let key of Object.getOwnPropertyNames(Theme.prototype)) {
					if (key == "constructor") continue;

					let h = hlist.nest(i++);
					let val: any = theme[key];

					let isHex = key.endsWith("_COL");
					if (isHex && val < 0) val += 4294967296;

					if (isHex) {
						ui.row([1 / 8, 7 / 8]);
						ui.text("", 0, val);
						if (ui.isHovered && ui.inputReleased) {
							h.color = theme[key];
							UIMenu.draw((ui) => {
								ui.changed = false;
								let color = ui.colorWheel(h, false, null, 11 * ui.t.ELEMENT_H * ui.SCALE(), true);
								theme[key] = color;
								if (ui.changed) UIMenu.keepOpen = true;
							}, 11);
						}
					}

					ui.changed = false;

					if (typeof val == "boolean") {
						h.selected = val;
						let b = ui.check(h, key);
						theme[key] = b;
					}
					else if (key == "LINK_STYLE") {
						let styles = [tr("Straight"), tr("Curved")];
						h.position = val;
						let i = ui.combo(h, styles, key, true);
						theme[key] = i;
					}
					else {
						h.text = isHex ? val.toString(16) : val.toString();
						let res = ui.textInput(h, key);
						if (isHex) theme[key] = parseInt(h.text, 16);
						else theme[key] = parseInt(h.text);
					}

					if (ui.changed) {
						for (let ui of Base.getUIs()) {
							ui.elementsBaked = false;
						}
					}
				}
			}

			if (ui.tab(BoxPreferences.htab, tr("Usage"), true)) {
				Context.raw.undoHandle = Zui.handle("boxpreferences_16", { value: Config.raw.undo_steps });
				Config.raw.undo_steps = Math.floor(ui.slider(Context.raw.undoHandle, tr("Undo Steps"), 1, 64, false, 1));
				if (Config.raw.undo_steps < 1) {
					Config.raw.undo_steps = Math.floor(Context.raw.undoHandle.value = 1);
				}
				if (Context.raw.undoHandle.changed) {
					ui.g.end();

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
					ui.g.begin(false);
				}

				///if is_paint
				Config.raw.dilate_radius = Math.floor(ui.slider(Zui.handle("boxpreferences_17", { value: Config.raw.dilate_radius }), tr("Dilate Radius"), 0.0, 16.0, true, 1));
				if (ui.isHovered) ui.tooltip(tr("Dilate painted textures to prevent seams"));

				let dilateHandle = Zui.handle("boxpreferences_18", { position: Config.raw.dilate });
				ui.combo(dilateHandle, [tr("Instant"), tr("Delayed")], tr("Dilate"), true);
				if (dilateHandle.changed) {
					Config.raw.dilate = dilateHandle.position;
				}
				///end

				///if is_lab
				let workspaceHandle = Zui.handle("boxpreferences_19", { position: Config.raw.workspace });
				ui.combo(workspaceHandle, [tr("3D View"), tr("2D View")], tr("Default Workspace"), true);
				if (workspaceHandle.changed) {
					Config.raw.workspace = workspaceHandle.position;
				}
				///end

				let cameraControlsHandle = Zui.handle("boxpreferences_20", { position: Config.raw.camera_controls });
				ui.combo(cameraControlsHandle, [tr("Orbit"), tr("Rotate"), tr("Fly")], tr("Default Camera Controls"), true);
				if (cameraControlsHandle.changed) {
					Config.raw.camera_controls = cameraControlsHandle.position;
				}

				let layerResHandle = Zui.handle("boxpreferences_21", { position: Config.raw.layer_res });

				///if is_paint
				///if (krom_android || krom_ios)
				ui.combo(layerResHandle, ["128", "256", "512", "1K", "2K", "4K"], tr("Default Layer Resolution"), true);
				///else
				ui.combo(layerResHandle, ["128", "256", "512", "1K", "2K", "4K", "8K"], tr("Default Layer Resolution"), true);
				///end
				///end

				///if is_lab
				///if (krom_android || krom_ios)
				ui.combo(layerResHandle, ["2K", "4K"], tr("Default Layer Resolution"), true);
				///else
				ui.combo(layerResHandle, ["2K", "4K", "8K", "16K"], tr("Default Layer Resolution"), true);
				///end
				///end

				if (layerResHandle.changed) {
					Config.raw.layer_res = layerResHandle.position;
				}

				let serverHandle = Zui.handle("boxpreferences_22", { text: Config.raw.server });
				Config.raw.server = ui.textInput(serverHandle, tr("Cloud Server"));

				///if (is_paint || is_sculpt)
				let materialLiveHandle = Zui.handle("boxpreferences_23", {selected: Config.raw.material_live });
				Config.raw.material_live = ui.check(materialLiveHandle, tr("Live Material Preview"));
				if (ui.isHovered) ui.tooltip(tr("Instantly update material preview on node change"));

				let brushLiveHandle = Zui.handle("boxpreferences_24", { selected: Config.raw.brush_live });
				Config.raw.brush_live = ui.check(brushLiveHandle, tr("Live Brush Preview"));
				if (ui.isHovered) ui.tooltip(tr("Draw live brush preview in viewport"));
				if (brushLiveHandle.changed) Context.raw.ddirty = 2;

				let brush3dHandle = Zui.handle("boxpreferences_25", { selected: Config.raw.brush_3d });
				Config.raw.brush_3d = ui.check(brush3dHandle, tr("3D Cursor"));
				if (brush3dHandle.changed) MakeMaterial.parsePaintMaterial();

				ui.enabled = Config.raw.brush_3d;
				let brushDepthRejectHandle = Zui.handle("boxpreferences_26", { selected: Config.raw.brush_depth_reject });
				Config.raw.brush_depth_reject = ui.check(brushDepthRejectHandle, tr("Depth Reject"));
				if (brushDepthRejectHandle.changed) MakeMaterial.parsePaintMaterial();

				ui.row([0.5, 0.5]);

				let brushAngleRejectHandle = Zui.handle("boxpreferences_27", { selected: Config.raw.brush_angle_reject });
				Config.raw.brush_angle_reject = ui.check(brushAngleRejectHandle, tr("Angle Reject"));
				if (brushAngleRejectHandle.changed) MakeMaterial.parsePaintMaterial();

				if (!Config.raw.brush_angle_reject) ui.enabled = false;
				let angleDotHandle = Zui.handle("boxpreferences_28", { value: Context.raw.brushAngleRejectDot });
				Context.raw.brushAngleRejectDot = ui.slider(angleDotHandle, tr("Angle"), 0.0, 1.0, true);
				if (angleDotHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
				ui.enabled = true;
				///end

				///if is_lab
				Config.raw.gpu_inference = ui.check(Zui.handle("boxpreferences_29", { selected: Config.raw.gpu_inference }), tr("Use GPU"));
				if (ui.isHovered) ui.tooltip(tr("Use GPU to accelerate node graph processing"));
				///end
			}

			let penName;
			///if krom_ios
			penName = tr("Pencil");
			///else
			penName = tr("Pen");
			///end

			if (ui.tab(BoxPreferences.htab, penName, true)) {
				ui.text(tr("Pressure controls"));
				Config.raw.pressure_radius = ui.check(Zui.handle("boxpreferences_30", { selected: Config.raw.pressure_radius }), tr("Brush Radius"));
				Config.raw.pressure_sensitivity = ui.slider(Zui.handle("boxpreferences_31", { value: Config.raw.pressure_sensitivity }), tr("Sensitivity"), 0.0, 10.0, true);
				///if (is_paint || is_sculpt)
				Config.raw.pressure_hardness = ui.check(Zui.handle("boxpreferences_32", { selected: Config.raw.pressure_hardness }), tr("Brush Hardness"));
				Config.raw.pressure_opacity = ui.check(Zui.handle("boxpreferences_33", { selected: Config.raw.pressure_opacity }), tr("Brush Opacity"));
				Config.raw.pressure_angle = ui.check(Zui.handle("boxpreferences_34", { selected: Config.raw.pressure_angle }), tr("Brush Angle"));
				///end

				ui.endElement();
				ui.row([0.5]);
				if (ui.button(tr("Help"))) {
					///if (is_paint || is_sculpt)
					File.loadUrl("https://github.com/armory3d/armorpaint_docs///pen");
					///end
					///if is_lab
					File.loadUrl("https://github.com/armory3d/armorlab_docs///pen");
					///end
				}
			}

			Context.raw.hssao = Zui.handle("boxpreferences_35", { selected: Config.raw.rp_ssao });
			Context.raw.hssr = Zui.handle("boxpreferences_36", { selected: Config.raw.rp_ssr });
			Context.raw.hbloom = Zui.handle("boxpreferences_37", { selected: Config.raw.rp_bloom });
			Context.raw.hsupersample = Zui.handle("boxpreferences_38", { position: Config.getSuperSampleQuality(Config.raw.rp_supersample) });
			Context.raw.hvxao = Zui.handle("boxpreferences_39", { selected: Config.raw.rp_gi });
			if (ui.tab(BoxPreferences.htab, tr("Viewport"), true)) {
				///if (krom_direct3d12 || krom_vulkan || krom_metal)

				let hpathtracemode = Zui.handle("boxpreferences_40", { position: Context.raw.pathTraceMode });
				Context.raw.pathTraceMode = ui.combo(hpathtracemode, [tr("Core"), tr("Full")], tr("Path Tracer"), true);
				if (hpathtracemode.changed) {
					RenderPathRaytrace.ready = false;
				}

				///end

				let hrendermode = Zui.handle("boxpreferences_41", { position: Context.raw.renderMode });
				Context.raw.renderMode = ui.combo(hrendermode, [tr("Full"), tr("Mobile")], tr("Renderer"), true);
				if (hrendermode.changed) {
					Context.setRenderPath();
				}

				ui.combo(Context.raw.hsupersample, ["0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x"], tr("Super Sample"), true);
				if (Context.raw.hsupersample.changed) Config.applyConfig();

				if (Context.raw.renderMode == RenderMode.RenderDeferred) {
					///if arm_voxels
					ui.check(Context.raw.hvxao, tr("Voxel AO"));
					if (ui.isHovered) ui.tooltip(tr("Cone-traced AO and shadows"));
					if (Context.raw.hvxao.changed) {
						Config.applyConfig();
					}

					ui.enabled = Context.raw.hvxao.selected;
					let h = Zui.handle("boxpreferences_42", { value: Context.raw.vxaoOffset });
					Context.raw.vxaoOffset = ui.slider(h, tr("Cone Offset"), 1.0, 4.0, true);
					if (h.changed) Context.raw.ddirty = 2;
					h = Zui.handle("boxpreferences_43", { value: Context.raw.vxaoAperture });
					Context.raw.vxaoAperture = ui.slider(h, tr("Aperture"), 1.0, 4.0, true);
					if (h.changed) Context.raw.ddirty = 2;
					ui.enabled = true;
					///end

					ui.check(Context.raw.hssao, tr("SSAO"));
					if (Context.raw.hssao.changed) Config.applyConfig();
					ui.check(Context.raw.hssr, tr("SSR"));
					if (Context.raw.hssr.changed) Config.applyConfig();
					ui.check(Context.raw.hbloom, tr("Bloom"));
					if (Context.raw.hbloom.changed) Config.applyConfig();
				}

				let h = Zui.handle("boxpreferences_44", { value: Config.raw.rp_vignette });
				Config.raw.rp_vignette = ui.slider(h, tr("Vignette"), 0.0, 1.0, true);
				if (h.changed) Context.raw.ddirty = 2;

				h = Zui.handle("boxpreferences_45", { value: Config.raw.rp_grain });
				Config.raw.rp_grain = ui.slider(h, tr("Noise Grain"), 0.0, 1.0, true);
				if (h.changed) Context.raw.ddirty = 2;

				// let h = Zui.handle("boxpreferences_46", { value: Context.raw.autoExposureStrength });
				// Context.raw.autoExposureStrength = ui.slider(h, "Auto Exposure", 0.0, 2.0, true);
				// if (h.changed) Context.raw.ddirty = 2;

				let cam = Scene.camera;
				let camRaw = cam.data;
				let near_handle = Zui.handle("boxpreferences_47");
				let far_handle = Zui.handle("boxpreferences_48");
				near_handle.value = Math.floor(camRaw.near_plane * 1000) / 1000;
				far_handle.value = Math.floor(camRaw.far_plane * 100) / 100;
				camRaw.near_plane = ui.slider(near_handle, tr("Clip Start"), 0.001, 1.0, true);
				camRaw.far_plane = ui.slider(far_handle, tr("Clip End"), 50.0, 100.0, true);
				if (near_handle.changed || far_handle.changed) {
					CameraObject.buildProjection(cam);
				}

				let dispHandle = Zui.handle("boxpreferences_49", { value: Config.raw.displace_strength });
				Config.raw.displace_strength = ui.slider(dispHandle, tr("Displacement Strength"), 0.0, 10.0, true);
				if (dispHandle.changed) {
					Context.raw.ddirty = 2;
					MakeMaterial.parseMeshMaterial();
				}
			}
			if (ui.tab(BoxPreferences.htab, tr("Keymap"), true)) {

				if (BoxPreferences.filesKeymap == null) {
					BoxPreferences.fetchKeymaps();
				}

				ui.beginSticky();
				ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);

				BoxPreferences.presetHandle = Zui.handle("boxpreferences_50", { position: BoxPreferences.getPresetIndex() });
				ui.combo(BoxPreferences.presetHandle, BoxPreferences.filesKeymap, tr("Preset"));
				if (BoxPreferences.presetHandle.changed) {
					Config.raw.keymap = BoxPreferences.filesKeymap[BoxPreferences.presetHandle.position] + ".json";
					Config.applyConfig();
					Config.loadKeymap();
				}

				if (ui.button(tr("New"))) {
					UIBox.showCustom((ui: Zui) => {
						if (ui.tab(Zui.handle("boxpreferences_51"), tr("New Keymap"))) {
							ui.row([0.5, 0.5]);
							let keymapName = ui.textInput(Zui.handle("boxpreferences_52", { text: "new_keymap" }), tr("Name"));
							if (ui.button(tr("OK")) || ui.isReturnDown) {
								let template = JSON.stringify(Base.defaultKeymap);
								if (!keymapName.endsWith(".json")) keymapName += ".json";
								let path = Path.data() + Path.sep + "keymap_presets" + Path.sep + keymapName;
								Krom.fileSaveBytes(path, System.stringToBuffer(template));
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

				if (ui.button(tr("Import"))) {
					UIFiles.show("json", false, false, (path: string) => {
						ImportKeymap.run(path);
					});
				}
				if (ui.button(tr("Export"))) {
					UIFiles.show("json", true, false, (dest: string) => {
						if (!UIFiles.filename.endsWith(".json")) UIFiles.filename += ".json";
						let path = Path.data() + Path.sep + "keymap_presets" + Path.sep + Config.raw.keymap;
						File.copy(path, dest + Path.sep + UIFiles.filename);
					});
				}

				ui.endSticky();

				ui.separator(8, false);

				let i = 0;
				ui.changed = false;
				for (let key in Config.keymap) {
					let h = Zui.handle("boxpreferences_53").nest(i++);
					h.text = Config.keymap[key];
					let text = ui.textInput(h, key, Align.Left);
					Config.keymap[key] = text;
				}
				if (ui.changed) {
					Config.applyConfig();
					Config.saveKeymap();
				}
			}
			if (ui.tab(BoxPreferences.htab, tr("Plugins"), true)) {
				ui.beginSticky();
				ui.row([1 / 4, 1 / 4]);
				if (ui.button(tr("New"))) {
					UIBox.showCustom((ui: Zui) => {
						if (ui.tab(Zui.handle("boxpreferences_54"), tr("New Plugin"))) {
							ui.row([0.5, 0.5]);
							let pluginName = ui.textInput(Zui.handle("boxpreferences_55", { text: "new_plugin" }), tr("Name"));
							if (ui.button(tr("OK")) || ui.isReturnDown) {
								let template =
`let plugin = Plugin.create();
let h1 = new Handle();
plugin.drawUI = (ui) { =>
	if (ui.panel(h1, 'New Plugin')) {
		if (ui.button('Button')) {
			console.error('Hello');
		}
	}
}
`;
								if (!pluginName.endsWith(".js")) pluginName += ".js";
								let path = Path.data() + Path.sep + "plugins" + Path.sep + pluginName;
								Krom.fileSaveBytes(path, System.stringToBuffer(template));
								BoxPreferences.filesPlugin = null; // Refresh file list
								UIBox.hide();
								BoxPreferences.htab.position = 6; // Plugins
								BoxPreferences.show();
							}
						}
					});
				}
				if (ui.button(tr("Import"))) {
					UIFiles.show("js,zip", false, false, (path: string) => {
						ImportPlugin.run(path);
					});
				}
				ui.endSticky();

				if (BoxPreferences.filesPlugin == null) {
					BoxPreferences.fetchPlugins();
				}

				if (Config.raw.plugins == null) Config.raw.plugins = [];
				let h = Zui.handle("boxpreferences_56", { selected: false });
				for (let f of BoxPreferences.filesPlugin) {
					let isJs = f.endsWith(".js");
					if (!isJs) continue;
					let enabled = Config.raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					let tag = isJs ? f.split(".")[0] : f;
					ui.check(h, tag);
					if (h.changed && h.selected != enabled) {
						h.selected ? Config.enablePlugin(f) : Config.disablePlugin(f);
						Base.redrawUI();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw((ui: Zui) => {
							let path = Path.data() + Path.sep + "plugins" + Path.sep + f;
							if (UIMenu.menuButton(ui, tr("Edit in Text Editor"))) {
								File.start(path);
							}
							if (UIMenu.menuButton(ui, tr("Edit in Script Tab"))) {
								Data.getBlob("plugins/" + f, (blob: ArrayBuffer) => {
									TabScript.hscript.text = System.bufferToString(blob);
									Data.deleteBlob("plugins/" + f);
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
		UIBase.ui.setScale(scale);
		UIHeader.headerh = Math.floor(UIHeader.defaultHeaderH * scale);
		Config.raw.layout[LayoutSize.LayoutStatusH] = Math.floor(UIStatus.defaultStatusH * scale);
		UIMenubar.menubarw = Math.floor(UIMenubar.defaultMenubarW * scale);
		UIBase.setIconScale();
		UINodes.ui.setScale(scale);
		UIView2D.ui.setScale(scale);
		Base.uiBox.setScale(scale);
		Base.uiMenu.setScale(scale);
		Base.resize();
		///if (is_paint || is_sculpt)
		Config.raw.layout[LayoutSize.LayoutSidebarW] = Math.floor(UIBase.defaultSidebarW * scale);
		UIToolbar.toolbarw = Math.floor(UIToolbar.defaultToolbarW * scale);
		///end
	}
}
