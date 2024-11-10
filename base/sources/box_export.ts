
let box_export_htab: ui_handle_t = ui_handle_create();
let box_export_files: string[] = null;
let box_export_mesh_handle: ui_handle_t = ui_handle_create();

let box_export_hpreset: ui_handle_t = ui_handle_create();
let box_export_preset: export_preset_t = null;
let box_export_channels: string[] = [
	"base_r",
	"base_g",
	"base_b",
	"height",
	"metal",
	"nor_r",
	"nor_g",
	"nor_g_directx",
	"nor_b",
	"occ",
	"opac",
	"rough",
	"smooth",
	"emis",
	"subs",
	"0.0",
	"1.0"
];
let box_export_color_spaces: string[] = [
	"linear",
	"srgb"
];

let _box_export_bake_material: bool;
let _box_export_t: export_preset_texture_t;
let _box_export_apply_displacement: bool;

function box_export_show_textures() {
	ui_box_show_custom(function (ui: ui_t) {

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
		///if (arm_android || arm_ios)
		box_export_tab_export_mesh(ui, box_export_htab);
		///end
		///end

	}, 540, 310);
}

function box_export_show_bake_material() {
	ui_box_show_custom(function (ui: ui_t) {

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

function box_export_tab_export_textures(ui: ui_t, title: string, bake_material: bool = false) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (ui_tab(box_export_htab, title, tab_vertical)) {

		ui_row2();

		///if (arm_android || arm_ios)
		let base_res_combo: string[] = ["128", "256", "512", "1K", "2K", "4K"];
		///else
		let base_res_combo: string[] = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
		///end

		ui_combo(base_res_handle, base_res_combo, tr("Resolution"), true);
		if (base_res_handle.changed) {
			layers_on_resized();
		}

		///if (is_lab || arm_android || arm_ios)
		let base_bits_combo: string[] = ["8bit"];
		///else
		let base_bits_combo: string[] = ["8bit", "16bit", "32bit"];
		///end

		ui_combo(base_bits_handle, base_bits_combo, tr("Color"), true);
		if (base_bits_handle.changed) {
			app_notify_on_init(layers_set_bits);
		}

		ui_row2();
		if (base_bits_handle.position == texture_bits_t.BITS8) {
			let h: ui_handle_t = ui_handle(__ID__);
			if (h.init) {
				h.position = context_raw.format_type;
			}
			let format_combo: string[] = ["png", "jpg"];
			context_raw.format_type = ui_combo(h, format_combo, tr("Format"), true);
		}
		else {
			let h: ui_handle_t = ui_handle(__ID__);
			if (h.init) {
				h.position = context_raw.format_type;
			}
			let format_combo: string[] = ["exr"];
			context_raw.format_type = ui_combo(h, format_combo, tr("Format"), true);
		}

		ui.enabled = context_raw.format_type == texture_ldr_format_t.JPG &&
					 base_bits_handle.position == texture_bits_t.BITS8;

		let h_quality: ui_handle_t = ui_handle(__ID__);
		if (h_quality.init) {
			h_quality.value = context_raw.format_quality;
		}
		context_raw.format_quality = ui_slider(h_quality, tr("Quality"), 0.0, 100.0, true, 1);

		ui.enabled = true;

		///if is_paint
		ui_row2();
		ui.enabled = !bake_material;
		let layers_export_handle: ui_handle_t = ui_handle(__ID__);
		layers_export_handle.position = context_raw.layers_export;
		let layers_export_combo: string[] = [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")];
		context_raw.layers_export = ui_combo(layers_export_handle, layers_export_combo, tr("Layers"), true);
		ui.enabled = true;
		///end

		ui_combo(box_export_hpreset, box_export_files, tr("Preset"), true);
		if (box_export_hpreset.changed) {
			box_export_preset = null;
		}

		let layers_destination_handle: ui_handle_t = ui_handle(__ID__);
		layers_destination_handle.position = context_raw.layers_destination;

		let layers_destination_combo: string[] = [tr("Disk"), tr("Packed")];
		context_raw.layers_destination = ui_combo(layers_destination_handle, layers_destination_combo, tr("Destination"), true);

		_ui_end_element();

		ui_row2();
		if (ui_button(tr("Cancel"))) {
			ui_box_hide();
		}
		if (ui_button(tr("Export"))) {
			ui_box_hide();
			if (context_raw.layers_destination == export_destination_t.PACKED) {
				_box_export_bake_material = bake_material;
				context_raw.texture_export_path = "/";
				app_notify_on_init(function () {
					export_texture_run(context_raw.texture_export_path, _box_export_bake_material);
				});
			}
			else {
				let filters: string = base_bits_handle.position != texture_bits_t.BITS8 ? "exr" : context_raw.format_type == texture_ldr_format_t.PNG ? "png" : "jpg";
				_box_export_bake_material = bake_material;
				ui_files_show(filters, true, false, function (path: string) {
					context_raw.texture_export_path = path;
					///if (arm_android || arm_ios)
					console_toast(tr("Exporting textures"));
					///end
					app_notify_on_init(function () {
						export_texture_run(context_raw.texture_export_path, _box_export_bake_material);
					});
				});
			}
		}
		if (ui.is_hovered) {
			let key: string = map_get(config_keymap, "file_export_textures");
			let tip: string = tr("Export texture files") + " (" + key + ")";
			ui_tooltip(tip);
		}
	}
}

function box_export_tab_presets(ui: ui_t) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (ui_tab(box_export_htab, tr("Presets"), tab_vertical)) {

		let row: f32[] = [3 / 5, 1 / 5, 1 / 5];
		ui_row(row);

		ui_combo(box_export_hpreset, box_export_files, tr("Preset"));
		if (box_export_hpreset.changed) {
			box_export_preset = null;
		}

		if (ui_button(tr("New"))) {
			ui_box_show_custom(function (ui: ui_t) {
				let tab_vertical: bool = config_raw.touch_ui;
				if (ui_tab(ui_handle(__ID__), tr("New Preset"), tab_vertical)) {
					ui_row2();
					let h_preset: ui_handle_t = ui_handle(__ID__);
					if (h_preset.init) {
						h_preset.text = "new_preset";
					}
					let preset_name: string = ui_text_input(h_preset, tr("Name"));
					if (ui_button(tr("OK")) || ui.is_return_down) {
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

		if (ui_button(tr("Import"))) {
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
		ui_separator(10, false);
		ui_row6();
		ui_text(tr("Texture"));
		ui_text(tr("R"));
		ui_text(tr("G"));
		ui_text(tr("B"));
		ui_text(tr("A"));
		ui_text(tr("Color Space"));
		ui.changed = false;
		for (let i: i32 = 0; i < box_export_preset.textures.length; ++i) {
			let t: export_preset_texture_t = box_export_preset.textures[i];
			ui_row6();
			let htex: ui_handle_t = ui_nest(box_export_hpreset, i);
			htex.text = t.name;
			t.name = ui_text_input(htex);

			if (ui.is_hovered && ui.input_released_r) {
				_box_export_t = t;
				ui_menu_draw(function (ui: ui_t) {
					if (ui_menu_button(ui, tr("Delete"))) {
						array_remove(box_export_preset.textures, _box_export_t);
						box_export_save_preset();
					}
				});
			}

			let hr: ui_handle_t = ui_nest(htex, 0);
			hr.position = array_index_of(box_export_channels, t.channels[0]);
			let hg: ui_handle_t = ui_nest(htex, 1);
			hg.position = array_index_of(box_export_channels, t.channels[1]);
			let hb: ui_handle_t = ui_nest(htex, 2);
			hb.position = array_index_of(box_export_channels, t.channels[2]);
			let ha: ui_handle_t = ui_nest(htex, 3);
			ha.position = array_index_of(box_export_channels, t.channels[3]);

			ui_combo(hr, box_export_channels, tr("R"));
			if (hr.changed) {
				t.channels[0] = box_export_channels[hr.position];
			}
			ui_combo(hg, box_export_channels, tr("G"));
			if (hg.changed) {
				t.channels[1] = box_export_channels[hg.position];
			}
			ui_combo(hb, box_export_channels, tr("B"));
			if (hb.changed) {
				t.channels[2] = box_export_channels[hb.position];
			}
			ui_combo(ha, box_export_channels, tr("A"));
			if (ha.changed) {
				t.channels[3] = box_export_channels[ha.position];
			}

			let hspace: ui_handle_t = ui_nest(htex, 4);
			hspace.position = array_index_of(box_export_color_spaces, t.color_space);
			ui_combo(hspace, box_export_color_spaces, tr("Color Space"));
			if (hspace.changed) {
				t.color_space = box_export_color_spaces[hspace.position];
			}
		}

		if (ui.changed) {
			box_export_save_preset();
		}

		row = [1 / 8];
		ui_row(row);
		if (ui_button(tr("Add"))) {
			let tex: export_preset_texture_t = {
				name: "base",
				channels: ["base_r", "base_g", "base_b", "1.0"],
				color_space: "linear"
			};
			array_push(box_export_preset.textures, tex);
			box_export_hpreset.children = null;
			box_export_save_preset();
		}
	}
}

function box_export_tab_atlases(ui: ui_t) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (ui_tab(box_export_htab, tr("Atlases"), tab_vertical)) {
		if (project_atlas_objects == null || project_atlas_objects.length != project_paint_objects.length) {
			project_atlas_objects = [];
			project_atlas_names = [];
			for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
				array_push(project_atlas_objects, 0);
				let i1: i32 = i + 1;
				array_push(project_atlas_names, tr("Atlas") + " " + i1);
			}
		}
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			ui_row2();
			ui_text(project_paint_objects[i].base.name);
			let hatlas: ui_handle_t = ui_nest(ui_handle(__ID__), i);
			hatlas.position = project_atlas_objects[i];
			project_atlas_objects[i] = ui_combo(hatlas, project_atlas_names, tr("Atlas"));
		}
	}
}

function box_export_show_mesh() {
	box_export_mesh_handle.position = context_raw.export_mesh_index;
	ui_box_show_custom(function (ui: ui_t) {
		let htab: ui_handle_t = ui_handle(__ID__);
		box_export_tab_export_mesh(ui, htab);
	});
}

function box_export_tab_export_mesh(ui: ui_t, htab: ui_handle_t) {
	let tab_vertical: bool = config_raw.touch_ui;
	if (ui_tab(htab, tr("Export Mesh"), tab_vertical)) {

		ui_row2();

		let h_export_mesh_format: ui_handle_t = ui_handle(__ID__);
		if (h_export_mesh_format.init) {
			h_export_mesh_format.position = context_raw.export_mesh_format;
		}
		let export_mesh_format_combo: string[] = ["obj", "arm"];
		context_raw.export_mesh_format = ui_combo(h_export_mesh_format, export_mesh_format_combo, tr("Format"), true);

		let ar: string[] = [tr("All")];
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let p: mesh_object_t = project_paint_objects[i];
			array_push(ar, p.base.name);
		}
		ui_combo(box_export_mesh_handle, ar, tr("Meshes"), true);

		let apply_displacement: bool = ui_check(ui_handle(__ID__), tr("Apply Displacement"));

		let tris: i32 = 0;
		let pos: i32 = box_export_mesh_handle.position;
		let paint_objects: mesh_object_t[];
		if (pos == 0) {
			paint_objects = project_paint_objects;
		}
		else {
			let po: mesh_object_t = project_paint_objects[pos - 1];
			paint_objects = [po];
		}
		for (let i: i32 = 0; i < paint_objects.length; ++i) {
			let po: mesh_object_t = paint_objects[i];
			for (let i: i32 = 0; i < po.data.index_arrays.length; ++i) {
				let inda: index_array_t = po.data.index_arrays[i];
				tris += math_floor(inda.values.length / 3);
			}
		}
		ui_text(tris + " " + tr("triangles"));

		ui_row2();
		if (ui_button(tr("Cancel"))) {
			ui_box_hide();
		}
		if (ui_button(tr("Export"))) {
			ui_box_hide();
			_box_export_apply_displacement = apply_displacement;
			ui_files_show(context_raw.export_mesh_format == mesh_format_t.OBJ ? "obj" : "arm", true, false, function (path: string) {
				///if (arm_android || arm_ios)
				let f: string = sys_title();
				///else
				let f: string = ui_files_filename;
				///end
				if (f == "") {
					f = tr("untitled");
				}
				///if (arm_android || arm_ios)
				console_toast(tr("Exporting mesh"));
				///end

				let paint_objects: mesh_object_t[];
				if (box_export_mesh_handle.position == 0) {
					paint_objects = null;
				}
				else {
					let po: mesh_object_t = project_paint_objects[box_export_mesh_handle.position - 1];
					paint_objects = [po];
				}

				export_mesh_run(path + path_sep + f, paint_objects, _box_export_apply_displacement);
			});
		}
	}
}

function box_export_show_material() {
	ui_box_show_custom(function (ui: ui_t) {
		let htab: ui_handle_t = ui_handle(__ID__);
		let tab_vertical: bool = config_raw.touch_ui;
		if (ui_tab(htab, tr("Export Material"), tab_vertical)) {
			let h1: ui_handle_t = ui_handle(__ID__);
			let h2: ui_handle_t = ui_handle(__ID__);
			h1.selected = context_raw.pack_assets_on_export;
			h2.selected = context_raw.write_icon_on_export;
			context_raw.pack_assets_on_export = ui_check(h1, tr("Pack Assets"));
			context_raw.write_icon_on_export = ui_check(h2, tr("Export Icon"));
			ui_row2();
			if (ui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (ui_button(tr("Export"))) {
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
	ui_box_show_custom(function (ui: ui_t) {
		let htab: ui_handle_t = ui_handle(__ID__);
		let tab_vertical: bool = config_raw.touch_ui;
		if (ui_tab(htab, tr("Export Brush"), tab_vertical)) {
			let h1: ui_handle_t = ui_handle(__ID__);
			let h2: ui_handle_t = ui_handle(__ID__);
			h1.selected = context_raw.pack_assets_on_export;
			h2.selected = context_raw.write_icon_on_export;
			context_raw.pack_assets_on_export = ui_check(h1, tr("Pack Assets"));
			context_raw.write_icon_on_export = ui_check(h2, tr("Export Icon"));
			ui_row2();
			if (ui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (ui_button(tr("Export"))) {
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

function box_export_fetch_presets() {
	box_export_files = file_read_directory(path_data() + path_sep + "export_presets");
	for (let i: i32 = 0; i < box_export_files.length; ++i) {
		let s: string = box_export_files[i];
		box_export_files[i] = substring(s, 0, s.length - 5); // Strip .json
	}
}

function box_export_parse_preset() {
	let file: string = "export_presets/" + box_export_files[box_export_hpreset.position] + ".json";
	let blob: buffer_t = data_get_blob(file);
	box_export_preset = json_parse(sys_buffer_to_string(blob));
	data_delete_blob(file);
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
	iron_file_save_bytes(path, sys_string_to_buffer(template), 0);
}

function box_export_save_preset() {
	let name: string = box_export_files[box_export_hpreset.position];
	if (name == "generic") {
		return; // generic is const
	}
	let path: string = path_data() + path_sep + "export_presets" + path_sep + name + ".json";
	iron_file_save_bytes(path, sys_string_to_buffer(box_export_preset_to_json(box_export_preset)), 0);
}

function box_export_preset_to_json(p: export_preset_t): string {
	json_encode_begin();
	json_encode_begin_array("textures");
	for (let i: i32 = 0; i < p.textures.length; ++i) {
		json_encode_begin_object();
		json_encode_string("name", p.textures[i].name);
		json_encode_string_array("channels", p.textures[i].channels);
		json_encode_string("color_space", p.textures[i].color_space);
		json_encode_end_object();
	}
	json_encode_end_array();
	return json_encode_end();
}
