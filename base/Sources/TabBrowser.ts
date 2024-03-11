
class TabBrowser {

	static hpath: zui_handle_t = zui_handle_create();
	static hsearch: zui_handle_t = zui_handle_create();
	static known: bool = false;
	static last_path: string =  "";

	static show_directory = (directory: string) => {
		TabBrowser.hpath.text = directory;
		TabBrowser.hsearch.text = "";
		ui_base_htabs[tab_area_t.STATUS].position = 0;
	}

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = ui_base_ui;
		let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Browser")) && statush > ui_status_default_status_h * zui_SCALE(ui)) {

			if (config_raw.bookmarks == null) {
				config_raw.bookmarks = [];
			}

			let bookmarks_w: i32 = math_floor(100 * zui_SCALE(ui));

			if (TabBrowser.hpath.text == "" && config_raw.bookmarks.length > 0) { // Init to first bookmark
				TabBrowser.hpath.text = config_raw.bookmarks[0];
			}

			zui_begin_sticky();
			let step: i32 = (1 - bookmarks_w / ui._w);
			if (TabBrowser.hsearch.text != "") {
				zui_row([bookmarks_w / ui._w, step * 0.73, step * 0.07, step * 0.17, step * 0.03]);
			}
			else {
				zui_row([bookmarks_w / ui._w, step * 0.73, step * 0.07, step * 0.2]);
			}

			if (zui_button("+")) {
				config_raw.bookmarks.push(TabBrowser.hpath.text);
				config_save();
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
			let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (zui_button(tr("Refresh")) || (in_focus && ui.is_key_pressed && ui.key == key_code_t.F5)) {
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
			ui._x = bookmarks_w;
			ui._w -= bookmarks_w;
			ui_files_file_browser(ui, TabBrowser.hpath, false, true, TabBrowser.hsearch.text, refresh, (file: string) => {
				let file_name: string = file.substr(file.lastIndexOf(path_sep) + 1);
				if (file_name != "..") {
					ui_menu_draw((ui: zui_t) => {
						if (ui_menu_button(ui, tr("Import"))) {
							ImportAsset.run(file);
						}
						if (path_is_texture(file)) {
							if (ui_menu_button(ui, tr("Set as Envmap"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									base_notify_on_next_frame(() => {
										let asset_index: i32 = -1;
										for (let i: i32 = 0; i < project_assets.length; ++i) {
											if (project_assets[i].file == file) {
												asset_index = i;
												break;
											}
										}
										if (asset_index != -1) {
											ImportEnvmap.run(file, project_get_image(project_assets[asset_index]));
										}
									});
								});
							}

							///if (is_paint || is_sculpt)
							if (ui_menu_button(ui, tr("Set as Mask"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									base_notify_on_next_frame(() => {
										let asset_index: i32 = -1;
										for (let i: i32 = 0; i < project_assets.length; ++i) {
											if (project_assets[i].file == file) {
												asset_index = i;
												break;
											}
										}
										if (asset_index != -1) {
											base_create_image_mask(project_assets[asset_index]);
										}
									});
								});
							}
							///end

							///if is_paint
							if (ui_menu_button(ui, tr("Set as Color ID Map"))) {
								ImportAsset.run(file, -1.0, -1.0, true, true, () => {
									base_notify_on_next_frame(() => {
										let asset_index: i32 = -1;
										for (let i: i32 = 0; i < project_assets.length; ++i) {
											if (project_assets[i].file == file) {
												asset_index = i;
												break;
											}
										}
										if (asset_index != -1) {
											context_raw.colorid_handle.position = asset_index;
											context_raw.colorid_picked = false;
											ui_toolbar_handle.redraws = 1;
											if (context_raw.tool == workspace_tool_t.COLORID) {
												ui_header_handle.redraws = 2;
												context_raw.ddirty = 2;
											}
										}
									});
								});
							}
							///end
						}
						if (ui_menu_button(ui, tr("Open Externally"))) {
							file_start(file);
						}
					}, path_is_texture(file) ? 5 : 2);
				}
			});

			if (TabBrowser.known) {
				let path: string = TabBrowser.hpath.text;
				app_notify_on_init(() => {
					ImportAsset.run(path);
				});
				TabBrowser.hpath.text = TabBrowser.hpath.text.substr(0, TabBrowser.hpath.text.lastIndexOf(path_sep));
			}
			TabBrowser.known = TabBrowser.hpath.text.substr(TabBrowser.hpath.text.lastIndexOf(path_sep)).indexOf(".") > 0;
			///if krom_android
			if (TabBrowser.hpath.text.endsWith("." + manifest_title.toLowerCase())) TabBrowser.known = false;
			///end

			let bottom_y: i32 = ui._y;
			ui._x = 0;
			ui._y = _y;
			ui._w = bookmarks_w;

			if (zui_button(tr("Cloud"), zui_align_t.LEFT)) {
				TabBrowser.hpath.text = "cloud";
			}

			if (zui_button(tr("Disk"), zui_align_t.LEFT)) {
				///if krom_android
				ui_menu_draw((ui: zui_t) => {
					if (ui_menu_button(ui, tr("Download"))) {
						TabBrowser.hpath.text = ui_files_default_path;
					}
					if (ui_menu_button(ui, tr("Pictures"))) {
						TabBrowser.hpath.text = "/storage/emulated/0/Pictures";
					}
					if (ui_menu_button(ui, tr("Camera"))) {
						TabBrowser.hpath.text = "/storage/emulated/0/DCIM/Camera";
					}
					if (ui_menu_button(ui, tr("Projects"))) {
						TabBrowser.hpath.text = krom_save_path();
					}
				}, 4);
				///else
				TabBrowser.hpath.text = ui_files_default_path;
				///end
			}

			for (let b of config_raw.bookmarks) {
				let folder: string = b.substr(b.lastIndexOf(path_sep) + 1);

				if (zui_button(folder, zui_align_t.LEFT)) {
					TabBrowser.hpath.text = b;
				}

				if (ui.is_hovered && ui.input_released_r) {
					ui_menu_draw((ui: zui_t) => {
						if (ui_menu_button(ui, tr("Delete"))) {
							array_remove(config_raw.bookmarks, b);
							config_save();
						}
					}, 1);
				}
			}

			if (ui._y < bottom_y) ui._y = bottom_y;
		}
	}
}
