
let tab_browser_hpath: ui_handle_t   = ui_handle_create();
let tab_browser_hsearch: ui_handle_t = ui_handle_create();
let tab_browser_known: bool          = false;
let tab_browser_last_path: string    = "";
let _tab_browser_draw_file: string;
let _tab_browser_draw_b: string;
let tab_browser_refresh: bool = false;

function tab_browser_show_directory(directory: string) {
	tab_browser_hpath.text                    = directory;
	tab_browser_hsearch.text                  = "";
	ui_base_htabs[tab_area_t.STATUS].i        = 0;
	config_raw.layout_tabs[tab_area_t.STATUS] = 0;
}

function tab_browser_draw(htab: ui_handle_t) {
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_tab(htab, tr("Browser")) && statush > ui_statusbar_default_h * UI_SCALE()) {

		if (config_raw.bookmarks == null) {
			config_raw.bookmarks = [];
		}

		let bookmarks_w: i32 = math_floor(100 * UI_SCALE());

		if (tab_browser_hpath.text == "" && config_raw.bookmarks.length > 0) { // Init to first bookmark
			tab_browser_hpath.text = config_raw.bookmarks[0];
			/// if arm_windows
			tab_browser_hpath.text = string_replace_all(tab_browser_hpath.text, "/", "\\");
			/// end
		}

		ui_begin_sticky();
		let step: f32 = (1.0 - bookmarks_w / ui._w);
		if (tab_browser_hsearch.text != "") {
			let row: f32[] = [ bookmarks_w / ui._w, step * 0.07, step * 0.07, step * 0.66, step * 0.17, step * 0.03 ];
			ui_row(row);
		}
		else {
			let row: f32[] = [ bookmarks_w / ui._w, step * 0.07, step * 0.07, step * 0.66, step * 0.2 ];
			ui_row(row);
		}

		// Bookmark
		if (ui_button("+")) {
			let bookmark: string = tab_browser_hpath.text;
			/// if arm_windows
			bookmark = string_replace_all(bookmark, "\\", "/");
			/// end
			array_push(config_raw.bookmarks, bookmark);
			config_save();
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Add bookmark"));
		}

		// Refresh
		let in_focus: bool =
		    ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w && ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (ui_button(tr("Refresh")) || (in_focus && ui.is_key_pressed && ui.key_code == key_code_t.F5)) {
			tab_browser_refresh = true;
		}

		// Previous folder
		let text: string = tab_browser_hpath.text;
		let i1: i32      = string_index_of(text, path_sep);
		let nested: bool = i1 > -1 && text.length - 1 > i1;
		/// if arm_windows
		// Server addresses like \\server are not nested
		nested = nested && !(text.length >= 2 && char_at(text, 0) == path_sep && char_at(text, 1) == path_sep && string_last_index_of(text, path_sep) == 1);
		/// end

		ui.enabled = nested;
		if (ui_button("<")) {
			ui_files_go_up(tab_browser_hpath);
		}
		ui.enabled = true;
		if (ui.is_hovered) {
			ui_tooltip(tr("Previous folder"));
		}

		/// if arm_android
		let stripped: bool = false;
		let strip: string  = "/storage/emulated/0/";
		if (starts_with(tab_browser_hpath.text, strip)) {
			tab_browser_hpath.text = substring(tab_browser_hpath.text, strip.length - 1, tab_browser_hpath.text.length);
			stripped               = true;
		}
		/// end

		tab_browser_hpath.text = ui_text_input(tab_browser_hpath, tr("Path"));

		/// if arm_android
		if (stripped) {
			tab_browser_hpath.text = "/storage/emulated/0" + tab_browser_hpath.text;
		}
		/// end

		tab_browser_hsearch.text = ui_text_input(tab_browser_hsearch, tr("Search"), ui_align_t.LEFT, true, true);
		if (ui.is_hovered) {
			ui_tooltip(tr("ctrl+f to search") + "\n" + tr("esc to cancel"));
		}
		if (ui.is_ctrl_down && ui.is_key_pressed && ui.key_code == key_code_t.F) { // Start searching via ctrl+f
			ui_start_text_edit(tab_browser_hsearch);
		}
		if (tab_browser_hsearch.text != "" && (ui_button(tr("X")) || ui.is_escape_down)) {
			tab_browser_hsearch.text = "";
		}
		ui_end_sticky();

		if (tab_browser_last_path != tab_browser_hpath.text) {
			tab_browser_hsearch.text = "";
		}
		tab_browser_last_path = tab_browser_hpath.text;

		let _y: f32 = ui._y;
		ui._x       = bookmarks_w;
		ui._w -= bookmarks_w;

		ui_files_file_browser(tab_browser_hpath, true, tab_browser_hsearch.text, tab_browser_refresh, function(file: string) {
			let file_name: string  = substring(file, string_last_index_of(file, path_sep) + 1, file.length);
			_tab_browser_draw_file = file;

			// Context menu
			ui_menu_draw(function() {
				let file: string = _tab_browser_draw_file;

				if (ui_menu_button(tr("Import"))) {
					import_asset_run(file);
				}
				if (path_is_texture(file)) {
					if (ui_menu_button(tr("Set as Envmap"))) {

						import_asset_run(file, -1.0, -1.0, true, true, function() {
							sys_notify_on_next_frame(function() {
								let file: string = _tab_browser_draw_file;

								let asset_index: i32 = -1;
								for (let i: i32 = 0; i < project_assets.length; ++i) {
									if (project_assets[i].file == file) {
										asset_index = i;
										break;
									}
								}
								if (asset_index != -1) {
									import_envmap_run(file, project_get_image(project_assets[asset_index]));
								}
							});
						});
					}

					if (ui_menu_button(tr("Set as Mask"))) {
						import_asset_run(file, -1.0, -1.0, true, true, function() {
							sys_notify_on_next_frame(function() {
								let file: string = _tab_browser_draw_file;

								let asset_index: i32 = -1;
								for (let i: i32 = 0; i < project_assets.length; ++i) {
									if (project_assets[i].file == file) {
										asset_index = i;
										break;
									}
								}
								if (asset_index != -1) {
									layers_create_image_mask(project_assets[asset_index]);
								}
							});
						});
					}

					if (ui_menu_button(tr("Set as Color ID Map"))) {
						import_asset_run(file, -1.0, -1.0, true, true, function() {
							sys_notify_on_next_frame(function() {
								let file: string = _tab_browser_draw_file;

								let asset_index: i32 = -1;
								for (let i: i32 = 0; i < project_assets.length; ++i) {
									if (project_assets[i].file == file) {
										asset_index = i;
										break;
									}
								}
								if (asset_index != -1) {
									context_raw.colorid_handle.i = asset_index;
									context_raw.colorid_picked   = false;
									ui_toolbar_handle.redraws    = 1;
									if (context_raw.tool == tool_type_t.COLORID) {
										ui_header_handle.redraws = 2;
										context_raw.ddirty       = 2;
									}
								}
							});
						});
					}
				}
				if (ui_menu_button(tr("Open Externally"))) {
					file_start(file);
				}
			});
		});

		tab_browser_refresh = false;

		if (tab_browser_known) {
			let path: string = tab_browser_hpath.text;
			sys_notify_on_next_frame(function(path: string) {
				import_asset_run(path);
			}, path);
			tab_browser_hpath.text = substring(tab_browser_hpath.text, 0, string_last_index_of(tab_browser_hpath.text, path_sep));
		}
		let hpath_text: string = tab_browser_hpath.text;
		tab_browser_known =
		    string_index_of(substring(tab_browser_hpath.text, string_last_index_of(tab_browser_hpath.text, path_sep), hpath_text.length), ".") > 0;
		/// if arm_android
		if (ends_with(tab_browser_hpath.text, "." + to_lower_case(manifest_title))) {
			tab_browser_known = false;
		}
		/// end

		let bottom_y: i32 = ui._y;
		ui._x             = 0;
		ui._y             = _y;
		ui._w             = bookmarks_w;

		if (ui_button(tr("Cloud"), ui_align_t.LEFT)) {
			tab_browser_hpath.text = "cloud";
		}

		if (ui_button(tr("Disk"), ui_align_t.LEFT)) {
			/// if arm_android
			ui_menu_draw(function() {
				if (ui_menu_button(tr("Download"))) {
					tab_browser_hpath.text = ui_files_default_path;
				}
				if (ui_menu_button(tr("Pictures"))) {
					tab_browser_hpath.text = "/storage/emulated/0/Pictures";
				}
				if (ui_menu_button(tr("Camera"))) {
					tab_browser_hpath.text = "/storage/emulated/0/DCIM/Camera";
				}
				if (ui_menu_button(tr("Projects"))) {
					tab_browser_hpath.text = iron_internal_save_path();
				}
			});
			/// else
			tab_browser_hpath.text = ui_files_default_path;
			/// end
		}

		for (let i: i32 = 0; i < config_raw.bookmarks.length; ++i) {
			let b: string      = config_raw.bookmarks[i];
			let folder: string = substring(b, string_last_index_of(b, "/") + 1, b.length);

			if (ui_button(folder, ui_align_t.LEFT)) {
				tab_browser_hpath.text = b;
				/// if arm_windows
				tab_browser_hpath.text = string_replace_all(tab_browser_hpath.text, "/", "\\");
				/// end
			}

			if (ui.is_hovered && ui.input_released_r) {
				_tab_browser_draw_b = b;
				ui_menu_draw(function() {
					if (ui_menu_button(tr("Delete"))) {
						array_remove(config_raw.bookmarks, _tab_browser_draw_b);
						config_save();
					}
				});
			}
		}

		if (ui._y < bottom_y) {
			ui._y = bottom_y;
		}
	}
}
