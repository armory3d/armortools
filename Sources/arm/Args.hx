package arm;

import arm.sys.Path;
import arm.sys.File;
import arm.io.ExportTexture;
import arm.io.ExportMesh;
import arm.io.ImportAsset;
import arm.ui.BoxExport;
import arm.ui.UIFiles;
import arm.Enums;

class Args {

	static var useArgs = false;
	static var projectPath = "";
	static var meshPath = "";
	static var reloadMesh = false;
	static var exportTextures = false;
	static var exportTexturesType = "";
	static var exportTexturesPreset = "";
	static var exportTexturesPath = "";
	static var exportMesh = false;
	static var exportMeshPath = "";
	static var backgroundProcessing = false;

	public static function parse() {
		if (Krom.getArgCount() > 1) {
			useArgs = true;

			for (v in 0...Krom.getArgCount()) {
				// Process each arg
				var currentArg = Krom.getArg(v);
				if (Path.isProject(currentArg)) {
					projectPath = currentArg;
					Project.filepath = projectPath;
				}
				if (Path.isMesh(currentArg)) meshPath = currentArg;
				if (currentArg == "--reload-mesh") reloadMesh = true;
				if (currentArg == "--export-textures" && (v + 3) <= Krom.getArgCount()) {
					exportTextures = true;
					exportTexturesType = Krom.getArg(v + 1);
					exportTexturesPreset = Krom.getArg(v + 2);
					exportTexturesPath = Krom.getArg(v + 3);
				}
				if (currentArg == "--export-mesh" && (v + 1) <= Krom.getArgCount()) {
					exportMesh = true;
					exportMeshPath = Krom.getArg(v + 1);
				}
				if (currentArg == "--b" || currentArg == "--background") backgroundProcessing = true;
			}
		}
	}

	public static function run() {
		if (useArgs) {
			iron.App.notifyOnInit(function() {
				if (projectPath != "") ImportAsset.run(projectPath, -1, -1, false);
				if (meshPath != "") ImportAsset.run(meshPath, -1, -1, false);
				if (reloadMesh) Project.reimportMesh();
				if (exportTextures) {
					if (exportTexturesType == "png" ||
						exportTexturesType == "jpg"||
						exportTexturesType == "exr16"||
						exportTexturesType == "exr32") {
						if (Path.isFolder(exportTexturesPath)) {
							//Applying the correct format type from args
							if (exportTexturesType == "png") {
								App.bitsHandle.position = Bits8;
								Context.formatType = FormatPng;
							}
							if (exportTexturesType == "jpg") {
								App.bitsHandle.position = Bits8;
								Context.formatType = FormatJpg;
							}
							if (exportTexturesType == "exr16") {
								App.bitsHandle.position = Bits16;
							}
							if (exportTexturesType == "exr32") {
								App.bitsHandle.position = Bits32;
							}

							Context.layersExport = 0;

							//Get export preset and apply the correct one from args
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

							//Export queue
							function export(_) {
								ExportTexture.run(exportTexturesPath);
								iron.App.removeRender(export);
							}
							iron.App.notifyOnRender(export);
							trace("Export Textures: done");
						}
						else {
							trace("Export Textures: export directory invalid");
						}
					}
					else {
						trace("Export Textures: type invalid");
					}
				}
				if (exportMesh) {
					if (Path.isFolder(exportMeshPath)) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						ExportMesh.run(exportMeshPath + Path.sep + f, false);
						trace("Export Mesh: done");
					}
					else {
						trace("Export Mesh: export directory invalid");
					}
				}
				if (backgroundProcessing) kha.System.stop();
			});
		}
	}
}
