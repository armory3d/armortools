package arm.ui;

import haxe.Json;
import kha.System;
import kha.Blob;
import zui.Zui;
import iron.system.Input;
import iron.data.Data;
import arm.util.ViewportUtil;
using StringTools;

class UIMenu {

	public static var show = false;
	public static var menuCategory = 0;
	public static var propChanged = false;
	static var showMenuFirst = true;
	static var menuX = 0;
	static var menuY = 0;
	static var menuCommands:Zui->Void = null;

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {
		var ui = App.uibox;

		var panelx = iron.App.x() - UITrait.inst.toolbarw;
		var C = Config.raw;

		var menuButtonW = Std.int(ui.ELEMENT_W() * 0.5);
		var px = panelx + menuButtonW * menuCategory;
		var py = UITrait.inst.headerh;
		var menuItems = [6, 3, 14, 5];
		var ph = 24 * menuItems[menuCategory] * ui.SCALE;

		g.color = ui.t.SEPARATOR_COL;
		var menuw = Std.int(ui.ELEMENT_W() * 1.5);
		var sepw = menuw / ui.SCALE;

		if (menuCommands != null) {
			px = menuX;
			py = menuY;
		}
		else {
			g.fillRect(px, py, menuw, ph);
		}

		g.end();

		ui.beginLayout(g, Std.int(px), Std.int(py), menuw);
		var BUTTON_COL = ui.t.BUTTON_COL;
		ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

		var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
		ui.t.ELEMENT_OFFSET = 0;
		var ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Std.int(24);

		ui.changed = false;

		if (menuCommands != null) {
			menuCommands(ui);
		}
		else {
			if (menuCategory == 0) {
				if (ui.button("New Project...", Left, Config.keymap.file_new)) Project.projectNewBox();
				if (ui.button("Open...", Left, Config.keymap.file_open)) Project.projectOpen();
				if (ui.button("Save", Left, Config.keymap.file_save)) Project.projectSave();
				if (ui.button("Save As...", Left, Config.keymap.file_save_as)) Project.projectSaveAs();
				if (ui.button("Reload Assets", Left, Config.keymap.file_reload_assets)) Project.reloadAssets();
				// ui.button("Import Asset...", Left);
				// ui.button("Export Textures...", Left);
				// ui.button("Export Mesh...", Left);
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Exit", Left)) System.stop();
			}
			else if (menuCategory == 1) {
				if (ui.button("Undo", Left, Config.keymap.edit_undo)) History.undo();
				if (ui.button("Redo", Left, Config.keymap.edit_redo)) History.redo();
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Preferences...", Left, Config.keymap.edit_prefs)) BoxPreferences.show();
			}
			else if (menuCategory == 2) {
				if (ui.button("Reset", Left, Config.keymap.view_reset)) { ViewportUtil.resetViewport(); ViewportUtil.scaleToBounds(); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Front", Left, Config.keymap.view_front)) { ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0); }
				if (ui.button("Back", Left, Config.keymap.view_back)) { ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI); }
				if (ui.button("Right", Left, Config.keymap.view_right)) { ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2); }
				if (ui.button("Left", Left, Config.keymap.view_left)) { ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2); }
				if (ui.button("Top", Left, Config.keymap.view_top)) { ViewportUtil.setView(0, 0, 1, 0, 0, 0); }
				if (ui.button("Bottom", Left, Config.keymap.view_bottom)) { ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Orbit Left", Left, Config.keymap.view_orbit_left)) { ViewportUtil.orbit(-Math.PI / 12, 0); }
				if (ui.button("Orbit Right", Left, Config.keymap.view_orbit_right)) { ViewportUtil.orbit(Math.PI / 12, 0); }
				if (ui.button("Orbit Up", Left, Config.keymap.view_orbit_up)) { ViewportUtil.orbit(0, -Math.PI / 12); }
				if (ui.button("Orbit Down", Left, Config.keymap.view_orbit_down)) { ViewportUtil.orbit(0, Math.PI / 12); }
				if (ui.button("Orbit Opposite", Left, Config.keymap.view_orbit_opposite)) { ViewportUtil.orbit(Math.PI, 0); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Distract Free", Left, Config.keymap.view_distract_free)) {
					UITrait.inst.toggleDistractFree();
					UITrait.inst.ui.isHovered = false;
				}
				if (ui.button("Split View", Left)) {
					UITrait.inst.splitView = !UITrait.inst.splitView;
					App.resize();
				}

				// ui.button("Show Envmap", Left);
				// ui.button("Wireframe", Left);
			}
			else if (menuCategory == 3) {
				if (ui.button("Manual", Left)) {
					#if krom_windows
					Krom.sysCommand('explorer "https://armorpaint.org/manual"');
					#elseif krom_linux
					Krom.sysCommand('xdg-open "https://armorpaint.org/manual"');
					#else
					Krom.sysCommand('open "https://armorpaint.org/manual"');
					#end
				}
				if (ui.button("Issue Tracker", Left)) {
					#if krom_windows
					Krom.sysCommand('explorer "https://github.com/armory3d/armorpaint/issues"');
					#elseif krom_linux
					Krom.sysCommand('xdg-open "https://github.com/armory3d/armorpaint/issues"');
					#else
					Krom.sysCommand('open "https://github.com/armory3d/armorpaint/issues"');
					#end
				}
				if (ui.button("Report Bug", Left)) {
					var ver = App.version;
					var sha = Macro.buildSha();
					sha = sha.substr(1, sha.length - 2);
					var os = System.systemId;
					var url = "https://github.com/armory3d/armorpaint/issues/new?labels=bug&template=bug_report.md&body=*ArmorPaint%20" + ver + "-" + sha + ",%20" + os + "*";
					#if krom_windows
					Krom.sysCommand('explorer "$url"');
					#elseif krom_linux
					Krom.sysCommand('xdg-open $url');
					#else
					Krom.sysCommand('open $url');
					#end
				}
				// if (ui.button("Request Feature", Left)) {}
				if (ui.button("Check for Updates...", Left)) {
					// Retrieve latest version number
					var outFile = Krom.getFilesLocation() + '/' + Data.dataPath + "update.txt";
					var uri = "'https://luboslenco.gitlab.io/armorpaint/index.html'";
					#if krom_windows
					Krom.sysCommand('powershell -c "Invoke-WebRequest -Uri ' + uri + " -OutFile '" + outFile + "'");
					#else
					Krom.sysCommand('curl ' + uri + ' -o ' + outFile);
					#end
					// Compare versions
					Data.getBlob(outFile, function(blob:Blob) {
						var update = Json.parse(blob.toString());
						var updateVersion = Std.int(update.version);
						if (updateVersion > 0) {
							var date = Macro.buildDate().split(" ")[0].substr(2); // 2019 -> 19
							var dateInt = Std.parseInt(date.replace("-", ""));
							if (updateVersion > dateInt) {
								UIBox.showMessage("Update", "Update is available!\nPlease visit armorpaint.org to download.");
							}
							else {
								UIBox.showMessage("Update", "You are up to date!");
							}
						}
						Data.deleteBlob(outFile);
					});
				}
				if (ui.button("About...", Left)) {
					var sha = Macro.buildSha();
					sha = sha.substr(1, sha.length - 2);
					var date = Macro.buildDate().split(" ")[0];
					var gapi = #if (kha_direct3d11) "Direct3D11" #elseif (kha_direct3d12) "Direct3D12" #else "OpenGL" #end;
					var msg = "ArmorPaint.org - v" + App.version + " (" + date + ") - " + sha + "\n";
					msg += System.systemId + " - " + gapi;

					#if krom_windows
					var save = Krom.getFilesLocation() + "\\" + Data.dataPath + "gpu.txt";
					Krom.sysCommand('wmic path win32_VideoController get name' + ' > "' + save + '"');
					var bytes = haxe.io.Bytes.ofData(Krom.loadBlob(save));
					var gpu = "";
					for (i in 30...Std.int(bytes.length / 2)) {
						var c = String.fromCharCode(bytes.get(i * 2));
						if (c == '\n') continue;
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

		// Hide menu
		var first = showMenuFirst;
		showMenuFirst = false;
		if (first) {
			// App.uienabled = false;
		}
		else {
			if (ui.changed || ui.inputReleased || ui.inputReleasedR || ui.isEscapeDown) {
				if (propChanged) {
					propChanged = false;
				}
				else {
					show = false;
					App.redrawUI();
					showMenuFirst = true;
					menuCommands = null;
					// App.uienabled = true;
				}
			}
		}

		ui.t.BUTTON_COL = BUTTON_COL;
		ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
		ui.t.ELEMENT_H = ELEMENT_H;
		ui.endLayout();

		g.begin(false);
	}

	public static function draw(commands:Zui->Void = null, x = -1, y = -1) {
		var uibox = App.uibox;
		show = true;
		menuCommands = commands;
		menuX = x > -1 ? x : Std.int(Input.getMouse().x);
		menuY = y > -1 ? y : Std.int(Input.getMouse().y);
		var menuw = uibox.ELEMENT_W() * 1.5;
		if (menuX + menuw > System.windowWidth()) {
			menuX = Std.int(System.windowWidth() - menuw);
		}
	}
}
