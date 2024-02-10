
class BoxExport {

	static htab = zui_handle_create();
	static files: string[] = null;
	static exportMeshHandle = zui_handle_create();

	///if (is_paint || is_lab)
	static hpreset = zui_handle_create();
	static preset: TExportPreset = null;
	static channels = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_g_directx", "nor_b", "occ", "opac", "rough", "smooth", "emis", "subs", "0.0", "1.0"];
	static colorSpaces = ["linear", "srgb"];
	///end

	///if (is_paint || is_lab)
	static showTextures = () => {
		UIBox.showCustom((ui: zui_t) => {

			if (BoxExport.files == null) {
				BoxExport.fetchPresets();
				BoxExport.hpreset.position = BoxExport.files.indexOf("generic");
			}
			if (BoxExport.preset == null) {
				BoxExport.parsePreset();
				BoxExport.hpreset.children = null;
			}

			BoxExport.tabExportTextures(ui, tr("Export Textures"));
			BoxExport.tabPresets(ui);

			///if is_paint
			BoxExport.tabAtlases(ui);
			///if (krom_android || krom_ios)
			BoxExport.tabExportMesh(ui, BoxExport.htab);
			///end
			///end

		}, 540, 310);
	}
	///end

	///if is_paint
	static showBakeMaterial = () => {
		UIBox.showCustom((ui: zui_t) => {

			if (BoxExport.files == null) {
				BoxExport.fetchPresets();
				BoxExport.hpreset.position = BoxExport.files.indexOf("generic");
			}
			if (BoxExport.preset == null) {
				BoxExport.parsePreset();
				BoxExport.hpreset.children = null;
			}

			BoxExport.tabExportTextures(ui, tr("Bake to Textures"), true);
			BoxExport.tabPresets(ui);

		}, 540, 310);
	}
	///end

