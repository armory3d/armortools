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

				ui.row([1/4]);
				if (ui.button("Install")) {
					UIFiles.show = true;
					UIFiles.isSave = false;
					UIFiles.filters = "js,wasm,zip";
					UIFiles.filesDone = function(path:String) {
						ImportPlugin.run(path);
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
							ui.fill(0, 0, ui._w / ui.SCALE, ui.t.ELEMENT_H * 3, ui.t.SEPARATOR_COL);
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

			#if arm_creator
			if (ui.panel(Id.handle({selected: false}), "Package", 1)) {
				if (ui.button("Make Package")) {
					UIFiles.show = true;
					UIFiles.isSave = true;
					UIFiles.filters = "";
					UIFiles.filesDone = function(path:String) {
						#if krom_windows
						var sep = "\\";
						var cd = "cd";
						var copy = "copy";
						var dataPath = Data.dataPath.replace("/", "\\");
						#else
						var sep = "/";
						var cd = "echo $PWD";
						var copy = "cp";
						var dataPath = Data.dataPath;
						#end
						var save = Krom.getFilesLocation() + sep + dataPath + "tmp.txt";
						Krom.sysCommand(cd + ' > "' + save + '"');

						var bytes = haxe.io.Bytes.ofData(Krom.loadBlob(save));
						var exe = bytes.toString();
						exe = exe.substr(0, exe.length - 1);
						exe += '\\' + Krom.getArg(0);

						var sourceData = Krom.getFilesLocation() + sep + dataPath;
						var dest = path + sep + UIFiles.filename;
						var destData = dest + sep + "data";
						Krom.sysCommand("mkdir " + dest);
						Krom.sysCommand("mkdir " + destData);
						Krom.sysCommand(copy + ' ' + sourceData + "player.bin" + ' ' + dest + sep + "krom.bin");

						var fileList = [
							"ammo.wasm.js", "ammo.wasm.wasm", "brdf.png",
							"clouds_base.raw", "clouds_detail.raw", "clouds_map.png",
							"config.arm", "deferred_light.arm", "font_default.ttf", "noise256.png",
							"Scene.arm", "shader_datas.arm", "smaa_area.png", "smaa_search.png",
							"water_base.png", "water_detail.png", "water_foam.png", "water_pass.arm",
							"World_irradiance.arm", "world_pass.arm", "World_radiance.hdr",
							"World_radiance_0.hdr", "World_radiance_1.hdr", "World_radiance_2.hdr",
							"World_radiance_3.hdr", "World_radiance_4.hdr", "World_radiance_5.hdr",
							"World_radiance_6.hdr", "World_radiance_7.hdr"];
						for (file in fileList) {
							Krom.sysCommand(copy + ' ' + sourceData + file + ' ' + destData + sep + file);
						}

						dest += sep + UIFiles.filename;
						#if krom_windows
						dest += ".exe";
						#end
						Krom.sysCommand(copy + ' ' + exe + ' ' + dest);
					}
				}
			}
			ui.separator();
			#end

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
