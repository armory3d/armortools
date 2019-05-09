package arm.ui;

import zui.*;
import arm.App;
import arm.ProjectFormat.TAPConfig;
import arm.util.ViewportUtil;

class UIMenu {

	public static var menuCategory = 0;
	static var showMenuFirst = true;
	static var menuX = 0;
	static var menuY = 0;
	static var menuCommands:Zui->Void = null;

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {
		var ui = App.uibox;

		var panelx = iron.App.x() - UITrait.inst.toolbarw;
		var C:TAPConfig = cast armory.data.Config.raw;
		if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - UITrait.inst.toolbarw;

		var menuButtonW = Std.int(ui.ELEMENT_W() * 0.5);
		var px = panelx + menuButtonW * menuCategory;
		var py = UITrait.inst.headerh;
		var menuItems = [5, 2, 13, 4];
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
				if (ui.button("New Project...", Left, 'Ctrl+N')) UIBox.newProject();
				if (ui.button("Open...", Left, 'Ctrl+O')) Project.projectOpen();
				if (ui.button("Save", Left, 'Ctrl+S')) Project.projectSave();
				if (ui.button("Save As...", Left, 'Ctrl+Shift+S')) Project.projectSaveAs();
				// ui.button("Import Asset...", Left);
				// ui.button("Export Textures...", Left);
				// ui.button("Export Mesh...", Left);
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Exit", Left)) { kha.System.stop(); }
			}
			else if (menuCategory == 1) {
				if (ui.button("Undo", Left, "Ctrl+Z")) UITrait.inst.doUndo();
				if (ui.button("Redo", Left, "Ctrl+Shift+Z")) UITrait.inst.doRedo();
				// ui.button("Preferences...", Left);
			}
			else if (menuCategory == 2) {
				if (ui.button("Reset", Left, "0")) { ViewportUtil.resetViewport(); ViewportUtil.scaleToBounds(); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Front", Left, "1")) { ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0); }
				if (ui.button("Back", Left, "Ctrl+1")) { ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI); }
				if (ui.button("Right", Left, "3")) { ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2); }
				if (ui.button("Left", Left, "Ctrl+3")) { ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2); }
				if (ui.button("Top", Left, "7")) { ViewportUtil.setView(0, 0, 1, 0, 0, 0); }
				if (ui.button("Bottom", Left, "Ctrl+7")) { ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Orbit Left", Left, "4")) { ViewportUtil.orbit(-Math.PI / 12, 0); }
				if (ui.button("Orbit Right", Left, "6")) { ViewportUtil.orbit(Math.PI / 12, 0); }
				if (ui.button("Orbit Up", Left, "8")) { ViewportUtil.orbit(0, -Math.PI / 12); }
				if (ui.button("Orbit Down", Left, "2")) { ViewportUtil.orbit(0, Math.PI / 12); }
				if (ui.button("Orbit Opposite", Left, "9")) { ViewportUtil.orbit(Math.PI, 0); }
				ui.fill(0, 0, sepw, 1, ui.t.ACCENT_SELECT_COL);
				if (ui.button("Distract Free", Left, "F11")) {
					UITrait.inst.show = !UITrait.inst.show;
					arm.App.resize();
					UITrait.inst.ui.isHovered = false;
				}

				// ui.button("Show Envmap", Left);
				// ui.button("Show Grid", Left);
				// ui.button("Wireframe", Left);
			}
			else if (menuCategory == 3) {
				if (ui.button("Manual", Left)) {
					#if kha_krom
					if (kha.System.systemId == "Windows") {
						Krom.sysCommand('explorer "https://armorpaint.org/manual"');
					}
					else if (kha.System.systemId == "Linux") {
						Krom.sysCommand('xdg-explorer "https://armorpaint.org/manual"');
					}
					else {
						Krom.sysCommand('open "https://armorpaint.org/manual"');
					}
					#end
				}
				if (ui.button("Report Bug", Left)) {
					#if kha_krom
					if (kha.System.systemId == "Windows") {
						Krom.sysCommand('explorer "https://github.com/armory3d/armorpaint/issues"');
					}
					else if (kha.System.systemId == "Linux") {
						Krom.sysCommand('xdg-explorer "https://github.com/armory3d/armorpaint/issues"');
					}
					else {
						Krom.sysCommand('open "https://github.com/armory3d/armorpaint/issues"');
					}
					#end
				}
				if (ui.button("Check for Updates...", Left)) {
					// Retrieve latest version number
					#if kha_krom
					var outFile = Krom.getFilesLocation() + '/' + iron.data.Data.dataPath + "update.txt";
					var uri = "'https://luboslenco.gitlab.io/armorpaint/index.html'";
					if (kha.System.systemId == "Windows") {
						Krom.sysCommand('powershell -c "Invoke-WebRequest -Uri ' + uri + " -OutFile '" + outFile + "'");
					}
					else {
						Krom.sysCommand('curl ' + uri + ' -o ' + outFile);
					}
					// Compare versions
					iron.data.Data.getBlob(outFile, function(blob:kha.Blob) {
						var update = haxe.Json.parse(blob.toString());
						var updateVersion = Std.int(update.version);
						if (updateVersion > 0) {
							var date = Macro.buildDate().split(" ")[0].substr(2); // 2019 -> 19
							var dateInt = Std.parseInt(StringTools.replace(date, "-", ""));
							if (updateVersion > dateInt) {
								UIBox.showMessage("Update is available!\nPlease visit armorpaint.org to download.");
							}
							else {
								UIBox.showMessage("You are up to date!");
							}
						}
					});
					#end
				}
				if (ui.button("About...", Left)) {
					var sha = Macro.buildSha();
					sha = sha.substr(1, sha.length - 2);
					var date = Macro.buildDate().split(" ")[0];
					var gapi = #if (kha_direct3d11) "Direct3D11" #else "OpenGL" #end;
					var msg = "ArmorPaint.org - v" + App.version + " (" + date + ") - git " + sha + "\n";
					msg += kha.System.systemId + " - " + gapi;
					UIBox.showMessage(msg);
				}
			}
		}

		// Hide menu
		var first = showMenuFirst;
		showMenuFirst = false;
		if (first) {
			// arm.App.uienabled = false;
		}
		else {
			if (ui.changed || ui.inputReleased || ui.inputReleasedR || ui.isEscapeDown) {
				App.showMenu = false;
				showMenuFirst = true;
				menuCommands = null;
				// arm.App.uienabled = true;
			}
		}

		ui.t.BUTTON_COL = BUTTON_COL;
		ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
		ui.t.ELEMENT_H = ELEMENT_H;
		ui.endLayout();

		g.begin(false);
	}

	public static function show(commands:Zui->Void = null) {
		var uibox = App.uibox;
		App.showMenu = true;
		menuCommands = commands;
		menuX = Std.int(iron.App.x() + iron.system.Input.getMouse().x);
		menuY = Std.int(iron.App.y() + iron.system.Input.getMouse().y);
		var menuw = uibox.ELEMENT_W() * 1.5;
		if (menuX + menuw > kha.System.windowWidth()) {
			menuX = Std.int(kha.System.windowWidth() - menuw);
		}
	}
}