	///if (is_paint || is_lab)
	static tabExportTextures = (ui: zui_t, title: string, bakeMaterial = false) => {
		let tabVertical = Config.raw.touch_ui;
		if (zui_tab(BoxExport.htab, title, tabVertical)) {

			zui_row([0.5, 0.5]);

			///if is_paint
			///if (krom_android || krom_ios)
			zui_combo(Base.resHandle, ["128", "256", "512", "1K", "2K", "4K"], tr("Resolution"), true);
			///else
			zui_combo(Base.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], tr("Resolution"), true);
			///end
			///end

			///if is_lab
			///if (krom_android || krom_ios)
			zui_combo(Base.resHandle, ["2K", "4K"], tr("Resolution"), true);
			///else
			zui_combo(Base.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"), true);
			///end
			///end

			if (Base.resHandle.changed) {
				Base.onLayersResized();
			}

			///if (is_lab || krom_android || krom_ios)
			zui_combo(Base.bitsHandle, ["8bit"], tr("Color"), true);
			///else
			zui_combo(Base.bitsHandle, ["8bit", "16bit", "32bit"], tr("Color"), true);
			///end

			///if is_paint
			if (Base.bitsHandle.changed) {
				app_notify_on_init(Base.setLayerBits);
			}
			///end

			zui_row([0.5, 0.5]);
			if (Base.bitsHandle.position == TextureBits.Bits8) {
				Context.raw.formatType = zui_combo(zui_handle("boxexport_0", { position: Context.raw.formatType }), ["png", "jpg"], tr("Format"), true);
			}
			else {
				Context.raw.formatType = zui_combo(zui_handle("boxexport_1", { position: Context.raw.formatType }), ["exr"], tr("Format"), true);
			}

			ui.enabled = Context.raw.formatType == TextureLdrFormat.FormatJpg && Base.bitsHandle.position == TextureBits.Bits8;
			Context.raw.formatQuality = zui_slider(zui_handle("boxexport_2", { value: Context.raw.formatQuality }), tr("Quality"), 0.0, 100.0, true, 1);
			ui.enabled = true;

			///if is_paint
			zui_row([0.5, 0.5]);
			ui.enabled = !bakeMaterial;
			let layersExportHandle = zui_handle("boxexport_3");
			layersExportHandle.position = Context.raw.layersExport;
			Context.raw.layersExport = zui_combo(layersExportHandle, [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")], tr("Layers"), true);
			ui.enabled = true;
			///end

			zui_combo(BoxExport.hpreset, BoxExport.files, tr("Preset"), true);
			if (BoxExport.hpreset.changed) BoxExport.preset = null;

			let layersDestinationHandle = zui_handle("boxexport_4");
			layersDestinationHandle.position = Context.raw.layersDestination;
			Context.raw.layersDestination = zui_combo(layersDestinationHandle, [tr("Disk"), tr("Packed")], tr("Destination"), true);

			zui_end_element();

			zui_row([0.5, 0.5]);
			if (zui_button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (zui_button(tr("Export"))) {
				UIBox.hide();
				if (Context.raw.layersDestination == ExportDestination.DestinationPacked) {
					Context.raw.textureExportPath = "/";
					let _init = () => {
						///if is_paint
						ExportTexture.run(Context.raw.textureExportPath, bakeMaterial);
						///end
						///if is_lab
						ExportTexture.run(Context.raw.textureExportPath);
						///end
					}
					app_notify_on_init(_init);
				}
				else {
					let filters = Base.bitsHandle.position != TextureBits.Bits8 ? "exr" : Context.raw.formatType == TextureLdrFormat.FormatPng ? "png" : "jpg";
					UIFiles.show(filters, true, false, (path: string) => {
						Context.raw.textureExportPath = path;
						let doExport = () => {
							let _init = () => {
								///if is_paint
								ExportTexture.run(Context.raw.textureExportPath, bakeMaterial);
								///end
								///if is_lab
								ExportTexture.run(Context.raw.textureExportPath);
								///end
							}
							app_notify_on_init(_init);
						}
						///if (krom_android || krom_ios)
						Base.notifyOnNextFrame(() => {
							Console.toast(tr("Exporting textures"));
							Base.notifyOnNextFrame(doExport);
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

	static tabPresets = (ui: zui_t) => {
		let tabVertical = Config.raw.touch_ui;
		if (zui_tab(BoxExport.htab, tr("Presets"), tabVertical)) {
			zui_row([3 / 5, 1 / 5, 1 / 5]);

			zui_combo(BoxExport.hpreset, BoxExport.files, tr("Preset"));
			if (BoxExport.hpreset.changed) BoxExport.preset = null;

			if (zui_button(tr("New"))) {
				UIBox.showCustom((ui: zui_t) => {
					let tabVertical = Config.raw.touch_ui;
					if (zui_tab(zui_handle("boxexport_5"), tr("New Preset"), tabVertical)) {
						zui_row([0.5, 0.5]);
						let presetName = zui_text_input(zui_handle("boxexport_6", { text: "new_preset" }), tr("Name"));
						if (zui_button(tr("OK")) || ui.is_return_down) {
							BoxExport.newPreset(presetName);
							BoxExport.fetchPresets();
							BoxExport.preset = null;
							BoxExport.hpreset.position = BoxExport.files.indexOf(presetName);
							UIBox.hide();
							BoxExport.htab.position = 1; // Presets
							BoxExport.showTextures();
						}
					}
				});
			}

			if (zui_button(tr("Import"))) {
				UIFiles.show("json", false, false, (path: string) => {
					path = path.toLowerCase();
					if (path.endsWith(".json")) {
						let filename = path.substr(path.lastIndexOf(Path.sep) + 1);
						let dstPath = Path.data() + Path.sep + "export_presets" + Path.sep + filename;
						File.copy(path, dstPath); // Copy to presets folder
						BoxExport.fetchPresets();
						BoxExport.preset = null;
						BoxExport.hpreset.position = BoxExport.files.indexOf(filename.substr(0, filename.length - 5)); // Strip .json
						Console.info(tr("Preset imported:") + " " + filename);
					}
					else Console.error(Strings.error1());
				});
			}

			if (BoxExport.preset == null) {
				BoxExport.parsePreset();
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
			for (let i = 0; i < BoxExport.preset.textures.length; ++i) {
				let t = BoxExport.preset.textures[i];
				zui_row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
				let htex = zui_nest(BoxExport.hpreset, i);
				htex.text = t.name;
				t.name = zui_text_input(htex);

				if (ui.is_hovered && ui.input_released_r) {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menuButton(ui, tr("Delete"))) {
							array_remove(BoxExport.preset.textures, t);
							BoxExport.savePreset();
						}
					}, 1);
				}

				let hr = zui_nest(htex, 0);
				hr.position = BoxExport.channels.indexOf(t.channels[0]);
				let hg = zui_nest(htex, 1);
				hg.position = BoxExport.channels.indexOf(t.channels[1]);
				let hb = zui_nest(htex, 2);
				hb.position = BoxExport.channels.indexOf(t.channels[2]);
				let ha = zui_nest(htex, 3);
				ha.position = BoxExport.channels.indexOf(t.channels[3]);

				zui_combo(hr, BoxExport.channels, tr("R"));
				if (hr.changed) t.channels[0] = BoxExport.channels[hr.position];
				zui_combo(hg, BoxExport.channels, tr("G"));
				if (hg.changed) t.channels[1] = BoxExport.channels[hg.position];
				zui_combo(hb, BoxExport.channels, tr("B"));
				if (hb.changed) t.channels[2] = BoxExport.channels[hb.position];
				zui_combo(ha, BoxExport.channels, tr("A"));
				if (ha.changed) t.channels[3] = BoxExport.channels[ha.position];

				let hspace = zui_nest(htex, 4);
				hspace.position = BoxExport.colorSpaces.indexOf(t.color_space);
				zui_combo(hspace, BoxExport.colorSpaces, tr("Color Space"));
				if (hspace.changed) t.color_space = BoxExport.colorSpaces[hspace.position];
			}

			if (ui.changed) {
				BoxExport.savePreset();
			}

			zui_row([1 / 8]);
			if (zui_button(tr("Add"))) {
				BoxExport.preset.textures.push({ name: "base", channels: ["base_r", "base_g", "base_b", "1.0"], color_space: "linear" });
				BoxExport.hpreset.children = null;
				BoxExport.savePreset();
			}
		}
	}
	///end

	///if is_paint
	static tabAtlases = (ui: zui_t) => {
		let tabVertical = Config.raw.touch_ui;
		if (zui_tab(BoxExport.htab, tr("Atlases"), tabVertical)) {
			if (Project.atlasObjects == null || Project.atlasObjects.length != Project.paintObjects.length) {
				Project.atlasObjects = [];
				Project.atlasNames = [];
				for (let i = 0; i < Project.paintObjects.length; ++i) {
					Project.atlasObjects.push(0);
					Project.atlasNames.push(tr("Atlas") + " " + (i + 1));
				}
			}
			for (let i = 0; i < Project.paintObjects.length; ++i) {
				zui_row([1 / 2, 1 / 2]);
				zui_text(Project.paintObjects[i].base.name);
				let hatlas = zui_nest(zui_handle("boxexport_7"), i);
				hatlas.position = Project.atlasObjects[i];
				Project.atlasObjects[i] = zui_combo(hatlas, Project.atlasNames, tr("Atlas"));
			}
		}
	}
	///end

	static showMesh = () => {
		BoxExport.exportMeshHandle.position = Context.raw.exportMeshIndex;
		UIBox.showCustom((ui: zui_t) => {
			let htab = zui_handle("boxexport_8");
			BoxExport.tabExportMesh(ui, htab);
		});
	}

	static tabExportMesh = (ui: zui_t, htab: zui_handle_t) => {
		let tabVertical = Config.raw.touch_ui;
		if (zui_tab(htab, tr("Export Mesh"), tabVertical)) {

			zui_row([1 / 2, 1 / 2]);

			Context.raw.exportMeshFormat = zui_combo(zui_handle("boxexport_9", { position: Context.raw.exportMeshFormat }), ["obj", "arm"], tr("Format"), true);

			let ar = [tr("All")];
			for (let p of Project.paintObjects) ar.push(p.base.name);
			zui_combo(BoxExport.exportMeshHandle, ar, tr("Meshes"), true);

			let applyDisplacement = zui_check(zui_handle("boxexport_10"), tr("Apply Displacement"));

			let tris = 0;
			let pos = BoxExport.exportMeshHandle.position;
			let paintObjects = pos == 0 ? Project.paintObjects : [Project.paintObjects[pos - 1]];
			for (let po of paintObjects) {
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
				UIFiles.show(Context.raw.exportMeshFormat == MeshFormat.FormatObj ? "obj" : "arm", true, false, (path: string) => {
					///if (krom_android || krom_ios)
					let f = sys_title();
					///else
					let f = UIFiles.filename;
					///end
					if (f == "") f = tr("untitled");
					let doExport = () => {
						ExportMesh.run(path + Path.sep + f, BoxExport.exportMeshHandle.position == 0 ? null : [Project.paintObjects[BoxExport.exportMeshHandle.position - 1]], applyDisplacement);
					}
					///if (krom_android || krom_ios)
					Base.notifyOnNextFrame(() => {
						Console.toast(tr("Exporting mesh"));
						Base.notifyOnNextFrame(doExport);
					});
					///else
					doExport();
					///end
				});
			}
		}
	}

	///if (is_paint || is_sculpt)
	static showMaterial = () => {
		UIBox.showCustom((ui: zui_t) => {
			let htab = zui_handle("boxexport_11");
			let tabVertical = Config.raw.touch_ui;
			if (zui_tab(htab, tr("Export Material"), tabVertical)) {
				let h1 = zui_handle("boxexport_12");
				let h2 = zui_handle("boxexport_13");
				h1.selected = Context.raw.packAssetsOnExport;
				h2.selected = Context.raw.writeIconOnExport;
				Context.raw.packAssetsOnExport = zui_check(h1, tr("Pack Assets"));
				Context.raw.writeIconOnExport = zui_check(h2, tr("Export Icon"));
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Export"))) {
					UIBox.hide();
					UIFiles.show("arm", true, false, (path: string) => {
						let f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						app_notify_on_init(() => {
							ExportArm.runMaterial(path + Path.sep + f);
						});
					});
				}
			}
		});
	}

	static showBrush = () => {
		UIBox.showCustom((ui: zui_t) => {
			let htab = zui_handle("boxexport_14");
			let tabVertical = Config.raw.touch_ui;
			if (zui_tab(htab, tr("Export Brush"), tabVertical)) {
				let h1 = zui_handle("boxexport_15");
				let h2 = zui_handle("boxexport_16");
				h1.selected = Context.raw.packAssetsOnExport;
				h2.selected = Context.raw.writeIconOnExport;
				Context.raw.packAssetsOnExport = zui_check(h1, tr("Pack Assets"));
				Context.raw.writeIconOnExport = zui_check(h2, tr("Export Icon"));
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Export"))) {
					UIBox.hide();
					UIFiles.show("arm", true, false, (path: string) => {
						let f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						app_notify_on_init(() => {
							ExportArm.runBrush(path + Path.sep + f);
						});
					});
				}
			}
		});
	}
	///end

	///if (is_paint || is_lab)
	static fetchPresets = () => {
		BoxExport.files = File.readDirectory(Path.data() + Path.sep + "export_presets");
		for (let i = 0; i < BoxExport.files.length; ++i) {
			BoxExport.files[i] = BoxExport.files[i].substr(0, BoxExport.files[i].length - 5); // Strip .json
		}
	}

	static parsePreset = () => {
		let file = "export_presets/" + BoxExport.files[BoxExport.hpreset.position] + ".json";
		data_get_blob(file, (blob: ArrayBuffer) => {
			BoxExport.preset = JSON.parse(sys_buffer_to_string(blob));
			data_delete_blob("export_presets/" + file);
		});
	}

	static newPreset = (name: string) => {
		let template =
`{
	"textures": [
		{ "name": "base", "channels": ["base_r", "base_g", "base_b", "1.0"], "color_space": "linear" }
	]
}
`;
		if (!name.endsWith(".json")) name += ".json";
		let path = Path.data() + Path.sep + "export_presets" + Path.sep + name;
		krom_file_save_bytes(path, sys_string_to_buffer(template));
	}

	static savePreset = () => {
		let name = BoxExport.files[BoxExport.hpreset.position];
		if (name == "generic") return; // generic is const
		let path = Path.data() + Path.sep + "export_presets" + Path.sep + name + ".json";
		krom_file_save_bytes(path, sys_string_to_buffer(JSON.stringify(BoxExport.preset)));
	}
	///end
}
