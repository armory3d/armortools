
let box_export_htab: zui_handle_t = zui_handle_create();
let box_export_files: string[] = null;
let box_export_mesh_handle: zui_handle_t = zui_handle_create();

///if (is_paint || is_lab)
let box_export_hpreset: zui_handle_t = zui_handle_create();
let box_export_preset: export_preset_t = null;
let box_export_channels: string[] = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_g_directx", "nor_b", "occ", "opac", "rough", "smooth", "emis", "subs", "0.0", "1.0"];
let box_export_color_spaces: string[] = ["linear", "srgb"];
///end

///if (is_paint || is_lab)
function box_export_show_textures() {
	ui_box_show_custom(function (ui: zui_t) {

		if (box_export_files == null) {
			box_export_fetch_presets();
			box_export_hpreset.position = array_index_of(box_export_files, "generic");
		}
		if (box_export_preset == null) {
			box_export_parse_preset();
			box_export_hpreset.children = null;
		}

		box_export_tab_export_textures(ui, tr("Export Textures"));
		box_export_tab_presets(ui);

		///if is_paint
		box_export_tab_atlases(ui);
		///if (krom_android || krom_ios)
		box_export_tab_export_mesh(ui, box_export_htab);
		///end
		///end

	}, 540, 310);
}
///end

///if is_paint
function box_export_show_bake_material() {
	ui_box_show_custom(function (ui: zui_t) {

		if (box_export_files == null) {
			box_export_fetch_presets();
			box_export_hpreset.position = array_index_of(box_export_files, "generic");
		}
		if (box_export_preset == null) {
			box_export_parse_preset();
			box_export_hpreset.children = null;
		}

		box_export_tab_export_textures(ui, tr("Bake to Textures"), true);
		box_export_tab_presets(ui);

	}, 540, 310);
}
///end

let _box_export_bake_material: bool;

///if (is_paint || is_lab)
function box_export_tab_export_textures(ui: zui_t, title: string, bake_material: bool = false) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (zui_tab(box_export_htab, title, tab_vertical)) {

		zui_row([0.5, 0.5]);

		///if is_paint
		///if (krom_android || krom_ios)
		zui_combo(base_res_handle, ["128", "256", "512", "1K", "2K", "4K"], tr("Resolution"), true);
		///else
		zui_combo(base_res_handle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], tr("Resolution"), true);
		///end
		///end

		///if is_lab
		///if (krom_android || krom_ios)
		zui_combo(base_res_handle, ["2K", "4K"], tr("Resolution"), true);
		///else
		zui_combo(base_res_handle, ["2K", "4K", "8K", "16K"], tr("Resolution"), true);
		///end
		///end

		if (base_res_handle.changed) {
			base_on_layers_resized();
		}

		///if (is_lab || krom_android || krom_ios)
		zui_combo(base_bits_handle, ["8bit"], tr("Color"), true);
		///else
		zui_combo(base_bits_handle, ["8bit", "16bit", "32bit"], tr("Color"), true);
		///end

		///if is_paint
		if (base_bits_handle.changed) {
			app_notify_on_init(base_set_layer_bits);
		}
		///end

		zui_row([0.5, 0.5]);
		if (base_bits_handle.position == texture_bits_t.BITS8) {
			context_raw.format_type = zui_combo(zui_handle(__ID__, { position: context_raw.format_type }), ["png", "jpg"], tr("Format"), true);
		}
		else {
			context_raw.format_type = zui_combo(zui_handle(__ID__, { position: context_raw.format_type }), ["exr"], tr("Format"), true);
		}

		ui.enabled = context_raw.format_type == texture_ldr_format_t.JPG && base_bits_handle.position == texture_bits_t.BITS8;
		context_raw.format_quality = zui_slider(zui_handle(__ID__, { value: context_raw.format_quality }), tr("Quality"), 0.0, 100.0, true, 1);
		ui.enabled = true;

		///if is_paint
		zui_row([0.5, 0.5]);
		ui.enabled = !bake_material;
		let layers_export_handle: zui_handle_t = zui_handle(__ID__);
		layers_export_handle.position = context_raw.layers_export;
		context_raw.layers_export = zui_combo(layers_export_handle, [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")], tr("Layers"), true);
		ui.enabled = true;
		///end

		zui_combo(box_export_hpreset, box_export_files, tr("Preset"), true);
		if (box_export_hpreset.changed) box_export_preset = null;

		let layers_destination_handle: zui_handle_t = zui_handle(__ID__);
		layers_destination_handle.position = context_raw.layers_destination;
		context_raw.layers_destination = zui_combo(layers_destination_handle, [tr("Disk"), tr("Packed")], tr("Destination"), true);

		zui_end_element();

		zui_row([0.5, 0.5]);
		if (zui_button(tr("Cancel"))) {
			ui_box_hide();
		}
		if (zui_button(tr("Export"))) {
			ui_box_hide();
			if (context_raw.layers_destination == export_destination_t.PACKED) {
				_box_export_bake_material = bake_material;
				context_raw.texture_export_path = "/";
				app_notify_on_init(function () {
					///if is_paint
					export_texture_run(context_raw.texture_export_path, _box_export_bake_material);
					///end
					///if is_lab
					export_texture_run(context_raw.texture_export_path);
					///end
				});
			}
			else {
				let filters = base_bits_handle.position != texture_bits_t.BITS8 ? "exr" : context_raw.format_type == texture_ldr_format_t.PNG ? "png" : "jpg";
				ui_files_show(filters, true, false, function (path: string) {
					context_raw.texture_export_path = path;
					///if (krom_android || krom_ios)
					console_toast(tr("Exporting textures"));
					krom_g4_swap_buffers();
					///end
					_box_export_bake_material = bake_material;
					app_notify_on_init(function () {
						///if is_paint
						export_texture_run(context_raw.texture_export_path, _box_export_bake_material);
						///end
						///if is_lab
						export_texture_run(context_raw.texture_export_path);
						///end
					});
				});
			}
		}
		if (ui.is_hovered) {
			zui_tooltip(tr("Export texture files") + " (" + map_get(config_keymap, "file_export_textures") + ")");
		}
	}
}

