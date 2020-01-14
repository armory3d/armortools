package arm.ui;

import zui.Zui;
import zui.Id;
import arm.sys.Path;
import arm.io.ImportAsset;

class TabBrowser {

	static var hpath = new Handle();
	static var known = false;

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.statustab, "Browser") && UITrait.inst.statush > UITrait.defaultStatusH * ui.SCALE()) {

			if (Config.raw.bookmarks == null) {
				Config.raw.bookmarks = [];
			}

			var bookmarksW = Std.int(100 * ui.SCALE());

			var _y = ui._y;
			ui._x = bookmarksW;
			ui._w -= bookmarksW;
			if (hpath.text == "" && Config.raw.bookmarks.length > 0) { // Init to first bookmark
				hpath.text = Config.raw.bookmarks[0];
			}
			hpath.text = ui.textInput(hpath, "Path");
			UIFiles.fileBrowser(ui, hpath);

			if (known) {
				ImportAsset.run(hpath.text);
				hpath.text = hpath.text.substr(0, hpath.text.lastIndexOf(Path.sep));
			}
			known = hpath.text.indexOf(".") > 0;

			var bottomY = ui._y;
			var _h = ui._h;
			ui._x = 0;
			ui._y = _y;
			ui._w = bookmarksW;

			if (ui.button("+")) {
				Config.raw.bookmarks.push(hpath.text);
				Config.save();
			}

			for (b in Config.raw.bookmarks) {
				var folder = b.substr(b.lastIndexOf(Path.sep) + 1);

				if (ui.button(folder, Left)) {
					hpath.text = b;
				}

				if (ui.isHovered && ui.inputReleasedR) {
					UIMenu.draw(function(ui: Zui) {
						ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 2, ui.t.SEPARATOR_COL);
						ui.text(folder, Right, ui.t.HIGHLIGHT_COL);
						if (ui.button("Delete", Left)) {
							Config.raw.bookmarks.remove(b);
							Config.save();
						}
					});
				}
			}

			ui._y = bottomY;
		}
	}
}
