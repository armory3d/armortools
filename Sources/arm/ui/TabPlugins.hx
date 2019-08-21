package arm.ui;

import haxe.io.Bytes;
import haxe.Json;
import zui.Id;
import iron.data.Data;
import iron.system.ArmPack;
using StringTools;

class TabPlugins{

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Plugins")) {
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

						var player = Krom.getFilesLocation() + sep + dataPath + "player.bin";

						var dest = path + sep + UIFiles.filename;
						Krom.sysCommand("mkdir " + dest);
						Krom.sysCommand("mkdir " + dest + sep + "data");
						Krom.sysCommand(copy + ' ' + player + ' ' + dest + sep + "krom.bin");

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
