package arm.ui;

import haxe.io.Bytes;
import zui.Zui;
import zui.Id;
import arm.util.UVUtil;
import arm.io.ExportMesh;
import arm.io.ExportTexture;
import arm.io.ExportArm;
import arm.sys.Path;
import arm.sys.File;
import arm.Enums;

class BoxExport {

	public static var htab = Id.handle();
	public static var hpreset = Id.handle();
	public static var files: Array<String> = null;
	public static var preset: TExportPreset = null;
	static var channels = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_g_directx", "nor_b", "occ", "opac", "rough", "smooth", "emis", "subs", "0.0", "1.0"];
	static var colorSpaces = ["linear", "srgb"];
	static var exportMeshHandle = Id.handle();

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
			tabAtlases(ui);
			#if (krom_android || krom_ios)
			tabExportMesh(ui, htab);
			#end

		}, 540, 310);
	}

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

	static function tabExportTextures(ui: Zui, title: String, bakeMaterial = false) {
		if (ui.tab(htab, title)) {
			ui.row([0.5, 0.5]);
			#if (krom_android || krom_ios)
			ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K"], tr("Resolution"), true);
			#else
			ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], tr("Resolution"), true);
			#end
			if (App.resHandle.changed) {
				Layers.onLayersResized();
			}
			#if (krom_android || krom_ios)
			ui.combo(App.bitsHandle, ["8bit"], tr("Color"), true);
			#else
			ui.combo(App.bitsHandle, ["8bit", "16bit", "32bit"], tr("Color"), true);
			#end
			if (App.bitsHandle.changed) {
				iron.App.notifyOnInit(Layers.setLayerBits);
			}

			ui.row([0.5, 0.5]);
			if (App.bitsHandle.position == Bits8) {
				Context.formatType = ui.combo(Id.handle({ position: Context.formatType }), ["png", "jpg"], tr("Format"), true);
			}
			else {
				Context.formatType = ui.combo(Id.handle({ position: Context.formatType }), ["exr"], tr("Format"), true);
			}
			ui.enabled = Context.formatType == FormatJpg && App.bitsHandle.position == Bits8;
			Context.formatQuality = ui.slider(Id.handle({ value: Context.formatQuality }), tr("Quality"), 0.0, 100.0, true, 1);
			ui.enabled = true;
			ui.row([0.5, 0.5]);
			ui.enabled = !bakeMaterial;
			var layersExportHandle = Id.handle();
			layersExportHandle.position = Context.layersExport;
			Context.layersExport = ui.combo(layersExportHandle, [tr("Visible"), tr("Selected"), tr("Per Object"), tr("Per Udim Tile")], tr("Layers"), true);
			ui.enabled = true;
			ui.combo(hpreset, files, tr("Preset"), true);
			if (hpreset.changed) preset = null;

			var layersDestinationHandle = Id.handle();
			layersDestinationHandle.position = Context.layersDestination;
			Context.layersDestination = ui.combo(layersDestinationHandle, [tr("Disk"), tr("Packed")], tr("Destination"), true);

			@:privateAccess ui.endElement();

			ui.row([0.5, 0.5]);
			if (ui.button(tr("Cancel"))) {
				UIBox.show = false;
			}
			if (ui.button(tr("Export"))) {
				UIBox.show = false;
				if (Context.layersDestination == DestinationPacked) {
					Context.textureExportPath = "/";
					function _init() {
						ExportTexture.run(Context.textureExportPath, bakeMaterial);
					}
					iron.App.notifyOnInit(_init);
				}
				else {
					var filters = App.bitsHandle.position != Bits8 ? "exr" : Context.formatType == FormatPng ? "png" : "jpg";
					UIFiles.show(filters, true, false, function(path: String) {
						Context.textureExportPath = path;
						function doExport() {
							function _init() {
								ExportTexture.run(Context.textureExportPath, bakeMaterial);
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
		if (ui.tab(htab, tr("Presets"))) {
			ui.row([3 / 5, 1 / 5, 1 / 5]);

			ui.combo(hpreset, files, tr("Preset"));
			if (hpreset.changed) preset = null;

			if (ui.button(tr("New"))) {
				UIBox.showCustom(function(ui: Zui) {
					if (ui.tab(Id.handle(), tr("New Preset"))) {
						ui.row([0.5, 0.5]);
						var presetName = ui.textInput(Id.handle({ text: "new_preset" }), tr("Name"));
						if (ui.button(tr("OK")) || ui.isReturnDown) {
							newPreset(presetName);
							fetchPresets();
							preset = null;
							hpreset.position = files.indexOf(presetName);
							UIBox.show = false;
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
						ui.text(t.name, Right, ui.t.HIGHLIGHT_COL);
						if (ui.button(tr("Delete"), Left)) {
							preset.textures.remove(t);
							savePreset();
						}
					}, 2);
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

	static function tabAtlases(ui: Zui) {
		if (ui.tab(htab, tr("Atlases"))) {
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
				var hatlas = Id.handle().nest(i);
				hatlas.position = Project.atlasObjects[i];
				Project.atlasObjects[i] = ui.combo(hatlas, Project.atlasNames, tr("Atlas"));
			}
		}
	}

	public static function showMesh() {
		exportMeshHandle.position = Context.exportMeshIndex;
		UIBox.showCustom(function(ui: Zui) {
			var htab = Id.handle();
			tabExportMesh(ui, htab);
		});
	}

	static function tabExportMesh(ui: Zui, htab: zui.Zui.Handle) {
		if (ui.tab(htab, tr("Export Mesh"))) {

			ui.row([1 / 2, 1 / 2]);

			Context.exportMeshFormat = ui.combo(Id.handle({ position: Context.exportMeshFormat }), ["obj", "arm"], tr("Format"), true);

			var ar = [tr("All")];
			for (p in Project.paintObjects) ar.push(p.name);
			ui.combo(exportMeshHandle, ar, tr("Meshes"), true);

			var applyDisplacement = ui.check(Id.handle(), tr("Apply Displacement"));

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
				UIBox.show = false;
			}
			if (ui.button(tr("Export"))) {
				UIBox.show = false;
				UIFiles.show(Context.exportMeshFormat == FormatObj ? "obj" : "arm", true, false, function(path: String) {
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

	public static function showMaterial() {
		UIBox.showCustom(function(ui: Zui) {
			var htab = Id.handle();
			if (ui.tab(htab, tr("Export Material"))) {
				var h1 = Id.handle();
				var h2 = Id.handle();
				h1.selected = Context.packAssetsOnExport;
				h2.selected = Context.writeIconOnExport;
				Context.packAssetsOnExport = ui.check(h1, tr("Pack Assets"));
				Context.writeIconOnExport = ui.check(h2, tr("Export Icon"));
				ui.row([0.5, 0.5]);
				if (ui.button(tr("Cancel"))) {
					UIBox.show = false;
				}
				if (ui.button(tr("Export"))) {
					UIBox.show = false;
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
			var htab = Id.handle();
			if (ui.tab(htab, tr("Export Brush"))) {
				var h1 = Id.handle();
				var h2 = Id.handle();
				h1.selected = Context.packAssetsOnExport;
				h2.selected = Context.writeIconOnExport;
				Context.packAssetsOnExport = ui.check(h1, tr("Pack Assets"));
				Context.writeIconOnExport = ui.check(h2, tr("Export Icon"));
				ui.row([0.5, 0.5]);
				if (ui.button(tr("Cancel"))) {
					UIBox.show = false;
				}
				if (ui.button(tr("Export"))) {
					UIBox.show = false;
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
}
