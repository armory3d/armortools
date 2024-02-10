
class TabBrowser {

	static hpath = zui_handle_create();
	static hsearch = zui_handle_create();
	static known = false;
	static lastPath =  "";

	static showDirectory = (directory: string) => {
		TabBrowser.hpath.text = directory;
		TabBrowser.hsearch.text = "";
		UIBase.htabs[TabArea.TabStatus].position = 0;
	}

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, tr("Browser")) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

			if (Config.raw.bookmarks == null) {
				Config.raw.bookmarks = [];
			}

			let bookmarksW = Math.floor(100 * zui_SCALE(ui));

			if (TabBrowser.hpath.text == "" && Config.raw.bookmarks.length > 0) { // Init to first bookmark
				TabBrowser.hpath.text = Config.raw.bookmarks[0];
			}

			zui_begin_sticky();
			let step = (1 - bookmarksW / ui._w);
			if (TabBrowser.hsearch.text != "") {
				zui_row([bookmarksW / ui._w, step * 0.73, step * 0.07, step * 0.17, step * 0.03]);
			}
			else {
				zui_row([bookmarksW / ui._w, step * 0.73, step * 0.07, step * 0.2]);
			}

			if (zui_button("+")) {
				Config.raw.bookmarks.push(TabBrowser.hpath.text);
				Config.save();
			}
			if (ui.is_hovered) zui_tooltip(tr("Add bookmark"));

			///if krom_android
			let stripped = false;
			let strip = "/storage/emulated/0/";
			if (TabBrowser.hpath.text.startsWith(strip)) {
				TabBrowser.hpath.text = TabBrowser.hpath.text.substr(strip.length - 1);
				stripped = true;
			}
			///end

			TabBrowser.hpath.text = zui_text_input(TabBrowser.hpath, tr("Path"));

			///if krom_android
			if (stripped) {
				TabBrowser.hpath.text = "/storage/emulated/0" + TabBrowser.hpath.text;
			}
			///end

			let refresh = false;
			let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (zui_button(tr("Refresh")) || (inFocus && ui.is_key_pressed && ui.key == key_code_t.F5)) {
				refresh = true;
			}
			TabBrowser.hsearch.text = zui_text_input(TabBrowser.hsearch, tr("Search"), Align.Left, true, true);
			if (ui.is_hovered) zui_tooltip(tr("ctrl+f to search") + "\n" + tr("esc to cancel"));
			if (ui.is_ctrl_down && ui.is_key_pressed && ui.key == key_code_t.F) { // Start searching via ctrl+f
				zui_start_text_edit(TabBrowser.hsearch);
			}
			if (TabBrowser.hsearch.text != "" && (zui_button(tr("X")) || ui.is_escape_down)) {
				TabBrowser.hsearch.text = "";
			}
			zui_end_sticky();

			if (TabBrowser.lastPath != TabBrowser.hpath.text) {
				TabBrowser.hsearch.text = "";
			}
			TabBrowser.lastPath = TabBrowser.hpath.text;

			let _y = ui._y;
			ui._x = bookmarksW;
			ui._w -= bookmarksW;
			UIFiles.fileBrowser(ui, TabBrowser.hpath, false, true, TabBrowser.hsearch.text, refresh, (file: string) => {
				let fileName = file.substr(file.lastIndexOf(Path.sep) + 1);
				if (fileName != "..") {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menuButton(ui, tr("Import"))) {
							ImportAsset.run(file);
						}
						if (Path.isTexture(file)) {
							if (UIMenu.menuButton(ui, tr("Set as Envmap"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									Base.notifyOnNextFrame(() => {
										let assetIndex = -1;
										for (let i = 0; i < Project.assets.length; ++i) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											ImportEnvmap.run(file, Project.getImage(Project.assets[assetIndex]));
										}
									});
								});
							}

							///if (is_paint || is_sculpt)
							if (UIMenu.menuButton(ui, tr("Set as Mask"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									Base.notifyOnNextFrame(() => {
										let assetIndex = -1;
										for (let i = 0; i < Project.assets.length; ++i) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											Base.createImageMask(Project.assets[assetIndex]);
										}
									});
								});
							}
							///end

							///if is_paint
							if (UIMenu.menuButton(ui, tr("Set as Color ID Map"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									Base.notifyOnNextFrame(() => {
										let assetIndex = -1;
										for (let i = 0; i < Project.assets.length; ++i) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											Context.raw.colorIdHandle.position = assetIndex;
											Context.raw.colorIdPicked = false;
											UIToolbar.toolbarHandle.redraws = 1;
											if (Context.raw.tool == WorkspaceTool.ToolColorId) {
												UIHeader.headerHandle.redraws = 2;
												Context.raw.ddirty = 2;
											}
										}
									});
								});
							}
							///end
						}
						if (UIMenu.menuButton(ui, tr("Open Externally"))) {
							File.start(file);
						}
					}, Path.isTexture(file) ? 5 : 2);
				}
			});

			if (TabBrowser.known) {
				let path = TabBrowser.hpath.text;
				app_notify_on_init(() => {
					ImportAsset.run(path);
				});
				TabBrowser.hpath.text = TabBrowser.hpath.text.substr(0, TabBrowser.hpath.text.lastIndexOf(Path.sep));
			}
			TabBrowser.known = TabBrowser.hpath.text.substr(TabBrowser.hpath.text.lastIndexOf(Path.sep)).indexOf(".") > 0;
			///if krom_android
			if (TabBrowser.hpath.text.endsWith("." + manifest_title.toLowerCase())) TabBrowser.known = false;
			///end

			let bottomY = ui._y;
			ui._x = 0;
			ui._y = _y;
			ui._w = bookmarksW;

			if (zui_button(tr("Cloud"), Align.Left)) {
				TabBrowser.hpath.text = "cloud";
			}

			if (zui_button(tr("Disk"), Align.Left)) {
				///if krom_android
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menuButton(ui, tr("Download"))) {
						TabBrowser.hpath.text = UIFiles.defaultPath;
					}
					if (UIMenu.menuButton(ui, tr("Pictures"))) {
						TabBrowser.hpath.text = "/storage/emulated/0/Pictures";
					}
					if (UIMenu.menuButton(ui, tr("Camera"))) {
						TabBrowser.hpath.text = "/storage/emulated/0/DCIM/Camera";
					}
					if (UIMenu.menuButton(ui, tr("Projects"))) {
						TabBrowser.hpath.text = krom_save_path();
					}
				}, 4);
				///else
				TabBrowser.hpath.text = UIFiles.defaultPath;
				///end
			}

			for (let b of Config.raw.bookmarks) {
				let folder = b.substr(b.lastIndexOf(Path.sep) + 1);

				if (zui_button(folder, Align.Left)) {
					TabBrowser.hpath.text = b;
				}

				if (ui.is_hovered && ui.input_released_r) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menuButton(ui, tr("Delete"))) {
							array_remove(Config.raw.bookmarks, b);
							Config.save();
						}
					}, 1);
				}
			}

			if (ui._y < bottomY) ui._y = bottomY;
		}
	}
}
