
class UIFiles {

	static default_path: string =
		///if krom_windows
		"C:\\Users"
		///elseif krom_android
		"/storage/emulated/0/Download"
		///elseif krom_darwin
		"/Users"
		///else
		"/"
		///end
	;

	static filename: string;
	static path: string = UIFiles.default_path;
	static last_path: string = "";
	static last_search: string = "";
	static files: string[] = null;
	static icon_map: Map<string, image_t> = null;
	static selected: i32 = -1;
	static show_extensions: bool = false;
	static offline: bool = false;

	static show = (filters: string, isSave: bool, openMultiple: bool, filesDone: (s: string)=>void) => {
		if (isSave) {
			UIFiles.path = krom_save_dialog(filters, "");
			if (UIFiles.path != null) {
				while (UIFiles.path.indexOf(path_sep + path_sep) >= 0) UIFiles.path = string_replace_all(UIFiles.path, path_sep + path_sep, path_sep);
				UIFiles.path = string_replace_all(UIFiles.path, "\r", "");
				UIFiles.filename = UIFiles.path.substr(UIFiles.path.lastIndexOf(path_sep) + 1);
				UIFiles.path = UIFiles.path.substr(0, UIFiles.path.lastIndexOf(path_sep));
				filesDone(UIFiles.path);
			}
		}
		else {
			let paths: string[] = krom_open_dialog(filters, "", openMultiple);
			if (paths != null) {
				for (let path of paths) {
					while (path.indexOf(path_sep + path_sep) >= 0) path = string_replace_all(path, path_sep + path_sep, path_sep);
					path = string_replace_all(path, "\r", "");
					UIFiles.filename = path.substr(path.lastIndexOf(path_sep) + 1);
					filesDone(path);
				}
			}
		}

		UIFiles.release_keys();
	}

	// static show_custom = (filters: string, isSave: bool, filesDone: (s: string)=>void) => {
	// 	let known: bool = false;
	// 	UIBox.show_custom((ui: ZuiRaw) => {
	// 		if (Zui.tab(Zui.handle(), tr("File Browser"))) {
	// 			let path_handle: zui_handle_t = Zui.handle();
	// 			let file_handle: zui_handle_t = Zui.handle();
	// 			Zui.row([6 / 10, 2 / 10, 2 / 10]);
	// 			filename = Zui.textInput(file_handle, tr("File"));
	// 			Zui.text("*." + filters, Center);
	// 			if (Zui.button(isSave ? tr("Save") : tr("Open")) || known || ui.isReturnDown) {
	// 				UIBox.hide();
	// 				filesDone((known || isSave) ? path : path + sep + filename);
	// 				if (known) path_handle.text = path_handle.text.substr(0, path_handle.text.lastIndexOf(sep));
	// 			}
	// 			known = isTexture(path) || isMesh(path) || isProject(path);
	// 			path = fileBrowser(ui, path_handle, false);
	// 			if (path_handle.changed) ui.currentWindow.redraws = 3;
	// 		}
	// 	}, 600, 500);
	// }

	static release_keys = () => {
		// File dialog may prevent firing key up events
		keyboard_up_listener(key_code_t.SHIFT);
		keyboard_up_listener(key_code_t.CONTROL);
		///if krom_darwin
		keyboard_up_listener(key_code_t.META);
		///end
	}

