package arm.ui;

import haxe.io.Bytes;
import haxe.Json;
import kha.System;
import kha.Image;
import zui.Zui;
import zui.Id;
import zui.Ext;
import iron.Scene;
import iron.system.Input;
import arm.util.ViewportUtil;
import arm.util.UVUtil;
import arm.util.BuildMacros;
import arm.sys.Path;
import arm.sys.File;
import arm.node.MaterialParser;
import arm.io.ImportAsset;
import arm.Tool;
using StringTools;

class UIMenu {

	public static var show = false;
	public static var menuCategory = 0;
	public static var menuX = 0;
	public static var menuY = 0;
	public static var keepOpen = false;
	public static var menuCommands: Zui->Void = null;
	static var changeStarted = false;
	static var showMenuFirst = true;
	static var hideMenu = false;
	static var viewportColorHandle = Id.handle({selected: false});
	static var envmapLoaded = false;

	@:access(zui.Zui)
	public static function render(g: kha.graphics2.Graphics) {
		var ui = App.uimenu;
		var menuW = Std.int(ui.ELEMENT_W() * 2.0);
		var BUTTON_COL = ui.t.BUTTON_COL;
		ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;
		var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
		ui.t.ELEMENT_OFFSET = 0;
		var ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = 28;

		ui.beginRegion(g, menuX, menuY, menuW);

		if (menuCommands != null) {
			menuCommands(ui);
		}
		else {
			var menuItems = [12, 3, 14, 17, 5];
			if (viewportColorHandle.selected) menuItems[2] += 6;
			var sepw = menuW / ui.SCALE();
			g.color = ui.t.SEPARATOR_COL;
			g.fillRect(menuX, menuY, menuW, 28 * menuItems[menuCategory] * ui.SCALE());

			if (menuCategory == MenuFile) {
				if (ui.button("      New Project...", Left, Config.keymap.file_new)) Project.projectNewBox();
				if (ui.button("      Open...", Left, Config.keymap.file_open)) Project.projectOpen();
				if (ui.button("      Save", Left, Config.keymap.file_save)) Project.projectSave();
				if (ui.button("      Save As...", Left, Config.keymap.file_save_as)) Project.projectSaveAs();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      Import Texture...", Left, Config.keymap.file_import_assets)) Project.importAsset(Path.textureFormats.join(","));
				if (ui.button("      Import Font...", Left)) Project.importAsset("ttf");
				if (ui.button("      Import Material...", Left)) Project.importMaterial();
				if (ui.button("      Import Mesh...", Left)) Project.importMesh();
				if (ui.button("      Reimport Mesh", Left, Config.keymap.file_reimport_mesh)) Project.reimportMesh();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      Export Textures...", Left, Config.keymap.file_export_textures_as)) BoxExport.showTextures();
				if (ui.button("      Export Mesh...", Left)) BoxExport.showMesh();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      Exit", Left)) System.stop();
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
				if (ui.button("      Undo " + stepUndo, Left, Config.keymap.edit_undo)) History.undo();
				ui.enabled = History.redos > 0;
				if (ui.button("      Redo " + stepRedo, Left, Config.keymap.edit_redo)) History.redo();
				ui.enabled = true;
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      Preferences...", Left, Config.keymap.edit_prefs)) BoxPreferences.show();
			}
			else if (menuCategory == MenuViewport) {
				// if (Scene.active.world.probe.radianceMipmaps.length > 0) {
					// ui.image(Scene.active.world.probe.radianceMipmaps[0]);
				// }

				if (ui.button("      Import Envmap...", Left)) {
					UIFiles.show("hdr", false, function(path: String) {
						if (!path.endsWith(".hdr")) {
							Log.error("Error: .hdr file expected");
							return;
						}
						ImportAsset.run(path);
					});
				}

				if (ui.button("      Distract Free", Left, Config.keymap.view_distract_free)) {
					UITrait.inst.toggleDistractFree();
					UITrait.inst.ui.isHovered = false;
				}

				ui.changed = false;

				var p = Scene.active.world.probe;
				var envHandle = Id.handle();
				envHandle.value = p.raw.strength;
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				p.raw.strength = ui.slider(envHandle, "Environment", 0.0, 8.0, true);
				if (envHandle.changed) Context.ddirty = 2;

				if (Scene.active.lights.length > 0) {
					var light = Scene.active.lights[0];

					var lhandle = Id.handle();
					#if arm_world
					var scale = 1;
					#else
					var scale = 1333;
					#end
					lhandle.value = light.data.raw.strength / scale;
					lhandle.value = Std.int(lhandle.value * 100) / 100;
					ui.row([1 / 8, 7 / 8]); ui.endElement();
					light.data.raw.strength = ui.slider(lhandle, "Light", 0.0, 4.0, true) * scale;
					if (lhandle.changed) Context.ddirty = 2;

					var sxhandle = Id.handle();
					sxhandle.value = light.data.raw.size;
					ui.row([1 / 8, 7 / 8]); ui.endElement();
					light.data.raw.size = ui.slider(sxhandle, "Light Size", 0.0, 4.0, true);
					if (sxhandle.changed) Context.ddirty = 2;
				}

				var dispHandle = Id.handle({value: UITrait.inst.displaceStrength});
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				UITrait.inst.displaceStrength = ui.slider(dispHandle, "Displace", 0.0, 2.0, true);
				if (dispHandle.changed) {
					MaterialParser.parseMeshMaterial();
				}

				var splitViewHandle = Id.handle({selected: UITrait.inst.splitView});
				UITrait.inst.splitView = ui.check(splitViewHandle, " Split View");
				if (splitViewHandle.changed) {
					App.resize();
				}

				var cullHandle = Id.handle({selected: UITrait.inst.cullBackfaces});
				UITrait.inst.cullBackfaces = ui.check(cullHandle, " Cull Backfaces");
				if (cullHandle.changed) {
					MaterialParser.parseMeshMaterial();
				}

				var filterHandle = Id.handle({selected: UITrait.inst.textureFilter});
				UITrait.inst.textureFilter = ui.check(filterHandle, " Filter Textures");
				if (filterHandle.changed) {
					MaterialParser.parsePaintMaterial();
					MaterialParser.parseMeshMaterial();
				}

				UITrait.inst.drawWireframe = ui.check(UITrait.inst.wireframeHandle, " Wireframe");
				if (UITrait.inst.wireframeHandle.changed) {
					ui.g.end();
					UVUtil.cacheUVMap();
					ui.g.begin(false);
					MaterialParser.parseMeshMaterial();
				}
				UITrait.inst.drawTexels = ui.check(UITrait.inst.texelsHandle, " Texels");
				if (UITrait.inst.texelsHandle.changed) {
					MaterialParser.parseMeshMaterial();
				}

				var compassHandle = Id.handle({selected: UITrait.inst.showCompass});
				UITrait.inst.showCompass = ui.check(compassHandle, " Compass");
				if (compassHandle.changed) Context.ddirty = 2;

				UITrait.inst.showEnvmap = ui.check(UITrait.inst.showEnvmapHandle, " Envmap");
				if (UITrait.inst.showEnvmapHandle.changed) {
					var world = Scene.active.world;
					if (!envmapLoaded) {
						// TODO: Unable to share texture for both radiance and envmap - reload image
						envmapLoaded = true;
						iron.data.Data.cachedImages.remove("World_radiance.k");
					}
					world.loadEnvmap(function(_) {});
					if (UITrait.inst.savedEnvmap == null) UITrait.inst.savedEnvmap = world.envmap;
					Context.ddirty = 2;
				}

				if (UITrait.inst.showEnvmap) {
					UITrait.inst.showEnvmapBlur = ui.check(UITrait.inst.showEnvmapBlurHandle, " Blurred");
					if (UITrait.inst.showEnvmapBlurHandle.changed) Context.ddirty = 2;
				}
				else {
					if (ui.panel(viewportColorHandle, " Viewport Color")) {
						var hwheel = Id.handle({color: 0xff030303});
						var worldColor: kha.Color = Ext.colorWheel(ui, hwheel);
						if (hwheel.changed) {
							// var b = UITrait.inst.emptyEnvmap.lock(); // No lock for d3d11
							// b.set(0, worldColor.Rb);
							// b.set(1, worldColor.Gb);
							// b.set(2, worldColor.Bb);
							// UITrait.inst.emptyEnvmap.unlock();
							// UITrait.inst.emptyEnvmap.unload(); //
							var b = Bytes.alloc(4);
							b.set(0, worldColor.Rb);
							b.set(1, worldColor.Gb);
							b.set(2, worldColor.Bb);
							b.set(3, 255);
							UITrait.inst.emptyEnvmap = Image.fromBytes(b, 1, 1);
							Context.ddirty = 2;
							if (ui.inputStarted) changeStarted = true;
						}
					}
				}

				if (UITrait.inst.showEnvmap) {
					Scene.active.world.envmap = UITrait.inst.showEnvmapBlur ? Scene.active.world.probe.radianceMipmaps[0] : UITrait.inst.savedEnvmap;
				}
				else {
					Scene.active.world.envmap = UITrait.inst.emptyEnvmap;
				}

				#if arm_creator
				// ui.check(Id.handle({selected: true}), "Sun");
				// ui.check(Id.handle({selected: true}), "Clouds");
				Project.waterPass = ui.check(Id.handle({selected: Project.waterPass}), " Water");
				// var world = iron.Scene.active.world;
				// var light = iron.Scene.active.lights[0];
				// // Sync sun direction
				// var v = light.look();
				// world.raw.sun_direction[0] = v.x;
				// world.raw.sun_direction[1] = v.y;
				// world.raw.sun_direction[2] = v.z;
				#end

				if (ui.changed) keepOpen = true;
			}
			else if (menuCategory == MenuCamera) {
				if (ui.button("      Reset", Left, Config.keymap.view_reset)) { ViewportUtil.resetViewport(); ViewportUtil.scaleToBounds(); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("      Front", Left, Config.keymap.view_front)) { ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0); }
				if (ui.button("      Back", Left, Config.keymap.view_back)) { ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI); }
				if (ui.button("      Right", Left, Config.keymap.view_right)) { ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2); }
				if (ui.button("      Left", Left, Config.keymap.view_left)) { ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2); }
				if (ui.button("      Top", Left, Config.keymap.view_top)) { ViewportUtil.setView(0, 0, 1, 0, 0, 0); }
				if (ui.button("      Bottom", Left, Config.keymap.view_bottom)) { ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);

				ui.changed = false;

				if (ui.button("      Orbit Left", Left, Config.keymap.view_orbit_left)) { ViewportUtil.orbit(-Math.PI / 12, 0); }
				if (ui.button("      Orbit Right", Left, Config.keymap.view_orbit_right)) { ViewportUtil.orbit(Math.PI / 12, 0); }
				if (ui.button("      Orbit Up", Left, Config.keymap.view_orbit_up)) { ViewportUtil.orbit(0, -Math.PI / 12); }
				if (ui.button("      Orbit Down", Left, Config.keymap.view_orbit_down)) { ViewportUtil.orbit(0, Math.PI / 12); }
				if (ui.button("      Orbit Opposite", Left, Config.keymap.view_orbit_opposite)) { ViewportUtil.orbit(Math.PI, 0); }
				// ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);

				var cam = Scene.active.camera;
				var camRaw = cam.data.raw;
				var near_handle = Id.handle({value: camRaw.near_plane});
				var far_handle = Id.handle({value: camRaw.far_plane});
				near_handle.value = Std.int(near_handle.value * 1000) / 1000;
				far_handle.value = Std.int(far_handle.value * 100) / 100;
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				camRaw.near_plane = ui.slider(near_handle, "Clip Start", 0.001, 1.0, true);
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				camRaw.far_plane = ui.slider(far_handle, "Clip End", 50.0, 100.0, true);
				if (near_handle.changed || far_handle.changed) {
					Scene.active.camera.buildProjection();
				}

				UITrait.inst.fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
				ui.row([1 / 8, 7 / 8]); ui.endElement();
				cam.data.raw.fov = ui.slider(UITrait.inst.fovHandle, "FoV", 0.3, 2.0, true);
				if (UITrait.inst.fovHandle.changed) {
					ViewportUtil.updateCameraType(UITrait.inst.cameraType);
				}

				ui.row([1 / 8, 7 / 8]); ui.endElement();
				UITrait.inst.cameraControls = Ext.inlineRadio(ui, Id.handle({position: UITrait.inst.cameraControls}), ["Orbit", "Rotate", "Fly"], Left);

				ui.row([1 / 8, 7 / 8]); ui.endElement();
				UITrait.inst.cameraType = Ext.inlineRadio(ui, UITrait.inst.camHandle, ["Perspective", "Orthographic"], Left);

				if (ui.isHovered) ui.tooltip("Camera Type (" + Config.keymap.view_camera_type + ")");
				if (UITrait.inst.camHandle.changed) {
					ViewportUtil.updateCameraType(UITrait.inst.cameraType);
				}

				if (ui.changed) keepOpen = true;

			}
			else if (menuCategory == MenuHelp) {
				if (ui.button("      Manual", Left)) {
					File.explorer("https://armorpaint.org/manual");
				}
				if (ui.button("      Issue Tracker", Left)) {
					File.explorer("https://github.com/armory3d/armorpaint/issues");
				}
				if (ui.button("      Report Bug", Left)) {
					var ver = App.version;
					var sha = BuildMacros.sha();
					sha = sha.substr(1, sha.length - 2);
					var os = System.systemId;
					var url = "https://github.com/armory3d/armorpaint/issues/new?labels=bug&template=bug_report.md&body=*ArmorPaint%20" + ver + "-" + sha + ",%20" + os + "*";
					File.explorer(url);
				}
				if (ui.button("      Check for Updates...", Left)) {
					// Retrieve latest version number
					var url = "'https://luboslenco.gitlab.io/armorpaint/index.html'";
					var blob = File.downloadBytes(url);
					// Compare versions
					var update = Json.parse(blob.toString());
					var updateVersion = Std.int(update.version);
					if (updateVersion > 0) {
						var date = BuildMacros.date().split(" ")[0].substr(2); // 2019 -> 19
						var dateInt = Std.parseInt(date.replace("-", ""));
						if (updateVersion > dateInt) {
							UIBox.showMessage("Update", "Update is available!\nPlease visit armorpaint.org to download.");
						}
						else {
							UIBox.showMessage("Update", "You are up to date!");
						}
					}
				}
				if (ui.button("      About...", Left)) {
					var sha = BuildMacros.sha();
					sha = sha.substr(1, sha.length - 2);
					var date = BuildMacros.date().split(" ")[0];
					var gapi = #if (kha_direct3d11) "Direct3D11" #elseif (kha_direct3d12) "Direct3D12" #else "OpenGL" #end;
					var msg = "ArmorPaint.org - v" + App.version + " (" + date + ") - " + sha + "\n";
					msg += System.systemId + " - " + gapi;

					#if krom_windows
					var save = Path.data() + Path.sep + "gpu.txt";
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

					UIBox.showMessage("About", msg);
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
		//var ui = App.uimenu;
		if (hideMenu) {
			show = false;
			App.redrawUI();
			showMenuFirst = true;
			menuCommands = null;
		}
	}

	public static function draw(commands: Zui->Void = null, x = -1, y = -1) {
		show = true;
		menuCommands = commands;
		menuX = x > -1 ? x : Std.int(Input.getMouse().x);
		menuY = y > -1 ? y : Std.int(Input.getMouse().y);
		var menuW = App.uimenu.ELEMENT_W() * 1.7;
		if (menuX + menuW > System.windowWidth()) {
			menuX = Std.int(System.windowWidth() - menuW);
		}
	}
}
