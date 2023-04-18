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
import iron.object.MeshObject;
import iron.system.Input;
import iron.system.Time;
import iron.Scene;
import arm.io.ExportTexture;
import arm.Viewport;
import arm.ProjectFormat;
import arm.Res;

@:access(zui.Zui)
class UIBase {

	public static var inst: UIBase;
	public var show = true;
	public var ui: Zui;
	public var hwnds = [Id.handle()];
	public var htabs = [Id.handle()];
	public var hwndTabs = [
		[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabSwatches.draw, TabPlugins.draw, TabScript.draw, TabConsole.draw]
	];
	var borderStarted = 0;
	var borderHandle: Handle = null;
	var action_paint_remap = "";
	var operatorSearchOffset = 0;

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

		arm.logic.LogicParser.parse(Project.canvas, false);

		if (Project.raw.swatches == null) {
			Project.setDefaultSwatches();
			Context.raw.swatch = Project.raw.swatches[0];
		}

		if (Context.raw.emptyEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 8);
			b.set(1, 8);
			b.set(2, 8);
			b.set(3, 255);
			Context.raw.emptyEnvmap = Image.fromBytes(b, 1, 1);
		}
		if (Context.raw.previewEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 0);
			b.set(1, 0);
			b.set(2, 0);
			b.set(3, 255);
			Context.raw.previewEnvmap = Image.fromBytes(b, 1, 1);
		}

		var world = Scene.active.world;
		if (Context.raw.savedEnvmap == null) {
			// Context.raw.savedEnvmap = world.envmap;
			Context.raw.defaultIrradiance = world.probe.irradiance;
			Context.raw.defaultRadiance = world.probe.radiance;
			Context.raw.defaultRadianceMipmaps = world.probe.radianceMipmaps;
		}
		world.envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;
		Context.raw.ddirty = 1;

		History.reset();

		var scale = Config.raw.window_scale;
		ui = new Zui({ theme: App.theme, font: App.font, scaleFactor: scale, color_wheel: App.colorWheel, black_white_gradient: App.colorWheelGradient });
		Zui.onBorderHover = onBorderHover;
		Zui.onTextHover = onTextHover;
		Zui.onDeselectText = onDeselectText;
		Zui.onTabDrop = onTabDrop;

		var resources = ["cursor.k", "icons.k", "placeholder.k"];
		Res.load(resources, done);

		Context.raw.projectObjects = [];
		for (m in Scene.active.meshes) Context.raw.projectObjects.push(m);

		Operator.register("view_top", view_top);
	}

	function done() {
		if (ui.SCALE() > 1) setIconScale();

		Context.raw.paintObject = cast(Scene.active.getChild(".Cube"), MeshObject);
		Project.paintObjects = [Context.raw.paintObject];

		if (Project.filepath == "") {
			iron.App.notifyOnInit(App.initLayers);
		}
	}

	public function update() {
		updateUI();
		Operator.update();

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
			if (Context.raw.textureExportPath == "") { // First export, ask for path
				BoxExport.showTextures();
			}
			else {
				function _init() {
					ExportTexture.run(Context.raw.textureExportPath);
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

		if ((Context.raw.brushCanLock || Context.raw.brushLocked) && mouse.moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutDown)) {
				if (Context.raw.brushLocked) {

						Context.raw.brushRadius += mouse.movementX / 150;
						Context.raw.brushRadius = Math.max(0.01, Math.min(4.0, Context.raw.brushRadius));
						Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;

					UIHeader.inst.headerHandle.redraws = 2;
				}
				else if (Context.raw.brushCanLock) {
					Context.raw.brushCanLock = false;
					Context.raw.brushLocked = true;
				}
			}
		}

		var right = iron.App.w();

		var isTyping = UINodes.inst.ui.isTyping;

		// Viewport shortcuts
		if (Context.inPaintArea() && !isTyping) {
			if (UIHeader.inst.worktab.position == Space3D) {
				// Radius
				if (Context.raw.tool == ToolEraser ||
					Context.raw.tool == ToolClone  ||
					Context.raw.tool == ToolBlur   ||
					Context.raw.tool == ToolSmudge) {
					if (Operator.shortcut(Config.keymap.brush_radius)) {
						Context.raw.brushCanLock = true;
						if (!Input.getPen().connected) mouse.lock();
						Context.raw.lockStartedX = mouse.x;
						Context.raw.lockStartedY = mouse.y;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutRepeat)) {
						Context.raw.brushRadius -= getRadiusIncrement();
						Context.raw.brushRadius = Math.max(Math.round(Context.raw.brushRadius * 100) / 100, 0.01);
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
						UIHeader.inst.headerHandle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_increase, ShortcutRepeat)) {
						Context.raw.brushRadius += getRadiusIncrement();
						Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
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
				else if (Operator.shortcut(Config.keymap.view_camera_type)) {
					Context.raw.cameraType = Context.raw.cameraType == CameraPerspective ? CameraOrthographic : CameraPerspective;
					Context.raw.camHandle.position = Context.raw.cameraType;
					Viewport.updateCameraType(Context.raw.cameraType);
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
						modeHandle.position = Context.raw.viewportMode;
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

			if (Operator.shortcut(Config.keymap.operator_search)) operatorSearch();
		}

		if (Context.raw.brushCanLock || Context.raw.brushLocked) {
			if (mouse.moved && Context.raw.brushCanUnlock) {
				Context.raw.brushLocked = false;
				Context.raw.brushCanUnlock = false;
			}
			if ((Context.raw.brushCanLock || Context.raw.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutDown)) {
				mouse.unlock();
				Context.raw.lastPaintX = -1;
				Context.raw.lastPaintY = -1;
				if (Context.raw.brushCanLock) {
					Context.raw.brushCanLock = false;
					Context.raw.brushCanUnlock = false;
					Context.raw.brushLocked = false;
				}
				else {
					Context.raw.brushCanUnlock = true;
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
			else if (borderHandle == hwnds[TabStatus]) {
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

	function view_top() {
		var isTyping = ui.isTyping || UINodes.inst.ui.isTyping;
		var mouse = Input.getMouse();
		if (Context.inPaintArea() && !isTyping) {
			if (mouse.viewX < iron.App.w()) {
				Viewport.setView(0, 0, 1, 0, 0, 0);
			}
		}
	}

	function operatorSearch() {
		var kb = Input.getKeyboard();
		var searchHandle = Id.handle();
		var first = true;
		UIMenu.draw(function(ui: Zui) {
			ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 8, ui.t.SEPARATOR_COL);
			var search = ui.textInput(searchHandle, "", Left, true, true);
			ui.changed = false;
			if (first) {
				first = false;
				searchHandle.text = "";
				ui.startTextEdit(searchHandle); // Focus search bar
			}

			if (searchHandle.changed) operatorSearchOffset = 0;

			if (ui.isKeyPressed) { // Move selection
				if (ui.key == kha.input.KeyCode.Down && operatorSearchOffset < 6) operatorSearchOffset++;
				if (ui.key == kha.input.KeyCode.Up && operatorSearchOffset > 0) operatorSearchOffset--;
			}
			var enter = kb.down("enter");
			var count = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;

			for (n in Reflect.fields(Config.keymap)) {
				if (n.indexOf(search) >= 0) {
					ui.t.BUTTON_COL = count == operatorSearchOffset ? ui.t.HIGHLIGHT_COL : ui.t.SEPARATOR_COL;
					if (ui.button(n, Left) || (enter && count == operatorSearchOffset)) {
						if (enter) {
							ui.changed = true;
							count = 6; // Trigger break
						}
						Operator.run(n);
					}
					if (++count > 6) break;
				}
			}

			if (enter && count == 0) { // Hide popup on enter when command is not found
				ui.changed = true;
				searchHandle.text = "";
			}
			ui.t.BUTTON_COL = BUTTON_COL;
		}, 8, -1, -1);
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
			if (Console.messageTimer <= 0) hwnds[TabStatus].redraws = 2;
		}

		if (!App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var setCloneSource = Context.raw.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		var down = Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
				   (Input.getPen().down() && !kb.down("alt"));

		if (Config.raw.touch_ui) {
			if (Input.getPen().down()) {
				Context.raw.penPaintingOnly = true;
			}
			else if (Context.raw.penPaintingOnly) {
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

				if (Context.raw.brushTime == 0 &&
					!App.isDragging &&
					!App.isResizing &&
					!App.isComboSelected()) { // Paint started

					// Draw line
					if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
						Context.raw.lastPaintVecX = Context.raw.lastPaintX;
						Context.raw.lastPaintVecY = Context.raw.lastPaintY;
					}

					// History.pushUndo = true;
				}
				Context.raw.brushTime += Time.delta;
				if (Context.runBrush != null) Context.runBrush(0);
			}
		}
		else if (Context.raw.brushTime > 0) { // Brush released
			Context.raw.brushTime = 0;
			Context.raw.prevPaintVecX = -1;
			Context.raw.prevPaintVecY = -1;
			#if (!kha_direct3d12 && !kha_vulkan) // Keep accumulated samples for D3D12
			Context.raw.ddirty = 3;
			#end
			Context.raw.brushBlendDirty = true; // Update brush mask
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
		if (handle != hwnds[TabStatus] &&
			handle != UINodes.inst.hwnd) return; // Scalable handles
		if (handle == UINodes.inst.hwnd && side != SideLeft && side != SideTop) return;
		if (handle == UINodes.inst.hwnd && side == SideTop) return;
		if (handle == hwnds[TabStatus] && side != SideTop) return;
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
		@:privateAccess kb.sendUpEvent(KeyCode.Shift);
		#end
	}

	function onTabDrop(to: Handle, toPosition: Int, from: Handle, fromPosition: Int) {
		var i = htabs.indexOf(to);
		var j = htabs.indexOf(from);
		if (i > -1 && j > -1) {
			var element = hwndTabs[j][fromPosition];
			hwndTabs[j].splice(fromPosition, 1);
			hwndTabs[i].insert(toPosition, element);
			hwnds[i].redraws = 2;
			hwnds[j].redraws = 2;
		}
	}

	public function tagUIRedraw() {
		UIHeader.inst.headerHandle.redraws = 2;
		hwnds[TabStatus].redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
	}
}
