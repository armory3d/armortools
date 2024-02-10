
class UIFiles {

	static defaultPath =
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
	static path = UIFiles.defaultPath;
	static lastPath = "";
	static lastSearch = "";
	static files: string[] = null;
	static iconMap: Map<string, image_t> = null;
	static selected = -1;
	static showExtensions = false;
	static offline = false;

	static show = (filters: string, isSave: bool, openMultiple: bool, filesDone: (s: string)=>void) => {
		if (isSave) {
			UIFiles.path = krom_save_dialog(filters, "");
			if (UIFiles.path != null) {
				while (UIFiles.path.indexOf(Path.sep + Path.sep) >= 0) UIFiles.path = UIFiles.path.replaceAll(Path.sep + Path.sep, Path.sep);
				UIFiles.path = UIFiles.path.replaceAll("\r", "");
				UIFiles.filename = UIFiles.path.substr(UIFiles.path.lastIndexOf(Path.sep) + 1);
				UIFiles.path = UIFiles.path.substr(0, UIFiles.path.lastIndexOf(Path.sep));
				filesDone(UIFiles.path);
			}
		}
		else {
			let paths = krom_open_dialog(filters, "", openMultiple);
			if (paths != null) {
				for (let path of paths) {
					while (path.indexOf(Path.sep + Path.sep) >= 0) path = path.replaceAll(Path.sep + Path.sep, Path.sep);
					path = path.replaceAll("\r", "");
					UIFiles.filename = path.substr(path.lastIndexOf(Path.sep) + 1);
					filesDone(path);
				}
			}
		}

		UIFiles.releaseKeys();
	}

	// static showCustom = (filters: string, isSave: bool, filesDone: (s: string)=>void) => {
	// 	let known = false;
	// 	UIBox.showCustom((ui: ZuiRaw) => {
	// 		if (Zui.tab(Zui.handle(), tr("File Browser"))) {
	// 			let pathHandle = Zui.handle();
	// 			let fileHandle = Zui.handle();
	// 			Zui.row([6 / 10, 2 / 10, 2 / 10]);
	// 			filename = Zui.textInput(fileHandle, tr("File"));
	// 			Zui.text("*." + filters, Center);
	// 			if (Zui.button(isSave ? tr("Save") : tr("Open")) || known || ui.isReturnDown) {
	// 				UIBox.hide();
	// 				filesDone((known || isSave) ? path : path + Path.sep + filename);
	// 				if (known) pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(Path.sep));
	// 			}
	// 			known = Path.isTexture(path) || Path.isMesh(path) || Path.isProject(path);
	// 			path = fileBrowser(ui, pathHandle, false);
	// 			if (pathHandle.changed) ui.currentWindow.redraws = 3;
	// 		}
	// 	}, 600, 500);
	// }

	static releaseKeys = () => {
		// File dialog may prevent firing key up events
		keyboard_up_listener(key_code_t.SHIFT);
		keyboard_up_listener(key_code_t.CONTROL);
		///if krom_darwin
		keyboard_up_listener(key_code_t.META);
		///end
	}

