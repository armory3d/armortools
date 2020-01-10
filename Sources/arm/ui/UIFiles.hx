package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.sys.Path;
import arm.sys.File;
using StringTools;

class UIFiles {

	public static var filename = "untitled";
	public static var path = "/";
	static var lastPath = "";
	static var files: Array<String> = null;

	public static function show(filters: String, isSave: Bool, filesDone: String->Void) {
		if (!UITrait.inst.nativeBrowser) {
			if (path == null) path = "/";
			showCustom(filters, isSave, filesDone);
			return;
		}

		path = isSave ? Krom.saveDialog(filters, "") : Krom.openDialog(filters, "");
		if (path != null) {
			#if krom_windows
			if (!Path.isAscii(path)) path = Path.shortPath(path);
			#end
			path = path.replace("\\\\", "\\");
			path = path.replace("\r", "");
			filename = path.substr(path.lastIndexOf(Path.sep) + 1);
			if (isSave) path = path.substr(0, path.lastIndexOf(Path.sep));
			filesDone(path);
		}
		releaseKeys();
	}

	@:access(zui.Zui) //
	static function showCustom(filters: String, isSave: Bool, filesDone: String->Void) {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), "File Browser")) {
				var pathHandle = Id.handle();
				var fileHandle = Id.handle();
				ui.row([6 / 10, 2 / 10, 2 / 10]);
				filename = ui.textInput(fileHandle, "File");
				ui.text("*." + filters, Center);
				var known = Path.isTexture(path) || Path.isMesh(path) || Path.isProject(path);
				if (ui.button(isSave ? "Save" : "Open") || known || ui.isReturnDown) {
					UIBox.show = false;
					filesDone((known || isSave) ? path : path + Path.sep + filename);
					if (known) pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(Path.sep));
				}
				path = fileBrowser(ui, pathHandle, false);
				if (pathHandle.changed) ui.currentWindow.redraws = 3;
			}
		}, 600, 500);
	}

	static function releaseKeys() {
		// File dialog may prevent firing key up events
		var kb = Input.getKeyboard();
		@:privateAccess kb.upListener(kha.input.KeyCode.Shift);
		@:privateAccess kb.upListener(kha.input.KeyCode.Control);
	}

	@:access(zui.Zui)
	public static function fileBrowser(ui: Zui, handle: Handle, foldersOnly = false): String {

		var icons = Res.get("icons.k");
		var folder = Res.tile50(icons, 2, 1);
		var file = Res.tile50(icons, 3, 1);

		if (handle.text == "") initPath(handle);
		if (handle.text != lastPath) {
			var filesAll = File.readDirectory(handle.text, foldersOnly);
			files = [];
			for (f in filesAll) {
				if (f == "" || f.charAt(0) == ".") continue; // Skip hidden
				files.push(f);
			}
		}
		lastPath = handle.text;

		// Up directory
		var i1 = handle.text.indexOf("/");
		var i2 = handle.text.indexOf("\\");
		var nested =
			(i1 > -1 && handle.text.length - 1 > i1) ||
			(i2 > -1 && handle.text.length - 1 > i2);
		handle.changed = false;
		if (nested && ui.button("..", Align.Left)) {
			handle.changed = ui.changed = true;
			handle.text = handle.text.substring(0, handle.text.lastIndexOf(Path.sep));
			// Drive root
			if (handle.text.length == 2 && handle.text.charAt(1) == ":") handle.text += Path.sep;
		}

		var slotw = Std.int(51 * ui.SCALE());
		var num = Std.int(UITrait.inst.windowW / slotw);

		// Directory contents
		for (row in 0...Std.int(Math.ceil(files.length / num))) {

			ui.row([for (i in 0...num * 2) 1 / num]);
			if (row > 0) ui._y += ui.ELEMENT_OFFSET() * 10.0;

			for (j in 0...num) {
				var i = j + row * num;
				if (i >= files.length) {
					@:privateAccess ui.endElement(slotw);
					@:privateAccess ui.endElement(slotw);
					continue;
				}

				var f = files[i];
				var _x = ui._x;

				if (ui.image(icons, ui.t.LABEL_COL, folder.h, folder.x, folder.y, folder.w, folder.h) == Released) {
					handle.changed = ui.changed = true;
					if (handle.text.charAt(handle.text.length - 1) != Path.sep) {
						handle.text += Path.sep;
					}
					handle.text += f;
				}

				ui._x = _x;
				ui._y += slotw * 0.9;
				ui.text(f, Center);
				ui._y -= slotw * 0.9;
			}
		}

		return handle.text;
	}

	static function initPath(handle: Handle) {
		#if krom_windows
		handle.text = "C:\\Users";
		// %HOMEDRIVE% + %HomePath%
		#else
		handle.text = "/";
		// ~
		#end
	}
}
