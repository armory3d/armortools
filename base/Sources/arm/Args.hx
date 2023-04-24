package arm;

import arm.sys.Path;
import arm.io.ImportAsset;
import arm.io.ImportArm;
#if is_paint
import arm.io.ExportArm;
#end
#if (is_paint || is_sculpt)
import arm.io.ExportMesh;
import arm.ui.UIFiles;
import arm.ui.UIBase;
#end
#if (is_paint || is_lab)
import arm.sys.File;
import arm.io.ExportTexture;
import arm.ui.BoxExport;
#end

class Args {

	static var useArgs = false;
	static var assetPath = "";
	static var background = false;
	#if (is_paint || is_lab)
	static var exportTextures = false;
	static var exportTexturesType = "";
	static var exportTexturesPreset = "";
	static var exportTexturesPath = "";
	#end
	#if (is_paint || is_sculpt)
	static var reimportMesh = false;
	static var exportMesh = false;
	static var exportMeshPath = "";
	#end
	#if is_paint
	static var exportMaterial = false;
	static var exportMaterialPath = "";
	#end

	public static function parse() {
		if (Krom.getArgCount() > 1) {
			useArgs = true;

			var i = 0;
			while (i < Krom.getArgCount()) {
				// Process each arg
				var currentArg = Krom.getArg(i);

				if (Path.isProject(currentArg)) {
					Project.filepath = currentArg;
				}
				else if (currentArg == "--b" || currentArg == "--background") {
					background = true;
				}

				#if (is_paint || is_lab)
				else if (Path.isTexture(currentArg)) {
					assetPath = currentArg;
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
				#end

				#if (is_paint || is_sculpt)
				else if (currentArg == "--reload-mesh") {
					reimportMesh = true;
				}
				else if (currentArg == "--export-mesh" && (i + 1) <= Krom.getArgCount()) {
					exportMesh = true;
					++i;
					exportMeshPath = Krom.getArg(i);
				}
				else if (Path.isMesh(currentArg) || (i > 1 && !currentArg.startsWith("-") && Path.isFolder(currentArg))) {
					assetPath = currentArg;
				}
				#end

				#if is_paint
				else if (currentArg == "--export-material" && (i + 1) <= Krom.getArgCount()) {
					exportMaterial = true;
					++i;
					exportMaterialPath = Krom.getArg(i);
				}
				#end

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
					#if is_paint
					if (Path.isTexture(assetPath)) {
						UIBase.inst.show2DView(View2DAsset);
					}
					#end
				}
				#if (is_paint || is_sculpt)
				else if (reimportMesh) {
					Project.reimportMesh();
				}
				#end

				#if (is_paint || is_lab)
				if (exportTextures) {
					if (exportTexturesType == "png" ||
						exportTexturesType == "jpg" ||
						exportTexturesType == "exr16" ||
						exportTexturesType == "exr32") {
						if (Path.isFolder(exportTexturesPath)) {
							// Applying the correct format type from args
							if (exportTexturesType == "png") {
								#if is_paint
								App.bitsHandle.position = Bits8;
								#end
								Context.raw.formatType = FormatPng;
							}
							else if (exportTexturesType == "jpg") {
								#if is_paint
								App.bitsHandle.position = Bits8;
								#end
								Context.raw.formatType = FormatJpg;
							}
							else if (exportTexturesType == "exr16") {
								#if is_paint
								App.bitsHandle.position = Bits16;
								#end
							}
							else if (exportTexturesType == "exr32") {
								#if is_paint
								App.bitsHandle.position = Bits32;
								#end
							}

							#if is_paint
							Context.raw.layersExport = ExportVisible;
							#end

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
							Krom.log(tr("Invalid export directory"));
						}
					}
					else {
						Krom.log(tr("Invalid texture type"));
					}
				}
				#end

				#if (is_paint || is_sculpt)
				else if (exportMesh) {
					if (Path.isFolder(exportMeshPath)) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						ExportMesh.run(exportMeshPath + Path.sep + f, null, false);
					}
					else {
						Krom.log(tr("Invalid export directory"));
					}
				}
				#end

				#if is_paint
				else if (exportMaterial) {
					Context.raw.writeIconOnExport = true;
					ExportArm.runMaterial(exportMaterialPath);
				}
				#end

				if (background) kha.System.stop();
			});
		}
	}
}
