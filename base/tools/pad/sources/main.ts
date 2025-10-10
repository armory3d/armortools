
type storage_t = {
	project?: string;
	file?: string;
	text?: string;
	modified?: bool;
	expanded?: string[];
	window_w?: i32;
	window_h?: i32;
	window_x?: i32;
	window_y?: i32;
	sidebar_w?: i32;
};

let ui: ui_t;
let theme: ui_theme_t;
let text_handle: ui_handle_t = ui_handle_create();
let sidebar_handle: ui_handle_t = ui_handle_create();
let editor_handle: ui_handle_t = ui_handle_create();
let storage: storage_t = null;
let resizing_sidebar: bool = false;
let minimap_w: i32 = 150;
let minimap_h: i32 = 0;
let minimap_box_h: i32 = 0;
let minimap_scrolling: bool = false;
let minimap: gpu_texture_t = null;
let window_header_h: i32 = 0;

function drop_files(path: string) {
	storage.project = path;
	sidebar_handle.redraws = 1;
}

function encode_storage(): string {
	json_encode_begin();
	json_encode_string("project", storage.project);
	json_encode_string("file", storage.file);
	// json_encode_string("text", storage.text);
	json_encode_string("text", "");
	json_encode_bool("modified", storage.modified);
	json_encode_string_array("expanded", storage.expanded);
	json_encode_i32("window_w", storage.window_w);
	json_encode_i32("window_h", storage.window_h);
	json_encode_i32("window_x", storage.window_x);
	json_encode_i32("window_y", storage.window_y);
	json_encode_i32("sidebar_w", storage.sidebar_w);
	let config_json: string = json_encode_end();
	return config_json;
}

function on_shutdown() {
	let storage_string: string = sys_string_to_buffer(encode_storage());
	iron_file_save_bytes(iron_internal_save_path() + "/config.json", storage_string, 0);
}

function main() {
	iron_set_app_name("ArmorPad");

	let blob_storage: buffer_t = iron_load_blob(iron_internal_save_path() + "/config.json");
	if (blob_storage == null) {
		storage = {};
		storage.project = "";
		storage.file = "untitled";
		storage.text = "";
		storage.modified = false;
		storage.expanded = [];
		storage.window_w = 1600;
		storage.window_h = 900;
		storage.window_x = -1;
		storage.window_y = -1;
		storage.sidebar_w = 230;
	}
	else {
		storage = json_parse(sys_buffer_to_string(blob_storage));
	}

	text_handle.text = storage.text;

	let ops: iron_window_options_t = {
		title: "ArmorPad",
		x: storage.window_x,
		y: storage.window_y,
		width: storage.window_w,
		height: storage.window_h,
		features: window_features_t.RESIZABLE |
				  window_features_t.MAXIMIZABLE |
				  window_features_t.MINIMIZABLE,
		mode: window_mode_t.WINDOWED,
		frequency: 60,
		vsync: true,
		display_index: 0,
		visible: true,
		color_bits: 32,
		depth_bits: 0
	};
	sys_start(ops);

	let font: draw_font_t = data_get_font("font_mono.ttf");
	draw_font_init(font);

	theme = {};
	ui_theme_default(theme);
	theme.WINDOW_BG_COL = 0xff000000;
	theme.SEPARATOR_COL = 0xff000000;
	theme.BUTTON_COL = 0xff000000;
	theme.FONT_SIZE = 18;

	let ui_ops: ui_options_t = {
		scale_factor: 1.0,
		theme: theme,
		font: font
	};
	ui = ui_create(ui_ops);

	let blob_coloring: buffer_t = data_get_blob("text_coloring.json");
	let text_coloring: ui_text_coloring_t = json_parse(sys_buffer_to_string(blob_coloring));
	ui_text_area_coloring = text_coloring;
	ui_on_border_hover = on_border_hover;
	ui_text_area_line_numbers = true;
	ui_text_area_scroll_past_end = true;

	sys_notify_on_render(render);
	_iron_set_drop_files_callback(drop_files);
	iron_set_application_state_callback(null, null, null, null, on_shutdown);
}

