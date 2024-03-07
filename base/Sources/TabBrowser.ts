
class TabBrowser {

	static hpath: zui_handle_t = zui_handle_create();
	static hsearch: zui_handle_t = zui_handle_create();
	static known: bool = false;
	static last_path: string =  "";

	static show_directory = (directory: string) => {
		TabBrowser.hpath.text = directory;
		TabBrowser.hsearch.text = "";
		UIBase.htabs[tab_area_t.STATUS].position = 0;
	}

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Browser")) && statush > UIStatus.default_status_h * zui_SCALE(ui)) {

			if (Config.raw.bookmarks == null) {
				Config.raw.bookmarks = [];
			}

			let bookmarksW: i32 = Math.floor(100 * zui_SCALE(ui));

			if (TabBrowser.hpath.text == "" && Config.raw.bookmarks.length > 0) { // Init to first bookmark
				TabBrowser.hpath.text = Config.raw.bookmarks[0];
			}

			zui_begin_sticky();
			let step: i32 = (1 - bookmarksW / ui._w);
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
			let stripped: bool = false;
			let strip: string = "/storage/emulated/0/";
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

			let refresh: bool = false;
			let inFocus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (zui_button(tr("Refresh")) || (inFocus && ui.is_key_pressed && ui.key == key_code_t.F5)) {
				refresh = true;
			}
			TabBrowser.hsearch.text = zui_text_input(TabBrowser.hsearch, tr("Search"), zui_align_t.LEFT, true, true);
			if (ui.is_hovered) zui_tooltip(tr("ctrl+f to search") + "\n" + tr("esc to cancel"));
			if (ui.is_ctrl_down && ui.is_key_pressed && ui.key == key_code_t.F) { // Start searching via ctrl+f
				zui_start_text_edit(TabBrowser.hsearch);
			}
			if (TabBrowser.hsearch.text != "" && (zui_button(tr("X")) || ui.is_escape_down)) {
				TabBrowser.hsearch.text = "";
			}
			zui_end_sticky();

			if (TabBrowser.last_path != TabBrowser.hpath.text) {
				TabBrowser.hsearch.text = "";
			}
			TabBrowser.last_path = TabBrowser.hpath.text;

			let _y: f32 = ui._y;
			ui._x = bookmarksW;
			ui._w -= bookmarksW;
			UIFiles.file_browser(ui, TabBrowser.hpath, false, true, TabBrowser.hsearch.text, refresh, (file: string) => {
				let fileName: string = file.substr(file.lastIndexOf(Path.sep) + 1);
				if (fileName != "..") {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Import"))) {
							ImportAsset.run(file);
						}
						if (Path.is_texture(file)) {
							if (UIMenu.menu_button(ui, tr("Set as Envmap"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									Base.notify_on_next_frame(() => {
										let assetIndex: i32 = -1;
										for (let i: i32 = 0; i < Project.assets.length; ++i) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											ImportEnvmap.run(file, Project.get_image(Project.assets[assetIndex]));
										}
									});
								});
							}

							///if (is_paint || is_sculpt)
							if (UIMenu.menu_button(ui, tr("Set as Mask"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									Base.notify_on_next_frame(() => {
										let assetIndex: i32 = -1;
										for (let i: i32 = 0; i < Project.assets.length; ++i) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											Base.create_image_mask(Project.assets[assetIndex]);
										}
									});
								});
							}
							///end

							///if is_paint
							if (UIMenu.menu_button(ui, tr("Set as Color ID Map"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									Base.notify_on_next_frame(() => {
										let assetIndex: i32 = -1;
										for (let i: i32 = 0; i < Project.assets.length; ++i) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											Context.raw.colorid_handle.position = assetIndex;
											Context.raw.colorid_picked = false;
											UIToolbar.toolbar_handle.redraws = 1;
											if (Context.raw.tool == workspace_tool_t.COLORID) {
												UIHeader.header_handle.redraws = 2;
												Context.raw.ddirty = 2;
											}
										}
									});
								});
							}
							///end
						}
						if (UIMenu.menu_button(ui, tr("Open Externally"))) {
							File.start(file);
						}
					}, Path.is_texture(file) ? 5 : 2);
				}
			});

			if (TabBrowser.known) {
				let path: string = TabBrowser.hpath.text;
				app_notify_on_init(() => {
					ImportAsset.run(path);
				});
				TabBrowser.hpath.text = TabBrowser.hpath.text.substr(0, TabBrowser.hpath.text.lastIndexOf(Path.sep));
			}
			TabBrowser.known = TabBrowser.hpath.text.substr(TabBrowser.hpath.text.lastIndexOf(Path.sep)).indexOf(".") > 0;
			///if krom_android
			if (TabBrowser.hpath.text.endsWith("." + manifest_title.toLowerCase())) TabBrowser.known = false;
			///end

			let bottomY: i32 = ui._y;
			ui._x = 0;
			ui._y = _y;
			ui._w = bookmarksW;

			if (zui_button(tr("Cloud"), zui_align_t.LEFT)) {
				TabBrowser.hpath.text = "cloud";
			}

			if (zui_button(tr("Disk"), zui_align_t.LEFT)) {
				///if krom_android
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menu_button(ui, tr("Download"))) {
						TabBrowser.hpath.text = UIFiles.default_path;
					}
					if (UIMenu.menu_button(ui, tr("Pictures"))) {
						TabBrowser.hpath.text = "/storage/emulated/0/Pictures";
					}
					if (UIMenu.menu_button(ui, tr("Camera"))) {
						TabBrowser.hpath.text = "/storage/emulated/0/DCIM/Camera";
					}
					if (UIMenu.menu_button(ui, tr("Projects"))) {
						TabBrowser.hpath.text = krom_save_path();
					}
				}, 4);
				///else
				TabBrowser.hpath.text = UIFiles.default_path;
				///end
			}

			for (let b of Config.raw.bookmarks) {
				let folder: string = b.substr(b.lastIndexOf(Path.sep) + 1);

				if (zui_button(folder, zui_align_t.LEFT)) {
					TabBrowser.hpath.text = b;
				}

				if (ui.is_hovered && ui.input_released_r) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Delete"))) {
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
