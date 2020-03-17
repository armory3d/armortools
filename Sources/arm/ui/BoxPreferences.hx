package arm.ui;

import haxe.io.Bytes;
import haxe.Json;
import zui.Id;
import zui.Zui;
import iron.data.Data;
import arm.node.MaterialParser;
import arm.data.LayerSlot;
import arm.io.ImportPlugin;
import arm.io.ImportKeymap;
import arm.sys.Path;
import arm.sys.File;
import arm.App.tr;
using StringTools;

class BoxPreferences {

	public static var htab = Id.handle();
	public static var filesPlugin: Array<String> = null;
	public static var filesKeymap: Array<String> = null;
	public static var presetHandle: Handle;

	@:access(zui.Zui)
	public static function show() {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(htab, tr("Interface"), true)) {

				var hscale = Id.handle({value: Config.raw.window_scale});
				ui.slider(hscale, tr("UI Scale"), 1.0, 4.0, false, 10);
				if (!hscale.changed && UITrait.inst.hscaleWasChanged) {
					if (hscale.value == null || Math.isNaN(hscale.value)) hscale.value = 1.0;
					Config.raw.window_scale = hscale.value;
					Config.save();
					setScale();
				}
				UITrait.inst.hscaleWasChanged = hscale.changed;
				var themeHandle = Id.handle();
				var themes = ["Dark", "Light"];
				ui.combo(themeHandle, themes, tr("Theme"), true);
				if (themeHandle.changed) {
					var theme = themes[themeHandle.position].toLowerCase();
					if (theme == "dark") { // Built-in default
						App.theme = zui.Themes.dark;
					}
					else {
						Data.getBlob("themes/theme_" + theme + ".arm", function(b: kha.Blob) {
							App.theme = Json.parse(b.toString());
						});
					}
					ui.t = App.theme;
					UITrait.inst.ui.t = App.theme;
					UINodes.inst.ui.t = App.theme;
					UIView2D.inst.ui.t = App.theme;
					UITrait.inst.tagUIRedraw();
				}

				#if (!krom_android && !krom_ios)
				UITrait.inst.nativeBrowser = ui.check(Id.handle({selected: UITrait.inst.nativeBrowser}), tr("Native File Browser"));
				#end

				UITrait.inst.cacheDraws = ui.check(Id.handle({selected: UITrait.inst.cacheDraws}), tr("Cache UI Draws"));
				if (ui.isHovered) ui.tooltip(tr("Enabling may reduce GPU usage"));

				ui.changed = false;
				UITrait.inst.showAssetNames = ui.check(Id.handle({selected: UITrait.inst.showAssetNames}), tr("Show Asset Names"));
				if (ui.changed) {
					UITrait.inst.tagUIRedraw();
				}

				// ui.text("Node Editor");
				// var gridSnap = ui.check(Id.handle({selected: false}), "Grid Snap");

				ui.endElement();
				ui.row([0.5]);
				if (ui.button(tr("Restore"))) {
					UIMenu.draw(function(ui: Zui) {
						ui.text(tr("Restore defaults?"), Right, ui.t.HIGHLIGHT_COL);
						if (ui.button(tr("Confirm"), Left)) {
							ui.t.ELEMENT_H = App.ELEMENT_H;
							Config.restore();
							setScale();
							if (filesPlugin != null) for (f in filesPlugin) Plugin.stop(f);
							filesPlugin = null;
							filesKeymap = null;
						}
					}, 2);
				}
			}
			if (ui.tab(htab, tr("Usage"), true)) {
				UITrait.inst.undoHandle = Id.handle({value: Config.raw.undo_steps});
				Config.raw.undo_steps = Std.int(ui.slider(UITrait.inst.undoHandle, tr("Undo Steps"), 1, 64, false, 1));
				if (UITrait.inst.undoHandle.changed) {
					ui.g.end();
					while (History.undoLayers.length < Config.raw.undo_steps) {
						var l = new LayerSlot("_undo" + History.undoLayers.length);
						l.createMask(0, false);
						History.undoLayers.push(l);
					}
					while (History.undoLayers.length > Config.raw.undo_steps) {
						var l = History.undoLayers.pop();
						l.unload();
					}
					History.reset();
					ui.g.begin(false);
					Config.save();
				}

				UITrait.inst.brushBias = ui.slider(Id.handle({value: UITrait.inst.brushBias}), tr("Paint Bleed"), 0.0, 2.0, true);
				if (ui.isHovered) ui.tooltip(tr("Stretch brush strokes on the uv map to prevent seams"));

				UITrait.inst.dilateRadius = ui.slider(Id.handle({value: UITrait.inst.dilateRadius}), tr("Dilate Radius"), 0.0, 64.0, true, 1);
				if (ui.isHovered) ui.tooltip(tr("Dilate baked textures to prevent seams"));

				var brushLiveHandle = Id.handle({selected: UITrait.inst.brushLive});
				UITrait.inst.brushLive = ui.check(brushLiveHandle, tr("Live Brush Preview"));
				if (ui.isHovered) ui.tooltip(tr("Draw live brush preview in viewport"));
				if (brushLiveHandle.changed) Context.ddirty = 2;

				var brush3dHandle = Id.handle({selected: UITrait.inst.brush3d});
				UITrait.inst.brush3d = ui.check(brush3dHandle, tr("3D Cursor"));
				if (brush3dHandle.changed) MaterialParser.parsePaintMaterial();

				ui.enabled = UITrait.inst.brush3d;
				var brushDepthRejectHandle = Id.handle({selected: UITrait.inst.brushDepthReject});
				UITrait.inst.brushDepthReject = ui.check(brushDepthRejectHandle, tr("Depth Reject"));
				if (brushDepthRejectHandle.changed) MaterialParser.parsePaintMaterial();

				ui.row([0.5, 0.5]);

				var brushAngleRejectHandle = Id.handle({selected: UITrait.inst.brushAngleReject});
				UITrait.inst.brushAngleReject = ui.check(brushAngleRejectHandle, tr("Angle Reject"));
				if (brushAngleRejectHandle.changed) MaterialParser.parsePaintMaterial();

				if (!UITrait.inst.brushAngleReject) ui.enabled = false;
				var angleDotHandle = Id.handle({value: UITrait.inst.brushAngleRejectDot});
				UITrait.inst.brushAngleRejectDot = ui.slider(angleDotHandle, tr("Angle"), 0.0, 1.0, true);
				if (angleDotHandle.changed) {
					MaterialParser.parsePaintMaterial();
				}
				ui.enabled = true;
			}
			if (ui.tab(htab, tr("Pen"), true)) {
				UITrait.penPressureRadius = ui.check(Id.handle({selected: UITrait.penPressureRadius}), tr("Brush Radius"));
				UITrait.penPressureHardness = ui.check(Id.handle({selected: UITrait.penPressureHardness}), tr("Brush Hardness"));
				UITrait.penPressureOpacity = ui.check(Id.handle({selected: UITrait.penPressureOpacity}), tr("Brush Opacity"));
			}

