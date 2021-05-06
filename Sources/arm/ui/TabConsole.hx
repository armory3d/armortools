package arm.ui;

import haxe.io.Bytes;
import zui.Zui;
import zui.Id;
import arm.sys.Path;
import arm.io.ImportAsset;
import arm.Enums;

class TabConsole {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;

		var title = Console.messageTimer > 0 ? Console.message + "        " : tr("Console");
		var color = Console.messageTimer > 0 ? Console.messageColor : -1;

		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(UIStatus.inst.statustab, title, false, color) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			ui.row([1 / 20, 1 / 20]);

			if (ui.button(tr("Clear"))) {
				Console.lastTraces = [];
			}
			if (ui.button(tr("Export"))) {
				var str = Console.lastTraces.join("\n");
				UIFiles.show("txt", true, function(path: String) {
					var f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".txt")) path += ".txt";
					Krom.fileSaveBytes(path, Bytes.ofString(str).getData());
				});
			}

			ui.endSticky();

			for (t in Console.lastTraces) {
				ui.text(t);
			}
		}
	}
}
