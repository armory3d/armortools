
class BoxExport {

	static htab: zui_handle_t = zui_handle_create();
	static files: string[] = null;
	static export_mesh_handle: zui_handle_t = zui_handle_create();

	///if (is_paint || is_lab)
	static hpreset: zui_handle_t = zui_handle_create();
	static preset: export_preset_t = null;
	static channels: string[] = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_g_directx", "nor_b", "occ", "opac", "rough", "smooth", "emis", "subs", "0.0", "1.0"];
	static color_spaces: string[] = ["linear", "srgb"];
	///end

	///if (is_paint || is_lab)
	static show_textures = () => {
		UIBox.show_custom((ui: zui_t) => {

			if (BoxExport.files == null) {
				BoxExport.fetch_presets();
				BoxExport.hpreset.position = BoxExport.files.indexOf("generic");
			}
			if (BoxExport.preset == null) {
				BoxExport.parse_preset();
				BoxExport.hpreset.children = null;
			}

			BoxExport.tab_export_textures(ui, tr("Export Textures"));
			BoxExport.tab_presets(ui);

			///if is_paint
			BoxExport.tab_atlases(ui);
			///if (krom_android || krom_ios)
			BoxExport.tab_export_mesh(ui, BoxExport.htab);
			///end
			///end

		}, 540, 310);
	}
	///end

	///if is_paint
	static show_bake_material = () => {
		UIBox.show_custom((ui: zui_t) => {

			if (BoxExport.files == null) {
				BoxExport.fetch_presets();
				BoxExport.hpreset.position = BoxExport.files.indexOf("generic");
			}
			if (BoxExport.preset == null) {
				BoxExport.parse_preset();
				BoxExport.hpreset.children = null;
			}

			BoxExport.tab_export_textures(ui, tr("Bake to Textures"), true);
			BoxExport.tab_presets(ui);

		}, 540, 310);
	}
	///end

