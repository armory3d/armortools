
#include "global.h"

void tab_browser_show_directory(char *directory) {
	tab_browser_hpath->text                          = string_copy(directory);
	tab_browser_hsearch->text                        = "";
	ui_base_htabs->buffer[TAB_AREA_STATUS]->i        = 0;
	config_raw->layout_tabs->buffer[TAB_AREA_STATUS] = 0;
}

void tab_browser_draw_bookmark_menu() {
	if (ui_menu_button(tr("Delete", NULL), "", ICON_DELETE)) {
		string_array_remove(config_raw->bookmarks, _tab_browser_draw_b);
		config_save();
	}
}

void tab_browser_draw_import_asset(char *path) {
	import_asset_run(path, -1.0, -1.0, true, true, NULL);
}

void tab_browser_draw_set_as_color_id_map_on_next_frame(void *_) {
	char *file        = _tab_browser_draw_file;
	i32   asset_index = -1;
	for (i32 i = 0; i < project_assets->length; ++i) {
		if (string_equals(project_assets->buffer[i]->file, file)) {
			asset_index = i;
			break;
		}
	}

	if (asset_index != -1) {
		context_raw->colorid_handle->i = asset_index;
		context_raw->colorid_picked    = false;
		ui_toolbar_handle->redraws     = 1;
		if (context_raw->tool == TOOL_TYPE_COLORID) {
			ui_header_handle->redraws = 2;
			context_raw->ddirty       = 2;
		}
	}
}

void tab_browser_draw_set_as_color_id_map() {
	sys_notify_on_next_frame(&tab_browser_draw_set_as_color_id_map_on_next_frame, NULL);
}

void tab_browser_draw_set_as_mask_on_next_frame(void *_) {
	char *file        = _tab_browser_draw_file;
	i32   asset_index = -1;
	for (i32 i = 0; i < project_assets->length; ++i) {
		if (string_equals(project_assets->buffer[i]->file, file)) {
			asset_index = i;
			break;
		}
	}
	if (asset_index != -1) {
		layers_create_image_mask(project_assets->buffer[asset_index]);
	}
}

void tab_browser_draw_set_as_mask() {
	sys_notify_on_next_frame(&tab_browser_draw_set_as_mask_on_next_frame, NULL);
}

void tab_browser_draw_set_as_envmap_on_next_frame(void *_) {
	char *file        = _tab_browser_draw_file;
	i32   asset_index = -1;
	for (i32 i = 0; i < project_assets->length; ++i) {
		if (string_equals(project_assets->buffer[i]->file, file)) {
			asset_index = i;
			break;
		}
	}

	if (asset_index != -1) {
		import_envmap_run(file, project_get_image(project_assets->buffer[asset_index]));
	}
}

void tab_browser_draw_set_as_envmap() {
	sys_notify_on_next_frame(&tab_browser_draw_set_as_envmap_on_next_frame, NULL);
}

void tab_browser_draw_context_menu_draw() {
	char *file = _tab_browser_draw_file;
	if (ui_menu_button(tr("Import", NULL), "", ICON_IMPORT)) {
		import_asset_run(file, -1.0, -1.0, true, true, NULL);
	}
	if (path_is_texture(file)) {
		if (ui_menu_button(tr("Set as Envmap", NULL), "", ICON_LANDSCAPE)) {
			import_asset_run(file, -1.0, -1.0, true, true, &tab_browser_draw_set_as_envmap);
		}
		if (ui_menu_button(tr("Set as Mask", NULL), "", ICON_MASK)) {
			import_asset_run(file, -1.0, -1.0, true, true, &tab_browser_draw_set_as_mask);
		}
		if (ui_menu_button(tr("Set as Color ID Map", NULL), "", ICON_COLOR_ID)) {
			import_asset_run(file, -1.0, -1.0, true, true, &tab_browser_draw_set_as_color_id_map);
		}
	}
	if (ui_menu_button(tr("Open Externally", NULL), "", ICON_NONE)) {
		file_start(file);
	}
}

void tab_browser_draw_context_menu(char *file) {
	gc_unroot(_tab_browser_draw_file);
	_tab_browser_draw_file = string_copy(file);
	gc_root(_tab_browser_draw_file);

	// Context menu
	ui_menu_draw(&tab_browser_draw_context_menu_draw, -1, -1);
}

void tab_browser_draw_side_menu() {
	if (ui_menu_button(tr("Cloud", NULL), "", ICON_CLOUD)) {
		tab_browser_go_to_cloud();
	}
	if (ui_menu_button(tr("Disk", NULL), "", ICON_STORAGE)) {
		tab_browser_go_to_disk();
	}
}

