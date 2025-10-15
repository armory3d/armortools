
type draw_cloud_icon_data_t = {
	f: string; image : gpu_texture_t;
};

let ui_files_default_path: string =
    /// if arm_windows
    "C:\\Users"
/// elseif arm_android
"/storage/emulated/0/Download"
/// elseif arm_macos
"/Users"
/// else
"/"
    /// end
    ;

let ui_files_filename: string;
let ui_files_path: string                           = ui_files_default_path;
let ui_files_last_path: string                      = "";
let ui_files_last_search: string                    = "";
let ui_files_files: string[]                        = null;
let ui_files_icon_map: map_t<string, gpu_texture_t> = null;
let ui_files_icon_file_map: map_t<string, string>   = null;
let ui_files_selected: i32                          = -1;
let ui_files_show_extensions: bool                  = false;
let ui_files_offline: bool                          = false;
let _ui_files_file_browser_handle: ui_handle_t;
let _ui_files_file_browser_f: string;

function ui_files_show(filters: string, is_save: bool, open_multiple: bool, files_done: (s: string) => void) {
	if (is_save) {
		ui_files_path = iron_save_dialog(filters, "");
		if (ui_files_path != null) {
			let sep2: string = path_sep + path_sep;
			while (string_index_of(ui_files_path, sep2) >= 0) {
				ui_files_path = string_replace_all(ui_files_path, sep2, path_sep);
			}
			ui_files_path     = string_replace_all(ui_files_path, "\r", "");
			ui_files_filename = substring(ui_files_path, string_last_index_of(ui_files_path, path_sep) + 1, ui_files_path.length);
			ui_files_path     = substring(ui_files_path, 0, string_last_index_of(ui_files_path, path_sep));
			files_done(ui_files_path);
		}
	}
	else {
		let paths: string[] = iron_open_dialog(filters, "", open_multiple);
		if (paths != null) {
			for (let i: i32 = 0; i < paths.length; ++i) {
				let path: string = paths[i];
				let sep2: string = path_sep + path_sep;
				while (string_index_of(path, sep2) >= 0) {
					path = string_replace_all(path, sep2, path_sep);
				}
				path              = string_replace_all(path, "\r", "");
				ui_files_filename = substring(path, string_last_index_of(path, path_sep) + 1, path.length);
				files_done(path);
			}
		}
	}

	ui_files_release_keys();
}

function ui_files_release_keys() {
	// File dialog may prevent firing key up events
	keyboard_up_listener(key_code_t.SHIFT);
	keyboard_up_listener(key_code_t.CONTROL);
	/// if arm_macos
	keyboard_up_listener(key_code_t.META);
	/// end
}

function make_draw_cloud_icon_data(f: string, image: gpu_texture_t): draw_cloud_icon_data_t {
	let data: draw_cloud_icon_data_t = {f : f, image : image};
	return data;
}

