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
import arm.ui.UISidebar;
import arm.ui.UIToolbar;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIMenu;
import arm.ui.UIBox;
import arm.ui.UIFiles;
import arm.ui.UIHeader;
import arm.ui.UIStatus;
import arm.ui.UIMenubar;
import arm.ui.TabLayers;
import arm.ui.BoxExport;
import arm.io.ImportAsset;
import arm.io.ExportMesh;
import arm.io.ExportTexture;
import arm.sys.File;
import arm.sys.Path;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.ConstData;
import arm.plugin.Camera;
import arm.node.MaterialParser;
import arm.Enums;
import arm.ProjectFormat;
import arm.Res;

class App {

	public static var uiEnabled = true;
	public static var isDragging = false;
	public static var isResizing = false;
	public static var dragMaterial: MaterialSlot = null;
	public static var dragLayer: LayerSlot = null;
	public static var dragAsset: TAsset = null;
	public static var dragFile: String = null;
	public static var dragTint = 0xffffffff;
	public static var dragRect: TRect = null;
	public static var dragOffX = 0.0;
	public static var dragOffY = 0.0;
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font: Font = null;
	public static var theme: TTheme;
	public static var colorWheel: Image;
	public static var uiBox: Zui;
	public static var uiMenu: Zui;
	public static var defaultElementH = 28;
	public static var resHandle = new Handle({position: Res2048});
	public static var bitsHandle = new Handle();
	static var dropPaths: Array<String> = [];
	static var appx = 0;
	static var appy = 0;
	static var lastWindowHeight = 0;

