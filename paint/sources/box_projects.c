
#include "global.h"

void box_projects_show_box() {
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	box_projects_align_to_fullscreen();
#endif

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	box_projects_tab();
	box_projects_get_started_tab();
#else
	box_projects_recent_tab();
#endif
}

void box_projects_show() {
	if (box_projects_icon_map != NULL) {
		string_t_array_t *keys = map_keys(box_projects_icon_map);
		for (i32 i = 0; i < keys->length; ++i) {
			char *handle = keys->buffer[i];
			data_delete_image(handle);
		}
		gc_unroot(box_projects_icon_map);
		box_projects_icon_map = NULL;
	}

	bool draggable;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	draggable = false;
#else
	draggable = true;
#endif

	ui_box_show_custom(&box_projects_show_box, 600, 400, NULL, draggable, "");
}

void box_projects_tab_menu_on_next_frame(void *_) {
	iron_delete_file(_box_projects_path);
	iron_delete_file(_box_projects_icon_path);
	char *data_path = substring(_box_projects_path, 0, string_length(_box_projects_path) - 4);
	iron_delete_file(data_path);
	string_t_array_t *recent_projects = config_raw->recent_projects;
	array_splice(recent_projects, _box_projects_i, 1);
}

void box_projects_tab_menu() {
	// if (ui_menu_button(tr("Duplicate"), "", icon_t.DUPLICATE)) {}
	if (ui_menu_button(tr("Delete"), "", ICON_DELETE)) {
		sys_notify_on_next_frame(&box_projects_tab_menu_on_next_frame, NULL);
	}
}

void box_projects_tab_on_next_frame(char *path) {
	ui_box_hide();
	import_arm_run_project(path);
}

void box_projects_tab() {
	if (ui_tab(box_projects_htab, tr("Projects"), true, -1, false)) {
		ui_begin_sticky();

		ui_separator(UI_ELEMENT_H(), false);

		box_projects_draw_badge();

		if (ui_icon_button(tr("New"), ICON_PLUS, UI_ALIGN_CENTER)) {
			project_new(true);
			ui_box_hide();
			// Pick unique name
			i32   i     = 0;
			i32   j     = 0;
			char *title = string("%s%s", tr("untitled"), i32_to_string(i));
			while (j < config_raw->recent_projects->length) {
				char *base = config_raw->recent_projects->buffer[j];
				base       = string_copy(substring(base, string_last_index_of(base, PATH_SEP) + 1, string_last_index_of(base, ".")));
				j++;
				if (string_equals(title, base)) {
					i++;
					title = string("%s%s", tr("untitled"), i32_to_string(i));
					j     = 0;
				}
			}
			sys_title_set(title);
		}
		ui_end_sticky();
		ui_separator(3, false);

		i32 slotw = math_floor(150 * UI_SCALE());
		i32 num   = math_floor(iron_window_width() / (float)slotw);
		if (num == 0) {
			return;
		}
		string_t_array_t *recent_projects  = config_raw->recent_projects;
		bool              show_asset_names = true;

		for (i32 row = 0; row < math_ceil(recent_projects->length / (float)num); ++row) {
			i32          mult = show_asset_names ? 2 : 1;
			f32_array_t *ar   = f32_array_create_from_raw((f32[]){}, 0);
			for (i32 i = 0; i < num * mult; ++i) {
				f32_array_push(ar, 1 / (float)num);
			}
			ui_row(ar);

			ui->_x += 2;
			f32 off = show_asset_names ? 96 * UI_SCALE() : 16 * UI_SCALE();
			if (row > 0) {
				ui->_y += off;
			}

			for (i32 j = 0; j < num; ++j) {
				i32 imgw = math_floor(128 * UI_SCALE());
				i32 i    = j + row * num;
				if (i >= recent_projects->length) {
					ui_end_element_of_size(imgw);
					if (show_asset_names) {
						ui_end_element_of_size(0);
					}
					continue;
				}

				char *path = recent_projects->buffer[i];

#ifdef IRON_IOS
				char *document_directory = iron_save_dialog("", "");
				document_directory       = string_copy(substring(document_directory, 0, string_length(document_directory) - 8)); // Strip /"untitled"
				path                     = string("%s%s", document_directory, path);
#endif

				char *icon_path = string("%s_icon.png", substring(path, 0, string_length(path) - 4));
				if (box_projects_icon_map == NULL) {
					gc_unroot(box_projects_icon_map);
					box_projects_icon_map = any_map_create();
					gc_root(box_projects_icon_map);
				}
				gpu_texture_t *icon = any_map_get(box_projects_icon_map, icon_path);
				if (icon == NULL) {
					gpu_texture_t *image = data_get_image(icon_path);
					icon                 = image;
					any_map_set(box_projects_icon_map, icon_path, icon);
				}

				i32 uix = ui->_x;
				if (icon != NULL) {
					ui_fill(0, 0, 128, 128, ui->ops->theme->SEPARATOR_COL);

					i32 state = ui_image(icon, 0xffffffff, 128 * UI_SCALE());
					if (state == UI_STATE_RELEASED) {
						i32 _uix = ui->_x;
						ui->_x   = uix;
						ui_fill(0, 0, 128, 128, 0x66000000);
						ui->_x = _uix;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
						console_toast(tr("Opening project"));
#endif
						sys_notify_on_next_frame(&box_projects_tab_on_next_frame, path);
					}

					char *name = substring(path, string_last_index_of(path, PATH_SEP) + 1, string_last_index_of(path, "."));
					if (ui->is_hovered && ui->input_released_r) {
						gc_unroot(_box_projects_path);
						_box_projects_path = string_copy(path);
						gc_root(_box_projects_path);
						gc_unroot(_box_projects_icon_path);
						_box_projects_icon_path = string_copy(icon_path);
						gc_root(_box_projects_icon_path);
						_box_projects_i = i;
						ui_menu_draw(&box_projects_tab_menu, -1, -1);
					}

					if (show_asset_names) {
						ui->_x = uix - (150 - 128) / (float)2;
						ui->_y += slotw * 0.9;
						ui_text(name, UI_ALIGN_CENTER, 0x00000000);
						if (ui->is_hovered) {
							ui_tooltip(name);
						}
						ui->_y -= slotw * 0.9;
						if (i == recent_projects->length - 1) {
							ui->_y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
						}
					}
				}
				else {
					ui_end_element_of_size(0);
					if (show_asset_names) {
						ui_end_element_of_size(0);
					}
					ui->_x = uix;
				}
			}

			ui->_y += 150;
		}
	}
}

