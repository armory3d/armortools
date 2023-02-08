package arm;

import arm.sys.Path;
import arm.sys.File;
import arm.io.ExportTexture;
import arm.io.ExportMesh;
import arm.io.ExportArm;
import arm.io.ImportAsset;
import arm.io.ImportArm;
import arm.ui.BoxExport;
import arm.ui.UIFiles;
import arm.ui.UISidebar;
import arm.Enums;

class Args {

	static var useArgs = false;
	static var assetPath = "";
	static var reimportMesh = false;
	static var exportTextures = false;
	static var exportTexturesType = "";
	static var exportTexturesPreset = "";
	static var exportTexturesPath = "";
	static var exportMesh = false;
	static var exportMeshPath = "";
	static var exportMaterial = false;
	static var exportMaterialPath = "";
	static var background = false;

	public static function parse() {
		if (Krom.getArgCount() > 1) {
			useArgs = true;

			var i = 0;
			while (i < Krom.getArgCount()) {
				// Process each arg
				var currentArg = Krom.getArg(i);
				if (currentArg == "--reload-mesh") {
					reimportMesh = true;
				}
				else if (currentArg == "--export-textures" && (i + 3) <= Krom.getArgCount()) {
					exportTextures = true;
					++i;
					exportTexturesType = Krom.getArg(i);
					++i;
					exportTexturesPreset = Krom.getArg(i);
					++i;
					exportTexturesPath = Krom.getArg(i);

				}
				else if (currentArg == "--export-mesh" && (i + 1) <= Krom.getArgCount()) {
					exportMesh = true;
					++i;
					exportMeshPath = Krom.getArg(i);
				}
				else if (currentArg == "--export-material" && (i + 1) <= Krom.getArgCount()) {
					exportMaterial = true;
					++i;
					exportMaterialPath = Krom.getArg(i);
				}
				else if (currentArg == "--b" || currentArg == "--background") {
					background = true;
				}
				else if (Path.isProject(currentArg)) {
					Project.filepath = currentArg;
				}
				else if (Path.isMesh(currentArg) || Path.isTexture(currentArg) || (i > 1 && !currentArg.startsWith("-") && Path.isFolder(currentArg))) {
					assetPath = currentArg;
				}
				++i;
			}
		}
	}

	public static function run() {
		if (useArgs) {
			iron.App.notifyOnInit(function() {
				if (Project.filepath != "") {
					ImportArm.runProject(Project.filepath);
				}
				else if (assetPath != "") {
					ImportAsset.run(assetPath, -1, -1, false);
					if (Path.isTexture(assetPath)) UISidebar.inst.show2DView(View2DAsset);
				}
				else if (reimportMesh) {
					Project.reimportMesh();
				}

				if (exportTextures) {
					if (exportTexturesType == "png" ||
						exportTexturesType == "jpg" ||
						exportTexturesType == "exr16" ||
						exportTexturesType == "exr32") {
						if (Path.isFolder(exportTexturesPath)) {
							// Applying the correct format type from args
							if (exportTexturesType == "png") {
								App.bitsHandle.position = Bits8;
								Context.formatType = FormatPng;
							}
							else if (exportTexturesType == "jpg") {
								App.bitsHandle.position = Bits8;
								Context.formatType = FormatJpg;
							}
							else if (exportTexturesType == "exr16") {
								App.bitsHandle.position = Bits16;
							}
							else if (exportTexturesType == "exr32") {
								App.bitsHandle.position = Bits32;
							}

							Context.layersExport = ExportVisible;

							// Get export preset and apply the correct one from args
							BoxExport.files = File.readDirectory(Path.data() + Path.sep + "export_presets");
							for (i in 0...BoxExport.files.length) {
								BoxExport.files[i] = BoxExport.files[i].substr(0, BoxExport.files[i].length - 5); // Strip .json
							}

							var file = "export_presets/" + BoxExport.files[0] + ".json";
							for (f in BoxExport.files) if (f == exportTexturesPreset) {
								file = "export_presets/" + BoxExport.files[BoxExport.files.indexOf(f)] + ".json";
							}

							iron.data.Data.getBlob(file, function(blob: kha.Blob) {
								BoxExport.preset = haxe.Json.parse(blob.toString());
								iron.data.Data.deleteBlob("export_presets/" + file);
							});

							// Export queue
							function _init() {
								ExportTexture.run(exportTexturesPath);
							}
							iron.App.notifyOnInit(_init);
						}
						else {
							trace("Invalid export directory");
						}
					}
					else {
						trace("Invalid texture type");
					}
				}
				else if (exportMesh) {
					if (Path.isFolder(exportMeshPath)) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						ExportMesh.run(exportMeshPath + Path.sep + f, null, false);
					}
					else {
						trace("Invalid export directory");
					}
				}
				else if (exportMaterial) {
					Context.writeIconOnExport = true;
					ExportArm.runMaterial(exportMaterialPath);
				}

				if (background) kha.System.stop();
			});
		}
	}
}
