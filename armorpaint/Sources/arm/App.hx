package arm;

import kha.graphics4.*;
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
import iron.math.Mat4;
import iron.RenderPath;
import arm.ui.*;
import arm.data.*;
import arm.util.*;
import arm.io.ImportAsset;
import arm.shader.MakeMaterial;
import arm.render.RenderPathPaint;
import arm.Viewport;
import arm.Camera;
import arm.ProjectBaseFormat;
import arm.Res;

class App {

	public static var uiEnabled = true;
	public static var isDragging = false;
	public static var isResizing = false;
	public static var dragMaterial: MaterialSlot = null;
	public static var dragLayer: LayerSlot = null;
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
	public static var colorWheelGradient: Image;
	public static var uiBox: Zui;
	public static var uiMenu: Zui;
	public static var defaultElementW = 100;
	public static var defaultElementH = 28;
	public static var defaultFontSize = 13;
	public static var resHandle = new Handle();
	public static var bitsHandle = new Handle();
	static var dropPaths: Array<String> = [];
	static var appx = 0;
	static var appy = 0;
	static var lastWindowWidth = 0;
	static var lastWindowHeight = 0;

	public static var pipeMerge: PipelineState = null;
	public static var pipeMergeR: PipelineState = null;
	public static var pipeMergeG: PipelineState = null;
	public static var pipeMergeB: PipelineState = null;
	public static var pipeMergeA: PipelineState = null;
	public static var pipeCopy: PipelineState;
	public static var pipeCopy8: PipelineState;
	public static var pipeCopy128: PipelineState;
	public static var pipeCopyBGRA: PipelineState;
	public static var pipeCopyRGB: PipelineState = null;
	public static var pipeInvert8: PipelineState;
	public static var pipeApplyMask: PipelineState;
	public static var pipeMergeMask: PipelineState;
	public static var pipeColorIdToMask: PipelineState;
	public static var tex0: TextureUnit;
	public static var tex1: TextureUnit;
	public static var texmask: TextureUnit;
	public static var texa: TextureUnit;
	public static var opac: ConstantLocation;
	public static var blending: ConstantLocation;
	public static var tex0Mask: TextureUnit;
	public static var texaMask: TextureUnit;
	public static var tex0MergeMask: TextureUnit;
	public static var texaMergeMask: TextureUnit;
	public static var texColorId: TextureUnit;
	public static var texpaintColorId: TextureUnit;
	public static var opacMergeMask: ConstantLocation;
	public static var blendingMergeMask: ConstantLocation;
	public static var tempImage: Image = null;
	public static var tempMaskImage: Image = null;
	public static var expa: Image = null;
	public static var expb: Image = null;
	public static var expc: Image = null;
	public static var pipeCursor: PipelineState;
	public static var cursorVP: ConstantLocation;
	public static var cursorInvVP: ConstantLocation;
	public static var cursorMouse: ConstantLocation;
	public static var cursorTexStep: ConstantLocation;
	public static var cursorRadius: ConstantLocation;
	public static var cursorCameraRight: ConstantLocation;
	public static var cursorTint: ConstantLocation;
	public static var cursorTex: TextureUnit;
	public static var cursorGbufferD: TextureUnit;

	public static inline var defaultBase = 0.5;
	public static inline var defaultRough = 0.4;
	#if (krom_android || krom_ios)
	public static inline var maxLayers = 18;
	#else
	public static inline var maxLayers = 255;
	#end

