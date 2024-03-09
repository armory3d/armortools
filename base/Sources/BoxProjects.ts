
class BoxProjects {

	static htab: zui_handle_t = zui_handle_create();
	static hsearch: zui_handle_t = zui_handle_create();
	static icon_map: Map<string, image_t> = null;

	static show = () => {
		if (BoxProjects.icon_map != null) {
			for (let handle of BoxProjects.icon_map.keys()) {
				data_delete_image(handle);
			}
			BoxProjects.icon_map = null;
		}

		let draggable: bool;
		///if (krom_android || krom_ios)
		draggable = false;
		///else
		draggable = true;
		///end

		UIBox.show_custom((ui: zui_t) => {
			///if (krom_android || krom_ios)
			BoxProjects.align_to_fullscreen();
			///end

			///if (krom_android || krom_ios)
			BoxProjects.projects_tab(ui);
			BoxProjects.get_started_tab(ui);
			///else
			BoxProjects.recent_projects_tab(ui);
			///end

		}, 600, 400, null, draggable);
	}

	static projects_tab = (ui: zui_t) => {
		if (zui_tab(BoxProjects.htab, tr("Projects"), true)) {
			zui_begin_sticky();

			BoxProjects.draw_badge(ui);

			if (zui_button(tr("New"))) {
				project_new();
				viewport_scale_to_bounds();
				UIBox.hide();
				// Pick unique name
				let i: i32 = 0;
				let j: i32 = 0;
				let title: string = tr("untitled") + i;
				while (j < config_raw.recent_projects.length) {
					let base: string = config_raw.recent_projects[j];
					base = base.substring(base.lastIndexOf(path_sep) + 1, base.lastIndexOf("."));
					j++;
					if (title == base) {
						i++;
						title = tr("untitled") + i;
						j = 0;
					}
				}
				sys_title_set(title);
			}
			zui_end_sticky();
			zui_separator(3, false);

			let slotw: i32 = Math.floor(150 * zui_SCALE(ui));
			let num: i32 = Math.floor(sys_width() / slotw);
			let recent_projects: string[] = config_raw.recent_projects;
			let show_asset_names: bool = true;

			for (let row: i32 = 0; row < Math.ceil(recent_projects.length / num); ++row) {
				let mult = show_asset_names ? 2 : 1;
				let ar: f32[] = [];
				for (let i: i32 = 0; i < num * mult; ++i) ar.push(1 / num);
				zui_row(ar);

				ui._x += 2;
				let off: f32 = show_asset_names ? zui_ELEMENT_OFFSET(ui) * 16.0 : 6;
				if (row > 0) ui._y += off;

				for (let j: i32 = 0; j < num; ++j) {
					let imgw: i32 = Math.floor(128 * zui_SCALE(ui));
					let i: i32 = j + row * num;
					if (i >= recent_projects.length) {
						zui_end_element(imgw);
						if (show_asset_names) zui_end_element(0);
						continue;
					}

					let path: string = recent_projects[i];

					///if krom_ios
					let document_directory: string = krom_save_dialog("", "");
					document_directory = document_directory.substr(0, document_directory.length - 8); // Strip /'untitled'
					path = document_directory + path;
					///end

					let icon_path: string = path.substr(0, path.length - 4) + "_icon.png";
					if (BoxProjects.icon_map == null) BoxProjects.icon_map = new Map();
					let icon: image_t = BoxProjects.icon_map.get(icon_path);
					if (icon == null) {
						let image: image_t = data_get_image(icon_path);
						icon = image;
						BoxProjects.icon_map.set(icon_path, icon);
					}

					let uix: i32 = ui._x;
					if (icon != null) {
						zui_fill(0, 0, 128, 128, ui.t.SEPARATOR_COL);

						let state: i32 = zui_image(icon, 0xffffffff, 128  * zui_SCALE(ui));
						if (state == zui_state_t.RELEASED) {
							let _uix: i32 = ui._x;
							ui._x = uix;
							zui_fill(0, 0, 128, 128, 0x66000000);
							ui._x = _uix;
							let doImport = () => {
								app_notify_on_init(() => {
									UIBox.hide();
									ImportArm.run_project(path);
								});
							}

							///if (krom_android || krom_ios)
							base_notify_on_next_frame(() => {
								console_toast(tr("Opening project"));
								base_notify_on_next_frame(doImport);
							});
							///else
							doImport();
							///end
						}

						let name: string = path.substring(path.lastIndexOf(path_sep) + 1, path.lastIndexOf("."));
						if (ui.is_hovered && ui.input_released_r) {
							UIMenu.draw((ui: zui_t) => {
								// if (UIMenu.menuButton(ui, tr("Duplicate"))) {}
								if (UIMenu.menu_button(ui, tr("Delete"))) {
									app_notify_on_init(() => {
										file_delete(path);
										file_delete(icon_path);
										let data_path: string = path.substr(0, path.length - 4);
										file_delete(data_path);
										recent_projects.splice(i, 1);
									});
								}
							}, 1);
						}

						if (show_asset_names) {
							ui._x = uix - (150 - 128) / 2;
							ui._y += slotw * 0.9;
							zui_text(name, zui_align_t.CENTER);
							if (ui.is_hovered) zui_tooltip(name);
							ui._y -= slotw * 0.9;
							if (i == recent_projects.length - 1) {
								ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
							}
						}
					}
					else {
						zui_end_element(0);
						if (show_asset_names) zui_end_element(0);
						ui._x = uix;
					}
				}

				ui._y += 150;
			}
		}
	}

