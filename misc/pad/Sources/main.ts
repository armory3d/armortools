
type storage_t = {
	project: string;
	file: string;
	text: string;
	modified: bool;
	expanded: string[];
	window_w: i32;
	window_h: i32;
	window_x: i32;
	window_y: i32;
	sidebar_w: i32;
};

let ui: zui_t;
let text_handle = zui_handle_create();
let sidebar_handle = zui_handle_create();
let editor_handle = zui_handle_create();
let storage: storage_t = null;
let resizing_sidebar = false;
let minimap_w = 150;
let minimap_h = 0;
let minimap_box_h = 0;
let minimap_scrolling = false;
let minimap: image_t = null;
let window_header_h = 0;

function main() {
	zui_set_text_area_line_numbers(true);
	zui_set_text_area_scroll_past_end(true);

	krom_set_application_name("ArmorPad");
	let blob_storage = krom_load_blob(krom_save_path() + "/config.json");
	if (blob_storage == null) {
		storage = {
			project: "",
			file: "untitled",
			text: "",
			modified: false,
			expanded: [],
			window_w: 1600,
			window_h: 900,
			window_x: -1,
			window_y: -1,
			sidebar_w: 230,
		};
	}
	else {
		storage = JSON.parse(sys_buffer_to_string(blob_storage));
	}

	text_handle.text = storage.text;
	let ops: kinc_sys_ops_t = {
		title: "ArmorPad",
		x: storage.window_x,
		y: storage.window_y,
		width: storage.window_w,
		height: storage.window_h,
		features: WindowFeatures.FeatureResizable | WindowFeatures.FeatureMaximizable | WindowFeatures.FeatureMinimizable,
		mode: WindowMode.Windowed,
		frequency: 60,
		vsync: true
	};

	sys_start(ops, function() {
		data_get_font("font_mono.ttf", function (font: font_t) {
			data_get_blob("themes/dark.json", function (blob_theme: ArrayBuffer) {
				data_get_blob("text_coloring.json", function (blob_coloring: ArrayBuffer) {

					let parsed = JSON.parse(sys_buffer_to_string(blob_theme));
					let theme: any = zui_theme_create();
					for (let key of Object.getOwnPropertyNames(theme_t.prototype)) {
						if (key == "constructor") {
							continue;
						}
						theme[key] = parsed[key];
					}

					g2_font_init(font);
					ui = zui_create({ theme: theme, font: font, scaleFactor: 1.0, color_wheel: null, black_white_gradient: null });
					zui_set_on_border_hover(on_border_hover);
					zui_set_on_text_hover(on_text_hover);

					let text_coloring: zui_text_coloring_t = JSON.parse(sys_buffer_to_string(blob_coloring));
					text_coloring.default_color = Math.floor(text_coloring.default_color);
					for (let coloring of text_coloring.colorings) {
						coloring.color = Math.floor(coloring.color);
					}
					zui_set_text_area_coloring(text_coloring);

					sys_notify_on_frames(render);
				});
			});
		});
	});

	krom_set_drop_files_callback(function (path: string) {
		storage.project = path;
		sidebar_handle.redraws = 1;
	});

	krom_set_application_state_callback(
		function() {},
		function() {},
		function() {},
		function() {},
		function () { // Shutdown
			krom_file_save_bytes(krom_save_path() + "/config.json", sys_string_to_buffer(JSON.stringify(storage)));
		}
	);
}

function list_folder(path: string) {
	let files = krom_read_directory(path, false).split("\n");
	for (let f of files) {
		let abs = path + "/" + f;
		let is_file = f.indexOf(".") >= 0;
		let is_expanded = storage.expanded.indexOf(abs) >= 0;

		// Active file
		if (abs == storage.file) {
			zui_fill(0, 1, ui._w - 1, zui_ELEMENT_H(ui) - 1, ui.t.BUTTON_PRESSED_COL);
		}

		let prefix = "";
		if (!is_file) {
			prefix = is_expanded ? "- " : "+ ";
		}

		if (zui_button(prefix + f, Align.Left)) {
			// Open file
			if (is_file) {
				storage.file = abs;
				let bytes = krom_load_blob(storage.file);
				storage.text = f.endsWith(".arm") ? JSON.stringify(armpack_decode(bytes), null, 4) : sys_buffer_to_string(bytes);
				storage.text = storage.text.replaceAll("\r", "");
				text_handle.text = storage.text;
				editor_handle.redraws = 1;
				krom_set_window_title(abs);
			}
			// Expand folder
			else {
				storage.expanded.indexOf(abs) == -1 ? storage.expanded.push(abs) : array_remove(storage.expanded, abs);
			}
		}

		if (is_expanded) {
			// ui.indent(false);
			list_folder(abs);
			// ui.unindent(false);
		}
	}
}

