
class UIMenu {

	static show: bool = false;
	static menu_category: i32 = 0;
	static menu_category_w: i32 = 0;
	static menu_category_h: i32 = 0;
	static menu_x: i32 = 0;
	static menu_y: i32 = 0;
	static menu_elements: i32 = 0;
	static keep_open: bool = false;
	static menu_commands: (ui: zui_t)=>void = null;
	static show_menu_first: bool = true;
	static hide_menu: bool = false;

	static render = () => {
		let ui: zui_t = base_ui_menu;
		let menu_w: i32 = UIMenu.menu_commands != null ? Math.floor(base_default_element_w * zui_SCALE(base_ui_menu) * 2.3) : Math.floor(zui_ELEMENT_W(ui) * 2.3);
		let _BUTTON_COL: i32 = ui.t.BUTTON_COL;
		ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;
		let _ELEMENT_OFFSET: i32 = ui.t.ELEMENT_OFFSET;
		ui.t.ELEMENT_OFFSET = 0;
		let _ELEMENT_H: i32 = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Config.raw.touch_ui ? (28 + 2) : 28;

		zui_begin_region(ui, UIMenu.menu_x, UIMenu.menu_y, menu_w);

		if (UIMenu.menu_commands != null) {
			g2_set_color(ui.t.ACCENT_SELECT_COL);
			zui_draw_rect(true, ui._x + -1, ui._y + -1, ui._w + 2, zui_ELEMENT_H(ui) * UIMenu.menu_elements + 2);
			g2_set_color(ui.t.SEPARATOR_COL);
			zui_draw_rect(true, ui._x + 0, ui._y + 0, ui._w, zui_ELEMENT_H(ui) * UIMenu.menu_elements);
			g2_set_color(0xffffffff);

			UIMenu.menu_commands(ui);
		}
		else {
			UIMenu.menu_start(ui);
			if (UIMenu.menu_category == menu_category_t.FILE) {
				if (UIMenu.menu_button(ui, tr("New Project..."), Config.keymap.file_new)) Project.project_new_box();
				if (UIMenu.menu_button(ui, tr("Open..."), Config.keymap.file_open)) Project.project_open();
				if (UIMenu.menu_button(ui, tr("Open Recent..."), Config.keymap.file_open_recent)) BoxProjects.show();
				if (UIMenu.menu_button(ui, tr("Save"), Config.keymap.file_save)) Project.project_save();
				if (UIMenu.menu_button(ui, tr("Save As..."), Config.keymap.file_save_as)) Project.project_save_as();
				UIMenu.menu_separator(ui);
				if (UIMenu.menu_button(ui, tr("Import Texture..."), Config.keymap.file_import_assets)) Project.import_asset(Path.texture_formats.join(","), false);
				if (UIMenu.menu_button(ui, tr("Import Envmap..."))) {
					UIFiles.show("hdr", false, false, (path: string) => {
						if (!path.endsWith(".hdr")) {
							Console.error(tr("Error: .hdr file expected"));
							return;
						}
						ImportAsset.run(path);
					});
				}

				///if (is_paint || is_sculpt)
				if (UIMenu.menu_button(ui, tr("Import Font..."))) Project.import_asset("ttf,ttc,otf");
				if (UIMenu.menu_button(ui, tr("Import Material..."))) Project.import_material();
				if (UIMenu.menu_button(ui, tr("Import Brush..."))) Project.import_brush();
				///end

				///if (is_paint || is_lab)
				if (UIMenu.menu_button(ui, tr("Import Swatches..."))) Project.import_swatches();
				///end
				if (UIMenu.menu_button(ui, tr("Import Mesh..."))) Project.import_mesh();
				if (UIMenu.menu_button(ui, tr("Reimport Mesh"), Config.keymap.file_reimport_mesh)) Project.reimport_mesh();
				if (UIMenu.menu_button(ui, tr("Reimport Textures"), Config.keymap.file_reimport_textures)) Project.reimport_textures();
				UIMenu.menu_separator(ui);
				///if (is_paint || is_lab)
				if (UIMenu.menu_button(ui, tr("Export Textures..."), Config.keymap.file_export_textures_as)) {
					///if is_paint
					Context.raw.layers_export = export_mode_t.VISIBLE;
					///end
					BoxExport.show_textures();
				}
				if (UIMenu.menu_button(ui, tr("Export Swatches..."))) Project.export_swatches();
				///end
				if (UIMenu.menu_button(ui, tr("Export Mesh..."))) {
					Context.raw.export_mesh_index = 0; // All
					BoxExport.show_mesh();
				}

				///if is_paint
				if (UIMenu.menu_button(ui, tr("Bake Material..."))) BoxExport.show_bake_material();
				///end

				UIMenu.menu_separator(ui);
				if (UIMenu.menu_button(ui, tr("Exit"))) sys_stop();
			}
			else if (UIMenu.menu_category == menu_category_t.EDIT) {
				let step_undo: string = "";
				let step_redo: string = "";
				if (History.undos > 0) {
					step_undo = History.steps[History.steps.length - 1 - History.redos].name;
				}
				if (History.redos > 0) {
					step_redo = History.steps[History.steps.length - History.redos].name;
				}
				ui.enabled = History.undos > 0;
				if (UIMenu.menu_button(ui, tr("Undo {step}", new Map([["step", step_undo]])), Config.keymap.edit_undo)) History.undo();
				ui.enabled = History.redos > 0;
				if (UIMenu.menu_button(ui, tr("Redo {step}", new Map([["step", step_redo]])), Config.keymap.edit_redo)) History.redo();
				ui.enabled = true;
				UIMenu.menu_separator(ui);
				if (UIMenu.menu_button(ui, tr("Preferences..."), Config.keymap.edit_prefs)) BoxPreferences.show();
			}
			else if (UIMenu.menu_category == menu_category_t.VIEWPORT) {
				if (UIMenu.menu_button(ui, tr("Distract Free"), Config.keymap.view_distract_free)) {
					UIBase.toggle_distract_free();
					UIBase.ui.is_hovered = false;
				}

				///if !(krom_android || krom_ios)
				if (UIMenu.menu_button(ui, tr("Toggle Fullscreen"), "alt+enter")) {
					base_toggle_fullscreen();
				}
				///end

				ui.changed = false;

				UIMenu.menu_fill(ui);
				let p: world_data_t = scene_world;
				let env_handle: zui_handle_t = zui_handle("uimenu_0");
				env_handle.value = p.strength;
				UIMenu.menu_align(ui);
				p.strength = zui_slider(env_handle, tr("Environment"), 0.0, 8.0, true);
				if (env_handle.changed) Context.raw.ddirty = 2;

				UIMenu.menu_fill(ui);
				let enva_handle: zui_handle_t = zui_handle("uimenu_1");
				enva_handle.value = Context.raw.envmap_angle / Math.PI * 180.0;
				if (enva_handle.value < 0) {
					enva_handle.value += (Math.floor(-enva_handle.value / 360) + 1) * 360;
				}
				else if (enva_handle.value > 360) {
					enva_handle.value -= Math.floor(enva_handle.value / 360) * 360;
				}
				UIMenu.menu_align(ui);
				Context.raw.envmap_angle = zui_slider(enva_handle, tr("Environment Angle"), 0.0, 360.0, true, 1) / 180.0 * Math.PI;
				if (ui.is_hovered) zui_tooltip(tr("{shortcut} and move mouse", new Map([["shortcut", Config.keymap.rotate_envmap]])));
				if (enva_handle.changed) Context.raw.ddirty = 2;

				if (scene_lights.length > 0) {
					let light: light_object_t = scene_lights[0];

					UIMenu.menu_fill(ui);
					let lhandle: zui_handle_t = zui_handle("uimenu_2");
					let scale: f32 = 1333;
					lhandle.value = light.data.strength / scale;
					lhandle.value = Math.floor(lhandle.value * 100) / 100;
					UIMenu.menu_align(ui);
					light.data.strength = zui_slider(lhandle, tr("Light"), 0.0, 4.0, true) * scale;
					if (lhandle.changed) Context.raw.ddirty = 2;

					UIMenu.menu_fill(ui);
					light = scene_lights[0];
					let lahandle: zui_handle_t = zui_handle("uimenu_3");
					lahandle.value = Context.raw.light_angle / Math.PI * 180;
					UIMenu.menu_align(ui);
					let new_angle: f32 = zui_slider(lahandle, tr("Light Angle"), 0.0, 360.0, true, 1) / 180 * Math.PI;
					if (ui.is_hovered) zui_tooltip(tr("{shortcut} and move mouse", new Map([["shortcut", Config.keymap.rotate_light]])));
					let ldiff: f32 = new_angle - Context.raw.light_angle;
					if (Math.abs(ldiff) > 0.005) {
						if (new_angle < 0) new_angle += (Math.floor(-new_angle / (2 * Math.PI)) + 1) * 2 * Math.PI;
						else if (new_angle > 2 * Math.PI) new_angle -= Math.floor(new_angle / (2 * Math.PI)) * 2 * Math.PI;
						Context.raw.light_angle = new_angle;
						let m: mat4_t = mat4_rot_z(ldiff);
						mat4_mult_mat(light.base.transform.local, m);
						transform_decompose(light.base.transform);
						Context.raw.ddirty = 2;
					}

					UIMenu.menu_fill(ui);
					let sxhandle: zui_handle_t = zui_handle("uimenu_4");
					sxhandle.value = light.data.size;
					UIMenu.menu_align(ui);
					light.data.size = zui_slider(sxhandle, tr("Light Size"), 0.0, 4.0, true);
					if (sxhandle.changed) Context.raw.ddirty = 2;
				}

				///if (is_paint || is_sculpt)
				UIMenu.menu_fill(ui);
				let split_view_handle: zui_handle_t = zui_handle("uimenu_5", { selected: Context.raw.split_view });
				Context.raw.split_view = zui_check(split_view_handle, " " + tr("Split View"));
				if (split_view_handle.changed) {
					base_resize();
				}
				///end

				///if is_lab
				UIMenu.menu_fill(ui);
				let brush_scale_handle: zui_handle_t = zui_handle("uimenu_6", { value: Context.raw.brush_scale });
				UIMenu.menu_align(ui);
				Context.raw.brush_scale = zui_slider(brush_scale_handle, tr("UV Scale"), 0.01, 5.0, true);
				if (brush_scale_handle.changed) {
					MakeMaterial.parse_mesh_material();
					///if (krom_direct3d12 || krom_vulkan || krom_metal)
					RenderPathRaytrace.uv_scale = Context.raw.brush_scale;
					RenderPathRaytrace.ready = false;
					///end
				}
				///end

				UIMenu.menu_fill(ui);
				let cull_handle: zui_handle_t = zui_handle("uimenu_7", { selected: Context.raw.cull_backfaces });
				Context.raw.cull_backfaces = zui_check(cull_handle, " " + tr("Cull Backfaces"));
				if (cull_handle.changed) {
					MakeMaterial.parse_mesh_material();
				}

				UIMenu.menu_fill(ui);
				let filter_handle: zui_handle_t = zui_handle("uimenu_8", { selected: Context.raw.texture_filter });
				Context.raw.texture_filter = zui_check(filter_handle, " " + tr("Filter Textures"));
				if (filter_handle.changed) {
					MakeMaterial.parse_paint_material();
					MakeMaterial.parse_mesh_material();
				}

				///if (is_paint || is_sculpt)
				UIMenu.menu_fill(ui);
				Context.raw.draw_wireframe = zui_check(Context.raw.wireframe_handle, " " + tr("Wireframe"));
				if (Context.raw.wireframe_handle.changed) {
					let current: image_t = _g2_current;
					g2_end();
					UtilUV.cache_uv_map();
					g2_begin(current);
					MakeMaterial.parse_mesh_material();
				}
				///end

				///if is_paint
				UIMenu.menu_fill(ui);
				Context.raw.draw_texels = zui_check(Context.raw.texels_handle, " " + tr("Texels"));
				if (Context.raw.texels_handle.changed) {
					MakeMaterial.parse_mesh_material();
				}
				///end

				UIMenu.menu_fill(ui);
				let compass_handle: zui_handle_t = zui_handle("uimenu_9", { selected: Context.raw.show_compass });
				Context.raw.show_compass = zui_check(compass_handle, " " + tr("Compass"));
				if (compass_handle.changed) Context.raw.ddirty = 2;

				UIMenu.menu_fill(ui);
				Context.raw.show_envmap = zui_check(Context.raw.show_envmap_handle, " " + tr("Envmap"));
				if (Context.raw.show_envmap_handle.changed) {
					Context.load_envmap();
					Context.raw.ddirty = 2;
				}

				UIMenu.menu_fill(ui);
				Context.raw.show_envmap_blur = zui_check(Context.raw.show_envmap_blur_handle, " " + tr("Blur Envmap"));
				if (Context.raw.show_envmap_blur_handle.changed) Context.raw.ddirty = 2;

				Context.update_envmap();

				if (ui.changed) UIMenu.keep_open = true;
			}
			else if (UIMenu.menu_category == menu_category_t.MODE) {
				let mode_handle: zui_handle_t = zui_handle("uimenu_10");
				mode_handle.position = Context.raw.viewport_mode;
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

				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				if (krom_raytrace_supported()) {
					modes.push(tr("Path Traced"));
					shortcuts.push("p");
				}
				///end

				for (let i: i32 = 0; i < modes.length; ++i) {
					UIMenu.menu_fill(ui);
					let shortcut: string = Config.raw.touch_ui ? "" : Config.keymap.viewport_mode + ", " + shortcuts[i];
					zui_radio(mode_handle, i, modes[i], shortcut);
				}

				if (mode_handle.changed) {
					Context.set_viewport_mode(mode_handle.position);
					// TODO: rotate mode is not supported for path tracing yet
					if (mode_handle.position == viewport_mode_t.PATH_TRACE && Context.raw.camera_controls == camera_controls_t.ROTATE) {
						Context.raw.camera_controls = camera_controls_t.ORBIT;
						Viewport.reset();
					}
				}
			}
			else if (UIMenu.menu_category == menu_category_t.CAMERA) {
				if (UIMenu.menu_button(ui, tr("Reset"), Config.keymap.view_reset)) {
					Viewport.reset();
					Viewport.scale_to_bounds();
				}
				UIMenu.menu_separator(ui);
				if (UIMenu.menu_button(ui, tr("Front"), Config.keymap.view_front)) {
					Viewport.set_view(0, -1, 0, Math.PI / 2, 0, 0);
				}
				if (UIMenu.menu_button(ui, tr("Back"), Config.keymap.view_back)) {
					Viewport.set_view(0, 1, 0, Math.PI / 2, 0, Math.PI);
				}
				if (UIMenu.menu_button(ui, tr("Right"), Config.keymap.view_right)) {
					Viewport.set_view(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
				}
				if (UIMenu.menu_button(ui, tr("Left"), Config.keymap.view_left)) {
					Viewport.set_view(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
				}
				if (UIMenu.menu_button(ui, tr("Top"), Config.keymap.view_top)) {
					Viewport.set_view(0, 0, 1, 0, 0, 0);
				}
				if (UIMenu.menu_button(ui, tr("Bottom"), Config.keymap.view_bottom)) {
					Viewport.set_view(0, 0, -1, Math.PI, 0, Math.PI);
				}
				UIMenu.menu_separator(ui);

				ui.changed = false;

				if (UIMenu.menu_button(ui, tr("Orbit Left"), Config.keymap.view_orbit_left)) {
					Viewport.orbit(-Math.PI / 12, 0);
				}
				if (UIMenu.menu_button(ui, tr("Orbit Right"), Config.keymap.view_orbit_right)) {
					Viewport.orbit(Math.PI / 12, 0);
				}
				if (UIMenu.menu_button(ui, tr("Orbit Up"), Config.keymap.view_orbit_up)) {
					Viewport.orbit(0, -Math.PI / 12);
				}
				if (UIMenu.menu_button(ui, tr("Orbit Down"), Config.keymap.view_orbit_down)) {
					Viewport.orbit(0, Math.PI / 12);
				}
				if (UIMenu.menu_button(ui, tr("Orbit Opposite"), Config.keymap.view_orbit_opposite)) {
					Viewport.orbit_opposite();
				}
				if (UIMenu.menu_button(ui, tr("Zoom In"), Config.keymap.view_zoom_in)) {
					Viewport.zoom(0.2);
				}
				if (UIMenu.menu_button(ui, tr("Zoom Out"), Config.keymap.view_zoom_out)) {
					Viewport.zoom(-0.2);
				}
				// menuSeparator(ui);

				UIMenu.menu_fill(ui);
				let cam: camera_object_t = scene_camera;
				Context.raw.fov_handle = zui_handle("uimenu_11", { value: Math.floor(cam.data.fov * 100) / 100 });
				UIMenu.menu_align(ui);
				cam.data.fov = zui_slider(Context.raw.fov_handle, tr("FoV"), 0.3, 1.4, true);
				if (Context.raw.fov_handle.changed) {
					Viewport.update_camera_type(Context.raw.camera_type);
				}

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let camera_controls_handle: zui_handle_t = zui_handle("uimenu_12");
				camera_controls_handle.position = Context.raw.camera_controls;
				Context.raw.camera_controls = zui_inline_radio(camera_controls_handle, [tr("Orbit"), tr("Rotate"), tr("Fly")], zui_align_t.LEFT);

				let orbit_and_rotate_tooltip: string = tr("Orbit and Rotate mode:\n{rotate_shortcut} or move right mouse button to rotate.\n{zoom_shortcut} or scroll to zoom.\n{pan_shortcut} or move middle mouse to pan.",
					new Map([
						["rotate_shortcut", Config.keymap.action_rotate],
						["zoom_shortcut", Config.keymap.action_zoom],
						["pan_shortcut", Config.keymap.action_pan]
					])
				);
				let fly_tooltip: string = tr("Fly mode:\nHold the right mouse button and one of the following commands:\nmove mouse to rotate.\nw, up or scroll up to move forward.\ns, down or scroll down to move backward.\na or left to move left.\nd or right to move right.\ne to move up.\nq to move down.\nHold shift to move faster or alt to move slower.");
				if (ui.is_hovered) zui_tooltip(orbit_and_rotate_tooltip + "\n\n" + fly_tooltip);

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				Context.raw.camera_type = zui_inline_radio(Context.raw.cam_handle, [tr("Perspective"), tr("Orthographic")], zui_align_t.LEFT);
				if (ui.is_hovered) zui_tooltip(tr("Camera Type") + ` (${Config.keymap.view_camera_type})`);
				if (Context.raw.cam_handle.changed) {
					Viewport.update_camera_type(Context.raw.camera_type);
				}

				if (ui.changed) UIMenu.keep_open = true;
			}
			else if (UIMenu.menu_category == menu_category_t.HELP) {
				if (UIMenu.menu_button(ui, tr("Manual"))) {
					File.load_url(manifest_url + "/manual");
				}
				if (UIMenu.menu_button(ui, tr("How To"))) {
					File.load_url(manifest_url + "/howto");
				}
				if (UIMenu.menu_button(ui, tr("What's New"))) {
					File.load_url(manifest_url + "/notes");
				}
				if (UIMenu.menu_button(ui, tr("Issue Tracker"))) {
					File.load_url("https://github.com/armory3d/armortools/issues");
				}
				if (UIMenu.menu_button(ui, tr("Report Bug"))) {
					///if (krom_darwin || krom_ios) // Limited url length
					File.load_url("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.get_sha() + ",%20" + sys_system_id());
					///else
					File.load_url("https://github.com/armory3d/armortools/issues/new?labels=bug&template=bug_report.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.get_sha() + ",%20" + sys_system_id() + "*%0A%0A**Issue description:**%0A%0A**Steps to reproduce:**%0A%0A");
					///end
				}
				if (UIMenu.menu_button(ui, tr("Request Feature"))) {
					///if (krom_darwin || krom_ios) // Limited url length
					File.load_url("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.get_sha() + ",%20" + sys_system_id());
					///else
					File.load_url("https://github.com/armory3d/armortools/issues/new?labels=feature%20request&template=feature_request.md&body=*" + manifest_title + "%20" + manifest_version + "-" + Config.get_sha() + ",%20" + sys_system_id() + "*%0A%0A**Feature description:**%0A%0A");
					///end
				}
				UIMenu.menu_separator(ui);

				if (UIMenu.menu_button(ui, tr("Check for Updates..."))) {
					///if krom_android
					File.load_url(manifest_url_android);
					///elseif krom_ios
					File.load_url(manifest_url_ios);
					///else
					// Retrieve latest version number
					File.download_bytes("https://server.armorpaint.org/" + manifest_title.toLowerCase() + ".html", (buffer: ArrayBuffer) => {
						if (buffer != null)  {
							// Compare versions
							let update: any = json_parse(sys_buffer_to_string(buffer));
							let update_version: i32 = Math.floor(update.version);
							if (update_version > 0) {
								let date: string = Config.get_date().substr(2); // 2019 -> 19
								let date_int: i32 = parseInt(string_replace_all(date, "-", ""));
								if (update_version > date_int) {
									UIBox.show_message(tr("Update"), tr("Update is available!\nPlease visit {url}.", new Map([["url", manifest_url]])));
								}
								else {
									UIBox.show_message(tr("Update"), tr("You are up to date!"));
								}
							}
						}
						else {
							UIBox.show_message(tr("Update"), tr("Unable to check for updates.\nPlease visit {url}.", new Map([["url", manifest_url]])));
						}
					});
					///end
				}

				if (UIMenu.menu_button(ui, tr("About..."))) {

					let msg: string = manifest_title + ".org - v" + manifest_version + " (" + Config.get_date() + ") - " + Config.get_sha() + "\n";
					msg += sys_system_id() + " - " + Strings.graphics_api;

					///if krom_windows
					let save: string = (Path.is_protected() ? krom_save_path() : Path.data()) + Path.sep + "tmp.txt";
					krom_sys_command('wmic path win32_VideoController get name > "' + save + '"');
					let blob: buffer_t = krom_load_blob(save);
					let u8: Uint8Array = new Uint8Array(blob);
					let gpu_raw: string = "";
					for (let i: i32 = 0; i < Math.floor(u8.length / 2); ++i) {
						let c: string = String.fromCharCode(u8[i * 2]);
						gpu_raw += c;
					}

					let gpus: string[] = gpu_raw.split("\n");
					gpus = gpus.splice(1, gpus.length - 2);
					let gpu: string = "";
					for (let g of gpus) {
						gpu += trim_end(g) + ", ";
					}
					gpu = gpu.substr(0, gpu.length - 2);
					msg += `\n${gpu}`;
					///else
					// { lshw -C display }
					///end

					UIBox.show_custom((ui: zui_t) => {
						let tab_vertical: bool = Config.raw.touch_ui;
						if (zui_tab(zui_handle("uimenu_13"), tr("About"), tab_vertical)) {

							let img: image_t = data_get_image("badge.k");
							zui_image(img);
							zui_end_element();

							zui_text_area(zui_handle("uimenu_14", { text: msg }), zui_align_t.LEFT, false);

							zui_row([1 / 3, 1 / 3, 1 / 3]);

							///if (krom_windows || krom_linux || krom_darwin)
							if (zui_button(tr("Copy"))) {
								krom_copy_to_clipboard(msg);
							}
							///else
							zui_end_element();
							///end

							if (zui_button(tr("Contributors"))) {
								File.load_url("https://github.com/armory3d/armortools/graphs/contributors");
							}
							if (zui_button(tr("OK"))) {
								UIBox.hide();
							}
						}
					}, 400, 320);
				}
			}
		}

		UIMenu.hide_menu = ui.combo_selected_handle_ptr == 0 && !UIMenu.keep_open && !UIMenu.show_menu_first && (ui.changed || ui.input_released || ui.input_released_r || ui.is_escape_down);
		UIMenu.show_menu_first = false;
		UIMenu.keep_open = false;

		ui.t.BUTTON_COL = _BUTTON_COL;
		ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
		ui.t.ELEMENT_H = _ELEMENT_H;
		zui_end_region();

		if (UIMenu.hide_menu) {
			UIMenu.hide();
			UIMenu.show_menu_first = true;
			UIMenu.menu_commands = null;
		}
	}

	static hide = () => {
		UIMenu.show = false;
		base_redraw_ui();
	}

	static draw = (commands: (ui: zui_t)=>void = null, elements: i32, x: i32 = -1, y: i32 = -1) => {
		zui_end_input();
		UIMenu.show = true;
		UIMenu.menu_commands = commands;
		UIMenu.menu_elements = elements;
		UIMenu.menu_x = x > -1 ? x : Math.floor(mouse_x + 1);
		UIMenu.menu_y = y > -1 ? y : Math.floor(mouse_y + 1);
		UIMenu.fit_to_screen();
	}

	static fit_to_screen = () => {
		// Prevent the menu going out of screen
		let menu_w: f32 = base_default_element_w * zui_SCALE(base_ui_menu) * 2.3;
		if (UIMenu.menu_x + menu_w > sys_width()) {
			if (UIMenu.menu_x - menu_w > 0) {
				UIMenu.menu_x = Math.floor(UIMenu.menu_x - menu_w);
			}
			else {
				UIMenu.menu_x = Math.floor(sys_width() - menu_w);
			}
		}
		let menu_h: f32 = Math.floor(UIMenu.menu_elements * 30 * zui_SCALE(base_ui_menu)); // ui.t.ELEMENT_H
		if (UIMenu.menu_y + menu_h > sys_height()) {
			if (UIMenu.menu_y - menu_h > 0) {
				UIMenu.menu_y = Math.floor(UIMenu.menu_y - menu_h);
			}
			else {
				UIMenu.menu_y = sys_height() - menu_h;
			}
			UIMenu.menu_x += 1; // Move out of mouse focus
		}
	}

	static menu_fill = (ui: zui_t) => {
		g2_set_color(ui.t.ACCENT_SELECT_COL);
		g2_fill_rect(ui._x - 1, ui._y, ui._w + 2, zui_ELEMENT_H(ui) + 1 + 1);
		g2_set_color(ui.t.SEPARATOR_COL);
		g2_fill_rect(ui._x, ui._y, ui._w, zui_ELEMENT_H(ui) + 1);
		g2_set_color(0xffffffff);
	}

	static menu_separator = (ui: zui_t) => {
		ui._y++;
		if (Config.raw.touch_ui) {
			zui_fill(0, 0, ui._w / zui_SCALE(ui), 1, ui.t.ACCENT_SELECT_COL);
		}
		else {
			zui_fill(26, 0, ui._w / zui_SCALE(ui) - 26, 1, ui.t.ACCENT_SELECT_COL);
		}
	}

	static menu_button = (ui: zui_t, text: string, label: string = ""/*, icon: i32 = -1*/): bool => {
		UIMenu.menu_fill(ui);
		if (Config.raw.touch_ui) {
			label = "";
		}

		// let icons: image_t = icon > -1 ? Res.get("icons.k") : null;
		// let r: rect_t = Res.tile25(icons, icon, 8);
		// return Zui.button(Config.buttonSpacing + text, Config.buttonAlign, label, icons, r.x, r.y, r.w, r.h);

		return zui_button(Config.button_spacing + text, Config.button_align, label);
	}

	static menu_align = (ui: zui_t) => {
		if (!Config.raw.touch_ui) {
			zui_row([12 / 100, 88 / 100]);
			zui_end_element();
		}
	}

	static menu_start = (ui: zui_t) => {
		// Draw top border
		g2_set_color(ui.t.ACCENT_SELECT_COL);
		if (Config.raw.touch_ui) {
			g2_fill_rect(ui._x + ui._w / 2 + UIMenu.menu_category_w / 2, ui._y - 1, ui._w / 2 - UIMenu.menu_category_w / 2 + 1, 1);
			g2_fill_rect(ui._x - 1, ui._y - 1, ui._w / 2 - UIMenu.menu_category_w / 2 + 1, 1);
			g2_fill_rect(ui._x + ui._w / 2 - UIMenu.menu_category_w / 2, ui._y - UIMenu.menu_category_h, UIMenu.menu_category_w, 1);
			g2_fill_rect(ui._x + ui._w / 2 - UIMenu.menu_category_w / 2, ui._y - UIMenu.menu_category_h, 1, UIMenu.menu_category_h);
			g2_fill_rect(ui._x + ui._w / 2 + UIMenu.menu_category_w / 2, ui._y - UIMenu.menu_category_h, 1, UIMenu.menu_category_h);
		}
		else {
			g2_fill_rect(ui._x - 1 + UIMenu.menu_category_w, ui._y - 1, ui._w + 2 - UIMenu.menu_category_w, 1);
			g2_fill_rect(ui._x - 1, ui._y - UIMenu.menu_category_h, UIMenu.menu_category_w, 1);
			g2_fill_rect(ui._x - 1, ui._y - UIMenu.menu_category_h, 1, UIMenu.menu_category_h);
			g2_fill_rect(ui._x - 1 + UIMenu.menu_category_w, ui._y - UIMenu.menu_category_h, 1, UIMenu.menu_category_h);
		}
		g2_set_color(0xffffffff);
	}
}
