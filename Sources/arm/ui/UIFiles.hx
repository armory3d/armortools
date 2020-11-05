package arm.ui;

import haxe.io.Bytes;
import zui.Zui;
import zui.Id;
import iron.system.Input;
import iron.system.Time;
import iron.system.ArmPack;
import iron.system.Lz4;
import arm.sys.Path;
import arm.sys.File;

class UIFiles {

	public static var filename: String;
	public static var path = defaultPath;
	static var lastPath = "";
	static var files: Array<String> = null;
	static var iconMap: Map<String, kha.Image> = null;
	static var selected = -1;
	static var showExtensions = true;

	public static function show(filters: String, isSave: Bool, filesDone: String->Void) {

		#if krom_android
		if (path == null) path = defaultPath;
		showCustom(filters, isSave, filesDone);
		#else

		path = isSave ? Krom.saveDialog(filters, "") : Krom.openDialog(filters, "");
		if (path != null) {
			while (path.indexOf(Path.sep + Path.sep) >= 0) path = path.replace(Path.sep + Path.sep, Path.sep);
			path = path.replace("\r", "");
			filename = path.substr(path.lastIndexOf(Path.sep) + 1);
			if (isSave) path = path.substr(0, path.lastIndexOf(Path.sep));
			filesDone(path);
		}
		releaseKeys();
		#end
	}

	#if krom_android
	@:access(zui.Zui)
	static function showCustom(filters: String, isSave: Bool, filesDone: String->Void) {
		var known = false;
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), tr("File Browser"))) {
				var pathHandle = Id.handle();
				var fileHandle = Id.handle();
				ui.row([6 / 10, 2 / 10, 2 / 10]);
				filename = ui.textInput(fileHandle, tr("File"));
				ui.text("*." + filters, Center);
				if (ui.button(isSave ? tr("Save") : tr("Open")) || known || ui.isReturnDown) {
					UIBox.show = false;
					filesDone((known || isSave) ? path : path + Path.sep + filename);
					if (known) pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(Path.sep));
				}
				known = Path.isTexture(path) || Path.isMesh(path) || Path.isProject(path);
				path = fileBrowser(ui, pathHandle, false);
				if (pathHandle.changed) ui.currentWindow.redraws = 3;
			}
		}, 600, 500);
	}
	#end

	static function releaseKeys() {
		// File dialog may prevent firing key up events
		var kb = kha.input.Keyboard.get();
		@:privateAccess kb.sendUpEvent(kha.input.KeyCode.Shift);
		@:privateAccess kb.sendUpEvent(kha.input.KeyCode.Control);
		#if krom_darwin
		@:privateAccess kb.sendUpEvent(kha.input.KeyCode.Meta);
		#end
	}

	@:access(zui.Zui)
	public static function fileBrowser(ui: Zui, handle: Handle, foldersOnly = false, dragFiles = false): String {

		var icons = Res.get("icons.k");
		var folder = Res.tile50(icons, 2, 1);
		var file = Res.tile50(icons, 3, 1);

		if (handle.text == "") handle.text = defaultPath;
		if (handle.text != lastPath) {
			files = [];

			// Up directory
			var i1 = handle.text.indexOf(Path.sep);
			var nested = i1 > -1 && handle.text.length - 1 > i1;
			if (nested) files.push("..");

			var filesAll = File.readDirectory(handle.text, foldersOnly);
			for (f in filesAll) {
				if (f == "" || f.charAt(0) == ".") continue; // Skip hidden
				if (f.indexOf(".") > 0 && !Path.isKnown(f)) continue; // Skip unknown extensions
				files.push(f);
			}
		}
		lastPath = handle.text;
		handle.changed = false;

		var slotw = Std.int(70 * ui.SCALE());
		var num = Std.int(ui._w / slotw);

		// Directory contents
		for (row in 0...Std.int(Math.ceil(files.length / num))) {

			ui.row([for (i in 0...num * 2) 1 / num]);
			if (row > 0) ui._y += ui.ELEMENT_OFFSET() * 14.0;

			for (j in 0...num) {
				var i = j + row * num;
				if (i >= files.length) {
					@:privateAccess ui.endElement(slotw);
					@:privateAccess ui.endElement(slotw);
					continue;
				}

				var f = files[i];
				var _x = ui._x;

				var rect = f.indexOf(".") > 0 ? file : folder;
				var col = rect == file ? ui.t.LABEL_COL : ui.t.LABEL_COL - 0x00202020;
				if (selected == i) col = ui.t.HIGHLIGHT_COL;

				var off = ui._w / 2 - 25 * ui.SCALE();
				ui._x += off;

				var uix = ui._x;
				var uiy = ui._y;
				var state = Idle;
				var generic = true;

				if (f.endsWith(".arm")) {
					if (iconMap == null) iconMap = [];
					var icon = iconMap.get(handle.text + Path.sep + f);
					if (icon == null) {
						var bytes = Bytes.ofData(Krom.loadBlob(handle.text + Path.sep + f));
						var raw = ArmPack.decode(bytes);
						if (raw.material_icons != null) {
							var bytesIcon = raw.material_icons[0];
							icon = kha.Image.fromBytes(Lz4.decode(bytesIcon, 256 * 256 * 4), 256, 256);
							iconMap.set(handle.text + Path.sep + f, icon);
						}
					}
					if (icon != null) {
						state = ui.image(icon, 0xffffffff, 50 * ui.SCALE());
						generic = false;
					}
				}

				if (generic) {
					state = ui.image(icons, col, 50 * ui.SCALE(), rect.x, rect.y, rect.w, rect.h);
				}

				if (state == Started) {

					if (f != ".." && dragFiles) {
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragFile = handle.text;
						if (App.dragFile.charAt(App.dragFile.length - 1) != Path.sep) {
							App.dragFile += Path.sep;
						}
						App.dragFile += f;
					}

					selected = i;
					if (Time.time() - Context.selectTime < 0.25) {
						App.dragFile = null;
						App.isDragging = false;
						handle.changed = ui.changed = true;
						if (f == "..") { // Up
							handle.text = handle.text.substring(0, handle.text.lastIndexOf(Path.sep));
							// Drive root
							if (handle.text.length == 2 && handle.text.charAt(1) == ":") handle.text += Path.sep;
						}
						else {
							if (handle.text.charAt(handle.text.length - 1) != Path.sep) {
								handle.text += Path.sep;
							}
							handle.text += f;
						}
						selected = -1;
					}
					Context.selectTime = Time.time();
				}

				ui._x = _x;
				ui._y += slotw * 0.75;
				var label = (showExtensions || f.indexOf(".") <= 0) ? f : f.substr(0, f.lastIndexOf("."));
				ui.text(label, Center);
				ui._y -= slotw * 0.75;

				if (handle.changed) break;
			}

			if (handle.changed) break;
		}
		ui._y += slotw * 0.8;

		return handle.text;
	}

	static inline var defaultPath =
		#if krom_windows
		"C:\\Users"
		#elseif krom_android
		"/sdcard"
		#elseif krom_darwin
		"/Users"
		#else
		"/"
		#end
	;
}
