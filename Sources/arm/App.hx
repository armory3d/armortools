package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;

class App {

	public static var uienabled = true;
	public static var isDragging = false;
	public static var dragAsset:TAsset = null;
	public static var showFiles = false;
	public static var foldersOnly = false;
	public static var filesDone:String->Void;
	public static var dropPath = "";
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font:kha.Font = null;
	public static var theme:zui.Themes.TTheme;
	public static var color_wheel:kha.Image;
	static var uimodal:Zui;
	static var modalW = 625;
	static var modalH = 545;
	static var lastW = -1;
	static var lastH = -1;

	public static function getEnumTexts():Array<String> {
		return UITrait.inst.assetNames.length > 0 ? UITrait.inst.assetNames : [""];
	}

	public static function mapEnum(s:String):String {
		for (a in UITrait.inst.assets) if (a.name == s) return a.file;
		return "";
	}

	public function new() {

		kha.System.notifyOnDropFiles(function(filePath:String) {
			dropPath = StringTools.rtrim(filePath);
			var mouse = iron.system.Input.getMouse();
			dropX = mouse.x;
			dropY = mouse.y;
		});

		iron.data.Data.getFont("droid_sans.ttf", function(f:kha.Font) {
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
						#if arm_debug
						iron.Scene.active.sceneParent.getTrait(armory.trait.internal.DebugConsole).visible = false;
						#end
						iron.App.notifyOnUpdate(update);
						iron.Scene.active.root.addTrait(new UITrait());
						iron.Scene.active.root.addTrait(new UINodes());
						iron.Scene.active.root.addTrait(new UIView2D());
						iron.Scene.active.root.addTrait(new arm.trait.FlyCamera());
						iron.Scene.active.root.addTrait(new arm.trait.OrbitCamera());
						iron.App.notifyOnRender2D(render);
					});
				});
			});
		});
	}

	public static function w():Int {
		if (UITrait.inst != null && UITrait.inst.materialPreview) return 50;
		if (UITrait.inst != null && UITrait.inst.stickerPreview) return 512;
		
		var res = 0;
		if (UINodes.inst == null || UITrait.inst == null) {
			res = kha.System.windowWidth() - UITrait.defaultWindowW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = Std.int((kha.System.windowWidth() - UITrait.inst.windowW) / 2);
		}
		else {
			res = kha.System.windowWidth() - UITrait.inst.windowW;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h():Int {
		if (UITrait.inst != null && UITrait.inst.materialPreview) return 50;
		if (UITrait.inst != null && UITrait.inst.stickerPreview) return 512;

		var res = 0;
		res = kha.System.windowHeight();
		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	static var appx = 0;
	public static function x():Int {
		return appx;
	}

	public static function y():Int {
		return 0;
	}

	public static function realw():Int {
		return kha.System.windowWidth();
	}

	public static function realh():Int {
		return kha.System.windowHeight();
	}

	public static function resize() {
		iron.Scene.active.camera.buildProjection();
		UITrait.inst.dirty = 2;

		var lay = UITrait.inst.apconfig.ui_layout;
		appx = lay == 0 ? 0 : UITrait.inst.windowW;
		if (UINodes.inst.show) appx += UINodes.inst.ww;

		if (UINodes.inst.grid != null) {
			UINodes.inst.grid.unload();
			UINodes.inst.grid = null;
		}
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		isDragging = dragAsset != null;
		if (mouse.released() && isDragging) {
			if (UINodes.inst.show && mouse.x > UINodes.inst.wx && mouse.y > UINodes.inst.wy) {
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
			var p = dropPath.toLowerCase();
			if (StringTools.endsWith(p, ".obj") ||
				StringTools.endsWith(p, ".fbx") ||
				// StringTools.endsWith(p, ".blend") ||
				StringTools.endsWith(p, ".gltf")) {
				UITrait.inst.importMesh(dropPath);
			}
			else {
				UITrait.inst.importAsset(dropPath);
				// Place image node
				if (UINodes.inst.show && dropX > UINodes.inst.wx && dropX < UINodes.inst.wx + UINodes.inst.ww) {
					UINodes.inst.acceptDrag(UITrait.inst.assets.length - 1);
					UINodes.inst.nodes.nodeDrag = null;
					UINodes.inst.hwnd.redraws = 2;
				}
			}
			dropPath = "";
		}

		if (showFiles) updateFiles();
	}

	static function updateFiles() {
		var mouse = iron.system.Input.getMouse();
		if (mouse.released()) {
			var appw = kha.System.windowWidth();
			var apph = kha.System.windowHeight();
			var left = appw / 2 - modalW / 2;
			var right = appw / 2 + modalW / 2;
			var top = apph / 2 - modalH / 2;
			var bottom = apph / 2 + modalH / 2;
			if (mouse.x < left || mouse.x > right || mouse.y < top || mouse.y > bottom) {
				showFiles = false;
			}
		}
	}

	static function render(g:kha.graphics2.Graphics) {
		if (lastW >= 0 && arm.App.realw() > 0 && (lastW != arm.App.realw() || lastH != arm.App.realh())) {
			arm.App.resize();
		}
		lastW = arm.App.realw();
		lastH = arm.App.realh();

		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		uienabled = !showFiles;
		if (showFiles) renderFiles(g);
	}

	static function renderFiles(g:kha.graphics2.Graphics) {
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var left = Std.int(appw / 2 - modalW / 2);
		var right = Std.int(appw / 2 + modalW / 2);
		var top = Std.int(apph / 2 - modalH / 2);
		var bottom = Std.int(apph / 2 + modalH / 2);
		g.color = 0xff202020;
		g.fillRect(left, top, modalW, modalH);
		
		g.end();
		uimodal.begin(g);
		var path = '/';
		var pathHandle = Id.handle();
		var whandle = Id.handle();
		if (uimodal.window(whandle, left, top, modalW, modalH - 50, true)) {
			pathHandle.text = uimodal.textInput(pathHandle);
			path = zui.Ext.fileBrowser(uimodal, pathHandle, foldersOnly);
		}
		uimodal.end(false);
		g.begin(false);

		if (UITrait.checkImageFormat(path) || UITrait.checkMeshFormat(path)) {
			showFiles = false;
			filesDone(path);
			var sep = kha.System.systemId == "Windows" ? "\\" : "/";
			pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(sep));
			whandle.redraws = 2;
			UITrait.inst.dirty = 2;
		}

		uimodal.beginLayout(g, right - 100, bottom - 30, 100);
		if (uimodal.button("OK")) {
			showFiles = false;
			filesDone(path);
			UITrait.inst.dirty = 2;
		}
		uimodal.endLayout(false);

		uimodal.beginLayout(g, right - 200, bottom - 30, 100);
		if (uimodal.button("Cancel")) {
			showFiles = false;
			UITrait.inst.dirty = 2;
		}
		uimodal.endLayout();

		g.begin(false);
	}
}
