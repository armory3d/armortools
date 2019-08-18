package arm.ui;

import haxe.io.Bytes;
import haxe.Json;
import zui.Id;
import iron.data.Data;
import iron.system.ArmPack;

class TabPlugins{

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Plugins")) {
			if (ui.panel(Id.handle({selected: false}), "Console", 1)) {
				arm.plugin.Console.render(ui);
			}
			ui.separator();

			if (ui.panel(Id.handle({selected: false}), "Convert", 1)) {
				ui.row([1/2, 1/2]);
				if (ui.button(".arm to .json")) {
					UIFiles.show = true;
					UIFiles.isSave = false;
					UIFiles.filters = "arm";
					UIFiles.filesDone = function(path:String) {
						Data.getBlob(path, function(b:kha.Blob) {
							var parsed = ArmPack.decode(b.bytes);
							var out = Bytes.ofString(Json.stringify(parsed, null, "	")).getData();
							Krom.fileSaveBytes(path.substr(0, path.length - 3) + "json", out);
						});
					}
				}
				if (ui.button(".json to .arm")) {
					UIFiles.show = true;
					UIFiles.isSave = false;
					UIFiles.filters = "json";
					UIFiles.filesDone = function(path:String) {
						Data.getBlob(path, function(b:kha.Blob) {
							var parsed = Json.parse(b.toString());
							var out = ArmPack.encode(parsed).getData();
							Krom.fileSaveBytes(path.substr(0, path.length - 4) + "arm", out);
						});
					}
				}
			}
			ui.separator();

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
