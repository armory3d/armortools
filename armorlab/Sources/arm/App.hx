package arm;

import haxe.io.Bytes;
import kha.graphics2.truetype.StbTruetype;
import kha.Image;
import kha.Font;
import kha.System;
import zui.Zui;
import zui.Themes;
import zui.Nodes;
import iron.Scene;
import iron.data.Data;
import iron.system.Input;
import iron.system.Time;
import arm.ui.UISidebar;
import arm.ui.UINodes;
import arm.ui.UIMenu;
import arm.ui.UIBox;
import arm.ui.UIFiles;
import arm.ui.UIHeader;
import arm.ui.UIStatus;
import arm.ui.UIMenubar;
import arm.ui.TabSwatches;
import arm.ui.BoxExport;
import arm.io.ImportAsset;
import arm.io.ExportTexture;
import arm.sys.File;
import arm.sys.Path;
import arm.data.ConstData;
import arm.node.MakeMaterial;
import arm.Viewport;
import arm.Camera;
import arm.Enums;
import arm.ProjectFormat;
import arm.Res;

class App {

	public static var uiEnabled = true;
	public static var isDragging = false;
	public static var isResizing = false;
	public static var dragAsset: TAsset = null;
	public static var dragSwatch: TSwatchColor = null;
	public static var dragFile: String = null;
	public static var dragFileIcon: Image = null;
	public static var dragTint = 0xffffffff;
	public static var dragSize = -1;
	public static var dragRect: TRect = null;
	public static var dragOffX = 0.0;
	public static var dragOffY = 0.0;
	public static var dragStart = 0.0;
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font: Font = null;
	public static var theme: TTheme;
	public static var colorWheel: Image;
	public static var blackWhiteGradient: Image;
	public static var uiBox: Zui;
	public static var uiMenu: Zui;
	public static var defaultElementW = 100;
	public static var defaultElementH = 28;
	public static var defaultFontSize = 13;
	public static var resHandle = new Handle();
	static var dropPaths: Array<String> = [];
	static var appx = 0;
	static var appy = 0;
	static var lastWindowWidth = 0;
	static var lastWindowHeight = 0;