			UITrait.inst.hssgi = Id.handle({selected: Config.raw.rp_ssgi});
			UITrait.inst.hssr = Id.handle({selected: Config.raw.rp_ssr});
			UITrait.inst.hbloom = Id.handle({selected: Config.raw.rp_bloom});
			UITrait.inst.hsupersample = Id.handle({position: Config.getSuperSampleQuality(Config.raw.rp_supersample)});
			UITrait.inst.hvxao = Id.handle({selected: Config.raw.rp_gi});
			if (ui.tab(htab, tr("Viewport"), true)) {
				ui.combo(UITrait.inst.hsupersample, ["0.25x", "0.5x", "1.0x", "1.5x", "2.0x", "4.0x"], tr("Super Sample"), true);
				if (UITrait.inst.hsupersample.changed) Config.applyConfig();

				#if arm_debug
				var vsyncHandle = Id.handle({selected: Config.raw.window_vsync});
				Config.raw.window_vsync = ui.check(vsyncHandle, tr("VSync"));
				if (vsyncHandle.changed) Config.save();
				#end

				#if rp_voxelao
				ui.check(UITrait.inst.hvxao, tr("Voxel AO"));
				if (ui.isHovered) ui.tooltip(tr("Cone-traced AO and shadows"));
				if (UITrait.inst.hvxao.changed) {
					Config.applyConfig();
					#if arm_creator
					MaterialParser.parseMeshMaterial();
					#end
				}

				ui.row([0.5, 0.5]);
				ui.enabled = UITrait.inst.hvxao.selected;
				var h = Id.handle({value: UITrait.inst.vxaoOffset});
				UITrait.inst.vxaoOffset = ui.slider(h, tr("Cone Offset"), 1.0, 4.0, true);
				if (h.changed) Context.ddirty = 2;
				var h = Id.handle({value: UITrait.inst.vxaoAperture});
				UITrait.inst.vxaoAperture = ui.slider(h, tr("Aperture"), 1.0, 4.0, true);
				if (h.changed) Context.ddirty = 2;
				ui.enabled = true;
				#end
				ui.check(UITrait.inst.hssgi, tr("SSAO"));
				if (UITrait.inst.hssgi.changed) Config.applyConfig();
				ui.check(UITrait.inst.hbloom, tr("Bloom"));
				if (UITrait.inst.hbloom.changed) Config.applyConfig();
				ui.check(UITrait.inst.hssr, tr("SSR"));
				if (UITrait.inst.hssr.changed) Config.applyConfig();

				var h = Id.handle({value: UITrait.inst.vignetteStrength});
				UITrait.inst.vignetteStrength = ui.slider(h, tr("Vignette"), 0.0, 1.0, true);
				if (h.changed) Context.ddirty = 2;

				// var h = Id.handle({value: UITrait.inst.autoExposureStrength});
				// UITrait.inst.autoExposureStrength = ui.slider(h, "Auto Exposure", 0.0, 2.0, true);
				// if (h.changed) Context.ddirty = 2;

				#if arm_creator
				var h = Id.handle({value: UITrait.inst.vxaoExt});
				UITrait.inst.vxaoExt = ui.slider(h, tr("VXAO Ext"), 1.0, 10.0);
				if (h.changed) {
					Context.ddirty = 2;
					MaterialParser.parseMeshMaterial();
				}
				#end
			}
			if (ui.tab(htab, tr("Keymap"), true)) {

				if (filesKeymap == null) {
					fetchKeymaps();
				}

				ui.row([1 / 2, 1 / 4, 1 / 4]);

				presetHandle = Id.handle({position: getPresetIndex()});
				ui.combo(presetHandle, filesKeymap, tr("Preset"));
				if (presetHandle.changed) {
					Config.raw.keymap = filesKeymap[presetHandle.position] + ".json";
					Config.applyConfig();
					Config.loadKeymap();
				}

				if (ui.button(tr("Import"))) {
					UIFiles.show("json", false, function(path: String) {
						ImportKeymap.run(path);
					});
				}
				if (ui.button(tr("Export"))) {
					UIFiles.show("json", true, function(dest: String) {
						if (!UIFiles.filename.endsWith(".json")) UIFiles.filename += ".json";
						var path = Path.data() + Path.sep + "keymap_presets" + Path.sep + Config.raw.keymap;
						File.copy(path, dest + Path.sep + UIFiles.filename);
					});
				}

				ui.separator(8, false);

				var i = 0;
				ui.changed = false;
				for (key in Reflect.fields(Config.keymap)) {
					var h = Id.handle().nest(i++);
					h.text = Reflect.field(Config.keymap, key);
					var text = ui.textInput(h, key, Left);
					Reflect.setField(Config.keymap, key, text);
				}
				if (ui.changed) {
					Config.applyConfig();
					Config.saveKeymap();
				}
			}
			if (ui.tab(htab, tr("Plugins"), true)) {
				ui.row([1 / 4, 1 / 4]);
				if (ui.button(tr("New"))) {
					UIBox.showCustom(function(ui: Zui) {
						if (ui.tab(Id.handle(), tr("New Plugin"))) {
							ui.row([0.5, 0.5]);
							var pluginName = ui.textInput(Id.handle({text: "new_plugin"}), tr("Name"));
							if (ui.button(tr("OK")) || ui.isReturnDown) {
								var template =
"let plugin = new arm.Plugin();
let h1 = new zui.Handle();
plugin.drawUI = function(ui) {
	if (ui.panel(h1, 'New Plugin')) {
		if (ui.button('Button')) {
			arm.Log.error('Hello');
		}
	}
}
";
								if (!pluginName.endsWith(".js")) pluginName += ".js";
								var path = Path.data() + Path.sep + "plugins" + Path.sep + pluginName;
								Krom.fileSaveBytes(path, Bytes.ofString(template).getData());
								filesPlugin = null; // Refresh file list
								UIBox.show = false;
								App.redrawUI();
								BoxPreferences.htab.position = 5; // Plugins
								BoxPreferences.show();
							}
						}
					});
				}
				if (ui.button(tr("Import"))) {
					UIFiles.show("js,wasm,zip", false, function(path: String) {
						ImportPlugin.run(path);
					});
				}

				if (filesPlugin == null) {
					filesPlugin = File.readDirectory(Path.data() + Path.sep + "plugins");
				}

				if (Config.raw.plugins == null) Config.raw.plugins = [];
				var h = Id.handle({selected: false});
				for (f in filesPlugin) {
					var isJs = f.endsWith(".js");
					var isWasm = false; //f.endsWith(".wasm");
					if (!isJs && !isWasm) continue;
					var enabled = Config.raw.plugins.indexOf(f) >= 0;
					h.selected = enabled;
					var tag = isJs ? f.split(".")[0] : f;
					ui.check(h, tag);
					if (h.changed && h.selected != enabled) {
						if (h.selected) {
							Config.raw.plugins.push(f);
							Plugin.start(f);
						}
						else {
							Config.raw.plugins.remove(f);
							Plugin.stop(f);
						}
						Config.save();
						App.redrawUI();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							ui.text(f, Right, ui.t.HIGHLIGHT_COL);
							var path = Path.data() + Path.sep + "plugins" + Path.sep + f;
							if (ui.button(tr("Edit in Text Editor"), Left)) {
								File.start(path);
							}
							if (ui.button(tr("Edit in Script Tab"), Left)) {
								iron.data.Data.getBlob("plugins/" + f, function(blob: kha.Blob) {
									TabScript.hscript.text = blob.toString();
									iron.data.Data.deleteBlob("plugins/" + f);
									Log.info("Script opened");
								});

							}
							if (ui.button(tr("Export"), Left)) {
								UIFiles.show("js", true, function(dest: String) {
									if (!UIFiles.filename.endsWith(".js")) UIFiles.filename += ".js";
									File.copy(path, dest + Path.sep + UIFiles.filename);
								});
							}
							if (ui.button(tr("Delete"), Left)) {
								if (Config.raw.plugins.indexOf(f) >= 0) {
									Config.raw.plugins.remove(f);
									Plugin.stop(f);
								}
								filesPlugin.remove(f);
								File.delete(path);
							}
						}, 5);
					}
				}
			}
		}, 600, 400);
	}

	public static function fetchKeymaps() {
		filesKeymap = File.readDirectory(Path.data() + Path.sep + "keymap_presets");
		for (i in 0...filesKeymap.length) {
			filesKeymap[i] = filesKeymap[i].substr(0, filesKeymap[i].length - 5); // Strip .json
		}
	}

	public static function getPresetIndex(): Int {
		return filesKeymap.indexOf(Config.raw.keymap.substr(0, Config.raw.keymap.length - 5)); // Strip .json
	}

	static function setScale() {
		var scale = Config.raw.window_scale;
		UITrait.inst.ui.setScale(scale);
		UITrait.inst.windowW = Std.int(UITrait.defaultWindowW * scale);
		UITrait.inst.toolbarw = Std.int(UITrait.defaultToolbarW * scale);
		UITrait.inst.headerh = Std.int(UITrait.defaultHeaderH * scale);
		UITrait.inst.statush = Std.int(UITrait.defaultStatusH * scale);
		UITrait.inst.menubarw = Std.int(UITrait.defaultMenubarW * scale);
		UITrait.inst.setIconScale();
		UINodes.inst.ui.setScale(scale);
		UIView2D.inst.ui.setScale(scale);
		App.uibox.setScale(scale);
		App.uimenu.setScale(scale);
		App.resize();
	}
}