void tab_browser_draw(ui_handle_t *htab) {
	char *title = tr("Browser", NULL);

#ifdef IRON_IOS
	if (config_is_iphone()) {
		title = string("  %s", title);
	}
#endif

	if (ui_tab(htab, title, false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {
		if (config_raw->bookmarks == NULL) {
			config_raw->bookmarks = any_array_create_from_raw((void *[]){}, 0);
		}

		i32  bookmarks_w = math_floor(100 * UI_SCALE());
		bool show_full   = ui->_w > (500 * UI_SCALE());

		if (string_equals(tab_browser_hpath->text, "") && config_raw->bookmarks->length > 0) { // Init to first bookmark
			tab_browser_hpath->text = string_copy(config_raw->bookmarks->buffer[0]);
#ifdef IRON_WINDOWS
			tab_browser_hpath->text = string_copy(string_replace_all(tab_browser_hpath->text, "/", "\\"));
#endif
		}

		ui_begin_sticky();

		if (show_full) {
			f32 step = (1.0 - bookmarks_w / (float)ui->_w);
			if (!string_equals(tab_browser_hsearch->text, "")) {
				f32_array_t *row = f32_array_create_from_raw(
				    (f32[]){
				        bookmarks_w / (float)ui->_w,
				        step * 0.07,
				        step * 0.07,
				        step * 0.66,
				        step * 0.17,
				        step * 0.03,
				    },
				    6);
				ui_row(row);
			}
			else {
				f32_array_t *row = f32_array_create_from_raw(
				    (f32[]){
				        bookmarks_w / (float)ui->_w,
				        step * 0.07,
				        step * 0.07,
				        step * 0.66,
				        step * 0.2,
				    },
				    5);
				ui_row(row);
			}

			// Bookmark
			if (ui_icon_button(tr("Bookmark", NULL), ICON_PLUS, UI_ALIGN_LEFT)) {
				char *bookmark = tab_browser_hpath->text;
#ifdef IRON_WINDOWS
				bookmark = string_copy(string_replace_all(bookmark, "\\", "/"));
#endif
				any_array_push(config_raw->bookmarks, bookmark);
				config_save();
			}

			// Refresh
			bool in_focus = ui->input_x > ui->_window_x && ui->input_x < ui->_window_x + ui->_window_w && ui->input_y > ui->_window_y &&
			                ui->input_y < ui->_window_y + ui->_window_h;
			if (ui_icon_button(tr("Refresh", NULL), ICON_REFRESH, UI_ALIGN_CENTER) || (in_focus && ui->is_key_pressed && ui->key_code == KEY_CODE_F5)) {
				tab_browser_refresh = true;
			}
		}
		else {
			// Menu, Up, Refresh
			f32_array_t *row = f32_array_create_from_raw(
			    (f32[]){
			        0.5 / (float)4,
			        0.5 / (float)4,
			        3 / (float)4,
			    },
			    3);
			ui_row(row);
			if (ui_icon_button("", ICON_MENU, UI_ALIGN_CENTER)) {
				ui_menu_draw(&tab_browser_draw_side_menu, -1, -1);
			}
		}

		// Previous folder
		char *text   = tab_browser_hpath->text;
		i32   i1     = string_index_of(text, PATH_SEP);
		bool  nested = i1 > -1 && string_length(text) - 1 > i1;
#ifdef IRON_WINDOWS
		// Server addresses like \\server are not nested
		nested = nested && !(string_length(text) >= 2 && string_equals(char_at(text, 0), PATH_SEP) && string_equals(char_at(text, 1), PATH_SEP) &&
		                     string_last_index_of(text, PATH_SEP) == 1);
#endif
		ui->enabled = nested;
		if (ui_icon_button("", ICON_CHEVRON_LEFT, UI_ALIGN_CENTER)) {
			ui_files_go_up(tab_browser_hpath);
		}
		ui->enabled = true;
		if (ui->is_hovered) {
			ui_tooltip(tr("Previous folder", NULL));
		}

#ifdef IRON_ANDROID
		bool  stripped = false;
		char *strip    = "/storage/emulated/0/";
		if (starts_with(tab_browser_hpath->text, strip)) {
			tab_browser_hpath->text = string_copy(substring(tab_browser_hpath->text, string_length(strip) - 1, string_length(tab_browser_hpath->text)));
			stripped                = true;
		}
#endif

		tab_browser_hpath->text = string_copy(ui_text_input(tab_browser_hpath, tr("Path", NULL), UI_ALIGN_LEFT, true, false));

#ifdef IRON_ANDROID
		if (stripped) {
			tab_browser_hpath->text = string("/storage/emulated/0%s", tab_browser_hpath->text);
		}
#endif

		if (show_full) {
			tab_browser_hsearch->text = string_copy(ui_text_input(tab_browser_hsearch, tr("Search", NULL), UI_ALIGN_LEFT, true, true));
			if (ui->is_hovered) {
				ui_tooltip(string("%s\n%s", tr("ctrl+f to search", NULL), tr("esc to cancel", NULL)));
			}
			if (ui->is_ctrl_down && ui->is_key_pressed && ui->key_code == KEY_CODE_F) { // Start searching via ctrl+f
				ui_start_text_edit(tab_browser_hsearch, UI_ALIGN_LEFT);
			}
			if (!string_equals(tab_browser_hsearch->text, "") && (ui_button(tr("X", NULL), UI_ALIGN_CENTER, "") || ui->is_escape_down)) {
				tab_browser_hsearch->text = "";
			}
		}

		ui_end_sticky();

		if (!string_equals(tab_browser_last_path, tab_browser_hpath->text)) {
			tab_browser_hsearch->text = "";
		}
		gc_unroot(tab_browser_last_path);
		tab_browser_last_path = string_copy(tab_browser_hpath->text);
		gc_root(tab_browser_last_path);

		f32 _y = ui->_y;
		if (show_full) {
			ui->_x = bookmarks_w;
			ui->_w -= bookmarks_w;
		}

		ui_files_file_browser(tab_browser_hpath, true, tab_browser_hsearch->text, tab_browser_refresh, &tab_browser_draw_context_menu);

		tab_browser_refresh = false;

		if (tab_browser_known) {
			char *path = tab_browser_hpath->text;
			sys_notify_on_next_frame(&tab_browser_draw_import_asset, path);
			tab_browser_hpath->text = string_copy(substring(tab_browser_hpath->text, 0, string_last_index_of(tab_browser_hpath->text, PATH_SEP)));
		}
		char *hpath_text = tab_browser_hpath->text;
		tab_browser_known =
		    string_index_of(substring(tab_browser_hpath->text, string_last_index_of(tab_browser_hpath->text, PATH_SEP), string_length(hpath_text)), ".") > 0;
#ifdef IRON_ANDROID
		if (ends_with(tab_browser_hpath->text, string(".%s", to_lower_case(manifest_title)))) {
			tab_browser_known = false;
		}
#endif
		if (tab_browser_known && iron_is_directory(tab_browser_hpath->text)) {
			tab_browser_known = false;
		}

		if (show_full) {
			i32 bottom_y = ui->_y;
			ui->_x       = 0;
			ui->_y       = _y;
			ui->_w       = bookmarks_w;

			if (ui_icon_button(tr("Cloud", NULL), ICON_CLOUD, UI_ALIGN_LEFT)) {
				tab_browser_go_to_cloud();
			}

			if (ui_icon_button(tr("Disk", NULL), ICON_STORAGE, UI_ALIGN_LEFT)) {
				tab_browser_go_to_disk();
			}

			for (i32 i = 0; i < config_raw->bookmarks->length; ++i) {
				char *b      = config_raw->bookmarks->buffer[i];
				char *folder = substring(b, string_last_index_of(b, "/") + 1, string_length(b));

				if (ui_icon_button(folder, ICON_FOLDER, UI_ALIGN_LEFT)) {
					tab_browser_hpath->text = string_copy(b);
#ifdef IRON_WINDOWS
					tab_browser_hpath->text = string_copy(string_replace_all(tab_browser_hpath->text, "/", "\\"));
#endif
				}

				if (ui->is_hovered && ui->input_released_r) {
					gc_unroot(_tab_browser_draw_b);
					_tab_browser_draw_b = string_copy(b);
					gc_root(_tab_browser_draw_b);
					ui_menu_draw(&tab_browser_draw_bookmark_menu, -1, -1);
				}
			}
			if (ui->_y < bottom_y) {
				ui->_y = bottom_y;
			}
		}
	}
}

void tab_browser_go_to_cloud() {
	tab_browser_hpath->text = "cloud";
}

#ifdef IRON_ANDROID

void tab_browser_go_to_disk_android_menu() {
	if (ui_menu_button(tr("Download", NULL), "", ICON_FOLDER)) {
		tab_browser_hpath->text = string_copy(ui_files_default_path);
	}
	if (ui_menu_button(tr("Pictures", NULL), "", ICON_FOLDER)) {
		tab_browser_hpath->text = "/storage/emulated/0/Pictures";
	}
	if (ui_menu_button(tr("Camera", NULL), "", ICON_FOLDER)) {
		tab_browser_hpath->text = "/storage/emulated/0/DCIM/Camera";
	}
	if (ui_menu_button(tr("Projects", NULL), "", ICON_FOLDER)) {
		tab_browser_hpath->text = string_copy(iron_internal_save_path());
	}
}

#endif

void tab_browser_go_to_disk() {
#ifdef IRON_ANDROID
	ui_menu_draw(&tab_browser_go_to_disk_android_menu, -1, -1);
#else
	tab_browser_hpath->text = string_copy(ui_files_default_path);
#endif
}
