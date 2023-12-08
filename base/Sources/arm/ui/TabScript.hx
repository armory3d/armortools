package arm.ui;

import haxe.io.Bytes;
import zui.Zui;
import iron.System;
import iron.data.Data;
import arm.sys.Path;

class TabScript {

	public static var hscript = new Handle();
	static var textColoring: TTextColoring = null;

	@:access(zui.Zui)
	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(htab, tr("Script")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			if (Config.raw.touch_ui) {
				ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				ui.row([1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}
			if (ui.button(tr("Run"))) {
				try {
					js.Lib.eval(hscript.text);
				}
				catch(e: Dynamic) {
					Console.log(e);
				}
			}
			if (ui.button(tr("Clear"))) {
				hscript.text = "";
			}
			if (ui.button(tr("Import"))) {
				UIFiles.show("js", false, false, function(path: String) {
					Data.getBlob(path, function(b: js.lib.ArrayBuffer) {
						hscript.text = System.bufferToString(b);
						Data.deleteBlob(path);
					});
				});
			}
			if (ui.button(tr("Export"))) {
				var str = hscript.text;
				UIFiles.show("js", true, false, function(path: String) {
					var f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".js")) path += ".js";
					Krom.fileSaveBytes(path, Bytes.ofString(str).getData());
				});
			}
			ui.endSticky();

			var _font = ui.font;
			var _fontSize = ui.fontSize;
			Data.getFont("font_mono.ttf", function(f: Font) { ui.setFont(f); }); // Sync
			ui.fontSize = Std.int(15 * ui.SCALE());
			Zui.textAreaLineNumbers = true;
			Zui.textAreaScrollPastEnd = true;
			Zui.textAreaColoring = getTextColoring();
			ui.textArea(hscript);
			Zui.textAreaLineNumbers = false;
			Zui.textAreaScrollPastEnd = false;
			Zui.textAreaColoring = null;
			ui.setFont(_font);
			ui.fontSize = _fontSize;
		}
	}

	static function getTextColoring(): TTextColoring {
		if (textColoring == null) {
			Data.getBlob("text_coloring.json", function(blob: js.lib.ArrayBuffer) {
				textColoring = haxe.Json.parse(System.bufferToString(blob));
				textColoring.default_color = Std.int(textColoring.default_color);
				for (coloring in textColoring.colorings) coloring.color = Std.int(coloring.color);
			});
		}
		return textColoring;
	}
}
