package arm;

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
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIMenu;
import arm.ui.UIBox;
import arm.ui.UIFiles;
import arm.io.ImportAsset;
import arm.sys.Path;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
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
	public static var dragMaterial: MaterialSlot = null;
	public static var dragLayer: LayerSlot = null;
	public static var dragAsset: TAsset = null;
	public static var dragOffX = 0.0;
	public static var dragOffY = 0.0;
	static var dropPaths: Array<String> = [];
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font: Font = null;
	public static var theme: TTheme;
	public static var color_wheel: Image;
	public static var uibox: Zui;
	public static var uimenu: Zui;
	public static var fileArg = "";
	public static var ELEMENT_H = 28;
	public static var resHandle = new Handle({position: Res2048});
	public static var bitsHandle = new Handle();

	public function new() {
		Log.init();
		winw = System.windowWidth();
		winh = System.windowHeight();

		#if arm_resizable
		iron.App.onResize = onResize;
		#end

		System.notifyOnDropFiles(function(filePath: String) {
			#if krom_windows
			if (!Path.isAscii(filePath)) filePath = Path.shortPath(filePath);
			#end
			var dropPath = filePath;
			#if krom_linux
			dropPath = untyped decodeURIComponent(dropPath);
			dropPaths = dropPath.split("file://");
			for (i in 0...dropPaths.length) dropPaths[i] = dropPaths[i].rtrim();
			#else
			dropPath = dropPath.rtrim();
			dropPaths.push(dropPath);
			#end
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

		Data.getFont("font_default.ttf", function(f: Font) {
			Data.getImage("color_wheel.k", function(image: Image) {
				font = f;
				theme = zui.Themes.dark;
				theme.FILL_WINDOW_BG = true;

				var kimg: kha.Kravur.KravurImage = js.lib.Object.create(untyped kha.Kravur.KravurImage.prototype);
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
				Data.getBlob("font13.bin", function(fontbin: kha.Blob) {
					@:privateAccess kimg.texture = Image.fromBytes(fontbin.toBytes(), 128, 128, kha.graphics4.TextureFormat.L8);
					// @:privateAccess cast(font, kha.Kravur).images.set(130095, kimg);
					@:privateAccess cast(font, kha.Kravur).images.set(130174, kimg);
				});

				color_wheel = image;
				Nodes.enumTexts = enumTexts;
				uibox = new Zui({ font: f, scaleFactor: Config.raw.window_scale, color_wheel: color_wheel });
				uimenu = new Zui({ font: f, scaleFactor: Config.raw.window_scale, color_wheel: color_wheel });
				ELEMENT_H = uimenu.t.ELEMENT_H;

				// File to open passed as argument
				if (Krom.getArgCount() > 1) {
					var path = Krom.getArg(1);
					if (Path.isProject(path) ||
						Path.isMesh(path) ||
						Path.isTexture(path) ||
						Path.isFont(path)) {
						fileArg = path;
					}
				}
				iron.App.notifyOnUpdate(update);
				var root = Scene.active.root;
				new UITrait();
				new UINodes();
				new UIView2D();
				new Camera();
				iron.App.notifyOnRender2D(UITrait.inst.renderCursor);
				iron.App.notifyOnUpdate(UINodes.inst.update);
				iron.App.notifyOnRender2D(UINodes.inst.render);
				iron.App.notifyOnUpdate(UITrait.inst.update);
				iron.App.notifyOnRender2D(UITrait.inst.render);
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
					iron.App.notifyOnInit(function() {
						#if krom_windows
						fileArg = fileArg.replace("/", "\\");
						#end
						ImportAsset.run(fileArg, -1, -1, false);
						// Parse arguments
						// armorpaint import_path export_path export_file_name
						if (Krom.getArgCount() > 2) {
							UITrait.inst.textureExportPath = Krom.getArg(2);
							if (Krom.getArgCount() > 3) {
								UIFiles.filename = Krom.getArg(3);
								// hpreset.position = presets.indexOf("unreal")
							}
						}
					});
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

	public static function h(): Int {
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

	public static function x(): Int {
		if (UITrait.inst.viewIndex == 1) return appx + w();
		return appx;
	}

	public static function y(): Int {
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

		if (UITrait.inst.cameraType == CameraOrthographic) {
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
		//var kb = Input.getKeyboard();

		if ((dragAsset != null || dragMaterial != null || dragLayer != null) &&
			(mouse.movementX != 0 || mouse.movementY != 0)) {
			isDragging = true;
		}
		if (mouse.released() && (dragAsset != null || dragMaterial != null || dragLayer != null)) {
			var mx = mouse.x;
			var my = mouse.y;
			var inViewport = UITrait.inst.paintVec.x < 1 && UITrait.inst.paintVec.x > 0 &&
							 UITrait.inst.paintVec.y < 1 && UITrait.inst.paintVec.y > 0;
			var inLayers = UITrait.inst.htab.position == 0 &&
						   mx > UITrait.inst.tabx && my < UITrait.inst.tabh;
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
				dragLayer = null;
			}
			isDragging = false;
		}

		if (dropPaths.length > 0) {
			#if krom_linux
			var wait = !mouse.moved; // Mouse coords not updated on Linux during drag
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

		if (UIBox.show) UIBox.update();
		if (UIMenu.show) UIMenu.update();

		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var isPicker = Context.tool == ToolPicker;
		#if krom_windows
		Zui.alwaysRedrawWindow = !UITrait.inst.cacheDraws ||
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

	static function getDragImage(): kha.Image {
		if (dragAsset != null) return UITrait.inst.getImage(dragAsset);
		if (dragMaterial != null) return dragMaterial.imageIcon;
		if (dragLayer != null && Context.layerIsMask) return dragLayer.texpaint_mask_preview;
		else return dragLayer.texpaint_preview;
	}

	static function render(g: kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		var mouse = Input.getMouse();
		if (isDragging) {
			Krom.setMouseCursor(1); // Hand
			var img = getDragImage();
			@:privateAccess var size = 50 * UITrait.inst.ui.SCALE();
			var ratio = size / img.width;
			var h = img.height * ratio;
			#if (kha_opengl || kha_webgl)
			var inv = (dragMaterial != null || dragLayer != null) ? h : 0;
			#else
			var inv = 0;
			#end
			g.drawScaledImage(img, mouse.x + dragOffX, mouse.y + dragOffY + inv, size, h - inv * 2);
		}

		var usingMenu = false;
		if (UIMenu.show) usingMenu = mouse.y > UITrait.inst.headerh;

		uienabled = !UIBox.show && !usingMenu;
		if (UIBox.show) UIBox.render(g);
		if (UIMenu.show) UIMenu.render(g);
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

	public static function getAssetIndex(filename: String): Int {
		var i = Project.assetNames.indexOf(filename);
		return i >= 0 ? i : 0;
	}
}