function armpack_to_string(bytes: buffer_t): string {
	// Use plugin api + converter.js
	return "";
}

function list_folder(path: string) {
	let files: string[] = file_read_directory(path);
	for (let i: i32 = 0; i < files.length; ++i) {
		let f: string = files[i];
		let abs: string = path + "/" + f;
		let is_file: bool = string_index_of(f, ".") >= 0;
		let is_expanded: bool = array_index_of(storage.expanded, abs) >= 0;

		// Active file
		if (abs == storage.file) {
			ui_fill(0, 1, ui._w - 1, UI_ELEMENT_H() - 1, theme.PRESSED_COL);
		}

		let prefix: string = "";
		if (!is_file) {
			prefix = is_expanded ? "- " : "+ ";
		}

		if (ui_button(prefix + f, UI_ALIGN_LEFT, "")) {
			// Open file
			if (is_file) {
				storage.file = abs;
				let bytes: buffer_t = iron_load_blob(storage.file);
				if (ends_with(f, ".arm")) {
					storage.text = armpack_to_string(bytes);
				}
				else {
					storage.text = sys_buffer_to_string(bytes);
				}
				storage.text = string_replace_all(storage.text, "\r", "");
				text_handle.text = storage.text;
				editor_handle.redraws = 1;
				iron_window_set_title(abs);
			}
			// Expand folder
			else {
				if (array_index_of(storage.expanded, abs) == -1) {
					array_push(storage.expanded, abs);
				}
				else {
					array_remove(storage.expanded, abs);
				}
			}
		}

		if (is_expanded) {
			ui._x += 16;
			list_folder(abs);
			ui._x -= 16;
		}
	}
}

function render() {
	storage.window_w = iron_window_width();
	storage.window_h = iron_window_height();
	storage.window_x = iron_window_x();
	storage.window_y = iron_window_y();
	if (ui.input_dx != 0 || ui.input_dy != 0) {
		iron_mouse_set_cursor(cursor_t.ARROW);
	}

	ui_begin(ui);

	if (ui_window(sidebar_handle, 0, 0, storage.sidebar_w, iron_window_height(), false)) {
		if (storage.project != "") {
			list_folder(storage.project);
		}
		else {
			ui_button("Drop folder here", UI_ALIGN_LEFT, "");
		}

		ui_fill(iron_window_width() - minimap_w, 0, minimap_w, UI_ELEMENT_H() + UI_ELEMENT_OFFSET() + 1, theme.SEPARATOR_COL);
		ui_fill(storage.sidebar_w, 0, 1, iron_window_height(), theme.SEPARATOR_COL);
	}

	let editor_updated: bool = false;

	if (ui_window(editor_handle, storage.sidebar_w + 1, 0, iron_window_width() - storage.sidebar_w - minimap_w, iron_window_height(), false)) {
		editor_updated = true;
		let htab: ui_handle_t = ui_handle(__ID__);
		let file_name: string = substring(storage.file, string_last_index_of(storage.file, "/") + 1, storage.file.length);
		let file_names: string[] = [file_name];

		for (let i: i32 = 0; i < file_names.length; ++i) {
			let tab_name: string = file_names[i];
			if (storage.modified) {
				tab_name += "*";
			}
			if (ui_tab(htab, tab_name, false, -1)) {
				// File modified
				if (ui.is_key_pressed) {
					storage.modified = true;
				}

				// Save
				if (ui.is_ctrl_down && ui.key_code == key_code_t.S) {
					save_file();
				}

				storage.text = ui_text_area(text_handle, UI_ALIGN_LEFT, true, "", false);
			}
		}

		window_header_h = 32;//math_floor(ui.windowHeaderH);
	}

	if (resizing_sidebar) {
		storage.sidebar_w += math_floor(ui.input_dx);
	}
	if (!ui.input_down) {
		resizing_sidebar = false;
	}

	// Minimap controls
	let minimap_x: i32 = iron_window_width() - minimap_w;
	let minimap_y: i32 = window_header_h + 1;
	let redraw: bool = false;
	if (ui.input_started && hit_test(ui.input_x, ui.input_y, minimap_x + 5, minimap_y, minimap_w, minimap_h)) {
		minimap_scrolling = true;
	}
	if (!ui.input_down) {
		minimap_scrolling = false;
	}
	if (minimap_scrolling) {
		redraw = true;
	}

	// Build project
	if (ui.is_ctrl_down && ui.key_code == key_code_t.B) {
		save_file();
		build_project();
	}

	ui_end();

	if (redraw) {
		editor_handle.redraws = 2;
	}

	if (minimap != null) {
		draw_begin();
		draw_image(minimap, minimap_x, minimap_y);
		draw_end();
	}

	if (editor_updated) {
		draw_minimap();
	}
}

