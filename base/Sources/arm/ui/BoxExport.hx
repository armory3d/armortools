package arm.ui;

import zui.Zui;
import zui.Id;
import arm.io.ExportMesh;
import arm.sys.Path;
#if (is_paint || is_sculpt)
import arm.io.ExportArm;
#end
#if (is_paint || is_lab)
import haxe.io.Bytes;
import arm.io.ExportTexture;
import arm.sys.File;
#end

class BoxExport {

	public static var htab = new Handle();
	public static var files: Array<String> = null;
	static var exportMeshHandle = new Handle();

	#if (is_paint || is_lab)
	public static var hpreset = new Handle();
	public static var preset: TExportPreset = null;
	static var channels = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_g_directx", "nor_b", "occ", "opac", "rough", "smooth", "emis", "subs", "0.0", "1.0"];
	static var colorSpaces = ["linear", "srgb"];
	#end

	#if (is_paint || is_lab)
	public static function showTextures() {
		UIBox.showCustom(function(ui: Zui) {

			if (files == null) {
				fetchPresets();
				hpreset.position = files.indexOf("generic");
			}
			if (preset == null) {
				parsePreset();
				@:privateAccess hpreset.children = null;
			}

			tabExportTextures(ui, tr("Export Textures"));
			tabPresets(ui);

			#if is_paint
			tabAtlases(ui);
			#if (krom_android || krom_ios)
			tabExportMesh(ui, htab);
			#end
			#end

		}, 540, 310);
	}
	#end

	#if is_paint
	public static function showBakeMaterial() {
		UIBox.showCustom(function(ui: Zui) {

			if (files == null) {
				fetchPresets();
				hpreset.position = files.indexOf("generic");
			}
			if (preset == null) {
				parsePreset();
				@:privateAccess hpreset.children = null;
			}

			tabExportTextures(ui, tr("Bake to Textures"), true);
			tabPresets(ui);

		}, 540, 310);
	}
	#end

	#if (is_paint || is_lab)
	static function tabExportTextures(ui: Zui, title: String, bakeMaterial = false) {
		var tabVertical = Config.raw.touch_ui;
		if (ui.tab(htab, title, tabVertical)) {

			ui.row([0.5, 0.5]);

			#if is_paint
			#if (krom_android || krom_ios)
			ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K"], tr("Resolution"), true);
			#else
			ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], tr("Resolution"), true);
			#end
			#end