	static fileBrowser = (ui: zui_t, handle: zui_handle_t, foldersOnly = false, dragFiles = false, search = "", refresh = false, contextMenu: (s: string)=>void = null): string => {

		let icons = Res.get("icons.k");
		let folder = Res.tile50(icons, 2, 1);
		let file = Res.tile50(icons, 3, 1);
		let isCloud = handle.text.startsWith("cloud");

		if (isCloud && File.cloud == null) File.initCloud(() => { UIBase.hwnds[TabArea.TabStatus].redraws = 3; });
		if (isCloud && File.readDirectory("cloud", false).length == 0) return handle.text;

		///if krom_ios
		let documentDirectory = krom_save_dialog("", "");
		documentDirectory = documentDirectory.substr(0, documentDirectory.length - 8); // Strip /'untitled'
		///end

		if (handle.text == "") handle.text = UIFiles.defaultPath;
		if (handle.text != UIFiles.lastPath || search != UIFiles.lastSearch || refresh) {
			UIFiles.files = [];

			// Up directory
			let i1 = handle.text.indexOf(Path.sep);
			let nested = i1 > -1 && handle.text.length - 1 > i1;
			///if krom_windows
			// Server addresses like \\server are not nested
			nested = nested && !(handle.text.length >= 2 && handle.text.charAt(0) == Path.sep && handle.text.charAt(1) == Path.sep && handle.text.lastIndexOf(Path.sep) == 1);
			///end
			if (nested) UIFiles.files.push("..");

			let dirPath = handle.text;
			///if krom_ios
			if (!isCloud) dirPath = documentDirectory + dirPath;
			///end
			let filesAll = File.readDirectory(dirPath, foldersOnly);

			for (let f of filesAll) {
				if (f == "" || f.charAt(0) == ".") continue; // Skip hidden
				if (f.indexOf(".") > 0 && !Path.isKnown(f)) continue; // Skip unknown extensions
				if (isCloud && f.indexOf("_icon.") >= 0) continue; // Skip thumbnails
				if (f.toLowerCase().indexOf(search.toLowerCase()) < 0) continue; // Search filter
				UIFiles.files.push(f);
			}
		}
		UIFiles.lastPath = handle.text;
		UIFiles.lastSearch = search;
		handle.changed = false;

		let slotw = Math.floor(70 * zui_SCALE(ui));
		let num = Math.floor(ui._w / slotw);

		ui._y += 4; // Don't cut off the border around selected materials
		// Directory contents
		for (let row = 0; row < Math.floor(Math.ceil(UIFiles.files.length / num)); ++row) {
			let ar = [];
			for (let i = 0; i < num * 2; ++i) ar.push(1 / num);
			zui_row(ar);
			if (row > 0) ui._y += zui_ELEMENT_OFFSET(ui) * 14.0;

			for (let j = 0; j < num; ++j) {
				let i = j + row * num;
				if (i >= UIFiles.files.length) {
					zui_end_element(slotw);
					zui_end_element(slotw);
					continue;
				}

				let f = UIFiles.files[i];
				let _x = ui._x;

				let rect = f.indexOf(".") > 0 ? file : folder;
				let col = rect == file ? ui.t.LABEL_COL : ui.t.LABEL_COL - 0x00202020;
				if (UIFiles.selected == i) col = ui.t.HIGHLIGHT_COL;

				let off = ui._w / 2 - 25 * zui_SCALE(ui);
				ui._x += off;

				let uix = ui._x;
				let uiy = ui._y;
				let state = State.Idle;
				let generic = true;
				let icon: image_t = null;

				if (isCloud && f != ".." && !UIFiles.offline) {
					if (UIFiles.iconMap == null) UIFiles.iconMap = new Map();
					icon = UIFiles.iconMap.get(handle.text + Path.sep + f);
					if (icon == null) {
						let filesAll = File.readDirectory(handle.text);
						let iconFile = f.substr(0, f.lastIndexOf(".")) + "_icon.jpg";
						if (filesAll.indexOf(iconFile) >= 0) {
							let empty = render_path_render_targets.get("empty_black").image;
							UIFiles.iconMap.set(handle.text + Path.sep + f, empty);
							File.cacheCloud(handle.text + Path.sep + iconFile, (abs: string) => {
								if (abs != null) {
									data_get_image(abs, (image: image_t) => {
										app_notify_on_init(() => {
											if (Base.pipeCopyRGB == null) Base.makePipeCopyRGB();
											icon = image_create_render_target(image.width, image.height);
											if (f.endsWith(".arm")) { // Used for material sphere alpha cutout
												g2_begin(icon, false);

												///if (is_paint || is_sculpt)
												g2_draw_image(Project.materials[0].image, 0, 0);
												///end
											}
											else {
												g2_begin(icon, true, 0xffffffff);
											}
											g2_set_pipeline(Base.pipeCopyRGB);
											g2_draw_image(image, 0, 0);
											g2_set_pipeline(null);
											g2_end();
											UIFiles.iconMap.set(handle.text + Path.sep + f, icon);
											UIBase.hwnds[TabArea.TabStatus].redraws = 3;
										});
									});
								}
								else UIFiles.offline = true;
							});
						}
					}
					if (icon != null) {
						let w = 50;
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
				if (f.endsWith(".arm") && !isCloud) {
					if (UIFiles.iconMap == null) UIFiles.iconMap = new Map();
					let key = handle.text + Path.sep + f;
					icon = UIFiles.iconMap.get(key);
					if (!UIFiles.iconMap.has(key)) {
						let blobPath = key;

						///if krom_ios
						blobPath = documentDirectory + blobPath;
						// TODO: implement native .arm parsing first
						///else

						let buffer = krom_load_blob(blobPath);
						let raw = armpack_decode(buffer);
						if (raw.material_icons != null) {
							let bytesIcon = raw.material_icons[0];
							icon = image_from_bytes(lz4_decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}

						///if (is_paint || is_sculpt)
						else if (raw.mesh_icons != null) {
							let bytesIcon = raw.mesh_icons[0];
							icon = image_from_bytes(lz4_decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}
						else if (raw.brush_icons != null) {
							let bytesIcon = raw.brush_icons[0];
							icon = image_from_bytes(lz4_decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}
						///end

						///if is_lab
						if (raw.mesh_icon != null) {
							let bytesIcon = raw.mesh_icon;
							icon = image_from_bytes(lz4_decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}
						///end

						UIFiles.iconMap.set(key, icon);
						///end
					}
					if (icon != null) {
						let w = 50;
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

				if (Path.isTexture(f) && !isCloud) {
					let w = 50;
					if (UIFiles.iconMap == null) UIFiles.iconMap = new Map();
					let shandle = handle.text + Path.sep + f;
					icon = UIFiles.iconMap.get(shandle);
					if (icon == null) {
						let empty = render_path_render_targets.get("empty_black").image;
						UIFiles.iconMap.set(shandle, empty);
						data_get_image(shandle, (image: image_t) => {
							app_notify_on_init(() => {
								if (Base.pipeCopyRGB == null) Base.makePipeCopyRGB();
								let sw = image.width > image.height ? w : Math.floor(1.0 * image.width / image.height * w);
								let sh = image.width > image.height ? Math.floor(1.0 * image.height / image.width * w) : w;
								icon = image_create_render_target(sw, sh);
								g2_begin(icon, true, 0xffffffff);
								g2_set_pipeline(Base.pipeCopyRGB);
								g2_draw_scaled_image(image, 0, 0, sw, sh);
								g2_set_pipeline(null);
								g2_end();
								UIFiles.iconMap.set(shandle, icon);
								UIBase.hwnds[TabArea.TabStatus].redraws = 3;
								data_delete_image(shandle); // The big image is not needed anymore
							});
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
					contextMenu(handle.text + Path.sep + f);
				}

				if (state == State.Started) {
					if (f != ".." && dragFiles) {
						Base.dragOffX = -(mouse_x - uix - ui._window_x - 3);
						Base.dragOffY = -(mouse_y - uiy - ui._window_y + 1);
						Base.dragFile = handle.text;
						///if krom_ios
						if (!isCloud) Base.dragFile = documentDirectory + Base.dragFile;
						///end
						if (Base.dragFile.charAt(Base.dragFile.length - 1) != Path.sep) {
							Base.dragFile += Path.sep;
						}
						Base.dragFile += f;
						Base.dragFileIcon = icon;
					}

					UIFiles.selected = i;
					if (time_time() - Context.raw.selectTime < 0.25) {
						Base.dragFile = null;
						Base.dragFileIcon = null;
						Base.isDragging = false;
						handle.changed = ui.changed = true;
						if (f == "..") { // Up
							handle.text = handle.text.substring(0, handle.text.lastIndexOf(Path.sep));
							// Drive root
							if (handle.text.length == 2 && handle.text.charAt(1) == ":") handle.text += Path.sep;
						}
						else {
							if (handle.text.charAt(handle.text.length - 1) != Path.sep) {
								handle.text += Path.sep;
							}
							handle.text += f;
						}
						UIFiles.selected = -1;
					}
					Context.raw.selectTime = time_time();
				}

				// Label
				ui._x = _x;
				ui._y += slotw * 0.75;
				let label0 = (UIFiles.showExtensions || f.indexOf(".") <= 0) ? f : f.substr(0, f.lastIndexOf("."));
				let label1 = "";
				while (label0.length > 0 && g2_font_width(ui.font, ui.font_size, label0) > ui._w - 6) { // 2 line split
					label1 = label0.charAt(label0.length - 1) + label1;
					label0 = label0.substr(0, label0.length - 1);
				}
				if (label1 != "") ui.cur_ratio--;
				zui_text(label0, Align.Center);
				if (ui.is_hovered) zui_tooltip(label0 + label1);
				if (label1 != "") { // Second line
					ui._x = _x;
					ui._y += g2_font_height(ui.font, ui.font_size);
					zui_text(label1, Align.Center);
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
