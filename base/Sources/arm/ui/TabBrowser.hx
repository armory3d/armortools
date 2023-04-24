package arm.ui;

import kha.input.KeyCode;
import zui.Zui;
import arm.sys.File;
import arm.sys.Path;
import arm.io.ImportAsset;

class TabBrowser {

	static var hpath = new Handle();
	static var hsearch = new Handle();
	static var known = false;
	static var lastPath =  "";

	public static function showDirectory(directory: String) {
		hpath.text = directory;
		hsearch.text = "";
		UIBase.inst.htabs[TabStatus].position = 0;
	}

	@:access(zui.Zui)
	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(htab, tr("Browser")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			if (Config.raw.bookmarks == null) {
				Config.raw.bookmarks = [];
			}

			var bookmarksW = Std.int(100 * ui.SCALE());

			if (hpath.text == "" && Config.raw.bookmarks.length > 0) { // Init to first bookmark
				hpath.text = Config.raw.bookmarks[0];
			}

			ui.beginSticky();
			var step = (1 - bookmarksW / ui._w);
			if (hsearch.text != "") {
				ui.row([bookmarksW / ui._w, step * 0.73, step * 0.07, step * 0.17, step * 0.03]);
			}
			else {
				ui.row([bookmarksW / ui._w, step * 0.73, step * 0.07, step * 0.2]);
			}

			if (ui.button("+")) {
				Config.raw.bookmarks.push(hpath.text);
				Config.save();
			}
			if (ui.isHovered) ui.tooltip(tr("Add bookmark"));

			#if krom_android
			var stripped = false;
			var strip = "/storage/emulated/0/";
			if (hpath.text.startsWith(strip)) {
				hpath.text = hpath.text.substr(strip.length - 1);
				stripped = true;
			}
			#end

			hpath.text = ui.textInput(hpath, tr("Path"));

			#if krom_android
			if (stripped) {
				hpath.text = "/storage/emulated/0" + hpath.text;
			}
			#end

			var refresh = false;
			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (ui.button(tr("Refresh")) || (inFocus && ui.isKeyPressed && ui.key == kha.input.KeyCode.F5)) {
				refresh = true;
			}
			hsearch.text = ui.textInput(hsearch, tr("Search"), Align.Left, true, true);
			if (ui.isHovered) ui.tooltip(tr("ctrl+f to search") + "\n" + tr("esc to cancel"));
			if (ui.isCtrlDown && ui.isKeyPressed && ui.key == KeyCode.F) { // Start searching via ctrl+f
				ui.startTextEdit(hsearch);
			}
			if (hsearch.text != "" && (ui.button(tr("X")) || ui.isEscapeDown)) {
				hsearch.text = "";
			}
			ui.endSticky();

			if (lastPath != hpath.text) {
				hsearch.text = "";
			}
			lastPath = hpath.text;

			var _y = ui._y;
			ui._x = bookmarksW;
			ui._w -= bookmarksW;
			UIFiles.fileBrowser(ui, hpath, false, true, hsearch.text, refresh, function(file: String) {
				var fileName = file.substr(file.lastIndexOf(Path.sep) + 1);
				if (fileName != "..") {
					UIMenu.draw(function(ui: Zui) {
						ui.text(fileName, Right, ui.t.HIGHLIGHT_COL);
						if (ui.button(tr("Import"), Left)) {
							ImportAsset.run(file);
						}
						if (Path.isTexture(file)) {
							if (ui.button(tr("Set as Envmap"), Left)) {
								ImportAsset.run(file, -1.0, -1.0, true, true, function() {
									App.notifyOnNextFrame(function() {
										var assetIndex = -1;
										for (i in 0...Project.assets.length) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											arm.io.ImportEnvmap.run(file, Project.getImage(Project.assets[assetIndex]));
										}
									});
								});
							}

							#if (is_paint || is_sculpt)
							if (ui.button(tr("Set as Mask"), Left)) {
								ImportAsset.run(file, -1.0, -1.0, true, true, function() {
									App.notifyOnNextFrame(function() {
										var assetIndex = -1;
										for (i in 0...Project.assets.length) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											App.createImageMask(Project.assets[assetIndex]);
										}
									});
								});
							}
							#end

							#if is_paint
							if (ui.button(tr("Set as Color ID Map"), Left)) {
								ImportAsset.run(file, -1.0, -1.0, true, true, function() {
									App.notifyOnNextFrame(function() {
										var assetIndex = -1;
										for (i in 0...Project.assets.length) {
											if (Project.assets[i].file == file) {
												assetIndex = i;
												break;
											}
										}
										if (assetIndex != -1) {
											Context.raw.colorIdHandle.position = assetIndex;
											Context.raw.colorIdPicked = false;
											UIToolbar.inst.toolbarHandle.redraws = 1;
											if (Context.raw.tool == ToolColorId) {
												UIHeader.inst.headerHandle.redraws = 2;
												Context.raw.ddirty = 2;
											}
										}
									});
								});
							}
							#end
						}
						if (ui.button(tr("Open Externally"), Left)) {
							File.start(file);
						}
					}, Path.isTexture(file) ? 6 : 3);
				}
			});

			if (known) {
				var path = hpath.text;
				iron.App.notifyOnInit(function() {
					ImportAsset.run(path);
				});
				hpath.text = hpath.text.substr(0, hpath.text.lastIndexOf(Path.sep));
			}
			known = hpath.text.substr(hpath.text.lastIndexOf(Path.sep)).indexOf(".") > 0;
			#if krom_android
			#if is_paint
			if (hpath.text.endsWith(".armorpaint")) known = false;
			#end
			#if is_sculpt
			if (hpath.text.endsWith(".armorsculpt")) known = false;
			#end
			#if is_lab
			if (hpath.text.endsWith(".armorlab")) known = false;
			#end
			#end

			var bottomY = ui._y;
			ui._x = 0;
			ui._y = _y;
			ui._w = bookmarksW;

			if (ui.button(tr("Cloud"), Left)) {
				hpath.text = "cloud";
			}

			if (ui.button(tr("Disk"), Left)) {
				#if krom_android
				UIMenu.draw(function(ui: Zui) {
					ui.text(tr("Disk"), Right, ui.t.HIGHLIGHT_COL);
					if (ui.button(tr("Download"), Left)) {
						hpath.text = UIFiles.defaultPath;
					}
					if (ui.button(tr("Pictures"), Left)) {
						hpath.text = "/storage/emulated/0/Pictures";
					}
					if (ui.button(tr("Camera"), Left)) {
						hpath.text = "/storage/emulated/0/DCIM/Camera";
					}
					if (ui.button(tr("Projects"), Left)) {
						hpath.text = Krom.savePath();
					}
				}, 5);
				#else
				hpath.text = UIFiles.defaultPath;
				#end
			}

			for (b in Config.raw.bookmarks) {
				var folder = b.substr(b.lastIndexOf(Path.sep) + 1);

				if (ui.button(folder, Left)) {
					hpath.text = b;
				}

				if (ui.isHovered && ui.inputReleasedR) {
					UIMenu.draw(function(ui: Zui) {
						ui.text(folder, Right, ui.t.HIGHLIGHT_COL);
						if (ui.button(tr("Delete"), Left)) {
							Config.raw.bookmarks.remove(b);
							Config.save();
						}
					}, 2);
				}
			}

			if (ui._y < bottomY) ui._y = bottomY;
		}
	}
}
