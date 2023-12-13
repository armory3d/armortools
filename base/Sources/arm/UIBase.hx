package arm;

import zui.Zui;
import zui.Zui.Nodes;
import iron.App;
import iron.Input;
import iron.System;
import iron.Data;
import iron.MaterialData;
import iron.MeshObject;
import iron.ArmPack;
import iron.Input;
import iron.Time;
import iron.Scene;
import iron.Tween;
import iron.RayCaster;
import iron.Mat3;
import iron.Object;
import arm.ProjectFormat;
import arm.Res;

class UIBase {

	public static var inst: UIBase;
	public var show = true;
	public var ui: Zui;
	var borderStarted = 0;
	var borderHandle_ptr: Int = 0;
	var action_paint_remap = "";
	var operatorSearchOffset = 0;
	var undoTapTime = 0.0;
	var redoTapTime = 0.0;

	#if is_paint
	public var hwnds = [new Handle(), new Handle(), new Handle()];
	public var htabs = [new Handle(), new Handle(), new Handle()];
	public var hwndTabs = [
		[TabLayers.draw, TabHistory.draw, TabPlugins.draw #if is_forge , TabObjects.draw #end],
		[TabMaterials.draw, TabBrushes.draw, TabParticles.draw],
		[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabSwatches.draw, TabScript.draw, TabConsole.draw, UIStatus.drawVersionTab]
	];
	#end
	#if is_sculpt
	public var hwnds = [new Handle(), new Handle(), new Handle()];
	public var htabs = [new Handle(), new Handle(), new Handle()];
	public var hwndTabs = [
		[TabLayers.draw, TabHistory.draw, TabPlugins.draw #if is_forge , TabObjects.draw #end],
		[TabMaterials.draw, TabBrushes.draw, TabParticles.draw],
		[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabScript.draw, TabConsole.draw, UIStatus.drawVersionTab]
	];
	#end
	#if is_lab
	public var hwnds = [new Handle()];
	public var htabs = [new Handle()];
	public var hwndTabs = [
		[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabSwatches.draw, TabPlugins.draw, TabScript.draw, TabConsole.draw, UIStatus.drawVersionTab]
	];
	#end

	#if (is_paint || is_sculpt)
	public static inline var defaultSidebarMiniW = 56;
	public static inline var defaultSidebarFullW = 280;
	#if (krom_android || krom_ios)
	public static inline var defaultSidebarW = defaultSidebarMiniW;
	#else
	public static inline var defaultSidebarW = defaultSidebarFullW;
	#end
	public var tabx = 0;
	public var hminimized = new Handle();
	public static var sidebarMiniW = defaultSidebarMiniW;
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
				Project.materials.push(new SlotMaterial(m));
				Context.raw.material = Project.materials[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(new SlotBrush());
			Context.raw.brush = Project.brushes[0];
			MakeMaterial.parseBrush();
		}

		if (Project.fonts == null) {
			Project.fonts = [];
			Project.fonts.push(new SlotFont("default.ttf", Base.font));
			Context.raw.font = Project.fonts[0];
		}

		if (Project.layers == null) {
			Project.layers = [];
			Project.layers.push(new SlotLayer());
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
			Data.getBlob("default_brush.arm", function(b: js.lib.ArrayBuffer) {
				Project.defaultCanvas = b;
			});
		}

		Project.nodes = new Nodes();
		Project.canvas = ArmPack.decode(Project.defaultCanvas);
		Project.canvas.name = "Brush 1";

		Context.parseBrushInputs();

		ParserLogic.parse(Project.canvas);
		#end

		if (Project.raw.swatches == null) {
			Project.setDefaultSwatches();
			Context.raw.swatch = Project.raw.swatches[0];
		}

		if (Context.raw.emptyEnvmap == null) {
			var b = new js.lib.Uint8Array(4);
			b[0] = 8;
			b[1] = 8;
			b[2] = 8;
			b[3] = 255;
			Context.raw.emptyEnvmap = Image.fromBytes(b.buffer, 1, 1);
		}
		if (Context.raw.previewEnvmap == null) {
			var b = new js.lib.Uint8Array(4);
			b[0] = 0;
			b[1] = 0;
			b[2] = 0;
			b[3] = 255;
			Context.raw.previewEnvmap = Image.fromBytes(b.buffer, 1, 1);
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
		ui = new Zui({ theme: Base.theme, font: Base.font, scaleFactor: scale, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient });
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
			App.notifyOnInit(Base.initLayers);
		}

		Context.raw.projectObjects = [];
		for (m in Scene.active.meshes) Context.raw.projectObjects.push(m);

		Operator.register("view_top", view_top);
	}

	public function update() {
		updateUI();
		Operator.update();

		for (p in Plugin.plugins) if (p.update != null) p.update();

		if (!Base.uiEnabled) return;

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

			else if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
				#if (is_paint || is_sculpt)
				show2DView(View2DLayer);
				#else
				show2DView(View2DAsset);
				#end
			}
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
				App.notifyOnInit(_init);
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
			Base.toggleFullscreen();
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

		var isTyping = ui.isTyping || UIView2D.inst.ui.isTyping || UINodes.inst.ui.isTyping;

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
			if (mouse.viewX < App.w()) {
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
						var modeHandle = Zui.handle("uibase_0");
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

						#if (krom_direct3d12 || krom_vulkan || krom_metal)
						if (Krom.raytraceSupported()) {
							modes.push(tr("Path Traced"));
							shortcuts.push("p");
						}
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
					}, 16 #if (krom_direct3d12 || krom_vulkan || krom_metal) + 1 #end );
					#end
					#if is_lab
					}, 9 #if (krom_direct3d12 || krom_vulkan || krom_metal) + 1 #end );
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
		if (borderHandle_ptr != 0) {
			if (borderHandle_ptr == UINodes.inst.hwnd.ptr || borderHandle_ptr == UIView2D.inst.hwnd.ptr) {
				if (borderStarted == SideLeft) {
					Config.raw.layout[LayoutNodesW] -= Std.int(mouse.movementX);
					if (Config.raw.layout[LayoutNodesW] < 32) Config.raw.layout[LayoutNodesW] = 32;
					else if (Config.raw.layout[LayoutNodesW] > System.width * 0.7) Config.raw.layout[LayoutNodesW] = Std.int(System.width * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[LayoutNodesH] -= Std.int(mouse.movementY);
					if (Config.raw.layout[LayoutNodesH] < 32) Config.raw.layout[LayoutNodesH] = 32;
					else if (Config.raw.layout[LayoutNodesH] > App.h() * 0.95) Config.raw.layout[LayoutNodesH] = Std.int(App.h() * 0.95);
				}
			}
			else if (borderHandle_ptr == hwnds[TabStatus].ptr) {
				var my = Std.int(mouse.movementY);
				if (Config.raw.layout[LayoutStatusH] - my >= UIStatus.defaultStatusH * Config.raw.window_scale && Config.raw.layout[LayoutStatusH] - my < System.height * 0.7) {
					Config.raw.layout[LayoutStatusH] -= my;
				}
			}
			else {
				if (borderStarted == SideLeft) {
					Config.raw.layout[LayoutSidebarW] -= Std.int(mouse.movementX);
					if (Config.raw.layout[LayoutSidebarW] < sidebarMiniW) Config.raw.layout[LayoutSidebarW] = sidebarMiniW;
					else if (Config.raw.layout[LayoutSidebarW] > System.width - sidebarMiniW) Config.raw.layout[LayoutSidebarW] = System.width - sidebarMiniW;
				}
				else {
					var my = Std.int(mouse.movementY);
					if (borderHandle_ptr == hwnds[TabSidebar1].ptr && borderStarted == SideTop) {
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
		if (borderHandle_ptr != 0) {
			if (borderHandle_ptr == UINodes.inst.hwnd.ptr || borderHandle_ptr == UIView2D.inst.hwnd.ptr) {
				if (borderStarted == SideLeft) {
					Config.raw.layout[LayoutNodesW] -= Std.int(mouse.movementX);
					if (Config.raw.layout[LayoutNodesW] < 32) Config.raw.layout[LayoutNodesW] = 32;
					else if (Config.raw.layout[LayoutNodesW] > System.width * 0.7) Config.raw.layout[LayoutNodesW] = Std.int(System.width * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[LayoutNodesH] -= Std.int(mouse.movementY);
					if (Config.raw.layout[LayoutNodesH] < 32) Config.raw.layout[LayoutNodesH] = 32;
					else if (Config.raw.layout[LayoutNodesH] > App.h() * 0.95) Config.raw.layout[LayoutNodesH] = Std.int(App.h() * 0.95);
				}
			}
			else if (borderHandle_ptr == hwnds[TabStatus].ptr) {
				var my = Std.int(mouse.movementY);
				if (Config.raw.layout[LayoutStatusH] - my >= UIStatus.defaultStatusH * Config.raw.window_scale && Config.raw.layout[LayoutStatusH] - my < System.height * 0.7) {
					Config.raw.layout[LayoutStatusH] -= my;
				}
			}
		}
		#end

		if (!mouse.down()) {
			borderHandle_ptr = 0;
			Base.isResizing = false;
		}

		#if arm_physics
		if (Context.raw.tool == ToolParticle && Context.raw.particlePhysics && Context.inPaintArea() && !Context.raw.paint2d) {
			UtilParticle.initParticlePhysics();
			var world = PhysicsWorld.active;
			world.lateUpdate();
			Context.raw.ddirty = 2;
			Context.raw.rdirty = 2;
			if (mouse.started()) {
				if (Context.raw.particleTimer != null) {
					Tween.stop(Context.raw.particleTimer);
					Context.raw.particleTimer.done();
					Context.raw.particleTimer = null;
				}
				History.pushUndo = true;
				Context.raw.particleHitX = Context.raw.particleHitY = Context.raw.particleHitZ = 0;
				Scene.active.spawnObject(".Sphere", null, function(o: Object) {
					Data.getMaterial("Scene", ".Gizmo", function(md: MaterialData) {
						var mo: MeshObject = cast o;
						mo.name = ".Bullet";
						mo.materials[0] = md;
						mo.visible = true;

						var camera = Scene.active.camera;
						var ct = camera.transform;
						mo.transform.loc.set(ct.worldx(), ct.worldy(), ct.worldz());
						mo.transform.scale.set(Context.raw.brushRadius * 0.2, Context.raw.brushRadius * 0.2, Context.raw.brushRadius * 0.2);
						mo.transform.buildMatrix();

						var body = new PhysicsBody();
						body.shape = PhysicsBody.ShapeType.ShapeSphere;
						body.mass = 1.0;
						body.ccd = true;
						mo.transform.radius /= 10; // Lower ccd radius
						body.init(mo);
						mo.addTrait(body);
						mo.transform.radius *= 10;

						var ray = RayCaster.getRay(mouse.viewX, mouse.viewY, camera);
						body.applyImpulse(ray.direction.mult(0.15));

						Context.raw.particleTimer = Tween.timer(5, mo.remove);
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
		var isTyping = ui.isTyping || UIView2D.inst.ui.isTyping || UINodes.inst.ui.isTyping;

		var mouse = Input.getMouse();
		if (Context.inPaintArea() && !isTyping) {
			if (mouse.viewX < App.w()) {
				Viewport.setView(0, 0, 1, 0, 0, 0);
			}
		}
	}

	function operatorSearch() {
		var kb = Input.getKeyboard();
		var searchHandle = Zui.handle("uibase_1");
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
				if (ui.key == KeyCode.Down && operatorSearchOffset < 6) operatorSearchOffset++;
				if (ui.key == KeyCode.Up && operatorSearchOffset > 0) operatorSearchOffset--;
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
		Base.resize();
	}

	inline function getRadiusIncrement(): Float {
		return 0.1;
	}

	static function hitRect(mx: Float, my: Float, x: Int, y: Int, w: Int, h: Int) {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	#if (is_paint || is_sculpt)
	function getBrushStencilRect(): TRect {
		var w = Std.int(Context.raw.brushStencilImage.width * (Base.h() / Context.raw.brushStencilImage.height) * Context.raw.brushStencilScale);
		var h = Std.int(Base.h() * Context.raw.brushStencilScale);
		var x = Std.int(Base.x() + Context.raw.brushStencilX * Base.w());
		var y = Std.int(Base.y() + Context.raw.brushStencilY * Base.h());
		return { w: w, h: h, x: x, y: y };
	}
	#end

	function updateUI() {
		if (Console.messageTimer > 0) {
			Console.messageTimer -= Time.delta;
			if (Console.messageTimer <= 0) hwnds[TabStatus].redraws = 2;
		}

		#if (is_paint || is_sculpt)
		sidebarMiniW = Std.int(defaultSidebarMiniW * ui.SCALE());
		#end

		if (!Base.uiEnabled) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		#if (is_paint || is_sculpt)
		// Same mapping for paint and rotate (predefined in touch keymap)
		if (Context.inViewport()) {
			if (mouse.started() && Config.keymap.action_paint == Config.keymap.action_rotate) {
				action_paint_remap = Config.keymap.action_paint;
				UtilRender.pickPosNorTex();
				var isMesh = Math.abs(Context.raw.posXPicked) < 50 && Math.abs(Context.raw.posYPicked) < 50 && Math.abs(Context.raw.posZPicked) < 50;
				#if krom_android
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
					Context.raw.brushStencilX += mouse.movementX / Base.w();
					Context.raw.brushStencilY += mouse.movementY / Base.h();
				}
			}
			else Context.raw.brushStencilScaling = false;
			if (mouse.wheelDelta != 0) {
				Context.raw.brushStencilScale -= mouse.wheelDelta / 10;
			}
			// Center after scale
			var ratio = Base.h() / Context.raw.brushStencilImage.height;
			var oldW = _scale * Context.raw.brushStencilImage.width * ratio;
			var newW = Context.raw.brushStencilScale * Context.raw.brushStencilImage.width * ratio;
			var oldH = _scale * Base.h();
			var newH = Context.raw.brushStencilScale * Base.h();
			Context.raw.brushStencilX += (oldW - newW) / Base.w() / 2;
			Context.raw.brushStencilY += (oldH - newH) / Base.h() / 2;
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
			var ww = App.w();

			#if (is_paint || is_sculpt)
			if (Context.raw.paint2d) {
				mx -= App.w();
				ww = UIView2D.inst.ww;
			}
			#end

			if (mx < ww &&
				mx > App.x() &&
				my < App.h() &&
				my > App.y()) {

				if (setCloneSource) {
					Context.raw.cloneStartX = mx;
					Context.raw.cloneStartY = my;
				}
				else {
					if (Context.raw.brushTime == 0 &&
						!Base.isDragging &&
						!Base.isResizing &&
						!Base.isComboSelected()) { // Paint started

						// Draw line
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
							Context.raw.lastPaintVecX = Context.raw.lastPaintX;
							Context.raw.lastPaintVecY = Context.raw.lastPaintY;
						}

						#if (is_paint || is_sculpt)
						History.pushUndo = true;

						if (Context.raw.tool == ToolClone && Context.raw.cloneStartX >= 0.0) { // Clone delta
							Context.raw.cloneDeltaX = (Context.raw.cloneStartX - mx) / ww;
							Context.raw.cloneDeltaY = (Context.raw.cloneStartY - my) / App.h();
							Context.raw.cloneStartX = -1;
						}
						else if (Context.raw.tool == ToolParticle) {
							// Reset particles
							#if arm_particles
							var emitter: MeshObject = cast Scene.active.getChild(".ParticleEmitter");
							var psys = emitter.particleSystems[0];
							psys.time = 0;
							// psys.time = psys.seed * psys.animtime;
							// psys.seed++;
							#end
						}
						else if (Context.raw.tool == ToolFill && Context.raw.fillTypeHandle.position == FillUVIsland) {
							UtilUV.uvislandmapCached = false;
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
			#if (!krom_direct3d12 && !krom_vulkan && !krom_metal) // Keep accumulated samples for D3D12
			Context.raw.ddirty = 3;
			#end
			Context.raw.brushBlendDirty = true; // Update brush mask

			#if (is_paint || is_sculpt)
			Context.raw.layerPreviewDirty = true; // Update layer preview
			#end

			#if is_paint
			// New color id picked, update fill layer
			if (Context.raw.tool == ToolColorId && Context.raw.layer.fill_layer != null) {
				Base.notifyOnNextFrame(function() {
					Base.updateFillLayer();
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
			if (Base.pipeMerge == null) Base.makePipe();
			// Update all layer previews
			for (l in Project.layers) {
				if (l.isGroup()) continue;
				var target = l.texpaint_preview;
				var source = l.texpaint;
				var g2 = target.g2;
				g2.begin(true, 0x00000000);
				// g2.pipeline = l.isMask() ? Base.pipeCopy8 : Base.pipeCopy;
				g2.pipeline = Base.pipeCopy; // texpaint_preview is always RGBA32 for now
				g2.drawScaledImage(source, 0, 0, target.width, target.height);
				g2.pipeline = null;
				g2.end();
			}
			hwnds[TabSidebar0].redraws = 2;
		}
		if (Context.raw.layerPreviewDirty && !Context.raw.layer.isGroup()) {
			Context.raw.layerPreviewDirty = false;
			Context.raw.maskPreviewLast = null;
			if (Base.pipeMerge == null) Base.makePipe();
			// Update layer preview
			var l = Context.raw.layer;
			var target = l.texpaint_preview;
			var source = l.texpaint;
			var g2 = target.g2;
			g2.begin(true, 0x00000000);
			// g2.pipeline = Context.raw.layer.isMask() ? Base.pipeCopy8 : Base.pipeCopy;
			g2.pipeline = Base.pipeCopy; // texpaint_preview is always RGBA32 for now
			g2.drawScaledImage(source, 0, 0, target.width, target.height);
			g2.pipeline = null;
			g2.end();
			hwnds[TabSidebar0].redraws = 2;
		}
		#end

		var undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		var redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (kb.down("control") && kb.started("y"));

		// Two-finger tap to undo, three-finger tap to redo
		if (Context.inViewport() && Config.raw.touch_ui) {
			if (mouse.started("middle")) { redoTapTime = Time.time(); }
			else if (mouse.started("right")) { undoTapTime = Time.time(); }
			else if (mouse.released("middle") && Time.time() - redoTapTime < 0.1) { redoTapTime = undoTapTime = 0; redoPressed = true; }
			else if (mouse.released("right") && Time.time() - undoTapTime < 0.1) { redoTapTime = undoTapTime = 0; undoPressed = true; }
		}

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();

		#if (is_paint || is_sculpt)
		Gizmo.update();
		#end
	}

	public function render(g: Graphics2) {
		if (!show && Config.raw.touch_ui) {
			ui.inputEnabled = true;
			g.end();
			ui.begin(g);
			if (ui.window(Zui.handle("uibase_2"), 0, 0, 150, Std.int(ui.ELEMENT_H() + ui.ELEMENT_OFFSET() + 1))) {
				if (ui.button(tr("Close"))) {
					toggleDistractFree();
				}
			}
			ui.end();
			g.begin(false);
		}

		if (!show || System.width == 0 || System.height == 0) return;

		ui.inputEnabled = Base.uiEnabled;

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
		drawSidebar();
		#end

		ui.end();
		g.begin(false);
	}

	#if (is_paint || is_sculpt)
	function drawSidebar() {
		// Tabs
		var mini = Config.raw.layout[LayoutSidebarW] <= sidebarMiniW;
		var expandButtonOffset = Config.raw.touch_ui ? Std.int(ui.ELEMENT_H() + ui.ELEMENT_OFFSET()) : 0;
		tabx = System.width - Config.raw.layout[LayoutSidebarW];

		var _SCROLL_W = ui.t.SCROLL_W;
		if (mini) ui.t.SCROLL_W = ui.t.SCROLL_MINI_W;

		if (ui.window(hwnds[TabSidebar0], tabx, 0, Config.raw.layout[LayoutSidebarW], Config.raw.layout[LayoutSidebarH0])) {
			for (i in 0...(mini ? 1 : hwndTabs[TabSidebar0].length)) hwndTabs[TabSidebar0][i](htabs[TabSidebar0]);
		}
		if (ui.window(hwnds[TabSidebar1], tabx, Config.raw.layout[LayoutSidebarH0], Config.raw.layout[LayoutSidebarW], Config.raw.layout[LayoutSidebarH1] - expandButtonOffset)) {
			for (i in 0...(mini ? 1 : hwndTabs[TabSidebar1].length)) hwndTabs[TabSidebar1][i](htabs[TabSidebar1]);
		}

		ui.endWindow();
		ui.t.SCROLL_W = _SCROLL_W;

		// Collapse / expand button for mini sidebar
		if (Config.raw.touch_ui) {
			var width = Config.raw.layout[LayoutSidebarW];
			var height = Std.int(ui.ELEMENT_H() + ui.ELEMENT_OFFSET());
			if (ui.window(Zui.handle("uibase_3"), System.width - width, System.height - height, width, height + 1)) {
				ui._w = width;
				var _BUTTON_H = ui.t.BUTTON_H;
				var _BUTTON_COL = ui.t.BUTTON_COL;
				ui.t.BUTTON_H = ui.t.ELEMENT_H;
				ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
				if (ui.button(mini ? "<<" : ">>")) {
					Config.raw.layout[LayoutSidebarW] = mini ? defaultSidebarFullW : defaultSidebarMiniW;
					Config.raw.layout[LayoutSidebarW] = Std.int(Config.raw.layout[LayoutSidebarW] * ui.SCALE());
				}
				ui.t.BUTTON_H = _BUTTON_H;
				ui.t.BUTTON_COL = _BUTTON_COL;
			}
		}

		// Expand button
		if (Config.raw.layout[LayoutSidebarW] == 0) {
			var width = Std.int(ui.font.width(ui.fontSize, "<<") + 25 * ui.SCALE());
			if (ui.window(hminimized, System.width - width, 0, width, Std.int(ui.ELEMENT_H() + ui.ELEMENT_OFFSET() + 1))) {
				ui._w = width;
				var _BUTTON_H = ui.t.BUTTON_H;
				var _BUTTON_COL = ui.t.BUTTON_COL;
				ui.t.BUTTON_H = ui.t.ELEMENT_H;
				ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

				if (ui.button("<<")) {
					Config.raw.layout[LayoutSidebarW] = Context.raw.maximizedSidebarWidth != 0 ? Context.raw.maximizedSidebarWidth : Std.int(UIBase.defaultSidebarW * Config.raw.window_scale);
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
	}

	public function renderCursor(g: Graphics2) {
		if (!Base.uiEnabled) return;

		#if is_paint
		if (Context.raw.tool == ToolMaterial || Context.raw.tool == ToolBake) return;
		#end

		g.color = 0xffffffff;

		Context.raw.viewIndex = Context.raw.viewIndexLast;
		var mx = Base.x() + Context.raw.paintVec.x * Base.w();
		var my = Base.y() + Context.raw.paintVec.y * Base.h();
		Context.raw.viewIndex = -1;

		// Radius being scaled
		if (Context.raw.brushLocked) {
			mx += Context.raw.lockStartedX - System.width / 2;
			my += Context.raw.lockStartedY - System.height / 2;
		}

		#if is_paint
		if (Context.raw.brushStencilImage != null && Context.raw.tool != ToolBake && Context.raw.tool != ToolPicker && Context.raw.tool != ToolMaterial && Context.raw.tool != ToolColorId) {
			var r = getBrushStencilRect();
			if (!Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown)) {
				g.color = 0x88ffffff;
				var angle = Context.raw.brushStencilAngle;
				var cx = r.x + r.w / 2;
				var cy = r.y + r.h / 2;
				g.transformation = Mat3.translation(cx, cy).multmat(Mat3.rotation(-angle)).multmat(Mat3.translation(-cx, -cy));
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
				g.transformation = Mat3.translation(cx, cy).multmat(Mat3.rotation(-angle)).multmat(Mat3.translation(-cx, -cy));
				g.fillCircle(r.x + r.w / 2, r.y - 4, 8);
				g.transformation = null;
			}
		}
		#end

		// Show picked material next to cursor
		if (Context.raw.tool == ToolPicker && Context.raw.pickerSelectMaterial && Context.raw.colorPickerCallback == null) {
			var img = Context.raw.material.imageIcon;
			#if krom_opengl
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
			g.drawScaledImage(cursorImg, mx + Context.raw.cloneDeltaX * App.w() - psize / 2, my + Context.raw.cloneDeltaY * App.h() - psize / 2, psize, psize);
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
						Context.raw.decalX += (Context.raw.lockStartedX - System.width / 2) / Base.w();
						Context.raw.decalY += (Context.raw.lockStartedY - System.height / 2) / Base.h();
					}
				}

				if (!Config.raw.brush_live) {
					var psizex = Std.int(256 * ui.SCALE() * (Context.raw.brushRadius * Context.raw.brushNodesRadius * Context.raw.brushScaleX));
					var psizey = Std.int(256 * ui.SCALE() * (Context.raw.brushRadius * Context.raw.brushNodesRadius));

					Context.raw.viewIndex = Context.raw.viewIndexLast;
					var decalX = Base.x() + Context.raw.decalX * Base.w() - psizex / 2;
					var decalY = Base.y() + Context.raw.decalY * Base.h() - psizey / 2;
					Context.raw.viewIndex = -1;

					g.color = Color.fromFloats(1, 1, 1, decalAlpha);
					var angle = (Context.raw.brushAngle + Context.raw.brushNodesAngle) * (Math.PI / 180);
					var cx = decalX + psizex / 2;
					var cy = decalY + psizey / 2;
					g.transformation = Mat3.translation(cx, cy).multmat(Mat3.rotation(angle)).multmat(Mat3.translation(-cx, -cy));
					#if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
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
			var mx = Context.raw.brushLazyX * Base.w() + Base.x();
			var my = Context.raw.brushLazyY * Base.h() + Base.y();
			var radius = Context.raw.brushLazyRadius * 180;
			g.color = 0xff666666;
			g.drawScaledImage(cursorImg, mx - radius / 2, my - radius / 2, radius, radius);
			g.color = 0xffffffff;
		}
	}
	#end

	public function showMaterialNodes() {
		// Clear input state as ui receives input events even when not drawn
		UINodes.inst.ui.endInput();

		#if (is_paint || is_sculpt)
		UINodes.inst.show = !UINodes.inst.show || UINodes.inst.canvasType != CanvasMaterial;
		UINodes.inst.canvasType = CanvasMaterial;
		#end
		#if is_lab
		UINodes.inst.show = !UINodes.inst.show;
		#end

		Base.resize();
	}

	#if (is_paint || is_sculpt)
	public function showBrushNodes() {
		// Clear input state as ui receives input events even when not drawn
		UINodes.inst.ui.endInput();
		UINodes.inst.show = !UINodes.inst.show || UINodes.inst.canvasType != CanvasBrush;
		UINodes.inst.canvasType = CanvasBrush;
		Base.resize();
	}
	#end

	public function show2DView(type: View2DType) {
		// Clear input state as ui receives input events even when not drawn
		UIView2D.inst.ui.endInput();
		if (UIView2D.inst.type != type) UIView2D.inst.show = true;
		else UIView2D.inst.show = !UIView2D.inst.show;
		UIView2D.inst.type = type;
		UIView2D.inst.hwnd.redraws = 2;
		Base.resize();
	}

	public function toggleBrowser() {
		var minimized = Config.raw.layout[LayoutStatusH] <= (UIStatus.defaultStatusH * Config.raw.window_scale);
		Config.raw.layout[LayoutStatusH] = minimized ? 240 : UIStatus.defaultStatusH;
		Config.raw.layout[LayoutStatusH] = Std.int(Config.raw.layout[LayoutStatusH] * Config.raw.window_scale);
	}

	public function setIconScale() {
		if (ui.SCALE() > 1) {
			Res.load(["icons2x.k"], function() {
				Res.bundled.set("icons.k", Res.get("icons2x.k"));
			});
		}
		else {
			Res.load(["icons.k"], function() {});
		}
	}

	function onBorderHover(handle_ptr: Int, side: Int) {
		if (!Base.uiEnabled) return;

		#if (is_paint || is_sculpt)
		if (handle_ptr != hwnds[TabSidebar0].ptr &&
			handle_ptr != hwnds[TabSidebar1].ptr &&
			handle_ptr != hwnds[TabStatus].ptr &&
			handle_ptr != UINodes.inst.hwnd.ptr &&
			handle_ptr != UIView2D.inst.hwnd.ptr) return; // Scalable handles
		if (handle_ptr == UIView2D.inst.hwnd.ptr && side != SideLeft) return;
		if (handle_ptr == UINodes.inst.hwnd.ptr && side == SideTop && !UIView2D.inst.show) return;
		if (handle_ptr == hwnds[TabSidebar0].ptr && side == SideTop) return;
		#end

		#if is_lab
		if (handle_ptr != hwnds[TabStatus].ptr &&
			handle_ptr != UINodes.inst.hwnd.ptr &&
			handle_ptr != UIView2D.inst.hwnd.ptr) return; // Scalable handles
		if (handle_ptr == UIView2D.inst.hwnd.ptr && side != SideLeft) return;
		if (handle_ptr == UINodes.inst.hwnd.ptr && side == SideTop && !UIView2D.inst.show) return;
		#end

		if (handle_ptr == UINodes.inst.hwnd.ptr && side != SideLeft && side != SideTop) return;
		if (handle_ptr == hwnds[TabStatus].ptr && side != SideTop) return;
		if (side == SideRight) return; // UI is snapped to the right side

		side == SideLeft || side == SideRight ?
			Krom.setMouseCursor(3) : // Horizontal
			Krom.setMouseCursor(4);  // Vertical

		if (Zui.current.inputStarted) {
			borderStarted = side;
			borderHandle_ptr = handle_ptr;
			Base.isResizing = true;
		}
	}

	function onTextHover() {
		Krom.setMouseCursor(2); // I-cursor
	}

	function onDeselectText() {
		#if krom_ios
		Input.getKeyboard().upListener(KeyCode.Shift);
		#end
	}

	function onTabDrop(to_ptr: Int, toPosition: Int, from_ptr: Int, fromPosition: Int) {
		var i = -1;
		var j = -1;
		for (k in 0...htabs.length) {
			if (htabs[k].ptr == to_ptr) i = k;
			if (htabs[k].ptr == from_ptr) j = k;
		}
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
