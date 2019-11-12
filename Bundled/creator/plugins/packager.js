
let plugin = new arm.Plugin();

let h1 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Packager")) {
		if (ui.button("Export")) {
			arm.UIFiles.show("", true, function(path) {
				var sep = "/";
				var cd = "echo $PWD";
				var copy = "cp";
				var dataPath = arm.Data.dataPath;
				if (Krom.systemId() === "Windows") {
					sep = "\\";
					cd = "cd";
					copy = "copy";
					dataPath = arm.Data.dataPath.replace("/", "\\");
				}
				var save = Krom.getFilesLocation() + sep + dataPath + "tmp.txt";
				Krom.sysCommand(cd + ' > "' + save + '"');

				var bytes = arm.Bytes.ofData(Krom.loadBlob(save));
				var exe = bytes.toString();
				exe = exe.substr(0, exe.length - 1);
				exe += '\\' + Krom.getArg(0);

				var sourceData = Krom.getFilesLocation() + sep + dataPath;
				var dest = path + sep + arm.UIFiles.filename;
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
				for (const file of fileList) {
					Krom.sysCommand(copy + ' ' + sourceData + file + ' ' + destData + sep + file);
				}

				dest += sep + arm.UIFiles.filename;
				if (Krom.systemId() === "Windows") {
					dest += ".exe";
				}
				Krom.sysCommand(copy + ' ' + exe + ' ' + dest);
			});
		}
	}
}
