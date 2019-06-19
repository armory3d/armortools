package arm;

import zui.Zui;
import zui.Zui.Handle;
import zui.Canvas;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIMenu;
import arm.ui.UIBox;
import arm.ui.UIFiles;
import arm.Config;
import arm.Tool;
import kha.graphics2.truetype.StbTruetype;

class App extends iron.Trait {

	public static var version = "0.6";
	public static function x():Int { return appx; }
	public static function y():Int { return appy; }
	static var appx = 0;
	static var appy = 0;
	public static var uienabled = true;
	public static var isDragging = false;
	public static var dragMaterial:MaterialSlot = null;
	public static var dragAsset:TAsset = null;
	public static var dragOffX = 0.0;
	public static var dragOffY = 0.0;
	public static var showFiles = false;
	public static var showBox = false;
	public static var foldersOnly = false;
	public static var showFilename = false;
	public static var whandle = new Handle();
	public static var filenameHandle = new Handle({text: "untitled"});
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
	public static var fileArg = "";
	public static var saveAndQuit = false;

	public static var C:TConfig; // Config
	public static var K:Dynamic; // Config.Keymap

	public function new() {
		super();

		// Init config
		C = Config.init();
		K = C.keymap;

		#if arm_resizable
		iron.App.onResize = onResize;
		#end

		// Set base dir for file browser
		zui.Ext.dataPath = iron.data.Data.dataPath;

		kha.System.notifyOnDropFiles(function(filePath:String) {
			if (!checkAscii(filePath)) return;
			dropPath = filePath;
			dropPath = StringTools.replace(dropPath, "%20", " "); // Linux can pass %20 on drop
			dropPath = dropPath.split("file://")[0]; // Multiple files dropped on Linux, take first
			dropPath = StringTools.rtrim(dropPath);
		});

		#if krom_windows
		untyped Krom.setSaveAndQuitCallback(saveAndQuitCallback);
		#end

		iron.data.Data.getFont("font_default.ttf", function(f:kha.Font) {
			iron.data.Data.getImage('color_wheel.png', function(image:kha.Image) {
				font = f;
				theme = zui.Themes.dark;
				theme.FILL_WINDOW_BG = true;

				#if kha_krom // Pre-baked font texture
				var kimg:kha.Kravur.KravurImage = js.Object.create(untyped kha.Kravur.KravurImage.prototype);
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
				iron.data.Data.getBlob("font13.bin", function(fontbin:kha.Blob) {
					@:privateAccess kimg.texture = kha.Image.fromBytes(fontbin.toBytes(), 128, 128, kha.graphics4.TextureFormat.L8);
					// @:privateAccess cast(font, kha.Kravur).images.set(130095, kimg);
					@:privateAccess cast(font, kha.Kravur).images.set(130174, kimg);
				});
				#end

				color_wheel = image;
				zui.Nodes.getEnumTexts = getEnumTexts;
				zui.Nodes.mapEnum = mapEnum;
				uibox = new Zui({ font: f, scaleFactor: armory.data.Config.raw.window_scale });
				
				iron.App.notifyOnInit(function() {
					// File to open passed as argument
					#if kha_krom
					if (Krom.getArgCount() > 1) {
						var path = Krom.getArg(1);
						if (Format.checkProjectFormat(path) ||
							Format.checkMeshFormat(path) ||
							Format.checkTextureFormat(path) ||
							Format.checkFontFormat(path)) {
							fileArg = path;
						}
					}
					#end
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
					appx = C.ui_layout == 0 ? UITrait.inst.toolbarw : UITrait.inst.windowW + UITrait.inst.toolbarw;
					appy = UITrait.inst.headerh * 2;
					var cam = iron.Scene.active.camera;
					cam.data.raw.fov = Std.int(cam.data.raw.fov * 100) / 100;
					cam.buildProjection();
					if (fileArg != "") {
						arm.io.Importer.importFile(fileArg);
						if (Format.checkMeshFormat(fileArg)) {
							UITrait.inst.toggleDistractFree();
						}
						else if (Format.checkTextureFormat(fileArg)) {
							UITrait.inst.show2DView(1);
						}
					}
				});
			});
		});
	}

