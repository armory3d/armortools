
let box_projects_htab: ui_handle_t                      = ui_handle_create();
let box_projects_hsearch: ui_handle_t                   = ui_handle_create();
let box_projects_icon_map: map_t<string, gpu_texture_t> = null;
let _box_projects_path: string;
let _box_projects_icon_path: string;
let _box_projects_i: i32;

function box_projects_show() {
	if (box_projects_icon_map != null) {
		let keys: string[] = map_keys(box_projects_icon_map);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let handle: string = keys[i];
			data_delete_image(handle);
		}
		box_projects_icon_map = null;
	}

	let draggable: bool;
	/// if (arm_android || arm_ios)
	draggable = false;
	/// else
	draggable = true;
	/// end

	ui_box_show_custom(function() {
		/// if (arm_android || arm_ios)
		box_projects_align_to_fullscreen();
		/// end

		/// if (arm_android || arm_ios)
		box_projects_tab();
		box_projects_get_started_tab();
		/// else
		box_projects_recent_tab();
		/// end
	}, 600, 400, null, draggable);
}

function box_projects_tab() {
	if (ui_tab(box_projects_htab, tr("Projects"), true)) {
		ui_begin_sticky();

		box_projects_draw_badge();

		if (ui_button(tr("New"))) {
			project_new();
			ui_box_hide();
			// Pick unique name
			let i: i32        = 0;
			let j: i32        = 0;
			let title: string = "" + tr("untitled") + i;
			while (j < config_raw.recent_projects.length) {
				let base: string = config_raw.recent_projects[j];
				base             = substring(base, string_last_index_of(base, path_sep) + 1, string_last_index_of(base, "."));
				j++;
				if (title == base) {
					i++;
					title = "" + tr("untitled") + i;
					j     = 0;
				}
			}
			sys_title_set(title);
		}
		ui_end_sticky();
		ui_separator(3, false);

		let slotw: i32 = math_floor(150 * UI_SCALE());
		let num: i32   = math_floor(iron_window_width() / slotw);
		if (num == 0) {
			return;
		}
		let recent_projects: string[] = config_raw.recent_projects;
		let show_asset_names: bool    = true;

		for (let row: i32 = 0; row < math_ceil(recent_projects.length / num); ++row) {
			let mult: i32 = show_asset_names ? 2 : 1;
			let ar: f32[] = [];
			for (let i: i32 = 0; i < num * mult; ++i) {
				array_push(ar, 1 / num);
			}
			ui_row(ar);

			ui._x += 2;
			let off: f32 = show_asset_names ? UI_ELEMENT_OFFSET() * 16.0 : 6;
			if (row > 0) {
				ui._y += off;
			}

			for (let j: i32 = 0; j < num; ++j) {
				let imgw: i32 = math_floor(128 * UI_SCALE());
				let i: i32    = j + row * num;
				if (i >= recent_projects.length) {
					ui_end_element_of_size(imgw);
					if (show_asset_names) {
						ui_end_element_of_size(0);
					}
					continue;
				}

				let path: string = recent_projects[i];

				/// if arm_ios
				let document_directory: string = iron_save_dialog("", "");
				document_directory             = substring(document_directory, 0, document_directory.length - 8); // Strip /"untitled"
				path                           = document_directory + path;
				/// end

				let icon_path: string = substring(path, 0, path.length - 4) + "_icon.png";
				if (box_projects_icon_map == null) {
					box_projects_icon_map = map_create();
				}
				let icon: gpu_texture_t = map_get(box_projects_icon_map, icon_path);
				if (icon == null) {
					let image: gpu_texture_t = data_get_image(icon_path);
					icon                     = image;
					map_set(box_projects_icon_map, icon_path, icon);
				}

				let uix: i32 = ui._x;
				if (icon != null) {
					ui_fill(0, 0, 128, 128, ui.ops.theme.SEPARATOR_COL);

					let state: i32 = ui_image(icon, 0xffffffff, 128 * UI_SCALE());
					if (state == ui_state_t.RELEASED) {
						let _uix: i32 = ui._x;
						ui._x         = uix;
						ui_fill(0, 0, 128, 128, 0x66000000);
						ui._x = _uix;
						/// if (arm_android || arm_ios)
						console_toast(tr("Opening project"));
						/// end
						sys_notify_on_next_frame(function(path: string) {
							ui_box_hide();
							import_arm_run_project(path);
						}, path);
					}

					let name: string = substring(path, string_last_index_of(path, path_sep) + 1, string_last_index_of(path, "."));
					if (ui.is_hovered && ui.input_released_r) {
						_box_projects_path      = path;
						_box_projects_icon_path = icon_path;
						_box_projects_i         = i;
						ui_menu_draw(function() {
							// if (ui_menu_button(tr("Duplicate"))) {}
							if (ui_menu_button(tr("Delete"))) {
								sys_notify_on_next_frame(function() {
									iron_delete_file(_box_projects_path);
									iron_delete_file(_box_projects_icon_path);
									let data_path: string = substring(_box_projects_path, 0, _box_projects_path.length - 4);
									iron_delete_file(data_path);
									let recent_projects: string[] = config_raw.recent_projects;
									array_splice(recent_projects, _box_projects_i, 1);
								});
							}
						});
					}

					if (show_asset_names) {
						ui._x = uix - (150 - 128) / 2;
						ui._y += slotw * 0.9;
						ui_text(name, ui_align_t.CENTER);
						if (ui.is_hovered) {
							ui_tooltip(name);
						}
						ui._y -= slotw * 0.9;
						if (i == recent_projects.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
						}
					}
				}
				else {
					ui_end_element_of_size(0);
					if (show_asset_names) {
						ui_end_element_of_size(0);
					}
					ui._x = uix;
				}
			}

			ui._y += 150;
		}
	}
}