	public function new() {
		Console.init();
		lastWindowWidth = System.windowWidth();
		lastWindowHeight = System.windowHeight();

		#if arm_resizable
		iron.App.onResize = onResize;
		#end

		System.notifyOnDropFiles(function(dropPath: String) {
			#if krom_linux
			dropPath = untyped decodeURIComponent(dropPath);
			dropPaths = dropPath.split("file://");
			for (i in 0...dropPaths.length) dropPaths[i] = dropPaths[i].rtrim();
			#else
			dropPath = dropPath.rtrim();
			dropPaths.push(dropPath);
			#end
			// #if krom_ios
			// Import immediately while access to resource is unlocked
			// handleDropPaths();
			// #end
		});

		System.notifyOnApplicationState(
			function() { // Foreground
				Context.foregroundEvent = true;
				Context.lastPaintX = -1;
				Context.lastPaintY = -1;
			},
			function() {}, // Resume
			function() {}, // Pause
			function() { // Background
				// Release keys after alt-tab / win-tab
				@:privateAccess Input.getKeyboard().upListener(kha.input.KeyCode.Alt);
				@:privateAccess Input.getKeyboard().upListener(kha.input.KeyCode.Win);
			},
			function() { // Shutdown
				#if (krom_android || krom_ios)
				Project.projectSave();
				#end
			}
		);

		Krom.setSaveAndQuitCallback(saveAndQuitCallback);

		Data.getFont("font.ttf", function(f: Font) {
			Data.getImage("color_wheel.k", function(imageColorWheel: Image) {
				Data.getImage("black_white_gradient.k", function(imageBlackWhiteGradient: Image) {

					font = f;
					Config.loadTheme(Config.raw.theme, false);
					defaultElementW = theme.ELEMENT_W;
					defaultFontSize = theme.FONT_SIZE;
					Translator.loadTranslations(Config.raw.locale);
					UIFiles.filename = tr("untitled");
					#if (krom_android || krom_ios)
					kha.Window.get(0).title = tr("untitled");
					#end

					// Precompiled font for fast startup
					if (Config.raw.locale == "en") {
						var kimg: kha.Font.KravurImage = js.lib.Object.create(untyped kha.Font.KravurImage.prototype);
						@:privateAccess kimg.mySize = 13;
						@:privateAccess kimg.width = 128;
						@:privateAccess kimg.height = 128;
						@:privateAccess kimg.baseline = 10;
						var chars = new haxe.ds.Vector(ConstData.font_x0.length);
						kha.graphics2.Graphics.fontGlyphs = [for (i in 32...127) i];
						for (i in 0...95) chars[i] = new Stbtt_bakedchar();
						for (i in 0...ConstData.font_x0.length) chars[i].x0 = ConstData.font_x0[i];
						for (i in 0...ConstData.font_y0.length) chars[i].y0 = ConstData.font_y0[i];
						for (i in 0...ConstData.font_x1.length) chars[i].x1 = ConstData.font_x1[i];
						for (i in 0...ConstData.font_y1.length) chars[i].y1 = ConstData.font_y1[i];
						for (i in 0...ConstData.font_xoff.length) chars[i].xoff = ConstData.font_xoff[i];
						for (i in 0...ConstData.font_yoff.length) chars[i].yoff = ConstData.font_yoff[i];
						for (i in 0...ConstData.font_xadvance.length) chars[i].xadvance = ConstData.font_xadvance[i];
						@:privateAccess kimg.chars = chars;
						Data.getBlob("font13.bin", function(fontbin: kha.Blob) {
							@:privateAccess kimg.texture = Image.fromBytes(fontbin.toBytes(), 128, 128, kha.graphics4.TextureFormat.L8);
							@:privateAccess cast(font, kha.Font).images.set(130095, kimg);
						});
					}

					colorWheel = imageColorWheel;
					blackWhiteGradient = imageBlackWhiteGradient;
					Nodes.enumTexts = enumTexts;
					Nodes.tr = tr;
					uiBox = new Zui({ theme: App.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: colorWheel, black_white_gradient: blackWhiteGradient });
					uiMenu = new Zui({ theme: App.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: colorWheel, black_white_gradient: blackWhiteGradient });
					defaultElementH = uiMenu.t.ELEMENT_H;

					// Init plugins
					if (Config.raw.plugins != null) {
						for (plugin in Config.raw.plugins) {
							Plugin.start(plugin);
						}
					}

					Args.parse();

					iron.App.notifyOnUpdate(update);
					new UISidebar();
					new UINodes();
					new Camera();
					arm.node.brush.RandomNode.setSeed(Std.int(iron.system.Time.realTime() * 4294967295));
					iron.App.notifyOnUpdate(UINodes.inst.update);
					iron.App.notifyOnRender2D(UINodes.inst.render);
					iron.App.notifyOnUpdate(UISidebar.inst.update);
					iron.App.notifyOnRender2D(UISidebar.inst.render);
					iron.App.notifyOnRender2D(render);
					appx = 0;
					appy = UIHeader.inst.headerh * 2;
					var cam = Scene.active.camera;
					cam.data.raw.fov = Std.int(cam.data.raw.fov * 100) / 100;
					cam.buildProjection();

					Args.run();

					if (Config.raw.touch_ui) {
						if (Config.raw.recent_projects.length > 0) {
							arm.ui.BoxProjects.show();
						}
					}
				});
			});
		});
	}

	static function saveAndQuitCallback(save: Bool) {
		saveWindowRect();
		if (save) Project.projectSave(true);
		else System.stop();
	}

