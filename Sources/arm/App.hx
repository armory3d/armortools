package arm;

import kha.graphics2.truetype.StbTruetype;
import kha.Image;
import kha.Font;
import kha.System;
import zui.Zui;
import zui.Zui.Handle;
import zui.Themes;
import zui.Nodes;
import iron.Scene;
import iron.data.Data;
import iron.system.Input;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIMenu;
import arm.ui.UIBox;
import arm.ui.UIFiles;
import arm.io.Importer;
import arm.util.Path;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.data.MaterialSlot;
import arm.data.ConstData;
import arm.plugin.Camera;
import arm.Config;
import arm.Tool;
import arm.Project;
using StringTools;

class App {

	public static var version = "0.7";
	static var appx = 0;
	static var appy = 0;
	static var winw = 0;
	static var winh = 0;
	public static var uienabled = true;
	public static var isDragging = false;
	public static var isResizing = false;
	public static var dragMaterial:MaterialSlot = null;
	public static var dragAsset:TAsset = null;
	public static var dragOffX = 0.0;
	public static var dragOffY = 0.0;
	public static var dropPath = "";
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font:Font = null;
	public static var theme:TTheme;
	public static var color_wheel:Image;
	public static var uibox:Zui;
	public static var fileArg = "";
	public static var saveAndQuit = false;

	public function new() {
		Config.init();
		winw = System.windowWidth();
		winh = System.windowHeight();

		#if arm_resizable
		iron.App.onResize = onResize;
		#end

		System.notifyOnDropFiles(function(filePath:String) {
			#if krom_windows
			if (!checkAscii(filePath)) filePath = shortPath(filePath);
			#end
			dropPath = filePath;
			#if krom_linux
			dropPath = untyped decodeURIComponent(dropPath);
			dropPath = dropPath.split("file://")[0]; // Multiple files dropped, take first
			#end
			dropPath = dropPath.rtrim();
		});

		System.notifyOnApplicationState(
			// Release alt after alt-tab
			function(){ @:privateAccess Input.getKeyboard().upListener(kha.input.KeyCode.Alt); }, // Foreground
			function(){}, // Resume
			function(){}, // Pause
			function(){}, // Background
			function(){} // Shutdown
		);

		#if krom_windows
		Krom.setSaveAndQuitCallback(saveAndQuitCallback);
		#end

		Data.getFont("font_default.ttf", function(f:Font) {
			Data.getImage('color_wheel.png', function(image:Image) {
				font = f;
				theme = zui.Themes.dark;
				theme.FILL_WINDOW_BG = true;

				var kimg:kha.Kravur.KravurImage = js.lib.Object.create(untyped kha.Kravur.KravurImage.prototype);
				@:privateAccess kimg.mySize = 13;
				@:privateAccess kimg.width = 128;
				@:privateAccess kimg.height = 128;
				@:privateAccess kimg.baseline = 10;
				var chars = new haxe.ds.Vector(ConstData.font_x0.length);
				// kha.graphics2.Graphics.fontGlyphs = [for (i in 32...127) i];
				kha.graphics2.Graphics.fontGlyphs = [for (i in 32...206) i]; // Fix tiny font
				// for (i in 0...ConstData.font_x0.length) chars[i] = new Stbtt_bakedchar();
				for (i in 0...174) chars[i] = new Stbtt_bakedchar();
				for (i in 0...ConstData.font_x0.length) chars[i].x0 = ConstData.font_x0[i];
				for (i in 0...ConstData.font_y0.length) chars[i].y0 = ConstData.font_y0[i];
				for (i in 0...ConstData.font_x1.length) chars[i].x1 = ConstData.font_x1[i];
				for (i in 0...ConstData.font_y1.length) chars[i].y1 = ConstData.font_y1[i];
				for (i in 0...ConstData.font_xoff.length) chars[i].xoff = ConstData.font_xoff[i];
				for (i in 0...ConstData.font_yoff.length) chars[i].yoff = ConstData.font_yoff[i];
				for (i in 0...ConstData.font_xadvance.length) chars[i].xadvance = ConstData.font_xadvance[i];
				@:privateAccess kimg.chars = chars;
				Data.getBlob("font13.bin", function(fontbin:kha.Blob) {
					@:privateAccess kimg.texture = Image.fromBytes(fontbin.toBytes(), 128, 128, kha.graphics4.TextureFormat.L8);
					// @:privateAccess cast(font, kha.Kravur).images.set(130095, kimg);
					@:privateAccess cast(font, kha.Kravur).images.set(130174, kimg);
				});

				color_wheel = image;
				Nodes.getEnumTexts = getEnumTexts;
				Nodes.mapEnum = mapEnum;
				uibox = new Zui({ font: f, scaleFactor: Config.raw.window_scale });

				// File to open passed as argument
				if (Krom.getArgCount() > 1) {
					var path = Krom.getArg(1);
					if (Path.checkProjectFormat(path) ||
						Path.checkMeshFormat(path) ||
						Path.checkTextureFormat(path) ||
						Path.checkFontFormat(path)) {
						fileArg = path;
					}
				}
				iron.App.notifyOnUpdate(update);
				var root = Scene.active.root;
				new UITrait();
				new UINodes();
				new UIView2D();
				new Camera();
				iron.App.notifyOnRender2D(@:privateAccess UITrait.inst.renderCursor);
				iron.App.notifyOnUpdate(@:privateAccess UINodes.inst.update);
				iron.App.notifyOnRender2D(@:privateAccess UINodes.inst.render);
				iron.App.notifyOnUpdate(@:privateAccess UITrait.inst.update);
				iron.App.notifyOnRender2D(@:privateAccess UITrait.inst.render);
				iron.App.notifyOnRender2D(render);
				appx = UITrait.inst.toolbarw;
				appy = UITrait.inst.headerh * 2;
				var cam = Scene.active.camera;
				cam.data.raw.fov = Std.int(cam.data.raw.fov * 100) / 100;
				cam.buildProjection();
				#if arm_creator
				Project.projectNew(); // Spawn terrain plane
				#end
				if (fileArg != "") {
					Importer.importFile(fileArg);
					if (Path.checkMeshFormat(fileArg)) {
						UITrait.inst.toggleDistractFree();
					}
					else if (Path.checkTextureFormat(fileArg)) {
						UITrait.inst.show2DView(1);
					}
				}
			});
		});
	}

