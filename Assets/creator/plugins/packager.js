
let plugin = new arm.Plugin();

let h1 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Packager")) {
		if (ui.button("Export")) {
			arm.UIFiles.show("", true, function(path) {
				var sep = arm.Path.sep;

				var sourceData = arm.Path.data();
				var dest = path + sep + arm.UIFiles.filename;
				var destData = dest + sep + "data";
				arm.File.createDirectory(dest);
				arm.File.createDirectory(destData);
				arm.File.copy(sourceData + sep + "player.bin", dest + sep + "krom.bin");

				var fileList = [
					"ammo.wasm.js", "ammo.wasm.wasm", "brdf.png",
					"clouds_base.raw", "clouds_detail.raw", "clouds_map.png",
					"deferred_light.arm", "font_default.ttf", "noise256.png",
					"Scene.arm", "shader_datas.arm", "smaa_area.png", "smaa_search.png",
					"water_base.png", "water_detail.png", "water_foam.png", "water_pass.arm",
					"World_irradiance.arm", "world_pass.arm", "World_radiance.hdr",
					"World_radiance_0.hdr", "World_radiance_1.hdr", "World_radiance_2.hdr",
					"World_radiance_3.hdr", "World_radiance_4.hdr", "World_radiance_5.hdr",
					"World_radiance_6.hdr", "World_radiance_7.hdr"];
				for (const file of fileList) {
					arm.File.copy(sourceData + sep + file, destData + sep + file);
				}

				var destExe = dest + sep + arm.UIFiles.filename;
				if (Krom.systemId() === "Windows") {
					destExe += ".exe";
				}
				var sourceExe = arm.Path.workingDir() + sep + Krom.getArg(0);
				arm.Log.trace(sourceExe);
				arm.Log.trace(destExe);
				arm.File.copy(sourceExe, destExe);
			});
		}
	}
}
