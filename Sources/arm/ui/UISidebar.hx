package arm.ui;

import haxe.io.Bytes;
import kha.Image;
import kha.System;
import kha.input.KeyCode;
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
import arm.node.MakeMaterial;
import arm.util.RenderUtil;
import arm.Viewport;
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
	public static inline var defaultWindowW = 280;
	public var tabx = 0;
	public var show = true;
	public var ui: Zui;
	public var hwnd0 = Id.handle();
	public var hwnd1 = Id.handle();
	public var htab0 = Id.handle();
	public var htab1 = Id.handle();
	public var hminimize = Id.handle();
	var borderStarted = 0;
	var borderHandle: Handle = null;

	public function new() {
		inst = this;
		new UIToolbar();
		new UIHeader();
		new UIStatus();
		new UIMenubar();

		Context.textToolText = tr("Text");

		UIToolbar.inst.toolbarw = Std.int(UIToolbar.defaultToolbarW * Config.raw.window_scale);
		UIHeader.inst.headerh = Std.int(UIHeader.defaultHeaderH * Config.raw.window_scale);
		UIMenubar.inst.menubarw = Std.int(UIMenubar.defaultMenubarW * Config.raw.window_scale);

		if (Project.materials == null) {
			Project.materials = [];
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				Project.materials.push(new MaterialSlot(m));
				Context.material = Project.materials[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(new BrushSlot());
			Context.brush = Project.brushes[0];
			MakeMaterial.parseBrush();
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

		var resources = ["cursor.k", "icons.k"];
		Res.load(resources, done);

		Context.projectObjects = [];
		for (m in Scene.active.meshes) Context.projectObjects.push(m);
	}

	function done() {
		if (ui.SCALE() > 1) setIconScale();

		Context.gizmo = Scene.active.getChild(".Gizmo");
		Context.gizmoTranslateX = Context.gizmo.getChild(".TranslateX");
		Context.gizmoTranslateY = Context.gizmo.getChild(".TranslateY");
		Context.gizmoTranslateZ = Context.gizmo.getChild(".TranslateZ");
		Context.gizmoScaleX = Context.gizmo.getChild(".ScaleX");
		Context.gizmoScaleY = Context.gizmo.getChild(".ScaleY");
		Context.gizmoScaleZ = Context.gizmo.getChild(".ScaleZ");
		Context.gizmoRotateX = Context.gizmo.getChild(".RotateX");
		Context.gizmoRotateY = Context.gizmo.getChild(".RotateY");
		Context.gizmoRotateZ = Context.gizmo.getChild(".RotateZ");

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

		if (!UINodes.inst.ui.isTyping && !ui.isTyping) {
			if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
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
				Context.layersExport = ExportVisible;
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
			Context.layersExport = ExportVisible;
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
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);

		if ((Context.brushCanLock || Context.brushLocked) && mouse.moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_angle, ShortcutDown) ||
				(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutDown))) {
				if (Context.brushLocked) {
					if (Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown)) {
						Context.brushOpacity += mouse.movementX / 500;
						Context.brushOpacity = Math.max(0.0, Math.min(1.0, Context.brushOpacity));
						Context.brushOpacity = Math.round(Context.brushOpacity * 100) / 100;
						Context.brushOpacityHandle.value = Context.brushOpacity;
					}
					else if (Operator.shortcut(Config.keymap.brush_angle, ShortcutDown)) {
						Context.brushAngle += mouse.movementX / 5;
						Context.brushAngle = Std.int(Context.brushAngle) % 360;
						if (Context.brushAngle < 0) Context.brushAngle += 360;
						Context.brushAngleHandle.value = Context.brushAngle;
						MakeMaterial.parsePaintMaterial();
					}
					else if (decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutDown)) {
						Context.brushDecalMaskRadius += mouse.movementX / 150;
						Context.brushDecalMaskRadius = Math.max(0.01, Math.min(4.0, Context.brushDecalMaskRadius));
						Context.brushDecalMaskRadius = Math.round(Context.brushDecalMaskRadius * 100) / 100;
						Context.brushDecalMaskRadiusHandle.value = Context.brushDecalMaskRadius;
					}
					else {
						Context.brushRadius += mouse.movementX / 150;
						Context.brushRadius = Math.max(0.01, Math.min(4.0, Context.brushRadius));
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
		if (UIView2D.inst.show) right += UIView2D.inst.ww;

		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);

		var isTyping = ui.isTyping || UIView2D.inst.ui.isTyping || UINodes.inst.ui.isTyping;
		if (!isTyping) {
			if (Operator.shortcut(Config.keymap.select_material, ShortcutDown)) {
				UISidebar.inst.hwnd1.redraws = 2;
				for (i in 1...10) if (kb.started(i + "")) Context.selectMaterial(i - 1);
			}
			else if (Operator.shortcut(Config.keymap.select_layer, ShortcutDown)) {
				UISidebar.inst.hwnd0.redraws = 2;
				for (i in 1...10) if (kb.started(i + "")) Context.selectLayer(i - 1);
			}
		}

		// Viewport shortcuts
		var inViewport = mouse.viewX > 0 && mouse.viewX < right &&
						 mouse.viewY > 0 && mouse.viewY < iron.App.h();
		if (inViewport && !isTyping) {
			if (UIHeader.inst.worktab.position == SpacePaint) {
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
						Operator.shortcut(Config.keymap.brush_angle) ||
						(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius))) {
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
					else if (decalMask) {
						if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_decrease, ShortcutRepeat)) {
							Context.brushDecalMaskRadius -= getRadiusIncrement();
							Context.brushDecalMaskRadius = Math.max(Math.round(Context.brushDecalMaskRadius * 100) / 100, 0.01);
							Context.brushDecalMaskRadiusHandle.value = Context.brushDecalMaskRadius;
							UIHeader.inst.headerHandle.redraws = 2;
						}
						else if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_increase, ShortcutRepeat)) {
							Context.brushDecalMaskRadius += getRadiusIncrement();
							Context.brushDecalMaskRadius = Math.round(Context.brushDecalMaskRadius * 100) / 100;
							Context.brushDecalMaskRadiusHandle.value = Context.brushDecalMaskRadius;
							UIHeader.inst.headerHandle.redraws = 2;
						}
					}
				}

				if (decalMask && (Operator.shortcut(Config.keymap.decal_mask, ShortcutStarted) || Operator.shortcut(Config.keymap.decal_mask, ShortcutReleased))) {
					UIHeader.inst.headerHandle.redraws = 2;
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
							tr("Height"),
							tr("Emission"),
							tr("Subsurface"),
							tr("TexCoord"),
							tr("Object Normal"),
							tr("Material ID"),
							tr("Object ID"),
							tr("Mask")
						];
						var shortcuts = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];
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
					}, 16 #if (kha_direct3d12 || kha_vulkan) + 1 #end );
				}
			}
		}

		if (Context.brushCanLock || Context.brushLocked) {
			if (mouse.moved && Context.brushCanUnlock) {
				Context.brushLocked = false;
				Context.brushCanUnlock = false;
			}
			if ((Context.brushCanLock || Context.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_angle, ShortcutDown) &&
				!(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutDown))) {
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
			if (borderHandle == UINodes.inst.hwnd || borderHandle == UIView2D.inst.hwnd) {
				if (borderStarted == SideLeft) {
					Config.raw.layout[LayoutNodesW] -= Std.int(mouse.movementX);
					if (Config.raw.layout[LayoutNodesW] < 32) Config.raw.layout[LayoutNodesW] = 32;
					else if (Config.raw.layout[LayoutNodesW] > System.windowWidth() * 0.7) Config.raw.layout[LayoutNodesW] = Std.int(System.windowWidth() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[LayoutNodesH] -= Std.int(mouse.movementY);
					if (Config.raw.layout[LayoutNodesH] < 32) Config.raw.layout[LayoutNodesH] = 32;
					else if (Config.raw.layout[LayoutNodesH] > iron.App.h() * 0.95) Config.raw.layout[LayoutNodesH] = Std.int(iron.App.h() * 0.95);
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
					Config.raw.layout[LayoutSidebarW] -= Std.int(mouse.movementX);
					if (Config.raw.layout[LayoutSidebarW] < 32) Config.raw.layout[LayoutSidebarW] = 32;
					else if (Config.raw.layout[LayoutSidebarW] > System.windowWidth() - 32) Config.raw.layout[LayoutSidebarW] = System.windowWidth() - 32;
				}
				else {
					var my = Std.int(mouse.movementY);
					if (borderHandle == hwnd1 && borderStarted == SideTop) {
						if (Config.raw.layout[LayoutSidebarH0] + my > 32 && Config.raw.layout[LayoutSidebarH1] - my > 32) {
							Config.raw.layout[LayoutSidebarH0] += my;
							Config.raw.layout[LayoutSidebarH1] -= my;
						}
					}
				}
			}
		}
		if (!mouse.down()) {
			borderHandle = null;
			App.isResizing = false;
		}

		#if arm_physics
		if (Context.tool == ToolParticle && Context.particlePhysics && inViewport && !Context.paint2d) {
			arm.util.ParticleUtil.initParticlePhysics();
			var world = arm.plugin.PhysicsWorld.active;
			world.lateUpdate();
			Context.ddirty = 2;
			Context.rdirty = 2;
			if (mouse.started()) {
				if (Context.particleTimer != null) {
					iron.system.Tween.stop(Context.particleTimer);
					Context.particleTimer.done();
					Context.particleTimer = null;
				}
				History.pushUndo = true;
				Context.particleHitX = Context.particleHitY = Context.particleHitZ = 0;
				Scene.active.spawnObject(".Sphere", null, function(o: Object) {
					iron.data.Data.getMaterial("Scene", ".Gizmo", function(md: MaterialData) {
						var mo: MeshObject = cast o;
						mo.name = ".Bullet";
						mo.materials[0] = md;
						mo.visible = true;

						var camera = iron.Scene.active.camera;
						var ct = camera.transform;
						mo.transform.loc.set(ct.worldx(), ct.worldy(), ct.worldz());
						mo.transform.scale.set(Context.brushRadius * 0.2, Context.brushRadius * 0.2, Context.brushRadius * 0.2);
						mo.transform.buildMatrix();

						var body = new arm.plugin.PhysicsBody();
						body.shape = arm.plugin.PhysicsBody.ShapeType.ShapeSphere;
						body.mass = 1.0;
						body.ccd = true;
						mo.transform.radius /= 10; // Lower ccd radius
						mo.addTrait(body);
						mo.transform.radius *= 10;

						var ray = iron.math.RayCaster.getRay(mouse.viewX, mouse.viewY, camera);
						body.applyImpulse(ray.direction.mult(0.15));

						Context.particleTimer = iron.system.Tween.timer(5, mo.remove);
					});
				});
			}

			var pairs = world.getContactPairs(Context.paintBody);
			if (pairs != null) {
				for (p in pairs) {
					Context.lastParticleHitX = Context.particleHitX != 0 ? Context.particleHitX : p.posA.x;
					Context.lastParticleHitY = Context.particleHitY != 0 ? Context.particleHitY : p.posA.y;
					Context.lastParticleHitZ = Context.particleHitZ != 0 ? Context.particleHitZ : p.posA.z;
					Context.particleHitX = p.posA.x;
					Context.particleHitY = p.posA.y;
					Context.particleHitZ = p.posA.z;
					Context.pdirty = 1;
					break; // 1 pair for now
				}
			}
		}
		#end
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

		if (Console.messageTimer > 0) {
			Console.messageTimer -= Time.delta;
			if (Console.messageTimer <= 0) UIStatus.inst.statusHandle.redraws = 2;
		}

		if (!App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

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

		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
		var setCloneSource = Context.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		var down = Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
				   decalMask ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
				   (Input.getPen().down() && !kb.down("alt"));

		#if (krom_android || krom_ios)
		if (Input.getPen().down()) {
			Context.penPaintingOnly = true;
		}
		else if (Context.penPaintingOnly) {
			down = false;
		}
		#end


		#if arm_physics
		if (Context.tool == ToolParticle && Context.particlePhysics) {
			down = false;
		}
		#end

		#if krom_ios
		// No hover on iPad, decals are painted by pen release
		if (decal) {
			down = Input.getPen().released();
			if (!Context.penPaintingOnly) {
				down = down || Input.getMouse().released();
			}
		}
		#end

		if (down) {
			var mx = mouse.viewX;
			var my = mouse.viewY;
			var ww = iron.App.w();
			if (Context.paint2d) {
				mx -= iron.App.w();
				ww = UIView2D.inst.ww;
			}

			if (mx < ww &&
				mx > iron.App.x() &&
				my < iron.App.h() &&
				my > iron.App.y()) {

				if (setCloneSource) {
					Context.cloneStartX = mx;
					Context.cloneStartY = my;
				}
				else {
					if (Context.brushTime == 0 &&
						!App.isDragging &&
						!App.isResizing &&
						!App.isComboSelected()) { // Paint started

						// Draw line
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
							Context.lastPaintVecX = Context.lastPaintX;
							Context.lastPaintVecY = Context.lastPaintY;
						}

						History.pushUndo = true;

						if (Context.tool == ToolClone && Context.cloneStartX >= 0.0) { // Clone delta
							Context.cloneDeltaX = (Context.cloneStartX - mx) / ww;
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
						else if (Context.tool == ToolFill && Context.fillTypeHandle.position == FillUVIsland) {
							UVUtil.uvislandmapCached = false;
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

			// New color id picked, update fill layer
			if (Context.tool == ToolColorId && Context.layer.fill_layer != null) {
				App.notifyOnNextFrame(function() {
					Layers.updateFillLayer();
					MakeMaterial.parsePaintMaterial(false);
				});
			}
		}

		if (Context.layersPreviewDirty) {
			Context.layersPreviewDirty = false;
			Context.layerPreviewDirty = false;
			Context.maskPreviewLast = null;
			if (Layers.pipeMerge == null) Layers.makePipe();
			// Update all layer previews
			for (l in Project.layers) {
				if (l.isGroup()) continue;
				var target = l.texpaint_preview;
				var source = l.texpaint;
				var g2 = target.g2;
				g2.begin(true, 0x00000000);
				g2.pipeline = l.isMask() ? Layers.pipeCopy8 : Layers.pipeCopy;
				g2.drawScaledImage(source, 0, 0, target.width, target.height);
				g2.pipeline = null;
				g2.end();
			}
			hwnd0.redraws = 2;
		}
		if (Context.layerPreviewDirty && !Context.layer.isGroup()) {
			Context.layerPreviewDirty = false;
			Context.maskPreviewLast = null;
			if (Layers.pipeMerge == null) Layers.makePipe();
			// Update layer preview
			var l = Context.layer;
			var target = l.texpaint_preview;
			var source = l.texpaint;
			var g2 = target.g2;
			g2.begin(true, 0x00000000);
			g2.pipeline = Context.layer.isMask() ? Layers.pipeCopy8 : Layers.pipeCopy;
			g2.drawScaledImage(source, 0, 0, target.width, target.height);
			g2.pipeline = null;
			g2.end();
			hwnd0.redraws = 2;
		}

		var undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		var redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (kb.down("control") && kb.started("y"));

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();

		arm.render.Gizmo.update();
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

		UIToolbar.inst.renderUI(g);
		UIMenubar.inst.renderUI(g);
		UIHeader.inst.renderUI(g);
		UIStatus.inst.renderUI(g);

		tabx = System.windowWidth() - Config.raw.layout[LayoutSidebarW];
		if (ui.window(hwnd0, tabx, 0, Config.raw.layout[LayoutSidebarW], Config.raw.layout[LayoutSidebarH0])) {
			TabLayers.draw();
			TabHistory.draw();
			TabPlugins.draw();
		}
		if (ui.window(hwnd1, tabx, Config.raw.layout[LayoutSidebarH0], Config.raw.layout[LayoutSidebarW], Config.raw.layout[LayoutSidebarH1])) {
			TabMaterials.draw();
			TabBrushes.draw();
			TabParticles.draw();
		}
		if (Config.raw.layout[LayoutSidebarW] == 0) {
			var width = Std.int(ui.ops.font.width(ui.fontSize, "<<") + 25 * ui.SCALE());
			if (ui.window(hminimize, System.windowWidth() - width, 0, width, Std.int(ui.BUTTON_H()))) {
				ui._w = width;
				if (ui.button("<<"))
					Config.raw.layout[LayoutSidebarW] = Context.maximizedSidebarWidth != 0 ? Context.maximizedSidebarWidth : Std.int(UISidebar.defaultWindowW * Config.raw.window_scale);
			}
		}
		if (htab0.changed && (htab0.position == Context.lastHtab0Position) && Config.raw.layout[LayoutSidebarW] != 0) {
			Context.maximizedSidebarWidth = Config.raw.layout[LayoutSidebarW];
			Config.raw.layout[LayoutSidebarW] = 0 ;
		}
		Context.lastHtab0Position = htab0.position;
		ui.end();
		g.begin(false);
	}

	public function renderCursor(g: kha.graphics2.Graphics) {
		g.color = 0xffffffff;

		// Brush
		if (App.uiEnabled && UIHeader.inst.worktab.position == SpacePaint) {
			Context.viewIndex = Context.viewIndexLast;
			var mx = App.x() + Context.paintVec.x * App.w();
			var my = App.y() + Context.paintVec.y * App.h();
			Context.viewIndex = -1;

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
			if (Context.tool == ToolPicker && Context.pickerSelectMaterial && Context.colorPickerCallback == null) {
				var img = Context.material.imageIcon;
				#if kha_opengl
				g.drawScaledImage(img, mx + 10, my + 10 + img.height, img.width, -img.height);
				#else
				g.drawImage(img, mx + 10, my + 10);
				#end
			}
			if (Context.tool == ToolPicker && Context.colorPickerCallback != null) {
				var img = Res.get("icons.k");
				var rect = Res.tile50(img, ToolPicker, 0);
					
				g.drawSubImage(img, mx + 10, my + 10, rect.x, rect.y, rect.w, rect.h);
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

			if (!Config.raw.brush_3d || in2dView || decal) {
				var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);
				if (decal && !inNodes) {
					var decalAlpha = 0.5;
					if (!decalMask) {
						Context.decalX = Context.paintVec.x;
						Context.decalY = Context.paintVec.y;
						decalAlpha = Context.brushOpacity;

						// Radius being scaled
						if (Context.brushLocked) {
							Context.decalX += (Context.lockStartedX - System.windowWidth() / 2) / App.w();
							Context.decalY += (Context.lockStartedY - System.windowHeight() / 2) / App.h();
						}
					}

					if (!Config.raw.brush_live) {
						var psizex = Std.int(256 * ui.SCALE() * (Context.brushRadius * Context.brushNodesRadius * Context.brushScaleX));
						var psizey = Std.int(256 * ui.SCALE() * (Context.brushRadius * Context.brushNodesRadius));

						Context.viewIndex = Context.viewIndexLast;
						var decalX = App.x() + Context.decalX * App.w() - psizex / 2;
						var decalY = App.y() + Context.decalY * App.h() - psizey / 2;
						Context.viewIndex = -1;

						g.color = kha.Color.fromFloats(1, 1, 1, decalAlpha);
						var angle = (Context.brushAngle + Context.brushNodesAngle) * (Math.PI / 180);
						g.pushRotation(angle, decalX + psizex / 2, decalY + psizey / 2);
						#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
						g.drawScaledImage(Context.decalImage, decalX, decalY, psizex, psizey);
						#else
						g.drawScaledImage(Context.decalImage, decalX, decalY + psizey, psizex, -psizey);
						#end
						g.popTransformation();
						g.color = 0xffffffff;
					}
				}
				if (Context.tool == ToolBrush  ||
					Context.tool == ToolEraser ||
					Context.tool == ToolClone  ||
					Context.tool == ToolBlur   ||
					Context.tool == ToolParticle ||
					(decalMask && !Config.raw.brush_3d) ||
					(decalMask && in2dView)) {
					if (decalMask) {
						psize = Std.int(cursorImg.width * (Context.brushDecalMaskRadius * Context.brushNodesRadius) * ui.SCALE());
					}
					if (Config.raw.brush_3d && in2dView) {
						psize = Std.int(psize * UIView2D.inst.panScale);
					}
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
		if (handle != hwnd0 &&
			handle != hwnd1 &&
			handle != UIStatus.inst.statusHandle &&
			handle != UINodes.inst.hwnd &&
			handle != UIView2D.inst.hwnd) return; // Scalable handles
		if (handle == UINodes.inst.hwnd && side != SideLeft && side != SideTop) return;
		if (handle == UINodes.inst.hwnd && side == SideTop && !UIView2D.inst.show) return;
		if (handle == UIView2D.inst.hwnd && side != SideLeft) return;
		if (handle == hwnd0 && side == SideTop) return;
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

	public function tagUIRedraw() {
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		UIStatus.inst.statusHandle.redraws = 2;
		UIMenubar.inst.workspaceHandle.redraws = 2;
		UIMenubar.inst.menuHandle.redraws = 2;
		hwnd0.redraws = 2;
		hwnd1.redraws = 2;
	}
}