	///if (is_paint || is_lab)
	static tab_export_textures = (ui: zui_t, title: string, bake_material: bool = false) => {
		let tab_vertical: bool = Config.raw.touch_ui;
		if (zui_tab(BoxExport.htab, title, tab_vertical)) {

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
				Context.raw.format_type = zui_combo(zui_handle("boxexport_0", { position: Context.raw.format_type }), ["png", "jpg"], tr("Format"), true);
			}
			else {
				Context.raw.format_type = zui_combo(zui_handle("boxexport_1", { position: Context.raw.format_type }), ["exr"], tr("Format"), true);
			}

			ui.enabled = Context.raw.format_type == texture_ldr_format_t.JPG && base_bits_handle.position == texture_bits_t.BITS8;
			Context.raw.format_quality = zui_slider(zui_handle("boxexport_2", { value: Context.raw.format_quality }), tr("Quality"), 0.0, 100.0, true, 1);
			ui.enabled = true;

			///if is_paint
			zui_row([0.5, 0.5]);
			ui.enabled = !bake_material;
			let layers_export_handle: zui_handle_t = zui_handle("boxexport_3");
			layers_export_handle.position = Context.raw.layers_export;
			Context.raw.layers_export = zui_combo(layers_export_handle, [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")], tr("Layers"), true);
			ui.enabled = true;
			///end

			zui_combo(BoxExport.hpreset, BoxExport.files, tr("Preset"), true);
			if (BoxExport.hpreset.changed) BoxExport.preset = null;

			let layers_destination_handle: zui_handle_t = zui_handle("boxexport_4");
			layers_destination_handle.position = Context.raw.layers_destination;
			Context.raw.layers_destination = zui_combo(layers_destination_handle, [tr("Disk"), tr("Packed")], tr("Destination"), true);

			zui_end_element();

			zui_row([0.5, 0.5]);
			if (zui_button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (zui_button(tr("Export"))) {
				UIBox.hide();
				if (Context.raw.layers_destination == export_destination_t.PACKED) {
					Context.raw.texture_export_path = "/";
					let _init = () => {
						///if is_paint
						ExportTexture.run(Context.raw.texture_export_path, bake_material);
						///end
						///if is_lab
						ExportTexture.run(Context.raw.texture_export_path);
						///end
					}
					app_notify_on_init(_init);
				}
				else {
					let filters = base_bits_handle.position != texture_bits_t.BITS8 ? "exr" : Context.raw.format_type == texture_ldr_format_t.PNG ? "png" : "jpg";
					UIFiles.show(filters, true, false, (path: string) => {
						Context.raw.texture_export_path = path;
						let doExport = () => {
							let _init = () => {
								///if is_paint
								ExportTexture.run(Context.raw.texture_export_path, bake_material);
								///end
								///if is_lab
								ExportTexture.run(Context.raw.texture_export_path);
								///end
							}
							app_notify_on_init(_init);
						}
						///if (krom_android || krom_ios)
						base_notify_on_next_frame(() => {
							Console.toast(tr("Exporting textures"));
							base_notify_on_next_frame(doExport);
						});
						///else
						doExport();
						///end
					});
				}
			}
			if (ui.is_hovered) zui_tooltip(tr("Export texture files") + ` (${Config.keymap.file_export_textures})`);
		}
	}

	static tab_presets = (ui: zui_t) => {
		let tab_vertical: bool = Config.raw.touch_ui;
		if (zui_tab(BoxExport.htab, tr("Presets"), tab_vertical)) {
			zui_row([3 / 5, 1 / 5, 1 / 5]);

			zui_combo(BoxExport.hpreset, BoxExport.files, tr("Preset"));
			if (BoxExport.hpreset.changed) BoxExport.preset = null;

			if (zui_button(tr("New"))) {
				UIBox.show_custom((ui: zui_t) => {
					let tab_vertical: bool = Config.raw.touch_ui;
					if (zui_tab(zui_handle("boxexport_5"), tr("New Preset"), tab_vertical)) {
						zui_row([0.5, 0.5]);
						let preset_name: string = zui_text_input(zui_handle("boxexport_6", { text: "new_preset" }), tr("Name"));
						if (zui_button(tr("OK")) || ui.is_return_down) {
							BoxExport.new_preset(preset_name);
							BoxExport.fetch_presets();
							BoxExport.preset = null;
							BoxExport.hpreset.position = BoxExport.files.indexOf(preset_name);
							UIBox.hide();
							BoxExport.htab.position = 1; // Presets
							BoxExport.show_textures();
						}
					}
				});
			}

			if (zui_button(tr("Import"))) {
				UIFiles.show("json", false, false, (path: string) => {
					path = path.toLowerCase();
					if (path.endsWith(".json")) {
						let filename: string = path.substr(path.lastIndexOf(Path.sep) + 1);
						let dst_path: string = Path.data() + Path.sep + "export_presets" + Path.sep + filename;
						File.copy(path, dst_path); // Copy to presets folder
						BoxExport.fetch_presets();
						BoxExport.preset = null;
						BoxExport.hpreset.position = BoxExport.files.indexOf(filename.substr(0, filename.length - 5)); // Strip .json
						Console.info(tr("Preset imported:") + " " + filename);
					}
					else Console.error(Strings.error1());
				});
			}

			if (BoxExport.preset == null) {
				BoxExport.parse_preset();
				BoxExport.hpreset.children = null;
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
			for (let i: i32 = 0; i < BoxExport.preset.textures.length; ++i) {
				let t: export_preset_texture_t = BoxExport.preset.textures[i];
				zui_row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
				let htex: zui_handle_t = zui_nest(BoxExport.hpreset, i);
				htex.text = t.name;
				t.name = zui_text_input(htex);

				if (ui.is_hovered && ui.input_released_r) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Delete"))) {
							array_remove(BoxExport.preset.textures, t);
							BoxExport.save_preset();
						}
					}, 1);
				}

				let hr: zui_handle_t = zui_nest(htex, 0);
				hr.position = BoxExport.channels.indexOf(t.channels[0]);
				let hg: zui_handle_t = zui_nest(htex, 1);
				hg.position = BoxExport.channels.indexOf(t.channels[1]);
				let hb: zui_handle_t = zui_nest(htex, 2);
				hb.position = BoxExport.channels.indexOf(t.channels[2]);
				let ha: zui_handle_t = zui_nest(htex, 3);
				ha.position = BoxExport.channels.indexOf(t.channels[3]);

				zui_combo(hr, BoxExport.channels, tr("R"));
				if (hr.changed) t.channels[0] = BoxExport.channels[hr.position];
				zui_combo(hg, BoxExport.channels, tr("G"));
				if (hg.changed) t.channels[1] = BoxExport.channels[hg.position];
				zui_combo(hb, BoxExport.channels, tr("B"));
				if (hb.changed) t.channels[2] = BoxExport.channels[hb.position];
				zui_combo(ha, BoxExport.channels, tr("A"));
				if (ha.changed) t.channels[3] = BoxExport.channels[ha.position];

				let hspace: zui_handle_t = zui_nest(htex, 4);
				hspace.position = BoxExport.color_spaces.indexOf(t.color_space);
				zui_combo(hspace, BoxExport.color_spaces, tr("Color Space"));
				if (hspace.changed) t.color_space = BoxExport.color_spaces[hspace.position];
			}

			if (ui.changed) {
				BoxExport.save_preset();
			}

			zui_row([1 / 8]);
			if (zui_button(tr("Add"))) {
				BoxExport.preset.textures.push({ name: "base", channels: ["base_r", "base_g", "base_b", "1.0"], color_space: "linear" });
				BoxExport.hpreset.children = null;
				BoxExport.save_preset();
			}
		}
	}
	///end

	///if is_paint
	static tab_atlases = (ui: zui_t) => {
		let tab_vertical: bool = Config.raw.touch_ui;
		if (zui_tab(BoxExport.htab, tr("Atlases"), tab_vertical)) {
			if (Project.atlas_objects == null || Project.atlas_objects.length != Project.paint_objects.length) {
				Project.atlas_objects = [];
				Project.atlas_names = [];
				for (let i: i32 = 0; i < Project.paint_objects.length; ++i) {
					Project.atlas_objects.push(0);
					Project.atlas_names.push(tr("Atlas") + " " + (i + 1));
				}
			}
			for (let i: i32 = 0; i < Project.paint_objects.length; ++i) {
				zui_row([1 / 2, 1 / 2]);
				zui_text(Project.paint_objects[i].base.name);
				let hatlas: zui_handle_t = zui_nest(zui_handle("boxexport_7"), i);
				hatlas.position = Project.atlas_objects[i];
				Project.atlas_objects[i] = zui_combo(hatlas, Project.atlas_names, tr("Atlas"));
			}
		}
	}
	///end

	static show_mesh = () => {
		BoxExport.export_mesh_handle.position = Context.raw.export_mesh_index;
		UIBox.show_custom((ui: zui_t) => {
			let htab: zui_handle_t = zui_handle("boxexport_8");
			BoxExport.tab_export_mesh(ui, htab);
		});
	}

	static tab_export_mesh = (ui: zui_t, htab: zui_handle_t) => {
		let tab_vertical: bool = Config.raw.touch_ui;
		if (zui_tab(htab, tr("Export Mesh"), tab_vertical)) {

			zui_row([1 / 2, 1 / 2]);

			Context.raw.export_mesh_format = zui_combo(zui_handle("boxexport_9", { position: Context.raw.export_mesh_format }), ["obj", "arm"], tr("Format"), true);

			let ar: string[] = [tr("All")];
			for (let p of Project.paint_objects) ar.push(p.base.name);
			zui_combo(BoxExport.export_mesh_handle, ar, tr("Meshes"), true);

			let apply_displacement: bool = zui_check(zui_handle("boxexport_10"), tr("Apply Displacement"));

			let tris: i32 = 0;
			let pos: i32 = BoxExport.export_mesh_handle.position;
			let paint_objects: mesh_object_t[] = pos == 0 ? Project.paint_objects : [Project.paint_objects[pos - 1]];
			for (let po of paint_objects) {
				for (let inda of po.data.index_arrays) {
					tris += Math.floor(inda.values.length / 3);
				}
			}
			zui_text(tris + " " + tr("triangles"));

			zui_row([0.5, 0.5]);
			if (zui_button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (zui_button(tr("Export"))) {
				UIBox.hide();
				UIFiles.show(Context.raw.export_mesh_format == mesh_format_t.OBJ ? "obj" : "arm", true, false, (path: string) => {
					///if (krom_android || krom_ios)
					let f: string = sys_title();
					///else
					let f: string = UIFiles.filename;
					///end
					if (f == "") f = tr("untitled");
					let doExport = () => {
						ExportMesh.run(path + Path.sep + f, BoxExport.export_mesh_handle.position == 0 ? null : [Project.paint_objects[BoxExport.export_mesh_handle.position - 1]], apply_displacement);
					}
					///if (krom_android || krom_ios)
					base_notify_on_next_frame(() => {
						Console.toast(tr("Exporting mesh"));
						base_notify_on_next_frame(doExport);
					});
					///else
					doExport();
					///end
				});
			}
		}
	}

	///if (is_paint || is_sculpt)
	static show_material = () => {
		UIBox.show_custom((ui: zui_t) => {
			let htab: zui_handle_t = zui_handle("boxexport_11");
			let tab_vertical: bool = Config.raw.touch_ui;
			if (zui_tab(htab, tr("Export Material"), tab_vertical)) {
				let h1: zui_handle_t = zui_handle("boxexport_12");
				let h2: zui_handle_t = zui_handle("boxexport_13");
				h1.selected = Context.raw.pack_assets_on_export;
				h2.selected = Context.raw.write_icon_on_export;
				Context.raw.pack_assets_on_export = zui_check(h1, tr("Pack Assets"));
				Context.raw.write_icon_on_export = zui_check(h2, tr("Export Icon"));
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Export"))) {
					UIBox.hide();
					UIFiles.show("arm", true, false, (path: string) => {
						let f: string = UIFiles.filename;
						if (f == "") f = tr("untitled");
						app_notify_on_init(() => {
							ExportArm.run_material(path + Path.sep + f);
						});
					});
				}
			}
		});
	}

	static show_brush = () => {
		UIBox.show_custom((ui: zui_t) => {
			let htab: zui_handle_t = zui_handle("boxexport_14");
			let tab_vertical: bool = Config.raw.touch_ui;
			if (zui_tab(htab, tr("Export Brush"), tab_vertical)) {
				let h1: zui_handle_t = zui_handle("boxexport_15");
				let h2: zui_handle_t = zui_handle("boxexport_16");
				h1.selected = Context.raw.pack_assets_on_export;
				h2.selected = Context.raw.write_icon_on_export;
				Context.raw.pack_assets_on_export = zui_check(h1, tr("Pack Assets"));
				Context.raw.write_icon_on_export = zui_check(h2, tr("Export Icon"));
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Export"))) {
					UIBox.hide();
					UIFiles.show("arm", true, false, (path: string) => {
						let f: string = UIFiles.filename;
						if (f == "") f = tr("untitled");
						app_notify_on_init(() => {
							ExportArm.run_brush(path + Path.sep + f);
						});
					});
				}
			}
		});
	}
	///end

	///if (is_paint || is_lab)
	static fetch_presets = () => {
		BoxExport.files = File.read_directory(Path.data() + Path.sep + "export_presets");
		for (let i: i32 = 0; i < BoxExport.files.length; ++i) {
			BoxExport.files[i] = BoxExport.files[i].substr(0, BoxExport.files[i].length - 5); // Strip .json
		}
	}

	static parse_preset = () => {
		let file: string = "export_presets/" + BoxExport.files[BoxExport.hpreset.position] + ".json";
		let blob: ArrayBuffer = data_get_blob(file);
		BoxExport.preset = JSON.parse(sys_buffer_to_string(blob));
		data_delete_blob("export_presets/" + file);
	}

	static new_preset = (name: string) => {
		let template: string =
`{
	"textures": [
		{ "name": "base", "channels": ["base_r", "base_g", "base_b", "1.0"], "color_space": "linear" }
	]
}
`;
		if (!name.endsWith(".json")) name += ".json";
		let path: string = Path.data() + Path.sep + "export_presets" + Path.sep + name;
		krom_file_save_bytes(path, sys_string_to_buffer(template));
	}

	static save_preset = () => {
		let name: string = BoxExport.files[BoxExport.hpreset.position];
		if (name == "generic") return; // generic is const
		let path: string = Path.data() + Path.sep + "export_presets" + Path.sep + name + ".json";
		krom_file_save_bytes(path, sys_string_to_buffer(JSON.stringify(BoxExport.preset)));
	}
	///end
}