	static file_browser = (ui: zui_t, handle: zui_handle_t, foldersOnly: bool = false, dragFiles: bool = false, search: string = "", refresh: bool = false, contextMenu: (s: string)=>void = null): string => {

		let icons: image_t = resource_get("icons.k");
		let folder: rect_t = resource_tile50(icons, 2, 1);
		let file: rect_t = resource_tile50(icons, 3, 1);
		let is_cloud: bool = handle.text.startsWith("cloud");

		if (is_cloud && file_cloud == null) file_init_cloud(() => { UIBase.hwnds[tab_area_t.STATUS].redraws = 3; });
		if (is_cloud && file_read_directory("cloud", false).length == 0) return handle.text;

		///if krom_ios
		let document_directory: string = krom_save_dialog("", "");
		document_directory = document_directory.substr(0, document_directory.length - 8); // Strip /'untitled'
		///end

		if (handle.text == "") handle.text = UIFiles.default_path;
		if (handle.text != UIFiles.last_path || search != UIFiles.last_search || refresh) {
			UIFiles.files = [];

			// Up directory
			let i1: i32 = handle.text.indexOf(path_sep);
			let nested: bool = i1 > -1 && handle.text.length - 1 > i1;
			///if krom_windows
			// Server addresses like \\server are not nested
			nested = nested && !(handle.text.length >= 2 && handle.text.charAt(0) == path_sep && handle.text.charAt(1) == path_sep && handle.text.lastIndexOf(path_sep) == 1);
			///end
			if (nested) UIFiles.files.push("..");

			let dir_path: string = handle.text;
			///if krom_ios
			if (!is_cloud) dir_path = document_directory + dir_path;
			///end
			let files_all: string[] = file_read_directory(dir_path, foldersOnly);

			for (let f of files_all) {
				if (f == "" || f.charAt(0) == ".") continue; // Skip hidden
				if (f.indexOf(".") > 0 && !path_is_known(f)) continue; // Skip unknown extensions
				if (is_cloud && f.indexOf("_icon.") >= 0) continue; // Skip thumbnails
				if (f.toLowerCase().indexOf(search.toLowerCase()) < 0) continue; // Search filter
				UIFiles.files.push(f);
			}
		}
		UIFiles.last_path = handle.text;
		UIFiles.last_search = search;
		handle.changed = false;

		let slotw: i32 = Math.floor(70 * zui_SCALE(ui));
		let num: i32 = Math.floor(ui._w / slotw);

		ui._y += 4; // Don't cut off the border around selected materials
		// Directory contents
		for (let row: i32 = 0; row < Math.floor(Math.ceil(UIFiles.files.length / num)); ++row) {
			let ar: f32[] = [];
			for (let i: i32 = 0; i < num * 2; ++i) ar.push(1 / num);
			zui_row(ar);
			if (row > 0) ui._y += zui_ELEMENT_OFFSET(ui) * 14.0;

			for (let j: i32 = 0; j < num; ++j) {
				let i: i32 = j + row * num;
				if (i >= UIFiles.files.length) {
					zui_end_element(slotw);
					zui_end_element(slotw);
					continue;
				}

				let f: string = UIFiles.files[i];
				let _x: f32 = ui._x;

				let rect: rect_t = f.indexOf(".") > 0 ? file : folder;
				let col: i32 = rect == file ? ui.t.LABEL_COL : ui.t.LABEL_COL - 0x00202020;
				if (UIFiles.selected == i) col = ui.t.HIGHLIGHT_COL;

				let off: f32 = ui._w / 2 - 25 * zui_SCALE(ui);
				ui._x += off;

				let uix: f32 = ui._x;
				let uiy: f32 = ui._y;
				let state: zui_state_t = zui_state_t.IDLE;
				let generic: bool = true;
				let icon: image_t = null;

				if (is_cloud && f != ".." && !UIFiles.offline) {
					if (UIFiles.icon_map == null) UIFiles.icon_map = new Map();
					icon = UIFiles.icon_map.get(handle.text + path_sep + f);
					if (icon == null) {
						let files_all: string[] = file_read_directory(handle.text);
						let icon_file: string = f.substr(0, f.lastIndexOf(".")) + "_icon.jpg";
						if (files_all.indexOf(icon_file) >= 0) {
							let empty: image_t = render_path_render_targets.get("empty_black")._image;
							UIFiles.icon_map.set(handle.text + path_sep + f, empty);
							file_cache_cloud(handle.text + path_sep + icon_file, (abs: string) => {
								if (abs != null) {
									let image: image_t = data_get_image(abs);
									app_notify_on_init(() => {
										if (base_pipe_copy_rgb == null) base_make_pipe_copy_rgb();
										icon = image_create_render_target(image.width, image.height);
										if (f.endsWith(".arm")) { // Used for material sphere alpha cutout
											g2_begin(icon);

											///if (is_paint || is_sculpt)
											g2_draw_image(project_materials[0].image, 0, 0);
											///end
										}
										else {
											g2_begin(icon);
											g2_clear(0xffffffff);
										}
										g2_set_pipeline(base_pipe_copy_rgb);
										g2_draw_image(image, 0, 0);
										g2_set_pipeline(null);
										g2_end();
										UIFiles.icon_map.set(handle.text + path_sep + f, icon);
										UIBase.hwnds[tab_area_t.STATUS].redraws = 3;
									});
								}
								else UIFiles.offline = true;
							});
						}
					}
					if (icon != null) {
						let w: i32 = 50;
						if (i == UIFiles.selected) {
							zui_fill(-2,        -2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							zui_fill(-2,     w + 2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							zui_fill(-2,         0,     2, w + 4, ui.t.HIGHLIGHT_COL);
							zui_fill(w + 2 ,    -2,     2, w + 6, ui.t.HIGHLIGHT_COL);
						}
						state = zui_image(icon, 0xffffffff, w * zui_SCALE(ui));
						if (ui.is_hovered) {
							zui_tooltip_image(icon);
							zui_tooltip(f);
						}
						generic = false;
					}
				}
				if (f.endsWith(".arm") && !is_cloud) {
					if (UIFiles.icon_map == null) UIFiles.icon_map = new Map();
					let key: string = handle.text + path_sep + f;
					icon = UIFiles.icon_map.get(key);
					if (!UIFiles.icon_map.has(key)) {
						let blob_path: string = key;

						///if krom_ios
						blob_path = document_directory + blob_path;
						// TODO: implement native .arm parsing first
						///else

						let buffer: buffer_t = krom_load_blob(blob_path);
						let raw: any = armpack_decode(buffer);
						if (raw.material_icons != null) {
							let bytes_icon: any = raw.material_icons[0];
							icon = image_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
						}

						///if (is_paint || is_sculpt)
						else if (raw.mesh_icons != null) {
							let bytes_icon: any = raw.mesh_icons[0];
							icon = image_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
						}
						else if (raw.brush_icons != null) {
							let bytes_icon: any = raw.brush_icons[0];
							icon = image_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
						}
						///end

						///if is_lab
						if (raw.mesh_icon != null) {
							let bytes_icon: any = raw.mesh_icon;
							icon = image_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
						}
						///end

						UIFiles.icon_map.set(key, icon);
						///end
					}
					if (icon != null) {
						let w: i32 = 50;
						if (i == UIFiles.selected) {
							zui_fill(-2,        -2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							zui_fill(-2,     w + 2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							zui_fill(-2,         0,     2, w + 4, ui.t.HIGHLIGHT_COL);
							zui_fill(w + 2 ,    -2,     2, w + 6, ui.t.HIGHLIGHT_COL);
						}
						state = zui_image(icon, 0xffffffff, w * zui_SCALE(ui));
						if (ui.is_hovered) {
							zui_tooltip_image(icon);
							zui_tooltip(f);
						}
						generic = false;
					}
				}

				if (path_is_texture(f) && !is_cloud) {
					let w: i32 = 50;
					if (UIFiles.icon_map == null) UIFiles.icon_map = new Map();
					let shandle: string = handle.text + path_sep + f;
					icon = UIFiles.icon_map.get(shandle);
					if (icon == null) {
						let empty: image_t = render_path_render_targets.get("empty_black")._image;
						UIFiles.icon_map.set(shandle, empty);
						let image: image_t = data_get_image(shandle);
						app_notify_on_init(() => {
							if (base_pipe_copy_rgb == null) base_make_pipe_copy_rgb();
							let sw: i32 = image.width > image.height ? w : Math.floor(1.0 * image.width / image.height * w);
							let sh: i32 = image.width > image.height ? Math.floor(1.0 * image.height / image.width * w) : w;
							icon = image_create_render_target(sw, sh);
							g2_begin(icon);
							g2_clear(0xffffffff);
							g2_set_pipeline(base_pipe_copy_rgb);
							g2_draw_scaled_image(image, 0, 0, sw, sh);
							g2_set_pipeline(null);
							g2_end();
							UIFiles.icon_map.set(shandle, icon);
							UIBase.hwnds[tab_area_t.STATUS].redraws = 3;
							data_delete_image(shandle); // The big image is not needed anymore
						});
					}
					if (icon != null) {
						if (i == UIFiles.selected) {
							zui_fill(-2,        -2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							zui_fill(-2,     w + 2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							zui_fill(-2,         0,     2, w + 4, ui.t.HIGHLIGHT_COL);
							zui_fill(w + 2 ,    -2,     2, w + 6, ui.t.HIGHLIGHT_COL);
						}
						state = zui_image(icon, 0xffffffff, icon.height * zui_SCALE(ui));
						generic = false;
					}
				}

				if (generic) {
					state = zui_image(icons, col, 50 * zui_SCALE(ui), rect.x, rect.y, rect.w, rect.h);
				}

				if (ui.is_hovered && ui.input_released_r && contextMenu != null) {
					contextMenu(handle.text + path_sep + f);
				}

				if (state == zui_state_t.STARTED) {
					if (f != ".." && dragFiles) {
						base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
						base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
						base_drag_file = handle.text;
						///if krom_ios
						if (!is_cloud) base_drag_file = document_directory + base_drag_file;
						///end
						if (base_drag_file.charAt(base_drag_file.length - 1) != path_sep) {
							base_drag_file += path_sep;
						}
						base_drag_file += f;
						base_drag_file_icon = icon;
					}

					UIFiles.selected = i;
					if (time_time() - context_raw.select_time < 0.25) {
						base_drag_file = null;
						base_drag_file_icon = null;
						base_is_dragging = false;
						handle.changed = ui.changed = true;
						if (f == "..") { // Up
							handle.text = handle.text.substring(0, handle.text.lastIndexOf(path_sep));
							// Drive root
							if (handle.text.length == 2 && handle.text.charAt(1) == ":") handle.text += path_sep;
						}
						else {
							if (handle.text.charAt(handle.text.length - 1) != path_sep) {
								handle.text += path_sep;
							}
							handle.text += f;
						}
						UIFiles.selected = -1;
					}
					context_raw.select_time = time_time();
				}

				// Label
				ui._x = _x;
				ui._y += slotw * 0.75;
				let label0: string = (UIFiles.show_extensions || f.indexOf(".") <= 0) ? f : f.substr(0, f.lastIndexOf("."));
				let label1: string = "";
				while (label0.length > 0 && g2_font_width(ui.font, ui.font_size, label0) > ui._w - 6) { // 2 line split
					label1 = label0.charAt(label0.length - 1) + label1;
					label0 = label0.substr(0, label0.length - 1);
				}
				if (label1 != "") ui.cur_ratio--;
				zui_text(label0, zui_align_t.CENTER);
				if (ui.is_hovered) zui_tooltip(label0 + label1);
				if (label1 != "") { // Second line
					ui._x = _x;
					ui._y += g2_font_height(ui.font, ui.font_size);
					zui_text(label1, zui_align_t.CENTER);
					if (ui.is_hovered) zui_tooltip(label0 + label1);
					ui._y -= g2_font_height(ui.font, ui.font_size);
				}

				ui._y -= slotw * 0.75;

				if (handle.changed) break;
			}

			if (handle.changed) break;
		}
		ui._y += slotw * 0.8;

		return handle.text;
	}
}
