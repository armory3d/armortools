
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
	static iconMap: Map<string, Image> = null;
	static selected = -1;
	static showExtensions = false;
	static offline = false;

	static show = (filters: string, isSave: bool, openMultiple: bool, filesDone: (s: string)=>void) => {
		if (isSave) {
			UIFiles.path = Krom.saveDialog(filters, "");
			if (UIFiles.path != null) {
				while (UIFiles.path.indexOf(Path.sep + Path.sep) >= 0) UIFiles.path = UIFiles.path.replace(Path.sep + Path.sep, Path.sep);
				UIFiles.path = UIFiles.path.replace("\r", "");
				UIFiles.filename = UIFiles.path.substr(UIFiles.path.lastIndexOf(Path.sep) + 1);
				UIFiles.path = UIFiles.path.substr(0, UIFiles.path.lastIndexOf(Path.sep));
				filesDone(UIFiles.path);
			}
		}
		else {
			let paths = Krom.openDialog(filters, "", openMultiple);
			if (paths != null) {
				for (let path of paths) {
					while (path.indexOf(Path.sep + Path.sep) >= 0) path = path.replace(Path.sep + Path.sep, Path.sep);
					path = path.replace("\r", "");
					UIFiles.filename = path.substr(path.lastIndexOf(Path.sep) + 1);
					filesDone(path);
				}
			}
		}

		UIFiles.releaseKeys();
	}

	// static showCustom = (filters: string, isSave: bool, filesDone: (s: string)=>void) => {
	// 	let known = false;
	// 	UIBox.showCustom((ui: Zui) => {
	// 		if (ui.tab(Zui.handle(), tr("File Browser"))) {
	// 			let pathHandle = Zui.handle();
	// 			let fileHandle = Zui.handle();
	// 			ui.row([6 / 10, 2 / 10, 2 / 10]);
	// 			filename = ui.textInput(fileHandle, tr("File"));
	// 			ui.text("*." + filters, Center);
	// 			if (ui.button(isSave ? tr("Save") : tr("Open")) || known || ui.isReturnDown) {
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
		let kb = Input.getKeyboard();
		kb.upListener(KeyCode.Shift);
		kb.upListener(KeyCode.Control);
		///if krom_darwin
		kb.upListener(KeyCode.Meta);
		///end
	}

	static fileBrowser = (ui: Zui, handle: Handle, foldersOnly = false, dragFiles = false, search = "", refresh = false, contextMenu: (s: string)=>void = null): string => {

		let icons = Res.get("icons.k");
		let folder = Res.tile50(icons, 2, 1);
		let file = Res.tile50(icons, 3, 1);
		let isCloud = handle.text.startsWith("cloud");

		if (isCloud && File.cloud == null) File.initCloud(() => { UIBase.inst.hwnds[TabArea.TabStatus].redraws = 3; });
		if (isCloud && File.readDirectory("cloud", false).length == 0) return handle.text;

		///if krom_ios
		let documentDirectory = Krom.saveDialog("", "");
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

		let slotw = Math.floor(70 * ui.SCALE());
		let num = Math.floor(ui._w / slotw);

		ui._y += 4; // Don't cut off the border around selected materials
		// Directory contents
		for (let row = 0; row < Math.floor(Math.ceil(UIFiles.files.length / num)); ++row) {
			let ar = [];
			for (let i = 0; i < num * 2; ++i) ar.push(1 / num);
			ui.row(ar);
			if (row > 0) ui._y += ui.ELEMENT_OFFSET() * 14.0;

			for (let j = 0; j < num; ++j) {
				let i = j + row * num;
				if (i >= UIFiles.files.length) {
					ui.endElement(slotw);
					ui.endElement(slotw);
					continue;
				}

				let f = UIFiles.files[i];
				let _x = ui._x;

				let rect = f.indexOf(".") > 0 ? file : folder;
				let col = rect == file ? ui.t.LABEL_COL : ui.t.LABEL_COL - 0x00202020;
				if (UIFiles.selected == i) col = ui.t.HIGHLIGHT_COL;

				let off = ui._w / 2 - 25 * ui.SCALE();
				ui._x += off;

				let uix = ui._x;
				let uiy = ui._y;
				let state = State.Idle;
				let generic = true;
				let icon: Image = null;

				if (isCloud && f != ".." && !UIFiles.offline) {
					if (UIFiles.iconMap == null) UIFiles.iconMap = new Map();
					icon = UIFiles.iconMap.get(handle.text + Path.sep + f);
					if (icon == null) {
						let filesAll = File.readDirectory(handle.text);
						let iconFile = f.substr(0, f.lastIndexOf(".")) + "_icon.jpg";
						if (filesAll.indexOf(iconFile) >= 0) {
							let empty = RenderPath.active.renderTargets.get("empty_black").image;
							UIFiles.iconMap.set(handle.text + Path.sep + f, empty);
							File.cacheCloud(handle.text + Path.sep + iconFile, (abs: string) => {
								if (abs != null) {
									Data.getImage(abs, (image: Image) => {
										App.notifyOnInit(() => {
											if (Base.pipeCopyRGB == null) Base.makePipeCopyRGB();
											icon = Image.createRenderTarget(image.width, image.height);
											if (f.endsWith(".arm")) { // Used for material sphere alpha cutout
												icon.g2.begin(false);

												///if (is_paint || is_sculpt)
												icon.g2.drawImage(Project.materials[0].image, 0, 0);
												///end
											}
											else {
												icon.g2.begin(true, 0xffffffff);
											}
											icon.g2.pipeline = Base.pipeCopyRGB;
											icon.g2.drawImage(image, 0, 0);
											icon.g2.pipeline = null;
											icon.g2.end();
											UIFiles.iconMap.set(handle.text + Path.sep + f, icon);
											UIBase.inst.hwnds[TabArea.TabStatus].redraws = 3;
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
							ui.fill(-2,        -2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							ui.fill(-2,     w + 2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							ui.fill(-2,         0,     2, w + 4, ui.t.HIGHLIGHT_COL);
							ui.fill(w + 2 ,    -2,     2, w + 6, ui.t.HIGHLIGHT_COL);
						}
						state = ui.image(icon, 0xffffffff, w * ui.SCALE());
						if (ui.isHovered) {
							ui.tooltipImage(icon);
							ui.tooltip(f);
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

						let buffer = Krom.loadBlob(blobPath);
						let raw = ArmPack.decode(buffer);
						if (raw.material_icons != null) {
							let bytesIcon = raw.material_icons[0];
							icon = Image.fromBytes(Lz4.decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}

						///if (is_paint || is_sculpt)
						else if (raw.mesh_icons != null) {
							let bytesIcon = raw.mesh_icons[0];
							icon = Image.fromBytes(Lz4.decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}
						else if (raw.brush_icons != null) {
							let bytesIcon = raw.brush_icons[0];
							icon = Image.fromBytes(Lz4.decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}
						///end

						///if is_lab
						if (raw.mesh_icon != null) {
							let bytesIcon = raw.mesh_icon;
							icon = Image.fromBytes(Lz4.decode(bytesIcon, 256 * 256 * 4), 256, 256);
						}
						///end

						UIFiles.iconMap.set(key, icon);
						///end
					}
					if (icon != null) {
						let w = 50;
						if (i == UIFiles.selected) {
							ui.fill(-2,        -2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							ui.fill(-2,     w + 2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							ui.fill(-2,         0,     2, w + 4, ui.t.HIGHLIGHT_COL);
							ui.fill(w + 2 ,    -2,     2, w + 6, ui.t.HIGHLIGHT_COL);
						}
						state = ui.image(icon, 0xffffffff, w * ui.SCALE());
						if (ui.isHovered) {
							ui.tooltipImage(icon);
							ui.tooltip(f);
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
						let empty = RenderPath.active.renderTargets.get("empty_black").image;
						UIFiles.iconMap.set(shandle, empty);
						Data.getImage(shandle, (image: Image) => {
							App.notifyOnInit(() => {
								if (Base.pipeCopyRGB == null) Base.makePipeCopyRGB();
								let sw = image.width > image.height ? w : Math.floor(1.0 * image.width / image.height * w);
								let sh = image.width > image.height ? Math.floor(1.0 * image.height / image.width * w) : w;
								icon = Image.createRenderTarget(sw, sh);
								icon.g2.begin(true, 0xffffffff);
								icon.g2.pipeline = Base.pipeCopyRGB;
								icon.g2.drawScaledImage(image, 0, 0, sw, sh);
								icon.g2.pipeline = null;
								icon.g2.end();
								UIFiles.iconMap.set(shandle, icon);
								UIBase.inst.hwnds[TabArea.TabStatus].redraws = 3;
								Data.deleteImage(shandle); // The big image is not needed anymore
							});
						});
					}
					if (icon != null) {
						if (i == UIFiles.selected) {
							ui.fill(-2,        -2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							ui.fill(-2,     w + 2, w + 4,     2, ui.t.HIGHLIGHT_COL);
							ui.fill(-2,         0,     2, w + 4, ui.t.HIGHLIGHT_COL);
							ui.fill(w + 2 ,    -2,     2, w + 6, ui.t.HIGHLIGHT_COL);
						}
						state = ui.image(icon, 0xffffffff, icon.height * ui.SCALE());
						generic = false;
					}
				}

				if (generic) {
					state = ui.image(icons, col, 50 * ui.SCALE(), rect.x, rect.y, rect.w, rect.h);
				}

				if (ui.isHovered && ui.inputReleasedR && contextMenu != null) {
					contextMenu(handle.text + Path.sep + f);
				}

				if (state == State.Started) {
					if (f != ".." && dragFiles) {
						let mouse = Input.getMouse();
						Base.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						Base.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
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
					if (Time.time() - Context.raw.selectTime < 0.25) {
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
					Context.raw.selectTime = Time.time();
				}

				// Label
				ui._x = _x;
				ui._y += slotw * 0.75;
				let label0 = (UIFiles.showExtensions || f.indexOf(".") <= 0) ? f : f.substr(0, f.lastIndexOf("."));
				let label1 = "";
				while (label0.length > 0 && ui.font.width(ui.fontSize, label0) > ui._w - 6) { // 2 line split
					label1 = label0.charAt(label0.length - 1) + label1;
					label0 = label0.substr(0, label0.length - 1);
				}
				if (label1 != "") ui.curRatio--;
				ui.text(label0, Align.Center);
				if (ui.isHovered) ui.tooltip(label0 + label1);
				if (label1 != "") { // Second line
					ui._x = _x;
					ui._y += ui.font.height(ui.fontSize);
					ui.text(label1, Align.Center);
					if (ui.isHovered) ui.tooltip(label0 + label1);
					ui._y -= ui.font.height(ui.fontSize);
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
