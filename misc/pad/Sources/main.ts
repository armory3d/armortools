
type TStorage = {
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

class Main {

	static ui: ZuiRaw;
	static text_handle = zui_handle_create();
	static sidebar_handle = zui_handle_create();
	static editor_handle = zui_handle_create();
	static storage: TStorage = null;
	static resizing_sidebar = false;
	static minimap_w = 150;
	static minimap_h = 0;
	static minimap_box_h = 0;
	static minimap_scrolling = false;
	static minimap: ImageRaw = null;
	static window_header_h = 0;

	static main = () => {
		Zui.textAreaLineNumbers = true;
		Zui.textAreaScrollPastEnd = true;

		Krom.setApplicationName("ArmorPad");
		let blob_storage = Krom.loadBlob(Krom.savePath() + "/config.json");
		if (blob_storage == null) {
			Main.storage = {
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
		else Main.storage = JSON.parse(sys_buffer_to_string(blob_storage));

		Main.text_handle.text = Main.storage.text;
		let ops: kinc_sys_ops_t = {
			title: "ArmorPad",
			x: Main.storage.window_x,
			y: Main.storage.window_y,
			width: Main.storage.window_w,
			height: Main.storage.window_h,
			features: WindowFeatures.FeatureResizable | WindowFeatures.FeatureMaximizable | WindowFeatures.FeatureMinimizable,
			mode: WindowMode.Windowed,
			frequency: 60,
			vsync: true
		};

		sys_start(ops, () => {
			data_get_font("font_mono.ttf", (font: FontRaw) => {
				data_get_blob("themes/dark.json", (blob_theme: ArrayBuffer) => {
					data_get_blob("text_coloring.json", (blob_coloring: ArrayBuffer) => {

						let parsed = JSON.parse(sys_buffer_to_string(blob_theme));
						let theme: any = Theme.create();
						for (let key of Object.getOwnPropertyNames(Theme.prototype)) {
							if (key == "constructor") continue;
							theme[key] = parsed[key];
						}

						font_init(font);
						Main.ui = Zui.create({ theme: theme, font: font, scaleFactor: 1.0, color_wheel: null, black_white_gradient: null });
						Zui.onBorderHover = Main.on_border_hover;
						Zui.onTextHover = Main.on_text_hover;

						let textColoring: TTextColoring = JSON.parse(sys_buffer_to_string(blob_coloring));
						textColoring.default_color = Math.floor(textColoring.default_color);
						for (let coloring of textColoring.colorings) coloring.color = Math.floor(coloring.color);
						Zui.textAreaColoring = textColoring;

						sys_notify_on_frames(Main.render);
					});
				});
			});
		});

		Krom.setDropFilesCallback((path: string) => {
			Main.storage.project = path;
			Main.sidebar_handle.redraws = 1;
		});

		Krom.setApplicationStateCallback(() => {}, () => {}, () => {}, () => {},
			() => { // Shutdown
				Krom.fileSaveBytes(Krom.savePath() + "/config.json", sys_string_to_buffer(JSON.stringify(Main.storage)));
			}
		);
	}

	static list_folder = (path: string) => {
		let ui = Main.ui;
		let files = Krom.readDirectory(path, false).split("\n");
		for (let f of files) {
			let abs = path + "/" + f;
			let isFile = f.indexOf(".") >= 0;
			let isExpanded = Main.storage.expanded.indexOf(abs) >= 0;

			// Active file
			if (abs == Main.storage.file) {
				Zui.fill(0, 1, ui._w - 1, Zui.ELEMENT_H(ui) - 1, ui.t.BUTTON_PRESSED_COL);
			}

			let prefix = "";
			if (!isFile) prefix = isExpanded ? "- " : "+ ";

			if (Zui.button(prefix + f, Align.Left)) {
				// Open file
				if (isFile) {
					Main.storage.file = abs;
					let bytes = Krom.loadBlob(Main.storage.file);
					Main.storage.text = f.endsWith(".arm") ? JSON.stringify(decode(bytes), null, 4) : sys_buffer_to_string(bytes);
					Main.storage.text = Main.storage.text.replaceAll("\r", "");
					Main.text_handle.text = Main.storage.text;
					Main.editor_handle.redraws = 1;
					Krom.setWindowTitle(abs);
				}
				// Expand folder
				else {
					Main.storage.expanded.indexOf(abs) == -1 ? Main.storage.expanded.push(abs) : array_remove(Main.storage.expanded, abs);
				}
			}

			if (isExpanded) {
				// ui.indent(false);
				Main.list_folder(abs);
				// ui.unindent(false);
			}
		}
	}

	static render = (g2: Graphics2Raw, g4: Graphics4Raw): void => {
		let ui = Main.ui;
		Main.storage.window_w = sys_width();
		Main.storage.window_h = sys_height();
		Main.storage.window_x = Krom.windowX();
		Main.storage.window_y = Krom.windowY();
		if (ui.inputDX != 0 || ui.inputDY != 0) Krom.setMouseCursor(0); // Arrow

		Zui.begin(ui, g2);

		if (Zui.window(ui, Main.sidebar_handle, 0, 0, Main.storage.sidebar_w, sys_height(), false)) {
			let _BUTTON_TEXT_COL = ui.t.BUTTON_TEXT_COL;
			ui.t.BUTTON_TEXT_COL = ui.t.ACCENT_COL;
			if (Main.storage.project != "") {
				Main.list_folder(Main.storage.project);
			}
			else {
				Zui.button("Drop folder here", Align.Left);
			}
			ui.t.BUTTON_TEXT_COL = _BUTTON_TEXT_COL;
		}

		Zui.fill(sys_width() - Main.minimap_w, 0, Main.minimap_w, Zui.ELEMENT_H(ui) + Zui.ELEMENT_OFFSET(ui) + 1, ui.t.SEPARATOR_COL);
		Zui.fill(Main.storage.sidebar_w, 0, 1, sys_height(), ui.t.SEPARATOR_COL);

		let editor_updated = false;

		if (Zui.window(ui, Main.editor_handle, Main.storage.sidebar_w + 1, 0, sys_width() - Main.storage.sidebar_w - Main.minimap_w, sys_height(), false)) {
			editor_updated = true;
			let htab = Zui.handle("main_0", { position: 0 });
			let file_name = Main.storage.file.substring(Main.storage.file.lastIndexOf("/") + 1);
			let file_names = [file_name];

			for (let file_name of file_names) {
				if (Zui.tab(htab, file_name + (Main.storage.modified ? "*" : ""))) {
					// File modified
					if (ui.isKeyPressed) {
						Main.storage.modified = true;
					}

					// Save
					if (ui.isCtrlDown && ui.key == KeyCode.S) {
						Main.save_file();
					}

					Main.storage.text = Zui.textArea(Main.text_handle);
				}
			}

			Main.window_header_h = 32;//Math.floor(ui.windowHeaderH);
		}

		if (Main.resizing_sidebar) {
			Main.storage.sidebar_w += Math.floor(ui.inputDX);
		}
		if (!ui.inputDown) {
			Main.resizing_sidebar = false;
		}

		// Minimap controls
		let minimap_x = sys_width() - Main.minimap_w;
		let minimap_y = Main.window_header_h + 1;
		let redraw = false;
		if (ui.inputStarted && Main.hit_test(ui.inputX, ui.inputY, minimap_x + 5, minimap_y, Main.minimap_w, Main.minimap_h)) {
			Main.minimap_scrolling = true;
		}
		if (!ui.inputDown) {
			Main.minimap_scrolling = false;
		}
		if (Main.minimap_scrolling) {
			// Main.editor_handle.scrollOffset -= ui.inputDY * Zui.ELEMENT_H(ui) / 2;
			// // Main.editor_handle.scrollOffset = -((ui.inputY - minimap_y - Main.minimap_box_h / 2) * Zui.ELEMENT_H(ui) / 2);
			redraw = true;
		}

		// Build project
		if (ui.isCtrlDown && ui.key == KeyCode.B) {
			Main.save_file();
			Main.build_project();
		}

		Zui.end();

		if (redraw) {
			Main.editor_handle.redraws = 2;
		}

		if (Main.minimap != null) {
			Graphics2.begin(g2, false);
			Graphics2.drawImage(Main.minimap, minimap_x, minimap_y);
			Graphics2.end(g2);
		}

		if (editor_updated) {
			Main.draw_minimap();
		}
	}

	static save_file = () => {
		// Trim
		let lines = Main.storage.text.split("\n");
		for (let i = 0; i < lines.length; ++i) lines[i] = trim_end(lines[i]);
		Main.storage.text = lines.join("\n");
		// Spaces to tabs
		Main.storage.text = Main.storage.text.replaceAll("    ", "\t");
		Main.text_handle.text = Main.storage.text;
		// Write bytes
		let bytes = Main.storage.file.endsWith(".arm") ? encode(JSON.parse(Main.storage.text)) : sys_string_to_buffer(Main.storage.text);
		Krom.fileSaveBytes(Main.storage.file, bytes, bytes.byteLength);
		Main.storage.modified = false;
	}

	static get build_file(): string {
		///if krom_windows
		return "\\build.bat";
		///else
		return "/build.sh";
		///end
	}

	static build_project = () => {
		Krom.sysCommand(Main.storage.project + Main.build_file + " " + Main.storage.project);
	}

	static draw_minimap = () => {
		let ui = Main.ui;
		if (Main.minimap_h != sys_height()) {
			Main.minimap_h = sys_height();
			if (Main.minimap != null) image_unload(Main.minimap);
			Main.minimap = image_create_render_target(Main.minimap_w, Main.minimap_h);
		}

		Graphics2.begin(Main.minimap.g2, true, ui.t.SEPARATOR_COL);
		Main.minimap.g2.color = 0xff333333;
		let lines = Main.storage.text.split("\n");
		let minimap_full_h = lines.length * 2;
		let scrollProgress = -Main.editor_handle.scrollOffset / (lines.length * Zui.ELEMENT_H(ui));
		let outOfScreen = minimap_full_h - Main.minimap_h;
		if (outOfScreen < 0) outOfScreen = 0;
		let offset = Math.floor((outOfScreen * scrollProgress) / 2);

		for (let i = 0; i < lines.length; ++i) {
			if (i * 2 > Main.minimap_h || i + offset >= lines.length) {
				// Out of screen
				break;
			}
			let words = lines[i + offset].split(" ");
			let x = 0;
			for (let j = 0; j < words.length; ++j) {
				let word = words[j];
				Graphics2.fillRect(x, i * 2, word.length, 2);
				x += word.length + 1;
			}
		}

		// Current position
		let visibleArea = outOfScreen > 0 ? Main.minimap_h : minimap_full_h;
		Main.minimap.g2.color = 0x11ffffff;
		Main.minimap_box_h = Math.floor((sys_height() - Main.window_header_h) / Zui.ELEMENT_H(ui) * 2);
		Graphics2.fillRect(0, scrollProgress * visibleArea, Main.minimap_w, Main.minimap_box_h);
		Graphics2.end(Main.minimap.g2);
	}

	static hit_test = (mx: f32, my: f32, x: f32, y: f32, w: f32, h: f32): bool => {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	static on_border_hover = (handle: Handle, side: i32) => {
		if (handle != Main.sidebar_handle) return;
		if (side != 1) return; // Right

		Krom.setMouseCursor(3); // Horizontal

		if (Zui.current.inputStarted) {
			Main.resizing_sidebar = true;
		}
	}

	static on_text_hover = () => {
		Krom.setMouseCursor(2); // I-cursor
	}
}

Main.main();
