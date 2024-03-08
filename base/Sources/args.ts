
let args_use: bool = false;
let args_asset_path: string = "";
let args_background: bool = false;
///if (is_paint || is_lab)
let args_export_textures: bool = false;
let args_export_textures_type: string = "";
let args_export_textures_preset: string = "";
let args_export_textures_path: string = "";
///end
///if (is_paint || is_sculpt)
let args_reimport_mesh: bool = false;
let args_export_mesh: bool = false;
let args_export_mesh_path: string = "";
///end
///if is_paint
let args_export_material: bool = false;
let args_export_material_path: string = "";
///end

function args_parse() {
	if (krom_get_arg_count() > 1) {
		args_use = true;

		let i: i32 = 0;
		while (i < krom_get_arg_count()) {
			// Process each arg
			let current_arg: string = krom_get_arg(i);

			if (Path.is_project(current_arg)) {
				Project.filepath = current_arg;
			}
			else if (current_arg == "--b" || current_arg == "--background") {
				args_background = true;
			}

			///if (is_paint || is_lab)
			else if (Path.is_texture(current_arg)) {
				args_asset_path = current_arg;
			}
			else if (current_arg == "--export-textures" && (i + 3) <= krom_get_arg_count()) {
				args_export_textures = true;
				++i;
				args_export_textures_type = krom_get_arg(i);
				++i;
				args_export_textures_preset = krom_get_arg(i);
				++i;
				args_export_textures_path = krom_get_arg(i);
			}
			///end

			///if (is_paint || is_sculpt)
			else if (current_arg == "--reload-mesh") {
				args_reimport_mesh = true;
			}
			else if (current_arg == "--export-mesh" && (i + 1) <= krom_get_arg_count()) {
				args_export_mesh = true;
				++i;
				args_export_mesh_path = krom_get_arg(i);
			}
			else if (Path.is_mesh(current_arg) || (i > 1 && !current_arg.startsWith("-") && Path.is_folder(current_arg))) {
				args_asset_path = current_arg;
			}
			///end

			///if is_paint
			else if (current_arg == "--export-material" && (i + 1) <= krom_get_arg_count()) {
				args_export_material = true;
				++i;
				args_export_material_path = krom_get_arg(i);
			}
			///end

			++i;
		}
	}
}

function args_run() {
	if (args_use) {
		app_notify_on_init(() => {
			if (Project.filepath != "") {
				ImportArm.run_project(Project.filepath);
			}
			else if (args_asset_path != "") {
				ImportAsset.run(args_asset_path, -1, -1, false);
				///if is_paint
				if (Path.is_texture(args_asset_path)) {
					UIBase.show_2d_view(view_2d_type_t.ASSET);
				}
				///end
			}
			///if (is_paint || is_sculpt)
			else if (args_reimport_mesh) {
				Project.reimport_mesh();
			}
			///end

			///if (is_paint || is_lab)
			if (args_export_textures) {
				if (args_export_textures_type == "png" ||
					args_export_textures_type == "jpg" ||
					args_export_textures_type == "exr16" ||
					args_export_textures_type == "exr32") {
					if (Path.is_folder(args_export_textures_path)) {
						// Applying the correct format type from args
						if (args_export_textures_type == "png") {
							///if is_paint
							base_bits_handle.position = texture_bits_t.BITS8;
							///end
							Context.raw.format_type = texture_ldr_format_t.PNG;
						}
						else if (args_export_textures_type == "jpg") {
							///if is_paint
							base_bits_handle.position = texture_bits_t.BITS8;
							///end
							Context.raw.format_type = texture_ldr_format_t.JPG;
						}
						else if (args_export_textures_type == "exr16") {
							///if is_paint
							base_bits_handle.position = texture_bits_t.BITS16;
							///end
						}
						else if (args_export_textures_type == "exr32") {
							///if is_paint
							base_bits_handle.position = texture_bits_t.BITS32;
							///end
						}

						///if is_paint
						Context.raw.layers_export = export_mode_t.VISIBLE;
						///end

						// Get export preset and apply the correct one from args
						BoxExport.files = File.read_directory(Path.data() + Path.sep + "export_presets");
						for (let i: i32 = 0; i < BoxExport.files.length; ++i) {
							BoxExport.files[i] = BoxExport.files[i].substr(0, BoxExport.files[i].length - 5); // Strip .json
						}

						let file: string = "export_presets/" + BoxExport.files[0] + ".json";
						for (let f of BoxExport.files) if (f == args_export_textures_preset) {
							file = "export_presets/" + BoxExport.files[BoxExport.files.indexOf(f)] + ".json";
						}

						let blob: ArrayBuffer = data_get_blob(file);
						BoxExport.preset = JSON.parse(sys_buffer_to_string(blob));
						data_delete_blob("export_presets/" + file);

						// Export queue
						app_notify_on_init(() => {
							ExportTexture.run(args_export_textures_path);
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
			else if (args_export_mesh) {
				if (Path.is_folder(args_export_mesh_path)) {
					let f: string = UIFiles.filename;
					if (f == "") f = tr("untitled");
					ExportMesh.run(args_export_mesh_path + Path.sep + f, null, false);
				}
				else {
					krom_log(tr("Invalid export directory"));
				}
			}
			///end

			///if is_paint
			else if (args_export_material) {
				Context.raw.write_icon_on_export = true;
				ExportArm.run_material(args_export_material_path);
			}
			///end

			if (args_background) sys_stop();
		});
	}
}