function save_file() {
	// Trim
	let lines: string[] = string_split(storage.text, "\n");
	for (let i: i32 = 0; i < lines.length; ++i) {
		lines[i] = trim_end(lines[i]);
	}
	storage.text = string_array_join(lines, "\n");
	// Spaces to tabs
	storage.text = string_replace_all(storage.text, "    ", "\t");
	text_handle.text = storage.text;
	// Write bytes
	// let bytes: buffer_t;
	// if (ends_with(storage.file, ".arm")) {
	// 	armpack_encode(json_parse(storage.text));
	// }
	// else {
	// 	sys_string_to_buffer(storage.text);
	// }
	// iron_file_save_bytes(storage.file, bytes, bytes.length);
	storage.modified = false;
}

function build_file(): string {
	///if arm_windows
	return "\\build.bat";
	///else
	return "/build.sh";
	///end
}

function build_project() {
	iron_sys_command(storage.project + build_file() + " " + storage.project);
}

function draw_minimap() {
	if (minimap_h != iron_window_height()) {
		minimap_h = iron_window_height();
		if (minimap != null) {
			gpu_delete_texture(minimap);
		}
		minimap = gpu_create_render_target(minimap_w, minimap_h);
	}

	draw_begin(minimap, true, theme.SEPARATOR_COL);
	draw_set_color(0xff333333);
	let lines: string[] = string_split(storage.text, "\n");
	let minimap_full_h: i32 = lines.length * 2;
	let scroll_progress: f32 = -editor_handle.scroll_offset / (lines.length * UI_ELEMENT_H());
	let out_of_screen: i32 = minimap_full_h - minimap_h;
	if (out_of_screen < 0) {
		out_of_screen = 0;
	}
	let offset: i32 = math_floor((out_of_screen * scroll_progress) / 2);

	for (let i: i32 = 0; i < lines.length; ++i) {
		if (i * 2 > minimap_h || i + offset >= lines.length) {
			// Out of screen
			break;
		}
		let words: string[] = string_split(lines[i + offset], " ");
		let x: i32 = 0;
		for (let j: i32 = 0; j < words.length; ++j) {
			let word: string = words[j];
			draw_filled_rect(x, i * 2, word.length, 2);
			x += word.length + 1;
		}
	}

	// Current position
	let visible_area: i32 = out_of_screen > 0 ? minimap_h : minimap_full_h;
	draw_set_color(0x11ffffff);
	minimap_box_h = math_floor((iron_window_height() - window_header_h) / UI_ELEMENT_H() * 2);
	draw_filled_rect(0, scroll_progress * visible_area, minimap_w, minimap_box_h);
	draw_end();
}

function hit_test(mx: f32, my: f32, x: f32, y: f32, w: f32, h: f32): bool {
	return mx > x && mx < x + w && my > y && my < y + h;
}

function on_border_hover(handle: ui_handle_t, side: i32) {
	if (handle != sidebar_handle) {
		return;
	}
	if (side != 1) {
		return; // Right
	}

	iron_mouse_set_cursor(cursor_t.SIZEWE);

	if (ui.input_started) {
		resizing_sidebar = true;
	}
}

////
type config_t = { server: string; };
let config_raw: config_t;
function strings_check_internet_connection(): string { return ""; }
function console_error(s: string) {}
function console_info(s: string) {}
function plugin_embed() {}
function tr(id: string, vars: map_t<string, string> = null): string { return id; }
let pipes_offset: i32;
function pipes_get_constant_location(s: string): i32 { return 0; }
