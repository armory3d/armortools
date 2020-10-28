package arm.ui;

import haxe.io.Bytes;
import haxe.Json;
import kha.System;
import kha.Image;
import zui.Zui;
import zui.Id;
import zui.Ext;
import iron.Scene;
import iron.RenderPath;
import iron.system.Input;
import arm.util.ViewportUtil;
import arm.util.UVUtil;
import arm.util.BuildMacros;
import arm.sys.Path;
import arm.sys.File;
import arm.node.MakeMaterial;
import arm.io.ImportAsset;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathForward;
import arm.Enums;

class UIMenu {

	public static var show = false;
	public static var menuCategory = 0;
	public static var menuX = 0;
	public static var menuY = 0;
	public static var menuElements = 0;
	public static var keepOpen = false;
	public static var menuCommands: Zui->Void = null;
	static var changeStarted = false;
	static var showMenuFirst = true;
	static var hideMenu = false;
	static var askToResetLayout = false;

	@:access(zui.Zui)
	public static function render(g: kha.graphics2.Graphics) {
		var ui = App.uiMenu;
		var menuW = Std.int(ui.ELEMENT_W() * 2.0);
		var BUTTON_COL = ui.t.BUTTON_COL;
		ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;
		var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
		ui.t.ELEMENT_OFFSET = 0;
		var ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = 28;

		ui.beginRegion(g, menuX, menuY, menuW);

		if (menuCommands != null) {
			ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * menuElements, ui.t.SEPARATOR_COL);
			menuCommands(ui);
		}
		else {
			var menuItems = [
				16, // MenuFile
				4, // MenuEdit
				#if (krom_windows || krom_linux) 14 #else 13 #end, // MenuViewport
				#if (kha_direct3d12 || kha_vulkan) 13 #else 12 #end, // MenuMode
				17, // MenuCamera
				7 // MenuHelp
			];
			var sepw = menuW / ui.SCALE();
			g.color = ui.t.SEPARATOR_COL;
			g.fillRect(menuX, menuY, menuW, 28 * menuItems[menuCategory] * ui.SCALE());

			if (menuCategory == MenuFile) {
				if (ui.button("      " + tr("New Project..."), Left, Config.keymap.file_new)) Project.projectNewBox();
				if (ui.button("      " + tr("Open..."), Left, Config.keymap.file_open)) Project.projectOpen();
				if (ui.button("      " + tr("Open Recent..."), Left, Config.keymap.file_open_recent)) Project.projectOpenRecentBox();
				if (ui.button("      " + tr("Save"), Left, Config.keymap.file_save)) Project.projectSave();
				if (ui.button("      " + tr("Save As..."), Left, Config.keymap.file_save_as)) Project.projectSaveAs();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      " + tr("Import Texture..."), Left, Config.keymap.file_import_assets)) Project.importAsset(Path.textureFormats.join(","));
				if (ui.button("      " + tr("Import Font..."), Left)) Project.importAsset("ttf,ttc,otf");
				if (ui.button("      " + tr("Import Material..."), Left)) Project.importMaterial();
				if (ui.button("      " + tr("Import Brush..."), Left)) Project.importBrush();
				if (ui.button("      " + tr("Import Mesh..."), Left)) Project.importMesh();
				if (ui.button("      " + tr("Reimport Mesh"), Left, Config.keymap.file_reimport_mesh)) Project.reimportMesh();
				if (ui.button("      " + tr("Reimport Textures"), Left, Config.keymap.file_reimport_textures)) Project.reimportTextures();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      " + tr("Export Textures..."), Left, Config.keymap.file_export_textures_as)) {
					Context.layersExport = ExportVisible;
					BoxExport.showTextures();
				}
				if (ui.button("      " + tr("Export Mesh..."), Left)) BoxExport.showMesh();
				if (ui.button("      " + tr("Bake Material..."), Left)) BoxExport.showBakeMaterial();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      " + tr("Exit"), Left)) System.stop();
			}
			else if (menuCategory == MenuEdit) {
				var stepUndo = "";
				var stepRedo = "";
				if (History.undos > 0) {
					stepUndo = History.steps[History.steps.length - 1 - History.redos].name;
				}
				if (History.redos > 0) {
					stepRedo = History.steps[History.steps.length - History.redos].name;
				}
				ui.enabled = History.undos > 0;
				if (ui.button("      " + tr("Undo {step}", ["step" => stepUndo]), Left, Config.keymap.edit_undo)) History.undo();
				ui.enabled = History.redos > 0;
				if (ui.button("      " + tr("Redo {step}", ["step" => stepRedo]), Left, Config.keymap.edit_redo)) History.redo();
				ui.enabled = true;
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      " + tr("Reset Layout"), Left)) askToResetLayout = true;
				if (ui.button("      " + tr("Preferences..."), Left, Config.keymap.edit_prefs)) BoxPreferences.show();
			}
			else if (menuCategory == MenuViewport) {
				// if (Scene.active.world.probe.radianceMipmaps.length > 0) {
					// ui.image(Scene.active.world.probe.radianceMipmaps[0]);
				// }

				if (ui.button("      " + tr("Import Envmap..."), Left)) {
					UIFiles.show("hdr", false, function(path: String) {
						if (!path.endsWith(".hdr")) {
							Log.error("Error: .hdr file expected");
							return;
						}
						ImportAsset.run(path);
					});
				}

				if (ui.button("      " + tr("Distract Free"), Left, Config.keymap.view_distract_free)) {
					UISidebar.inst.toggleDistractFree();
					UISidebar.inst.ui.isHovered = false;
				}

				if (ui.button("      " + tr("Toggle Fullscreen"), Left, "alt+enter")) {
					App.toggleFullscreen();
				}

				ui.changed = false;

				var p = Scene.active.world.probe;
				var envHandle = Id.handle();
				envHandle.value = p.raw.strength;
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				p.raw.strength = ui.slider(envHandle, tr("Environment"), 0.0, 8.0, true);
				if (envHandle.changed) Context.ddirty = 2;

				if (Scene.active.lights.length > 0) {
					var light = Scene.active.lights[0];

					var lhandle = Id.handle();
					var scale = 1333;
					lhandle.value = light.data.raw.strength / scale;
					lhandle.value = Std.int(lhandle.value * 100) / 100;
					ui.row([1 / 8, 7 / 8]); ui.endElement();
					light.data.raw.strength = ui.slider(lhandle, tr("Light"), 0.0, 4.0, true) * scale;
					if (lhandle.changed) Context.ddirty = 2;

					var sxhandle = Id.handle();
					sxhandle.value = light.data.raw.size;
					ui.row([1 / 8, 7 / 8]); ui.endElement();
					light.data.raw.size = ui.slider(sxhandle, tr("Light Size"), 0.0, 4.0, true);
					if (sxhandle.changed) Context.ddirty = 2;
				}

				var splitViewHandle = Id.handle({selected: Context.splitView});
				Context.splitView = ui.check(splitViewHandle, " " + tr("Split View"));
				if (splitViewHandle.changed) {
					App.resize();
				}

				var cullHandle = Id.handle({selected: Context.cullBackfaces});
				Context.cullBackfaces = ui.check(cullHandle, " " + tr("Cull Backfaces"));
				if (cullHandle.changed) {
					MakeMaterial.parseMeshMaterial();
				}

				var filterHandle = Id.handle({selected: Context.textureFilter});
				Context.textureFilter = ui.check(filterHandle, " " + tr("Filter Textures"));
				if (filterHandle.changed) {
					MakeMaterial.parsePaintMaterial();
					MakeMaterial.parseMeshMaterial();
				}

				Context.drawWireframe = ui.check(Context.wireframeHandle, " " + tr("Wireframe"));
				if (Context.wireframeHandle.changed) {
					ui.g.end();
					UVUtil.cacheUVMap();
					ui.g.begin(false);
					MakeMaterial.parseMeshMaterial();
				}
				Context.drawTexels = ui.check(Context.texelsHandle, " " + tr("Texels"));
				if (Context.texelsHandle.changed) {
					MakeMaterial.parseMeshMaterial();
				}

				var compassHandle = Id.handle({selected: Context.showCompass});
				Context.showCompass = ui.check(compassHandle, " " + tr("Compass"));
				if (compassHandle.changed) Context.ddirty = 2;

				Context.showEnvmap = ui.check(Context.showEnvmapHandle, " " + tr("Envmap"));
				if (Context.showEnvmapHandle.changed) {
					Context.loadEnvmap();
					Context.ddirty = 2;
				}

				Context.showEnvmapBlur = ui.check(Context.showEnvmapBlurHandle, " " + tr("Blur Envmap"));
				if (Context.showEnvmapBlurHandle.changed) Context.ddirty = 2;

				if (Context.showEnvmap) {
					Scene.active.world.envmap = Context.showEnvmapBlur ? Scene.active.world.probe.radianceMipmaps[0] : Context.savedEnvmap;
				}
				else {
					Scene.active.world.envmap = Context.emptyEnvmap;
				}

				if (ui.changed) keepOpen = true;
			}
			else if (menuCategory == MenuMode) {
				var modeHandle = Id.handle();
				var modes = [
					tr("Lit"),
					tr("Base Color"),
					tr("Normal"),
					tr("Occlusion"),
					tr("Roughness"),
					tr("Metallic"),
					tr("Opacity"),
					tr("TexCoord"),
					tr("Object Normal"),
					tr("Material ID"),
					tr("Object ID"),
					tr("Mask")
				];
				#if (kha_direct3d12 || kha_vulkan)
				modes.push(tr("Path Traced"));
				#end
				for (i in 0...modes.length) {
					ui.radio(modeHandle, i, modes[i]);
				}

				Context.viewportMode = modeHandle.position;
				if (modeHandle.changed) {
					var deferred = Context.renderMode != RenderForward && (Context.viewportMode == ViewLit || Context.viewportMode == ViewPathTrace);
					if (deferred) {
						RenderPath.active.commands = RenderPathDeferred.commands;
					}
					// else if (Context.viewportMode == ViewPathTrace) {
					// }
					else {
						if (RenderPathForward.path == null) {
							RenderPathForward.init(RenderPath.active);
						}
						RenderPath.active.commands = RenderPathForward.commands;
					}
					var _workspace = UIHeader.inst.worktab.position;
					UIHeader.inst.worktab.position = SpacePaint;
					MakeMaterial.parseMeshMaterial();
					UIHeader.inst.worktab.position = _workspace;
				}
			}
			else if (menuCategory == MenuCamera) {
				if (ui.button("      " + tr("Reset"), Left, Config.keymap.view_reset)) { ViewportUtil.resetViewport(); ViewportUtil.scaleToBounds(); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      " + tr("Front"), Left, Config.keymap.view_front)) { ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0); }
				if (ui.button("      " + tr("Back"), Left, Config.keymap.view_back)) { ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI); }
				if (ui.button("      " + tr("Right"), Left, Config.keymap.view_right)) { ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2); }
				if (ui.button("      " + tr("Left"), Left, Config.keymap.view_left)) { ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2); }
				if (ui.button("      " + tr("Top"), Left, Config.keymap.view_top)) { ViewportUtil.setView(0, 0, 1, 0, 0, 0); }
				if (ui.button("      " + tr("Bottom"), Left, Config.keymap.view_bottom)) { ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);

				ui.changed = false;

				if (ui.button("      " + tr("Orbit Left"), Left, Config.keymap.view_orbit_left)) { ViewportUtil.orbit(-Math.PI / 12, 0); }
				if (ui.button("      " + tr("Orbit Right"), Left, Config.keymap.view_orbit_right)) { ViewportUtil.orbit(Math.PI / 12, 0); }
				if (ui.button("      " + tr("Orbit Up"), Left, Config.keymap.view_orbit_up)) { ViewportUtil.orbit(0, -Math.PI / 12); }
				if (ui.button("      " + tr("Orbit Down"), Left, Config.keymap.view_orbit_down)) { ViewportUtil.orbit(0, Math.PI / 12); }
				if (ui.button("      " + tr("Orbit Opposite"), Left, Config.keymap.view_orbit_opposite)) { ViewportUtil.orbitOpposite(); }
				if (ui.button("      " + tr("Zoom In"), Left, Config.keymap.view_zoom_in)) { ViewportUtil.zoom(0.2); }
				if (ui.button("      " + tr("Zoom Out"), Left, Config.keymap.view_zoom_out)) { ViewportUtil.zoom(-0.2); }
				// ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);

				var cam = Scene.active.camera;
				Context.fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				cam.data.raw.fov = ui.slider(Context.fovHandle, tr("FoV"), 0.3, 2.0, true);
				if (Context.fovHandle.changed) {
					ViewportUtil.updateCameraType(Context.cameraType);
				}

				ui.row([1 / 8, 7 / 8]); ui.endElement();
				Context.cameraControls = Ext.inlineRadio(ui, Id.handle({position: Context.cameraControls}), [tr("Orbit"), tr("Rotate"), tr("Fly")], Left);

				ui.row([1 / 8, 7 / 8]); ui.endElement();
				Context.cameraType = Ext.inlineRadio(ui, Context.camHandle, [tr("Perspective"), tr("Orthographic")], Left);

				if (ui.isHovered) ui.tooltip(tr("Camera Type") + ' (${Config.keymap.view_camera_type})');
				if (Context.camHandle.changed) {
					ViewportUtil.updateCameraType(Context.cameraType);
				}

				if (ui.changed) keepOpen = true;

			}
			else if (menuCategory == MenuHelp) {
				if (ui.button("      " + tr("Manual"), Left)) {
					File.explorer("https://armorpaint.org/manual");
				}
				if (ui.button("      " + tr("What's New"), Left)) {
					File.explorer("https://armorpaint.org/notes");
				}
				if (ui.button("      " + tr("Issue Tracker"), Left)) {
					File.explorer("https://github.com/armory3d/armorpaint/issues");
				}
				if (ui.button("      " + tr("Report Bug"), Left)) {
					var url = "https://github.com/armory3d/armorpaint/issues/new?labels=bug&template=bug_report.md&body=*ArmorPaint%20" + Main.version + "-" + Main.sha + ",%20" + System.systemId + "*%0A%0A**Issue description:**%0A%0A**Steps to reproduce:**%0A%0A";
					File.explorer(url);
				}
				if (ui.button("      " + tr("Request Feature"), Left)) {
					var url = "https://github.com/armory3d/armorpaint/issues/new?labels=feature%20request&template=feature_request.md&body=*ArmorPaint%20" + Main.version + "-" + Main.sha + ",%20" + System.systemId + "*%0A%0A**Feature description:**%0A%0A";
					File.explorer(url);
				}
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      " + tr("Check for Updates..."), Left)) {
					// Retrieve latest version number
					var url = "'https://luboslenco.gitlab.io/armorpaint/index.html'";
					var blob = File.downloadBytes(url);
					if (blob != null)  {
						// Compare versions
						var update = Json.parse(blob.toString());
						var updateVersion = Std.int(update.version);
						if (updateVersion > 0) {
							var date = BuildMacros.date().split(" ")[0].substr(2); // 2019 -> 19
							var dateInt = Std.parseInt(date.replace("-", ""));
							if (updateVersion > dateInt) {
								UIBox.showMessage(tr("Update"), tr("Update is available!\nPlease visit armorpaint.org to download."));
							}
							else {
								UIBox.showMessage(tr("Update"), tr("You are up to date!"));
							}
						}
					}
					else {
						UIBox.showMessage(tr("Update"), tr("Unable to check for updates.\nPlease visit armorpaint.org."));
					}
				}
				if (ui.button("      " + tr("About..."), Left)) {
					#if kha_direct3d11
					var gapi = "Direct3D11";
					#elseif kha_direct3d12
					var gapi = "Direct3D12";
					#elseif kha_metal
					var gapi = "Metal";
					#elseif kha_vulkan
					var gapi = "Vulkan";
					#else
					var gapi = "OpenGL";
					#end
					var msg = "ArmorPaint.org - v" + Main.version + " (" + Main.date + ") - " + Main.sha + "\n";
					msg += System.systemId + " - " + gapi;

					#if krom_windows
					var save = (Path.isProtected() ? Krom.savePath() : Path.data()) + Path.sep + "tmp.txt";
					Krom.sysCommand('wmic path win32_VideoController get name > "' + save + '"');
					var bytes = haxe.io.Bytes.ofData(Krom.loadBlob(save));
					var gpu = "";
					for (i in 30...Std.int(bytes.length / 2)) {
						var c = String.fromCharCode(bytes.get(i * 2));
						if (c == "\n") continue;
						gpu += c;
					}
					msg += '\n$gpu';
					#else
					// { lshw -C display }
					#end

					UIBox.showMessage(tr("About"), msg, true);
				}
			}
		}

		var first = showMenuFirst;
		hideMenu = ui.comboSelectedHandle == null && !changeStarted && !keepOpen && !first && (ui.changed || ui.inputReleased || ui.inputReleasedR || ui.isEscapeDown);
		showMenuFirst = false;
		keepOpen = false;
		if (ui.inputReleased) changeStarted = false;

		ui.t.BUTTON_COL = BUTTON_COL;
		ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
		ui.t.ELEMENT_H = ELEMENT_H;
		ui.endRegion();
	}

	public static function update() {
		//var ui = App.uiMenu;
		if (hideMenu) {
			show = false;
			App.redrawUI();
			showMenuFirst = true;
			menuCommands = null;
		}

		if (askToResetLayout) {
			askToResetLayout = false;
			UIMenu.draw(function(ui: Zui) {
				ui.text(tr("Reset layout?"), Right, ui.t.HIGHLIGHT_COL);
				if (ui.button(tr("Confirm"), Left)) {
					Config.initLayout();
					Config.save();
				}
			}, 2);
		}
	}

	public static function draw(commands: Zui->Void = null, elements: Int, x = -1, y = -1) {
		show = true;
		menuCommands = commands;
		menuElements = elements;
		menuX = x > -1 ? x : Std.int(Input.getMouse().x);
		menuY = y > -1 ? y : Std.int(Input.getMouse().y);
		var menuW = App.uiMenu.ELEMENT_W() * 2.0;
		if (menuX + menuW > System.windowWidth()) {
			menuX = Std.int(System.windowWidth() - menuW);
		}
		var menuH = menuElements * 28; // ui.t.ELEMENT_H
		if (menuY + menuH > System.windowHeight()) {
			menuY = System.windowHeight() - menuH;
			menuX += 1; // Move out of mouse focus
		}
	}
}