let _box_export_t: export_preset_texture_t;

function box_export_tab_presets(ui: zui_t) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (zui_tab(box_export_htab, tr("Presets"), tab_vertical)) {
		zui_row([3 / 5, 1 / 5, 1 / 5]);

		zui_combo(box_export_hpreset, box_export_files, tr("Preset"));
		if (box_export_hpreset.changed) {
			box_export_preset = null;
		}

		if (zui_button(tr("New"))) {
			ui_box_show_custom(function (ui: zui_t) {
				let tab_vertical: bool = config_raw.touch_ui;
				if (zui_tab(zui_handle(__ID__), tr("New Preset"), tab_vertical)) {
					zui_row([0.5, 0.5]);
					let preset_name: string = zui_text_input(zui_handle(__ID__, { text: "new_preset" }), tr("Name"));
					if (zui_button(tr("OK")) || ui.is_return_down) {
						box_export_new_preset(preset_name);
						box_export_fetch_presets();
						box_export_preset = null;
						box_export_hpreset.position = array_index_of(box_export_files, preset_name);
						ui_box_hide();
						box_export_htab.position = 1; // Presets
						box_export_show_textures();
					}
				}
			});
		}

		if (zui_button(tr("Import"))) {
			ui_files_show("json", false, false, function (path: string) {
				path = to_lower_case(path);
				if (ends_with(path, ".json")) {
					let filename: string = substring(path, string_last_index_of(path, path_sep) + 1, path.length);
					let dst_path: string = path_data() + path_sep + "export_presets" + path_sep + filename;
					file_copy(path, dst_path); // Copy to presets folder
					box_export_fetch_presets();
					box_export_preset = null;
					box_export_hpreset.position = array_index_of(box_export_files, substring(filename, 0, filename.length - 5)); // Strip .json
					console_info(tr("Preset imported:") + " " + filename);
				}
				else {
					console_error(strings_error1());
				}
			});
		}

		if (box_export_preset == null) {
			box_export_parse_preset();
			box_export_hpreset.children = null;
		}

		// Texture list
		zui_separator(10, false);
		zui_row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
		zui_text(tr("Texture"));
		zui_text(tr("R"));
		zui_text(tr("G"));
		zui_text(tr("B"));
		zui_text(tr("A"));
		zui_text(tr("Color Space"));
		ui.changed = false;
		for (let i: i32 = 0; i < box_export_preset.textures.length; ++i) {
			let t: export_preset_texture_t = box_export_preset.textures[i];
			zui_row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
			let htex: zui_handle_t = zui_nest(box_export_hpreset, i);
			htex.text = t.name;
			t.name = zui_text_input(htex);

			if (ui.is_hovered && ui.input_released_r) {
				_box_export_t = t;
				ui_menu_draw(function (ui: zui_t) {
					if (ui_menu_button(ui, tr("Delete"))) {
						array_remove(box_export_preset.textures, _box_export_t);
						box_export_save_preset();
					}
				}, 1);
			}

			let hr: zui_handle_t = zui_nest(htex, 0);
			hr.position = array_index_of(box_export_channels, t.channels[0]);
			let hg: zui_handle_t = zui_nest(htex, 1);
			hg.position = array_index_of(box_export_channels, t.channels[1]);
			let hb: zui_handle_t = zui_nest(htex, 2);
			hb.position = array_index_of(box_export_channels, t.channels[2]);
			let ha: zui_handle_t = zui_nest(htex, 3);
			ha.position = array_index_of(box_export_channels, t.channels[3]);

			zui_combo(hr, box_export_channels, tr("R"));
			if (hr.changed) {
				t.channels[0] = box_export_channels[hr.position];
			}
			zui_combo(hg, box_export_channels, tr("G"));
			if (hg.changed) {
				t.channels[1] = box_export_channels[hg.position];
			}
			zui_combo(hb, box_export_channels, tr("B"));
			if (hb.changed) {
				t.channels[2] = box_export_channels[hb.position];
			}
			zui_combo(ha, box_export_channels, tr("A"));
			if (ha.changed) {
				t.channels[3] = box_export_channels[ha.position];
			}

			let hspace: zui_handle_t = zui_nest(htex, 4);
			hspace.position = array_index_of(box_export_color_spaces, t.color_space);
			zui_combo(hspace, box_export_color_spaces, tr("Color Space"));
			if (hspace.changed) {
				t.color_space = box_export_color_spaces[hspace.position];
			}
		}

		if (ui.changed) {
			box_export_save_preset();
		}

		zui_row([1 / 8]);
		if (zui_button(tr("Add"))) {
			array_push(box_export_preset.textures, { name: "base", channels: ["base_r", "base_g", "base_b", "1.0"], color_space: "linear" });
			box_export_hpreset.children = null;
			box_export_save_preset();
		}
	}
}
///end