	public function new() {
		Log.init();
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
			#if krom_ios
			// Import immediately while access to resource is unlocked
			handleDropPaths();
			#end
		});

		System.notifyOnApplicationState(
			function() { // Foreground
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
			function() {} // Shutdown
		);

		Krom.setSaveAndQuitCallback(saveAndQuitCallback);

		Data.getFont("font.ttf", function(f: Font) {
			Data.getImage("color_wheel.k", function(image: Image) {

				font = f;
				Translator.loadTranslations(Config.raw.locale);
				UIFiles.filename = tr("untitled");

				theme = zui.Themes.dark;
				theme.FILL_WINDOW_BG = true;

				// Precompiled font for fast startup
				if (Config.raw.locale == "en") {
					var kimg: kha.Kravur.KravurImage = js.lib.Object.create(untyped kha.Kravur.KravurImage.prototype);
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
						@:privateAccess cast(font, kha.Kravur).images.set(130095, kimg);
					});
				}

				colorWheel = image;
				Nodes.enumTexts = enumTexts;
				uiBox = new Zui({ font: f, scaleFactor: Config.raw.window_scale, color_wheel: colorWheel });
				uiMenu = new Zui({ font: f, scaleFactor: Config.raw.window_scale, color_wheel: colorWheel });
				defaultElementH = uiMenu.t.ELEMENT_H;

				Args.parse();

				iron.App.notifyOnUpdate(update);
				new UISidebar();
				new UINodes();
				new UIView2D();
				new Camera();
				iron.App.notifyOnRender2D(UIView2D.inst.render);
				iron.App.notifyOnUpdate(UIView2D.inst.update);
				iron.App.notifyOnRender2D(UISidebar.inst.renderCursor);
				iron.App.notifyOnUpdate(UINodes.inst.update);
				iron.App.notifyOnRender2D(UINodes.inst.render);
				iron.App.notifyOnUpdate(UISidebar.inst.update);
				iron.App.notifyOnRender2D(UISidebar.inst.render);
				iron.App.notifyOnRender2D(render);
				appx = UIToolbar.inst.toolbarw;
				appy = UIHeader.inst.headerh * 2;
				var cam = Scene.active.camera;
				cam.data.raw.fov = Std.int(cam.data.raw.fov * 100) / 100;
				cam.buildProjection();
				#if arm_creator
				Project.projectNew(); // Spawns plane as default object
				#end

				Args.run();

				// Non-default theme selected
				if (Config.raw.theme != "default.json") {
					arm.ui.BoxPreferences.loadTheme(Config.raw.theme);
				}
			});
		});
	}

	static function saveAndQuitCallback(save: Bool) {
		saveWindowRect();
		if (save) Project.projectSave(true);
		else System.stop();
	}

	public static function w(): Int {
		// Draw material preview
		if (UISidebar.inst != null && Context.materialPreview) {
			return RenderUtil.matPreviewSize;
		}

		// Drawing decal preview
		if (UISidebar.inst != null && Context.decalPreview) {
			return RenderUtil.decalPreviewSize;
		}

		var res = 0;
		if (UINodes.inst == null || UISidebar.inst == null) {
			res = System.windowWidth() - UISidebar.defaultWindowW - UIToolbar.defaultToolbarW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = System.windowWidth() - UISidebar.inst.windowW - UINodes.inst.defaultWindowW - UIToolbar.inst.toolbarw;
		}
		else if (UISidebar.inst.show) {
			res = System.windowWidth() - UISidebar.inst.windowW - UIToolbar.inst.toolbarw;
		}
		else { // Distract free
			res = System.windowWidth();
		}
		if (UISidebar.inst != null && Context.viewIndex > -1) {
			res = Std.int(res / 2);
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h(): Int {
		// Draw material preview
		if (UISidebar.inst != null && Context.materialPreview) {
			return RenderUtil.matPreviewSize;
		}

		// Drawing decal preview
		if (UISidebar.inst != null && Context.decalPreview) {
			return RenderUtil.decalPreviewSize;
		}

		var res = System.windowHeight();
		if (UISidebar.inst == null) {
			res -= UIHeader.defaultHeaderH * 2 + UIStatus.defaultStatusH;
		}
		else if (UISidebar.inst != null && UISidebar.inst.show && res > 0) {
			res -= Std.int(UIHeader.defaultHeaderH * 2 * Config.raw.window_scale) + UIStatus.inst.statush;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function x(): Int {
		return Context.viewIndex == 1 ? appx + w() : appx;
	}

	public static function y(): Int {
		return appy;
	}

	#if arm_resizable
	static function onResize() {
		resize();

		var ratio = System.windowHeight() / lastWindowHeight;
		UISidebar.inst.tabh = Std.int(UISidebar.inst.tabh * ratio);
		UISidebar.inst.tabh1 = Std.int(UISidebar.inst.tabh1 * ratio);
		UISidebar.inst.tabh2 = System.windowHeight() - UISidebar.inst.tabh - UISidebar.inst.tabh1;
		lastWindowHeight = System.windowHeight();
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

		if (Context.cameraType == CameraOrthographic) {
			ViewportUtil.updateCameraType(Context.cameraType);
		}

		Context.ddirty = 2;

		if (UISidebar.inst.show) {
			appx = UIToolbar.inst.toolbarw;
			appy = UIHeader.inst.headerh * 2;
		}
		else {
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
		UISidebar.inst.hwnd.redraws = 2;
		UISidebar.inst.hwnd1.redraws = 2;
		UISidebar.inst.hwnd2.redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		UIStatus.inst.statusHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		if (Context.ddirty < 0) Context.ddirty = 0; // Redraw viewport
	}

	static function update() {
		var mouse = Input.getMouse();

		if ((dragAsset != null || dragMaterial != null || dragLayer != null || dragFile != null) &&
			(mouse.movementX != 0 || mouse.movementY != 0)) {
			isDragging = true;
		}
		if (mouse.released() && (dragAsset != null || dragMaterial != null || dragLayer != null || dragFile != null)) {
			var mx = mouse.x;
			var my = mouse.y;
			var inViewport = Context.paintVec.x < 1 && Context.paintVec.x > 0 &&
							 Context.paintVec.y < 1 && Context.paintVec.y > 0;
			var inLayers = UISidebar.inst.htab.position == 0 &&
						   mx > UISidebar.inst.tabx && my < UISidebar.inst.tabh;
			var in2dView = UIView2D.inst.show && UIView2D.inst.type == View2DLayer &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var inNodes = UINodes.inst.show &&
						  mx > UINodes.inst.wx && mx < UINodes.inst.wx + UINodes.inst.ww &&
						  my > UINodes.inst.wy && my < UINodes.inst.wy + UINodes.inst.wh;
			if (dragAsset != null) {
				if (inNodes) { // Create image texture
					var index = 0;
					for (i in 0...Project.assets.length) {
						if (Project.assets[i] == dragAsset) {
							index = i;
							break;
						}
					}
					UINodes.inst.acceptAssetDrag(index);
				}
				else if (inViewport || inLayers || in2dView) { // Create mask
					Layers.createImageMask(dragAsset);
				}
				dragAsset = null;
			}
			else if (dragMaterial != null) {
				// Material dragged onto viewport or layers tab
				if (inViewport || inLayers || in2dView) {
					Layers.createFillLayer();
				}
				else if (inNodes) {
					var index = 0;
					for (i in 0...Project.materials.length) {
						if (Project.materials[i] == dragMaterial) {
							index = i;
							break;
						}
					}
					UINodes.inst.acceptMaterialDrag(index);
				}
				dragMaterial = null;
			}
			else if (dragLayer != null) {
				if (inNodes) {
					var index = 0;
					for (i in 0...Project.layers.length) {
						if (Project.layers[i] == dragLayer) {
							index = i;
							break;
						}
					}
					UINodes.inst.acceptLayerDrag(index);
				}
				else if (inLayers && isDragging) {
					Project.layers.remove(dragLayer);
					Project.layers.insert(TabLayers.dragDestination, dragLayer);
					MaterialParser.parseMeshMaterial();
				}
				dragLayer = null;
			}
			else if (dragFile != null) {
				var inBrowser =
					mx > iron.App.x() && mx < iron.App.x() + (System.windowWidth() - UIToolbar.inst.toolbarw - UISidebar.inst.windowW) &&
					my > System.windowHeight() - UIStatus.inst.statush;
				if (!inBrowser) {
					dropX = mouse.x;
					dropY = mouse.y;
					ImportAsset.run(dragFile, dropX, dropY);
				}
				dragFile = null;
			}
			isDragging = false;
		}

		handleDropPaths();

		if (UIBox.show) UIBox.update();
		if (UIMenu.show) UIMenu.update();

		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var isPicker = Context.tool == ToolPicker;
		#if krom_windows
		Zui.alwaysRedrawWindow = !Context.cacheDraws ||
			UIMenu.show ||
			UIBox.show ||
			isDragging ||
			isPicker ||
			decal ||
			UIView2D.inst.show ||
			!Config.raw.brush_3d ||
			Context.frame < 3;
		#end
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

	static function getDragBackground(): TRect {
		var icons = Res.get("icons.k");
		if (dragLayer != null) return Res.tile50(icons, 4, 1);
		else return null;
	}

	static function getDragImage(): kha.Image {
		dragTint = 0xffffffff;
		dragRect = null;
		if (dragAsset != null) return UISidebar.inst.getImage(dragAsset);
		if (dragMaterial != null) return dragMaterial.imageIcon;
		if (dragLayer != null && Context.layerIsMask) return dragLayer.texpaint_mask_preview;
		if (dragFile != null) {
			var icons = Res.get("icons.k");
			dragRect = dragFile.indexOf(".") > 0 ? Res.tile50(icons, 3, 1) : Res.tile50(icons, 2, 1);
			dragTint = UISidebar.inst.ui.t.HIGHLIGHT_COL;
			return icons;
		}
		else return dragLayer.texpaint_preview;
	}

	static function render(g: kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Context.frame == 2) {
			RenderUtil.makeMaterialPreview();
			UISidebar.inst.hwnd1.redraws = 2;
			MaterialParser.parseMeshMaterial();
			MaterialParser.parsePaintMaterial();
			Context.ddirty = 0;
			History.reset();
			if (History.undoLayers == null) {
				History.undoLayers = [];
				for (i in 0...Config.raw.undo_steps) {
					var l = new LayerSlot("_undo" + History.undoLayers.length);
					l.createMask(0, false);
					History.undoLayers.push(l);
				}
			}
		}
		else if (Context.frame == 3) {
			Context.ddirty = 1;
		}
		Context.frame++;

		var mouse = Input.getMouse();
		if (isDragging) {
			Krom.setMouseCursor(1); // Hand
			var img = getDragImage();
			var size = 50 * UISidebar.inst.ui.ops.scaleFactor;
			var ratio = size / img.width;
			var h = img.height * ratio;
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			var inv = 0;
			#else
			var inv = (dragMaterial != null || dragLayer != null) ? h : 0;
			#end
			g.color = dragTint;
			var bgRect = getDragBackground();
			if (bgRect != null) g.drawScaledSubImage(Res.get("icons.k"), bgRect.x, bgRect.y, bgRect.w, bgRect.h, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2);
			dragRect == null ?
				g.drawScaledImage(img, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2) :
				g.drawScaledSubImage(img, dragRect.x, dragRect.y, dragRect.w, dragRect.h, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2);
			g.color = 0xffffffff;
		}

		var usingMenu = UIMenu.show && mouse.y > UIHeader.inst.headerh;
		uiEnabled = !UIBox.show && !usingMenu;
		if (UIBox.show) UIBox.render(g);
		if (UIMenu.show) UIMenu.render(g);

		// Save last pos for continuos paint
		if (mouse.down()) {
			Context.lastPaintVecX = Context.paintVec.x;
			Context.lastPaintVecY = Context.paintVec.y;
		}
		else {
			if (Context.splitView) {
				Context.viewIndex = Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
			}

			Context.lastPaintVecX = mouse.viewX / iron.App.w();
			Context.lastPaintVecY = mouse.viewY / iron.App.h();

			Context.viewIndex = -1;

			#if (krom_android || krom_ios)
			// No mouse move events for touch, re-init last paint position on touch start
			Context.lastPaintX = -1;
			Context.lastPaintY = -1;
			#end
		}
	}

	public static function enumTexts(nodeType: String): Array<String> {
		if (nodeType == "TEX_IMAGE") {
			return Project.assetNames.length > 0 ? Project.assetNames : [""];
		}
		else if (nodeType == "LAYER" || nodeType == "LAYER_MASK") {
			var layerNames: Array<String> = [];
			for (l in Project.layers) layerNames.push(l.name);
			return layerNames;
		}
		else if (nodeType == "MATERIAL") {
			var materialNames: Array<String> = [];
			for (m in Project.materials) materialNames.push(m.canvas.name);
			return materialNames;
		}
		return null;
	}

	public static function getAssetIndex(fileName: String): Int {
		var i = Project.assetNames.indexOf(fileName);
		return i >= 0 ? i : 0;
	}
}