	static function saveAndQuitCallback() {
		saveAndQuit = true;
		Project.projectSave();
	}

	public static function w():Int {
		// Draw material preview
		if (UITrait.inst != null && UITrait.inst.materialPreview) return arm.util.RenderUtil.matPreviewSize;

		// Drawing decal preview
		if (UITrait.inst != null && UITrait.inst.decalPreview) return arm.util.RenderUtil.decalPreviewSize;
		
		var res = 0;
		if (UINodes.inst == null || UITrait.inst == null) {
			res = kha.System.windowWidth() - UITrait.defaultWindowW;
			res -= UITrait.defaultToolbarW;
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
		if (UITrait.inst == null) res -= UITrait.defaultHeaderH * 3;
		if (UITrait.inst != null && UITrait.inst.show && res > 0) res -= UITrait.inst.headerh * 3;

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	#if arm_resizable
	static function onResize() {
		resize();
		
		// Save window size
		// C.window_w = kha.System.windowWidth();
		// C.window_h = kha.System.windowHeight();
		// Cap height, window is not centered properly
		// var disp =  kha.Display.primary;
		// if (disp.height > 0 && C.window_h > disp.height - 140) {
		// 	C.window_h = disp.height - 140;
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

		var lay = C.ui_layout;
		
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
		if (UITrait.inst.ddirty < 0) UITrait.inst.ddirty = 0; // Tag cached viewport texture redraw
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if ((dragAsset != null || dragMaterial != null) &&
			(mouse.movementX != 0 || mouse.movementY != 0)) {
			isDragging = true;
		}
		if (mouse.released() && (dragAsset != null || dragMaterial != null)) {
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
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
					for (i in 0...UITrait.inst.assets.length) {
						if (UITrait.inst.assets[i] == dragAsset) {
							index = i;
							break;
						}
					}
					UINodes.inst.acceptDrag(index);
				}
				// Create mask
				else if (inViewport || inLayers || in2dView) {
					UITrait.inst.createImageMask(dragAsset);
				}
				dragAsset = null;
			}
			if (dragMaterial != null) {
				// Material dragged onto viewport or layers tab
				if (inViewport || inLayers || in2dView) {
					UITrait.inst.createFillLayer();
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
				dropX = mouse.x + App.x();
				dropY = mouse.y + App.y();
				arm.io.Importer.importFile(dropPath, dropX, dropY);
				dropPath = "";
			}
		}

		if (showFiles || showBox) UIBox.update();

		var decal = UITrait.inst.selectedTool == ToolDecal || UITrait.inst.selectedTool == ToolText;
		var isPicker = UITrait.inst.selectedTool == ToolPicker;
		#if krom_windows
		Zui.alwaysRedrawWindow =
			showMenu ||
			showBox ||
			isDragging ||
			isPicker ||
			decal ||
			UIView2D.inst.show ||
			!UITrait.inst.brush3d ||
			UITrait.inst.frame < 3;
		#end
		if (Zui.alwaysRedrawWindow && UITrait.inst.ddirty < 0) UITrait.inst.ddirty = 0;
	}

	static function render(g:kha.graphics2.Graphics) {
		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		var mouse = iron.system.Input.getMouse();
		if (isDragging) {
			var img = dragAsset != null ? UITrait.inst.getImage(dragAsset) : dragMaterial.imageIcon;
			@:privateAccess var size = 50 * UITrait.inst.ui.SCALE;
			var ratio = size / img.width;
			var h = img.height * ratio;
			#if (kha_opengl || kha_webgl)
			var inv = dragMaterial != null ? h : 0;
			#else
			var inv = 0;
			#end
			g.drawScaledImage(img, mouse.x + iron.App.x() + dragOffX, mouse.y + iron.App.y() + dragOffY + inv, size, h - inv * 2);
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

	public static function checkAscii(s:String):Bool {
		for (i in 0...s.length) {
			if (s.charCodeAt(i) > 127) {
				// Bail out for now :(
				UITrait.inst.showError(Strings.error0);
				return false;
			}
		}
		return true;
	}
}
