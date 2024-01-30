
class BoxExport {

	static htab = new Handle();
	static files: string[] = null;
	static exportMeshHandle = new Handle();

	///if (is_paint || is_lab)
	static hpreset = new Handle();
	static preset: TExportPreset = null;
	static channels = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_g_directx", "nor_b", "occ", "opac", "rough", "smooth", "emis", "subs", "0.0", "1.0"];
	static colorSpaces = ["linear", "srgb"];
	///end

	///if (is_paint || is_lab)
	static showTextures = () => {
		UIBox.showCustom((ui: Zui) => {

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
		UIBox.showCustom((ui: Zui) => {

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
	static tabExportTextures = (ui: Zui, title: string, bakeMaterial = false) => {
		let tabVertical = Config.raw.touch_ui;
		if (ui.tab(BoxExport.htab, title, tabVertical)) {

			ui.row([0.5, 0.5]);

			///if is_paint
			///if (krom_android || krom_ios)
			ui.combo(Base.resHandle, ["128", "256", "512", "1K", "2K", "4K"], tr("Resolution"), true);
			///else
			ui.combo(Base.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], tr("Resolution"), true);
			///end
			///end

			///if is_lab
			///if (krom_android || krom_ios)
			ui.combo(Base.resHandle, ["2K", "4K"], tr("Resolution"), true);
			///else
			ui.combo(Base.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"), true);
			///end
			///end

			if (Base.resHandle.changed) {
				Base.onLayersResized();
			}

			///if (is_lab || krom_android || krom_ios)
			ui.combo(Base.bitsHandle, ["8bit"], tr("Color"), true);
			///else
			ui.combo(Base.bitsHandle, ["8bit", "16bit", "32bit"], tr("Color"), true);
			///end

			///if is_paint
			if (Base.bitsHandle.changed) {
				App.notifyOnInit(Base.setLayerBits);
			}
			///end

			ui.row([0.5, 0.5]);
			if (Base.bitsHandle.position == TextureBits.Bits8) {
				Context.raw.formatType = ui.combo(Zui.handle("boxexport_0", { position: Context.raw.formatType }), ["png", "jpg"], tr("Format"), true);
			}
			else {
				Context.raw.formatType = ui.combo(Zui.handle("boxexport_1", { position: Context.raw.formatType }), ["exr"], tr("Format"), true);
			}

			ui.enabled = Context.raw.formatType == TextureLdrFormat.FormatJpg && Base.bitsHandle.position == TextureBits.Bits8;
			Context.raw.formatQuality = ui.slider(Zui.handle("boxexport_2", { value: Context.raw.formatQuality }), tr("Quality"), 0.0, 100.0, true, 1);
			ui.enabled = true;

			///if is_paint
			ui.row([0.5, 0.5]);
			ui.enabled = !bakeMaterial;
			let layersExportHandle = Zui.handle("boxexport_3");
			layersExportHandle.position = Context.raw.layersExport;
			Context.raw.layersExport = ui.combo(layersExportHandle, [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")], tr("Layers"), true);
			ui.enabled = true;
			///end

			ui.combo(BoxExport.hpreset, BoxExport.files, tr("Preset"), true);
			if (BoxExport.hpreset.changed) BoxExport.preset = null;

			let layersDestinationHandle = Zui.handle("boxexport_4");
			layersDestinationHandle.position = Context.raw.layersDestination;
			Context.raw.layersDestination = ui.combo(layersDestinationHandle, [tr("Disk"), tr("Packed")], tr("Destination"), true);

			ui.endElement();

			ui.row([0.5, 0.5]);
			if (ui.button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (ui.button(tr("Export"))) {
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
					App.notifyOnInit(_init);
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
							App.notifyOnInit(_init);
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
			if (ui.isHovered) ui.tooltip(tr("Export texture files") + ` (${Config.keymap.file_export_textures})`);
		}
	}

	static tabPresets = (ui: Zui) => {
		let tabVertical = Config.raw.touch_ui;
		if (ui.tab(BoxExport.htab, tr("Presets"), tabVertical)) {
			ui.row([3 / 5, 1 / 5, 1 / 5]);

			ui.combo(BoxExport.hpreset, BoxExport.files, tr("Preset"));
			if (BoxExport.hpreset.changed) BoxExport.preset = null;

			if (ui.button(tr("New"))) {
				UIBox.showCustom((ui: Zui) => {
					let tabVertical = Config.raw.touch_ui;
					if (ui.tab(Zui.handle("boxexport_5"), tr("New Preset"), tabVertical)) {
						ui.row([0.5, 0.5]);
						let presetName = ui.textInput(Zui.handle("boxexport_6", { text: "new_preset" }), tr("Name"));
						if (ui.button(tr("OK")) || ui.isReturnDown) {
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

			if (ui.button(tr("Import"))) {
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
			ui.separator(10, false);
			ui.row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
			ui.text(tr("Texture"));
			ui.text(tr("R"));
			ui.text(tr("G"));
			ui.text(tr("B"));
			ui.text(tr("A"));
			ui.text(tr("Color Space"));
			ui.changed = false;
			for (let i = 0; i < BoxExport.preset.textures.length; ++i) {
				let t = BoxExport.preset.textures[i];
				ui.row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
				let htex = BoxExport.hpreset.nest(i);
				htex.text = t.name;
				t.name = ui.textInput(htex);

				if (ui.isHovered && ui.inputReleasedR) {
					UIMenu.draw((ui: Zui) => {
						if (UIMenu.menuButton(ui, tr("Delete"))) {
							array_remove(BoxExport.preset.textures, t);
							BoxExport.savePreset();
						}
					}, 1);
				}

				let hr = htex.nest(0);
				hr.position = BoxExport.channels.indexOf(t.channels[0]);
				let hg = htex.nest(1);
				hg.position = BoxExport.channels.indexOf(t.channels[1]);
				let hb = htex.nest(2);
				hb.position = BoxExport.channels.indexOf(t.channels[2]);
				let ha = htex.nest(3);
				ha.position = BoxExport.channels.indexOf(t.channels[3]);

				ui.combo(hr, BoxExport.channels, tr("R"));
				if (hr.changed) t.channels[0] = BoxExport.channels[hr.position];
				ui.combo(hg, BoxExport.channels, tr("G"));
				if (hg.changed) t.channels[1] = BoxExport.channels[hg.position];
				ui.combo(hb, BoxExport.channels, tr("B"));
				if (hb.changed) t.channels[2] = BoxExport.channels[hb.position];
				ui.combo(ha, BoxExport.channels, tr("A"));
				if (ha.changed) t.channels[3] = BoxExport.channels[ha.position];

				let hspace = htex.nest(4);
				hspace.position = BoxExport.colorSpaces.indexOf(t.color_space);
				ui.combo(hspace, BoxExport.colorSpaces, tr("Color Space"));
				if (hspace.changed) t.color_space = BoxExport.colorSpaces[hspace.position];
			}

			if (ui.changed) {
				BoxExport.savePreset();
			}

			ui.row([1 / 8]);
			if (ui.button(tr("Add"))) {
				BoxExport.preset.textures.push({ name: "base", channels: ["base_r", "base_g", "base_b", "1.0"], color_space: "linear" });
				BoxExport.hpreset.children = null;
				BoxExport.savePreset();
			}
		}
	}
	///end

	///if is_paint
	static tabAtlases = (ui: Zui) => {
		let tabVertical = Config.raw.touch_ui;
		if (ui.tab(BoxExport.htab, tr("Atlases"), tabVertical)) {
			if (Project.atlasObjects == null || Project.atlasObjects.length != Project.paintObjects.length) {
				Project.atlasObjects = [];
				Project.atlasNames = [];
				for (let i = 0; i < Project.paintObjects.length; ++i) {
					Project.atlasObjects.push(0);
					Project.atlasNames.push(tr("Atlas") + " " + (i + 1));
				}
			}
			for (let i = 0; i < Project.paintObjects.length; ++i) {
				ui.row([1 / 2, 1 / 2]);
				ui.text(Project.paintObjects[i].name);
				let hatlas = Zui.handle("boxexport_7").nest(i);
				hatlas.position = Project.atlasObjects[i];
				Project.atlasObjects[i] = ui.combo(hatlas, Project.atlasNames, tr("Atlas"));
			}
		}
	}
	///end

	static showMesh = () => {
		BoxExport.exportMeshHandle.position = Context.raw.exportMeshIndex;
		UIBox.showCustom((ui: Zui) => {
			let htab = Zui.handle("boxexport_8");
			BoxExport.tabExportMesh(ui, htab);
		});
	}

	static tabExportMesh = (ui: Zui, htab: Handle) => {
		let tabVertical = Config.raw.touch_ui;
		if (ui.tab(htab, tr("Export Mesh"), tabVertical)) {

			ui.row([1 / 2, 1 / 2]);

			Context.raw.exportMeshFormat = ui.combo(Zui.handle("boxexport_9", { position: Context.raw.exportMeshFormat }), ["obj", "arm"], tr("Format"), true);

			let ar = [tr("All")];
			for (let p of Project.paintObjects) ar.push(p.name);
			ui.combo(BoxExport.exportMeshHandle, ar, tr("Meshes"), true);

			let applyDisplacement = ui.check(Zui.handle("boxexport_10"), tr("Apply Displacement"));

			let tris = 0;
			let pos = BoxExport.exportMeshHandle.position;
			let paintObjects = pos == 0 ? Project.paintObjects : [Project.paintObjects[pos - 1]];
			for (let po of paintObjects) {
				for (let inda of po.data.index_arrays) {
					tris += Math.floor(inda.values.length / 3);
				}
			}
			ui.text(tris + " " + tr("triangles"));

			ui.row([0.5, 0.5]);
			if (ui.button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (ui.button(tr("Export"))) {
				UIBox.hide();
				UIFiles.show(Context.raw.exportMeshFormat == MeshFormat.FormatObj ? "obj" : "arm", true, false, (path: string) => {
					///if (krom_android || krom_ios)
					let f = System.title;
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
		UIBox.showCustom((ui: Zui) => {
			let htab = Zui.handle("boxexport_11");
			let tabVertical = Config.raw.touch_ui;
			if (ui.tab(htab, tr("Export Material"), tabVertical)) {
				let h1 = Zui.handle("boxexport_12");
				let h2 = Zui.handle("boxexport_13");
				h1.selected = Context.raw.packAssetsOnExport;
				h2.selected = Context.raw.writeIconOnExport;
				Context.raw.packAssetsOnExport = ui.check(h1, tr("Pack Assets"));
				Context.raw.writeIconOnExport = ui.check(h2, tr("Export Icon"));
				ui.row([0.5, 0.5]);
				if (ui.button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (ui.button(tr("Export"))) {
					UIBox.hide();
					UIFiles.show("arm", true, false, (path: string) => {
						let f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						App.notifyOnInit(() => {
							ExportArm.runMaterial(path + Path.sep + f);
						});
					});
				}
			}
		});
	}

	static showBrush = () => {
		UIBox.showCustom((ui: Zui) => {
			let htab = Zui.handle("boxexport_14");
			let tabVertical = Config.raw.touch_ui;
			if (ui.tab(htab, tr("Export Brush"), tabVertical)) {
				let h1 = Zui.handle("boxexport_15");
				let h2 = Zui.handle("boxexport_16");
				h1.selected = Context.raw.packAssetsOnExport;
				h2.selected = Context.raw.writeIconOnExport;
				Context.raw.packAssetsOnExport = ui.check(h1, tr("Pack Assets"));
				Context.raw.writeIconOnExport = ui.check(h2, tr("Export Icon"));
				ui.row([0.5, 0.5]);
				if (ui.button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (ui.button(tr("Export"))) {
					UIBox.hide();
					UIFiles.show("arm", true, false, (path: string) => {
						let f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						App.notifyOnInit(() => {
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
		Data.getBlob(file, (blob: ArrayBuffer) => {
			BoxExport.preset = JSON.parse(System.bufferToString(blob));
			Data.deleteBlob("export_presets/" + file);
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
		Krom.fileSaveBytes(path, System.stringToBuffer(template));
	}

	static savePreset = () => {
		let name = BoxExport.files[BoxExport.hpreset.position];
		if (name == "generic") return; // generic is const
		let path = Path.data() + Path.sep + "export_presets" + Path.sep + name + ".json";
		Krom.fileSaveBytes(path, System.stringToBuffer(JSON.stringify(BoxExport.preset)));
	}
	///end
}
