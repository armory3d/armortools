package arm.ui;

import haxe.io.Bytes;
import kha.input.KeyCode;
import kha.Image;
import kha.System;
import zui.Zui;
import zui.Id;
import iron.data.Data;
import iron.data.MaterialData;
import iron.object.MeshObject;
import iron.system.Input;
import iron.system.Time;
import iron.Scene;
import arm.io.ExportTexture;
import arm.ProjectFormat;
import arm.Viewport;
import arm.Res;
#if (is_paint || is_sculpt)
import kha.math.FastMatrix3;
import iron.object.Object;
import arm.shader.MakeMaterial;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.data.MaterialSlot;
#end
#if is_lab
import kha.Blob;
import zui.Nodes;
#end

@:access(zui.Zui)
class UIBase {

	public static var inst: UIBase;
	public var show = true;
	public var ui: Zui;
	var borderStarted = 0;
	var borderHandle: Handle = null;
	var action_paint_remap = "";
	var operatorSearchOffset = 0;

	#if (is_paint || is_sculpt)
	public var hwnds = [Id.handle(), Id.handle(), Id.handle()];
	public var htabs = [Id.handle(), Id.handle(), Id.handle()];
	public var hwndTabs = [
		[TabLayers.draw, TabHistory.draw, TabPlugins.draw #if is_forge , TabObjects.draw #end],
		[TabMaterials.draw, TabBrushes.draw, TabParticles.draw],
		[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabSwatches.draw, TabScript.draw, TabConsole.draw]
	];
	#end
	#if is_lab
	public var hwnds = [Id.handle()];
	public var htabs = [Id.handle()];
	public var hwndTabs = [
		[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabSwatches.draw, TabPlugins.draw, TabScript.draw, TabConsole.draw]
	];
	#end

	#if (is_paint || is_sculpt)
	public static inline var defaultWindowW = 280;
	public var tabx = 0;
	public var hminimized = Id.handle();
	#end

	public function new() {
		inst = this;

		#if (is_paint || is_sculpt)
		new UIToolbar();
		UIToolbar.inst.toolbarw = Std.int(UIToolbar.defaultToolbarW * Config.raw.window_scale);
		Context.raw.textToolText = tr("Text");
		#end

		new UIHeader();
		new UIStatus();
		new UIMenubar();

		UIHeader.headerh = Std.int(UIHeader.defaultHeaderH * Config.raw.window_scale);
		UIMenubar.inst.menubarw = Std.int(UIMenubar.defaultMenubarW * Config.raw.window_scale);

		#if (is_paint || is_sculpt)
		if (Project.materials == null) {
			Project.materials = [];
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				Project.materials.push(new MaterialSlot(m));
				Context.raw.material = Project.materials[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(new BrushSlot());
			Context.raw.brush = Project.brushes[0];
			MakeMaterial.parseBrush();
		}

		if (Project.fonts == null) {
			Project.fonts = [];
			Project.fonts.push(new FontSlot("default.ttf", App.font));
			Context.raw.font = Project.fonts[0];
		}

		if (Project.layers == null) {
			Project.layers = [];
			Project.layers.push(new LayerSlot());
			Context.raw.layer = Project.layers[0];
		}
		#end

		#if is_lab
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
		#end

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

		#if (is_paint || is_sculpt)
		var resources = ["cursor.k", "icons.k"];
		#end
		#if is_lab
		var resources = ["cursor.k", "icons.k", "placeholder.k"];
		#end

		#if (is_paint || is_sculpt)
		Context.raw.gizmo = Scene.active.getChild(".Gizmo");
		Context.raw.gizmoTranslateX = Context.raw.gizmo.getChild(".TranslateX");
		Context.raw.gizmoTranslateY = Context.raw.gizmo.getChild(".TranslateY");
		Context.raw.gizmoTranslateZ = Context.raw.gizmo.getChild(".TranslateZ");
		Context.raw.gizmoScaleX = Context.raw.gizmo.getChild(".ScaleX");
		Context.raw.gizmoScaleY = Context.raw.gizmo.getChild(".ScaleY");
		Context.raw.gizmoScaleZ = Context.raw.gizmo.getChild(".ScaleZ");
		Context.raw.gizmoRotateX = Context.raw.gizmo.getChild(".RotateX");
		Context.raw.gizmoRotateY = Context.raw.gizmo.getChild(".RotateY");
		Context.raw.gizmoRotateZ = Context.raw.gizmo.getChild(".RotateZ");
		#end

		Res.load(resources, function() {});

		if (ui.SCALE() > 1) setIconScale();

		Context.raw.paintObject = cast(Scene.active.getChild(".Cube"), MeshObject);
		Project.paintObjects = [Context.raw.paintObject];

		if (Project.filepath == "") {
			iron.App.notifyOnInit(App.initLayers);
		}

		Context.raw.projectObjects = [];
		for (m in Scene.active.meshes) Context.raw.projectObjects.push(m);

		Operator.register("view_top", view_top);
	}

	public function update() {
		updateUI();
		Operator.update();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		if (!App.uiEnabled) return;

		if (!UINodes.inst.ui.isTyping && !ui.isTyping) {
			if (Operator.shortcut(Config.keymap.toggle_node_editor)) {
				#if (is_paint || is_sculpt)
				UINodes.inst.canvasType == CanvasMaterial ? showMaterialNodes() : showBrushNodes();
				#end
				#if is_lab
				showMaterialNodes();
				#end
			}
			else if (Operator.shortcut(Config.keymap.toggle_browser)) {
				toggleBrowser();
			}

			#if (is_paint || is_sculpt)
			else if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
				show2DView(View2DLayer);
			}
			#end
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.projectSaveAs();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.projectSave();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.projectOpen();
		else if (Operator.shortcut(Config.keymap.file_open_recent)) BoxProjects.show();
		else if (Operator.shortcut(Config.keymap.file_reimport_mesh)) Project.reimportMesh();
		else if (Operator.shortcut(Config.keymap.file_reimport_textures)) Project.reimportTextures();
		else if (Operator.shortcut(Config.keymap.file_new)) Project.projectNewBox();
		#if (is_paint || is_lab)
		else if (Operator.shortcut(Config.keymap.file_export_textures)) {
			if (Context.raw.textureExportPath == "") { // First export, ask for path
				#if is_paint
				Context.raw.layersExport = ExportVisible;
				#end
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
			#if (is_paint || is_sculpt)
			Context.raw.layersExport = ExportVisible;
			#end
			BoxExport.showTextures();
		}
		#end
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

		#if (is_paint || is_sculpt)
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);

		if ((Context.raw.brushCanLock || Context.raw.brushLocked) && mouse.moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_angle, ShortcutDown) ||
				(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutDown))) {
				if (Context.raw.brushLocked) {
					if (Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown)) {
						Context.raw.brushOpacity += mouse.movementX / 500;
						Context.raw.brushOpacity = Math.max(0.0, Math.min(1.0, Context.raw.brushOpacity));
						Context.raw.brushOpacity = Math.round(Context.raw.brushOpacity * 100) / 100;
						Context.raw.brushOpacityHandle.value = Context.raw.brushOpacity;
					}
					else if (Operator.shortcut(Config.keymap.brush_angle, ShortcutDown)) {
						Context.raw.brushAngle += mouse.movementX / 5;
						Context.raw.brushAngle = Std.int(Context.raw.brushAngle) % 360;
						if (Context.raw.brushAngle < 0) Context.raw.brushAngle += 360;
						Context.raw.brushAngleHandle.value = Context.raw.brushAngle;
						MakeMaterial.parsePaintMaterial();
					}
					else if (decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutDown)) {
						Context.raw.brushDecalMaskRadius += mouse.movementX / 150;
						Context.raw.brushDecalMaskRadius = Math.max(0.01, Math.min(4.0, Context.raw.brushDecalMaskRadius));
						Context.raw.brushDecalMaskRadius = Math.round(Context.raw.brushDecalMaskRadius * 100) / 100;
						Context.raw.brushDecalMaskRadiusHandle.value = Context.raw.brushDecalMaskRadius;
					}
					else {
						Context.raw.brushRadius += mouse.movementX / 150;
						Context.raw.brushRadius = Math.max(0.01, Math.min(4.0, Context.raw.brushRadius));
						Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
					}
					UIHeader.inst.headerHandle.redraws = 2;
				}
				else if (Context.raw.brushCanLock) {
					Context.raw.brushCanLock = false;
					Context.raw.brushLocked = true;
				}
			}
		}
		#end

		#if is_lab
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
		#end

		#if (is_paint || is_sculpt)
		var isTyping = ui.isTyping || UIView2D.inst.ui.isTyping || UINodes.inst.ui.isTyping;
		#end
		#if is_lab
		var isTyping = ui.isTyping || UINodes.inst.ui.isTyping;
		#end

		#if (is_paint || is_sculpt)
		if (!isTyping) {
			if (Operator.shortcut(Config.keymap.select_material, ShortcutDown)) {
				hwnds[TabSidebar1].redraws = 2;
				for (i in 1...10) if (kb.started(i + "")) Context.selectMaterial(i - 1);
			}
			else if (Operator.shortcut(Config.keymap.select_layer, ShortcutDown)) {
				hwnds[TabSidebar0].redraws = 2;
				for (i in 1...10) if (kb.started(i + "")) Context.selectLayer(i - 1);
			}
		}
		#end

		// Viewport shortcuts
		if (Context.inPaintArea() && !isTyping) {

			#if is_paint
			if (!mouse.down("right")) { // Fly mode off
				if (Operator.shortcut(Config.keymap.tool_brush)) Context.selectTool(ToolBrush);
				else if (Operator.shortcut(Config.keymap.tool_eraser)) Context.selectTool(ToolEraser);
				else if (Operator.shortcut(Config.keymap.tool_fill)) Context.selectTool(ToolFill);
				else if (Operator.shortcut(Config.keymap.tool_colorid)) Context.selectTool(ToolColorId);
				else if (Operator.shortcut(Config.keymap.tool_decal)) Context.selectTool(ToolDecal);
				else if (Operator.shortcut(Config.keymap.tool_text)) Context.selectTool(ToolText);
				else if (Operator.shortcut(Config.keymap.tool_clone)) Context.selectTool(ToolClone);
				else if (Operator.shortcut(Config.keymap.tool_blur)) Context.selectTool(ToolBlur);
				else if (Operator.shortcut(Config.keymap.tool_smudge)) Context.selectTool(ToolSmudge);
				else if (Operator.shortcut(Config.keymap.tool_particle)) Context.selectTool(ToolParticle);
				else if (Operator.shortcut(Config.keymap.tool_picker)) Context.selectTool(ToolPicker);
				else if (Operator.shortcut(Config.keymap.tool_bake)) Context.selectTool(ToolBake);
				else if (Operator.shortcut(Config.keymap.tool_gizmo)) Context.selectTool(ToolGizmo);
				else if (Operator.shortcut(Config.keymap.tool_material)) Context.selectTool(ToolMaterial);
				else if (Operator.shortcut(Config.keymap.swap_brush_eraser)) Context.selectTool(Context.raw.tool == ToolBrush ? ToolEraser : ToolBrush);
			}

			// Radius
			if (Context.raw.tool == ToolBrush  ||
				Context.raw.tool == ToolEraser ||
				Context.raw.tool == ToolDecal  ||
				Context.raw.tool == ToolText   ||
				Context.raw.tool == ToolClone  ||
				Context.raw.tool == ToolBlur   ||
				Context.raw.tool == ToolSmudge   ||
				Context.raw.tool == ToolParticle) {
				if (Operator.shortcut(Config.keymap.brush_radius) ||
					Operator.shortcut(Config.keymap.brush_opacity) ||
					Operator.shortcut(Config.keymap.brush_angle) ||
					(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius))) {
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
				else if (decalMask) {
					if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_decrease, ShortcutRepeat)) {
						Context.raw.brushDecalMaskRadius -= getRadiusIncrement();
						Context.raw.brushDecalMaskRadius = Math.max(Math.round(Context.raw.brushDecalMaskRadius * 100) / 100, 0.01);
						Context.raw.brushDecalMaskRadiusHandle.value = Context.raw.brushDecalMaskRadius;
						UIHeader.inst.headerHandle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_increase, ShortcutRepeat)) {
						Context.raw.brushDecalMaskRadius += getRadiusIncrement();
						Context.raw.brushDecalMaskRadius = Math.round(Context.raw.brushDecalMaskRadius * 100) / 100;
						Context.raw.brushDecalMaskRadiusHandle.value = Context.raw.brushDecalMaskRadius;
						UIHeader.inst.headerHandle.redraws = 2;
					}
				}
			}

			if (decalMask && (Operator.shortcut(Config.keymap.decal_mask, ShortcutStarted) || Operator.shortcut(Config.keymap.decal_mask, ShortcutReleased))) {
				UIHeader.inst.headerHandle.redraws = 2;
			}
			#end

			#if is_lab
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
			#end

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
							tr("Height"),
							#if (is_paint || is_sculpt)
							tr("Emission"),
							tr("Subsurface"),
							tr("TexCoord"),
							tr("Object Normal"),
							tr("Material ID"),
							tr("Object ID"),
							tr("Mask")
							#end
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

					#if (is_paint || is_sculpt)
					}, 16 #if (kha_direct3d12 || kha_vulkan) + 1 #end );
					#end
					#if is_lab
					}, 9 #if (kha_direct3d12 || kha_vulkan) + 1 #end );
					#end
				}
			}

			if (Operator.shortcut(Config.keymap.operator_search)) operatorSearch();
		}

		if (Context.raw.brushCanLock || Context.raw.brushLocked) {
			if (mouse.moved && Context.raw.brushCanUnlock) {
				Context.raw.brushLocked = false;
				Context.raw.brushCanUnlock = false;
			}

			#if (is_paint || is_sculpt)
			var b = (Context.raw.brushCanLock || Context.raw.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_opacity, ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_angle, ShortcutDown) &&
				!(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutDown));
			#end
			#if is_lab
			var b = (Context.raw.brushCanLock || Context.raw.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutDown);
			#end

			if (b) {
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

		#if (is_paint || is_sculpt)
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
			else if (borderHandle == hwnds[TabStatus]) {
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
					if (borderHandle == hwnds[TabSidebar1] && borderStarted == SideTop) {
						if (Config.raw.layout[LayoutSidebarH0] + my > 32 && Config.raw.layout[LayoutSidebarH1] - my > 32) {
							Config.raw.layout[LayoutSidebarH0] += my;
							Config.raw.layout[LayoutSidebarH1] -= my;
						}
					}
				}
			}
		}
		#end

		#if is_lab
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
		#end

		if (!mouse.down()) {
			borderHandle = null;
			App.isResizing = false;
		}

		#if arm_physics
		if (Context.raw.tool == ToolParticle && Context.raw.particlePhysics && Context.inPaintArea() && !Context.raw.paint2d) {
			arm.util.ParticleUtil.initParticlePhysics();
			var world = arm.plugin.PhysicsWorld.active;
			world.lateUpdate();
			Context.raw.ddirty = 2;
			Context.raw.rdirty = 2;
			if (mouse.started()) {
				if (Context.raw.particleTimer != null) {
					iron.system.Tween.stop(Context.raw.particleTimer);
					Context.raw.particleTimer.done();
					Context.raw.particleTimer = null;
				}
				History.pushUndo = true;
				Context.raw.particleHitX = Context.raw.particleHitY = Context.raw.particleHitZ = 0;
				Scene.active.spawnObject(".Sphere", null, function(o: Object) {
					iron.data.Data.getMaterial("Scene", ".Gizmo", function(md: MaterialData) {
						var mo: MeshObject = cast o;
						mo.name = ".Bullet";
						mo.materials[0] = md;
						mo.visible = true;

						var camera = iron.Scene.active.camera;
						var ct = camera.transform;
						mo.transform.loc.set(ct.worldx(), ct.worldy(), ct.worldz());
						mo.transform.scale.set(Context.raw.brushRadius * 0.2, Context.raw.brushRadius * 0.2, Context.raw.brushRadius * 0.2);
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

						Context.raw.particleTimer = iron.system.Tween.timer(5, mo.remove);
					});
				});
			}

			var pairs = world.getContactPairs(Context.raw.paintBody);
			if (pairs != null) {
				for (p in pairs) {
					Context.raw.lastParticleHitX = Context.raw.particleHitX != 0 ? Context.raw.particleHitX : p.posA.x;
					Context.raw.lastParticleHitY = Context.raw.particleHitY != 0 ? Context.raw.particleHitY : p.posA.y;
					Context.raw.lastParticleHitZ = Context.raw.particleHitZ != 0 ? Context.raw.particleHitZ : p.posA.z;
					Context.raw.particleHitX = p.posA.x;
					Context.raw.particleHitY = p.posA.y;
					Context.raw.particleHitZ = p.posA.z;
					Context.raw.pdirty = 1;
					break; // 1 pair for now
				}
			}
		}
		#end
	}

	function view_top() {
		#if (is_paint || is_sculpt)
		var isTyping = ui.isTyping || UIView2D.inst.ui.isTyping || UINodes.inst.ui.isTyping;
		#end
		#if is_lab
		var isTyping = ui.isTyping || UINodes.inst.ui.isTyping;
		#end

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
					if (ui.button(n, Left, Reflect.field(Config.keymap, n)) || (enter && count == operatorSearchOffset)) {
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

	#if (is_paint || is_sculpt)
	function getBrushStencilRect(): TRect {
		var w = Std.int(Context.raw.brushStencilImage.width * (App.h() / Context.raw.brushStencilImage.height) * Context.raw.brushStencilScale);
		var h = Std.int(App.h() * Context.raw.brushStencilScale);
		var x = Std.int(App.x() + Context.raw.brushStencilX * App.w());
		var y = Std.int(App.y() + Context.raw.brushStencilY * App.h());
		return { w: w, h: h, x: x, y: y };
	}
	#end

	function updateUI() {

		if (Console.messageTimer > 0) {
			Console.messageTimer -= Time.delta;
			if (Console.messageTimer <= 0) hwnds[TabStatus].redraws = 2;
		}

		if (!App.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		#if (is_paint || is_sculpt)
		// Same mapping for paint and rotate (predefined in touch keymap)
		if (mouse.started() && Config.keymap.action_paint == Config.keymap.action_rotate) {
			action_paint_remap = Config.keymap.action_paint;
			RenderUtil.pickPosNorTex();
			#if kha_metal
			RenderUtil.pickPosNorTex(); // Flush
			#end
			var isMesh = Math.abs(Context.raw.posXPicked) < 50 && Math.abs(Context.raw.posYPicked) < 50 && Math.abs(Context.raw.posZPicked) < 50;
			#if kha_android
			// Allow rotating with both pen and touch, because hovering a pen prevents touch input on android
			var penOnly = false;
			#else
			var penOnly = Context.raw.penPaintingOnly;
			#end
			var isPen = penOnly && Input.getPen().down();
			// Mesh picked - disable rotate
			// Pen painting only - rotate with touch, paint with pen
			if ((isMesh && !penOnly) || isPen) {
				Config.keymap.action_rotate = "";
				Config.keymap.action_paint = action_paint_remap;
			}
			// World sphere picked - disable paint
			else {
				Config.keymap.action_paint = "";
				Config.keymap.action_rotate = action_paint_remap;
			}
		}
		else if (!mouse.down() && action_paint_remap != "") {
			Config.keymap.action_rotate = action_paint_remap;
			Config.keymap.action_paint = action_paint_remap;
			action_paint_remap = "";
		}

		if (Context.raw.brushStencilImage != null && Operator.shortcut(Config.keymap.stencil_transform, ShortcutDown)) {
			var r = getBrushStencilRect();
			if (mouse.started("left")) {
				Context.raw.brushStencilScaling =
					hitRect(mouse.x, mouse.y, r.x - 8,       r.y - 8,       16, 16) ||
					hitRect(mouse.x, mouse.y, r.x - 8,       r.h + r.y - 8, 16, 16) ||
					hitRect(mouse.x, mouse.y, r.w + r.x - 8, r.y - 8,       16, 16) ||
					hitRect(mouse.x, mouse.y, r.w + r.x - 8, r.h + r.y - 8, 16, 16);
				var cosa = Math.cos(-Context.raw.brushStencilAngle);
				var sina = Math.sin(-Context.raw.brushStencilAngle);
				var ox = 0;
				var oy = -r.h / 2;
				var x = ox * cosa - oy * sina;
				var y = ox * sina + oy * cosa;
				x += r.x + r.w / 2;
				y += r.y + r.h / 2;
				Context.raw.brushStencilRotating =
					hitRect(mouse.x, mouse.y, Std.int(x - 16), Std.int(y - 16), 32, 32);
			}
			var _scale = Context.raw.brushStencilScale;
			if (mouse.down("left")) {
				if (Context.raw.brushStencilScaling) {
					var mult = mouse.x > r.x + r.w / 2 ? 1 : -1;
					Context.raw.brushStencilScale += mouse.movementX / 400 * mult;
				}
				else if (Context.raw.brushStencilRotating) {
					var gizmoX = r.x + r.w / 2;
					var gizmoY = r.y + r.h / 2;
					Context.raw.brushStencilAngle = -Math.atan2(mouse.y - gizmoY, mouse.x - gizmoX) - Math.PI / 2;
				}
				else {
					Context.raw.brushStencilX += mouse.movementX / App.w();
					Context.raw.brushStencilY += mouse.movementY / App.h();
				}
			}
			else Context.raw.brushStencilScaling = false;
			if (mouse.wheelDelta != 0) {
				Context.raw.brushStencilScale -= mouse.wheelDelta / 10;
			}
			// Center after scale
			var ratio = App.h() / Context.raw.brushStencilImage.height;
			var oldW = _scale * Context.raw.brushStencilImage.width * ratio;
			var newW = Context.raw.brushStencilScale * Context.raw.brushStencilImage.width * ratio;
			var oldH = _scale * App.h();
			var newH = Context.raw.brushStencilScale * App.h();
			Context.raw.brushStencilX += (oldW - newW) / App.w() / 2;
			Context.raw.brushStencilY += (oldH - newH) / App.h() / 2;
		}
		#end

		var setCloneSource = Context.raw.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		#if (is_paint || is_sculpt)
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
		var down = Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
				   decalMask ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
				   (Input.getPen().down() && !kb.down("alt"));
		#end
		#if is_lab
		var down = Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
				   (Input.getPen().down() && !kb.down("alt"));
		#end

		if (Config.raw.touch_ui) {
			if (Input.getPen().down()) {
				Context.raw.penPaintingOnly = true;
			}
			else if (Context.raw.penPaintingOnly) {
				down = false;
			}
		}

		#if arm_physics
		if (Context.raw.tool == ToolParticle && Context.raw.particlePhysics) {
			down = false;
		}
		#end

		#if (is_paint || is_sculpt)
		#if krom_ios
		// No hover on iPad, decals are painted by pen release
		if (decal) {
			down = Input.getPen().released();
			if (!Context.raw.penPaintingOnly) {
				down = down || Input.getMouse().released();
			}
		}
		#end
		#end

		if (down) {
			var mx = mouse.viewX;
			var my = mouse.viewY;
			var ww = iron.App.w();

			#if (is_paint || is_sculpt)
			if (Context.raw.paint2d) {
				mx -= iron.App.w();
				ww = UIView2D.inst.ww;
			}
			#end

			if (mx < ww &&
				mx > iron.App.x() &&
				my < iron.App.h() &&
				my > iron.App.y()) {

				if (setCloneSource) {
					Context.raw.cloneStartX = mx;
					Context.raw.cloneStartY = my;
				}
				else {
					if (Context.raw.brushTime == 0 &&
						!App.isDragging &&
						!App.isResizing &&
						!App.isComboSelected()) { // Paint started

						// Draw line
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
							Context.raw.lastPaintVecX = Context.raw.lastPaintX;
							Context.raw.lastPaintVecY = Context.raw.lastPaintY;
						}

						#if (is_paint || is_sculpt)
						History.pushUndo = true;

						if (Context.raw.tool == ToolClone && Context.raw.cloneStartX >= 0.0) { // Clone delta
							Context.raw.cloneDeltaX = (Context.raw.cloneStartX - mx) / ww;
							Context.raw.cloneDeltaY = (Context.raw.cloneStartY - my) / iron.App.h();
							Context.raw.cloneStartX = -1;
						}
						else if (Context.raw.tool == ToolParticle) {
							// Reset particles
							#if arm_particles
							var emitter: MeshObject = cast Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							@:privateAccess psys.time = 0;
							// @:privateAccess psys.time = @:privateAccess psys.seed * @:privateAccess psys.animtime;
							// @:privateAccess psys.seed++;
							#end
						}
						else if (Context.raw.tool == ToolFill && Context.raw.fillTypeHandle.position == FillUVIsland) {
							UVUtil.uvislandmapCached = false;
						}
						#end
					}

					Context.raw.brushTime += Time.delta;

					#if (is_paint || is_sculpt)
					if (Context.raw.runBrush != null) {
						Context.raw.runBrush(0);
					}
					#end
					#if is_lab
					if (Context.runBrush != null) {
						Context.runBrush(0);
					}
					#end
				}
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

			#if (is_paint || is_sculpt)
			Context.raw.layerPreviewDirty = true; // Update layer preview
			#end

			#if is_paint
			// New color id picked, update fill layer
			if (Context.raw.tool == ToolColorId && Context.raw.layer.fill_layer != null) {
				App.notifyOnNextFrame(function() {
					App.updateFillLayer();
					MakeMaterial.parsePaintMaterial(false);
				});
			}
			#end
		}

		#if is_paint
		if (Context.raw.layersPreviewDirty) {
			Context.raw.layersPreviewDirty = false;
			Context.raw.layerPreviewDirty = false;
			Context.raw.maskPreviewLast = null;
			if (App.pipeMerge == null) App.makePipe();
			// Update all layer previews
			for (l in Project.layers) {
				if (l.isGroup()) continue;
				var target = l.texpaint_preview;
				var source = l.texpaint;
				var g2 = target.g2;
				g2.begin(true, 0x00000000);
				// g2.pipeline = l.isMask() ? App.pipeCopy8 : App.pipeCopy;
				g2.pipeline = App.pipeCopy; // texpaint_preview is always RGBA32 for now
				g2.drawScaledImage(source, 0, 0, target.width, target.height);
				g2.pipeline = null;
				g2.end();
			}
			hwnds[TabSidebar0].redraws = 2;
		}
		if (Context.raw.layerPreviewDirty && !Context.raw.layer.isGroup()) {
			Context.raw.layerPreviewDirty = false;
			Context.raw.maskPreviewLast = null;
			if (App.pipeMerge == null) App.makePipe();
			// Update layer preview
			var l = Context.raw.layer;
			var target = l.texpaint_preview;
			var source = l.texpaint;
			var g2 = target.g2;
			g2.begin(true, 0x00000000);
			// g2.pipeline = Context.raw.layer.isMask() ? App.pipeCopy8 : App.pipeCopy;
			g2.pipeline = App.pipeCopy; // texpaint_preview is always RGBA32 for now
			g2.drawScaledImage(source, 0, 0, target.width, target.height);
			g2.pipeline = null;
			g2.end();
			hwnds[TabSidebar0].redraws = 2;
		}
		#end

		var undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		var redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (kb.down("control") && kb.started("y"));

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();

		#if (is_paint || is_sculpt)
		arm.render.Gizmo.update();
		#end
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

		// Remember last tab positions
		for (i in 0...htabs.length) {
			if (htabs[i].changed) {
				Config.raw.layout_tabs[i] = htabs[i].position;
				Config.save();
			}
		}

		// Set tab positions
		for (i in 0...htabs.length) {
			htabs[i].position = Config.raw.layout_tabs[i];
		}

		g.end();
		ui.begin(g);

		#if (is_paint || is_sculpt)
		UIToolbar.inst.renderUI(g);
		#end
		UIMenubar.inst.renderUI(g);
		UIHeader.inst.renderUI(g);
		UIStatus.inst.renderUI(g);

		#if (is_paint || is_sculpt)
		tabx = System.windowWidth() - Config.raw.layout[LayoutSidebarW];
		if (ui.window(hwnds[TabSidebar0], tabx, 0, Config.raw.layout[LayoutSidebarW], Config.raw.layout[LayoutSidebarH0])) {
			for (draw in hwndTabs[TabSidebar0]) draw(htabs[TabSidebar0]);
		}
		if (ui.window(hwnds[TabSidebar1], tabx, Config.raw.layout[LayoutSidebarH0], Config.raw.layout[LayoutSidebarW], Config.raw.layout[LayoutSidebarH1])) {
			for (draw in hwndTabs[TabSidebar1]) draw(htabs[TabSidebar1]);
		}

		if (Config.raw.layout[LayoutSidebarW] == 0) {
			var width = Std.int(ui.ops.font.width(ui.fontSize, "<<") + 25 * ui.SCALE());
			if (ui.window(hminimized, System.windowWidth() - width, 0, width, Std.int(ui.ELEMENT_H() + ui.ELEMENT_OFFSET()))) {
				ui._w = width;
				var _BUTTON_H = ui.t.BUTTON_H;
				var _BUTTON_COL = ui.t.BUTTON_COL;
				ui.t.BUTTON_H = ui.t.ELEMENT_H;
				ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

				if (ui.button("<<")) {
					Config.raw.layout[LayoutSidebarW] = Context.raw.maximizedSidebarWidth != 0 ? Context.raw.maximizedSidebarWidth : Std.int(UIBase.defaultWindowW * Config.raw.window_scale);
				}
				ui.t.BUTTON_H = _BUTTON_H;
				ui.t.BUTTON_COL = _BUTTON_COL;
			}
		}
		else if (htabs[TabSidebar0].changed && htabs[TabSidebar0].position == Context.raw.lastHtab0Position) {
			if (Time.time() - Context.raw.selectTime < 0.25) {
				Context.raw.maximizedSidebarWidth = Config.raw.layout[LayoutSidebarW];
				Config.raw.layout[LayoutSidebarW] = 0;
			}
			Context.raw.selectTime = Time.time();
		}
		Context.raw.lastHtab0Position = htabs[TabSidebar0].position;
		#end



		ui.end();
		g.begin(false);
	}

	#if (is_paint || is_sculpt)
	public function renderCursor(g: kha.graphics2.Graphics) {
		if (!App.uiEnabled) return;

		#if is_paint
		if (Context.raw.tool == ToolMaterial || Context.raw.tool == ToolBake) return;
		#end

		g.color = 0xffffffff;

		Context.raw.viewIndex = Context.raw.viewIndexLast;
		var mx = App.x() + Context.raw.paintVec.x * App.w();
		var my = App.y() + Context.raw.paintVec.y * App.h();
		Context.raw.viewIndex = -1;

		// Radius being scaled
		if (Context.raw.brushLocked) {
			mx += Context.raw.lockStartedX - System.windowWidth() / 2;
			my += Context.raw.lockStartedY - System.windowHeight() / 2;
		}

		#if is_paint
		if (Context.raw.brushStencilImage != null && Context.raw.tool != ToolBake && Context.raw.tool != ToolPicker && Context.raw.tool != ToolMaterial && Context.raw.tool != ToolColorId) {
			var r = getBrushStencilRect();
			if (!Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown)) {
				g.color = 0x88ffffff;
				var angle = Context.raw.brushStencilAngle;
				var cx = r.x + r.w / 2;
				var cy = r.y + r.h / 2;
				g.transformation = FastMatrix3.translation(cx, cy).multmat(FastMatrix3.rotation(-angle)).multmat(FastMatrix3.translation(-cx, -cy));
				g.drawScaledImage(Context.raw.brushStencilImage, r.x, r.y, r.w, r.h);
				g.transformation = null;
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
				var angle = Context.raw.brushStencilAngle;
				var cx = r.x + r.w / 2;
				var cy = r.y + r.h / 2;
				g.transformation = FastMatrix3.translation(cx, cy).multmat(FastMatrix3.rotation(-angle)).multmat(FastMatrix3.translation(-cx, -cy));
				kha.graphics2.GraphicsExtension.fillCircle(g, r.x + r.w / 2, r.y - 4, 8);
				g.transformation = null;
			}
		}
		#end

		// Show picked material next to cursor
		if (Context.raw.tool == ToolPicker && Context.raw.pickerSelectMaterial && Context.raw.colorPickerCallback == null) {
			var img = Context.raw.material.imageIcon;
			#if kha_opengl
			g.drawScaledImage(img, mx + 10, my + 10 + img.height, img.width, -img.height);
			#else
			g.drawImage(img, mx + 10, my + 10);
			#end
		}
		if (Context.raw.tool == ToolPicker && Context.raw.colorPickerCallback != null) {
			var img = Res.get("icons.k");
			var rect = Res.tile50(img, ToolPicker, 0);
			g.drawSubImage(img, mx + 10, my + 10, rect.x, rect.y, rect.w, rect.h);
		}

		var cursorImg = Res.get("cursor.k");
		var psize = Std.int(cursorImg.width * (Context.raw.brushRadius * Context.raw.brushNodesRadius) * ui.SCALE());

		// Clone source cursor
		var mouse = Input.getMouse();
		var pen = Input.getPen();
		var kb = Input.getKeyboard();
		if (Context.raw.tool == ToolClone && !kb.down("alt") && (mouse.down() || pen.down())) {
			g.color = 0x66ffffff;
			g.drawScaledImage(cursorImg, mx + Context.raw.cloneDeltaX * iron.App.w() - psize / 2, my + Context.raw.cloneDeltaY * iron.App.h() - psize / 2, psize, psize);
			g.color = 0xffffffff;
		}

		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;

		if (!Config.raw.brush_3d || Context.in2dView() || decal) {
			var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);
			if (decal && !Context.inNodes()) {
				var decalAlpha = 0.5;
				if (!decalMask) {
					Context.raw.decalX = Context.raw.paintVec.x;
					Context.raw.decalY = Context.raw.paintVec.y;
					decalAlpha = Context.raw.brushOpacity;

					// Radius being scaled
					if (Context.raw.brushLocked) {
						Context.raw.decalX += (Context.raw.lockStartedX - System.windowWidth() / 2) / App.w();
						Context.raw.decalY += (Context.raw.lockStartedY - System.windowHeight() / 2) / App.h();
					}
				}

				if (!Config.raw.brush_live) {
					var psizex = Std.int(256 * ui.SCALE() * (Context.raw.brushRadius * Context.raw.brushNodesRadius * Context.raw.brushScaleX));
					var psizey = Std.int(256 * ui.SCALE() * (Context.raw.brushRadius * Context.raw.brushNodesRadius));

					Context.raw.viewIndex = Context.raw.viewIndexLast;
					var decalX = App.x() + Context.raw.decalX * App.w() - psizex / 2;
					var decalY = App.y() + Context.raw.decalY * App.h() - psizey / 2;
					Context.raw.viewIndex = -1;

					g.color = kha.Color.fromFloats(1, 1, 1, decalAlpha);
					var angle = (Context.raw.brushAngle + Context.raw.brushNodesAngle) * (Math.PI / 180);
					var cx = decalX + psizex / 2;
					var cy = decalY + psizey / 2;
					g.transformation = FastMatrix3.translation(cx, cy).multmat(FastMatrix3.rotation(angle)).multmat(FastMatrix3.translation(-cx, -cy));
					#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
					g.drawScaledImage(Context.raw.decalImage, decalX, decalY, psizex, psizey);
					#else
					g.drawScaledImage(Context.raw.decalImage, decalX, decalY + psizey, psizex, -psizey);
					#end
					g.transformation = null;
					g.color = 0xffffffff;
				}
			}
			if (Context.raw.tool == ToolBrush  ||
				Context.raw.tool == ToolEraser ||
				Context.raw.tool == ToolClone  ||
				Context.raw.tool == ToolBlur   ||
				Context.raw.tool == ToolSmudge   ||
				Context.raw.tool == ToolParticle ||
				(decalMask && !Config.raw.brush_3d) ||
				(decalMask && Context.in2dView())) {
				if (decalMask) {
					psize = Std.int(cursorImg.width * (Context.raw.brushDecalMaskRadius * Context.raw.brushNodesRadius) * ui.SCALE());
				}
				if (Config.raw.brush_3d && Context.in2dView()) {
					psize = Std.int(psize * UIView2D.inst.panScale);
				}
				g.drawScaledImage(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
			}
		}

		if (Context.raw.brushLazyRadius > 0 && !Context.raw.brushLocked &&
			(Context.raw.tool == ToolBrush ||
			 Context.raw.tool == ToolEraser ||
			 Context.raw.tool == ToolDecal ||
			 Context.raw.tool == ToolText ||
			 Context.raw.tool == ToolClone ||
			 Context.raw.tool == ToolBlur ||
			 Context.raw.tool == ToolSmudge ||
			 Context.raw.tool == ToolParticle)) {
			g.fillRect(mx - 1, my - 1, 2, 2);
			var mx = Context.raw.brushLazyX * App.w() + App.x();
			var my = Context.raw.brushLazyY * App.h() + App.y();
			var radius = Context.raw.brushLazyRadius * 180;
			g.color = 0xff666666;
			g.drawScaledImage(cursorImg, mx - radius / 2, my - radius / 2, radius, radius);
			g.color = 0xffffffff;
		}
	}
	#end

	public function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		@:privateAccess UINodes.inst.ui.endInput();

		#if (is_paint || is_sculpt)
		UINodes.inst.show = !UINodes.inst.show || UINodes.inst.canvasType != CanvasMaterial;
		UINodes.inst.canvasType = CanvasMaterial;
		#end
		#if is_lab
		UINodes.inst.show = !UINodes.inst.show;
		#end

		App.resize();
	}

	#if (is_paint || is_sculpt)
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
	#end

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

		#if (is_paint || is_sculpt)
		if (handle != hwnds[TabSidebar0] &&
			handle != hwnds[TabSidebar1] &&
			handle != hwnds[TabStatus] &&
			handle != UINodes.inst.hwnd &&
			handle != UIView2D.inst.hwnd) return; // Scalable handles
		if (handle == UIView2D.inst.hwnd && side != SideLeft) return;
		if (handle == UINodes.inst.hwnd && side == SideTop && !UIView2D.inst.show) return;
		if (handle == hwnds[TabSidebar0] && side == SideTop) return;
		#end

		#if is_lab
		if (handle != hwnds[TabStatus] &&
			handle != UINodes.inst.hwnd) return; // Scalable handles
		if (handle == UINodes.inst.hwnd && side == SideTop) return;
		#end

		if (handle == UINodes.inst.hwnd && side != SideLeft && side != SideTop) return;
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
		#if (is_paint || is_sculpt)
		hwnds[TabSidebar0].redraws = 2;
		hwnds[TabSidebar1].redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		#end
	}
}
