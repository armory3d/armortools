package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;
import arm.ui.*;
import arm.util.*;
import arm.ProjectFormat.TAPConfig;

class App extends iron.Trait {

	public static var version = "0.6";

	public static function x():Int { return appx; }
	public static function y():Int { return appy; }

	public static var uienabled = true;
	public static var isDragging = false;
	public static var dragAsset:TAsset = null;
	public static var showFiles = false;
	public static var showBox = false;
	public static var boxText = "";
	public static var foldersOnly = false;
	public static var showFilename = false;
	public static var whandle = new Zui.Handle();
	public static var filenameHandle = new Zui.Handle({text: "untitled"});
	public static var filesDone:String->Void;
	public static var dropPath = "";
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font:kha.Font = null;
	public static var theme:zui.Themes.TTheme;
	public static var color_wheel:kha.Image;
	public static var uimodal:Zui;
	public static var path = '/';
	static var modalW = 625;
	static var modalH = 545;
	static var appx = 0;
	static var appy = 0;

	public static var showMenu = false;
	public static var menuCategory = 0;
	static var showMenuFirst = true;

	public static function getEnumTexts():Array<String> {
		return UITrait.inst.assetNames.length > 0 ? UITrait.inst.assetNames : [""];
	}

	public static function mapEnum(s:String):String {
		for (a in UITrait.inst.assets) if (a.name == s) return a.file;
		return "";
	}

	public function new() {
		super();

		// Restore default config
		if (!armory.data.Config.configLoaded) {
			var C = armory.data.Config.raw;
			C.rp_bloom = true;
			C.rp_gi = false;
		    C.rp_motionblur = false;
		    C.rp_shadowmap_cube = 0;
		    C.rp_shadowmap_cascade = 0;
		    C.rp_ssgi = true;
		    C.rp_ssr = false;
		    C.rp_supersample = 1.0;
		}

		#if arm_resizable
		iron.App.onResize = onResize;
		#end

		// Set base dir for file browser
		zui.Ext.dataPath = iron.data.Data.dataPath;

		kha.System.notifyOnDropFiles(function(filePath:String) {
			dropPath = StringTools.rtrim(filePath);
			dropPath = StringTools.replace(dropPath, "%20", " "); // Linux can pass %20 on drop
			var mouse = iron.system.Input.getMouse();
			dropX = mouse.x;
			dropY = mouse.y;
		});

		iron.data.Data.getFont("font_default.ttf", function(f:kha.Font) {
			iron.data.Data.getBlob("theme.arm", function(b:kha.Blob) {
				iron.data.Data.getImage('color_wheel.png', function(image:kha.Image) {
					theme = haxe.Json.parse(b.toString());
					theme.WINDOW_BG_COL = Std.parseInt(cast theme.WINDOW_BG_COL);
					theme.WINDOW_TINT_COL = Std.parseInt(cast theme.WINDOW_TINT_COL);
					theme.ACCENT_COL = Std.parseInt(cast theme.ACCENT_COL);
					theme.ACCENT_HOVER_COL = Std.parseInt(cast theme.ACCENT_HOVER_COL);
					theme.ACCENT_SELECT_COL = Std.parseInt(cast theme.ACCENT_SELECT_COL);
					theme.PANEL_BG_COL = Std.parseInt(cast theme.PANEL_BG_COL);
					theme.PANEL_TEXT_COL = Std.parseInt(cast theme.PANEL_TEXT_COL);
					theme.BUTTON_COL = Std.parseInt(cast theme.BUTTON_COL);
					theme.BUTTON_TEXT_COL = Std.parseInt(cast theme.BUTTON_TEXT_COL);
					theme.BUTTON_HOVER_COL = Std.parseInt(cast theme.BUTTON_HOVER_COL);
					theme.BUTTON_PRESSED_COL = Std.parseInt(cast theme.BUTTON_PRESSED_COL);
					theme.TEXT_COL = Std.parseInt(cast theme.TEXT_COL);
					theme.LABEL_COL = Std.parseInt(cast theme.LABEL_COL);
					theme.ARROW_COL = Std.parseInt(cast theme.ARROW_COL);
					theme.SEPARATOR_COL = Std.parseInt(cast theme.SEPARATOR_COL);
					font = f;
					color_wheel = image;
					zui.Nodes.getEnumTexts = getEnumTexts;
					zui.Nodes.mapEnum = mapEnum;
					uimodal = new Zui({ font: f, scaleFactor: armory.data.Config.raw.window_scale });
					
					iron.App.notifyOnInit(function() {
						// #if arm_debug
						// iron.Scene.active.sceneParent.getTrait(armory.trait.internal.DebugConsole).visible = false;
						// #end
						iron.App.notifyOnUpdate(update);
						var root = iron.Scene.active.root;
						root.addTrait(new UITrait());
						root.addTrait(new UINodes());
						root.addTrait(new UIView2D());
						root.addTrait(new arm.trait.FlyCamera());
						root.addTrait(new arm.trait.OrbitCamera());
						root.addTrait(new arm.trait.RotateCamera());

						iron.App.notifyOnRender2D(@:privateAccess UITrait.inst.renderCursor);

						iron.App.notifyOnUpdate(@:privateAccess UINodes.inst.update);
						iron.App.notifyOnRender2D(@:privateAccess UINodes.inst.render2D);

						iron.App.notifyOnUpdate(@:privateAccess UITrait.inst.update);
						iron.App.notifyOnRender2D(@:privateAccess UITrait.inst.render);

						iron.App.notifyOnRender2D(render);
						
						appx = UITrait.inst.C.ui_layout == 0 ? UITrait.inst.toolbarw : UITrait.inst.windowW + UITrait.inst.toolbarw;
						appy = UITrait.inst.headerh * 2;
					});
				});
			});
		});
	}