	static function saveAndQuitCallback(save:Bool) {
		saveWindowRect();
		if (save) {
			saveAndQuit = true;
			Project.projectSave();
		}
		else System.stop();
	}

	public static function w():Int {
		// Draw material preview
		if (UITrait.inst != null && UITrait.inst.materialPreview) return RenderUtil.matPreviewSize;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return RenderUtil.decalPreviewSize;

		var res = 0;
		if (UINodes.inst == null || UITrait.inst == null) {
			res = System.windowWidth() - UITrait.defaultWindowW - UITrait.defaultToolbarW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = System.windowWidth() - UITrait.inst.windowW - UINodes.inst.defaultWindowW - UITrait.inst.toolbarw;
		}
		else if (UITrait.inst.show) {
			res = System.windowWidth() - UITrait.inst.windowW - UITrait.inst.toolbarw;
		}
		else { // Distract free
			res = System.windowWidth();
		}

		if (UITrait.inst != null && UITrait.inst.viewIndex > -1) res = Std.int(res / 2);

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h():Int {
		// Draw material preview
		if (UITrait.inst != null && UITrait.inst.materialPreview) return RenderUtil.matPreviewSize;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return RenderUtil.decalPreviewSize;

		var res = 0;
		res = System.windowHeight();
		if (UITrait.inst == null) res -= UITrait.defaultHeaderH * 3;
		if (UITrait.inst != null && UITrait.inst.show && res > 0) res -= UITrait.inst.headerh * 3;

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function x():Int {
		if (UITrait.inst.viewIndex == 1) return appx + w();
		return appx;
	}

	public static function y():Int {
		return appy;
	}

	#if arm_resizable
	static function onResize() {
		resize();

		var ratio = System.windowHeight() / winh;
		UITrait.inst.tabh = Std.int(UITrait.inst.tabh * ratio);
		UITrait.inst.tabh1 = Std.int(UITrait.inst.tabh1 * ratio);
		UITrait.inst.tabh2 = System.windowHeight() - UITrait.inst.tabh - UITrait.inst.tabh1;

		winw = System.windowWidth();
		winh = System.windowHeight();
	}
	#end

	static function saveWindowRect() {
		#if krom_windows
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

		if (UITrait.inst.cameraType == 1) {
			ViewportUtil.updateCameraType(UITrait.inst.cameraType);
		}

		Context.ddirty = 2;

		appx = UITrait.inst.toolbarw;
		appy = UITrait.inst.headerh * 2;

		if (!UITrait.inst.show) {
			appx = 0;
			appy = 0;
		}

		if (UINodes.inst.grid != null) {
			UINodes.inst.grid.unload();
			UINodes.inst.grid = null;
		}

		redrawUI();
	}

	public static function redrawUI() {
		UITrait.inst.hwnd.redraws = 2;
		UITrait.inst.hwnd1.redraws = 2;
		UITrait.inst.hwnd2.redraws = 2;
		UITrait.inst.headerHandle.redraws = 2;
		UITrait.inst.toolbarHandle.redraws = 2;
		UITrait.inst.statusHandle.redraws = 2;
		UITrait.inst.menuHandle.redraws = 2;
		UITrait.inst.workspaceHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		if (Context.ddirty < 0) Context.ddirty = 0; // Tag cached viewport texture redraw
	}

	static function update() {
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		if ((dragAsset != null || dragMaterial != null) &&
			(mouse.movementX != 0 || mouse.movementY != 0)) {
			isDragging = true;
		}
		if (mouse.released() && (dragAsset != null || dragMaterial != null)) {
			var mx = mouse.x;
			var my = mouse.y;
			var inViewport = UITrait.inst.paintVec.x < 1 && UITrait.inst.paintVec.x > 0 &&
							 UITrait.inst.paintVec.y < 1 && UITrait.inst.paintVec.y > 0;
			var inLayers = UITrait.inst.htab.position == 0 &&
						   mx > UITrait.inst.tabx && my < UITrait.inst.tabh;
			var in2dView = UIView2D.inst.show && UIView2D.inst.type == 0 &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var inNodes = UINodes.inst.show &&
						  mx > UINodes.inst.wx && mx < UINodes.inst.wx + UINodes.inst.ww &&
						  my > UINodes.inst.wy && my < UINodes.inst.wy + UINodes.inst.wh;
			if (dragAsset != null) {
				// Create image texture
				if (inNodes) {
					var index = 0;
					for (i in 0...Project.assets.length) {
						if (Project.assets[i] == dragAsset) {
							index = i;
							break;
						}
					}
					UINodes.inst.acceptDrag(index);
				}
				// Create mask
				else if (inViewport || inLayers || in2dView) {
					Layers.createImageMask(dragAsset);
				}
				dragAsset = null;
			}
			if (dragMaterial != null) {
				// Material dragged onto viewport or layers tab
				if (inViewport || inLayers || in2dView) {
					Layers.createFillLayer();
				}
				dragMaterial = null;
			}
			isDragging = false;
		}

		if (dropPath != "") {
			#if krom_linux
			var wait = !mouse.moved; // Mouse coords not updated on Linux during drag
			#else
			var wait = false;
			#end
			if (!wait) {
				dropX = mouse.x;
				dropY = mouse.y;
				Importer.importFile(dropPath, dropX, dropY);
				dropPath = "";
			}
		}

		if (UIBox.show) UIBox.update();

		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var isPicker = Context.tool == ToolPicker;
		#if krom_windows
		Zui.alwaysRedrawWindow =
			UIMenu.show ||
			UIBox.show ||
			isDragging ||
			isPicker ||
			decal ||
			UIView2D.inst.show ||
			!UITrait.inst.brush3d ||
			UITrait.inst.frame < 3;
		#end
		if (Zui.alwaysRedrawWindow && Context.ddirty < 0) Context.ddirty = 0;
	}

	static function render(g:kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		var mouse = Input.getMouse();
		if (isDragging) {
			Krom.setMouseCursor(1); // Hand
			var img = dragAsset != null ? UITrait.inst.getImage(dragAsset) : dragMaterial.imageIcon;
			@:privateAccess var size = 50 * UITrait.inst.ui.SCALE;
			var ratio = size / img.width;
			var h = img.height * ratio;
			#if (kha_opengl || kha_webgl)
			var inv = dragMaterial != null ? h : 0;
			#else
			var inv = 0;
			#end
			g.drawScaledImage(img, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2);
		}

		var usingMenu = false;
		if (UIMenu.show) usingMenu = mouse.y > UITrait.inst.headerh;

		uienabled = !UIBox.show && !usingMenu;
		if (UIFiles.show) UIFiles.render(g);
		else if (UIBox.show) UIBox.render(g);
		else if (UIMenu.show) UIMenu.render(g);
	}

	public static function getEnumTexts():Array<String> {
		return Project.assetNames.length > 0 ? Project.assetNames : [""];
	}

	public static function mapEnum(s:String):String {
		for (a in Project.assets) if (a.name == s) return a.file;
		return "";
	}

	public static function getAssetIndex(f:String):Int {
		for (i in 0...Project.assets.length) {
			if (Project.assets[i].file == f) {
				return i;
			}
		}
		return 0;
	}

	#if krom_windows
	public static function checkAscii(s:String):Bool {
		for (i in 0...s.length) if (s.charCodeAt(i) > 127) return false;
		return true;
	}

	public static function shortPath(s:String):String {
		var cmd = 'for %I in ("' + s + '") do echo %~sI';
		var save = Krom.getFilesLocation() + "\\data\\tmp.txt";
		Krom.sysCommand(cmd + ' > "' + save + '"');
		return haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString().rtrim();
	}
	#end
}
