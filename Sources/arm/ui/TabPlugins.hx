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
					var enabled = Config.raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					ui.check(h, f.split(".")[0]);
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
							ui.fill(0, 0, ui._w / ui.SCALE, ui.t.ELEMENT_H * 4, ui.t.SEPARATOR_COL);
							ui.text(f, Right);
							var path = Krom.getFilesLocation() + sep + dataPath + sep + "plugins" + sep + f;
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
							if (ui.button("Edit", Left)) {
								#if krom_windows
								Krom.sysCommand('"' + path + '"');
								#elseif krom_linux
								Krom.sysCommand('xdg-open "' + path + '"');
								#else
								Krom.sysCommand('open "' + path + '"');
								#end
							}
							if (ui.button("Reload", Left)) {
								if (Config.raw.plugins.indexOf(f) >= 0) {
									Plugin.stop(f);
								}
								iron.data.Data.deleteBlob("plugins/" + f);
								if (Config.raw.plugins.indexOf(f) >= 0) {
									Plugin.start(f);
								}
							}
						});
					}
				}
			}
			ui.separator();

			arm.plugin.Console.render(ui);
			ui.separator();

			#if arm_creator
			if (ui.panel(Id.handle({selected: false}), "Convert", 1)) {
				ui.row([1/2, 1/2]);
				if (ui.button(".arm to .json")) {
					UIFiles.show = true;
					UIFiles.isSave = false;
					UIFiles.filters = "arm";
					UIFiles.filesDone = function(path:String) {
						Data.getBlob(path, function(b:kha.Blob) {
							var parsed = ArmPack.decode(b.bytes);
							var out = Bytes.ofString(Json.stringify(parsed, function(key:Dynamic, value:Dynamic) {
								if (Std.is(value, js.lib.Float32Array)) {
									var ar = untyped Array.from(value);
									ar.unshift(0); // Annotate array type
									return ar;
								}
								else if (Std.is(value, js.lib.Uint32Array)) {
									var ar = untyped Array.from(value);
									ar.unshift(1);
									return ar;
								}
								else if (Std.is(value, js.lib.Int16Array)) {
									var ar = untyped Array.from(value);
									ar.unshift(2);
									return ar;
								}
								return value;
							}, "	")).getData();
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
							function iterate(d:Dynamic) {
								for (n in Reflect.fields(d)) {
									var v = Reflect.field(d, n);
									if (Std.is(v, Array)) {
										if (Std.is(v[0], Int)) {
											var ar:Dynamic = null;
											if (v[0] == 0) ar = new js.lib.Float32Array(v.length - 1);
											else if (v[0] == 1) ar = new js.lib.Uint32Array(v.length - 1);
											else if (v[0] == 2) ar = new js.lib.Int16Array(v.length - 1);
											for (i in 0...v.length - 1) ar[i] = v[i + 1];
											Reflect.setField(d, n, ar);
										}
										else for (e in v) if (Type.typeof(e) == TObject) iterate(e);
									}
									else if (Type.typeof(v) == TObject) iterate(v);
								}
							}
							iterate(parsed);
							var out = ArmPack.encode(parsed).getData();
							Krom.fileSaveBytes(path.substr(0, path.length - 4) + "arm", out);
						});
					}
				}
			}
			ui.separator();

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