function render() {
	storage.window_w = sys_width();
	storage.window_h = sys_height();
	storage.window_x = krom_window_x();
	storage.window_y = krom_window_y();
	if (ui.input_dx != 0 || ui.input_dy != 0) {
		krom_set_mouse_cursor(0); // Arrow
	}

	zui_begin(ui);

	if (zui_window(sidebar_handle, 0, 0, storage.sidebar_w, sys_height(), false)) {
		let _BUTTON_TEXT_COL = ui.t.BUTTON_TEXT_COL;
		ui.t.BUTTON_TEXT_COL = ui.t.ACCENT_COL;
		if (storage.project != "") {
			list_folder(storage.project);
		}
		else {
			zui_button("Drop folder here", Align.Left);
		}
		ui.t.BUTTON_TEXT_COL = _BUTTON_TEXT_COL;
	}

	zui_fill(sys_width() - minimap_w, 0, minimap_w, zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui) + 1, ui.t.SEPARATOR_COL);
	zui_fill(storage.sidebar_w, 0, 1, sys_height(), ui.t.SEPARATOR_COL);

	let editor_updated = false;

	if (zui_window(editor_handle, storage.sidebar_w + 1, 0, sys_width() - storage.sidebar_w - minimap_w, sys_height(), false)) {
		editor_updated = true;
		let htab = zui_handle("main_0", { position: 0 });
		let file_name = storage.file.substring(storage.file.lastIndexOf("/") + 1);
		let file_names = [file_name];

		for (let file_name of file_names) {
			if (zui_tab(htab, file_name + (storage.modified ? "*" : ""))) {
				// File modified
				if (ui.is_key_pressed) {
					storage.modified = true;
				}

				// Save
				if (ui.is_ctrl_down && ui.key == KeyCode.S) {
					save_file();
				}

				storage.text = zui_text_area(text_handle);
			}
		}

		window_header_h = 32;//Math.floor(ui.windowHeaderH);
	}

	if (resizing_sidebar) {
		storage.sidebar_w += Math.floor(ui.input_dx);
	}
	if (!ui.input_down) {
		resizing_sidebar = false;
	}

	// Minimap controls
	let minimap_x = sys_width() - minimap_w;
	let minimap_y = window_header_h + 1;
	let redraw = false;
	if (ui.input_started && hit_test(ui.input_x, ui.input_y, minimap_x + 5, minimap_y, minimap_w, minimap_h)) {
		minimap_scrolling = true;
	}
	if (!ui.input_down) {
		minimap_scrolling = false;
	}
	if (minimap_scrolling) {
		// editor_handle.scroll_offset -= ui.input_dy * zui_ELEMENT_H(ui) / 2;
		// // editor_handle.scroll_offset = -((ui.input_y - minimap_y - minimap_box_h / 2) * zui_ELEMENT_H(ui) / 2);
		redraw = true;
	}

	// Build project
	if (ui.is_ctrl_down && ui.key == KeyCode.B) {
		save_file();
		build_project();
	}

	zui_end();

	if (redraw) {
		editor_handle.redraws = 2;
	}

	if (minimap != null) {
		g2_begin();
		g2_draw_image(minimap, minimap_x, minimap_y);
		g2_end();
	}

	if (editor_updated) {
		draw_minimap();
	}
}

function save_file() {
	// Trim
	let lines = storage.text.split("\n");
	for (let i = 0; i < lines.length; ++i) {
		lines[i] = trim_end(lines[i]);
	}
	storage.text = lines.join("\n");
	// Spaces to tabs
	storage.text = storage.text.replaceAll("    ", "\t");
	text_handle.text = storage.text;
	// Write bytes
	let bytes = storage.file.endsWith(".arm") ? armpack_encode(JSON.parse(storage.text)) : sys_string_to_buffer(storage.text);
	krom_file_save_bytes(storage.file, bytes, bytes.byteLength);
	storage.modified = false;
}

function build_file(): string {
	///if krom_windows
	return "\\build.bat";
	///else
	return "/build.sh";
	///end
}

function build_project() {
	krom_sys_command(storage.project + build_file() + " " + storage.project);
}

function draw_minimap() {
	if (minimap_h != sys_height()) {
		minimap_h = sys_height();
		if (minimap != null) {
			image_unload(minimap);
		}
		minimap = image_create_render_target(minimap_w, minimap_h);
	}

	g2_begin(minimap, true, ui.t.SEPARATOR_COL);
	g2_set_color(0xff333333);
	let lines = storage.text.split("\n");
	let minimap_full_h = lines.length * 2;
	let scrollProgress = -editor_handle.scroll_offset / (lines.length * zui_ELEMENT_H(ui));
	let out_of_screen = minimap_full_h - minimap_h;
	if (out_of_screen < 0) {
		out_of_screen = 0;
	}
	let offset = Math.floor((out_of_screen * scrollProgress) / 2);

	for (let i = 0; i < lines.length; ++i) {
		if (i * 2 > minimap_h || i + offset >= lines.length) {
			// Out of screen
			break;
		}
		let words = lines[i + offset].split(" ");
		let x = 0;
		for (let j = 0; j < words.length; ++j) {
			let word = words[j];
			g2_fill_rect(x, i * 2, word.length, 2);
			x += word.length + 1;
		}
	}

	// Current position
	let visible_area = out_of_screen > 0 ? minimap_h : minimap_full_h;
	g2_set_color(0x11ffffff);
	minimap_box_h = Math.floor((sys_height() - window_header_h) / zui_ELEMENT_H(ui) * 2);
	g2_fill_rect(0, scrollProgress * visible_area, minimap_w, minimap_box_h);
	g2_end();
}

function hit_test(mx: f32, my: f32, x: f32, y: f32, w: f32, h: f32): bool {
	return mx > x && mx < x + w && my > y && my < y + h;
}

function on_border_hover(handle: zui_handle_t, side: i32) {
	if (handle != sidebar_handle) {
		return;
	}
	if (side != 1) {
		return; // Right
	}

	krom_set_mouse_cursor(3); // Horizontal

	if (zui_current.input_started) {
		resizing_sidebar = true;
	}
}

function on_text_hover() {
	krom_set_mouse_cursor(2); // I-cursor
}

main();
