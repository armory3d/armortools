package;

import js.lib.ArrayBuffer;
import haxe.Json;
import zui.Zui;
import iron.Data;
import iron.System;
import iron.ArmPack;
import iron.Input;
using StringTools;

typedef TStorage = {
	var project: String;
	var file: String;
	var text: String;
	var modified: Bool;
	var expanded: Array<String>;
	var window_w: Int;
	var window_h: Int;
	var window_x: Int;
	var window_y: Int;
	var sidebar_w: Int;
};

class Main {

	static var ui: Zui;
	static var text_handle = new Handle();
	static var sidebar_handle = new Handle();
	static var editor_handle = new Handle();
	static var storage: TStorage = null;
	static var resizing_sidebar = false;
	static var minimap_w = 150;
	static var minimap_h = 0;
	static var minimap_box_h = 0;
	static var minimap_scrolling = false;
	static var minimap: Image = null;
	static var window_header_h = 0;

	public static function main() {

		Zui.textAreaLineNumbers = true;
		Zui.textAreaScrollPastEnd = true;

		Krom.setApplicationName("ArmorPad");
		var blob_storage = Krom.loadBlob(Krom.savePath() + "/config.json");
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
		else storage = haxe.Json.parse(haxe.io.Bytes.ofData(blob_storage).toString());

		text_handle.text = storage.text;
		var ops: SystemOptions = {
			title: "ArmorPad",
			x: storage.window_x,
			y: storage.window_y,
			width: storage.window_w,
			height: storage.window_h,
			features: FeatureResizable | FeatureMaximizable | FeatureMinimizable,
			mode: WindowMode.Windowed,
			frequency: 60,
			vsync: true
		};

		System.start(ops, function() {
			Data.getFont("font_mono.ttf", function(font: Font) {
				Data.getBlob("themes/dark.json", function(blob_theme: ArrayBuffer) {
					Data.getBlob("text_coloring.json", function(blob_coloring: ArrayBuffer) {

						var parsed = Json.parse(System.bufferToString(blob_theme));
						var theme = new Theme();
						for (key in Type.getInstanceFields(Theme)) {
							if (key == "theme_") continue;
							if (key.startsWith("set_")) continue;
							if (key.startsWith("get_")) key = key.substr(4);
							Reflect.setProperty(theme, key, Reflect.getProperty(parsed, key));
						}

						font.init();
						ui = new Zui({ theme: theme, font: font, scaleFactor: 1.0, color_wheel: null, black_white_gradient: null });
						Zui.onBorderHover = on_border_hover;
						Zui.onTextHover = on_text_hover;

						var textColoring: TTextColoring = Json.parse(System.bufferToString(blob_coloring));
						textColoring.default_color = Std.int(textColoring.default_color);
						for (coloring in textColoring.colorings) coloring.color = Std.int(coloring.color);
						Zui.textAreaColoring = textColoring;

						System.notifyOnFrames(render);
					});
				});
			});
		});

		Krom.setDropFilesCallback(function(path: String) {
			storage.project = path;
			sidebar_handle.redraws = 1;
		});

		Krom.setApplicationStateCallback(function() {}, function() {}, function() {}, function() {},
			function() { // Shutdown
				Krom.fileSaveBytes(Krom.savePath() + "/config.json", System.stringToBuffer(Json.stringify(storage)));
			}
		);
	}

	static function list_folder(path: String) {
		var files = Krom.readDirectory(path, false).split("\n");
		for (f in files) {
			var abs = path + "/" + f;
			var isFile = f.indexOf(".") >= 0;
			var isExpanded = storage.expanded.indexOf(abs) >= 0;

			// Active file
			if (abs == storage.file) {
				ui.fill(0, 1, ui._w - 1, ui.ELEMENT_H() - 1, ui.t.BUTTON_PRESSED_COL);
			}

			var prefix = "";
			if (!isFile) prefix = isExpanded ? "- " : "+ ";

			if (ui.button(prefix + f, Left)) {
				// Open file
				if (isFile) {
					storage.file = abs;
					var bytes = Krom.loadBlob(storage.file);
					storage.text = f.endsWith(".arm") ? Json.stringify(ArmPack.decode(bytes), "    ") : System.bufferToString(bytes);
					storage.text = StringTools.replace(storage.text, "\r", "");
					text_handle.text = storage.text;
					editor_handle.redraws = 1;
					Krom.setWindowTitle(abs);
				}
				// Expand folder
				else {
					storage.expanded.indexOf(abs) == -1 ? storage.expanded.push(abs) : storage.expanded.remove(abs);
				}
			}

			if (isExpanded) {
				// ui.indent(false);
				list_folder(abs);
				// ui.unindent(false);
			}
		}
	}