	public static function w():Int {
		// Draw material preview
		if (UITrait.inst != null && UITrait.inst.materialPreview) return 100;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return 512;
		
		var res = 0;
		if (UINodes.inst == null || UITrait.inst == null) {
			res = kha.System.windowWidth() - UITrait.defaultWindowW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = Std.int((kha.System.windowWidth() - UITrait.inst.windowW) / 2);
			res -= UITrait.inst.toolbarw;
		}
		else if (UITrait.inst.show) {
			res = kha.System.windowWidth() - UITrait.inst.windowW;
			res -= UITrait.inst.toolbarw;
		}
		else {
			res = kha.System.windowWidth();
		}
		
		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h():Int {
		// Draw material preview
		if (UITrait.inst != null && UITrait.inst.materialPreview) return 100;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return 512;

		var res = 0;
		res = kha.System.windowHeight();
		if (UITrait.inst != null && UITrait.inst.show && res > 0) res -= UITrait.inst.headerh * 3;
		
		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	#if arm_resizable
	static function onResize() {
		resize();
		
		// Save window size
		// UITrait.inst.C.window_w = kha.System.windowWidth();
		// UITrait.inst.C.window_h = kha.System.windowHeight();
		// Cap height, window is not centered properly
		// var disp =  kha.Display.primary;
		// if (disp.height > 0 && UITrait.inst.C.window_h > disp.height - 140) {
		// 	UITrait.inst.C.window_h = disp.height - 140;
		// }
		// armory.data.Config.save();
	}
	#end

	public static function resize() {
		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		iron.Scene.active.camera.buildProjection();
		UITrait.inst.ddirty = 2;

		var lay = UITrait.inst.C.ui_layout;
		
		appx = lay == 0 ? UITrait.inst.toolbarw : UITrait.inst.windowW + UITrait.inst.toolbarw;
		if (lay == 1 && (UINodes.inst.show || UIView2D.inst.show)) {
			appx += iron.App.w() + UITrait.inst.toolbarw;
		}

		appy = UITrait.inst.headerh * 2;

		if (!UITrait.inst.show) {
			appx = 0;
			appy = 0;
		}

		if (UINodes.inst.grid != null) {
			UINodes.inst.grid.unload();
			UINodes.inst.grid = null;
		}
	}

	public static function getAssetIndex(f:String):Int {
		for (i in 0...UITrait.inst.assets.length) {
			if (UITrait.inst.assets[i].file == f) {
				return i;
			}
		}
		return 0;
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		isDragging = dragAsset != null;
		if (mouse.released() && isDragging) {
			var x = mouse.x + iron.App.x();
			var y = mouse.y + iron.App.y();
			if (UINodes.inst.show && x > UINodes.inst.wx && y > UINodes.inst.wy) {
				var index = 0;
				for (i in 0...UITrait.inst.assets.length) {
					if (UITrait.inst.assets[i] == dragAsset) {
						index = i;
						break;
					}
				}
				UINodes.inst.acceptDrag(index);
			}
			dragAsset = null;
		}

		if (dropPath != "") {
			Importer.importFile(dropPath, dropX, dropY);
			dropPath = "";
		}

		if (showFiles || showBox) updateModal();
	}

	static function updateModal() {
		var mouse = iron.system.Input.getMouse();
		if (mouse.released()) {
			var appw = kha.System.windowWidth();
			var apph = kha.System.windowHeight();
			var left = appw / 2 - modalW / 2;
			var right = appw / 2 + modalW / 2;
			var top = apph / 2 - modalH / 2;
			var bottom = apph / 2 + modalH / 2;
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
			if (mx < left || mx > right || my < top || my > bottom) {
				showFiles = showBox = false;
			}
		}
	}

	static function render(g:kha.graphics2.Graphics) {
		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		var mouse = iron.system.Input.getMouse();
		if (arm.App.dragAsset != null) {
			var img = UITrait.inst.getImage(arm.App.dragAsset);
			var ratio = 128 / img.width;
			var h = img.height * ratio;
			g.drawScaledImage(img, mouse.x + iron.App.x(), mouse.y + iron.App.y(), 128, h);
		}

		var usingMenu = false;
		if (showMenu) usingMenu = mouse.y + App.y() > UITrait.inst.headerh;

		uienabled = !showFiles && !showBox && !usingMenu;
		if (showFiles) renderFiles(g);
		else if (showBox) renderBox(g);
		else if (showMenu) renderMenu(g);
	}

	@:access(zui.Zui)
	static function renderFiles(g:kha.graphics2.Graphics) {
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var left = Std.int(appw / 2 - modalW / 2);
		var right = Std.int(appw / 2 + modalW / 2);
		var top = Std.int(apph / 2 - modalH / 2);
		var bottom = Std.int(apph / 2 + modalH / 2);
		
		// g.color = 0x44000000;
		// g.fillRect(0, 0, appw, apph);

		g.color = uimodal.t.SEPARATOR_COL;
		g.fillRect(left, top, modalW, modalH);
		
		g.end();
		uimodal.begin(g);
		var pathHandle = Id.handle();
		if (uimodal.window(whandle, left, top, modalW, modalH - 50)) {
			pathHandle.text = uimodal.textInput(pathHandle, "Path");
			if (showFilename) uimodal.textInput(filenameHandle, "File");
			path = zui.Ext.fileBrowser(uimodal, pathHandle, foldersOnly);
		}
		uimodal.end(false);
		g.begin(false);

		if (Format.checkTextureFormat(path) || Format.checkMeshFormat(path) || Format.checkProjectFormat(path)) {
			showFiles = false;
			filesDone(path);
			var sep = kha.System.systemId == "Windows" ? "\\" : "/";
			pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(sep));
			whandle.redraws = 2;
			UITrait.inst.ddirty = 2;
		}

		g.end();
		uimodal.beginLayout(g, right - Std.int(uimodal.ELEMENT_W()), bottom - Std.int(uimodal.ELEMENT_H() * 1.2), Std.int(uimodal.ELEMENT_W()));
		if (uimodal.button("OK")) {
			showFiles = false;
			filesDone(path);
			UITrait.inst.ddirty = 2;
		}
		uimodal.endLayout(false);

		uimodal.beginLayout(g, right - Std.int(uimodal.ELEMENT_W() * 2), bottom - Std.int(uimodal.ELEMENT_H() * 1.2), Std.int(uimodal.ELEMENT_W()));
		if (uimodal.button("Cancel")) {
			showFiles = false;
			UITrait.inst.ddirty = 2;
		}
		uimodal.endLayout();

		g.begin(false);
	}

	@:access(zui.Zui)
	static function renderBox(g:kha.graphics2.Graphics) {
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var modalW = 300;
		var modalH = 100;
		var left = Std.int(appw / 2 - modalW / 2);
		var right = Std.int(appw / 2 + modalW / 2);
		var top = Std.int(apph / 2 - modalH / 2);
		var bottom = Std.int(apph / 2 + modalH / 2);
		
		g.color = uimodal.t.SEPARATOR_COL;
		g.fillRect(left, top, modalW, modalH);
		
		g.end();
		uimodal.begin(g);
		if (uimodal.window(whandle, left, top, modalW, modalH)) {
			uimodal._y += 20;
			for (line in boxText.split("\n")) {
				uimodal.text(line);
			}
		}
		uimodal.end(false);

		uimodal.beginLayout(g, right - Std.int(uimodal.ELEMENT_W()), bottom - Std.int(uimodal.ELEMENT_H() * 1.2), Std.int(uimodal.ELEMENT_W()));
		if (uimodal.button("OK")) {
			showBox = false;
		}
		uimodal.endLayout(false);

		g.begin(false);
	}

	public static function showMessageBox(text:String) {
		showBox = true;
		boxText = text;
	}

	@:access(zui.Zui)
	static function renderMenu(g:kha.graphics2.Graphics) {
		
		var ui = uimodal;

		var panelx = iron.App.x() - UITrait.inst.toolbarw;
		var C:TAPConfig = cast armory.data.Config.raw;
		if (C.ui_layout == 1 && (UINodes.inst.show || UIView2D.inst.show)) panelx = panelx - App.w() - UITrait.inst.toolbarw;

		var menuButtonW = Std.int(ui.ELEMENT_W() * 0.5);
		var px = panelx + menuButtonW * menuCategory;
		var py = UITrait.inst.headerh;
		var menuItems = [5, 2, 8, 2];
		var ph = 24 * menuItems[menuCategory] * ui.SCALE;
		
		g.color = ui.t.SEPARATOR_COL;
		var menuw = Std.int(ui.ELEMENT_W() * 1.5);
		g.fillRect(px, py, menuw, ph);

		g.end();

		ui.beginLayout(g, Std.int(px), Std.int(py), menuw);
		var BUTTON_COL = ui.t.BUTTON_COL;
		ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

		var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
		ui.t.ELEMENT_OFFSET = 0;
		var ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Std.int(24 * ui.SCALE);

		ui.changed = false;

		if (menuCategory == 0) {
			if (ui.button("New Project", Left)) { Project.projectNew(); ViewportUtil.scaleToBounds(); }
			if (ui.button("Open...", Left, 'Ctrl+O')) Project.projectOpen();
			if (ui.button("Save", Left, 'Ctrl+S')) Project.projectSave();
			if (ui.button("Save As...", Left, 'Ctrl+Shift+S')) Project.projectSaveAs();
			// ui.button("Import Asset...", Left);
			// ui.button("Export Textures...", Left);
			// ui.button("Export Mesh...", Left);
			ui.fill(0, 0, menuw, 1, ui.t.ACCENT_SELECT_COL);
			if (ui.button("Exit", Left)) { kha.System.stop(); }
		}
		else if (menuCategory == 1) {
			if (ui.button("Undo", Left, "Ctrl+Z")) UITrait.inst.doUndo();
			if (ui.button("Redo", Left, "Ctrl+Shift+Z")) UITrait.inst.doRedo();
			// ui.button("Preferences...", Left);
		}
		else if (menuCategory == 2) {

			if (ui.button("Reset", Left, "0")) { ViewportUtil.resetViewport(); ViewportUtil.scaleToBounds(); }
			if (ui.button("Front", Left, "1")) { ViewportUtil.setView(0, -3, 0, Math.PI / 2, 0, 0); }
			if (ui.button("Back", Left, "Ctrl+1")) { ViewportUtil.setView(0, 3, 0, Math.PI / 2, 0, Math.PI); }
			if (ui.button("Right", Left, "3")) { ViewportUtil.setView(3, 0, 0, Math.PI / 2, 0, Math.PI / 2); }
			if (ui.button("Left", Left, "Ctrl+3")) { ViewportUtil.setView(-3, 0, 0, Math.PI / 2, 0, -Math.PI / 2); }
			if (ui.button("Top", Left, "7")) { ViewportUtil.setView(0, 0, 3, 0, 0, 0); }
			if (ui.button("Bottom", Left, "Ctrl+7")) { ViewportUtil.setView(0, 0, -3, Math.PI, 0, Math.PI); }
			ui.fill(0, 0, menuw, 1, ui.t.ACCENT_SELECT_COL);
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
			// ui.button("Manual...", Left);
			if (ui.button("Check for Updates...", Left)) {
				// Retrieve latest version number
				var outFile = Krom.getFilesLocation() + '/' + iron.data.Data.dataPath + "update.txt";
				var uri = "'https://luboslenco.gitlab.io/armorpaint/index.html'";
				#if kha_krom
				if (kha.System.systemId == "Windows") {
					Krom.sysCommand('powershell -c "Invoke-WebRequest -Uri ' + uri + " -OutFile '" + outFile + "'");
				}
				// Compare versions
				iron.data.Data.getBlob(outFile, function(blob:kha.Blob) {
					var update = haxe.Json.parse(blob.toString());
					var updateVersion = Std.int(update.version);
					if (updateVersion > 0) {
						var date = Macro.buildDate().split(" ")[0];
						var dateInt = Std.parseInt(StringTools.replace(date, "-", ""));
						if (updateVersion > dateInt) {
							arm.App.showMessageBox("Update is available!");
						}
						else {
							arm.App.showMessageBox("You are up to date!");
						}
					}
				});
				#end
			}
			if (ui.button("About...", Left)) {
				var sha = Macro.buildSha();
				var date = Macro.buildDate().split(" ")[0];
				var gapi = #if (kha_direct3d11) "Direct3D11" #else "OpenGL" #end;
				var renderer = #if (rp_renderer == "Deferred") "Deferred" #else "Forward" #end;
				var msg = "ArmorPaint.org - v" + version + " (" + date + ") - " + sha + "\n";
				msg += kha.System.systemId + " - " + gapi + " - " + renderer;
				arm.App.showMessageBox(msg);
			}
		}

		// Hide menu
		var first = showMenuFirst;
		showMenuFirst = false;
		if (first) {
			// arm.App.uienabled = false;
		}
		else {
			if (ui.changed || ui.inputReleased || ui.isEscapeDown) {
				showMenu = false;
				showMenuFirst = true;
				// arm.App.uienabled = true;
			}
		}

		ui.t.BUTTON_COL = BUTTON_COL;
		ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
		ui.t.ELEMENT_H = ELEMENT_H;
		ui.endLayout();

		g.begin(false);
	}
}
