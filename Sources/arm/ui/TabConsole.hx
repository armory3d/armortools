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
			#if (krom_windows || krom_linux || krom_darwin)
			ui.row([1 / 14, 1 / 14, 1 / 14]); // Copy
			#elseif arm_touchui
			ui.row([1 / 4, 1 / 4]);
			#else
			ui.row([1 / 14, 1 / 14]);
			#end

			if (ui.button(tr("Clear"))) {
				Console.lastTraces = [];
			}
			if (ui.button(tr("Export"))) {
				var str = Console.lastTraces.join("\n");
				UIFiles.show("txt", true, false, function(path: String) {
					var f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".txt")) path += ".txt";
					Krom.fileSaveBytes(path, Bytes.ofString(str).getData());
				});
			}
			#if (krom_windows || krom_linux || krom_darwin)
			if (ui.button(tr("Copy"))) {
				var str = Console.lastTraces.join("\n");
				Krom.copyToClipboard(str);
			}
			#end

			ui.endSticky();

			for (t in Console.lastTraces) {
				ui.text(t);
			}
		}
	}
}
