
let args_use: bool = false;
let args_asset_path: string = "";
let args_background: bool = false;
let args_export_textures: bool = false;
let args_export_textures_type: string = "";
let args_export_textures_preset: string = "";
let args_export_textures_path: string = "";
let args_reimport_mesh: bool = false;
let args_export_mesh: bool = false;
let args_export_mesh_path: string = "";
let args_export_material: bool = false;
let args_export_material_path: string = "";

function args_parse() {
	if (iron_get_arg_count() > 1) {
		args_use = true;

		let i: i32 = 0;
		while (i < iron_get_arg_count()) {
			// Process each arg
			let current_arg: string = iron_get_arg(i);

			if (path_is_project(current_arg)) {
				project_filepath = current_arg;
			}
			else if (current_arg == "--b" || current_arg == "--background") {
				args_background = true;
			}

			///if (is_paint || is_lab)
			else if (path_is_texture(current_arg)) {
				args_asset_path = current_arg;
			}
			else if (current_arg == "--export-textures" && (i + 3) <= iron_get_arg_count()) {
				args_export_textures = true;
				++i;
				args_export_textures_type = iron_get_arg(i);
				++i;
				args_export_textures_preset = iron_get_arg(i);
				++i;
				args_export_textures_path = iron_get_arg(i);
			}
			///end

			///if (is_paint || is_sculpt)
			else if (current_arg == "--reload-mesh") {
				args_reimport_mesh = true;
			}
			else if (current_arg == "--export-mesh" && (i + 1) <= iron_get_arg_count()) {
				args_export_mesh = true;
				++i;
				args_export_mesh_path = iron_get_arg(i);
			}
			else if (path_is_mesh(current_arg) || (i > 1 && !starts_with(current_arg, "-") && path_is_folder(current_arg))) {
				args_asset_path = current_arg;
			}
			///end

			///if is_paint
			else if (current_arg == "--export-material" && (i + 1) <= iron_get_arg_count()) {
				args_export_material = true;
				++i;
				args_export_material_path = iron_get_arg(i);
			}
			///end

			++i;
		}
	}
}

function args_run() {
	if (args_use) {
		app_notify_on_init(function () {
			if (project_filepath != "") {
				import_arm_run_project(project_filepath);
			}
			else if (args_asset_path != "") {
				import_asset_run(args_asset_path, -1, -1, false);
				///if is_paint
				if (path_is_texture(args_asset_path)) {
					ui_base_show_2d_view(view_2d_type_t.ASSET);
				}
				///end
			}
			///if (is_paint || is_sculpt)
			else if (args_reimport_mesh) {
				project_reimport_mesh();
			}
			///end

			///if (is_paint || is_lab)
			if (args_export_textures) {
				if (args_export_textures_type == "png" ||
					args_export_textures_type == "jpg" ||
					args_export_textures_type == "exr16" ||
					args_export_textures_type == "exr32") {
					if (path_is_folder(args_export_textures_path)) {
						// Applying the correct format type from args
						if (args_export_textures_type == "png") {
							///if is_paint
							base_bits_handle.position = texture_bits_t.BITS8;
							///end
							context_raw.format_type = texture_ldr_format_t.PNG;
						}
						else if (args_export_textures_type == "jpg") {
							///if is_paint
							base_bits_handle.position = texture_bits_t.BITS8;
							///end
							context_raw.format_type = texture_ldr_format_t.JPG;
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
						context_raw.layers_export = export_mode_t.VISIBLE;
						///end

						// Get export preset and apply the correct one from args
						box_export_files = file_read_directory(path_data() + path_sep + "export_presets");
						for (let i: i32 = 0; i < box_export_files.length; ++i) {
							let s: string = box_export_files[i];
							box_export_files[i] = substring(s, 0, s.length - 5); // Strip .json
						}

						let file: string = "export_presets/" + box_export_files[0] + ".json";
						for (let i: i32 = 0; i < box_export_files.length; ++i) {
							let f: string = box_export_files[i];
							if (f == args_export_textures_preset) {
								file = "export_presets/" + box_export_files[array_index_of(box_export_files, f)] + ".json";
							}
						}

						let blob: buffer_t = data_get_blob(file);
						box_export_preset = json_parse(sys_buffer_to_string(blob));
						data_delete_blob("export_presets/" + file);

						// Export queue
						app_notify_on_init(function () {
							export_texture_run(args_export_textures_path);
						});
					}
					else {
						iron_log(tr("Invalid export directory"));
					}
				}
				else {
					iron_log(tr("Invalid texture type"));
				}
			}
			///end

			///if (is_paint || is_sculpt)
			else if (args_export_mesh) {
				if (path_is_folder(args_export_mesh_path)) {
					let f: string = ui_files_filename;
					if (f == "") {
						f = tr("untitled");
					}
					export_mesh_run(args_export_mesh_path + path_sep + f, null, false);
				}
				else {
					iron_log(tr("Invalid export directory"));
				}
			}
			///end

			///if is_paint
			else if (args_export_material) {
				context_raw.write_icon_on_export = true;
				export_arm_run_material(args_export_material_path);
			}
			///end

			if (args_background) {
				sys_stop();
			}
		});
	}
}
