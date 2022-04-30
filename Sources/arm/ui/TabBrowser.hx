package arm.ui;

import kha.input.KeyCode;
import zui.Zui;
import zui.Id;
import arm.sys.Path;
import arm.io.ImportAsset;
import arm.Enums;

class TabBrowser {

	static var hpath = new Handle();
	static var hsearch = new Handle();
	static var known = false;
	static var lastPath =  "";

	public static function showDirectory(directory: String) {
		hpath.text = directory;
		hsearch.text = "";
		UIStatus.inst.statustab.position = 0;
	}

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(UIStatus.inst.statustab, tr("Browser")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			if (Config.raw.bookmarks == null) {
				Config.raw.bookmarks = [];
			}

			var bookmarksW = Std.int(100 * ui.SCALE());

			if (hpath.text == "" && Config.raw.bookmarks.length > 0) { // Init to first bookmark
				hpath.text = Config.raw.bookmarks[0];
			}

			ui.beginSticky();
			if (hsearch.text != "") {
				ui.row([bookmarksW / ui._w, (1 - bookmarksW / ui._w) * 0.75, (1 - bookmarksW / ui._w) * 0.05, (1 - bookmarksW / ui._w) * 0.17, (1 - bookmarksW / ui._w) * 0.03]);
			}
			else {
				ui.row([bookmarksW / ui._w, (1 - bookmarksW / ui._w) * 0.75, (1 - bookmarksW / ui._w) * 0.05, (1 - bookmarksW / ui._w) * 0.2]);
			}

			if (ui.button("+")) {
				Config.raw.bookmarks.push(hpath.text);
				Config.save();
			}
			hpath.text = ui.textInput(hpath, tr("Path"));
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
			UIFiles.fileBrowser(ui, hpath, false, true, hsearch.text, refresh);

			if (known) {
				var path = hpath.text;
				iron.App.notifyOnInit(function() {
					ImportAsset.run(path);
				});
				hpath.text = hpath.text.substr(0, hpath.text.lastIndexOf(Path.sep));
			}
			known = hpath.text.substr(hpath.text.lastIndexOf(Path.sep)).indexOf(".") > 0;
			#if krom_android
			if (hpath.text.endsWith(".armorpaint")) known = false;
			#end

			var bottomY = ui._y;
			ui._x = 0;
			ui._y = _y;
			ui._w = bookmarksW;

			if (ui.button(tr("Cloud"), Left)) {
				hpath.text = "cloud";
			}

			if (ui.button(tr("Disk"), Left)) {
				hpath.text = UIFiles.defaultPath;
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