	public static function w(): Int {
		var res = 0;
		if (UINodes.inst == null) {
			res = System.windowWidth();
		}
		else if (UINodes.inst.show) {
			res = System.windowWidth() - Config.raw.layout[LayoutNodesW];
		}
		else { // Distract free
			res = System.windowWidth();
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h(): Int {
		var res = System.windowHeight();
		if (UISidebar.inst == null) {
			res -= UIHeader.defaultHeaderH * 2 + UIStatus.defaultStatusH;
		}
		else if (UISidebar.inst != null && res > 0) {
			var statush = Config.raw.layout[LayoutStatusH];
			res -= Std.int(UIHeader.defaultHeaderH * 2 * Config.raw.window_scale) + statush;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function x(): Int {
		return appx;
	}

	public static function y(): Int {
		return appy;
	}

	#if arm_resizable
	static function onResize() {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		var ratioW = System.windowWidth() / lastWindowWidth;
		lastWindowWidth = System.windowWidth();
		var ratioH = System.windowHeight() / lastWindowHeight;
		lastWindowHeight = System.windowHeight();

		Config.raw.layout[LayoutNodesW] = Std.int(Config.raw.layout[LayoutNodesW] * ratioW);

		resize();

		#if (krom_linux || krom_darwin)
		saveWindowRect();
		#end
	}
	#end

	static function saveWindowRect() {
		#if (krom_windows || krom_linux || krom_darwin)
		Config.raw.window_w = System.windowWidth();
		Config.raw.window_h = System.windowHeight();
		Config.raw.window_x = kha.Window.get(0).x;
		Config.raw.window_y = kha.Window.get(0).y;
		Config.save();
		#end
	}

	public static function resize() {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		var cam = Scene.active.camera;
		if (cam.data.raw.ortho != null) {
			cam.data.raw.ortho[2] = -2 * (iron.App.h() / iron.App.w());
			cam.data.raw.ortho[3] =  2 * (iron.App.h() / iron.App.w());
		}
		cam.buildProjection();

		if (Context.cameraType == CameraOrthographic) {
			Viewport.updateCameraType(Context.cameraType);
		}

		Context.ddirty = 2;

		if (UISidebar.inst.show) {
			appx = 0;
			appy = UIHeader.inst.headerh * 2;
		}
		else {
			appx = 0;
			appy = 0;
		}

		if (UINodes.inst.grid != null) {
			var _grid = UINodes.inst.grid;
			function _next() {
				_grid.unload();
			}
			App.notifyOnNextFrame(_next);
			UINodes.inst.grid = null;
		}

		redrawUI();
	}

	public static function redrawUI() {
		UIHeader.inst.headerHandle.redraws = 2;
		UIStatus.inst.statusHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		UIBox.hwnd.redraws = 2;
		if (Context.ddirty < 0) Context.ddirty = 0; // Redraw viewport
	}

	static function update() {
		var mouse = Input.getMouse();

		if (mouse.movementX != 0 || mouse.movementY != 0) {
			Krom.setMouseCursor(0); // Arrow
		}

		var hasDrag = dragAsset != null || dragFile != null || dragSwatch != null;

		if (Config.raw.touch_ui) {
			// Touch and hold to activate dragging
			if (dragStart < 0.2) {
				if (hasDrag && mouse.down()) dragStart += Time.realDelta;
				else dragStart = 0;
				hasDrag = false;
			}
			if (mouse.released()) {
				dragStart = 0;
			}
			var moved = Math.abs(mouse.movementX) > 1 && Math.abs(mouse.movementY) > 1;
			if ((mouse.released() || moved) && !hasDrag) {
				dragAsset = null;
				dragSwatch = null;
				dragFile = null;
				dragFileIcon = null;
				isDragging = false;
			}
			// Disable touch scrolling while dragging is active
			Zui.touchScroll = !isDragging;
		}

		if (hasDrag && (mouse.movementX != 0 || mouse.movementY != 0)) {
			isDragging = true;
		}
		if (mouse.released() && hasDrag) {
			var mx = mouse.x;
			var my = mouse.y;
			var inViewport = Context.paintVec.x < 1 && Context.paintVec.x > 0 &&
							 Context.paintVec.y < 1 && Context.paintVec.y > 0;
			var inNodes = UINodes.inst.show &&
						  mx > UINodes.inst.wx && mx < UINodes.inst.wx + UINodes.inst.ww &&
						  my > UINodes.inst.wy && my < UINodes.inst.wy + UINodes.inst.wh;
			var inSwatches = UIStatus.inst.statustab.position == 4 &&
						  mx > iron.App.x() && mx < iron.App.x() + System.windowWidth() &&
						  my > System.windowHeight() - Config.raw.layout[LayoutStatusH];
			if (dragAsset != null) {
				if (inNodes) { // Create image texture
					UINodes.inst.acceptAssetDrag(Project.assets.indexOf(dragAsset));
				}
				else if (inViewport) {
					if (dragAsset.file.toLowerCase().endsWith(".hdr")) {
						var image = Project.getImage(dragAsset);
						arm.io.ImportEnvmap.run(dragAsset.file, image);
					}
				}
				else if (inSwatches) {
					TabSwatches.acceptSwatchDrag(dragSwatch);
				}
				dragAsset = null;
			}
			else if (dragSwatch != null) {
				if (inNodes) { // Create RGB node
					UINodes.inst.acceptSwatchDrag(dragSwatch);
				}
				dragSwatch = null;
			}
			else if (dragFile != null) {
				var statush = Config.raw.layout[LayoutStatusH];
				var inBrowser =
					mx > iron.App.x() && mx < iron.App.x() + System.windowWidth() &&
					my > System.windowHeight() - statush;
				if (!inBrowser) {
					dropX = mouse.x;
					dropY = mouse.y;
					ImportAsset.run(dragFile, dropX, dropY);
				}
				dragFile = null;
				dragFileIcon = null;
			}
			Krom.setMouseCursor(0); // Arrow
			isDragging = false;
		}
		if (Context.colorPickerCallback != null && (mouse.released() || mouse.released("right"))) {
			Context.colorPickerCallback = null;
			Context.selectTool(Context.colorPickerPreviousTool);
		}

		handleDropPaths();

		var isPicker = Context.tool == ToolPicker;
		if (Zui.alwaysRedrawWindow && Context.ddirty < 0) Context.ddirty = 0;
	}

	static function handleDropPaths() {
		if (dropPaths.length > 0) {
			var mouse = Input.getMouse();
			#if (krom_linux || krom_darwin)
			var wait = !mouse.moved; // Mouse coords not updated during drag
			#else
			var wait = false;
			#end
			if (!wait) {
				dropX = mouse.x;
				dropY = mouse.y;
				var dropPath = dropPaths.shift();
				ImportAsset.run(dropPath, dropX, dropY);
			}
		}
	}

	static function getDragImage(): kha.Image {
		dragTint = 0xffffffff;
		dragSize = -1;
		dragRect = null;
		if (dragAsset != null) {
			return Project.getImage(dragAsset);
		}
		if (dragSwatch != null) {
			dragTint = dragSwatch.base;
			dragSize = 26;
			return TabSwatches.empty;
		}
		if (dragFile != null) {
			if (dragFileIcon != null) return dragFileIcon;
			var icons = Res.get("icons.k");
			dragRect = dragFile.indexOf(".") > 0 ? Res.tile50(icons, 3, 1) : Res.tile50(icons, 2, 1);
			dragTint = uiBox.t.HIGHLIGHT_COL;
			return icons;
		}
		return null;
	}

	static function render(g: kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Context.frame == 2) {
			MakeMaterial.parseMeshMaterial();
			MakeMaterial.parsePaintMaterial();
			Context.ddirty = 0;
			// Default workspace
			if (Config.raw.workspace != 0) {
				UIHeader.inst.worktab.position = Config.raw.workspace;
				UIMenubar.inst.workspaceHandle.redraws = 2;
				UIHeader.inst.worktab.changed = true;
			}
		}
		else if (Context.frame == 3) {
			Context.ddirty = 2;
		}
		Context.frame++;

		var mouse = Input.getMouse();
		if (isDragging) {
			Krom.setMouseCursor(1); // Hand
			var img = getDragImage();
			var size = (dragSize == -1 ? 50 : dragSize) * uiBox.ops.scaleFactor;
			var ratio = size / img.width;
			var h = img.height * ratio;
			var inv = 0;
			g.color = dragTint;
			dragRect == null ?
				g.drawScaledImage(img, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2) :
				g.drawScaledSubImage(img, dragRect.x, dragRect.y, dragRect.w, dragRect.h, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2);
			g.color = 0xffffffff;
		}

		var usingMenu = UIMenu.show && mouse.y > UIHeader.inst.headerh;
		uiEnabled = !UIBox.show && !usingMenu && !isComboSelected();
		if (UIBox.show) UIBox.render(g);
		if (UIMenu.show) UIMenu.render(g);

		// Save last pos for continuos paint
		Context.lastPaintVecX = Context.paintVec.x;
		Context.lastPaintVecY = Context.paintVec.y;

		#if (krom_android || krom_ios)
		// No mouse move events for touch, re-init last paint position on touch start
		if (!mouse.down()) {
			Context.lastPaintX = -1;
			Context.lastPaintY = -1;
		}
		#end
	}

	public static function enumTexts(nodeType: String): Array<String> {
		if (nodeType == "ImageTextureNode") {
			return Project.assetNames.length > 0 ? Project.assetNames : [""];
		}
		return null;
	}

	public static function getAssetIndex(fileName: String): Int {
		var i = Project.assetNames.indexOf(fileName);
		return i >= 0 ? i : 0;
	}

	public static function notifyOnNextFrame(f: Void->Void) {
		function _render(_) {
			iron.App.notifyOnInit(function() {
				function _update() {
					iron.App.notifyOnInit(f);
					iron.App.removeUpdate(_update);
				}
				iron.App.notifyOnUpdate(_update);
			});
			iron.App.removeRender(_render);
		}
		iron.App.notifyOnRender(_render);
	}

	public static function toggleFullscreen() {
		if (kha.Window.get(0).mode == kha.WindowMode.Windowed) {
			#if (krom_windows || krom_linux || krom_darwin)
			Config.raw.window_w = System.windowWidth();
			Config.raw.window_h = System.windowHeight();
			Config.raw.window_x = kha.Window.get(0).x;
			Config.raw.window_y = kha.Window.get(0).y;
			#end
			kha.Window.get(0).mode = kha.WindowMode.Fullscreen;
		}
		else {
			kha.Window.get(0).mode = kha.WindowMode.Windowed;
			kha.Window.get(0).resize(Config.raw.window_w, Config.raw.window_h);
			kha.Window.get(0).move(Config.raw.window_x, Config.raw.window_y);
		}
	}

	public static function isScrolling(): Bool {
		return UISidebar.inst.ui.isScrolling;
		for (ui in getUIs()) if (ui.isScrolling) return true;
		return false;
	}

	public static function isComboSelected(): Bool {
		for (ui in getUIs()) if (@:privateAccess ui.comboSelectedHandle != null) return true;
		return false;
	}

	public static function getUIs(): Array<Zui> {
		return [App.uiBox, App.uiMenu, UISidebar.inst.ui, arm.ui.UINodes.inst.ui];
	}

	public static function redrawStatus() {
		if (arm.ui.UIStatus.inst != null) {
			arm.ui.UIStatus.inst.statusHandle.redraws = 2;
		}
	}

	public static function redrawConsole() {
		if (arm.ui.UIStatus.inst != null) {
			arm.ui.UIStatus.inst.statusHandle.redraws = 2;
		}
	}

	public static function initLayout() {
		var show2d = (UINodes.inst != null && UINodes.inst.show);
		var raw = Config.raw;
		raw.layout = [
			#if krom_ios
			show2d ? Std.int((iron.App.w() + raw.layout[LayoutNodesW]) * 0.6) : Std.int(iron.App.w() * 0.6),
			#else
			show2d ? Std.int((iron.App.w() + raw.layout[LayoutNodesW]) / 2) : Std.int(iron.App.w() / 2),
			#end
			Std.int(UIStatus.defaultStatusH * raw.window_scale)
		];
	}

	public static function initConfig() {
		var raw = Config.raw;
		raw.recent_projects = [];
		raw.bookmarks = [];
		raw.plugins = [];
		raw.keymap = "default.json";
		raw.theme = "default.json";
		raw.server = "https://armorpaint.fra1.digitaloceanspaces.com";
		raw.undo_steps = 4;
		raw.pressure_radius = true;
		raw.pressure_sensitivity = 1.0;
		raw.camera_zoom_speed = 1.0;
		raw.camera_pan_speed = 1.0;
		raw.camera_rotation_speed = 1.0;
		raw.zoom_direction = ZoomVertical;
		raw.displace_strength = 0.0;
		raw.wrap_mouse = false;
		raw.workspace = Space2D;
		raw.layer_res = Res2048;
		raw.gpu_inference = true;
		#if (krom_android || krom_ios)
		raw.touch_ui = true;
		#else
		raw.touch_ui = false;
		#end
	}
}
