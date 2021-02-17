package arm.ui;

import zui.Zui;
import zui.Id;
import arm.sys.Path;
import arm.io.ImportAsset;
import arm.Enums;

class TabBrowser {

	static var hpath = new Handle();
	static var known = false;

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
			ui.row([bookmarksW / ui._w, 1 - bookmarksW / ui._w]);
			if (ui.button("+")) {
				Config.raw.bookmarks.push(hpath.text);
				Config.save();
			}
			hpath.text = ui.textInput(hpath, tr("Path"));
			ui.endSticky();

			var _y = ui._y;
			ui._x = bookmarksW;
			ui._w -= bookmarksW;
			UIFiles.fileBrowser(ui, hpath, false, true);

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
