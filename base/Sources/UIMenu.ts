
class UIMenu {

	static show = false;
	static menuCategory = 0;
	static menuCategoryW = 0;
	static menuCategoryH = 0;
	static menuX = 0;
	static menuY = 0;
	static menuElements = 0;
	static keepOpen = false;
	static menuCommands: (ui: zui_t)=>void = null;
	static showMenuFirst = true;
	static hideMenu = false;

	static render = () => {
		let ui = Base.uiMenu;
		let menuW = UIMenu.menuCommands != null ? Math.floor(Base.defaultElementW * zui_SCALE(Base.uiMenu) * 2.3) : Math.floor(zui_ELEMENT_W(ui) * 2.3);
		let _BUTTON_COL = ui.t.BUTTON_COL;
		ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;
		let _ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
		ui.t.ELEMENT_OFFSET = 0;
		let _ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Config.raw.touch_ui ? (28 + 2) : 28;

		zui_begin_region(ui, UIMenu.menuX, UIMenu.menuY, menuW);

		if (UIMenu.menuCommands != null) {
			g2_set_color(ui.t.ACCENT_SELECT_COL);
			zui_draw_rect(true, ui._x + -1, ui._y + -1, ui._w + 2, zui_ELEMENT_H(ui) * UIMenu.menuElements + 2);
			g2_set_color(ui.t.SEPARATOR_COL);
			zui_draw_rect(true, ui._x + 0, ui._y + 0, ui._w, zui_ELEMENT_H(ui) * UIMenu.menuElements);
			g2_set_color(0xffffffff);

			UIMenu.menuCommands(ui);
		}
		else {
			UIMenu.menuStart(ui);
			if (UIMenu.menuCategory == MenuCategory.MenuFile) {
				if (UIMenu.menuButton(ui, tr("New Project..."), Config.keymap.file_new)) Project.projectNewBox();
				if (UIMenu.menuButton(ui, tr("Open..."), Config.keymap.file_open)) Project.projectOpen();
				if (UIMenu.menuButton(ui, tr("Open Recent..."), Config.keymap.file_open_recent)) BoxProjects.show();
				if (UIMenu.menuButton(ui, tr("Save"), Config.keymap.file_save)) Project.projectSave();
				if (UIMenu.menuButton(ui, tr("Save As..."), Config.keymap.file_save_as)) Project.projectSaveAs();
				UIMenu.menuSeparator(ui);
				if (UIMenu.menuButton(ui, tr("Import Texture..."), Config.keymap.file_import_assets)) Project.importAsset(Path.textureFormats.join(","), false);
				if (UIMenu.menuButton(ui, tr("Import Envmap..."))) {
					UIFiles.show("hdr", false, false, (path: string) => {
						if (!path.endsWith(".hdr")) {
							Console.error(tr("Error: .hdr file expected"));
							return;
						}
						ImportAsset.run(path);
					});
				}

				///if (is_paint || is_sculpt)
				if (UIMenu.menuButton(ui, tr("Import Font..."))) Project.importAsset("ttf,ttc,otf");
				if (UIMenu.menuButton(ui, tr("Import Material..."))) Project.importMaterial();
				if (UIMenu.menuButton(ui, tr("Import Brush..."))) Project.importBrush();
				///end

				///if (is_paint || is_lab)
				if (UIMenu.menuButton(ui, tr("Import Swatches..."))) Project.importSwatches();
				///end
				if (UIMenu.menuButton(ui, tr("Import Mesh..."))) Project.importMesh();
				if (UIMenu.menuButton(ui, tr("Reimport Mesh"), Config.keymap.file_reimport_mesh)) Project.reimportMesh();
				if (UIMenu.menuButton(ui, tr("Reimport Textures"), Config.keymap.file_reimport_textures)) Project.reimportTextures();
				UIMenu.menuSeparator(ui);
				///if (is_paint || is_lab)
				if (UIMenu.menuButton(ui, tr("Export Textures..."), Config.keymap.file_export_textures_as)) {
					///if is_paint
					Context.raw.layersExport = ExportMode.ExportVisible;
					///end
					BoxExport.showTextures();
				}
				if (UIMenu.menuButton(ui, tr("Export Swatches..."))) Project.exportSwatches();
				///end
				if (UIMenu.menuButton(ui, tr("Export Mesh..."))) {
					Context.raw.exportMeshIndex = 0; // All
					BoxExport.showMesh();
				}

				///if is_paint
				if (UIMenu.menuButton(ui, tr("Bake Material..."))) BoxExport.showBakeMaterial();
				///end

				UIMenu.menuSeparator(ui);
				if (UIMenu.menuButton(ui, tr("Exit"))) sys_stop();
			}
			else if (UIMenu.menuCategory == MenuCategory.MenuEdit) {
				let stepUndo = "";
				let stepRedo = "";
				if (History.undos > 0) {
					stepUndo = History.steps[History.steps.length - 1 - History.redos].name;
				}
				if (History.redos > 0) {
					stepRedo = History.steps[History.steps.length - History.redos].name;
				}
				ui.enabled = History.undos > 0;
				if (UIMenu.menuButton(ui, tr("Undo {step}", new Map([["step", stepUndo]])), Config.keymap.edit_undo)) History.undo();
				ui.enabled = History.redos > 0;
				if (UIMenu.menuButton(ui, tr("Redo {step}", new Map([["step", stepRedo]])), Config.keymap.edit_redo)) History.redo();
				ui.enabled = true;
				UIMenu.menuSeparator(ui);
				if (UIMenu.menuButton(ui, tr("Preferences..."), Config.keymap.edit_prefs)) BoxPreferences.show();
			}
			else if (UIMenu.menuCategory == MenuCategory.MenuViewport) {
				if (UIMenu.menuButton(ui, tr("Distract Free"), Config.keymap.view_distract_free)) {
					UIBase.toggleDistractFree();
					UIBase.ui.is_hovered = false;
				}

				///if !(krom_android || krom_ios)
				if (UIMenu.menuButton(ui, tr("Toggle Fullscreen"), "alt+enter")) {
					Base.toggleFullscreen();
				}
				///end

				ui.changed = false;

				UIMenu.menuFill(ui);
				let p = scene_world;
				let envHandle = zui_handle("uimenu_0");
				envHandle.value = p.strength;
				UIMenu.menuAlign(ui);
				p.strength = zui_slider(envHandle, tr("Environment"), 0.0, 8.0, true);
				if (envHandle.changed) Context.raw.ddirty = 2;

				UIMenu.menuFill(ui);
				let envaHandle = zui_handle("uimenu_1");
				envaHandle.value = Context.raw.envmapAngle / Math.PI * 180.0;
				if (envaHandle.value < 0) {
					envaHandle.value += (Math.floor(-envaHandle.value / 360) + 1) * 360;
				}
				else if (envaHandle.value > 360) {
					envaHandle.value -= Math.floor(envaHandle.value / 360) * 360;
				}
				UIMenu.menuAlign(ui);
				Context.raw.envmapAngle = zui_slider(envaHandle, tr("Environment Angle"), 0.0, 360.0, true, 1) / 180.0 * Math.PI;
				if (ui.is_hovered) zui_tooltip(tr("{shortcut} and move mouse", new Map([["shortcut", Config.keymap.rotate_envmap]])));
				if (envaHandle.changed) Context.raw.ddirty = 2;

				if (scene_lights.length > 0) {
					let light = scene_lights[0];

					UIMenu.menuFill(ui);
					let lhandle = zui_handle("uimenu_2");
					let scale = 1333;
					lhandle.value = light.data.strength / scale;
					lhandle.value = Math.floor(lhandle.value * 100) / 100;
					UIMenu.menuAlign(ui);
					light.data.strength = zui_slider(lhandle, tr("Light"), 0.0, 4.0, true) * scale;
					if (lhandle.changed) Context.raw.ddirty = 2;

					UIMenu.menuFill(ui);
					light = scene_lights[0];
					let lahandle = zui_handle("uimenu_3");
					lahandle.value = Context.raw.lightAngle / Math.PI * 180;
					UIMenu.menuAlign(ui);
					let newAngle = zui_slider(lahandle, tr("Light Angle"), 0.0, 360.0, true, 1) / 180 * Math.PI;
					if (ui.is_hovered) zui_tooltip(tr("{shortcut} and move mouse", new Map([["shortcut", Config.keymap.rotate_light]])));
					let ldiff = newAngle - Context.raw.lightAngle;
					if (Math.abs(ldiff) > 0.005) {
						if (newAngle < 0) newAngle += (Math.floor(-newAngle / (2 * Math.PI)) + 1) * 2 * Math.PI;
						else if (newAngle > 2 * Math.PI) newAngle -= Math.floor(newAngle / (2 * Math.PI)) * 2 * Math.PI;
						Context.raw.lightAngle = newAngle;
						let m = mat4_rot_z(ldiff);
						mat4_mult_mat(light.base.transform.local, m);
						transform_decompose(light.base.transform);
						Context.raw.ddirty = 2;
					}

					UIMenu.menuFill(ui);
					let sxhandle = zui_handle("uimenu_4");
					sxhandle.value = light.data.size;
					UIMenu.menuAlign(ui);
					light.data.size = zui_slider(sxhandle, tr("Light Size"), 0.0, 4.0, true);
					if (sxhandle.changed) Context.raw.ddirty = 2;
				}

				///if (is_paint || is_sculpt)
				UIMenu.menuFill(ui);
				let splitViewHandle = zui_handle("uimenu_5", { selected: Context.raw.splitView });
				Context.raw.splitView = zui_check(splitViewHandle, " " + tr("Split View"));
				if (splitViewHandle.changed) {
					Base.resize();
				}
				///end

				///if is_lab
				UIMenu.menuFill(ui);
				let brushScaleHandle = zui_handle("uimenu_6", { value: Context.raw.brushScale });
				UIMenu.menuAlign(ui);
				Context.raw.brushScale = zui_slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
				if (brushScaleHandle.changed) {
					MakeMaterial.parseMeshMaterial();
					///if (krom_direct3d12 || krom_vulkan || krom_metal)
					RenderPathRaytrace.uvScale = Context.raw.brushScale;
					RenderPathRaytrace.ready = false;
					///end
				}
				///end

				UIMenu.menuFill(ui);
				let cullHandle = zui_handle("uimenu_7", { selected: Context.raw.cullBackfaces });
				Context.raw.cullBackfaces = zui_check(cullHandle, " " + tr("Cull Backfaces"));
				if (cullHandle.changed) {
					MakeMaterial.parseMeshMaterial();
				}

				UIMenu.menuFill(ui);
				let filterHandle = zui_handle("uimenu_8", { selected: Context.raw.textureFilter });
				Context.raw.textureFilter = zui_check(filterHandle, " " + tr("Filter Textures"));
				if (filterHandle.changed) {
					MakeMaterial.parsePaintMaterial();
					MakeMaterial.parseMeshMaterial();
				}

				///if (is_paint || is_sculpt)
				UIMenu.menuFill(ui);
				Context.raw.drawWireframe = zui_check(Context.raw.wireframeHandle, " " + tr("Wireframe"));
				if (Context.raw.wireframeHandle.changed) {
					let current = _g2_current;
					g2_end();
					UtilUV.cacheUVMap();
					g2_begin(current, false);
					MakeMaterial.parseMeshMaterial();
				}
				///end

				///if is_paint
				UIMenu.menuFill(ui);
				Context.raw.drawTexels = zui_check(Context.raw.texelsHandle, " " + tr("Texels"));
				if (Context.raw.texelsHandle.changed) {
					MakeMaterial.parseMeshMaterial();
				}
				///end

				UIMenu.menuFill(ui);
				let compassHandle = zui_handle("uimenu_9", { selected: Context.raw.showCompass });
				Context.raw.showCompass = zui_check(compassHandle, " " + tr("Compass"));
				if (compassHandle.changed) Context.raw.ddirty = 2;

				UIMenu.menuFill(ui);
				Context.raw.showEnvmap = zui_check(Context.raw.showEnvmapHandle, " " + tr("Envmap"));
				if (Context.raw.showEnvmapHandle.changed) {
					Context.loadEnvmap();
					Context.raw.ddirty = 2;
				}

				UIMenu.menuFill(ui);
				Context.raw.showEnvmapBlur = zui_check(Context.raw.showEnvmapBlurHandle, " " + tr("Blur Envmap"));
				if (Context.raw.showEnvmapBlurHandle.changed) Context.raw.ddirty = 2;

				Context.updateEnvmap();

				if (ui.changed) UIMenu.keepOpen = true;
			}
			else if (UIMenu.menuCategory == MenuCategory.MenuMode) {
				let modeHandle = zui_handle("uimenu_10");
				modeHandle.position = Context.raw.viewportMode;
				let modes = [
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
				let shortcuts = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];

				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				if (krom_raytrace_supported()) {
					modes.push(tr("Path Traced"));
					shortcuts.push("p");
				}
				///end

				for (let i = 0; i < modes.length; ++i) {
					UIMenu.menuFill(ui);
					let shortcut = Config.raw.touch_ui ? "" : Config.keymap.viewport_mode + ", " + shortcuts[i];
					zui_radio(modeHandle, i, modes[i], shortcut);
				}

				if (modeHandle.changed) {
					Context.setViewportMode(modeHandle.position);
					// TODO: rotate mode is not supported for path tracing yet
					if (modeHandle.position == ViewportMode.ViewPathTrace && Context.raw.cameraControls == CameraControls.ControlsRotate) {
						Context.raw.cameraControls = CameraControls.ControlsOrbit;
						Viewport.reset();
					}
				}
			}
			else if (UIMenu.menuCategory == MenuCategory.MenuCamera) {
				if (UIMenu.menuButton(ui, tr("Reset"), Config.keymap.view_reset)) {
					Viewport.reset();
					Viewport.scaleToBounds();
				}
				UIMenu.menuSeparator(ui);
				if (UIMenu.menuButton(ui, tr("Front"), Config.keymap.view_front)) {
					Viewport.setView(0, -1, 0, Math.PI / 2, 0, 0);
				}
				if (UIMenu.menuButton(ui, tr("Back"), Config.keymap.view_back)) {
					Viewport.setView(0, 1, 0, Math.PI / 2, 0, Math.PI);
				}
				if (UIMenu.menuButton(ui, tr("Right"), Config.keymap.view_right)) {
					Viewport.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
				}
				if (UIMenu.menuButton(ui, tr("Left"), Config.keymap.view_left)) {
					Viewport.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
				}
				if (UIMenu.menuButton(ui, tr("Top"), Config.keymap.view_top)) {
					Viewport.setView(0, 0, 1, 0, 0, 0);
				}
				if (UIMenu.menuButton(ui, tr("Bottom"), Config.keymap.view_bottom)) {
					Viewport.setView(0, 0, -1, Math.PI, 0, Math.PI);
				}
				UIMenu.menuSeparator(ui);

				ui.changed = false;

				if (UIMenu.menuButton(ui, tr("Orbit Left"), Config.keymap.view_orbit_left)) {
					Viewport.orbit(-Math.PI / 12, 0);
				}
				if (UIMenu.menuButton(ui, tr("Orbit Right"), Config.keymap.view_orbit_right)) {
					Viewport.orbit(Math.PI / 12, 0);
				}
				if (UIMenu.menuButton(ui, tr("Orbit Up"), Config.keymap.view_orbit_up)) {
					Viewport.orbit(0, -Math.PI / 12);
				}
				if (UIMenu.menuButton(ui, tr("Orbit Down"), Config.keymap.view_orbit_down)) {
					Viewport.orbit(0, Math.PI / 12);
				}
				if (UIMenu.menuButton(ui, tr("Orbit Opposite"), Config.keymap.view_orbit_opposite)) {
					Viewport.orbitOpposite();
				}
				if (UIMenu.menuButton(ui, tr("Zoom In"), Config.keymap.view_zoom_in)) {
					Viewport.zoom(0.2);
				}
				if (UIMenu.menuButton(ui, tr("Zoom Out"), Config.keymap.view_zoom_out)) {
					Viewport.zoom(-0.2);
				}
				// menuSeparator(ui);

				UIMenu.menuFill(ui);
				let cam = scene_camera;
				Context.raw.fovHandle = zui_handle("uimenu_11", { value: Math.floor(cam.data.fov * 100) / 100 });
				UIMenu.menuAlign(ui);
				cam.data.fov = zui_slider(Context.raw.fovHandle, tr("FoV"), 0.3, 1.4, true);
				if (Context.raw.fovHandle.changed) {
					Viewport.updateCameraType(Context.raw.cameraType);
				}

				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				let cameraControlsHandle = zui_handle("uimenu_12");
				cameraControlsHandle.position = Context.raw.cameraControls;
				Context.raw.cameraControls = zui_inline_radio(cameraControlsHandle, [tr("Orbit"), tr("Rotate"), tr("Fly")], Align.Left);

				let orbitAndRotateTooltip = tr("Orbit and Rotate mode:\n{rotate_shortcut} or move right mouse button to rotate.\n{zoom_shortcut} or scroll to zoom.\n{pan_shortcut} or move middle mouse to pan.",
					new Map([
						["rotate_shortcut", Config.keymap.action_rotate],
						["zoom_shortcut", Config.keymap.action_zoom],
						["pan_shortcut", Config.keymap.action_pan]
					])
				);
				let flyTooltip = tr("Fly mode:\nHold the right mouse button and one of the following commands:\nmove mouse to rotate.\nw, up or scroll up to move forward.\ns, down or scroll down to move backward.\na or left to move left.\nd or right to move right.\ne to move up.\nq to move down.\nHold shift to move faster or alt to move slower.");
				if (ui.is_hovered) zui_tooltip(orbitAndRotateTooltip + "\n\n" + flyTooltip);

				UIMenu.menuFill(ui);
				UIMenu.menuAlign(ui);
				Context.raw.cameraType = zui_inline_radio(Context.raw.camHandle, [tr("Perspective"), tr("Orthographic")], Align.Left);
				if (ui.is_hovered) zui_tooltip(tr("Camera Type") + ` (${Config.keymap.view_camera_type})`);
				if (Context.raw.camHandle.changed) {
					Viewport.updateCameraType(Context.raw.cameraType);
				}

				if (ui.changed) UIMenu.keepOpen = true;
			}
			else if (UIMenu.menuCategory == MenuCategory.MenuHelp) {
				if (UIMenu.menuButton(ui, tr("Manual"))) {
					File.loadUrl(manifest_url + "/manual");
				}
				if (UIMenu.menuButton(ui, tr("How To"))) {
					File.loadUrl(manifest_url + "/howto");
				}
				if (UIMenu.menuButton(ui, tr("What's New"))) {
					File.loadUrl(manifest_url + "/notes");
				}
				if (UIMenu.menuButton(ui, tr("Issue Tracker"))) {
					File.loadUrl("https://github.com/armory3d/armortools/issues");
				}
				if (UIMenu.menuButton(ui, tr("Report Bug"))) {
					///if (krom_darwin || krom_ios) // Limited url length
					File.loadUrl("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.getSha() + ",%20" + sys_system_id());
					///else
					File.loadUrl("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.getSha() + ",%20" + sys_system_id() + "*%0A%0A**Issue description:**%0A%0A**Steps to reproduce:**%0A%0A");
					///end
				}
				if (UIMenu.menuButton(ui, tr("Request Feature"))) {
					///if (krom_darwin || krom_ios) // Limited url length
					File.loadUrl("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.getSha() + ",%20" + sys_system_id());
					///else
					File.loadUrl("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.getSha() + ",%20" + sys_system_id() + "*%0A%0A**Feature description:**%0A%0A");
					///end
				}
				UIMenu.menuSeparator(ui);

				if (UIMenu.menuButton(ui, tr("Check for Updates..."))) {
					///if krom_android
					File.loadUrl(manifest_url_android);
					///elseif krom_ios
					File.loadUrl(manifest_url_ios);
					///else
					// Retrieve latest version number
					File.downloadBytes("https://server.armorpaint.org/" + manifest_title.toLowerCase() + ".html", (buffer: ArrayBuffer) => {
						if (buffer != null)  {
							// Compare versions
							let update = JSON.parse(sys_buffer_to_string(buffer));
							let updateVersion = Math.floor(update.version);
							if (updateVersion > 0) {
								let date = Config.getDate().substr(2); // 2019 -> 19
								let dateInt = parseInt(date.replaceAll("-", ""));
								if (updateVersion > dateInt) {
									UIBox.showMessage(tr("Update"), tr("Update is available!\nPlease visit {url}.", new Map([["url", manifest_url]])));
								}
								else {
									UIBox.showMessage(tr("Update"), tr("You are up to date!"));
								}
							}
						}
						else {
							UIBox.showMessage(tr("Update"), tr("Unable to check for updates.\nPlease visit {url}.", new Map([["url", manifest_url]])));
						}
					});
					///end
				}

				if (UIMenu.menuButton(ui, tr("About..."))) {

					let msg = manifest_title + ".org - v" + manifest_version + " (" + Config.getDate() + ") - " + Config.getSha() + "\n";
					msg += sys_system_id() + " - " + Strings.graphics_api;

					///if krom_windows
					let save = (Path.isProtected() ? krom_save_path() : Path.data()) + Path.sep + "tmp.txt";
					krom_sys_command('wmic path win32_VideoController get name > "' + save + '"');
					let blob = krom_load_blob(save);
					let u8 = new Uint8Array(blob);
					let gpuRaw = "";
					for (let i = 0; i < Math.floor(u8.length / 2); ++i) {
						let c = String.fromCharCode(u8[i * 2]);
						gpuRaw += c;
					}

					let gpus = gpuRaw.split("\n");
					gpus = gpus.splice(1, gpus.length - 2);
					let gpu = "";
					for (let g of gpus) {
						gpu += trim_end(g) + ", ";
					}
					gpu = gpu.substr(0, gpu.length - 2);
					msg += `\n${gpu}`;
					///else
					// { lshw -C display }
					///end

					UIBox.showCustom((ui: zui_t) => {
						let tabVertical = Config.raw.touch_ui;
						if (zui_tab(zui_handle("uimenu_13"), tr("About"), tabVertical)) {

							data_get_image("badge.k", (img: image_t) => {
								zui_image(img);
								zui_end_element();
							});

							zui_text_area(zui_handle("uimenu_14", { text: msg }), Align.Left, false);

							zui_row([1 / 3, 1 / 3, 1 / 3]);

							///if (krom_windows || krom_linux || krom_darwin)
							if (zui_button(tr("Copy"))) {
								krom_copy_to_clipboard(msg);
							}
							///else
							zui_end_element();
							///end

							if (zui_button(tr("Contributors"))) {
								File.loadUrl("https://github.com/armory3d/armortools/graphs/contributors");
							}
							if (zui_button(tr("OK"))) {
								UIBox.hide();
							}
						}
					}, 400, 320);
				}
			}
		}

		UIMenu.hideMenu = ui.combo_selected_handle_ptr == null && !UIMenu.keepOpen && !UIMenu.showMenuFirst && (ui.changed || ui.input_released || ui.input_released_r || ui.is_escape_down);
		UIMenu.showMenuFirst = false;
		UIMenu.keepOpen = false;

		ui.t.BUTTON_COL = _BUTTON_COL;
		ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
		ui.t.ELEMENT_H = _ELEMENT_H;
		zui_end_region();

		if (UIMenu.hideMenu) {
			UIMenu.hide();
			UIMenu.showMenuFirst = true;
			UIMenu.menuCommands = null;
		}
	}

	static hide = () => {
		UIMenu.show = false;
		Base.redrawUI();
	}

	static draw = (commands: (ui: zui_t)=>void = null, elements: i32, x = -1, y = -1) => {
		zui_end_input();
		UIMenu.show = true;
		UIMenu.menuCommands = commands;
		UIMenu.menuElements = elements;
		UIMenu.menuX = x > -1 ? x : Math.floor(mouse_x + 1);
		UIMenu.menuY = y > -1 ? y : Math.floor(mouse_y + 1);
		UIMenu.fitToScreen();
	}

	static fitToScreen = () => {
		// Prevent the menu going out of screen
		let menuW = Base.defaultElementW * zui_SCALE(Base.uiMenu) * 2.3;
		if (UIMenu.menuX + menuW > sys_width()) {
			if (UIMenu.menuX - menuW > 0) {
				UIMenu.menuX = Math.floor(UIMenu.menuX - menuW);
			}
			else {
				UIMenu.menuX = Math.floor(sys_width() - menuW);
			}
		}
		let menuH = Math.floor(UIMenu.menuElements * 30 * zui_SCALE(Base.uiMenu)); // ui.t.ELEMENT_H
		if (UIMenu.menuY + menuH > sys_height()) {
			if (UIMenu.menuY - menuH > 0) {
				UIMenu.menuY = Math.floor(UIMenu.menuY - menuH);
			}
			else {
				UIMenu.menuY = sys_height() - menuH;
			}
			UIMenu.menuX += 1; // Move out of mouse focus
		}
	}

	static menuFill = (ui: zui_t) => {
		g2_set_color(ui.t.ACCENT_SELECT_COL);
		g2_fill_rect(ui._x - 1, ui._y, ui._w + 2, zui_ELEMENT_H(ui) + 1 + 1);
		g2_set_color(ui.t.SEPARATOR_COL);
		g2_fill_rect(ui._x, ui._y, ui._w, zui_ELEMENT_H(ui) + 1);
		g2_set_color(0xffffffff);
	}

	static menuSeparator = (ui: zui_t) => {
		ui._y++;
		if (Config.raw.touch_ui) {
			zui_fill(0, 0, ui._w / zui_SCALE(ui), 1, ui.t.ACCENT_SELECT_COL);
		}
		else {
			zui_fill(26, 0, ui._w / zui_SCALE(ui) - 26, 1, ui.t.ACCENT_SELECT_COL);
		}
	}

	static menuButton = (ui: zui_t, text: string, label = ""/*, icon = -1*/): bool => {
		UIMenu.menuFill(ui);
		if (Config.raw.touch_ui) {
			label = "";
		}

		// let icons = icon > -1 ? Res.get("icons.k") : null;
		// let r = Res.tile25(icons, icon, 8);
		// return Zui.button(Config.buttonSpacing + text, Config.buttonAlign, label, icons, r.x, r.y, r.w, r.h);

		return zui_button(Config.buttonSpacing + text, Config.buttonAlign, label);
	}

	static menuAlign = (ui: zui_t) => {
		if (!Config.raw.touch_ui) {
			zui_row([12 / 100, 88 / 100]);
			zui_end_element();
		}
	}

	static menuStart = (ui: zui_t) => {
		// Draw top border
		g2_set_color(ui.t.ACCENT_SELECT_COL);
		if (Config.raw.touch_ui) {
			g2_fill_rect(ui._x + ui._w / 2 + UIMenu.menuCategoryW / 2, ui._y - 1, ui._w / 2 - UIMenu.menuCategoryW / 2 + 1, 1);
			g2_fill_rect(ui._x - 1, ui._y - 1, ui._w / 2 - UIMenu.menuCategoryW / 2 + 1, 1);
			g2_fill_rect(ui._x + ui._w / 2 - UIMenu.menuCategoryW / 2, ui._y - UIMenu.menuCategoryH, UIMenu.menuCategoryW, 1);
			g2_fill_rect(ui._x + ui._w / 2 - UIMenu.menuCategoryW / 2, ui._y - UIMenu.menuCategoryH, 1, UIMenu.menuCategoryH);
			g2_fill_rect(ui._x + ui._w / 2 + UIMenu.menuCategoryW / 2, ui._y - UIMenu.menuCategoryH, 1, UIMenu.menuCategoryH);
		}
		else {
			g2_fill_rect(ui._x - 1 + UIMenu.menuCategoryW, ui._y - 1, ui._w + 2 - UIMenu.menuCategoryW, 1);
			g2_fill_rect(ui._x - 1, ui._y - UIMenu.menuCategoryH, UIMenu.menuCategoryW, 1);
			g2_fill_rect(ui._x - 1, ui._y - UIMenu.menuCategoryH, 1, UIMenu.menuCategoryH);
			g2_fill_rect(ui._x - 1 + UIMenu.menuCategoryW, ui._y - UIMenu.menuCategoryH, 1, UIMenu.menuCategoryH);
		}
		g2_set_color(0xffffffff);
	}
}