///if is_paint
function box_export_tab_atlases(ui: zui_t) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (zui_tab(box_export_htab, tr("Atlases"), tab_vertical)) {
		if (project_atlas_objects == null || project_atlas_objects.length != project_paint_objects.length) {
			project_atlas_objects = [];
			project_atlas_names = [];
			for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
				array_push(project_atlas_objects, 0);
				array_push(project_atlas_names, tr("Atlas") + " " + (i + 1));
			}
		}
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			zui_row([1 / 2, 1 / 2]);
			zui_text(project_paint_objects[i].base.name);
			let hatlas: zui_handle_t = zui_nest(zui_handle(__ID__), i);
			hatlas.position = project_atlas_objects[i];
			project_atlas_objects[i] = zui_combo(hatlas, project_atlas_names, tr("Atlas"));
		}
	}
}
///end

function box_export_show_mesh() {
	box_export_mesh_handle.position = context_raw.export_mesh_index;
	ui_box_show_custom(function (ui: zui_t) {
		let htab: zui_handle_t = zui_handle(__ID__);
		box_export_tab_export_mesh(ui, htab);
	});
}

function box_export_tab_export_mesh(ui: zui_t, htab: zui_handle_t) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (zui_tab(htab, tr("Export Mesh"), tab_vertical)) {

		zui_row([1 / 2, 1 / 2]);

		context_raw.export_mesh_format = zui_combo(zui_handle(__ID__, { position: context_raw.export_mesh_format }), ["obj", "arm"], tr("Format"), true);

		let ar: string[] = [tr("All")];
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let p: mesh_object_t = project_paint_objects[i];
			array_push(ar, p.base.name);
		}
		zui_combo(box_export_mesh_handle, ar, tr("Meshes"), true);

		let apply_displacement: bool = zui_check(zui_handle(__ID__), tr("Apply Displacement"));

		let tris: i32 = 0;
		let pos: i32 = box_export_mesh_handle.position;
		let paint_objects: mesh_object_t[] = pos == 0 ? project_paint_objects : [project_paint_objects[pos - 1]];
		for (let i: i32 = 0; i < paint_objects.length; ++i) {
			let po: mesh_object_t = paint_objects[i];
			for (let i: i32 = 0; i < po.data.index_arrays.length; ++i) {
				let inda: index_array_t = po.data.index_arrays[i];
				tris += math_floor(inda.values.length / 3);
			}
		}
		zui_text(tris + " " + tr("triangles"));

		zui_row([0.5, 0.5]);
		if (zui_button(tr("Cancel"))) {
			ui_box_hide();
		}
		if (zui_button(tr("Export"))) {
			ui_box_hide();
			ui_files_show(context_raw.export_mesh_format == mesh_format_t.OBJ ? "obj" : "arm", true, false, function (path: string) {
				///if (krom_android || krom_ios)
				let f: string = sys_title();
				///else
				let f: string = ui_files_filename;
				///end
				if (f == "") {
					f = tr("untitled");
				}
				///if (krom_android || krom_ios)
				console_toast(tr("Exporting mesh"));
				krom_g4_swap_buffers();
				///end
				export_mesh_run(path + path_sep + f, box_export_mesh_handle.position == 0 ? null : [project_paint_objects[box_export_mesh_handle.position - 1]], apply_displacement);
			});
		}
	}
}