			#if is_lab
			#if (krom_android || krom_ios)
			ui.combo(App.resHandle, ["2K", "4K"], tr("Resolution"), true);
			#else
			ui.combo(App.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"), true);
			#end
			#end

			if (App.resHandle.changed) {
				App.onLayersResized();
			}

			#if (is_lab || krom_android || krom_ios)
			ui.combo(App.bitsHandle, ["8bit"], tr("Color"), true);
			#else
			ui.combo(App.bitsHandle, ["8bit", "16bit", "32bit"], tr("Color"), true);
			#end

			#if is_paint
			if (App.bitsHandle.changed) {
				iron.App.notifyOnInit(App.setLayerBits);
			}
			#end

			ui.row([0.5, 0.5]);
			if (App.bitsHandle.position == Bits8) {
				Context.raw.formatType = ui.combo(Id.handle("boxexport_0", { position: Context.raw.formatType }), ["png", "jpg"], tr("Format"), true);
			}
			else {
				Context.raw.formatType = ui.combo(Id.handle("boxexport_1", { position: Context.raw.formatType }), ["exr"], tr("Format"), true);
			}

			ui.enabled = Context.raw.formatType == FormatJpg && App.bitsHandle.position == Bits8;
			Context.raw.formatQuality = ui.slider(Id.handle("boxexport_2", { value: Context.raw.formatQuality }), tr("Quality"), 0.0, 100.0, true, 1);
			ui.enabled = true;

			#if is_paint
			ui.row([0.5, 0.5]);
			ui.enabled = !bakeMaterial;
			var layersExportHandle = Id.handle("boxexport_3");
			layersExportHandle.position = Context.raw.layersExport;
			Context.raw.layersExport = ui.combo(layersExportHandle, [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")], tr("Layers"), true);
			ui.enabled = true;
			#end

			ui.combo(hpreset, files, tr("Preset"), true);
			if (hpreset.changed) preset = null;

			var layersDestinationHandle = Id.handle("boxexport_4");
			layersDestinationHandle.position = Context.raw.layersDestination;
			Context.raw.layersDestination = ui.combo(layersDestinationHandle, [tr("Disk"), tr("Packed")], tr("Destination"), true);

			@:privateAccess ui.endElement();

			ui.row([0.5, 0.5]);
			if (ui.button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (ui.button(tr("Export"))) {
				UIBox.hide();
				if (Context.raw.layersDestination == DestinationPacked) {
					Context.raw.textureExportPath = "/";
					function _init() {
						#if is_paint
						ExportTexture.run(Context.raw.textureExportPath, bakeMaterial);
						#end
						#if is_lab
						ExportTexture.run(Context.raw.textureExportPath);
						#end
					}
					iron.App.notifyOnInit(_init);
				}
				else {
					var filters = App.bitsHandle.position != Bits8 ? "exr" : Context.raw.formatType == FormatPng ? "png" : "jpg";
					UIFiles.show(filters, true, false, function(path: String) {
						Context.raw.textureExportPath = path;
						function doExport() {
							function _init() {
								#if is_paint
								ExportTexture.run(Context.raw.textureExportPath, bakeMaterial);
								#end
								#if is_lab
								ExportTexture.run(Context.raw.textureExportPath);
								#end
							}
							iron.App.notifyOnInit(_init);
						}
						#if (krom_android || krom_ios)
						arm.App.notifyOnNextFrame(function() {
							Console.toast(tr("Exporting textures"));
							arm.App.notifyOnNextFrame(doExport);
						});
						#else
						doExport();
						#end
					});
				}
			}
			if (ui.isHovered) ui.tooltip(tr("Export texture files") + ' (${Config.keymap.file_export_textures})');
		}
	}

	static function tabPresets(ui: Zui) {
		var tabVertical = Config.raw.touch_ui;
		if (ui.tab(htab, tr("Presets"), tabVertical)) {
			ui.row([3 / 5, 1 / 5, 1 / 5]);

			ui.combo(hpreset, files, tr("Preset"));
			if (hpreset.changed) preset = null;

			if (ui.button(tr("New"))) {
				UIBox.showCustom(function(ui: Zui) {
					var tabVertical = Config.raw.touch_ui;
					if (ui.tab(Id.handle("boxexport_5"), tr("New Preset"), tabVertical)) {
						ui.row([0.5, 0.5]);
						var presetName = ui.textInput(Id.handle("boxexport_6", { text: "new_preset" }), tr("Name"));
						if (ui.button(tr("OK")) || ui.isReturnDown) {
							newPreset(presetName);
							fetchPresets();
							preset = null;
							hpreset.position = files.indexOf(presetName);
							UIBox.hide();
							BoxExport.htab.position = 1; // Presets
							BoxExport.showTextures();
						}
					}
				});
			}

			if (ui.button(tr("Import"))) {
				UIFiles.show("json", false, false, function(path: String) {
					path = path.toLowerCase();
					if (path.endsWith(".json")) {
						var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
						var dstPath = Path.data() + Path.sep + "export_presets" + Path.sep + filename;
						File.copy(path, dstPath); // Copy to presets folder
						fetchPresets();
						preset = null;
						hpreset.position = files.indexOf(filename.substr(0, filename.length - 5)); // Strip .json
						Console.info(tr("Preset imported:") + " " + filename);
					}
					else Console.error(Strings.error1());
				});
			}

			if (preset == null) {
				parsePreset();
				@:privateAccess hpreset.children = null;
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
			for (i in 0...preset.textures.length) {
				var t = preset.textures[i];
				ui.row([1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6, 1 / 6]);
				var htex = hpreset.nest(i);
				htex.text = t.name;
				t.name = ui.textInput(htex);

				if (ui.isHovered && ui.inputReleasedR) {
					UIMenu.draw(function(ui: Zui) {
						if (UIMenu.menuButton(ui, tr("Delete"))) {
							preset.textures.remove(t);
							savePreset();
						}
					}, 1);
				}

				var hr = htex.nest(0);
				hr.position = channels.indexOf(t.channels[0]);
				var hg = htex.nest(1);
				hg.position = channels.indexOf(t.channels[1]);
				var hb = htex.nest(2);
				hb.position = channels.indexOf(t.channels[2]);
				var ha = htex.nest(3);
				ha.position = channels.indexOf(t.channels[3]);

				ui.combo(hr, channels, tr("R"));
				if (hr.changed) t.channels[0] = channels[hr.position];
				ui.combo(hg, channels, tr("G"));
				if (hg.changed) t.channels[1] = channels[hg.position];
				ui.combo(hb, channels, tr("B"));
				if (hb.changed) t.channels[2] = channels[hb.position];
				ui.combo(ha, channels, tr("A"));
				if (ha.changed) t.channels[3] = channels[ha.position];

				var hspace = htex.nest(4);
				hspace.position = colorSpaces.indexOf(t.color_space);
				ui.combo(hspace, colorSpaces, tr("Color Space"));
				if (hspace.changed) t.color_space = colorSpaces[hspace.position];
			}

			if (ui.changed) {
				savePreset();
			}

			ui.row([1 / 8]);
			if (ui.button(tr("Add"))) {
				preset.textures.push({ name: "base", channels: ["base_r", "base_g", "base_b", "1.0"], color_space: "linear" });
				@:privateAccess hpreset.children = null;
				savePreset();
			}
		}
	}
	#end

	#if is_paint
	static function tabAtlases(ui: Zui) {
		var tabVertical = Config.raw.touch_ui;
		if (ui.tab(htab, tr("Atlases"), tabVertical)) {
			if (Project.atlasObjects == null || Project.atlasObjects.length != Project.paintObjects.length) {
				Project.atlasObjects = [];
				Project.atlasNames = [];
				for (i in 0...Project.paintObjects.length) {
					Project.atlasObjects.push(0);
					Project.atlasNames.push(tr("Atlas") + " " + (i + 1));
				}
			}
			for (i in 0...Project.paintObjects.length) {
				ui.row([1 / 2, 1 / 2]);
				ui.text(Project.paintObjects[i].name);
				var hatlas = Id.handle("boxexport_7").nest(i);
				hatlas.position = Project.atlasObjects[i];
				Project.atlasObjects[i] = ui.combo(hatlas, Project.atlasNames, tr("Atlas"));
			}
		}
	}
	#end

	public static function showMesh() {
		exportMeshHandle.position = Context.raw.exportMeshIndex;
		UIBox.showCustom(function(ui: Zui) {
			var htab = Id.handle("boxexport_8");
			tabExportMesh(ui, htab);
		});
	}

	static function tabExportMesh(ui: Zui, htab: zui.Zui.Handle) {
		var tabVertical = Config.raw.touch_ui;
		if (ui.tab(htab, tr("Export Mesh"), tabVertical)) {

			ui.row([1 / 2, 1 / 2]);

			Context.raw.exportMeshFormat = ui.combo(Id.handle("boxexport_9", { position: Context.raw.exportMeshFormat }), ["obj", "arm"], tr("Format"), true);

			var ar = [tr("All")];
			for (p in Project.paintObjects) ar.push(p.name);
			ui.combo(exportMeshHandle, ar, tr("Meshes"), true);

			var applyDisplacement = ui.check(Id.handle("boxexport_10"), tr("Apply Displacement"));

			var tris = 0;
			var pos = exportMeshHandle.position;
			var paintObjects = pos == 0 ? Project.paintObjects : [Project.paintObjects[pos - 1]];
			for (po in paintObjects) {
				for (inda in po.data.raw.index_arrays) {
					tris += Std.int(inda.values.length / 3);
				}
			}
			ui.text(tris + " " + tr("triangles"));

			ui.row([0.5, 0.5]);
			if (ui.button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (ui.button(tr("Export"))) {
				UIBox.hide();
				UIFiles.show(Context.raw.exportMeshFormat == FormatObj ? "obj" : "arm", true, false, function(path: String) {
					#if (krom_android || krom_ios)
					var f = kha.Window.get(0).title;
					#else
					var f = UIFiles.filename;
					#end
					if (f == "") f = tr("untitled");
					function doExport() {
						ExportMesh.run(path + Path.sep + f, exportMeshHandle.position == 0 ? null : [Project.paintObjects[exportMeshHandle.position - 1]], applyDisplacement);
					}
					#if (krom_android || krom_ios)
					arm.App.notifyOnNextFrame(function() {
						Console.toast(tr("Exporting mesh"));
						arm.App.notifyOnNextFrame(doExport);
					});
					#else
					doExport();
					#end
				});
			}
		}
	}

	#if (is_paint || is_sculpt)
	public static function showMaterial() {
		UIBox.showCustom(function(ui: Zui) {
			var htab = Id.handle("boxexport_11");
			var tabVertical = Config.raw.touch_ui;
			if (ui.tab(htab, tr("Export Material"), tabVertical)) {
				var h1 = Id.handle("boxexport_12");
				var h2 = Id.handle("boxexport_13");
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
					UIFiles.show("arm", true, false, function(path: String) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						iron.App.notifyOnInit(function() {
							ExportArm.runMaterial(path + Path.sep + f);
						});
					});
				}
			}
		});
	}

	public static function showBrush() {
		UIBox.showCustom(function(ui: Zui) {
			var htab = Id.handle("boxexport_14");
			var tabVertical = Config.raw.touch_ui;
			if (ui.tab(htab, tr("Export Brush"), tabVertical)) {
				var h1 = Id.handle("boxexport_15");
				var h2 = Id.handle("boxexport_16");
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
					UIFiles.show("arm", true, false, function(path: String) {
						var f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						iron.App.notifyOnInit(function() {
							ExportArm.runBrush(path + Path.sep + f);
						});
					});
				}
			}
		});
	}
	#end

	#if (is_paint || is_lab)
	static function fetchPresets() {
		files = File.readDirectory(Path.data() + Path.sep + "export_presets");
		for (i in 0...files.length) {
			files[i] = files[i].substr(0, files[i].length - 5); // Strip .json
		}
	}

	static function parsePreset() {
		var file = "export_presets/" + files[hpreset.position] + ".json";
		iron.data.Data.getBlob(file, function(blob: kha.Blob) {
			preset = haxe.Json.parse(blob.toString());
			iron.data.Data.deleteBlob("export_presets/" + file);
		});
	}

	static function newPreset(name: String) {
		var template =
'{
	"textures": [
		{ "name": "base", "channels": ["base_r", "base_g", "base_b", "1.0"], "color_space": "linear" }
	]
}
';
		if (!name.endsWith(".json")) name += ".json";
		var path = Path.data() + Path.sep + "export_presets" + Path.sep + name;
		Krom.fileSaveBytes(path, Bytes.ofString(template).getData());
	}

	static function savePreset() {
		var name = files[hpreset.position];
		if (name == "generic") return; // generic is const
		var path = Path.data() + Path.sep + "export_presets" + Path.sep + name + ".json";
		Krom.fileSaveBytes(path, Bytes.ofString(haxe.Json.stringify(preset)).getData());
	}
	#end
}