	static recent_projects_tab = (ui: zui_t) => {
		if (zui_tab(BoxProjects.htab, tr("Recent"), true)) {

			BoxProjects.draw_badge(ui);

			ui.enabled = config_raw.recent_projects.length > 0;
			BoxProjects.hsearch.text = zui_text_input(BoxProjects.hsearch, tr("Search"), zui_align_t.LEFT, true, true);
			ui.enabled = true;

			for (let path of config_raw.recent_projects) {
				let file: string = path;
				///if krom_windows
				file = string_replace_all(path, "/", "\\");
				///else
				file = string_replace_all(path, "\\", "/");
				///end
				file = file.substr(file.lastIndexOf(path_sep) + 1);

				if (file.toLowerCase().indexOf(BoxProjects.hsearch.text.toLowerCase()) < 0) continue; // Search filter

				if (zui_button(file, zui_align_t.LEFT) && file_exists(path)) {
					let current: image_t = _g2_current;
					if (current != null) g2_end();

					ImportArm.run_project(path);

					if (current != null) g2_begin(current);
					UIBox.hide();
				}
				if (ui.is_hovered) zui_tooltip(path);
			}

			ui.enabled = config_raw.recent_projects.length > 0;
			if (zui_button(tr("Clear"), zui_align_t.LEFT)) {
				config_raw.recent_projects = [];
				config_save();
			}
			ui.enabled = true;

			zui_end_element();
			if (zui_button(tr("New .."), zui_align_t.LEFT)) project_new_box();
			if (zui_button(tr("Open..."), zui_align_t.LEFT)) project_open();
		}
	}

	static draw_badge = (ui: zui_t) => {
		let img: image_t = data_get_image("badge.k");
		zui_image(img);
		zui_end_element();
	}

	static get_started_tab = (ui: zui_t) => {
		if (zui_tab(BoxProjects.htab, tr("Get Started"), true)) {
			if (zui_button(tr("Manual"))) {
				file_load_url(manifest_url + "/manual");
			}
			if (zui_button(tr("How To"))) {
				file_load_url(manifest_url + "/howto");
			}
			if (zui_button(tr("What's New"))) {
				file_load_url(manifest_url + "/notes");
			}
		}
	}

	static align_to_fullscreen = () => {
		UIBox.modalw = Math.floor(sys_width() / zui_SCALE(base_ui_box));
		UIBox.modalh = Math.floor(sys_height() / zui_SCALE(base_ui_box));
		let appw: i32 = sys_width();
		let apph: i32 = sys_height();
		let mw: i32 = appw;
		let mh: i32 = apph;
		UIBox.hwnd.drag_x = Math.floor(-appw / 2 + mw / 2);
		UIBox.hwnd.drag_y = Math.floor(-apph / 2 + mh / 2);
	}
}