///if (is_paint || is_sculpt)
function box_export_show_material() {
	ui_box_show_custom(function (ui: zui_t) {
		let htab: zui_handle_t = zui_handle(__ID__);
		let tab_vertical: bool = config_raw.touch_ui;
		if (zui_tab(htab, tr("Export Material"), tab_vertical)) {
			let h1: zui_handle_t = zui_handle(__ID__);
			let h2: zui_handle_t = zui_handle(__ID__);
			h1.selected = context_raw.pack_assets_on_export;
			h2.selected = context_raw.write_icon_on_export;
			context_raw.pack_assets_on_export = zui_check(h1, tr("Pack Assets"));
			context_raw.write_icon_on_export = zui_check(h2, tr("Export Icon"));
			zui_row([0.5, 0.5]);
			if (zui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (zui_button(tr("Export"))) {
				ui_box_hide();
				ui_files_show("arm", true, false, function (path: string) {
					let f: string = ui_files_filename;
					if (f == "") {
						f = tr("untitled");
					}
					app_notify_on_init(function (path: string) {
						export_arm_run_material(path);
					}, path + path_sep + f);
				});
			}
		}
	});
}

function box_export_show_brush() {
	ui_box_show_custom(function (ui: zui_t) {
		let htab: zui_handle_t = zui_handle(__ID__);
		let tab_vertical: bool = config_raw.touch_ui;
		if (zui_tab(htab, tr("Export Brush"), tab_vertical)) {
			let h1: zui_handle_t = zui_handle(__ID__);
			let h2: zui_handle_t = zui_handle(__ID__);
			h1.selected = context_raw.pack_assets_on_export;
			h2.selected = context_raw.write_icon_on_export;
			context_raw.pack_assets_on_export = zui_check(h1, tr("Pack Assets"));
			context_raw.write_icon_on_export = zui_check(h2, tr("Export Icon"));
			zui_row([0.5, 0.5]);
			if (zui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (zui_button(tr("Export"))) {
				ui_box_hide();
				ui_files_show("arm", true, false, function (path: string) {
					let f: string = ui_files_filename;
					if (f == "") f = tr("untitled");
					app_notify_on_init(function (path: string) {
						export_arm_run_brush(path);
					}, path + path_sep + f);
				});
			}
		}
	});
}
///end

///if (is_paint || is_lab)
function box_export_fetch_presets() {
	box_export_files = file_read_directory(path_data() + path_sep + "export_presets");
	for (let i: i32 = 0; i < box_export_files.length; ++i) {
		box_export_files[i] = substring(box_export_files[i], 0, box_export_files[i].length - 5); // Strip .json
	}
}

function box_export_parse_preset() {
	let file: string = "export_presets/" + box_export_files[box_export_hpreset.position] + ".json";
	let blob: buffer_t = data_get_blob(file);
	box_export_preset = json_parse(sys_buffer_to_string(blob));
	data_delete_blob("export_presets/" + file);
}

function box_export_new_preset(name: string) {
	let template: string =
"{\
\"textures\": [\
	{ \"name\": \"base\", \"channels\": [\"base_r\", \"base_g\", \"base_b\", \"1.0\"], \"color_space\": \"linear\" }\
]\
}\
";
	if (!ends_with(name, ".json")) {
		name += ".json";
	}
	let path: string = path_data() + path_sep + "export_presets" + path_sep + name;
	krom_file_save_bytes(path, sys_string_to_buffer(template));
}

function box_export_save_preset() {
	let name: string = box_export_files[box_export_hpreset.position];
	if (name == "generic") {
		return; // generic is const
	}
	let path: string = path_data() + path_sep + "export_presets" + path_sep + name + ".json";
	krom_file_save_bytes(path, sys_string_to_buffer(json_stringify(box_export_preset)));
}
///end
