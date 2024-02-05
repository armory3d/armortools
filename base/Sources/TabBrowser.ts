
class TabBrowser {

	static hpath = Handle.create();
	static hsearch = Handle.create();
	static known = false;
	static lastPath =  "";

	static showDirectory = (directory: string) => {
		TabBrowser.hpath.text = directory;
		TabBrowser.hsearch.text = "";
		UIBase.htabs[TabArea.TabStatus].position = 0;
	}

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (Zui.tab(htab, tr("Browser")) && statush > UIStatus.defaultStatusH * Zui.SCALE(ui)) {

			if (Config.raw.bookmarks == null) {
				Config.raw.bookmarks = [];
			}

			let bookmarksW = Math.floor(100 * Zui.SCALE(ui));

			if (TabBrowser.hpath.text == "" && Config.raw.bookmarks.length > 0) { // Init to first bookmark
				TabBrowser.hpath.text = Config.raw.bookmarks[0];
			}

			Zui.beginSticky();
			let step = (1 - bookmarksW / ui._w);
			if (TabBrowser.hsearch.text != "") {
				Zui.row([bookmarksW / ui._w, step * 0.73, step * 0.07, step * 0.17, step * 0.03]);
			}
			else {
				Zui.row([bookmarksW / ui._w, step * 0.73, step * 0.07, step * 0.2]);
			}

			if (Zui.button("+")) {
				Config.raw.bookmarks.push(TabBrowser.hpath.text);
				Config.save();
			}
			if (ui.isHovered) Zui.tooltip(tr("Add bookmark"));

			///if krom_android
			let stripped = false;
			let strip = "/storage/emulated/0/";
			if (TabBrowser.hpath.text.startsWith(strip)) {
				TabBrowser.hpath.text = TabBrowser.hpath.text.substr(strip.length - 1);
				stripped = true;
			}
			///end

			TabBrowser.hpath.text = Zui.textInput(TabBrowser.hpath, tr("Path"));

			///if krom_android
			if (stripped) {
				TabBrowser.hpath.text = "/storage/emulated/0" + TabBrowser.hpath.text;
			}
			///end

			let refresh = false;
			let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (Zui.button(tr("Refresh")) || (inFocus && ui.isKeyPressed && ui.key == KeyCode.F5)) {
				refresh = true;
			}
			TabBrowser.hsearch.text = Zui.textInput(TabBrowser.hsearch, tr("Search"), Align.Left, true, true);
			if (ui.isHovered) Zui.tooltip(tr("ctrl+f to search") + "\n" + tr("esc to cancel"));
			if (ui.isCtrlDown && ui.isKeyPressed && ui.key == KeyCode.F) { // Start searching via ctrl+f
				Zui.startTextEdit(TabBrowser.hsearch);
			}
			if (TabBrowser.hsearch.text != "" && (Zui.button(tr("X")) || ui.isEscapeDown)) {
				TabBrowser.hsearch.text = "";
			}
			Zui.endSticky();

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
					UIMenu.draw((ui: ZuiRaw) => {
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
				App.notifyOnInit(() => {
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

			if (Zui.button(tr("Cloud"), Align.Left)) {
				TabBrowser.hpath.text = "cloud";
			}

			if (Zui.button(tr("Disk"), Align.Left)) {
				///if krom_android
				UIMenu.draw((ui: ZuiRaw) => {
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
						TabBrowser.hpath.text = Krom.savePath();
					}
				}, 4);
				///else
				TabBrowser.hpath.text = UIFiles.defaultPath;
				///end
			}

			for (let b of Config.raw.bookmarks) {
				let folder = b.substr(b.lastIndexOf(Path.sep) + 1);

				if (Zui.button(folder, Align.Left)) {
					TabBrowser.hpath.text = b;
				}

				if (ui.isHovered && ui.inputReleasedR) {
					UIMenu.draw((ui: ZuiRaw) => {
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