void box_projects_recent_tab() {
	if (ui_tab(box_projects_htab, tr("Recent"), true, -1, false)) {

		box_projects_draw_badge();

		ui->enabled                = config_raw->recent_projects->length > 0;
		box_projects_hsearch->text = string_copy(ui_text_input(box_projects_hsearch, tr("Search"), UI_ALIGN_LEFT, true, true));
		ui->enabled                = true;
		for (i32 i = 0; i < config_raw->recent_projects->length; ++i) {
			char *path = config_raw->recent_projects->buffer[i];
#ifdef IRON_WINDOWS
			path = string_copy(string_replace_all(path, "/", "\\"));
#else
			path = string_copy(string_replace_all(path, "\\", "/"));
#endif
			char *file = substring(path, string_last_index_of(path, PATH_SEP) + 1, string_length(path));

			if (string_index_of(to_lower_case(file), to_lower_case(box_projects_hsearch->text)) < 0) {
				continue; // Search filter
			}
			if (ui_button(file, UI_ALIGN_LEFT, "") && iron_file_exists(path)) {
				gpu_texture_t *current = _draw_current;
				bool           in_use  = gpu_in_use;
				if (in_use)
					draw_end();

				import_arm_run_project(path);

				if (in_use)
					draw_begin(current, false, 0);
				ui_box_hide();
			}
			if (ui->is_hovered) {
				ui_tooltip(path);
			}
		}

		ui->enabled = config_raw->recent_projects->length > 0;
		if (ui_icon_button(tr("Clear"), ICON_ERASE, UI_ALIGN_LEFT)) {
			config_raw->recent_projects = any_array_create_from_raw((void *[]){}, 0);
			config_save();
		}
		ui->enabled = true;

		ui_end_element();
		if (ui_icon_button(tr("New Project..."), ICON_FILE_NEW, UI_ALIGN_LEFT)) {
			project_new_box();
		}
		if (ui_icon_button(tr("Open..."), ICON_FOLDER_OPEN, UI_ALIGN_LEFT)) {
			project_open();
		}
	}
}

void box_projects_draw_badge() {
	gpu_texture_t *img = data_get_image("badge.k");
	ui_image(img, 0xffffffff, -1.0);
	ui_end_element();
}

void box_projects_get_started_tab() {
	if (ui_tab(box_projects_htab, tr("Get Started"), true, -1, false)) {

		ui_separator(UI_ELEMENT_H(), false);

		if (ui_icon_button(tr("Manual"), ICON_HELP, UI_ALIGN_CENTER)) {
			iron_load_url(string("%s/manual", manifest_url));
		}
		if (ui_icon_button(tr("How To"), ICON_HELP, UI_ALIGN_CENTER)) {
			iron_load_url(string("%s/howto", manifest_url));
		}
		if (ui_icon_button(tr("What's New"), ICON_LINK, UI_ALIGN_CENTER)) {
			iron_load_url(string("%s/notes", manifest_url));
		}
	}
}

void box_projects_align_to_fullscreen() {
	ui_box_modalw       = math_floor(iron_window_width() / (float)UI_SCALE());
	ui_box_modalh       = math_floor(iron_window_height() / (float)UI_SCALE());
	i32 appw            = iron_window_width();
	i32 apph            = iron_window_height();
	i32 mw              = appw;
	i32 mh              = apph;
	ui_box_hwnd->drag_x = math_floor(-appw / (float)2 + mw / (float)2);
	ui_box_hwnd->drag_y = math_floor(-apph / (float)2 + mh / (float)2);
}
