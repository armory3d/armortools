package arm.ui;

import haxe.io.Bytes;
import kha.Image;
import kha.System;
import kha.Blob;
import kha.input.KeyCode;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.data.Data;
import iron.data.MaterialData;
import iron.object.Object;
import iron.object.MeshObject;
import iron.math.Mat4;
import iron.math.Vec4;
import iron.system.Input;
import iron.system.Time;
import iron.RenderPath;
import iron.Scene;
import arm.node.MakeMaterial;
import arm.Viewport;
import arm.io.ExportTexture;
import arm.Enums;
import arm.ProjectFormat;
import arm.Res;

@:access(zui.Zui)
class UISidebar {

	public static var inst: UISidebar;
	public var show = true;
	public var ui: Zui;
	var borderStarted = 0;
	var borderHandle: Handle = null;

	public function new() {
		inst = this;

		new UIHeader();
		new UIStatus();
		new UIMenubar();

		UIHeader.inst.headerh = Std.int(UIHeader.defaultHeaderH * Config.raw.window_scale);
		UIMenubar.inst.menubarw = Std.int(UIMenubar.defaultMenubarW * Config.raw.window_scale);

		if (Project.materialData == null) {
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				Project.materialData = m;
			});
		}

		if (Project.defaultCanvas == null) { // Synchronous
			Data.getBlob("default_brush.arm", function(b: Blob) {
				Project.defaultCanvas = b;
			});
		}
		Project.nodes = new Nodes();
		Project.canvas = iron.system.ArmPack.decode(Project.defaultCanvas.toBytes());
		Project.canvas.name = "Brush 1";

		Context.parseBrushInputs();

		arm.node.Brush.parse(Project.canvas, false);

		if (Project.raw.swatches == null) {
			Project.setDefaultSwatches();
			Context.swatch = Project.raw.swatches[0];
		}

		if (Context.emptyEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 2);
			b.set(1, 2);
			b.set(2, 2);
			b.set(3, 255);
			Context.emptyEnvmap = Image.fromBytes(b, 1, 1);
		}
		if (Context.previewEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 0);
			b.set(1, 0);
			b.set(2, 0);
			b.set(3, 255);
			Context.previewEnvmap = Image.fromBytes(b, 1, 1);
		}

		var world = Scene.active.world;
		if (Context.savedEnvmap == null) {
			// Context.savedEnvmap = world.envmap;
			Context.defaultIrradiance = world.probe.irradiance;
			Context.defaultRadiance = world.probe.radiance;
			Context.defaultRadianceMipmaps = world.probe.radianceMipmaps;
		}
		world.envmap = Context.showEnvmap ? Context.savedEnvmap : Context.emptyEnvmap;
		Context.ddirty = 1;

		History.reset();

		var scale = Config.raw.window_scale;
		ui = new Zui({ theme: App.theme, font: App.font, scaleFactor: scale, color_wheel: App.colorWheel, black_white_gradient: App.blackWhiteGradient });
		Zui.onBorderHover = onBorderHover;
		Zui.onTextHover = onTextHover;
		Zui.onDeselectText = onDeselectText;

		var resources = ["cursor.k", "icons.k", "placeholder.k"];
		Res.load(resources, done);

		Context.projectObjects = [];
		for (m in Scene.active.meshes) Context.projectObjects.push(m);
	}

	function done() {
		if (ui.SCALE() > 1) setIconScale();

		Context.paintObject = cast(Scene.active.getChild(".Cube"), MeshObject);
		Project.paintObjects = [Context.paintObject];

		if (Project.filepath == "") {
			iron.App.notifyOnInit(Layers.initLayers);
		}
	}

	public function update() {
		updateUI();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		if (!App.uiEnabled) return;

		if (!UINodes.inst.ui.isTyping) {
			if (Operator.shortcut(Config.keymap.toggle_node_editor)) {
				showMaterialNodes();
			}
			else if (Operator.shortcut(Config.keymap.toggle_browser)) {
				toggleBrowser();
			}
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.projectSaveAs();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.projectSave();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.projectOpen();
		else if (Operator.shortcut(Config.keymap.file_open_recent)) Project.projectOpenRecentBox();
		else if (Operator.shortcut(Config.keymap.file_reimport_textures)) Project.reimportTextures();
		else if (Operator.shortcut(Config.keymap.file_new)) Project.projectNewBox();
		else if (Operator.shortcut(Config.keymap.file_export_textures)) {
			if (Context.textureExportPath == "") { // First export, ask for path
				BoxExport.showTextures();
			}
			else {
				function _init() {
					ExportTexture.run(Context.textureExportPath);
				}
				iron.App.notifyOnInit(_init);
			}
		}
		else if (Operator.shortcut(Config.keymap.file_export_textures_as)) {
			BoxExport.showTextures();
		}
		else if (Operator.shortcut(Config.keymap.file_import_assets)) Project.importAsset();
		else if (Operator.shortcut(Config.keymap.edit_prefs)) BoxPreferences.show();

		var kb = Input.getKeyboard();
		if (kb.started(Config.keymap.view_distract_free) ||
		   (kb.started("escape") && !show && !UIBox.show)) {
			toggleDistractFree();
		}

		#if krom_linux
		if (Operator.shortcut("alt+enter", ShortcutStarted)) {
			App.toggleFullscreen();
		}
		#end

		var mouse = Input.getMouse();

		if ((Context.brushCanLock || Context.brushLocked) && mouse.moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutDown)) {
				if (Context.brushLocked) {

						Context.brushRadius += mouse.movementX / 150;
						Context.brushRadius = Math.max(0.01, Math.min(4.0, Context.brushRadius));
						Context.brushRadius = Math.round(Context.brushRadius * 100) / 100;
						Context.brushRadiusHandle.value = Context.brushRadius;

					UIHeader.inst.headerHandle.redraws = 2;
				}
				else if (Context.brushCanLock) {
					Context.brushCanLock = false;
					Context.brushLocked = true;
				}
			}
		}

		var right = iron.App.w();

		var isTyping = UINodes.inst.ui.isTyping;

		// Viewport shortcuts
		var inViewport = mouse.viewX > 0 && mouse.viewX < right &&
						 mouse.viewY > 0 && mouse.viewY < iron.App.h();
		if (inViewport && !isTyping) {
			if (UIHeader.inst.worktab.position == Space3D) {
				// Radius
				if (Context.tool == ToolEraser ||
					Context.tool == ToolClone  ||
					Context.tool == ToolBlur) {
					if (Operator.shortcut(Config.keymap.brush_radius)) {
						Context.brushCanLock = true;
						if (!Input.getPen().connected) mouse.lock();
						Context.lockStartedX = mouse.x;
						Context.lockStartedY = mouse.y;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutRepeat)) {
						Context.brushRadius -= getRadiusIncrement();
						Context.brushRadius = Math.max(Math.round(Context.brushRadius * 100) / 100, 0.01);
						Context.brushRadiusHandle.value = Context.brushRadius;
						UIHeader.inst.headerHandle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_increase, ShortcutRepeat)) {
						Context.brushRadius += getRadiusIncrement();
						Context.brushRadius = Math.round(Context.brushRadius * 100) / 100;
						Context.brushRadiusHandle.value = Context.brushRadius;
						UIHeader.inst.headerHandle.redraws = 2;
					}
				}
			}

			// Viewpoint
			if (mouse.viewX < iron.App.w()) {
				if (Operator.shortcut(Config.keymap.view_reset)) {
					Viewport.reset();
					Viewport.scaleToBounds();
				}
				else if (Operator.shortcut(Config.keymap.view_back)) Viewport.setView(0, 1, 0, Math.PI / 2, 0, Math.PI);
				else if (Operator.shortcut(Config.keymap.view_front)) Viewport.setView(0, -1, 0, Math.PI / 2, 0, 0);
				else if (Operator.shortcut(Config.keymap.view_left)) Viewport.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
				else if (Operator.shortcut(Config.keymap.view_right)) Viewport.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
				else if (Operator.shortcut(Config.keymap.view_bottom)) Viewport.setView(0, 0, -1, Math.PI, 0, Math.PI);
				else if (Operator.shortcut(Config.keymap.view_top)) Viewport.setView(0, 0, 1, 0, 0, 0);
				else if (Operator.shortcut(Config.keymap.view_camera_type)) {
					Context.cameraType = Context.cameraType == CameraPerspective ? CameraOrthographic : CameraPerspective;
					Context.camHandle.position = Context.cameraType;
					Viewport.updateCameraType(Context.cameraType);
				}
				else if (Operator.shortcut(Config.keymap.view_orbit_left, ShortcutRepeat)) Viewport.orbit(-Math.PI / 12, 0);
				else if (Operator.shortcut(Config.keymap.view_orbit_right, ShortcutRepeat)) Viewport.orbit(Math.PI / 12, 0);
				else if (Operator.shortcut(Config.keymap.view_orbit_up, ShortcutRepeat)) Viewport.orbit(0, -Math.PI / 12);
				else if (Operator.shortcut(Config.keymap.view_orbit_down, ShortcutRepeat)) Viewport.orbit(0, Math.PI / 12);
				else if (Operator.shortcut(Config.keymap.view_orbit_opposite)) Viewport.orbitOpposite();
				else if (Operator.shortcut(Config.keymap.view_zoom_in, ShortcutRepeat)) Viewport.zoom(0.2);
				else if (Operator.shortcut(Config.keymap.view_zoom_out, ShortcutRepeat)) Viewport.zoom(-0.2);
				else if (Operator.shortcut(Config.keymap.viewport_mode)) {
					UIMenu.draw(function(ui: Zui) {
						var modeHandle = Id.handle();
						modeHandle.position = Context.viewportMode;
						ui.text(tr("Viewport Mode"), Right, ui.t.HIGHLIGHT_COL);
						var modes = [
							tr("Lit"),
							tr("Base Color"),
							tr("Normal"),
							tr("Occlusion"),
							tr("Roughness"),
							tr("Metallic"),
							tr("Opacity"),
							tr("Height")
						];
						var shortcuts = ["l", "b", "n", "o", "r", "m", "a", "h"];
						#if (kha_direct3d12 || kha_vulkan)
						modes.push(tr("Path Traced"));
						shortcuts.push("p");
						#end
						for (i in 0...modes.length) {
							ui.radio(modeHandle, i, modes[i], shortcuts[i]);
						}

						var index = shortcuts.indexOf(Keyboard.keyCode(ui.key));
						if (ui.isKeyPressed && index != -1) {
							modeHandle.position = index;
							ui.changed = true;
							Context.setViewportMode(modeHandle.position);
						}
						else if (modeHandle.changed) {
							Context.setViewportMode(modeHandle.position);
							ui.changed = true;
						}
					}, 9 #if (kha_direct3d12 || kha_vulkan) + 1 #end );
				}
			}
		}

		if (Context.brushCanLock || Context.brushLocked) {
			if (mouse.moved && Context.brushCanUnlock) {
				Context.brushLocked = false;
				Context.brushCanUnlock = false;
			}
			if ((Context.brushCanLock || Context.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutDown)) {
				mouse.unlock();
				Context.lastPaintX = -1;
				Context.lastPaintY = -1;
				if (Context.brushCanLock) {
					Context.brushCanLock = false;
					Context.brushCanUnlock = false;
					Context.brushLocked = false;
				}
				else {
					Context.brushCanUnlock = true;
				}
			}
		}

		if (borderHandle != null) {
			if (borderHandle == UINodes.inst.hwnd) {
				if (borderStarted == SideLeft) {
					Config.raw.layout[LayoutNodesW] -= Std.int(mouse.movementX);
					if (Config.raw.layout[LayoutNodesW] < 32) Config.raw.layout[LayoutNodesW] = 32;
					else if (Config.raw.layout[LayoutNodesW] > System.windowWidth() * 0.7) Config.raw.layout[LayoutNodesW] = Std.int(System.windowWidth() * 0.7);
				}
				else { // UINodes

				}
			}
			else if (borderHandle == UIStatus.inst.statusHandle) {
				var my = Std.int(mouse.movementY);
				if (Config.raw.layout[LayoutStatusH] - my >= UIStatus.defaultStatusH * Config.raw.window_scale && Config.raw.layout[LayoutStatusH] - my < System.windowHeight() * 0.7) {
					Config.raw.layout[LayoutStatusH] -= my;
				}
			}
			else {
				if (borderStarted == SideLeft) {

				}
				else {
					var my = Std.int(mouse.movementY);

				}
			}
		}
		if (!mouse.down()) {
			borderHandle = null;
			App.isResizing = false;
		}
	}

	public function toggleDistractFree() {
		show = !show;
		App.resize();
	}

	inline function getRadiusIncrement(): Float {
		return 0.1;
	}

	static function hitRect(mx: Float, my: Float, x: Int, y: Int, w: Int, h: Int) {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	function updateUI() {

		if (Console.messageTimer > 0) {
			Console.messageTimer -= Time.delta;
			if (Console.messageTimer <= 0) UIStatus.inst.statusHandle.redraws = 2;
		}

		if (!App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var setCloneSource = Context.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		var down = Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
				   (Input.getPen().down() && !kb.down("alt"));

		if (Config.raw.touch_ui) {
			if (Input.getPen().down()) {
				Context.penPaintingOnly = true;
			}
			else if (Context.penPaintingOnly) {
				down = false;
			}
		}

		if (down) {
			var mx = mouse.viewX;
			var my = mouse.viewY;
			var ww = iron.App.w();

			if (mx < ww &&
				mx > iron.App.x() &&
				my < iron.App.h() &&
				my > iron.App.y()) {

				if (Context.brushTime == 0 &&
					!App.isDragging &&
					!App.isResizing &&
					!App.isComboSelected()) { // Paint started

					// Draw line
					if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
						Context.lastPaintVecX = Context.lastPaintX;
						Context.lastPaintVecY = Context.lastPaintY;
					}

					// History.pushUndo = true;
				}
				Context.brushTime += Time.delta;
				if (Context.runBrush != null) Context.runBrush(0);
			}
		}
		else if (Context.brushTime > 0) { // Brush released
			Context.brushTime = 0;
			Context.prevPaintVecX = -1;
			Context.prevPaintVecY = -1;
			#if (!kha_direct3d12 && !kha_vulkan) // Keep accumulated samples for D3D12
			Context.ddirty = 3;
			#end
			Context.brushBlendDirty = true; // Update brush mask
		}

		var undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		var redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (kb.down("control") && kb.started("y"));

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();
	}

	public function render(g: kha.graphics2.Graphics) {
		#if (krom_android || krom_ios)
		if (!show) {
			ui.inputEnabled = true;
			g.end();
			ui.begin(g);
			if (ui.window(Id.handle(), 0, 0, 150, Std.int(ui.ELEMENT_H() + ui.ELEMENT_OFFSET()))) {
				if (ui.button(tr("Close"))) {
					toggleDistractFree();
				}
			}
			ui.end();
			g.begin(false);
		}
		#end

		if (!show || System.windowWidth() == 0 || System.windowHeight() == 0) return;

		ui.inputEnabled = App.uiEnabled;

		g.end();
		ui.begin(g);

		UIMenubar.inst.renderUI(g);
		UIHeader.inst.renderUI(g);
		UIStatus.inst.renderUI(g);

		ui.end();
		g.begin(false);
	}

	public function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();
		UINodes.inst.show = !UINodes.inst.show;
		App.resize();
	}

	public function toggleBrowser() {
		var minimized = Config.raw.layout[LayoutStatusH] <= (UIStatus.defaultStatusH * Config.raw.window_scale);
		Config.raw.layout[LayoutStatusH] = minimized ? 240 : UIStatus.defaultStatusH;
		Config.raw.layout[LayoutStatusH] = Std.int(Config.raw.layout[LayoutStatusH] * Config.raw.window_scale);
	}

	public function setIconScale() {
		if (ui.SCALE() > 1) {
			Res.load(["icons2x.k"], function() {
				@:privateAccess Res.bundled.set("icons.k", Res.get("icons2x.k"));
			});
		}
		else {
			Res.load(["icons.k"], function() {});
		}
	}

	function onBorderHover(handle: Handle, side: Int) {
		if (!App.uiEnabled) return;
		if (handle != UIStatus.inst.statusHandle &&
			handle != UINodes.inst.hwnd) return; // Scalable handles
		if (handle == UINodes.inst.hwnd && side != SideLeft && side != SideTop) return;
		if (handle == UINodes.inst.hwnd && side == SideTop) return;
		if (handle == UIStatus.inst.statusHandle && side != SideTop) return;
		if (side == SideRight) return; // UI is snapped to the right side

		side == SideLeft || side == SideRight ?
			Krom.setMouseCursor(3) : // Horizontal
			Krom.setMouseCursor(4);  // Vertical

		if (Zui.current.inputStarted) {
			borderStarted = side;
			borderHandle = handle;
			App.isResizing = true;
		}
	}

	function onTextHover() {
		Krom.setMouseCursor(2); // I-cursor
	}

	function onDeselectText() {
		#if krom_ios
		var kb = kha.input.Keyboard.get();
		@:privateAccess kb.sendUpEvent(kha.input.KeyCode.Shift);
		#end
	}

	public function tagUIRedraw() {
		UIHeader.inst.headerHandle.redraws = 2;
		UIStatus.inst.statusHandle.redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
	}
}
