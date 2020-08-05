package arm.ui;

import haxe.io.Bytes;
import kha.Image;
import kha.System;
import zui.Zui;
import zui.Id;
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
import arm.node.MaterialParser;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.UVUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.data.MaterialSlot;
import arm.io.ImportFont;
import arm.io.ExportTexture;
import arm.Enums;
import arm.ProjectFormat;
import arm.Res;

@:access(zui.Zui)
class UISidebar {

	public static var inst: UISidebar;
	public static var defaultWindowW = 280;
	public var windowW = 280; // Panel width
	public var tabx = 0;
	public var tabh = 0;
	public var tabh1 = 0;
	public var tabh2 = 0;
	public var show = true;
	public var isScrolling = false;
	public var ui: Zui;
	public var hwnd = Id.handle();
	public var hwnd1 = Id.handle();
	public var hwnd2 = Id.handle();
	public var htab = Id.handle();
	public var htab1 = Id.handle();
	public var htab2 = Id.handle();
	var borderStarted = 0;
	var borderHandle: Handle = null;

	public function new() {
		inst = this;
		new UIToolbar();
		new UIHeader();
		new UIStatus();
		new UIMenubar();

		Context.textToolText = tr("Text");

		windowW = Std.int(defaultWindowW * Config.raw.window_scale);
		UIToolbar.inst.toolbarw = Std.int(UIToolbar.defaultToolbarW * Config.raw.window_scale);
		UIHeader.inst.headerh = Std.int(UIHeader.defaultHeaderH * Config.raw.window_scale);
		UIStatus.inst.statush = Std.int(UIStatus.defaultStatusH * Config.raw.window_scale);
		UIMenubar.inst.menubarw = Std.int(UIMenubar.defaultMenubarW * Config.raw.window_scale);

		if (Project.materials == null) {
			Project.materials = [];
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				Project.materials.push(new MaterialSlot(m));
				Context.material = Project.materials[0];
			});
		}
		if (Project.materialsScene == null) {
			Project.materialsScene = [];
			Data.getMaterial("Scene", "Material2", function(m: MaterialData) {
				Project.materialsScene.push(new MaterialSlot(m));
				Context.materialScene = Project.materialsScene[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(new BrushSlot());
			Context.brush = Project.brushes[0];
			MaterialParser.parseBrush();
			Context.parseBrushInputs();
		}

		if (Project.fonts == null) {
			Project.fonts = [];
			var fontNames = App.font.getFontNames();
			Project.fonts.push(new FontSlot(fontNames.length > 0 ? fontNames[0] : "default.ttf", App.font));
			Context.font = Project.fonts[0];
		}

		if (Project.layers == null) {
			Project.layers = [];
			Project.layers.push(new LayerSlot());
			Context.layer = Project.layers[0];
		}

		if (Context.emptyEnvmap == null) {
			var b = Bytes.alloc(4);
			b.set(0, 3);
			b.set(1, 3);
			b.set(2, 3);
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
			// defaultEnvmap = world.envmap;
			Context.defaultIrradiance = world.probe.irradiance;
			Context.defaultRadiance = world.probe.radiance;
			Context.defaultRadianceMipmaps = world.probe.radianceMipmaps;
		}
		world.envmap = Context.showEnvmap ? Context.savedEnvmap : Context.emptyEnvmap;
		Context.ddirty = 1;

		var scale = Config.raw.window_scale;
		ui = new Zui( { theme: App.theme, font: App.font, scaleFactor: scale, color_wheel: App.colorWheel } );
		Zui.onBorderHover = onBorderHover;
		Zui.onTextHover = onTextHover;

		var resources = ["cursor.k", "icons.k"];
		Res.load(resources, done);

		Context.projectObjects = [];
		for (m in Scene.active.meshes) Context.projectObjects.push(m);
	}

	function done() {
		if (ui.SCALE() > 1) setIconScale();
		//
		Context.gizmo = Scene.active.getChild(".GizmoTranslate");
		Context.gizmo.transform.scale.set(0.5, 0.5, 0.5);
		Context.gizmo.transform.buildMatrix();
		Context.gizmoX = Scene.active.getChild("GizmoX");
		Context.gizmoY = Scene.active.getChild("GizmoY");
		Context.gizmoZ = Scene.active.getChild("GizmoZ");
		//

		Context.object = Scene.active.getChild("Cube");
		Context.paintObject = cast(Context.object, MeshObject);
		Project.paintObjects = [Context.paintObject];

		if (Project.filepath == "") {
			iron.App.notifyOnRender(Layers.initLayers);
		}

		// Init plugins
		if (Config.raw.plugins != null) {
			for (plugin in Config.raw.plugins) {
				Plugin.start(plugin);
			}
		}
	}

	public function update() {
		isScrolling = ui.isScrolling;
		updateUI();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		if (!App.uiEnabled) return;

		if (!UINodes.inst.ui.isTyping && !ui.isTyping) {
			if (Operator.shortcut(Config.keymap.cycle_layers)) {
				var i = (Project.layers.indexOf(Context.layer) + 1) % Project.layers.length;
				Context.setLayer(Project.layers[i]);
			}
			else if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
				show2DView(View2DLayer);
			}
			else if (Operator.shortcut(Config.keymap.toggle_node_editor)) {
				UINodes.inst.canvasType == CanvasMaterial ? showMaterialNodes() : showBrushNodes();
			}
			else if (Operator.shortcut(Config.keymap.toggle_browser)) {
				toggleBrowser();
			}
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.projectSaveAs();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.projectSave();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.projectOpen();
		else if (Operator.shortcut(Config.keymap.file_open_recent)) Project.projectOpenRecentBox();
		else if (Operator.shortcut(Config.keymap.file_reimport_mesh)) Project.reimportMesh();
		else if (Operator.shortcut(Config.keymap.file_reimport_textures)) Project.reimportTextures();
		else if (Operator.shortcut(Config.keymap.file_new)) Project.projectNewBox();
		else if (Operator.shortcut(Config.keymap.file_export_textures)) {
			if (Context.textureExportPath == "") { // First export, ask for path
				BoxExport.showTextures();
			}
			else {
				function export(_) {
					ExportTexture.run(Context.textureExportPath);
					iron.App.removeRender(export);
				}
				iron.App.notifyOnRender(export);
			}
		}
		else if (Operator.shortcut(Config.keymap.file_export_textures_as)) BoxExport.showTextures();
		else if (Operator.shortcut(Config.keymap.file_import_assets)) Project.importAsset();
		else if (Operator.shortcut(Config.keymap.edit_prefs)) BoxPreferences.show();

		var kb = Input.getKeyboard();
		if (kb.started(Config.keymap.view_distract_free) ||
		   (kb.started("escape") && !show && !UIBox.show)) {
			toggleDistractFree();
		}

		var mouse = Input.getMouse();

		if ((Context.brushCanLock || Context.brushLocked) && mouse.moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_angle, ShortcutDown)) {
				if (Context.brushLocked) {
					if (Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown)) {
						Context.brushOpacity += mouse.movementX / 500;
						Context.brushOpacity = Math.max(0.0, Math.min(1.0, Context.brushOpacity));
						Context.brushOpacity = Math.round(Context.brushOpacity * 100) / 100;
						Context.brushOpacityHandle.value = Context.brushOpacity;
					}
					else if(Operator.shortcut(Config.keymap.brush_angle, ShortcutDown)) {
						Context.brushAngle -= mouse.movementX / 5;
						Context.brushAngle = Std.int(Context.brushAngle) % 360;
						if (Context.brushAngle < 0) Context.brushAngle += 360;
						Context.brushAngleHandle.value = Context.brushAngle;
						MaterialParser.parsePaintMaterial();
					}
					else {
						Context.brushRadius += mouse.movementX / 150;
						Context.brushRadius = Math.max(0.05, Math.min(4.0, Context.brushRadius));
						Context.brushRadius = Math.round(Context.brushRadius * 100) / 100;
						Context.brushRadiusHandle.value = Context.brushRadius;
					}
					UIHeader.inst.headerHandle.redraws = 2;
				}
				else if (Context.brushCanLock) {
					Context.brushCanLock = false;
					Context.brushLocked = true;
				}
			}
		}

		var right = iron.App.w();
		if (UIView2D.inst.show) right = iron.App.w() * 2;

		// Viewport shortcuts
		if (mouse.viewX > 0 && mouse.viewX < right &&
			mouse.viewY > 0 && mouse.viewY < iron.App.h() &&
			!ui.isTyping && !UIView2D.inst.ui.isTyping && !UINodes.inst.ui.isTyping) {

			if (UIHeader.inst.worktab.position == SpacePaint) {
				if (kb.down("shift")) {
					if (kb.started("1")) Context.selectMaterial(0);
					else if (kb.started("2")) Context.selectMaterial(1);
					else if (kb.started("3")) Context.selectMaterial(2);
					else if (kb.started("4")) Context.selectMaterial(3);
					else if (kb.started("5")) Context.selectMaterial(4);
					else if (kb.started("6")) Context.selectMaterial(5);
				}

				if (!mouse.down("right")) { // Fly mode off
					if (Operator.shortcut(Config.keymap.tool_brush)) Context.selectTool(ToolBrush);
					else if (Operator.shortcut(Config.keymap.tool_eraser)) Context.selectTool(ToolEraser);
					else if (Operator.shortcut(Config.keymap.tool_fill)) Context.selectTool(ToolFill);
					else if (Operator.shortcut(Config.keymap.tool_colorid)) Context.selectTool(ToolColorId);
					else if (Operator.shortcut(Config.keymap.tool_decal)) Context.selectTool(ToolDecal);
					else if (Operator.shortcut(Config.keymap.tool_text)) Context.selectTool(ToolText);
					else if (Operator.shortcut(Config.keymap.tool_clone)) Context.selectTool(ToolClone);
					else if (Operator.shortcut(Config.keymap.tool_blur)) Context.selectTool(ToolBlur);
					else if (Operator.shortcut(Config.keymap.tool_particle)) Context.selectTool(ToolParticle);
					else if (Operator.shortcut(Config.keymap.tool_picker)) Context.selectTool(ToolPicker);
					else if (Operator.shortcut(Config.keymap.swap_brush_eraser)) Context.selectTool(Context.tool == ToolBrush ? ToolEraser : ToolBrush);
				}

				// Radius
				if (Context.tool == ToolBrush  ||
					Context.tool == ToolEraser ||
					Context.tool == ToolDecal  ||
					Context.tool == ToolText   ||
					Context.tool == ToolClone  ||
					Context.tool == ToolBlur   ||
					Context.tool == ToolParticle) {
					if (Operator.shortcut(Config.keymap.brush_radius) ||
						Operator.shortcut(Config.keymap.brush_opacity) ||
						Operator.shortcut(Config.keymap.brush_angle)) {
						Context.brushCanLock = true;
						if (!Input.getPen().connected) mouse.lock();
						Context.lockStartedX = mouse.x;
						Context.lockStartedY = mouse.y;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutRepeat)) {
						Context.brushRadius -= getRadiusIncrement();
						Context.brushRadius = Math.round(Context.brushRadius * 100) / 100;
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
			if (Operator.shortcut(Config.keymap.view_reset)) {
				ViewportUtil.resetViewport();
				ViewportUtil.scaleToBounds();
			}
			else if (Operator.shortcut(Config.keymap.view_back)) ViewportUtil.setView(0, 1, 0, Math.PI / 2, 0, Math.PI);
			else if (Operator.shortcut(Config.keymap.view_front)) ViewportUtil.setView(0, -1, 0, Math.PI / 2, 0, 0);
			else if (Operator.shortcut(Config.keymap.view_left)) ViewportUtil.setView(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
			else if (Operator.shortcut(Config.keymap.view_right)) ViewportUtil.setView(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
			else if (Operator.shortcut(Config.keymap.view_bottom)) ViewportUtil.setView(0, 0, -1, Math.PI, 0, Math.PI);
			else if (Operator.shortcut(Config.keymap.view_top)) ViewportUtil.setView(0, 0, 1, 0, 0, 0);
			else if (Operator.shortcut(Config.keymap.view_camera_type)) {
				Context.cameraType = Context.cameraType == CameraPerspective ? CameraOrthographic : CameraPerspective;
				Context.camHandle.position = Context.cameraType;
				ViewportUtil.updateCameraType(Context.cameraType);
			}
			else if (Operator.shortcut(Config.keymap.view_orbit_left, ShortcutRepeat)) ViewportUtil.orbit(-Math.PI / 12, 0);
			else if (Operator.shortcut(Config.keymap.view_orbit_right, ShortcutRepeat)) ViewportUtil.orbit(Math.PI / 12, 0);
			else if (Operator.shortcut(Config.keymap.view_orbit_up, ShortcutRepeat)) ViewportUtil.orbit(0, -Math.PI / 12);
			else if (Operator.shortcut(Config.keymap.view_orbit_down, ShortcutRepeat)) ViewportUtil.orbit(0, Math.PI / 12);
			else if (Operator.shortcut(Config.keymap.view_orbit_opposite)) ViewportUtil.orbit(Math.PI, 0);
			else if (Operator.shortcut(Config.keymap.view_zoom_in, ShortcutRepeat)) ViewportUtil.zoom(0.2);
			else if (Operator.shortcut(Config.keymap.view_zoom_out, ShortcutRepeat)) ViewportUtil.zoom(-0.2);
		}

		if (Context.brushCanLock || Context.brushLocked) {
			if (mouse.moved && Context.brushCanUnlock) {
				Context.brushLocked = false;
				Context.brushCanUnlock = false;
			}
			if (kb.released(Config.keymap.brush_radius)) {
				mouse.unlock();
				Context.brushCanUnlock = true;
				Context.lastPaintX = -1;
				Context.lastPaintY = -1;
			}
		}

		if (borderHandle != null) {
			if (borderHandle == UINodes.inst.hwnd || borderHandle == UIView2D.inst.hwnd) {
				if (borderStarted == SideLeft) {
					UINodes.inst.defaultWindowW -= Std.int(mouse.movementX);
					if (UINodes.inst.defaultWindowW < 32) UINodes.inst.defaultWindowW = 32;
					else if (UINodes.inst.defaultWindowW > System.windowWidth() * 0.7) UINodes.inst.defaultWindowW = Std.int(System.windowWidth() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					UINodes.inst.defaultWindowH -= Std.int(mouse.movementY);
					if (UINodes.inst.defaultWindowH < 32) UINodes.inst.defaultWindowH = 32;
					else if (UINodes.inst.defaultWindowH > iron.App.h() * 0.95) UINodes.inst.defaultWindowH = Std.int(iron.App.h() * 0.95);
				}
			}
			else if (borderHandle == UIStatus.inst.statusHandle) {
				var my = Std.int(mouse.movementY);
				if (UIStatus.inst.statush - my >= UIStatus.defaultStatusH * Config.raw.window_scale && UIStatus.inst.statush - my < System.windowHeight() * 0.7) {
					UIStatus.inst.statush -= my;
				}
			}
			else {
				if (borderStarted == SideLeft) {
					defaultWindowW -= Std.int(mouse.movementX);
					if (defaultWindowW < 32) defaultWindowW = 32;
					else if (defaultWindowW > System.windowWidth() - 32) defaultWindowW = System.windowWidth() - 32;
					windowW = Std.int(defaultWindowW * Config.raw.window_scale);
				}
				else {
					var my = Std.int(mouse.movementY);
					if (borderHandle == hwnd1 && borderStarted == SideTop) {
						if (tabh + my > 32 && tabh1 - my > 32) {
							tabh += my;
							tabh1 -= my;
						}
					}
					else if (borderHandle == hwnd2 && borderStarted == SideTop) {
						if (tabh1 + my > 32 && tabh2 - my > 32) {
							tabh1 += my;
							tabh2 -= my;
						}
					}
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

	function getBrushStencilRect(): TRect {
		var w = Std.int(Context.brushStencilImage.width * (App.h() / Context.brushStencilImage.height) * Context.brushStencilScale);
		var h = Std.int(App.h() * Context.brushStencilScale);
		var x = Std.int(App.x() + Context.brushStencilX * App.w());
		var y = Std.int(App.y() + Context.brushStencilY * App.h());
		return { w: w, h: h, x: x, y: y };
	}

	function updateUI() {

		if (Log.messageTimer > 0) {
			Log.messageTimer -= Time.delta;
			if (Log.messageTimer <= 0) UIStatus.inst.statusHandle.redraws = 2;
		}

		if (!App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var setCloneSource = Context.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		if (Context.brushStencilImage != null && Operator.shortcut(Config.keymap.stencil_transform, ShortcutDown)) {
			var r = getBrushStencilRect();
			if (mouse.started("left")) {
				Context.brushStencilScaling =
					hitRect(mouse.x, mouse.y, r.x - 8,       r.y - 8,       16, 16) ||
					hitRect(mouse.x, mouse.y, r.x - 8,       r.h + r.y - 8, 16, 16) ||
					hitRect(mouse.x, mouse.y, r.w + r.x - 8, r.y - 8,       16, 16) ||
					hitRect(mouse.x, mouse.y, r.w + r.x - 8, r.h + r.y - 8, 16, 16);
				var cosa = Math.cos(-Context.brushStencilAngle);
				var sina = Math.sin(-Context.brushStencilAngle);
				var ox = 0;
				var oy = -r.h / 2;
				var x = ox * cosa - oy * sina;
				var y = ox * sina + oy * cosa;
				x += r.x + r.w / 2;
				y += r.y + r.h / 2;
				Context.brushStencilRotating =
					hitRect(mouse.x, mouse.y, Std.int(x - 16), Std.int(y - 16), 32, 32);
			}
			var _scale = Context.brushStencilScale;
			if (mouse.down("left")) {
				if (Context.brushStencilScaling) {
					var mult = mouse.x > r.x + r.w / 2 ? 1 : -1;
					Context.brushStencilScale += mouse.movementX / 400 * mult;
				}
				else if (Context.brushStencilRotating) {
					var gizmoX = r.x + r.w / 2;
					var gizmoY = r.y + r.h / 2;
					Context.brushStencilAngle = -Math.atan2(mouse.y - gizmoY, mouse.x - gizmoX) - Math.PI / 2;
				}
				else {
					Context.brushStencilX += mouse.movementX / App.w();
					Context.brushStencilY += mouse.movementY / App.h();
				}
			}
			else Context.brushStencilScaling = false;
			if (mouse.wheelDelta != 0) {
				Context.brushStencilScale -= mouse.wheelDelta / 10;
			}
			// Center after scale
			var ratio = App.h() / Context.brushStencilImage.height;
			var oldW = _scale * Context.brushStencilImage.width * ratio;
			var newW = Context.brushStencilScale * Context.brushStencilImage.width * ratio;
			var oldH = _scale * App.h();
			var newH = Context.brushStencilScale * App.h();
			Context.brushStencilX += (oldW - newW) / App.w() / 2;
			Context.brushStencilY += (oldH - newH) / App.h() / 2;
		}

		var down = Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
				   (Input.getPen().down() && !kb.down("alt"));
		if (down) {
			var mx = mouse.viewX;
			var my = mouse.viewY;
			if (Context.paint2d) mx -= iron.App.w();

			if (mx < iron.App.w() && mx > iron.App.x() &&
				my < iron.App.h() && my > iron.App.y()) {

				if (setCloneSource) {
					Context.cloneStartX = mx;
					Context.cloneStartY = my;
				}
				else {
					if (Context.brushTime == 0 &&
						!App.isDragging &&
						!App.isResizing &&
						@:privateAccess ui.comboSelectedHandle == null) { // Paint started

						// Draw line
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
							Context.lastPaintVecX = Context.lastPaintX;
							Context.lastPaintVecY = Context.lastPaintY;
						}

						History.pushUndo = true;
						if (Context.tool == ToolClone && Context.cloneStartX >= 0.0) { // Clone delta
							Context.cloneDeltaX = (Context.cloneStartX - mx) / iron.App.w();
							Context.cloneDeltaY = (Context.cloneStartY - my) / iron.App.h();
							Context.cloneStartX = -1;
						}
						else if (Context.tool == ToolParticle) {
							// Reset particles
							#if arm_particles
							var emitter: MeshObject = cast Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							@:privateAccess psys.time = 0;
							// @:privateAccess psys.time = @:privateAccess psys.seed * @:privateAccess psys.animtime;
							// @:privateAccess psys.seed++;
							#end
						}
					}
					Context.brushTime += Time.delta;
					if (Context.runBrush != null) Context.runBrush(0);
				}
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
			Context.layerPreviewDirty = true; // Update layer preview
		}

		if (Context.layersPreviewDirty) {
			Context.layersPreviewDirty = false;
			Context.layerPreviewDirty = false;
			if (Layers.pipeMerge == null) Layers.makePipe();
			// Update all layer previews
			for (l in Project.layers) {
				if (l.getChildren() != null) continue;
				var target = l.texpaint_preview;
				var source = l.texpaint;
				var g2 = target.g2;
				g2.begin(true, 0x00000000);
				g2.pipeline = Layers.pipeCopy;
				g2.drawScaledImage(source, 0, 0, target.width, target.height);
				g2.pipeline = null;
				g2.end();
				if (l.texpaint_mask != null) {
					var target = l.texpaint_mask_preview;
					var source = l.texpaint_mask;
					var g2 = target.g2;
					g2.begin(true, 0x00000000);
					g2.pipeline = Layers.pipeCopy8;
					g2.drawScaledImage(source, 0, 0, target.width, target.height);
					g2.pipeline = null;
					g2.end();
				}
			}
			hwnd.redraws = 2;
		}
		if (Context.layerPreviewDirty && Context.layer.getChildren() == null) {
			Context.layerPreviewDirty = false;
			if (Layers.pipeMerge == null) Layers.makePipe();
			// Update layer preview
			var l = Context.layer;
			var target = Context.layerIsMask ? l.texpaint_mask_preview : l.texpaint_preview;
			var source = Context.layerIsMask ? l.texpaint_mask : l.texpaint;
			var g2 = target.g2;
			g2.begin(true, 0x00000000);
			g2.pipeline = Context.layerIsMask ? Layers.pipeCopy8 : Layers.pipeCopy;
			g2.drawScaledImage(source, 0, 0, target.width, target.height);
			g2.pipeline = null;
			g2.end();
			hwnd.redraws = 2;
		}

		var undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		var redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (kb.down("control") && kb.started("y"));

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();

		arm.plugin.Gizmo.update();

		if (Context.lastCombo != null || (ui.tooltipImg == null && Context.lastTooltip != null)) App.redrawUI();
		Context.lastCombo = ui.comboSelectedHandle;
		Context.lastTooltip = ui.tooltipImg;
	}

	public function render(g: kha.graphics2.Graphics) {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (!App.uiEnabled && ui.inputRegistered) ui.unregisterInput();
		if (App.uiEnabled && !ui.inputRegistered) ui.registerInput();

		if (!show) return;

		g.end();
		ui.begin(g);

		UIToolbar.inst.renderUI(g);
		UIMenubar.inst.renderUI(g);
		UIHeader.inst.renderUI(g);
		UIStatus.inst.renderUI(g);

		tabx = System.windowWidth() - windowW;
		if (tabh == 0) {
			tabh = tabh1 = tabh2 = Std.int(System.windowHeight() / 3);
		}

		if (ui.window(hwnd, tabx, 0, windowW, tabh)) {
			TabLayers.draw();
			TabHistory.draw();
			TabPlugins.draw();

			if (UIHeader.inst.worktab.position == SpaceRender) {
				TabOutliner.draw();
			}
		}
		if (ui.window(hwnd1, tabx, tabh, windowW, tabh1)) {
			Context.object = Context.paintObject;
			TabMaterials.draw();
			TabBrushes.draw();
			TabParticles.draw();

			if (UIHeader.inst.worktab.position == SpaceRender) {
				TabProperties.draw();
				#if arm_creator
				TabTraits.draw();
				#end
			}
		}
		if (ui.window(hwnd2, tabx, tabh + tabh1, windowW, tabh2)) {
			TabTextures.draw();
			TabMeshes.draw();
			TabFonts.draw();
		}

		ui.end();
		g.begin(false);
	}

	public function renderCursor(g: kha.graphics2.Graphics) {
		g.color = 0xffffffff;

		// Brush
		if (App.uiEnabled && UIHeader.inst.worktab.position == SpacePaint) {
			var mx = App.x() + Context.paintVec.x * App.w();
			var my = App.y() + Context.paintVec.y * App.h();

			// Radius being scaled
			if (Context.brushLocked) {
				mx += Context.lockStartedX - System.windowWidth() / 2;
				my += Context.lockStartedY - System.windowHeight() / 2;
			}

			if (Context.brushStencilImage != null && Context.tool != ToolBake && Context.tool != ToolPicker && Context.tool != ToolColorId) {
				var r = getBrushStencilRect();
				if (!Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown)) {
					g.color = 0x88ffffff;
					g.pushRotation(-Context.brushStencilAngle, r.x + r.w / 2, r.y + r.h / 2);
					g.drawScaledImage(Context.brushStencilImage, r.x, r.y, r.w, r.h);
					g.popTransformation();
					g.color = 0xffffffff;
				}
				var transform = Operator.shortcut(Config.keymap.stencil_transform, ShortcutDown);
				if (transform) {
					// Outline
					g.drawRect(r.x, r.y, r.w, r.h);
					// Scale
					g.drawRect(r.x - 8,       r.y - 8,       16, 16);
					g.drawRect(r.x - 8 + r.w, r.y - 8,       16, 16);
					g.drawRect(r.x - 8,       r.y - 8 + r.h, 16, 16);
					g.drawRect(r.x - 8 + r.w, r.y - 8 + r.h, 16, 16);
					// Rotate
					g.pushRotation(-Context.brushStencilAngle, r.x + r.w / 2, r.y + r.h / 2);
					kha.graphics2.GraphicsExtension.fillCircle(g, r.x + r.w / 2, r.y - 4, 8);
					g.popTransformation();
				}
			}

			// Show picked material next to cursor
			if (Context.tool == ToolPicker && Context.pickerSelectMaterial) {
				var img = Context.material.imageIcon;
				#if kha_opengl
				g.drawScaledImage(img, mx + 10, my + 10 + img.height, img.width, -img.height);
				#else
				g.drawImage(img, mx + 10, my + 10);
				#end
			}

			var cursorImg = Res.get("cursor.k");
			var psize = Std.int(cursorImg.width * (Context.brushRadius * Context.brushNodesRadius) * ui.SCALE());

			// Clone source cursor
			var mouse = Input.getMouse();
			var pen = Input.getPen();
			var kb = Input.getKeyboard();
			if (Context.tool == ToolClone && !kb.down("alt") && (mouse.down() || pen.down())) {
				g.color = 0x66ffffff;
				g.drawScaledImage(cursorImg, mx + Context.cloneDeltaX * iron.App.w() - psize / 2, my + Context.cloneDeltaY * iron.App.h() - psize / 2, psize, psize);
				g.color = 0xffffffff;
			}

			var in2dView = UIView2D.inst.show && UIView2D.inst.type == View2DLayer &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var inNodes = UINodes.inst.show &&
						  mx > UINodes.inst.wx && mx < UINodes.inst.wx + UINodes.inst.ww &&
						  my > UINodes.inst.wy && my < UINodes.inst.wy + UINodes.inst.wh;
			var decal = Context.tool == ToolDecal || Context.tool == ToolText;

			if (!Config.raw.brush_3d || in2dView || (decal && !Config.raw.brush_live)) {
				if (decal && !inNodes) {
					var psizex = Std.int(256 * ui.SCALE() * (Context.brushRadius * Context.brushNodesRadius * Context.brushScaleX));
					var psizey = Std.int(256 * ui.SCALE() * (Context.brushRadius * Context.brushNodesRadius));
					g.color = kha.Color.fromFloats(1, 1, 1, Context.brushOpacity);
					var angle = (Context.brushAngle + Context.brushNodesAngle) * (Math.PI / 180);
					g.pushRotation(-angle, mx, my);
					#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
					g.drawScaledImage(Context.decalImage, mx - psizex / 2, my - psizey / 2, psizex, psizey);
					#else
					g.drawScaledImage(Context.decalImage, mx - psizex / 2, my - psizey / 2 + psizey, psizex, -psizey);
					#end
					g.popTransformation();
					g.color = 0xffffffff;
				}
				else if (Context.tool == ToolBrush  ||
						 Context.tool == ToolEraser ||
						 Context.tool == ToolClone  ||
						 Context.tool == ToolBlur   ||
						 Context.tool == ToolParticle) {
						g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
				}
			}

			if (Context.brushLazyRadius > 0 && !Context.brushLocked &&
				(Context.tool == ToolBrush ||
				 Context.tool == ToolEraser ||
				 Context.tool == ToolDecal ||
				 Context.tool == ToolText ||
				 Context.tool == ToolClone ||
				 Context.tool == ToolBlur ||
				 Context.tool == ToolParticle)) {
				g.fillRect(mx - 1, my - 1, 2, 2);
				var mx = Context.brushLazyX * App.w() + App.x();
				var my = Context.brushLazyY * App.h() + App.y();
				var radius = Context.brushLazyRadius * 180;
				g.color = 0xff666666;
				g.drawScaledImage(cursorImg, mx - radius / 2, my - radius / 2, radius, radius);
				g.color = 0xffffffff;
			}
		}
	}

	public function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();
		UINodes.inst.show = !UINodes.inst.show || UINodes.inst.canvasType != CanvasMaterial;
		UINodes.inst.canvasType = CanvasMaterial;
		App.resize();
	}

	public function showBrushNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();
		UINodes.inst.show = !UINodes.inst.show || UINodes.inst.canvasType != CanvasBrush;
		UINodes.inst.canvasType = CanvasBrush;
		App.resize();
	}

	public function show2DView(type: View2DType) {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UIView2D.inst.ui.endInput();
		if (UIView2D.inst.type != type) UIView2D.inst.show = true;
		else UIView2D.inst.show = !UIView2D.inst.show;
		UIView2D.inst.type = type;
		UIView2D.inst.hwnd.redraws = 2;
		App.resize();
	}

	public function toggleBrowser() {
		var minimized = UIStatus.inst.statush <= UIStatus.defaultStatusH * Config.raw.window_scale;
		UIStatus.inst.statush = minimized ? 240 : UIStatus.defaultStatusH;
		UIStatus.inst.statush = Std.int(UIStatus.inst.statush * Config.raw.window_scale);
	}

	public function getImage(asset: TAsset): Image {
		return asset != null ? Project.assetMap.get(asset.id) : null;
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
		if (handle != hwnd &&
			handle != hwnd1 &&
			handle != hwnd2 &&
			handle != UIStatus.inst.statusHandle &&
			handle != UINodes.inst.hwnd &&
			handle != UIView2D.inst.hwnd) return; // Scalable handles
		if (handle == UINodes.inst.hwnd && side != SideLeft && side != SideTop) return;
		if (handle == UINodes.inst.hwnd && side == SideTop && !UIView2D.inst.show) return;
		if (handle == UIView2D.inst.hwnd && side != SideLeft) return;
		if (handle == hwnd && side == SideTop) return;
		if (handle == hwnd2 && side == SideBottom) return;
		if (handle == UIStatus.inst.statusHandle && side != SideTop) return;
		if (side == SideRight) return; // UI is snapped to the right side

		side == SideLeft || side == SideRight ?
			Krom.setMouseCursor(6) : // Horizontal
			Krom.setMouseCursor(5);  // Vertical

		if (ui.inputStarted) {
			borderStarted = side;
			borderHandle = handle;
			App.isResizing = true;
		}
	}

	function onTextHover() {
		Krom.setMouseCursor(3); // I-cursor
	}

	public function tagUIRedraw() {
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		UIStatus.inst.statusHandle.redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
		hwnd.redraws = 2;
		hwnd1.redraws = 2;
		hwnd2.redraws = 2;
	}
}
