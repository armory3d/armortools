
class Args {

	static useArgs = false;
	static assetPath = "";
	static background = false;
	///if (is_paint || is_lab)
	static exportTextures = false;
	static exportTexturesType = "";
	static exportTexturesPreset = "";
	static exportTexturesPath = "";
	///end
	///if (is_paint || is_sculpt)
	static reimportMesh = false;
	static exportMesh = false;
	static exportMeshPath = "";
	///end
	///if is_paint
	static exportMaterial = false;
	static exportMaterialPath = "";
	///end

	static parse = () => {
		if (krom_get_arg_count() > 1) {
			Args.useArgs = true;

			let i = 0;
			while (i < krom_get_arg_count()) {
				// Process each arg
				let currentArg = krom_get_arg(i);

				if (Path.isProject(currentArg)) {
					Project.filepath = currentArg;
				}
				else if (currentArg == "--b" || currentArg == "--background") {
					Args.background = true;
				}

				///if (is_paint || is_lab)
				else if (Path.isTexture(currentArg)) {
					Args.assetPath = currentArg;
				}
				else if (currentArg == "--export-textures" && (i + 3) <= krom_get_arg_count()) {
					Args.exportTextures = true;
					++i;
					Args.exportTexturesType = krom_get_arg(i);
					++i;
					Args.exportTexturesPreset = krom_get_arg(i);
					++i;
					Args.exportTexturesPath = krom_get_arg(i);
				}
				///end

				///if (is_paint || is_sculpt)
				else if (currentArg == "--reload-mesh") {
					Args.reimportMesh = true;
				}
				else if (currentArg == "--export-mesh" && (i + 1) <= krom_get_arg_count()) {
					Args.exportMesh = true;
					++i;
					Args.exportMeshPath = krom_get_arg(i);
				}
				else if (Path.isMesh(currentArg) ||
						(i > 1 && !currentArg.startsWith("-") && Path.isFolder(currentArg))) {
					Args.assetPath = currentArg;
				}
				///end

				///if is_paint
				else if (currentArg == "--export-material" && (i + 1) <= krom_get_arg_count()) {
					Args.exportMaterial = true;
					++i;
					Args.exportMaterialPath = krom_get_arg(i);
				}
				///end

				++i;
			}
		}
	}

	static run = () => {
		if (Args.useArgs) {
			app_notify_on_init(() => {
				if (Project.filepath != "") {
					ImportArm.runProject(Project.filepath);
				}
				else if (Args.assetPath != "") {
					ImportAsset.run(Args.assetPath, -1, -1, false);
					///if is_paint
					if (Path.isTexture(Args.assetPath)) {
						UIBase.show2DView(View2DType.View2DAsset);
					}
					///end
				}
				///if (is_paint || is_sculpt)
				else if (Args.reimportMesh) {
					Project.reimportMesh();
				}
				///end

				///if (is_paint || is_lab)
				if (Args.exportTextures) {
					if (Args.exportTexturesType == "png" ||
						Args.exportTexturesType == "jpg" ||
						Args.exportTexturesType == "exr16" ||
						Args.exportTexturesType == "exr32") {
						if (Path.isFolder(Args.exportTexturesPath)) {
							// Applying the correct format type from args
							if (Args.exportTexturesType == "png") {
								///if is_paint
								Base.bitsHandle.position = TextureBits.Bits8;
								///end
								Context.raw.formatType = TextureLdrFormat.FormatPng;
							}
							else if (Args.exportTexturesType == "jpg") {
								///if is_paint
								Base.bitsHandle.position = TextureBits.Bits8;
								///end
								Context.raw.formatType = TextureLdrFormat.FormatJpg;
							}
							else if (Args.exportTexturesType == "exr16") {
								///if is_paint
								Base.bitsHandle.position = TextureBits.Bits16;
								///end
							}
							else if (Args.exportTexturesType == "exr32") {
								///if is_paint
								Base.bitsHandle.position = TextureBits.Bits32;
								///end
							}

							///if is_paint
							Context.raw.layersExport = ExportMode.ExportVisible;
							///end

							// Get export preset and apply the correct one from args
							BoxExport.files = File.readDirectory(Path.data() + Path.sep + "export_presets");
							for (let i = 0; i < BoxExport.files.length; ++i) {
								BoxExport.files[i] = BoxExport.files[i].substr(0, BoxExport.files[i].length - 5); // Strip .json
							}

							let file = "export_presets/" + BoxExport.files[0] + ".json";
							for (let f of BoxExport.files) if (f == Args.exportTexturesPreset) {
								file = "export_presets/" + BoxExport.files[BoxExport.files.indexOf(f)] + ".json";
							}

							data_get_blob(file, (blob: ArrayBuffer) => {
								BoxExport.preset = JSON.parse(sys_buffer_to_string(blob));
								data_delete_blob("export_presets/" + file);
							});

							// Export queue
							app_notify_on_init(() => {
								ExportTexture.run(Args.exportTexturesPath);
							});
						}
						else {
							krom_log(tr("Invalid export directory"));
						}
					}
					else {
						krom_log(tr("Invalid texture type"));
					}
				}
				///end

				///if (is_paint || is_sculpt)
				else if (Args.exportMesh) {
					if (Path.isFolder(Args.exportMeshPath)) {
						let f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						ExportMesh.run(Args.exportMeshPath + Path.sep + f, null, false);
					}
					else {
						krom_log(tr("Invalid export directory"));
					}
				}
				///end

				///if is_paint
				else if (Args.exportMaterial) {
					Context.raw.writeIconOnExport = true;
					ExportArm.runMaterial(Args.exportMaterialPath);
				}
				///end

				if (Args.background) sys_stop();
			});
		}
	}
}
