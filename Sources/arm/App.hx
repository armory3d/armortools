package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;
import arm.ui.*;
import arm.util.*;

class App extends iron.Trait {

	public static var version = "0.6";
	public static function x():Int { return appx; }
	public static function y():Int { return appy; }
	static var appx = 0;
	static var appy = 0;
	public static var uienabled = true;
	public static var isDragging = false;
	public static var dragAsset:TAsset = null;
	public static var showFiles = false;
	public static var showBox = false;
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
	public static var uibox:Zui;
	public static var path = '/';
	public static var showMenu = false;

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
			dropPath = filePath;
			dropPath = StringTools.replace(dropPath, "%20", " "); // Linux can pass %20 on drop
			dropPath = dropPath.split("file://")[0]; // Multiple files dropped on Linux, take first
			dropPath = StringTools.rtrim(dropPath);
		});

		iron.data.Data.getFont("font_default.ttf", function(f:kha.Font) {
			iron.data.Data.getBlob("theme_dark.arm", function(b:kha.Blob) {
				iron.data.Data.getImage('color_wheel.png', function(image:kha.Image) {
					parseTheme(b);
					font = f;
					color_wheel = image;
					zui.Nodes.getEnumTexts = getEnumTexts;
					zui.Nodes.mapEnum = mapEnum;
					uibox = new Zui({ font: f, scaleFactor: armory.data.Config.raw.window_scale });
					
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
						iron.App.notifyOnRender2D(@:privateAccess UINodes.inst.render);

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

	public static function parseTheme(b:kha.Blob) {
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
	}

	public static function w():Int {
		// Draw material preview
		if (UITrait.inst != null && UITrait.inst.materialPreview) return arm.util.RenderUtil.matPreviewSize;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return arm.util.RenderUtil.decalPreviewSize;
		
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
		if (UITrait.inst != null && UITrait.inst.materialPreview) return arm.util.RenderUtil.matPreviewSize;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return arm.util.RenderUtil.decalPreviewSize;

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

		var cam = iron.Scene.active.camera;
		if (cam.data.raw.ortho != null) {
			cam.data.raw.ortho[2] = -2 * (iron.App.h() / iron.App.w());
			cam.data.raw.ortho[3] =  2 * (iron.App.h() / iron.App.w());
		}
		cam.buildProjection();
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
			var wait = kha.System.systemId == "Linux" && !mouse.moved; // Mouse coords not updated on Linux during drag
			if (!wait) {
				dropX = mouse.x + App.x();
				dropY = mouse.y + App.y();
				Importer.importFile(dropPath, dropX, dropY);
				dropPath = "";
			}
		}

		if (showFiles || showBox) UIBox.update();
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
		if (showFiles) UIFiles.render(g);
		else if (showBox) UIBox.render(g);
		else if (showMenu) UIMenu.render(g);
	}

	public static function getEnumTexts():Array<String> {
		return UITrait.inst.assetNames.length > 0 ? UITrait.inst.assetNames : [""];
	}

	public static function mapEnum(s:String):String {
		for (a in UITrait.inst.assets) if (a.name == s) return a.file;
		return "";
	}

	public static function getAssetIndex(f:String):Int {
		for (i in 0...UITrait.inst.assets.length) {
			if (UITrait.inst.assets[i].file == f) {
				return i;
			}
		}
		return 0;
	}
}
