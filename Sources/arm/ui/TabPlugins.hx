package arm.ui;

import haxe.io.Bytes;
import haxe.Json;
import zui.Zui;
import zui.Id;
import iron.data.Data;
import iron.system.ArmPack;
import arm.io.ImportPlugin;
using StringTools;

@:access(zui.Zui)
class TabPlugins{

	public static var files:Array<String> = null;

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Plugins")) {

			if (ui.panel(Id.handle({selected: false}), "Manager", 1)) {

				#if krom_windows
				var sep = "\\";
				var dataPath = Data.dataPath.replace("/", "\\");
				#else
				var sep = "/";
				var dataPath = Data.dataPath;
				#end

				ui.row([1/4, 1/4]);
				if (ui.button("New")) {
					var template =
"let plugin = new arm.Plugin();
let h1 = new zui.Handle();
plugin.drawUI = function(ui) {
	if (ui.panel(h1, 'New Plugin')) {
		if (ui.button('Button')) {
			arm.Log.showError('Hello');
		}
	}
}
";
					UIBox.showCustom(function(ui:Zui) {
						if (ui.tab(Id.handle(), "New Plugin")) {
							ui.row([1/2, 1/2]);
							var pluginName = ui.textInput(Id.handle({text: "new_plugin"}), "Name");
							if (ui.button("OK") || ui.isReturnDown) {
								if (!pluginName.endsWith(".js")) pluginName += ".js";
								var path = Krom.getFilesLocation() + sep + dataPath + sep + "plugins" + sep + pluginName;
								Krom.fileSaveBytes(path, Bytes.ofString(template).getData());
								arm.ui.TabPlugins.files = null; // Refresh file list
								UIBox.show = false;
								App.redrawUI();
							}
						}
					});
				}
				if (ui.button("Install")) {
					UIFiles.show = true;
					UIFiles.isSave = false;
					UIFiles.filters = "js,wasm,zip";
					UIFiles.filesDone = function(path:String) {
						ImportPlugin.run(path);
					}
				}

				if (files == null) {
					#if krom_windows
					var cmd = "dir /b ";
					#else
					var cmd = "ls ";
					#end
					#if krom_linux
					var save = "/tmp";
					#else
					var save = Krom.savePath();
					#end
					save += sep + "dir.txt";
					var path = Krom.getFilesLocation() + sep + dataPath + sep + "plugins";
					Krom.sysCommand(cmd + '"' + path + '"' + ' > ' + '"' + save + '"');
					var str = Bytes.ofData(Krom.loadBlob(save)).toString();
					files = str.split("\n");
					files.pop();
					for (i in 0...files.length) {
						files[i] = files[i].replace("\r", "");
					}
				}

				var h = Id.handle({selected: false});
				for (f in files) {
					var isJs = f.endsWith(".js");
					var isWasm = f.endsWith(".wasm");
					if (!isJs && !isWasm) continue;
					var enabled = Config.raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					var tag = isJs ? f.split(".")[0] : f;
					ui.check(h, tag);
					if (h.changed && h.selected != enabled) {
						if (h.selected) {
							Config.raw.plugins.push(f);
							Plugin.start(f);
						}
						else {
							Config.raw.plugins.remove(f);
							Plugin.stop(f);
						}
						Config.save();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui:Zui) {
							ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 4, ui.t.SEPARATOR_COL);
							ui.text(f, Right);
							var path = Krom.getFilesLocation() + sep + dataPath + sep + "plugins" + sep + f;
							if (ui.button("Edit", Left)) {
								#if krom_windows
								Krom.sysCommand('start "" "' + path + '"');
								#elseif krom_linux
								Krom.sysCommand('xdg-open "' + path + '"');
								#else
								Krom.sysCommand('open "' + path + '"');
								#end
							}
							if (ui.button("Export", Left)) {
								UIFiles.show = true;
								UIFiles.isSave = true;
								UIFiles.filters = "js";
								UIFiles.filesDone = function(dest:String) {
									#if krom_windows
									var copy = "copy";
									#else
									var copy = "cp";
									#end
									if (!UIFiles.filename.endsWith(".js")) UIFiles.filename += ".js";
									Krom.sysCommand(copy + ' ' + path + ' ' + dest + sep + UIFiles.filename);
								}
							}
							if (ui.button("Delete", Left)) {
								if (Config.raw.plugins.indexOf(f) >= 0) {
									Config.raw.plugins.remove(f);
									Plugin.stop(f);
								}
								files.remove(f);
								#if krom_windows
								var cmd = "del /f ";
								#else
								var cmd = "rm ";
								#end
								Krom.sysCommand(cmd + '"' + path + '"');
							}
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
