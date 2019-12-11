package arm.ui;

import haxe.io.Bytes;
import zui.Zui;
import zui.Id;
import arm.util.UVUtil;
import arm.io.ExportMesh;
import arm.io.ExportTexture;
import arm.sys.Path;
import arm.sys.File;
import arm.Tool;
using StringTools;

class BoxExport {

	public static var htab = Id.handle();
	public static var hpreset = Id.handle();
	public static var files: Array<String> = null;
	public static var preset: TExportPreset = null;
	static var channels = ["base_r", "base_g", "base_b", "height", "metal", "nor_r", "nor_g", "nor_b", "occ", "opac", "rough", "smooth", "0.0", "1.0"];

	public static function showTextures() {
		UIBox.showCustom(function(ui: Zui) {

			if (files == null) fetchPresets();
			if (preset == null) {
				parsePreset();
				@:privateAccess hpreset.children = null;
			}

			if (ui.tab(htab, "Export Textures")) {
				ui.row([0.5, 0.5]);
				ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], "Res", true);
				if (App.resHandle.changed) {
					iron.App.notifyOnRender(Layers.resizeLayers);
					UVUtil.uvmap = null;
					UVUtil.uvmapCached = false;
					UVUtil.trianglemap = null;
					UVUtil.trianglemapCached = false;
					#if kha_direct3d12
					arm.render.RenderPathRaytrace.ready = false;
					#end
				}
				ui.combo(App.bitsHandle, ["8bit", "16bit", "32bit"], "Color", true);
				if (App.bitsHandle.changed) {
					iron.App.notifyOnRender(Layers.setLayerBits);
				}

				ui.row([0.5, 0.5]);
				if (App.bitsHandle.position == Bits8) {
					UITrait.inst.formatType = ui.combo(Id.handle({position: UITrait.inst.formatType}), ["png", "jpg"], "Format", true);
				}
				else {
					UITrait.inst.formatType = ui.combo(Id.handle({position: UITrait.inst.formatType}), ["exr"], "Format", true);
				}
				ui.enabled = UITrait.inst.formatType == FormatJpg && App.bitsHandle.position == Bits8;
				UITrait.inst.formatQuality = ui.slider(Id.handle({value: UITrait.inst.formatQuality}), "Quality", 0.0, 100.0, true, 1);
				ui.enabled = true;
				ui.row([0.5, 0.5]);
				UITrait.inst.layersExport = ui.combo(Id.handle({position: UITrait.inst.layersExport}), ["Visible", "Selected"], "Layers", true);
				ui.combo(hpreset, files, "Preset", true);
				if (hpreset.changed) preset = null;

				@:privateAccess ui.endElement();

				ui.row([0.5, 0.5]);
				if (ui.button("Cancel")) {
					UIBox.show = false;
				}
				if (ui.button("Export")) {
					UIBox.show = false;
					var filters = App.bitsHandle.position != Bits8 ? "exr" : UITrait.inst.formatType == FormatPng ? "png" : "jpg";
					UIFiles.show(filters, true, function(path: String) {
						UITrait.inst.textureExportPath = path;
						function export(_) {
							ExportTexture.run(path);
							iron.App.removeRender(export);
						}
						iron.App.notifyOnRender(export);
					});
				}
				if (ui.isHovered) ui.tooltip("Export texture files (" + Config.keymap.file_export_textures + ")");
			}

			if (ui.tab(htab, "Presets")) {
				ui.row([3 / 5, 1 / 5, 1 / 5]);

				ui.combo(hpreset, files, "Preset");
				if (hpreset.changed) preset = null;

				if (ui.button("New")) {
					UIBox.showCustom(function(ui: Zui) {
						if (ui.tab(Id.handle(), "New Preset")) {
							ui.row([0.5, 0.5]);
							var presetName = ui.textInput(Id.handle({text: "new_preset"}), "Name");
							if (ui.button("OK") || ui.isReturnDown) {
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

				if (ui.button("Import")) {
					UIFiles.show("json", false, function(path: String) {
						path = path.toLowerCase();
						if (path.endsWith(".json")) {
							var filename = path.substr(path.lastIndexOf(Path.sep) + 1);
							var dstPath = Path.data() + Path.sep + "export_presets" + Path.sep + filename;
							File.copy(path, dstPath); // Copy to presets folder
							fetchPresets();
							preset = null;
							hpreset.position = files.indexOf(filename.substr(0, filename.length - 5)); // Strip .json
							Log.info("Preset '" + filename + "' imported.");
						}
						else Log.error(Strings.error1);
					});
				}

				if (preset == null) {
					parsePreset();
					@:privateAccess hpreset.children = null;
				}

				// Texture list
				ui.separator(10, false);
				ui.row([0.2, 0.2, 0.2, 0.2, 0.2]);
				ui.text("Texture");
				ui.text("R");
				ui.text("G");
				ui.text("B");
				ui.text("A");
				ui.changed = false;
				for (i in 0...preset.textures.length) {
					var t = preset.textures[i];
					ui.row([0.2, 0.2, 0.2, 0.2, 0.2]);
					var htex = hpreset.nest(i, {text: t.name});
					t.name = ui.textInput(htex);

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							ui.fill(0, 0, @:privateAccess ui._w / ui.SCALE(), ui.t.ELEMENT_H * 2, ui.t.SEPARATOR_COL);
							ui.text(t.name, Right, ui.t.HIGHLIGHT_COL);
							if (ui.button("Delete", Left)) {
								preset.textures.remove(t);
								savePreset();
							}
						});
					}

					var hr = htex.nest(0, {position: channels.indexOf(t.channels[0])});
					var hg = htex.nest(1, {position: channels.indexOf(t.channels[1])});
					var hb = htex.nest(2, {position: channels.indexOf(t.channels[2])});
					var ha = htex.nest(3, {position: channels.indexOf(t.channels[3])});

					ui.combo(hr, channels, "R");
					if (hr.changed) t.channels[0] = channels[hr.position];
					ui.combo(hg, channels, "G");
					if (hg.changed) t.channels[1] = channels[hg.position];
					ui.combo(hb, channels, "B");
					if (hb.changed) t.channels[2] = channels[hb.position];
					ui.combo(ha, channels, "A");
					if (ha.changed) t.channels[3] = channels[ha.position];
				}

				if (ui.changed) {
					savePreset();
				}

				ui.row([1 / 8]);
				if (ui.button("Add")) {
					preset.textures.push({name: "base", channels: ["base_r", "base_g", "base_b", "1.0"]});
					@:privateAccess hpreset.children = null;
					savePreset();
				}
			}
		}, 500, 310);
	}

	public static function showMesh() {
		UIBox.showCustom(function(ui: Zui) {
			var htab = Id.handle();
			if (ui.tab(htab, "Export Mesh")) {

				UITrait.inst.exportMeshFormat = ui.combo(Id.handle({position: UITrait.inst.exportMeshFormat}), ["obj", "arm"], "Format", true);
				var mesh = Context.paintObject.data.raw;
				var inda = mesh.index_arrays[0].values;
				var tris = Std.int(inda.length / 3);
				ui.text(tris + " triangles");

				@:privateAccess ui.endElement();

				ui.row([0.5, 0.5]);
				if (ui.button("Cancel")) {
					UIBox.show = false;
				}
				if (ui.button("Export")) {
					UIBox.show = false;
					UIFiles.show(UITrait.inst.exportMeshFormat == FormatObj ? "obj" : "arm", true, function(path: String) {
						var f = UIFiles.filename;
						if (f == "") f = "untitled";
						ExportMesh.run(path + "/" + f);
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
		{ "name": "base", "channels": ["base_r", "base_g", "base_b", "1.0"] }
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