	public function new() {
		Console.init();
		lastWindowWidth = System.windowWidth();
		lastWindowHeight = System.windowHeight();

		System.notifyOnDropFiles(function(dropPath: String) {
			#if krom_linux
			dropPath = untyped decodeURIComponent(dropPath);
			#end
			dropPath = dropPath.rtrim();
			dropPaths.push(dropPath);
		});

		System.notifyOnApplicationState(
			function() { // Foreground
				Context.raw.foregroundEvent = true;
				Context.raw.lastPaintX = -1;
				Context.raw.lastPaintY = -1;
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
				Data.getImage("color_wheel_gradient.k", function(imageColorWheelGradient: Image) {
					font = f;
					Config.loadTheme(Config.raw.theme, false);
					defaultElementW = theme.ELEMENT_W;
					defaultFontSize = theme.FONT_SIZE;
					Translator.loadTranslations(Config.raw.locale);
					UIFiles.filename = tr("untitled");
					#if (krom_android || krom_ios)
					kha.Window.get(0).title = tr("untitled");
					#end

					// Baked font for fast startup
					if (Config.raw.locale == "en") {
						font.font_ = Krom.g2_font_13(font.blob.bytes.getData());
						font.fontGlyphs = kha.graphics2.Graphics.fontGlyphs;
					}

					colorWheel = imageColorWheel;
					colorWheelGradient = imageColorWheelGradient;
					Nodes.enumTexts = enumTexts;
					Nodes.tr = tr;
					uiBox = new Zui({ theme: App.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: colorWheel, black_white_gradient: colorWheelGradient });
					uiMenu = new Zui({ theme: App.theme, font: f, scaleFactor: Config.raw.window_scale, color_wheel: colorWheel, black_white_gradient: colorWheelGradient });
					defaultElementH = uiMenu.t.ELEMENT_H;

					// Init plugins
					PluginAPI.init();
					if (Config.raw.plugins != null) {
						for (plugin in Config.raw.plugins) {
							Plugin.start(plugin);
						}
					}

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
					iron.App.notifyOnUpdate(Camera.inst.update);
					iron.App.notifyOnRender2D(render);
					appx = UIToolbar.inst.toolbarw;
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
		// Drawing material preview
		if (UISidebar.inst != null && Context.raw.materialPreview) {
			return RenderUtil.materialPreviewSize;
		}

		// Drawing decal preview
		if (UISidebar.inst != null && Context.raw.decalPreview) {
			return RenderUtil.decalPreviewSize;
		}

		var res = 0;
		if (UINodes.inst == null || UISidebar.inst == null) {
			var sidebarw = Config.raw.layout == null ? UISidebar.defaultWindowW : Config.raw.layout[LayoutSidebarW];
			res = System.windowWidth() - sidebarw - UIToolbar.defaultToolbarW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = System.windowWidth() - Config.raw.layout[LayoutSidebarW] - Config.raw.layout[LayoutNodesW] - UIToolbar.inst.toolbarw;
		}
		else if (UISidebar.inst.show) {
			res = System.windowWidth() - Config.raw.layout[LayoutSidebarW] - UIToolbar.inst.toolbarw;
		}
		else { // Distract free
			res = System.windowWidth();
		}
		if (UISidebar.inst != null && Context.raw.viewIndex > -1) {
			res = Std.int(res / 2);
		}
		if (Context.raw.paint2dView) {
			res = UIView2D.inst.ww;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h(): Int {
		// Drawing material preview
		if (UISidebar.inst != null && Context.raw.materialPreview) {
			return RenderUtil.materialPreviewSize;
		}

		// Drawing decal preview
		if (UISidebar.inst != null && Context.raw.decalPreview) {
			return RenderUtil.decalPreviewSize;
		}

		var res = System.windowHeight();
		if (UISidebar.inst == null) {
			res -= UIHeader.defaultHeaderH * 2 + UIStatus.defaultStatusH;
		}
		else if (UISidebar.inst != null && UISidebar.inst.show && res > 0) {
			var statush = Config.raw.layout[LayoutStatusH];
			res -= Std.int(UIHeader.defaultHeaderH * 2 * Config.raw.window_scale) + statush;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function x(): Int {
		return Context.raw.viewIndex == 1 ? appx + w() : appx;
	}

	public static function y(): Int {
		return appy;
	}

	public static function onResize() {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		var ratioW = System.windowWidth() / lastWindowWidth;
		lastWindowWidth = System.windowWidth();
		var ratioH = System.windowHeight() / lastWindowHeight;
		lastWindowHeight = System.windowHeight();

		Config.raw.layout[LayoutNodesW] = Std.int(Config.raw.layout[LayoutNodesW] * ratioW);
		Config.raw.layout[LayoutSidebarH0] = Std.int(Config.raw.layout[LayoutSidebarH0] * ratioH);
		Config.raw.layout[LayoutSidebarH1] = System.windowHeight() - Config.raw.layout[LayoutSidebarH0];

		resize();

		#if (krom_linux || krom_darwin)
		saveWindowRect();
		#end
	}

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

		if (Context.raw.cameraType == CameraOrthographic) {
			Viewport.updateCameraType(Context.raw.cameraType);
		}

		Context.raw.ddirty = 2;

		if (UISidebar.inst.show) {
			appx = UIToolbar.inst.toolbarw;
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
		UISidebar.inst.hwnd0.redraws = 2;
		UISidebar.inst.hwnd1.redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		UIStatus.inst.statusHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
		UIBox.hwnd.redraws = 2;
		if (Context.raw.ddirty < 0) Context.raw.ddirty = 0; // Redraw viewport
		if (Context.raw.splitView) Context.raw.ddirty = 1;
	}

	static function update() {
		var mouse = Input.getMouse();

		if (mouse.movementX != 0 || mouse.movementY != 0) {
			Krom.setMouseCursor(0); // Arrow
		}

		var hasDrag = dragAsset != null || dragMaterial != null || dragLayer != null || dragFile != null || dragSwatch != null;

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
				dragMaterial = null;
				dragSwatch = null;
				dragLayer = null;
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
			if (dragAsset != null) {
				if (Context.inNodes()) { // Create image texture
					UINodes.inst.acceptAssetDrag(Project.assets.indexOf(dragAsset));
				}
				else if (Context.inLayers() || Context.in2dView()) { // Create mask
					App.createImageMask(dragAsset);
				}
				else if (Context.inViewport()) {
					if (dragAsset.file.toLowerCase().endsWith(".hdr")) {
						var image = Project.getImage(dragAsset);
						arm.io.ImportEnvmap.run(dragAsset.file, image);
					}
				}
				dragAsset = null;
			}
			else if (dragSwatch != null) {
				if (Context.inNodes()) { // Create RGB node
					UINodes.inst.acceptSwatchDrag(dragSwatch);
				}
				else if (Context.inMaterials()) {
					TabMaterials.acceptSwatchDrag(dragSwatch);
				}
				else if (Context.inLayers() || Context.inViewport()) {
					var color = dragSwatch.base;
					color.A = dragSwatch.opacity;

					App.createColorLayer(color.value, dragSwatch.occlusion, dragSwatch.roughness, dragSwatch.metallic);
				}
				else if (Context.inSwatches()) {
					TabSwatches.acceptSwatchDrag(dragSwatch);
				}
				dragSwatch = null;
			}
			else if (dragMaterial != null) {
				materialDropped();
			}
			else if (dragLayer != null) {
				if (Context.inNodes()) {
					UINodes.inst.acceptLayerDrag(Project.layers.indexOf(dragLayer));
				}
				else if (Context.inLayers() && isDragging) {
					dragLayer.move(Context.raw.dragDestination);
					MakeMaterial.parseMeshMaterial();
				}
				dragLayer = null;
			}
			else if (dragFile != null) {
				if (!Context.inBrowser()) {
					dropX = mouse.x;
					dropY = mouse.y;
					var materialCount = Project.materials.length;
					ImportAsset.run(dragFile, dropX, dropY, true, true, function() {
						// Asset was material
						if (Project.materials.length > materialCount) {
							dragMaterial = Context.raw.material;
							materialDropped();
						}
					});
				}
				dragFile = null;
				dragFileIcon = null;
			}
			Krom.setMouseCursor(0); // Arrow
			isDragging = false;
		}
		if (Context.raw.colorPickerCallback != null && (mouse.released() || mouse.released("right"))) {
			Context.raw.colorPickerCallback = null;
			Context.selectTool(Context.raw.colorPickerPreviousTool);
		}

		handleDropPaths();

		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var isPicker = Context.raw.tool == ToolPicker;
		#if krom_windows
		Zui.alwaysRedrawWindow = !Context.raw.cacheDraws ||
			UIMenu.show ||
			UIBox.show ||
			isDragging ||
			isPicker ||
			decal ||
			UIView2D.inst.show ||
			!Config.raw.brush_3d ||
			Context.raw.frame < 3;
		#end
		if (Zui.alwaysRedrawWindow && Context.raw.ddirty < 0) Context.raw.ddirty = 0;
	}

	static function materialDropped() {
		// Material drag and dropped onto viewport or layers tab
		if (Context.inViewport() || Context.inLayers()) {
			var uvType = Input.getKeyboard().down("control") ? UVProject : UVMap;
			var decalMat = uvType == UVProject ? RenderUtil.getDecalMat() : null;
			App.createFillLayer(uvType, decalMat);
		}
		else if (Context.inNodes()) {
			UINodes.inst.acceptMaterialDrag(Project.materials.indexOf(dragMaterial));
		}
		dragMaterial = null;
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
		if (dragLayer != null && !dragLayer.isGroup() && dragLayer.fill_layer == null) {
			return Res.tile50(icons, 4, 1);
		}
		return null;
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
		if (dragMaterial != null) {
			return dragMaterial.imageIcon;
		}
		if (dragLayer != null && dragLayer.isGroup()) {
			var icons = Res.get("icons.k");
			var folderClosed = Res.tile50(icons, 2, 1);
			var folderOpen = Res.tile50(icons, 8, 1);
			dragRect = dragLayer.show_panel ? folderOpen : folderClosed;
			dragTint = UISidebar.inst.ui.t.LABEL_COL - 0x00202020;
			return icons;
		}
		if (dragLayer != null && dragLayer.isMask() && dragLayer.fill_layer == null) {
			TabLayers.makeMaskPreviewRgba32(dragLayer);
			return Context.raw.maskPreviewRgba32;
		}
		if (dragLayer != null) {
			return dragLayer.fill_layer != null ? dragLayer.fill_layer.imageIcon : dragLayer.texpaint_preview;
		}
		if (dragFile != null) {
			if (dragFileIcon != null) return dragFileIcon;
			var icons = Res.get("icons.k");
			dragRect = dragFile.indexOf(".") > 0 ? Res.tile50(icons, 3, 1) : Res.tile50(icons, 2, 1);
			dragTint = UISidebar.inst.ui.t.HIGHLIGHT_COL;
			return icons;
		}
		return null;
	}

	static function render(g: kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Context.raw.frame == 2) {
			RenderUtil.makeMaterialPreview();
			UISidebar.inst.hwnd1.redraws = 2;
			MakeMaterial.parseMeshMaterial();
			MakeMaterial.parsePaintMaterial();
			Context.raw.ddirty = 0;
			if (History.undoLayers == null) {
				History.undoLayers = [];
				for (i in 0...Config.raw.undo_steps) {
					var l = new LayerSlot("_undo" + History.undoLayers.length);
					History.undoLayers.push(l);
				}
			}
			// Default workspace
			if (Config.raw.workspace != 0) {
				UIHeader.inst.worktab.position = Config.raw.workspace;
				UIMenubar.inst.workspaceHandle.redraws = 2;
				if (UIHeader.inst.worktab.position == SpaceBake) {
					Context.selectTool(ToolBake);
				}
				else {
					Context.selectTool(ToolGizmo);
				}
				if (UIHeader.inst.worktab.position == SpaceMaterial) {
					App.updateFillLayers();
					UINodes.inst.show = true;
				}
			}
		}
		else if (Context.raw.frame == 3) {
			Context.raw.ddirty = 3;
		}
		Context.raw.frame++;

		var mouse = Input.getMouse();
		if (isDragging) {
			Krom.setMouseCursor(1); // Hand
			var img = getDragImage();
			var size = (dragSize == -1 ? 50 : dragSize) * UISidebar.inst.ui.ops.scaleFactor;
			var ratio = size / img.width;
			var h = img.height * ratio;
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			var inv = 0;
			#else
			var inv = (dragMaterial != null || (dragLayer != null && dragLayer.fill_layer != null)) ? h : 0;
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
		uiEnabled = !UIBox.show && !usingMenu && !isComboSelected();
		if (UIBox.show) UIBox.render(g);
		if (UIMenu.show) UIMenu.render(g);

		// Save last pos for continuos paint
		Context.raw.lastPaintVecX = Context.raw.paintVec.x;
		Context.raw.lastPaintVecY = Context.raw.paintVec.y;

		#if (krom_android || krom_ios)
		// No mouse move events for touch, re-init last paint position on touch start
		if (!mouse.down()) {
			Context.raw.lastPaintX = -1;
			Context.raw.lastPaintY = -1;
		}
		#end
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
		for (ui in getUIs()) if (ui.isScrolling) return true;
		return false;
	}

	public static function isComboSelected(): Bool {
		for (ui in getUIs()) if (@:privateAccess ui.comboSelectedHandle != null) return true;
		return false;
	}

	public static function getUIs(): Array<Zui> {
		return [App.uiBox, App.uiMenu, arm.ui.UISidebar.inst.ui, arm.ui.UINodes.inst.ui, arm.ui.UIView2D.inst.ui];
	}

	public static function isDecalLayer(): Bool {
		var isPaint = UIHeader.inst.worktab.position == SpacePaint;
		return isPaint && Context.raw.layer.fill_layer != null && Context.raw.layer.uvType == UVProject;
	}

	public static function redrawStatus() {
		if (arm.ui.UIStatus.inst != null) {
			arm.ui.UIStatus.inst.statusHandle.redraws = 2;
		}
	}

	public static function redrawConsole() {
		var statush = Config.raw.layout[LayoutStatusH];
		if (arm.ui.UIStatus.inst != null && arm.ui.UISidebar.inst != null && arm.ui.UISidebar.inst.ui != null && statush > arm.ui.UIStatus.defaultStatusH * arm.ui.UISidebar.inst.ui.SCALE()) {
			arm.ui.UIStatus.inst.statusHandle.redraws = 2;
		}
	}

	public static function initLayout() {
		var show2d = (UINodes.inst != null && UINodes.inst.show) || (UIView2D.inst != null && UIView2D.inst.show);
		var raw = Config.raw;
		raw.layout = [
			Std.int(UISidebar.defaultWindowW * raw.window_scale),
			Std.int(kha.System.windowHeight() / 2),
			Std.int(kha.System.windowHeight() / 2),
			#if krom_ios
			show2d ? Std.int((iron.App.w() + raw.layout[LayoutNodesW]) * 0.6) : Std.int(iron.App.w() * 0.6),
			#else
			show2d ? Std.int((iron.App.w() + raw.layout[LayoutNodesW]) / 2) : Std.int(iron.App.w() / 2),
			#end
			Std.int(iron.App.h() / 2),
			Std.int(UIStatus.defaultStatusH * raw.window_scale)
		];
	}

	public static function initConfig() {
		var raw = Config.raw;
		raw.recent_projects = [];
		raw.bookmarks = [];
		raw.plugins = [];
		#if (krom_android || krom_ios)
		raw.keymap = "touch.json";
		#else
		raw.keymap = "default.json";
		#end
		raw.theme = "default.json";
		raw.server = "https://armorpaint.fra1.digitaloceanspaces.com";
		raw.undo_steps = 4;
		raw.pressure_radius = true;
		raw.pressure_hardness = true;
		raw.pressure_angle = false;
		raw.pressure_opacity = false;
		raw.pressure_sensitivity = 1.0;
		#if kha_vulkan
		raw.material_live = false;
		#else
		raw.material_live = true;
		#end
		raw.brush_3d = true;
		raw.brush_depth_reject = true;
		raw.brush_angle_reject = true;
		raw.brush_live = false;
		raw.camera_zoom_speed = 1.0;
		raw.camera_pan_speed = 1.0;
		raw.camera_rotation_speed = 1.0;
		raw.zoom_direction = ZoomVertical;
		raw.displace_strength = 0.0;
		raw.show_asset_names = false;
		raw.wrap_mouse = false;
		raw.node_preview = true;
		raw.workspace = 0;
		raw.layer_res = Res2048;
		raw.dilate = DilateInstant;
		raw.dilate_radius = 2;
		#if (krom_android || krom_ios)
		raw.touch_ui = true;
		#else
		raw.touch_ui = false;
		#end
	}

	public static function initLayers() {
		Project.layers[0].clear(kha.Color.fromFloats(defaultBase, defaultBase, defaultBase, 1.0));
	}

	public static function resizeLayers() {
		var C = Config.raw;
		if (App.resHandle.position >= Std.int(Res16384)) { // Save memory for >=16k
			C.undo_steps = 1;
			if (Context.raw.undoHandle != null) {
				Context.raw.undoHandle.value = C.undo_steps;
			}
			while (History.undoLayers.length > C.undo_steps) {
				var l = History.undoLayers.pop();
				App.notifyOnNextFrame(function() {
					l.unload();
				});
			}
		}
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
		var rts = RenderPath.active.renderTargets;
		var _texpaint_blend0 = rts.get("texpaint_blend0").image;
		App.notifyOnNextFrame(function() {
			_texpaint_blend0.unload();
		});
		rts.get("texpaint_blend0").raw.width = Config.getTextureResX();
		rts.get("texpaint_blend0").raw.height = Config.getTextureResY();
		rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		var _texpaint_blend1 = rts.get("texpaint_blend1").image;
		App.notifyOnNextFrame(function() {
			_texpaint_blend1.unload();
		});
		rts.get("texpaint_blend1").raw.width = Config.getTextureResX();
		rts.get("texpaint_blend1").raw.height = Config.getTextureResY();
		rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		Context.raw.brushBlendDirty = true;
		if (rts.get("texpaint_blur") != null) {
			var _texpaint_blur = rts.get("texpaint_blur").image;
			App.notifyOnNextFrame(function() {
				_texpaint_blur.unload();
			});
			var sizeX = Std.int(Config.getTextureResX() * 0.95);
			var sizeY = Std.int(Config.getTextureResY() * 0.95);
			rts.get("texpaint_blur").raw.width = sizeX;
			rts.get("texpaint_blur").raw.height = sizeY;
			rts.get("texpaint_blur").image = Image.createRenderTarget(sizeX, sizeY);
		}
		if (RenderPathPaint.liveLayer != null) RenderPathPaint.liveLayer.resizeAndSetBits();
		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false; // Rebuild baketex
		#end
		Context.raw.ddirty = 2;
	}

	public static function setLayerBits() {
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
	}

	static function makeMergePipe(red: Bool, green: Bool, blue: Bool, alpha: Bool): PipelineState {
		var pipe = new PipelineState();
		pipe.vertexShader = kha.Shaders.getVertex("pass.vert");
		pipe.fragmentShader = kha.Shaders.getFragment("layer_merge.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipe.inputLayout = [vs];
		pipe.colorWriteMasksRed = [red];
		pipe.colorWriteMasksGreen = [green];
		pipe.colorWriteMasksBlue = [blue];
		pipe.colorWriteMasksAlpha = [alpha];
		pipe.compile();
		return pipe;
	}

	public static function makePipe() {
		pipeMerge = makeMergePipe(true, true, true, true);
		pipeMergeR = makeMergePipe(true, false, false, false);
		pipeMergeG = makeMergePipe(false, true, false, false);
		pipeMergeB = makeMergePipe(false, false, true, false);
		pipeMergeA = makeMergePipe(false, false, false, true);
		tex0 = pipeMerge.getTextureUnit("tex0"); // Always binding texpaint.a for blending
		tex1 = pipeMerge.getTextureUnit("tex1");
		texmask = pipeMerge.getTextureUnit("texmask");
		texa = pipeMerge.getTextureUnit("texa");
		opac = pipeMerge.getConstantLocation("opac");
		blending = pipeMerge.getConstantLocation("blending");

		pipeCopy = new PipelineState();
		pipeCopy.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeCopyBGRA = new PipelineState();
		pipeCopyBGRA.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyBGRA.fragmentShader = kha.Shaders.getFragment("layer_copy_bgra.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyBGRA.inputLayout = [vs];
		pipeCopyBGRA.compile();

		#if (kha_metal || kha_vulkan || kha_direct3d12)
		pipeCopy8 = new PipelineState();
		pipeCopy8.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy8.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopy8.inputLayout = [vs];
		pipeCopy8.colorAttachmentCount = 1;
		pipeCopy8.colorAttachments[0] = TextureFormat.L8;
		pipeCopy8.compile();

		pipeCopy128 = new PipelineState();
		pipeCopy128.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy128.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopy128.inputLayout = [vs];
		pipeCopy128.colorAttachmentCount = 1;
		pipeCopy128.colorAttachments[0] = TextureFormat.RGBA128;
		pipeCopy128.compile();
		#else
		pipeCopy8 = pipeCopy;
		pipeCopy128 = pipeCopy;
		#end

		pipeInvert8 = new PipelineState();
		pipeInvert8.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeInvert8.fragmentShader = kha.Shaders.getFragment("layer_invert.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeInvert8.inputLayout = [vs];
		pipeInvert8.colorAttachmentCount = 1;
		pipeInvert8.colorAttachments[0] = TextureFormat.L8;
		pipeInvert8.compile();

		pipeApplyMask = new PipelineState();
		pipeApplyMask.vertexShader = kha.Shaders.getVertex("pass.vert");
		pipeApplyMask.fragmentShader = kha.Shaders.getFragment("mask_apply.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeApplyMask.inputLayout = [vs];
		pipeApplyMask.compile();
		tex0Mask = pipeApplyMask.getTextureUnit("tex0");
		texaMask = pipeApplyMask.getTextureUnit("texa");

		pipeMergeMask = new PipelineState();
		pipeMergeMask.vertexShader = kha.Shaders.getVertex("pass.vert");
		pipeMergeMask.fragmentShader = kha.Shaders.getFragment("mask_merge.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeMergeMask.inputLayout = [vs];
		pipeMergeMask.compile();
		tex0MergeMask = pipeMergeMask.getTextureUnit("tex0");
		texaMergeMask = pipeMergeMask.getTextureUnit("texa");
		opacMergeMask = pipeMergeMask.getConstantLocation("opac");
		blendingMergeMask = pipeMergeMask.getConstantLocation("blending");

		pipeColorIdToMask = new PipelineState();
		pipeColorIdToMask.vertexShader = kha.Shaders.getVertex("pass.vert");
		pipeColorIdToMask.fragmentShader = kha.Shaders.getFragment("mask_colorid.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeColorIdToMask.inputLayout = [vs];
		pipeColorIdToMask.compile();
		texpaintColorId = pipeColorIdToMask.getTextureUnit("texpaint_colorid");
		texColorId = pipeColorIdToMask.getTextureUnit("texcolorid");
	}

	public static function makePipeCopyRGB() {
		pipeCopyRGB = new PipelineState();
		pipeCopyRGB.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyRGB.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyRGB.inputLayout = [vs];
		pipeCopyRGB.colorWriteMasksAlpha = [false];
		pipeCopyRGB.compile();
	}

	public static function makeCursorPipe() {
		pipeCursor = new PipelineState();
		pipeCursor.vertexShader = kha.Shaders.getVertex("cursor.vert");
		pipeCursor.fragmentShader = kha.Shaders.getFragment("cursor.frag");
		var vs = new VertexStructure();
		#if (kha_metal || kha_vulkan)
		vs.add("tex", VertexData.Short2Norm);
		#else
		vs.add("pos", VertexData.Short4Norm);
		vs.add("nor", VertexData.Short2Norm);
		vs.add("tex", VertexData.Short2Norm);
		#end
		pipeCursor.inputLayout = [vs];
		pipeCursor.blendSource = BlendingFactor.SourceAlpha;
		pipeCursor.blendDestination = BlendingFactor.InverseSourceAlpha;
		pipeCursor.depthWrite = false;
		pipeCursor.depthMode = CompareMode.Always;
		pipeCursor.compile();
		cursorVP = pipeCursor.getConstantLocation("VP");
		cursorInvVP = pipeCursor.getConstantLocation("invVP");
		cursorMouse = pipeCursor.getConstantLocation("mouse");
		cursorTexStep = pipeCursor.getConstantLocation("texStep");
		cursorRadius = pipeCursor.getConstantLocation("radius");
		cursorCameraRight = pipeCursor.getConstantLocation("cameraRight");
		cursorTint = pipeCursor.getConstantLocation("tint");
		cursorGbufferD = pipeCursor.getTextureUnit("gbufferD");
		cursorTex = pipeCursor.getTextureUnit("tex");
	}

	public static function makeTempImg() {
		var l = Project.layers[0];
		if (tempImage != null && (tempImage.width != l.texpaint.width || tempImage.height != l.texpaint.height || tempImage.format != l.texpaint.format)) {
			var _temptex0 = RenderPath.active.renderTargets.get("temptex0");
			App.notifyOnNextFrame(function() {
				_temptex0.unload();
			});
			RenderPath.active.renderTargets.remove("temptex0");
			tempImage = null;
		}
		if (tempImage == null) {
			var format = App.bitsHandle.position == Bits8  ? "RGBA32" :
					 	 App.bitsHandle.position == Bits16 ? "RGBA64" :
					 										 "RGBA128";
			var t = new RenderTargetRaw();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			tempImage = rt.image;
		}
	}

	public static function makeTempMaskImg() {
		if (tempMaskImage != null && (tempMaskImage.width != Config.getTextureResX() || tempMaskImage.height != Config.getTextureResY())) {
			var _tempMaskImage = tempMaskImage;
			App.notifyOnNextFrame(function() {
				_tempMaskImage.unload();
			});
			tempMaskImage = null;
		}
		if (tempMaskImage == null) {
			tempMaskImage = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		}
	}

	public static function makeExportImg() {
		var l = Project.layers[0];
		if (expa != null && (expa.width != l.texpaint.width || expa.height != l.texpaint.height || expa.format != l.texpaint.format)) {
			var _expa = expa;
			var _expb = expb;
			var _expc = expc;
			App.notifyOnNextFrame(function() {
				_expa.unload();
				_expb.unload();
				_expc.unload();
			});
			expa = null;
			expb = null;
			expc = null;
			RenderPath.active.renderTargets.remove("expa");
			RenderPath.active.renderTargets.remove("expb");
			RenderPath.active.renderTargets.remove("expc");
		}
		if (expa == null) {
			var format = App.bitsHandle.position == Bits8  ? "RGBA32" :
					 	 App.bitsHandle.position == Bits16 ? "RGBA64" :
					 										 "RGBA128";
			var t = new RenderTargetRaw();
			t.name = "expa";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			expa = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expb";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			expb = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expc";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			expc = rt.image;
		}
	}

	public static function duplicateLayer(l: LayerSlot) {
		if (!l.isGroup()) {
			var newLayer = l.duplicate();
			Context.setLayer(newLayer);
			var masks = l.getMasks(false);
			if (masks != null) {
				for (m in masks) {
					m = m.duplicate();
					m.parent = newLayer;
					Project.layers.remove(m);
					Project.layers.insert(Project.layers.indexOf(newLayer), m);
				}
			}
			Context.setLayer(newLayer);
		}
		else {
			var newGroup = App.newGroup();
			Project.layers.remove(newGroup);
			Project.layers.insert(Project.layers.indexOf(l) + 1, newGroup);
			// group.show_panel = true;
			for (c in l.getChildren()) {
				var masks = c.getMasks(false);
				var newLayer = c.duplicate();
				newLayer.parent = newGroup;
				Project.layers.remove(newLayer);
				Project.layers.insert(Project.layers.indexOf(newGroup), newLayer);
				if (masks != null) {
					for (m in masks) {
						var newMask = m.duplicate();
						newMask.parent = newLayer;
						Project.layers.remove(newMask);
						Project.layers.insert(Project.layers.indexOf(newLayer), newMask);
					}
				}
			}
			var groupMasks = l.getMasks();
			if (groupMasks != null) {
				for (m in groupMasks) {
					var newMask = m.duplicate();
					newMask.parent = newGroup;
					Project.layers.remove(newMask);
					Project.layers.insert(Project.layers.indexOf(newGroup), newMask);
				}
			}
			Context.setLayer(newGroup);
		}
	}

	public static function applyMasks(l: LayerSlot) {
		var masks = l.getMasks();

		if (masks != null) {
			for (i in 0...masks.length - 1) {
				mergeLayer(masks[i + 1], masks[i]);
				masks[i].delete();
			}
			masks[masks.length - 1].applyMask();
			Context.raw.layerPreviewDirty = true;
		}
	}

	public static function mergeDown() {
		var l1 = Context.raw.layer;

		if (l1.isGroup()) {
			l1 = mergeGroup(l1);
		}
		else if (l1.hasMasks()) { // It is a layer
			applyMasks(l1);
			Context.setLayer(l1);
		}

		var l0 = Project.layers[Project.layers.indexOf(l1) - 1];

		if (l0.isGroup()) {
			l0 = mergeGroup(l0);
		}
		else if (l0.hasMasks()) { // It is a layer
			applyMasks(l0);
			Context.setLayer(l0);
		}

		mergeLayer(l0, l1);
		l1.delete();
		Context.setLayer(l0);
		Context.raw.layerPreviewDirty = true;
	}

	public static function mergeGroup(l: LayerSlot) {
		if (!l.isGroup()) return null;

		var children = l.getChildren();

		if (children.length == 1 && children[0].hasMasks(false)) {
			App.applyMasks(children[0]);
		}

		for (i in 0...children.length - 1) {
			Context.setLayer(children[children.length - 1 - i]);
			History.mergeLayers();
			App.mergeDown();
		}

		// Now apply the group masks
		var masks = l.getMasks();
		if (masks != null) {
			for (i in 0...masks.length - 1) {
				mergeLayer(masks[i + 1], masks[i]);
				masks[i].delete();
			}
			App.applyMask(children[0], masks[masks.length - 1]);
		}

		children[0].parent = null;
		children[0].name = l.name;
		if (children[0].fill_layer != null) children[0].toPaintLayer();
		l.delete();
		return children[0];
	}

	public static function mergeLayer(l0 : LayerSlot, l1: LayerSlot, use_mask = false) {
		if (!l1.visible || l1.isGroup()) return;

		if (pipeMerge == null) makePipe();
		makeTempImg();
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

		tempImage.g2.begin(false); // Copy to temp
		tempImage.g2.pipeline = pipeCopy;
		tempImage.g2.drawImage(l0.texpaint, 0, 0);
		tempImage.g2.pipeline = null;
		tempImage.g2.end();

		var empty = RenderPath.active.renderTargets.get("empty_white").image;
		var mask = empty;
		var l1masks =  use_mask ? l1.getMasks() : null;
		if (l1masks != null) {
			// for (i in 1...l1masks.length - 1) {
			// 	mergeLayer(l1masks[i + 1], l1masks[i]);
			// }
			mask = l1masks[0].texpaint;
		}

		if (l1.isMask()) {
			l0.texpaint.g4.begin();
			l0.texpaint.g4.setPipeline(pipeMergeMask);
			l0.texpaint.g4.setTexture(App.tex0MergeMask, l1.texpaint);
			l0.texpaint.g4.setTexture(App.texaMergeMask, tempImage);
			l0.texpaint.g4.setFloat(opacMergeMask, l1.getOpacity());
			l0.texpaint.g4.setInt(blendingMergeMask, l1.blending);
			l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
			l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
			l0.texpaint.g4.drawIndexedVertices();
			l0.texpaint.g4.end();
		}

		if (l1.isLayer()) {
			if (l1.paintBase) {
				l0.texpaint.g4.begin();
				l0.texpaint.g4.setPipeline(pipeMerge);
				l0.texpaint.g4.setTexture(tex0, l1.texpaint);
				l0.texpaint.g4.setTexture(tex1, empty);
				l0.texpaint.g4.setTexture(texmask, mask);
				l0.texpaint.g4.setTexture(texa, tempImage);
				l0.texpaint.g4.setFloat(opac, l1.getOpacity());
				l0.texpaint.g4.setInt(blending, l1.blending);
				l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				l0.texpaint.g4.drawIndexedVertices();
				l0.texpaint.g4.end();
			}

			tempImage.g2.begin(false);
			tempImage.g2.pipeline = pipeCopy;
			tempImage.g2.drawImage(l0.texpaint_nor, 0, 0);
			tempImage.g2.pipeline = null;
			tempImage.g2.end();

			if (l1.paintNor) {
				l0.texpaint_nor.g4.begin();
				l0.texpaint_nor.g4.setPipeline(pipeMerge);
				l0.texpaint_nor.g4.setTexture(tex0, l1.texpaint);
				l0.texpaint_nor.g4.setTexture(tex1, l1.texpaint_nor);
				l0.texpaint_nor.g4.setTexture(texmask, mask);
				l0.texpaint_nor.g4.setTexture(texa, tempImage);
				l0.texpaint_nor.g4.setFloat(opac, l1.getOpacity());
				l0.texpaint_nor.g4.setInt(blending, l1.paintNorBlend ? -2 : -1);
				l0.texpaint_nor.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				l0.texpaint_nor.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				l0.texpaint_nor.g4.drawIndexedVertices();
				l0.texpaint_nor.g4.end();
			}

			tempImage.g2.begin(false);
			tempImage.g2.pipeline = pipeCopy;
			tempImage.g2.drawImage(l0.texpaint_pack, 0, 0);
			tempImage.g2.pipeline = null;
			tempImage.g2.end();

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					commandsMergePack(pipeMerge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) commandsMergePack(pipeMergeR, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintRough) commandsMergePack(pipeMergeG, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintMet) commandsMergePack(pipeMergeB, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
				}
			}
		}
	}

	public static function flatten(heightToNormal = false, layers: Array<LayerSlot> = null): Dynamic {
		if (layers == null) layers = Project.layers;
		App.makeTempImg();
		App.makeExportImg();
		if (App.pipeMerge == null) App.makePipe();
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		var empty = iron.RenderPath.active.renderTargets.get("empty_white").image;

		// Clear export layer
		App.expa.g4.begin();
		App.expa.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0));
		App.expa.g4.end();
		App.expb.g4.begin();
		App.expb.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0));
		App.expb.g4.end();
		App.expc.g4.begin();
		App.expc.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0));
		App.expc.g4.end();

		// Flatten layers
		for (l1 in layers) {
			if (!l1.isVisible()) continue;
			if (!l1.isLayer()) continue;

			var mask = empty;
			var l1masks = l1.getMasks();
			if (l1masks != null) {
				if (l1masks.length > 1) {
					App.makeTempMaskImg();
					App.tempMaskImage.g2.begin(true, 0x00000000);
					App.tempMaskImage.g2.end();
					var l1 = { texpaint: App.tempMaskImage };
					for (i in 0...l1masks.length) {
						App.mergeLayer(untyped l1, l1masks[i]);
					}
					mask = App.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				App.tempImage.g2.begin(false); // Copy to temp
				App.tempImage.g2.pipeline = App.pipeCopy;
				App.tempImage.g2.drawImage(App.expa, 0, 0);
				App.tempImage.g2.pipeline = null;
				App.tempImage.g2.end();

				App.expa.g4.begin();
				App.expa.g4.setPipeline(App.pipeMerge);
				App.expa.g4.setTexture(App.tex0, l1.texpaint);
				App.expa.g4.setTexture(App.tex1, empty);
				App.expa.g4.setTexture(App.texmask, mask);
				App.expa.g4.setTexture(App.texa, App.tempImage);
				App.expa.g4.setFloat(App.opac, l1.getOpacity());
				App.expa.g4.setInt(App.blending, layers.length > 1 ? l1.blending : 0);
				App.expa.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				App.expa.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				App.expa.g4.drawIndexedVertices();
				App.expa.g4.end();
			}

			if (l1.paintNor) {
				App.tempImage.g2.begin(false);
				App.tempImage.g2.pipeline = App.pipeCopy;
				App.tempImage.g2.drawImage(App.expb, 0, 0);
				App.tempImage.g2.pipeline = null;
				App.tempImage.g2.end();

				App.expb.g4.begin();
				App.expb.g4.setPipeline(App.pipeMerge);
				App.expb.g4.setTexture(App.tex0, l1.texpaint);
				App.expb.g4.setTexture(App.tex1, l1.texpaint_nor);
				App.expb.g4.setTexture(App.texmask, mask);
				App.expb.g4.setTexture(App.texa, App.tempImage);
				App.expb.g4.setFloat(App.opac, l1.getOpacity());
				App.expb.g4.setInt(App.blending, l1.paintNorBlend ? -2 : -1);
				App.expb.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				App.expb.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				App.expb.g4.drawIndexedVertices();
				App.expb.g4.end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				App.tempImage.g2.begin(false);
				App.tempImage.g2.pipeline = App.pipeCopy;
				App.tempImage.g2.drawImage(App.expc, 0, 0);
				App.tempImage.g2.pipeline = null;
				App.tempImage.g2.end();

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					App.commandsMergePack(App.pipeMerge, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) App.commandsMergePack(App.pipeMergeR, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintRough) App.commandsMergePack(App.pipeMergeG, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintMet) App.commandsMergePack(App.pipeMergeB, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
				}
			}
		}

		var l0 = { texpaint: App.expa, texpaint_nor: App.expb, texpaint_pack: App.expc };

		// Merge height map into normal map
		if (heightToNormal && MakeMaterial.heightUsed) {

			tempImage.g2.begin(false);
			tempImage.g2.pipeline = App.pipeCopy;
			tempImage.g2.drawImage(l0.texpaint_nor, 0, 0);
			tempImage.g2.pipeline = null;
			tempImage.g2.end();

			l0.texpaint_nor.g4.begin();
			l0.texpaint_nor.g4.setPipeline(App.pipeMerge);
			l0.texpaint_nor.g4.setTexture(App.tex0, tempImage);
			l0.texpaint_nor.g4.setTexture(App.tex1, l0.texpaint_pack);
			l0.texpaint_nor.g4.setTexture(App.texmask, empty);
			l0.texpaint_nor.g4.setTexture(App.texa, empty);
			l0.texpaint_nor.g4.setFloat(App.opac, 1.0);
			l0.texpaint_nor.g4.setInt(App.blending, -4);
			l0.texpaint_nor.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
			l0.texpaint_nor.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
			l0.texpaint_nor.g4.drawIndexedVertices();
			l0.texpaint_nor.g4.end();
		}

		return untyped l0;
	}

	public static function applyMask(l: LayerSlot, m: LayerSlot) {
		if (!l.isLayer() || !m.isMask()) return;

		if (App.pipeMerge == null) App.makePipe();
		App.makeTempImg();

		// Copy layer to temp
		tempImage.g2.begin(false);
		tempImage.g2.pipeline = App.pipeCopy;
		tempImage.g2.drawImage(l.texpaint, 0, 0);
		tempImage.g2.pipeline = null;
		tempImage.g2.end();

		// Apply mask
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		l.texpaint.g4.begin();
		l.texpaint.g4.setPipeline(App.pipeApplyMask);
		l.texpaint.g4.setTexture(App.tex0Mask, tempImage);
		l.texpaint.g4.setTexture(App.texaMask, m.texpaint);
		l.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l.texpaint.g4.drawIndexedVertices();
		l.texpaint.g4.end();
	}

	public static function commandsMergePack(pipe: PipelineState, i0: kha.Image, i1: kha.Image, i1pack: kha.Image, i1maskOpacity: Float, i1texmask: kha.Image, i1blending = -1) {
		i0.g4.begin();
		i0.g4.setPipeline(pipe);
		i0.g4.setTexture(tex0, i1);
		i0.g4.setTexture(tex1, i1pack);
		i0.g4.setTexture(texmask, i1texmask);
		i0.g4.setTexture(texa, tempImage);
		i0.g4.setFloat(opac, i1maskOpacity);
		i0.g4.setInt(blending, i1blending);
		i0.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		i0.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		i0.g4.drawIndexedVertices();
		i0.g4.end();
	}

	public static function isFillMaterial(): Bool {
		if (UIHeader.inst.worktab.position == SpaceMaterial) return true;
		var m = Context.raw.material;
		for (l in Project.layers) if (l.fill_layer == m) return true;
		return false;
	}

	public static function updateFillLayers() {
		var _layer = Context.raw.layer;
		var _tool = Context.raw.tool;
		var _fillType = Context.raw.fillTypeHandle.position;
		var current: kha.graphics2.Graphics = null;

		if (UIHeader.inst.worktab.position == SpaceMaterial) {
			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
			}

			current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();

			UIHeader.inst.worktab.position = SpacePaint;
			Context.raw.tool = ToolFill;
			Context.raw.fillTypeHandle.position = FillObject;
			MakeMaterial.parsePaintMaterial(false);
			Context.raw.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint(false);
			RenderPathPaint.dilate(true, true);
			RenderPathPaint.useLiveLayer(false);
			Context.raw.tool = _tool;
			Context.raw.fillTypeHandle.position = _fillType;
			Context.raw.pdirty = 0;
			Context.raw.rdirty = 2;
			UIHeader.inst.worktab.position = SpaceMaterial;

			if (current != null) current.begin(false);
			return;
		}

		var hasFillLayer = false;
		var hasFillMask = false;
		for (l in Project.layers) if (l.isLayer() && l.fill_layer == Context.raw.material) hasFillLayer = true;
		for (l in Project.layers) if (l.isMask() && l.fill_layer == Context.raw.material) hasFillMask = true;

		if (hasFillLayer || hasFillMask) {
			current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();
			Context.raw.pdirty = 1;
			Context.raw.tool = ToolFill;
			Context.raw.fillTypeHandle.position = FillObject;

			if (hasFillLayer) {
				var first = true;
				for (l in Project.layers) {
					if (l.isLayer() && l.fill_layer == Context.raw.material) {
						Context.raw.layer = l;
						if (first) {
							first = false;
							MakeMaterial.parsePaintMaterial(false);
						}
						setObjectMask();
						l.clear();
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}
			if (hasFillMask) {
				var first = true;
				for (l in Project.layers) {
					if (l.isMask() && l.fill_layer == Context.raw.material) {
						Context.raw.layer = l;
						if (first) {
							first = false;
							MakeMaterial.parsePaintMaterial(false);
						}
						setObjectMask();
						l.clear();
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}

			Context.raw.pdirty = 0;
			Context.raw.ddirty = 2;
			Context.raw.rdirty = 2;
			Context.raw.layersPreviewDirty = true; // Repaint all layer previews as multiple layers might have changed.
			if (current != null) current.begin(false);
			Context.raw.layer = _layer;
			setObjectMask();
			Context.raw.tool = _tool;
			Context.raw.fillTypeHandle.position = _fillType;
			MakeMaterial.parsePaintMaterial(false);
		}
	}

	public static function updateFillLayer(parsePaint = true) {
		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		var _tool = Context.raw.tool;
		var _fillType = Context.raw.fillTypeHandle.position;
		Context.raw.tool = ToolFill;
		Context.raw.fillTypeHandle.position = FillObject;
		Context.raw.pdirty = 1;
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = SpacePaint;
		Context.raw.layer.clear();

		if (parsePaint) MakeMaterial.parsePaintMaterial(false);
		RenderPathPaint.commandsPaint(false);
		RenderPathPaint.dilate(true, true);

		Context.raw.rdirty = 2;
		Context.raw.tool = _tool;
		Context.raw.fillTypeHandle.position = _fillType;
		UIHeader.inst.worktab.position = _workspace;
		if (current != null) current.begin(false);
	}

	public static function setObjectMask() {
		var ar = [tr("None")];
		for (p in Project.paintObjects) ar.push(p.name);

		var mask = Context.objectMaskUsed() ? Context.raw.layer.getObjectMask() : 0;
		if (Context.layerFilterUsed()) mask = Context.raw.layerFilter;
		if (mask > 0) {
			if (Context.raw.mergedObject != null) {
				Context.raw.mergedObject.visible = false;
			}
			var o = Project.paintObjects[0];
			for (p in Project.paintObjects) {
				if (p.name == ar[mask]) {
					o = p;
					break;
				}
			}
			Context.selectPaintObject(o);
		}
		else {
			var isAtlas = Context.raw.layer.getObjectMask() > 0 && Context.raw.layer.getObjectMask() <= Project.paintObjects.length;
			if (Context.raw.mergedObject == null || isAtlas || Context.raw.mergedObjectIsAtlas) {
				var visibles = isAtlas ? Project.getAtlasObjects(Context.raw.layer.getObjectMask()) : null;
				MeshUtil.mergeMesh(visibles);
			}
			Context.selectPaintObject(Context.mainObject());
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.visible = true;
		}
		UVUtil.dilatemapCached = false;
	}

	public static function newLayer(clear = true): LayerSlot {
		if (Project.layers.length > maxLayers) return null;
		var l = new LayerSlot();
		l.objectMask = Context.raw.layerFilter;
		if (Context.raw.layer.isMask()) Context.setLayer(Context.raw.layer.parent);
		Project.layers.insert(Project.layers.indexOf(Context.raw.layer) + 1, l);
		Context.setLayer(l);
		var li = Project.layers.indexOf(Context.raw.layer);
		if (li > 0) {
			var below = Project.layers[li - 1];
			if (below.isLayer()) {
				Context.raw.layer.parent = below.parent;
			}
		}
		if (clear) iron.App.notifyOnInit(function() { l.clear(); });
		Context.raw.layerPreviewDirty = true;
		return l;
	}

	public static function newMask(clear = true, parent: LayerSlot, position = -1): LayerSlot {
		if (Project.layers.length > maxLayers) return null;
		var l = new LayerSlot("", SlotMask, parent);
		if (position == -1) position = Project.layers.indexOf(parent);
		Project.layers.insert(position, l);
		Context.setLayer(l);
		if (clear) iron.App.notifyOnInit(function() { l.clear(); });
		Context.raw.layerPreviewDirty = true;
		return l;
	}

	public static function newGroup(): LayerSlot {
		if (Project.layers.length > maxLayers) return null;
		var l = new LayerSlot("", SlotGroup);
		Project.layers.push(l);
		Context.setLayer(l);
		return l;
	}

	public static function createFillLayer(uvType = UVMap, decalMat: Mat4 = null) {
		function _init() {
			var l = newLayer(false);
			History.newLayer();
			l.uvType = uvType;
			if (decalMat != null) l.decalMat = decalMat;
			l.objectMask = Context.raw.layerFilter;
			History.toFillLayer();
			l.toFillLayer();
		}
		iron.App.notifyOnInit(_init);
	}

	public static function createImageMask(asset: TAsset) {
		var l = Context.raw.layer;
		if (l.isMask() || l.isGroup()) {
			return;
		}

		History.newLayer();
		var m = App.newMask(false, l);
		m.clear(0x00000000, Project.getImage(asset));
		Context.raw.layerPreviewDirty = true;
	}

	public static function createColorLayer(baseColor: Int, occlusion = 1.0, roughness = App.defaultRough, metallic = 0.0) {
		function _init() {
			var l = newLayer(false);
			History.newLayer();
			l.uvType = UVMap;
			l.objectMask = Context.raw.layerFilter;
			l.clear(baseColor, occlusion, roughness, metallic);
		}
		iron.App.notifyOnInit(_init);
	}

	public static function onLayersResized() {
		iron.App.notifyOnInit(function() {
			App.resizeLayers();
			var _layer = Context.raw.layer;
			var _material = Context.raw.material;
			for (l in arm.Project.layers) {
				if (l.fill_layer != null) {
					Context.raw.layer = l;
					Context.raw.material = l.fill_layer;
					App.updateFillLayer();
				}
			}
			Context.raw.layer = _layer;
			Context.raw.material = _material;
			MakeMaterial.parsePaintMaterial();
		});
		UVUtil.uvmap = null;
		UVUtil.uvmapCached = false;
		UVUtil.trianglemap = null;
		UVUtil.trianglemapCached = false;
		UVUtil.dilatemapCached = false;
		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}
}
