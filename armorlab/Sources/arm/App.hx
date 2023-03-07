package arm;

import kha.graphics4.*;
import kha.Image;
import kha.Font;
import kha.System;
import zui.Zui;
import zui.Themes;
import zui.Nodes;
import iron.data.Data;
import iron.system.Input;
import iron.system.Time;
import iron.RenderPath;
import iron.Scene;
import arm.ui.*;
import arm.io.ImportAsset;
import arm.data.ConstData;
import arm.shader.MakeMaterial;
import arm.Viewport;
import arm.Camera;
import arm.ProjectBaseFormat;
import arm.Res;
import arm.ProjectFormat;

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
	public static var colorWheelGradient: Image;
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

	public static var pipeCopy: PipelineState;
	public static var pipeCopyR: PipelineState;
	public static var pipeCopyG: PipelineState;
	public static var pipeCopyB: PipelineState;
	public static var pipeCopyA: PipelineState;
	public static var pipeCopyATex: TextureUnit;
	public static var pipeCopy8: PipelineState;
	public static var pipeCopy128: PipelineState;
	public static var pipeCopyBGRA: PipelineState;
	public static var pipeCopyRGB: PipelineState = null;
	public static var pipeApplyMask: PipelineState;
	public static var tex0Mask: TextureUnit;
	public static var texaMask: TextureUnit;
	public static var tempImage: Image = null;
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

	public function new() {
		Console.init();
		lastWindowWidth = System.windowWidth();
		lastWindowHeight = System.windowHeight();

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
					new Camera();
					arm.logic.RandomNode.setSeed(Std.int(iron.system.Time.realTime() * 4294967295));
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

	public static function onResize() {
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
		if (Context.raw.ddirty < 0) Context.raw.ddirty = 0; // Redraw viewport
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
			var inViewport = Context.raw.paintVec.x < 1 && Context.raw.paintVec.x > 0 &&
							 Context.raw.paintVec.y < 1 && Context.raw.paintVec.y > 0;
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
		if (Context.raw.colorPickerCallback != null && (mouse.released() || mouse.released("right"))) {
			Context.raw.colorPickerCallback = null;
			Context.selectTool(Context.raw.colorPickerPreviousTool);
		}

		handleDropPaths();

		var isPicker = Context.raw.tool == ToolPicker;
		if (Zui.alwaysRedrawWindow && Context.raw.ddirty < 0) Context.raw.ddirty = 0;
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

		if (Context.raw.frame == 2) {
			MakeMaterial.parseMeshMaterial();
			MakeMaterial.parsePaintMaterial();
			Context.raw.ddirty = 0;
			// Default workspace
			if (Config.raw.workspace != 0) {
				UIHeader.inst.worktab.position = Config.raw.workspace;
				UIMenubar.inst.workspaceHandle.redraws = 2;
				UIHeader.inst.worktab.changed = true;
			}
		}
		else if (Context.raw.frame == 3) {
			Context.raw.ddirty = 2;
		}
		Context.raw.frame++;

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

	public static function isDecalLayer(): Bool {
		return false;
	}

	public static function initLayers() {
		var texpaint = iron.RenderPath.active.renderTargets.get("texpaint").image;
		var texpaint_nor = iron.RenderPath.active.renderTargets.get("texpaint_nor").image;
		var texpaint_pack = iron.RenderPath.active.renderTargets.get("texpaint_pack").image;
		// texpaint.g4.begin();
		// texpaint.g4.clear(kha.Color.fromFloats(0.5, 0.5, 0.5, 0.0)); // Base
		// texpaint.g4.end();
		texpaint.g2.begin(false);
		texpaint.g2.drawScaledImage(Res.get("placeholder.k"), 0, 0, Config.getTextureResX(), Config.getTextureResY()); // Base
		texpaint.g2.end();
		texpaint_nor.g4.begin();
		texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor.g4.end();
		texpaint_pack.g4.begin();
		texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack.g4.end();
		var texpaint_nor_empty = iron.RenderPath.active.renderTargets.get("texpaint_nor_empty").image;
		var texpaint_pack_empty = iron.RenderPath.active.renderTargets.get("texpaint_pack_empty").image;
		texpaint_nor_empty.g4.begin();
		texpaint_nor_empty.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor_empty.g4.end();
		texpaint_pack_empty.g4.begin();
		texpaint_pack_empty.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack_empty.g4.end();
	}

	public static function makePipe() {
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

		pipeCopyR = new PipelineState();
		pipeCopyR.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyR.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyR.inputLayout = [vs];
		pipeCopyR.colorWriteMasksGreen = [false];
		pipeCopyR.colorWriteMasksBlue = [false];
		pipeCopyR.colorWriteMasksAlpha = [false];
		pipeCopyR.compile();

		pipeCopyG = new PipelineState();
		pipeCopyG.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyG.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyG.inputLayout = [vs];
		pipeCopyG.colorWriteMasksRed = [false];
		pipeCopyG.colorWriteMasksBlue = [false];
		pipeCopyG.colorWriteMasksAlpha = [false];
		pipeCopyG.compile();

		pipeCopyB = new PipelineState();
		pipeCopyB.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyB.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyB.inputLayout = [vs];
		pipeCopyB.colorWriteMasksRed = [false];
		pipeCopyB.colorWriteMasksGreen = [false];
		pipeCopyB.colorWriteMasksAlpha = [false];
		pipeCopyB.compile();

		pipeApplyMask = new PipelineState();
		pipeApplyMask.vertexShader = kha.Shaders.getVertex("pass.vert");
		pipeApplyMask.fragmentShader = kha.Shaders.getFragment("mask_apply.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeApplyMask.inputLayout = [vs];
		pipeApplyMask.compile();
		tex0Mask = pipeApplyMask.getTextureUnit("tex0");
		texaMask = pipeApplyMask.getTextureUnit("texa");
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

	public static function makePipeCopyA() {
		pipeCopyA = new PipelineState();
		pipeCopyA.vertexShader = kha.Shaders.getVertex("pass.vert");
		pipeCopyA.fragmentShader = kha.Shaders.getFragment("layer_copy_rrrr.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeCopyA.inputLayout = [vs];
		pipeCopyA.colorWriteMasksRed = [false];
		pipeCopyA.colorWriteMasksGreen = [false];
		pipeCopyA.colorWriteMasksBlue = [false];
		pipeCopyA.compile();
		pipeCopyATex = pipeCopyA.getTextureUnit("tex");
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
		var l = arm.logic.BrushOutputNode.inst;
		if (tempImage != null && (tempImage.width != l.texpaint.width || tempImage.height != l.texpaint.height || tempImage.format != l.texpaint.format)) {
			var _temptex0 = RenderPath.active.renderTargets.get("temptex0");
			App.notifyOnNextFrame(function() {
				_temptex0.unload();
			});
			RenderPath.active.renderTargets.remove("temptex0");
			tempImage = null;
		}
		if (tempImage == null) {
			var t = new RenderTargetRaw();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			tempImage = rt.image;
		}
	}

	public static function makeExportImg() {
		var l = arm.logic.BrushOutputNode.inst;
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
			var t = new RenderTargetRaw();
			t.name = "expa";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			expa = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expb";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			expb = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expc";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			expc = rt.image;
		}
	}

	public static function flatten(heightToNormal = false): Dynamic {
		var texpaint = arm.logic.BrushOutputNode.inst.texpaint;
		var texpaint_nor = arm.logic.BrushOutputNode.inst.texpaint_nor;
		var texpaint_pack = arm.logic.BrushOutputNode.inst.texpaint_pack;
		return { texpaint: texpaint, texpaint_nor: texpaint_nor, texpaint_pack: texpaint_pack };
	}

	public static function onLayersResized() {
		arm.logic.BrushOutputNode.inst.texpaint.unload();
		arm.logic.BrushOutputNode.inst.texpaint = iron.RenderPath.active.renderTargets.get("texpaint").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		arm.logic.BrushOutputNode.inst.texpaint_nor.unload();
		arm.logic.BrushOutputNode.inst.texpaint_nor = iron.RenderPath.active.renderTargets.get("texpaint_nor").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		arm.logic.BrushOutputNode.inst.texpaint_pack.unload();
		arm.logic.BrushOutputNode.inst.texpaint_pack = iron.RenderPath.active.renderTargets.get("texpaint_pack").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());

		if (@:privateAccess arm.logic.InpaintNode.image != null) {
			@:privateAccess arm.logic.InpaintNode.image.unload();
			@:privateAccess arm.logic.InpaintNode.image = null;
			@:privateAccess arm.logic.InpaintNode.mask.unload();
			@:privateAccess arm.logic.InpaintNode.mask = null;
			arm.logic.InpaintNode.init();
		}

		if (@:privateAccess arm.logic.PhotoToPBRNode.images != null) {
			for (image in @:privateAccess arm.logic.PhotoToPBRNode.images) image.unload();
			@:privateAccess arm.logic.PhotoToPBRNode.images = null;
			arm.logic.PhotoToPBRNode.init();
		}

		if (@:privateAccess arm.logic.TilingNode.image != null) {
			@:privateAccess arm.logic.TilingNode.image.unload();
			@:privateAccess arm.logic.TilingNode.image = null;
			arm.logic.TilingNode.init();
		}

		iron.RenderPath.active.renderTargets.get("texpaint_blend0").image.unload();
		iron.RenderPath.active.renderTargets.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		iron.RenderPath.active.renderTargets.get("texpaint_blend1").image.unload();
		iron.RenderPath.active.renderTargets.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);

		if (iron.RenderPath.active.renderTargets.get("texpaint_node") != null) {
			iron.RenderPath.active.renderTargets.remove("texpaint_node");
		}
		if (iron.RenderPath.active.renderTargets.get("texpaint_node_target") != null) {
			iron.RenderPath.active.renderTargets.remove("texpaint_node_target");
		}

		App.notifyOnNextFrame(function() {
			initLayers();
		});

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}
}