	static function render(g2: Graphics2, g4: Graphics4): Void {
		storage.window_w = System.width;
		storage.window_h = System.height;
		storage.window_x = Krom.windowX();
		storage.window_y = Krom.windowY();
		if (ui.inputDX != 0 || ui.inputDY != 0) Krom.setMouseCursor(0); // Arrow

		ui.begin(g2);

		if (ui.window(sidebar_handle, 0, 0, storage.sidebar_w, System.height, false)) {
			var _BUTTON_TEXT_COL = ui.t.BUTTON_TEXT_COL;
			ui.t.BUTTON_TEXT_COL = ui.t.ACCENT_COL;
			if (storage.project != "") {
				list_folder(storage.project);
			}
			else {
				ui.button("Drop folder here", Left);
			}
			ui.t.BUTTON_TEXT_COL = _BUTTON_TEXT_COL;
		}

		ui.fill(System.width - minimap_w, 0, minimap_w, ui.ELEMENT_H() + ui.ELEMENT_OFFSET() + 1, ui.t.SEPARATOR_COL);
		ui.fill(storage.sidebar_w, 0, 1, System.height, ui.t.SEPARATOR_COL);

		var editor_updated = false;

		if (ui.window(editor_handle, storage.sidebar_w + 1, 0, System.width - storage.sidebar_w - minimap_w, System.height, false)) {
			editor_updated = true;
			var htab = Zui.handle("main_0", { position: 0 });
			var file_name = storage.file.substring(storage.file.lastIndexOf("/") + 1);
			var file_names = [file_name];

			for (file_name in file_names) {
				if (ui.tab(htab, file_name + (storage.modified ? "*" : ""))) {
					// File modified
					if (ui.isKeyPressed) {
						storage.modified = true;
					}

					// Save
					if (ui.isCtrlDown && ui.key == KeyCode.S) {
						save_file();
					}

					storage.text = ui.textArea(text_handle);
				}
			}

			window_header_h = 32;//Std.int(ui.windowHeaderH);
		}

		if (resizing_sidebar) {
			storage.sidebar_w += Std.int(ui.inputDX);
		}
		if (!ui.inputDown) {
			resizing_sidebar = false;
		}

		// Minimap controls
		var minimap_x = System.width - minimap_w;
		var minimap_y = window_header_h + 1;
		var redraw = false;
		if (ui.inputStarted && hit_test(ui.inputX, ui.inputY, minimap_x + 5, minimap_y, minimap_w, minimap_h)) {
			minimap_scrolling = true;
		}
		if (!ui.inputDown) {
			minimap_scrolling = false;
		}
		if (minimap_scrolling) {
			// editor_handle.scrollOffset -= ui.inputDY * ui.ELEMENT_H() / 2;
			// // editor_handle.scrollOffset = -((ui.inputY - minimap_y - minimap_box_h / 2) * ui.ELEMENT_H() / 2);
			redraw = true;
		}

		// Build project
		if (ui.isCtrlDown && ui.key == KeyCode.B) {
			save_file();
			build_project();
		}

		ui.end();

		if (redraw) {
			editor_handle.redraws = 2;
		}

		if (minimap != null) {
			g2.begin(false);
			g2.drawImage(minimap, minimap_x, minimap_y);
			g2.end();
		}

		if (editor_updated) {
			draw_minimap();
		}
	}

	static function save_file() {
		// Trim
		var lines = storage.text.split("\n");
		for (i in 0...lines.length) lines[i] = StringTools.rtrim(lines[i]);
		storage.text = lines.join("\n");
		// Spaces to tabs
		storage.text = StringTools.replace(storage.text, "    ", "\t");
		text_handle.text = storage.text;
		// Write bytes
		var bytes = storage.file.endsWith(".arm") ? ArmPack.encode(Json.parse(storage.text)) : System.stringToBuffer(storage.text);
		Krom.fileSaveBytes(storage.file, bytes, bytes.byteLength);
		storage.modified = false;
	}

	static function build_project() {
		#if krom_windows
		var build_file = "\\build.bat";
		#else
		var build_file = "/build.sh";
		#end
		Krom.sysCommand(storage.project + build_file + " " + storage.project);
	}

	static function draw_minimap() {
		if (minimap_h != System.height) {
			minimap_h = System.height;
			if (minimap != null) minimap.unload();
			minimap = Image.createRenderTarget(minimap_w, minimap_h);
		}

		minimap.g2.begin(true, ui.t.SEPARATOR_COL);
		minimap.g2.color = 0xff333333;
		var lines = storage.text.split("\n");
		var minimap_full_h = lines.length * 2;
		var scrollProgress = -editor_handle.scrollOffset / (lines.length * ui.ELEMENT_H());
		var outOfScreen = minimap_full_h - minimap_h;
		if (outOfScreen < 0) outOfScreen = 0;
		var offset = Std.int((outOfScreen * scrollProgress) / 2);

		for (i in 0...lines.length) {
			if (i * 2 > minimap_h || i + offset >= lines.length) {
				// Out of screen
				break;
			}
			var words = lines[i + offset].split(" ");
			var x = 0;
			for (j in 0...words.length) {
				var word = words[j];
				minimap.g2.fillRect(x, i * 2, word.length, 2);
				x += word.length + 1;
			}
		}

		// Current position
		var visibleArea = outOfScreen > 0 ? minimap_h : minimap_full_h;
		minimap.g2.color = 0x11ffffff;
		minimap_box_h = Std.int((System.height - window_header_h) / ui.ELEMENT_H() * 2);
		minimap.g2.fillRect(0, scrollProgress * visibleArea, minimap_w, minimap_box_h);
		minimap.g2.end();
	}

	static function hit_test(mx: Float, my: Float, x: Float, y: Float, w: Float, h: Float): Bool {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	static function on_border_hover(handle: Handle, side: Int) {
		if (handle != sidebar_handle) return;
		if (side != 1) return; // Right

		Krom.setMouseCursor(3); // Horizontal

		if (Zui.current.inputStarted) {
			resizing_sidebar = true;
		}
	}

	static function on_text_hover() {
		Krom.setMouseCursor(2); // I-cursor
	}
}