function box_projects_recent_tab() {
	if (ui_tab(box_projects_htab, tr("Recent"), true)) {

		box_projects_draw_badge();

		ui.enabled                = config_raw.recent_projects.length > 0;
		box_projects_hsearch.text = ui_text_input(box_projects_hsearch, tr("Search"), ui_align_t.LEFT, true, true);
		ui.enabled                = true;

		for (let i: i32 = 0; i < config_raw.recent_projects.length; ++i) {
			let path: string = config_raw.recent_projects[i];
			/// if arm_windows
			path = string_replace_all(path, "/", "\\");
			/// else
			path = string_replace_all(path, "\\", "/");
			/// end
			let file: string = substring(path, string_last_index_of(path, path_sep) + 1, path.length);

			if (string_index_of(to_lower_case(file), to_lower_case(box_projects_hsearch.text)) < 0) {
				continue; // Search filter
			}

			if (ui_button(file, ui_align_t.LEFT) && iron_file_exists(path)) {
				let current: gpu_texture_t = _draw_current;
				let in_use: bool           = gpu_in_use;
				if (in_use)
					draw_end();

				import_arm_run_project(path);

				if (in_use)
					draw_begin(current);
				ui_box_hide();
			}
			if (ui.is_hovered) {
				ui_tooltip(path);
			}
		}

		ui.enabled = config_raw.recent_projects.length > 0;
		if (ui_button(tr("Clear"), ui_align_t.LEFT)) {
			config_raw.recent_projects = [];
			config_save();
		}
		ui.enabled = true;

		ui_end_element();
		if (ui_button(tr("New Project..."), ui_align_t.LEFT)) {
			project_new_box();
		}
		if (ui_button(tr("Open..."), ui_align_t.LEFT)) {
			project_open();
		}
	}
}

function box_projects_draw_badge() {
	let img: gpu_texture_t = data_get_image("badge.k");
	ui_image(img);
	ui_end_element();
}

function box_projects_get_started_tab() {
	if (ui_tab(box_projects_htab, tr("Get Started"), true)) {
		if (ui_button(tr("Manual"))) {
			iron_load_url(manifest_url + "/manual");
		}
		if (ui_button(tr("How To"))) {
			iron_load_url(manifest_url + "/howto");
		}
		if (ui_button(tr("What's New"))) {
			iron_load_url(manifest_url + "/notes");
		}
	}
}

function box_projects_align_to_fullscreen() {
	ui_box_modalw      = math_floor(iron_window_width() / UI_SCALE());
	ui_box_modalh      = math_floor(iron_window_height() / UI_SCALE());
	let appw: i32      = iron_window_width();
	let apph: i32      = iron_window_height();
	let mw: i32        = appw;
	let mh: i32        = apph;
	ui_box_hwnd.drag_x = math_floor(-appw / 2 + mw / 2);
	ui_box_hwnd.drag_y = math_floor(-apph / 2 + mh / 2);
}
