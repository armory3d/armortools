
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
					"ammo.wasm.js", "ammo.wasm.wasm", "brdf.k",
					"clouds_base.raw", "clouds_detail.raw", "clouds_map.k",
					"deferred_light.arm", "font_default.ttf", "noise256.k",
					"Scene.arm", "shader_datas.arm", "smaa_area.k", "smaa_search.k",
					"water_base.k", "water_detail.k", "water_foam.k", "water_pass.arm",
					"World_irradiance.arm", "world_pass.arm", "World_radiance.k",
					"World_radiance_0.k", "World_radiance_1.k", "World_radiance_2.k",
					"World_radiance_3.k", "World_radiance_4.k", "World_radiance_5.k",
					"World_radiance_6.k", "World_radiance_7.k"];
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