function ui_files_file_browser(handle: ui_handle_t, drag_files: bool = false, search: string = "", refresh: bool = false,
                               context_menu: (s: string) => void = null): string {

	let icons: gpu_texture_t = resource_get("icons.k");
	let folder: rect_t       = resource_tile50(icons, 2, 1);
	let file: rect_t         = resource_tile50(icons, 3, 1);
	let is_cloud: bool       = starts_with(handle.text, "cloud");

	if (is_cloud && file_cloud == null) {
		file_init_cloud(function() {
			ui_base_hwnds[tab_area_t.STATUS].redraws = 3;
			tab_browser_refresh                      = true;
		});
	}
	if (is_cloud && file_read_directory("cloud").length == 0) {
		return handle.text;
	}

	/// if arm_ios
	let document_directory: string = iron_save_dialog("", "");
	document_directory             = substring(document_directory, 0, document_directory.length - 8); // Strip /"untitled"
	/// end

	if (handle.text == "") {
		handle.text = ui_files_default_path;
	}

	if (handle.text != ui_files_last_path || search != ui_files_last_search || refresh) {
		ui_files_files = [];

		let dir_path: string = handle.text;
		/// if arm_ios
		if (!is_cloud) {
			dir_path = document_directory + dir_path;
		}
		/// end
		let files_all: string[] = file_read_directory(dir_path);

		for (let i: i32 = 0; i < files_all.length; ++i) {
			let f: string = files_all[i];
			if (f == "" || char_at(f, 0) == ".") {
				continue; // Skip hidden
			}
			if (string_index_of(f, ".") > 0 && !path_is_known(f)) {
				continue; // Skip unknown extensions
			}
			if (is_cloud && string_index_of(f, "_icon.") >= 0) {
				continue; // Skip thumbnails
			}
			if (string_index_of(to_lower_case(f), to_lower_case(search)) < 0) {
				continue; // Search filter
			}
			array_push(ui_files_files, f);
		}
	}
	ui_files_last_path   = handle.text;
	ui_files_last_search = search;
	handle.changed       = false;

	let slotw: i32 = math_floor(70 * UI_SCALE());
	let num: i32   = math_floor(ui._w / slotw);
	if (num == 0) {
		return handle.text;
	}

	ui._y += 4; // Don't cut off the border around selected materials
	// Directory contents
	for (let row: i32 = 0; row < math_floor(math_ceil(ui_files_files.length / num)); ++row) {
		let ar: f32[] = [];
		for (let i: i32 = 0; i < num * 2; ++i) {
			array_push(ar, 1 / num);
		}
		ui_row(ar);
		if (row > 0) {
			ui._y += UI_ELEMENT_OFFSET() * 14.0;
		}

		for (let j: i32 = 0; j < num; ++j) {
			let i: i32 = j + row * num;
			if (i >= ui_files_files.length) {
				ui_end_element_of_size(slotw);
				ui_end_element_of_size(slotw);
				continue;
			}

			let f: string = ui_files_files[i];
			let _x: f32   = ui._x;

			let rect: rect_t = string_index_of(f, ".") > 0 ? file : folder;
			let col: i32     = rect == file ? ui.ops.theme.LABEL_COL : ui.ops.theme.LABEL_COL - 0x00202020;
			if (ui_files_selected == i)
				col = ui.ops.theme.HIGHLIGHT_COL;

			let off: f32 = ui._w / 2 - 25 * UI_SCALE();
			ui._x += off;

			let uix: f32            = ui._x;
			let uiy: f32            = ui._y;
			let state: ui_state_t   = ui_state_t.IDLE;
			let generic: bool       = true;
			let icon: gpu_texture_t = null;

			if (is_cloud && !ui_files_offline) {
				if (ui_files_icon_map == null) {
					ui_files_icon_map = map_create();
				}
				if (ui_files_icon_file_map == null) {
					ui_files_icon_file_map = map_create();
				}
				icon = map_get(ui_files_icon_map, handle.text + path_sep + f);
				if (icon == null) {
					let dot: i32 = string_last_index_of(f, ".");
					if (dot > -1) {
						let files_all: string[] = file_read_directory(handle.text);
						let icon_file: string   = substring(f, 0, dot) + "_icon.jpg";
						if (array_index_of(files_all, icon_file) >= 0) {
							let rt: render_target_t  = map_get(render_path_render_targets, "empty_black");
							let empty: gpu_texture_t = rt._image;
							map_set(ui_files_icon_map, handle.text + path_sep + f, empty);

							_ui_files_file_browser_handle = handle;
							map_set(ui_files_icon_file_map, icon_file, f);

							file_cache_cloud(handle.text + path_sep + icon_file, function(abs: string) {
								if (abs != null) {
									let image: gpu_texture_t = data_get_image(abs);
									/// if arm_windows
									abs = string_replace_all(abs, "\\", "/");
									/// end
									let icon_file: string            = substring(abs, string_last_index_of(abs, "/") + 1, abs.length);
									let f: string                    = map_get(ui_files_icon_file_map, icon_file);
									let data: draw_cloud_icon_data_t = make_draw_cloud_icon_data(f, image);

									sys_notify_on_next_frame(function(data: draw_cloud_icon_data_t) {
										let icon: gpu_texture_t = gpu_create_render_target(data.image.width, data.image.height);
										if (ends_with(data.f, ".arm")) { // Used for material sphere alpha cutout
											draw_begin(icon);
											draw_image(project_materials[0].image, 0, 0);
										}
										else {
											draw_begin(icon, true, 0xffffffff);
										}
										draw_set_pipeline(pipes_copy_rgb);
										draw_image(data.image, 0, 0);
										draw_set_pipeline(null);
										draw_end();

										map_set(ui_files_icon_map, _ui_files_file_browser_handle.text + path_sep + data.f, icon);
										ui_base_hwnds[tab_area_t.STATUS].redraws = 3;
									}, data);
								}
								else {
									ui_files_offline = true;
								}
							});
						}
					}
				}
				if (icon != null) {
					let w: i32 = 50;
					if (i == ui_files_selected) {
						ui_fill(-2, -2, w + 4, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(-2, w + 2, w + 4, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(-2, 0, 2, w + 4, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(w + 2, -2, 2, w + 6, ui.ops.theme.HIGHLIGHT_COL);
					}
					state = ui_image(icon, 0xffffffff, w * UI_SCALE());
					if (ui.is_hovered) {
						ui_tooltip_image(icon);
						ui_tooltip(f);
					}
					generic = false;
				}
			}
			if (ends_with(f, ".arm") && !is_cloud) {
				if (ui_files_icon_map == null) {
					ui_files_icon_map = map_create();
				}
				let key: string = handle.text + path_sep + f;
				icon            = map_get(ui_files_icon_map, key);
				if (map_get(ui_files_icon_map, key) == null) {
					let blob_path: string = key;

					/// if arm_ios
					blob_path = document_directory + blob_path;
					/// end

					let buffer: buffer_t      = iron_load_blob(blob_path);
					let raw: project_format_t = armpack_decode(buffer);
					if (raw.material_icons != null) {
						let bytes_icon: any = raw.material_icons[0];
						icon                = gpu_create_texture_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
					}
					else if (raw.mesh_icons != null) {
						let bytes_icon: any = raw.mesh_icons[0];
						icon                = gpu_create_texture_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
					}
					else if (raw.brush_icons != null) {
						let bytes_icon: any = raw.brush_icons[0];
						icon                = gpu_create_texture_from_bytes(lz4_decode(bytes_icon, 256 * 256 * 4), 256, 256);
					}

					map_set(ui_files_icon_map, key, icon);
				}
				if (icon != null) {
					let w: i32 = 50;
					if (i == ui_files_selected) {
						ui_fill(-2, -2, w + 4, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(-2, w + 2, w + 4, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(-2, 0, 2, w + 4, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(w + 2, -2, 2, w + 6, ui.ops.theme.HIGHLIGHT_COL);
					}
					state = ui_image(icon, 0xffffffff, w * UI_SCALE());
					if (ui.is_hovered) {
						ui_tooltip_image(icon);
						ui_tooltip(f);
					}
					generic = false;
				}
			}

			if (path_is_texture(f) && !is_cloud) {
				let w: i32 = 50;
				if (ui_files_icon_map == null) {
					ui_files_icon_map = map_create();
				}
				let shandle: string = handle.text + path_sep + f;
				/// if arm_ios
				shandle = document_directory + shandle;
				/// end
				icon = map_get(ui_files_icon_map, shandle);
				if (icon == null) {
					let rt: render_target_t  = map_get(render_path_render_targets, "empty_black");
					let empty: gpu_texture_t = rt._image;
					map_set(ui_files_icon_map, shandle, empty);
					let image: gpu_texture_t = data_get_image(shandle);

					let args: ui_files_make_icon_t = {image : image, shandle : shandle, w : w};
					sys_notify_on_next_frame(ui_files_make_icon, args);
				}
				if (icon != null) {
					if (i == ui_files_selected) {
						ui_fill(-2, -2, w + 4, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(-2, w + 2, w + 4, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(-2, 0, 2, w + 4, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(w + 2, -2, 2, w + 6, ui.ops.theme.HIGHLIGHT_COL);
					}
					state   = ui_image(icon, 0xffffffff, icon.height * UI_SCALE());
					generic = false;
				}
			}

			if (generic) {
				state = ui_sub_image(icons, col, 50 * UI_SCALE(), rect.x, rect.y, rect.w, rect.h);
			}

			if (ui.is_hovered && ui.input_released_r && context_menu != null) {
				context_menu(handle.text + path_sep + f);
			}

			if (state == ui_state_t.STARTED) {
				if (drag_files) {
					base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
					base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
					base_drag_file  = handle.text;
					/// if arm_ios
					if (!is_cloud) {
						base_drag_file = document_directory + base_drag_file;
					}
					/// end
					if (char_at(base_drag_file, base_drag_file.length - 1) != path_sep) {
						base_drag_file += path_sep;
					}
					base_drag_file += f;
					base_drag_file_icon = icon;
				}

				ui_files_selected = i;
				if (sys_time() - context_raw.select_time < 0.2) {
					base_drag_file      = null;
					base_drag_file_icon = null;
					base_is_dragging    = false;
					handle.changed = ui.changed = true;
					if (char_at(handle.text, handle.text.length - 1) != path_sep) {
						handle.text += path_sep;
					}
					handle.text += f;
					ui_files_selected = -1;
				}
				context_raw.select_time = sys_time();
			}

			// Label
			ui._x = _x;
			ui._y += slotw * 0.75;
			let label0: string = (ui_files_show_extensions || string_index_of(f, ".") <= 0) ? f : substring(f, 0, string_last_index_of(f, "."));
			let label1: string = "";
			while (label0.length > 0 && draw_string_width(ui.ops.font, ui.font_size, label0) > ui._w - 6) { // 2 line split
				label1 = char_at(label0, label0.length - 1) + label1;
				label0 = substring(label0, 0, label0.length - 1);
			}
			if (label1 != "") {
				ui.current_ratio--;
			}
			ui_text(label0, ui_align_t.CENTER);
			if (ui.is_hovered) {
				ui_tooltip(label0 + label1);
			}
			if (label1 != "") { // Second line
				ui._x = _x;
				ui._y += draw_font_height(ui.ops.font, ui.font_size);
				ui_text(label1, ui_align_t.CENTER);
				if (ui.is_hovered) {
					ui_tooltip(label0 + label1);
				}
				ui._y -= draw_font_height(ui.ops.font, ui.font_size);
			}

			ui._y -= slotw * 0.75;

			if (handle.changed) {
				break;
			}
		}

		if (handle.changed) {
			break;
		}
	}
	ui._y += slotw * 0.8;

	return handle.text;
}

function ui_files_make_icon(args: ui_files_make_icon_t) {
	let w: i32               = args.w;
	let image: gpu_texture_t = args.image;
	let sw: i32              = image.width > image.height ? w : math_floor(1.0 * image.width / image.height * w);
	let sh: i32              = image.width > image.height ? math_floor(1.0 * image.height / image.width * w) : w;
	let icon: gpu_texture_t  = gpu_create_render_target(sw, sh);
	draw_begin(icon, true, 0xffffffff);
	draw_set_pipeline(pipes_copy_rgb);
	draw_scaled_image(image, 0, 0, sw, sh);
	draw_set_pipeline(null);
	draw_end();
	map_set(ui_files_icon_map, args.shandle, icon);
	ui_base_hwnds[tab_area_t.STATUS].redraws = 3;

	let found: bool = false;
	for (let i: i32 = 0; i < project_assets.length; ++i) {
		let a: asset_t = project_assets[i];
		if (a.file == args.shandle) {
			found = true;
			break;
		}
	}

	if (!found) {
		data_delete_image(args.shandle); // The big image is not needed anymore
	}
}

function ui_files_go_up(handle: ui_handle_t) {
	handle.text = substring(handle.text, 0, string_last_index_of(handle.text, path_sep));
	// Drive root
	if (handle.text.length == 2 && char_at(handle.text, 1) == ":") {
		handle.text += path_sep;
	}
}

type ui_files_make_icon_t = {
	image: gpu_texture_t; shandle : string; w : i32;
};
