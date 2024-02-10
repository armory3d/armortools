
///if is_paint
/// <reference path='../../armorpaint/Sources/TabLayers.ts'/>
///end
///if is_sculpt
/// <reference path='../../armorsculpt/Sources/TabLayers.ts'/>
///end
///if is_forge
/// <reference path='../../armorforge/Sources/TabObjects.ts'/>
///end
/// <reference path='./TabHistory.ts'/>
/// <reference path='./TabPlugins.ts'/>
/// <reference path='./TabMaterials.ts'/>
/// <reference path='./TabBrushes.ts'/>
/// <reference path='./TabParticles.ts'/>
/// <reference path='./TabBrowser.ts'/>
/// <reference path='./TabTextures.ts'/>
/// <reference path='./TabMeshes.ts'/>
/// <reference path='./TabFonts.ts'/>
/// <reference path='./TabSwatches.ts'/>
/// <reference path='./TabScript.ts'/>
/// <reference path='./TabConsole.ts'/>
/// <reference path='./UIStatus.ts'/>

class UIBase {

	static show = true;
	static ui: zui_t;
	static borderStarted = 0;
	static borderHandle_ptr: i32 = 0;
	static action_paint_remap = "";
	static operatorSearchOffset = 0;
	static undoTapTime = 0.0;
	static redoTapTime = 0.0;

	static init_hwnds = (): zui_handle_t[] => {
		///if is_paint
		return [zui_handle_create(), zui_handle_create(), zui_handle_create()];
		///end
		///if is_sculpt
		return [zui_handle_create(), zui_handle_create(), zui_handle_create()];
		///end
		///if is_lab
		return [zui_handle_create()];
		///end
	}

	static init_htabs = (): zui_handle_t[] => {
		///if is_paint
		return [zui_handle_create(), zui_handle_create(), zui_handle_create()];
		///end
		///if is_sculpt
		return [zui_handle_create(), zui_handle_create(), zui_handle_create()];
		///end
		///if is_lab
		return [zui_handle_create()];
		///end
	}

	static init_hwnd_tabs = (): any[] => {
		///if is_paint
		return [
			[TabLayers.draw, TabHistory.draw, TabPlugins.draw
				///if is_forge
				, TabObjects.draw
				///end
			],
			[TabMaterials.draw, TabBrushes.draw, TabParticles.draw],
			[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabSwatches.draw, TabScript.draw, TabConsole.draw, UIStatus.drawVersionTab]
		];
		///end
		///if is_sculpt
		return [
			[TabLayers.draw, TabHistory.draw, TabPlugins.draw
				///if is_forge
				, TabObjects.draw
				///end
			],
			[TabMaterials.draw, TabBrushes.draw, TabParticles.draw],
			[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabScript.draw, TabConsole.draw, UIStatus.drawVersionTab]
		];
		///end
		///if is_lab
		return [
			[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabSwatches.draw, TabPlugins.draw, TabScript.draw, TabConsole.draw, UIStatus.drawVersionTab]
		];
		///end
	}

	static hwnds = UIBase.init_hwnds();
	static htabs = UIBase.init_htabs();
	static hwndTabs = UIBase.init_hwnd_tabs();

	///if (is_paint || is_sculpt)
	static defaultSidebarMiniW = 56;
	static defaultSidebarFullW = 280;
	///if (krom_android || krom_ios)
	static defaultSidebarW = UIBase.defaultSidebarMiniW;
	///else
	static defaultSidebarW = UIBase.defaultSidebarFullW;
	///end
	static tabx = 0;
	static hminimized = zui_handle_create();
	static sidebarMiniW = UIBase.defaultSidebarMiniW;
	///end

	constructor() {
		///if (is_paint || is_sculpt)
		new UIToolbar();
		UIToolbar.toolbarw = Math.floor(UIToolbar.defaultToolbarW * Config.raw.window_scale);
		Context.raw.textToolText = tr("Text");
		///end

		new UIHeader();
		new UIStatus();
		new UIMenubar();

		UIHeader.headerh = Math.floor(UIHeader.defaultHeaderH * Config.raw.window_scale);
		UIMenubar.menubarw = Math.floor(UIMenubar.defaultMenubarW * Config.raw.window_scale);

		///if (is_paint || is_sculpt)
		if (Project.materials == null) {
			Project.materials = [];
			data_get_material("Scene", "Material", (m: material_data_t) => {
				Project.materials.push(SlotMaterial.create(m));
				Context.raw.material = Project.materials[0];
			});
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(SlotBrush.create());
			Context.raw.brush = Project.brushes[0];
			MakeMaterial.parseBrush();
		}

		if (Project.fonts == null) {
			Project.fonts = [];
			Project.fonts.push(SlotFont.create("default.ttf", Base.font));
			Context.raw.font = Project.fonts[0];
		}

		if (Project.layers == null) {
			Project.layers = [];
			Project.layers.push(SlotLayer.create());
			Context.raw.layer = Project.layers[0];
		}
		///end

		///if is_lab
		if (Project.materialData == null) {
			data_get_material("Scene", "Material", (m: material_data_t) => {
				Project.materialData = m;
			});
		}

		if (Project.defaultCanvas == null) { // Synchronous
			data_get_blob("default_brush.arm", (b: ArrayBuffer) => {
				Project.defaultCanvas = b;
			});
		}

		Project.nodes = zui_nodes_create();
		Project.canvas = armpack_decode(Project.defaultCanvas);
		Project.canvas.name = "Brush 1";

		Context.parseBrushInputs();

		ParserLogic.parse(Project.canvas);
		///end

		if (Project.raw.swatches == null) {
			Project.setDefaultSwatches();
			Context.raw.swatch = Project.raw.swatches[0];
		}

		if (Context.raw.emptyEnvmap == null) {
			let b = new Uint8Array(4);
			b[0] = 8;
			b[1] = 8;
			b[2] = 8;
			b[3] = 255;
			Context.raw.emptyEnvmap = image_from_bytes(b.buffer, 1, 1);
		}
		if (Context.raw.previewEnvmap == null) {
			let b = new Uint8Array(4);
			b[0] = 0;
			b[1] = 0;
			b[2] = 0;
			b[3] = 255;
			Context.raw.previewEnvmap = image_from_bytes(b.buffer, 1, 1);
		}

		let world = scene_world;
		if (Context.raw.savedEnvmap == null) {
			// Context.raw.savedEnvmap = world._envmap;
			Context.raw.defaultIrradiance = world._irradiance;
			Context.raw.defaultRadiance = world._radiance;
			Context.raw.defaultRadianceMipmaps = world._radiance_mipmaps;
		}
		world._envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;
		Context.raw.ddirty = 1;

		History.reset();

		let scale = Config.raw.window_scale;
		UIBase.ui = zui_create({ theme: Base.theme, font: Base.font, scaleFactor: scale, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient });
		zui_set_on_border_hover(UIBase.onBorderHover);
		zui_set_on_text_hover(UIBase.onTextHover);
		zui_set_on_deselect_text(UIBase.onDeselectText);
		zui_set_on_tab_drop(UIBase.onTabDrop);

		///if (is_paint || is_sculpt)
		let resources = ["cursor.k", "icons.k"];
		///end
		///if is_lab
		let resources = ["cursor.k", "icons.k", "placeholder.k"];
		///end

		///if (is_paint || is_sculpt)
		Context.raw.gizmo = scene_get_child(".Gizmo");
		Context.raw.gizmoTranslateX = object_get_child(Context.raw.gizmo, ".TranslateX");
		Context.raw.gizmoTranslateY = object_get_child(Context.raw.gizmo, ".TranslateY");
		Context.raw.gizmoTranslateZ = object_get_child(Context.raw.gizmo, ".TranslateZ");
		Context.raw.gizmoScaleX = object_get_child(Context.raw.gizmo, ".ScaleX");
		Context.raw.gizmoScaleY = object_get_child(Context.raw.gizmo, ".ScaleY");
		Context.raw.gizmoScaleZ = object_get_child(Context.raw.gizmo, ".ScaleZ");
		Context.raw.gizmoRotateX = object_get_child(Context.raw.gizmo, ".RotateX");
		Context.raw.gizmoRotateY = object_get_child(Context.raw.gizmo, ".RotateY");
		Context.raw.gizmoRotateZ = object_get_child(Context.raw.gizmo, ".RotateZ");
		///end

		Res.load(resources, () => {});

		if (zui_SCALE(UIBase.ui) > 1) UIBase.setIconScale();

		Context.raw.paintObject = scene_get_child(".Cube").ext;
		Project.paintObjects = [Context.raw.paintObject];

		if (Project.filepath == "") {
			app_notify_on_init(Base.initLayers);
		}

		Context.raw.projectObjects = [];
		for (let m of scene_meshes) Context.raw.projectObjects.push(m);

		Operator.register("view_top", UIBase.view_top);
	}

	static update = () => {
		UIBase.updateUI();
		Operator.update();

		for (let p of Plugin.plugins.values()) if (p.update != null) p.update();

		if (!Base.uiEnabled) return;

		if (!UINodes.ui.is_typing && !UIBase.ui.is_typing) {
			if (Operator.shortcut(Config.keymap.toggle_node_editor)) {
				///if (is_paint || is_sculpt)
				UINodes.canvasType == CanvasType.CanvasMaterial ? UIBase.showMaterialNodes() : UIBase.showBrushNodes();
				///end
				///if is_lab
				UIBase.showMaterialNodes();
				///end
			}
			else if (Operator.shortcut(Config.keymap.toggle_browser)) {
				UIBase.toggleBrowser();
			}

			else if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
				///if (is_paint || is_sculpt)
				UIBase.show2DView(View2DType.View2DLayer);
				///else
				UIBase.show2DView(View2DType.View2DAsset);
				///end
			}
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.projectSaveAs();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.projectSave();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.projectOpen();
		else if (Operator.shortcut(Config.keymap.file_open_recent)) BoxProjects.show();
		else if (Operator.shortcut(Config.keymap.file_reimport_mesh)) Project.reimportMesh();
		else if (Operator.shortcut(Config.keymap.file_reimport_textures)) Project.reimportTextures();
		else if (Operator.shortcut(Config.keymap.file_new)) Project.projectNewBox();
		///if (is_paint || is_lab)
		else if (Operator.shortcut(Config.keymap.file_export_textures)) {
			if (Context.raw.textureExportPath == "") { // First export, ask for path
				///if is_paint
				Context.raw.layersExport = ExportMode.ExportVisible;
				///end
				BoxExport.showTextures();
			}
			else {
				let _init = () => {
					ExportTexture.run(Context.raw.textureExportPath);
				}
				app_notify_on_init(_init);
			}
		}
		else if (Operator.shortcut(Config.keymap.file_export_textures_as)) {
			///if (is_paint || is_sculpt)
			Context.raw.layersExport = ExportMode.ExportVisible;
			///end
			BoxExport.showTextures();
		}
		///end
		else if (Operator.shortcut(Config.keymap.file_import_assets)) Project.importAsset();
		else if (Operator.shortcut(Config.keymap.edit_prefs)) BoxPreferences.show();

		if (keyboard_started(Config.keymap.view_distract_free) ||
		   (keyboard_started("escape") && !UIBase.show && !UIBox.show)) {
			UIBase.toggleDistractFree();
		}

		///if krom_linux
		if (Operator.shortcut("alt+enter", ShortcutType.ShortcutStarted)) {
			Base.toggleFullscreen();
		}
		///end

		///if (is_paint || is_sculpt)
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);

		if ((Context.raw.brushCanLock || Context.raw.brushLocked) && mouse_moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_opacity, ShortcutType.ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_angle, ShortcutType.ShortcutDown) ||
				(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutType.ShortcutDown))) {
				if (Context.raw.brushLocked) {
					if (Operator.shortcut(Config.keymap.brush_opacity, ShortcutType.ShortcutDown)) {
						Context.raw.brushOpacity += mouse_movement_x / 500;
						Context.raw.brushOpacity = Math.max(0.0, Math.min(1.0, Context.raw.brushOpacity));
						Context.raw.brushOpacity = Math.round(Context.raw.brushOpacity * 100) / 100;
						Context.raw.brushOpacityHandle.value = Context.raw.brushOpacity;
					}
					else if (Operator.shortcut(Config.keymap.brush_angle, ShortcutType.ShortcutDown)) {
						Context.raw.brushAngle += mouse_movement_x / 5;
						Context.raw.brushAngle = Math.floor(Context.raw.brushAngle) % 360;
						if (Context.raw.brushAngle < 0) Context.raw.brushAngle += 360;
						Context.raw.brushAngleHandle.value = Context.raw.brushAngle;
						MakeMaterial.parsePaintMaterial();
					}
					else if (decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutType.ShortcutDown)) {
						Context.raw.brushDecalMaskRadius += mouse_movement_x / 150;
						Context.raw.brushDecalMaskRadius = Math.max(0.01, Math.min(4.0, Context.raw.brushDecalMaskRadius));
						Context.raw.brushDecalMaskRadius = Math.round(Context.raw.brushDecalMaskRadius * 100) / 100;
						Context.raw.brushDecalMaskRadiusHandle.value = Context.raw.brushDecalMaskRadius;
					}
					else {
						Context.raw.brushRadius += mouse_movement_x / 150;
						Context.raw.brushRadius = Math.max(0.01, Math.min(4.0, Context.raw.brushRadius));
						Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
					}
					UIHeader.headerHandle.redraws = 2;
				}
				else if (Context.raw.brushCanLock) {
					Context.raw.brushCanLock = false;
					Context.raw.brushLocked = true;
				}
			}
		}
		///end

		///if is_lab
		if ((Context.raw.brushCanLock || Context.raw.brushLocked) && mouse_moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown)) {
				if (Context.raw.brushLocked) {
					Context.raw.brushRadius += mouse_movement_x / 150;
					Context.raw.brushRadius = Math.max(0.01, Math.min(4.0, Context.raw.brushRadius));
					Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
					Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
					UIHeader.headerHandle.redraws = 2;
				}
				else if (Context.raw.brushCanLock) {
					Context.raw.brushCanLock = false;
					Context.raw.brushLocked = true;
				}
			}
		}
		///end

		let isTyping = UIBase.ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;

		///if (is_paint || is_sculpt)
		if (!isTyping) {
			if (Operator.shortcut(Config.keymap.select_material, ShortcutType.ShortcutDown)) {
				UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
				for (let i = 1; i < 10; ++i) if (keyboard_started(i + "")) Context.selectMaterial(i - 1);
			}
			else if (Operator.shortcut(Config.keymap.select_layer, ShortcutType.ShortcutDown)) {
				UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
				for (let i = 1; i < 10; ++i) if (keyboard_started(i + "")) Context.selectLayer(i - 1);
			}
		}
		///end

		// Viewport shortcuts
		if (Context.inPaintArea() && !isTyping) {

			///if is_paint
			if (!mouse_down("right")) { // Fly mode off
				if (Operator.shortcut(Config.keymap.tool_brush)) Context.selectTool(WorkspaceTool.ToolBrush);
				else if (Operator.shortcut(Config.keymap.tool_eraser)) Context.selectTool(WorkspaceTool.ToolEraser);
				else if (Operator.shortcut(Config.keymap.tool_fill)) Context.selectTool(WorkspaceTool.ToolFill);
				else if (Operator.shortcut(Config.keymap.tool_colorid)) Context.selectTool(WorkspaceTool.ToolColorId);
				else if (Operator.shortcut(Config.keymap.tool_decal)) Context.selectTool(WorkspaceTool.ToolDecal);
				else if (Operator.shortcut(Config.keymap.tool_text)) Context.selectTool(WorkspaceTool.ToolText);
				else if (Operator.shortcut(Config.keymap.tool_clone)) Context.selectTool(WorkspaceTool.ToolClone);
				else if (Operator.shortcut(Config.keymap.tool_blur)) Context.selectTool(WorkspaceTool.ToolBlur);
				else if (Operator.shortcut(Config.keymap.tool_smudge)) Context.selectTool(WorkspaceTool.ToolSmudge);
				else if (Operator.shortcut(Config.keymap.tool_particle)) Context.selectTool(WorkspaceTool.ToolParticle);
				else if (Operator.shortcut(Config.keymap.tool_picker)) Context.selectTool(WorkspaceTool.ToolPicker);
				else if (Operator.shortcut(Config.keymap.tool_bake)) Context.selectTool(WorkspaceTool.ToolBake);
				else if (Operator.shortcut(Config.keymap.tool_gizmo)) Context.selectTool(WorkspaceTool.ToolGizmo);
				else if (Operator.shortcut(Config.keymap.tool_material)) Context.selectTool(WorkspaceTool.ToolMaterial);
				else if (Operator.shortcut(Config.keymap.swap_brush_eraser)) Context.selectTool(Context.raw.tool == WorkspaceTool.ToolBrush ? WorkspaceTool.ToolEraser : WorkspaceTool.ToolBrush);
			}

			// Radius
			if (Context.raw.tool == WorkspaceTool.ToolBrush  ||
				Context.raw.tool == WorkspaceTool.ToolEraser ||
				Context.raw.tool == WorkspaceTool.ToolDecal  ||
				Context.raw.tool == WorkspaceTool.ToolText   ||
				Context.raw.tool == WorkspaceTool.ToolClone  ||
				Context.raw.tool == WorkspaceTool.ToolBlur   ||
				Context.raw.tool == WorkspaceTool.ToolSmudge   ||
				Context.raw.tool == WorkspaceTool.ToolParticle) {
				if (Operator.shortcut(Config.keymap.brush_radius) ||
					Operator.shortcut(Config.keymap.brush_opacity) ||
					Operator.shortcut(Config.keymap.brush_angle) ||
					(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius))) {
					Context.raw.brushCanLock = true;
					if (!pen_connected) mouse_lock();
					Context.raw.lockStartedX = mouse_x;
					Context.raw.lockStartedY = mouse_y;
				}
				else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutType.ShortcutRepeat)) {
					Context.raw.brushRadius -= UIBase.getRadiusIncrement();
					Context.raw.brushRadius = Math.max(Math.round(Context.raw.brushRadius * 100) / 100, 0.01);
					Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
					UIHeader.headerHandle.redraws = 2;
				}
				else if (Operator.shortcut(Config.keymap.brush_radius_increase, ShortcutType.ShortcutRepeat)) {
					Context.raw.brushRadius += UIBase.getRadiusIncrement();
					Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
					Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
					UIHeader.headerHandle.redraws = 2;
				}
				else if (decalMask) {
					if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_decrease, ShortcutType.ShortcutRepeat)) {
						Context.raw.brushDecalMaskRadius -= UIBase.getRadiusIncrement();
						Context.raw.brushDecalMaskRadius = Math.max(Math.round(Context.raw.brushDecalMaskRadius * 100) / 100, 0.01);
						Context.raw.brushDecalMaskRadiusHandle.value = Context.raw.brushDecalMaskRadius;
						UIHeader.headerHandle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_increase, ShortcutType.ShortcutRepeat)) {
						Context.raw.brushDecalMaskRadius += UIBase.getRadiusIncrement();
						Context.raw.brushDecalMaskRadius = Math.round(Context.raw.brushDecalMaskRadius * 100) / 100;
						Context.raw.brushDecalMaskRadiusHandle.value = Context.raw.brushDecalMaskRadius;
						UIHeader.headerHandle.redraws = 2;
					}
				}
			}

			if (decalMask && (Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutStarted) || Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutReleased))) {
				UIHeader.headerHandle.redraws = 2;
			}
			///end

			///if is_lab
			if (UIHeader.worktab.position == SpaceType.Space3D) {
				// Radius
				if (Context.raw.tool == WorkspaceTool.ToolEraser ||
					Context.raw.tool == WorkspaceTool.ToolClone  ||
					Context.raw.tool == WorkspaceTool.ToolBlur   ||
					Context.raw.tool == WorkspaceTool.ToolSmudge) {
					if (Operator.shortcut(Config.keymap.brush_radius)) {
						Context.raw.brushCanLock = true;
						if (!pen_connected) mouse_lock();
						Context.raw.lockStartedX = mouse_x;
						Context.raw.lockStartedY = mouse_y;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutType.ShortcutRepeat)) {
						Context.raw.brushRadius -= UIBase.getRadiusIncrement();
						Context.raw.brushRadius = Math.max(Math.round(Context.raw.brushRadius * 100) / 100, 0.01);
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
						UIHeader.headerHandle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_increase, ShortcutType.ShortcutRepeat)) {
						Context.raw.brushRadius += UIBase.getRadiusIncrement();
						Context.raw.brushRadius = Math.round(Context.raw.brushRadius * 100) / 100;
						Context.raw.brushRadiusHandle.value = Context.raw.brushRadius;
						UIHeader.headerHandle.redraws = 2;
					}
				}
			}
			///end

			// Viewpoint
			if (mouse_view_x() < app_w()) {
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
					Context.raw.cameraType = Context.raw.cameraType == CameraType.CameraPerspective ? CameraType.CameraOrthographic : CameraType.CameraPerspective;
					Context.raw.camHandle.position = Context.raw.cameraType;
					Viewport.updateCameraType(Context.raw.cameraType);
				}
				else if (Operator.shortcut(Config.keymap.view_orbit_left, ShortcutType.ShortcutRepeat)) Viewport.orbit(-Math.PI / 12, 0);
				else if (Operator.shortcut(Config.keymap.view_orbit_right, ShortcutType.ShortcutRepeat)) Viewport.orbit(Math.PI / 12, 0);
				else if (Operator.shortcut(Config.keymap.view_orbit_up, ShortcutType.ShortcutRepeat)) Viewport.orbit(0, -Math.PI / 12);
				else if (Operator.shortcut(Config.keymap.view_orbit_down, ShortcutType.ShortcutRepeat)) Viewport.orbit(0, Math.PI / 12);
				else if (Operator.shortcut(Config.keymap.view_orbit_opposite)) Viewport.orbitOpposite();
				else if (Operator.shortcut(Config.keymap.view_zoom_in, ShortcutType.ShortcutRepeat)) Viewport.zoom(0.2);
				else if (Operator.shortcut(Config.keymap.view_zoom_out, ShortcutType.ShortcutRepeat)) Viewport.zoom(-0.2);
				else if (Operator.shortcut(Config.keymap.viewport_mode)) {

					let count;

					///if (is_paint || is_sculpt)
					count = 16;
					///if (krom_direct3d12 || krom_vulkan || krom_metal)
					count += 1;
					///end
					///end

					///if is_lab
					count = 9;
					///if (krom_direct3d12 || krom_vulkan || krom_metal)
					count += 1;
					///end
					///end

					UIMenu.draw((ui: zui_t) => {
						let modeHandle = zui_handle("uibase_0");
						modeHandle.position = Context.raw.viewportMode;
						zui_text(tr("Viewport Mode"), Align.Right, ui.t.HIGHLIGHT_COL);
						let modes = [
							tr("Lit"),
							tr("Base Color"),
							tr("Normal"),
							tr("Occlusion"),
							tr("Roughness"),
							tr("Metallic"),
							tr("Opacity"),
							tr("Height"),
							///if (is_paint || is_sculpt)
							tr("Emission"),
							tr("Subsurface"),
							tr("TexCoord"),
							tr("Object Normal"),
							tr("Material ID"),
							tr("Object ID"),
							tr("Mask")
							///end
						];

						let shortcuts = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];

						///if (krom_direct3d12 || krom_vulkan || krom_metal)
						if (krom_raytrace_supported()) {
							modes.push(tr("Path Traced"));
							shortcuts.push("p");
						}
						///end

						for (let i = 0; i < modes.length; ++i) {
							zui_radio(modeHandle, i, modes[i], shortcuts[i]);
						}

						let index = shortcuts.indexOf(keyboard_key_code(ui.key));
						if (ui.is_key_pressed && index != -1) {
							modeHandle.position = index;
							ui.changed = true;
							Context.setViewportMode(modeHandle.position);
						}
						else if (modeHandle.changed) {
							Context.setViewportMode(modeHandle.position);
							ui.changed = true;
						}
					}, count);
				}
			}

			if (Operator.shortcut(Config.keymap.operator_search)) UIBase.operatorSearch();
		}

		if (Context.raw.brushCanLock || Context.raw.brushLocked) {
			if (mouse_moved && Context.raw.brushCanUnlock) {
				Context.raw.brushLocked = false;
				Context.raw.brushCanUnlock = false;
			}

			///if (is_paint || is_sculpt)
			let b = (Context.raw.brushCanLock || Context.raw.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_opacity, ShortcutType.ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_angle, ShortcutType.ShortcutDown) &&
				!(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutType.ShortcutDown));
			///end
			///if is_lab
			let b = (Context.raw.brushCanLock || Context.raw.brushLocked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown);
			///end

			if (b) {
				mouse_unlock();
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

		///if (is_paint || is_sculpt)
		if (UIBase.borderHandle_ptr != 0) {
			if (UIBase.borderHandle_ptr == UINodes.hwnd.ptr || UIBase.borderHandle_ptr == UIView2D.hwnd.ptr) {
				if (UIBase.borderStarted == BorderSide.SideLeft) {
					Config.raw.layout[LayoutSize.LayoutNodesW] -= Math.floor(mouse_movement_x);
					if (Config.raw.layout[LayoutSize.LayoutNodesW] < 32) Config.raw.layout[LayoutSize.LayoutNodesW] = 32;
					else if (Config.raw.layout[LayoutSize.LayoutNodesW] > sys_width() * 0.7) Config.raw.layout[LayoutSize.LayoutNodesW] = Math.floor(sys_width() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[LayoutSize.LayoutNodesH] -= Math.floor(mouse_movement_y);
					if (Config.raw.layout[LayoutSize.LayoutNodesH] < 32) Config.raw.layout[LayoutSize.LayoutNodesH] = 32;
					else if (Config.raw.layout[LayoutSize.LayoutNodesH] > app_h() * 0.95) Config.raw.layout[LayoutSize.LayoutNodesH] = Math.floor(app_h() * 0.95);
				}
			}
			else if (UIBase.borderHandle_ptr == UIBase.hwnds[TabArea.TabStatus].ptr) {
				let my = Math.floor(mouse_movement_y);
				if (Config.raw.layout[LayoutSize.LayoutStatusH] - my >= UIStatus.defaultStatusH * Config.raw.window_scale && Config.raw.layout[LayoutSize.LayoutStatusH] - my < sys_height() * 0.7) {
					Config.raw.layout[LayoutSize.LayoutStatusH] -= my;
				}
			}
			else {
				if (UIBase.borderStarted == BorderSide.SideLeft) {
					Config.raw.layout[LayoutSize.LayoutSidebarW] -= Math.floor(mouse_movement_x);
					if (Config.raw.layout[LayoutSize.LayoutSidebarW] < UIBase.sidebarMiniW) Config.raw.layout[LayoutSize.LayoutSidebarW] = UIBase.sidebarMiniW;
					else if (Config.raw.layout[LayoutSize.LayoutSidebarW] > sys_width() - UIBase.sidebarMiniW) Config.raw.layout[LayoutSize.LayoutSidebarW] = sys_width() - UIBase.sidebarMiniW;
				}
				else {
					let my = Math.floor(mouse_movement_y);
					if (UIBase.borderHandle_ptr == UIBase.hwnds[TabArea.TabSidebar1].ptr && UIBase.borderStarted == BorderSide.SideTop) {
						if (Config.raw.layout[LayoutSize.LayoutSidebarH0] + my > 32 && Config.raw.layout[LayoutSize.LayoutSidebarH1] - my > 32) {
							Config.raw.layout[LayoutSize.LayoutSidebarH0] += my;
							Config.raw.layout[LayoutSize.LayoutSidebarH1] -= my;
						}
					}
				}
			}
		}
		///end

		///if is_lab
		if (UIBase.borderHandle_ptr != 0) {
			if (UIBase.borderHandle_ptr == UINodes.hwnd.ptr || UIBase.borderHandle_ptr == UIView2D.hwnd.ptr) {
				if (UIBase.borderStarted == BorderSide.SideLeft) {
					Config.raw.layout[LayoutSize.LayoutNodesW] -= Math.floor(mouse_movement_x);
					if (Config.raw.layout[LayoutSize.LayoutNodesW] < 32) Config.raw.layout[LayoutSize.LayoutNodesW] = 32;
					else if (Config.raw.layout[LayoutSize.LayoutNodesW] > sys_width() * 0.7) Config.raw.layout[LayoutSize.LayoutNodesW] = Math.floor(sys_width() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[LayoutSize.LayoutNodesH] -= Math.floor(mouse_movement_y);
					if (Config.raw.layout[LayoutSize.LayoutNodesH] < 32) Config.raw.layout[LayoutSize.LayoutNodesH] = 32;
					else if (Config.raw.layout[LayoutSize.LayoutNodesH] > app_h() * 0.95) Config.raw.layout[LayoutSize.LayoutNodesH] = Math.floor(app_h() * 0.95);
				}
			}
			else if (UIBase.borderHandle_ptr == UIBase.hwnds[TabArea.TabStatus].ptr) {
				let my = Math.floor(mouse_movement_y);
				if (Config.raw.layout[LayoutSize.LayoutStatusH] - my >= UIStatus.defaultStatusH * Config.raw.window_scale && Config.raw.layout[LayoutSize.LayoutStatusH] - my < sys_height() * 0.7) {
					Config.raw.layout[LayoutSize.LayoutStatusH] -= my;
				}
			}
		}
		///end

		if (!mouse_down()) {
			UIBase.borderHandle_ptr = 0;
			Base.isResizing = false;
		}

		///if arm_physics
		if (Context.raw.tool == WorkspaceTool.ToolParticle && Context.raw.particlePhysics && Context.inPaintArea() && !Context.raw.paint2d) {
			UtilParticle.initParticlePhysics();
			let world = PhysicsWorld.active;
			PhysicsWorld.lateUpdate(world);
			Context.raw.ddirty = 2;
			Context.raw.rdirty = 2;
			if (mouse_started()) {
				if (Context.raw.particleTimer != null) {
					tween_stop(Context.raw.particleTimer);
					Context.raw.particleTimer.done();
					Context.raw.particleTimer = null;
				}
				History.pushUndo = true;
				Context.raw.particleHitX = Context.raw.particleHitY = Context.raw.particleHitZ = 0;
				scene_spawn_object(".Sphere", null, (o: object_t) => {
					data_get_material("Scene", ".Gizmo", (md: material_data_t) => {
						let mo: mesh_object_t = o.ext;
						mo.base.name = ".Bullet";
						mo.materials[0] = md;
						mo.base.visible = true;

						let camera = scene_camera;
						let ct = camera.base.transform;
						vec4_set(mo.base.transform.loc, transform_world_x(ct), transform_world_y(ct), transform_world_z(ct));
						vec4_set(mo.base.transform.scale, Context.raw.brushRadius * 0.2, Context.raw.brushRadius * 0.2, Context.raw.brushRadius * 0.2);
						transform_build_matrix(mo.base.transform);

						let body = PhysicsBody.create();
						body.shape = ShapeType.ShapeSphere;
						body.mass = 1.0;
						body.ccd = true;
						mo.base.transform.radius /= 10; // Lower ccd radius
						PhysicsBody.init(body, mo.base);
						(mo.base as any).physicsBody = body;
						mo.base.transform.radius *= 10;

						let ray = raycast_get_ray(mouse_view_x(), mouse_view_y(), camera);
						PhysicsBody.applyImpulse(body, vec4_mult(ray.dir, 0.15));

						Context.raw.particleTimer = tween_timer(5, function() { mesh_object_remove(mo); });
					});
				});
			}

			let pairs = PhysicsWorld.getContactPairs(world, Context.raw.paintBody);
			if (pairs != null) {
				for (let p of pairs) {
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
		///end
	}

	static view_top = () => {
		let isTyping = UIBase.ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;

		if (Context.inPaintArea() && !isTyping) {
			if (mouse_view_x() < app_w()) {
				Viewport.setView(0, 0, 1, 0, 0, 0);
			}
		}
	}

	static operatorSearch = () => {
		let searchHandle = zui_handle("uibase_1");
		let first = true;
		UIMenu.draw((ui: zui_t) => {
			zui_fill(0, 0, ui._w / zui_SCALE(ui), ui.t.ELEMENT_H * 8, ui.t.SEPARATOR_COL);
			let search = zui_text_input(searchHandle, "", Align.Left, true, true);
			ui.changed = false;
			if (first) {
				first = false;
				searchHandle.text = "";
				zui_start_text_edit(searchHandle); // Focus search bar
			}

			if (searchHandle.changed) UIBase.operatorSearchOffset = 0;

			if (ui.is_key_pressed) { // Move selection
				if (ui.key == key_code_t.DOWN && UIBase.operatorSearchOffset < 6) UIBase.operatorSearchOffset++;
				if (ui.key == key_code_t.UP && UIBase.operatorSearchOffset > 0) UIBase.operatorSearchOffset--;
			}
			let enter = keyboard_down("enter");
			let count = 0;
			let BUTTON_COL = ui.t.BUTTON_COL;

			for (let n in Config.keymap) {
				if (n.indexOf(search) >= 0) {
					ui.t.BUTTON_COL = count == UIBase.operatorSearchOffset ? ui.t.HIGHLIGHT_COL : ui.t.SEPARATOR_COL;
					if (zui_button(n, Align.Left, Config.keymap[n]) || (enter && count == UIBase.operatorSearchOffset)) {
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

	static toggleDistractFree = () => {
		UIBase.show = !UIBase.show;
		Base.resize();
	}

	static getRadiusIncrement = (): f32 => {
		return 0.1;
	}

	static hitRect = (mx: f32, my: f32, x: i32, y: i32, w: i32, h: i32) => {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	///if (is_paint || is_sculpt)
	static getBrushStencilRect = (): TRect => {
		let w = Math.floor(Context.raw.brushStencilImage.width * (Base.h() / Context.raw.brushStencilImage.height) * Context.raw.brushStencilScale);
		let h = Math.floor(Base.h() * Context.raw.brushStencilScale);
		let x = Math.floor(Base.x() + Context.raw.brushStencilX * Base.w());
		let y = Math.floor(Base.y() + Context.raw.brushStencilY * Base.h());
		return { w: w, h: h, x: x, y: y };
	}
	///end

	static updateUI = () => {
		if (Console.messageTimer > 0) {
			Console.messageTimer -= time_delta();
			if (Console.messageTimer <= 0) UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		}

		///if (is_paint || is_sculpt)
		UIBase.sidebarMiniW = Math.floor(UIBase.defaultSidebarMiniW * zui_SCALE(UIBase.ui));
		///end

		if (!Base.uiEnabled) return;

		///if (is_paint || is_sculpt)
		// Same mapping for paint and rotate (predefined in touch keymap)
		if (Context.inViewport()) {
			if (mouse_started() && Config.keymap.action_paint == Config.keymap.action_rotate) {
				UIBase.action_paint_remap = Config.keymap.action_paint;
				UtilRender.pickPosNorTex();
				let isMesh = Math.abs(Context.raw.posXPicked) < 50 && Math.abs(Context.raw.posYPicked) < 50 && Math.abs(Context.raw.posZPicked) < 50;
				///if krom_android
				// Allow rotating with both pen and touch, because hovering a pen prevents touch input on android
				let penOnly = false;
				///else
				let penOnly = Context.raw.penPaintingOnly;
				///end
				let isPen = penOnly && pen_down();
				// Mesh picked - disable rotate
				// Pen painting only - rotate with touch, paint with pen
				if ((isMesh && !penOnly) || isPen) {
					Config.keymap.action_rotate = "";
					Config.keymap.action_paint = UIBase.action_paint_remap;
				}
				// World sphere picked - disable paint
				else {
					Config.keymap.action_paint = "";
					Config.keymap.action_rotate = UIBase.action_paint_remap;
				}
			}
			else if (!mouse_down() && UIBase.action_paint_remap != "") {
				Config.keymap.action_rotate = UIBase.action_paint_remap;
				Config.keymap.action_paint = UIBase.action_paint_remap;
				UIBase.action_paint_remap = "";
			}
		}

		if (Context.raw.brushStencilImage != null && Operator.shortcut(Config.keymap.stencil_transform, ShortcutType.ShortcutDown)) {
			let r = UIBase.getBrushStencilRect();
			if (mouse_started("left")) {
				Context.raw.brushStencilScaling =
					UIBase.hitRect(mouse_x, mouse_y, r.x - 8,       r.y - 8,       16, 16) ||
					UIBase.hitRect(mouse_x, mouse_y, r.x - 8,       r.h + r.y - 8, 16, 16) ||
					UIBase.hitRect(mouse_x, mouse_y, r.w + r.x - 8, r.y - 8,       16, 16) ||
					UIBase.hitRect(mouse_x, mouse_y, r.w + r.x - 8, r.h + r.y - 8, 16, 16);
				let cosa = Math.cos(-Context.raw.brushStencilAngle);
				let sina = Math.sin(-Context.raw.brushStencilAngle);
				let ox = 0;
				let oy = -r.h / 2;
				let x = ox * cosa - oy * sina;
				let y = ox * sina + oy * cosa;
				x += r.x + r.w / 2;
				y += r.y + r.h / 2;
				Context.raw.brushStencilRotating =
					UIBase.hitRect(mouse_x, mouse_y, Math.floor(x - 16), Math.floor(y - 16), 32, 32);
			}
			let _scale = Context.raw.brushStencilScale;
			if (mouse_down("left")) {
				if (Context.raw.brushStencilScaling) {
					let mult = mouse_x > r.x + r.w / 2 ? 1 : -1;
					Context.raw.brushStencilScale += mouse_movement_x / 400 * mult;
				}
				else if (Context.raw.brushStencilRotating) {
					let gizmoX = r.x + r.w / 2;
					let gizmoY = r.y + r.h / 2;
					Context.raw.brushStencilAngle = -Math.atan2(mouse_y - gizmoY, mouse_x - gizmoX) - Math.PI / 2;
				}
				else {
					Context.raw.brushStencilX += mouse_movement_x / Base.w();
					Context.raw.brushStencilY += mouse_movement_y / Base.h();
				}
			}
			else Context.raw.brushStencilScaling = false;
			if (mouse_wheel_delta != 0) {
				Context.raw.brushStencilScale -= mouse_wheel_delta / 10;
			}
			// Center after scale
			let ratio = Base.h() / Context.raw.brushStencilImage.height;
			let oldW = _scale * Context.raw.brushStencilImage.width * ratio;
			let newW = Context.raw.brushStencilScale * Context.raw.brushStencilImage.width * ratio;
			let oldH = _scale * Base.h();
			let newH = Context.raw.brushStencilScale * Base.h();
			Context.raw.brushStencilX += (oldW - newW) / Base.w() / 2;
			Context.raw.brushStencilY += (oldH - newH) / Base.h() / 2;
		}
		///end

		let setCloneSource = Context.raw.tool == WorkspaceTool.ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		///if (is_paint || is_sculpt)
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
		let down = Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   decalMask ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   (pen_down() && !keyboard_down("alt"));
		///end
		///if is_lab
		let down = Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   setCloneSource ||
				   Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   (pen_down() && !keyboard_down("alt"));
		///end

		if (Config.raw.touch_ui) {
			if (pen_down()) {
				Context.raw.penPaintingOnly = true;
			}
			else if (Context.raw.penPaintingOnly) {
				down = false;
			}
		}

		///if arm_physics
		if (Context.raw.tool == WorkspaceTool.ToolParticle && Context.raw.particlePhysics) {
			down = false;
		}
		///end

		///if (is_paint || is_sculpt)
		///if krom_ios
		// No hover on iPad, decals are painted by pen release
		if (decal) {
			down = pen_released();
			if (!Context.raw.penPaintingOnly) {
				down = down || mouse_released();
			}
		}
		///end
		///end

		if (down) {
			let mx = mouse_view_x();
			let my = mouse_view_y();
			let ww = app_w();

			///if (is_paint || is_sculpt)
			if (Context.raw.paint2d) {
				mx -= app_w();
				ww = UIView2D.ww;
			}
			///end

			if (mx < ww &&
				mx > app_x() &&
				my < app_h() &&
				my > app_y()) {

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
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown)) {
							Context.raw.lastPaintVecX = Context.raw.lastPaintX;
							Context.raw.lastPaintVecY = Context.raw.lastPaintY;
						}

						///if (is_paint || is_sculpt)
						History.pushUndo = true;

						if (Context.raw.tool == WorkspaceTool.ToolClone && Context.raw.cloneStartX >= 0.0) { // Clone delta
							Context.raw.cloneDeltaX = (Context.raw.cloneStartX - mx) / ww;
							Context.raw.cloneDeltaY = (Context.raw.cloneStartY - my) / app_h();
							Context.raw.cloneStartX = -1;
						}
						else if (Context.raw.tool == WorkspaceTool.ToolParticle) {
							// Reset particles
							///if arm_particles
							let emitter: mesh_object_t = scene_get_child(".ParticleEmitter").ext;
							let psys = emitter.particle_systems[0];
							psys.time = 0;
							// psys.time = psys.seed * psys.animtime;
							// psys.seed++;
							///end
						}
						else if (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillUVIsland) {
							UtilUV.uvislandmapCached = false;
						}
						///end
					}

					Context.raw.brushTime += time_delta();

					///if (is_paint || is_sculpt)
					if (Context.raw.runBrush != null) {
						Context.raw.runBrush(0);
					}
					///end
					///if is_lab
					if (Context.runBrush != null) {
						Context.runBrush(0);
					}
					///end
				}
			}
		}
		else if (Context.raw.brushTime > 0) { // Brush released
			Context.raw.brushTime = 0;
			Context.raw.prevPaintVecX = -1;
			Context.raw.prevPaintVecY = -1;
			///if (!krom_direct3d12 && !krom_vulkan && !krom_metal) // Keep accumulated samples for D3D12
			Context.raw.ddirty = 3;
			///end
			Context.raw.brushBlendDirty = true; // Update brush mask

			///if (is_paint || is_sculpt)
			Context.raw.layerPreviewDirty = true; // Update layer preview
			///end

			///if is_paint
			// New color id picked, update fill layer
			if (Context.raw.tool == WorkspaceTool.ToolColorId && Context.raw.layer.fill_layer != null) {
				Base.notifyOnNextFrame(() => {
					Base.updateFillLayer();
					MakeMaterial.parsePaintMaterial(false);
				});
			}
			///end
		}

		///if is_paint
		if (Context.raw.layersPreviewDirty) {
			Context.raw.layersPreviewDirty = false;
			Context.raw.layerPreviewDirty = false;
			Context.raw.maskPreviewLast = null;
			if (Base.pipeMerge == null) Base.makePipe();
			// Update all layer previews
			for (let l of Project.layers) {
				if (SlotLayer.isGroup(l)) continue;
				let target = l.texpaint_preview;
				let source = l.texpaint;
				g2_begin(target, true, 0x00000000);
				// g2_set_pipeline(l.isMask() ? Base.pipeCopy8 : Base.pipeCopy);
				g2_set_pipeline(Base.pipeCopy); // texpaint_preview is always RGBA32 for now
				g2_draw_scaled_image(source, 0, 0, target.width, target.height);
				g2_set_pipeline(null);
				g2_end();
			}
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		}
		if (Context.raw.layerPreviewDirty && !SlotLayer.isGroup(Context.raw.layer)) {
			Context.raw.layerPreviewDirty = false;
			Context.raw.maskPreviewLast = null;
			if (Base.pipeMerge == null) Base.makePipe();
			// Update layer preview
			let l = Context.raw.layer;
			let target = l.texpaint_preview;
			let source = l.texpaint;
			g2_begin(target, true, 0x00000000);
			// g2_set_pipeline(Context.raw.layer.isMask() ? Base.pipeCopy8 : Base.pipeCopy);
			g2_set_pipeline(Base.pipeCopy); // texpaint_preview is always RGBA32 for now
			g2_draw_scaled_image(source, 0, 0, target.width, target.height);
			g2_set_pipeline(null);
			g2_end();
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		}
		///end

		let undoPressed = Operator.shortcut(Config.keymap.edit_undo);
		let redoPressed = Operator.shortcut(Config.keymap.edit_redo) ||
						  (keyboard_down("control") && keyboard_started("y"));

		// Two-finger tap to undo, three-finger tap to redo
		if (Context.inViewport() && Config.raw.touch_ui) {
			if (mouse_started("middle")) { UIBase.redoTapTime = time_time(); }
			else if (mouse_started("right")) { UIBase.undoTapTime = time_time(); }
			else if (mouse_released("middle") && time_time() - UIBase.redoTapTime < 0.1) { UIBase.redoTapTime = UIBase.undoTapTime = 0; redoPressed = true; }
			else if (mouse_released("right") && time_time() - UIBase.undoTapTime < 0.1) { UIBase.redoTapTime = UIBase.undoTapTime = 0; undoPressed = true; }
		}

		if (undoPressed) History.undo();
		else if (redoPressed) History.redo();

		///if (is_paint || is_sculpt)
		Gizmo.update();
		///end
	}

	static render = () => {
		if (!UIBase.show && Config.raw.touch_ui) {
			UIBase.ui.input_enabled = true;
			g2_end();
			zui_begin(UIBase.ui);
			if (zui_window(zui_handle("uibase_2"), 0, 0, 150, Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui) + 1))) {
				if (zui_button(tr("Close"))) {
					UIBase.toggleDistractFree();
				}
			}
			zui_end();
			g2_begin(null, false);
		}

		if (!UIBase.show || sys_width() == 0 || sys_height() == 0) return;

		UIBase.ui.input_enabled = Base.uiEnabled;

		// Remember last tab positions
		for (let i = 0; i < UIBase.htabs.length; ++i) {
			if (UIBase.htabs[i].changed) {
				Config.raw.layout_tabs[i] = UIBase.htabs[i].position;
				Config.save();
			}
		}

		// Set tab positions
		for (let i = 0; i < UIBase.htabs.length; ++i) {
			UIBase.htabs[i].position = Config.raw.layout_tabs[i];
		}

		g2_end();
		zui_begin(UIBase.ui);

		///if (is_paint || is_sculpt)
		UIToolbar.renderUI();
		///end
		UIMenubar.renderUI();
		UIHeader.renderUI();
		UIStatus.renderUI();

		///if (is_paint || is_sculpt)
		UIBase.drawSidebar();
		///end

		zui_end();
		g2_begin(null, false);
	}

	///if (is_paint || is_sculpt)
	static drawSidebar = () => {
		// Tabs
		let mini = Config.raw.layout[LayoutSize.LayoutSidebarW] <= UIBase.sidebarMiniW;
		let expandButtonOffset = Config.raw.touch_ui ? Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui)) : 0;
		UIBase.tabx = sys_width() - Config.raw.layout[LayoutSize.LayoutSidebarW];

		let _SCROLL_W = UIBase.ui.t.SCROLL_W;
		if (mini) UIBase.ui.t.SCROLL_W = UIBase.ui.t.SCROLL_MINI_W;

		if (zui_window(UIBase.hwnds[TabArea.TabSidebar0], UIBase.tabx, 0, Config.raw.layout[LayoutSize.LayoutSidebarW], Config.raw.layout[LayoutSize.LayoutSidebarH0])) {
			for (let i = 0; i < (mini ? 1 : UIBase.hwndTabs[TabArea.TabSidebar0].length); ++i) UIBase.hwndTabs[TabArea.TabSidebar0][i](UIBase.htabs[TabArea.TabSidebar0]);
		}
		if (zui_window(UIBase.hwnds[TabArea.TabSidebar1], UIBase.tabx, Config.raw.layout[LayoutSize.LayoutSidebarH0], Config.raw.layout[LayoutSize.LayoutSidebarW], Config.raw.layout[LayoutSize.LayoutSidebarH1] - expandButtonOffset)) {
			for (let i = 0; i < (mini ? 1 : UIBase.hwndTabs[TabArea.TabSidebar1].length); ++i) UIBase.hwndTabs[TabArea.TabSidebar1][i](UIBase.htabs[TabArea.TabSidebar1]);
		}

		zui_end_window();
		UIBase.ui.t.SCROLL_W = _SCROLL_W;

		// Collapse / expand button for mini sidebar
		if (Config.raw.touch_ui) {
			let width = Config.raw.layout[LayoutSize.LayoutSidebarW];
			let height = Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui));
			if (zui_window(zui_handle("uibase_3"), sys_width() - width, sys_height() - height, width, height + 1)) {
				UIBase.ui._w = width;
				let _BUTTON_H = UIBase.ui.t.BUTTON_H;
				let _BUTTON_COL = UIBase.ui.t.BUTTON_COL;
				UIBase.ui.t.BUTTON_H = UIBase.ui.t.ELEMENT_H;
				UIBase.ui.t.BUTTON_COL = UIBase.ui.t.WINDOW_BG_COL;
				if (zui_button(mini ? "<<" : ">>")) {
					Config.raw.layout[LayoutSize.LayoutSidebarW] = mini ? UIBase.defaultSidebarFullW : UIBase.defaultSidebarMiniW;
					Config.raw.layout[LayoutSize.LayoutSidebarW] = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarW] * zui_SCALE(UIBase.ui));
				}
				UIBase.ui.t.BUTTON_H = _BUTTON_H;
				UIBase.ui.t.BUTTON_COL = _BUTTON_COL;
			}
		}

		// Expand button
		if (Config.raw.layout[LayoutSize.LayoutSidebarW] == 0) {
			let width = Math.floor(g2_font_width(UIBase.ui.font, UIBase.ui.font_size, "<<") + 25 * zui_SCALE(UIBase.ui));
			if (zui_window(UIBase.hminimized, sys_width() - width, 0, width, Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui) + 1))) {
				UIBase.ui._w = width;
				let _BUTTON_H = UIBase.ui.t.BUTTON_H;
				let _BUTTON_COL = UIBase.ui.t.BUTTON_COL;
				UIBase.ui.t.BUTTON_H = UIBase.ui.t.ELEMENT_H;
				UIBase.ui.t.BUTTON_COL = UIBase.ui.t.SEPARATOR_COL;

				if (zui_button("<<")) {
					Config.raw.layout[LayoutSize.LayoutSidebarW] = Context.raw.maximizedSidebarWidth != 0 ? Context.raw.maximizedSidebarWidth : Math.floor(UIBase.defaultSidebarW * Config.raw.window_scale);
				}
				UIBase.ui.t.BUTTON_H = _BUTTON_H;
				UIBase.ui.t.BUTTON_COL = _BUTTON_COL;
			}
		}
		else if (UIBase.htabs[TabArea.TabSidebar0].changed && UIBase.htabs[TabArea.TabSidebar0].position == Context.raw.lastHtab0Position) {
			if (time_time() - Context.raw.selectTime < 0.25) {
				Context.raw.maximizedSidebarWidth = Config.raw.layout[LayoutSize.LayoutSidebarW];
				Config.raw.layout[LayoutSize.LayoutSidebarW] = 0;
			}
			Context.raw.selectTime = time_time();
		}
		Context.raw.lastHtab0Position = UIBase.htabs[TabArea.TabSidebar0].position;
	}

	static renderCursor = () => {
		if (!Base.uiEnabled) return;

		///if is_paint
		if (Context.raw.tool == WorkspaceTool.ToolMaterial || Context.raw.tool == WorkspaceTool.ToolBake) return;
		///end

		g2_set_color(0xffffffff);

		Context.raw.viewIndex = Context.raw.viewIndexLast;
		let mx = Base.x() + Context.raw.paintVec.x * Base.w();
		let my = Base.y() + Context.raw.paintVec.y * Base.h();
		Context.raw.viewIndex = -1;

		// Radius being scaled
		if (Context.raw.brushLocked) {
			mx += Context.raw.lockStartedX - sys_width() / 2;
			my += Context.raw.lockStartedY - sys_height() / 2;
		}

		let tool = Context.raw.tool as WorkspaceTool;

		///if is_paint
		if (Context.raw.brushStencilImage != null &&
			tool != WorkspaceTool.ToolBake &&
			tool != WorkspaceTool.ToolPicker &&
			tool != WorkspaceTool.ToolMaterial &&
			tool != WorkspaceTool.ToolColorId) {
			let r = UIBase.getBrushStencilRect();
			if (!Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown)) {
				g2_set_color(0x88ffffff);
				let angle = Context.raw.brushStencilAngle;
				let cx = r.x + r.w / 2;
				let cy = r.y + r.h / 2;
				g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(-angle)), mat3_translation(-cx, -cy)));
				g2_draw_scaled_image(Context.raw.brushStencilImage, r.x, r.y, r.w, r.h);
				g2_set_transformation(null);
				g2_set_color(0xffffffff);
			}
			let transform = Operator.shortcut(Config.keymap.stencil_transform, ShortcutType.ShortcutDown);
			if (transform) {
				// Outline
				g2_draw_rect(r.x, r.y, r.w, r.h);
				// Scale
				g2_draw_rect(r.x - 8,       r.y - 8,       16, 16);
				g2_draw_rect(r.x - 8 + r.w, r.y - 8,       16, 16);
				g2_draw_rect(r.x - 8,       r.y - 8 + r.h, 16, 16);
				g2_draw_rect(r.x - 8 + r.w, r.y - 8 + r.h, 16, 16);
				// Rotate
				let angle = Context.raw.brushStencilAngle;
				let cx = r.x + r.w / 2;
				let cy = r.y + r.h / 2;
				g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(-angle)), mat3_translation(-cx, -cy)));
				g2_fill_circle(r.x + r.w / 2, r.y - 4, 8);
				g2_set_transformation(null);
			}
		}
		///end

		// Show picked material next to cursor
		if (Context.raw.tool == WorkspaceTool.ToolPicker && Context.raw.pickerSelectMaterial && Context.raw.colorPickerCallback == null) {
			let img = Context.raw.material.imageIcon;
			///if krom_opengl
			g2_draw_scaled_image(img, mx + 10, my + 10 + img.height, img.width, -img.height);
			///else
			g2_draw_image(img, mx + 10, my + 10);
			///end
		}
		if (Context.raw.tool == WorkspaceTool.ToolPicker && Context.raw.colorPickerCallback != null) {
			let img = Res.get("icons.k");
			let rect = Res.tile50(img, WorkspaceTool.ToolPicker, 0);
			g2_draw_sub_image(img, mx + 10, my + 10, rect.x, rect.y, rect.w, rect.h);
		}

		let cursorImg = Res.get("cursor.k");
		let psize = Math.floor(cursorImg.width * (Context.raw.brushRadius * Context.raw.brushNodesRadius) * zui_SCALE(UIBase.ui));

		// Clone source cursor
		if (Context.raw.tool == WorkspaceTool.ToolClone && !keyboard_down("alt") && (mouse_down() || pen_down())) {
			g2_set_color(0x66ffffff);
			g2_draw_scaled_image(cursorImg, mx + Context.raw.cloneDeltaX * app_w() - psize / 2, my + Context.raw.cloneDeltaY * app_h() - psize / 2, psize, psize);
			g2_set_color(0xffffffff);
		}

		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;

		if (!Config.raw.brush_3d || Context.in2dView() || decal) {
			let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
			if (decal && !Context.inNodes()) {
				let decalAlpha = 0.5;
				if (!decalMask) {
					Context.raw.decalX = Context.raw.paintVec.x;
					Context.raw.decalY = Context.raw.paintVec.y;
					decalAlpha = Context.raw.brushOpacity;

					// Radius being scaled
					if (Context.raw.brushLocked) {
						Context.raw.decalX += (Context.raw.lockStartedX - sys_width() / 2) / Base.w();
						Context.raw.decalY += (Context.raw.lockStartedY - sys_height() / 2) / Base.h();
					}
				}

				if (!Config.raw.brush_live) {
					let psizex = Math.floor(256 * zui_SCALE(UIBase.ui) * (Context.raw.brushRadius * Context.raw.brushNodesRadius * Context.raw.brushScaleX));
					let psizey = Math.floor(256 * zui_SCALE(UIBase.ui) * (Context.raw.brushRadius * Context.raw.brushNodesRadius));

					Context.raw.viewIndex = Context.raw.viewIndexLast;
					let decalX = Base.x() + Context.raw.decalX * Base.w() - psizex / 2;
					let decalY = Base.y() + Context.raw.decalY * Base.h() - psizey / 2;
					Context.raw.viewIndex = -1;

					g2_set_color(color_from_floats(1, 1, 1, decalAlpha));
					let angle = (Context.raw.brushAngle + Context.raw.brushNodesAngle) * (Math.PI / 180);
					let cx = decalX + psizex / 2;
					let cy = decalY + psizey / 2;
					g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(angle)), mat3_translation(-cx, -cy)));
					///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
					g2_draw_scaled_image(Context.raw.decalImage, decalX, decalY, psizex, psizey);
					///else
					g2_draw_scaled_image(Context.raw.decalImage, decalX, decalY + psizey, psizex, -psizey);
					///end
					g2_set_transformation(null);
					g2_set_color(0xffffffff);
				}
			}
			if (Context.raw.tool == WorkspaceTool.ToolBrush  ||
				Context.raw.tool == WorkspaceTool.ToolEraser ||
				Context.raw.tool == WorkspaceTool.ToolClone  ||
				Context.raw.tool == WorkspaceTool.ToolBlur   ||
				Context.raw.tool == WorkspaceTool.ToolSmudge   ||
				Context.raw.tool == WorkspaceTool.ToolParticle ||
				(decalMask && !Config.raw.brush_3d) ||
				(decalMask && Context.in2dView())) {
				if (decalMask) {
					psize = Math.floor(cursorImg.width * (Context.raw.brushDecalMaskRadius * Context.raw.brushNodesRadius) * zui_SCALE(UIBase.ui));
				}
				if (Config.raw.brush_3d && Context.in2dView()) {
					psize = Math.floor(psize * UIView2D.panScale);
				}
				g2_draw_scaled_image(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
			}
		}

		if (Context.raw.brushLazyRadius > 0 && !Context.raw.brushLocked &&
			(Context.raw.tool == WorkspaceTool.ToolBrush ||
			 Context.raw.tool == WorkspaceTool.ToolEraser ||
			 Context.raw.tool == WorkspaceTool.ToolDecal ||
			 Context.raw.tool == WorkspaceTool.ToolText ||
			 Context.raw.tool == WorkspaceTool.ToolClone ||
			 Context.raw.tool == WorkspaceTool.ToolBlur ||
			 Context.raw.tool == WorkspaceTool.ToolSmudge ||
			 Context.raw.tool == WorkspaceTool.ToolParticle)) {
			g2_fill_rect(mx - 1, my - 1, 2, 2);
			mx = Context.raw.brushLazyX * Base.w() + Base.x();
			my = Context.raw.brushLazyY * Base.h() + Base.y();
			let radius = Context.raw.brushLazyRadius * 180;
			g2_set_color(0xff666666);
			g2_draw_scaled_image(cursorImg, mx - radius / 2, my - radius / 2, radius, radius);
			g2_set_color(0xffffffff);
		}
	}
	///end

	static showMaterialNodes = () => {
		// Clear input state as ui receives input events even when not drawn
		zui_end_input();

		///if (is_paint || is_sculpt)
		UINodes.show = !UINodes.show || UINodes.canvasType != CanvasType.CanvasMaterial;
		UINodes.canvasType = CanvasType.CanvasMaterial;
		///end
		///if is_lab
		UINodes.show = !UINodes.show;
		///end

		Base.resize();
	}

	///if (is_paint || is_sculpt)
	static showBrushNodes = () => {
		// Clear input state as ui receives input events even when not drawn
		zui_end_input();
		UINodes.show = !UINodes.show || UINodes.canvasType != CanvasType.CanvasBrush;
		UINodes.canvasType = CanvasType.CanvasBrush;
		Base.resize();
	}
	///end

	static show2DView = (type: View2DType) => {
		// Clear input state as ui receives input events even when not drawn
		zui_end_input();
		if (UIView2D.type != type) UIView2D.show = true;
		else UIView2D.show = !UIView2D.show;
		UIView2D.type = type;
		UIView2D.hwnd.redraws = 2;
		Base.resize();
	}

	static toggleBrowser = () => {
		let minimized = Config.raw.layout[LayoutSize.LayoutStatusH] <= (UIStatus.defaultStatusH * Config.raw.window_scale);
		Config.raw.layout[LayoutSize.LayoutStatusH] = minimized ? 240 : UIStatus.defaultStatusH;
		Config.raw.layout[LayoutSize.LayoutStatusH] = Math.floor(Config.raw.layout[LayoutSize.LayoutStatusH] * Config.raw.window_scale);
	}

	static setIconScale = () => {
		if (zui_SCALE(UIBase.ui) > 1) {
			Res.load(["icons2x.k"], () => {
				Res.bundled.set("icons.k", Res.get("icons2x.k"));
			});
		}
		else {
			Res.load(["icons.k"], () => {});
		}
	}

	static onBorderHover = (handle_ptr: i32, side: i32) => {
		if (!Base.uiEnabled) return;

		///if (is_paint || is_sculpt)
		if (handle_ptr != UIBase.hwnds[TabArea.TabSidebar0].ptr &&
			handle_ptr != UIBase.hwnds[TabArea.TabSidebar1].ptr &&
			handle_ptr != UIBase.hwnds[TabArea.TabStatus].ptr &&
			handle_ptr != UINodes.hwnd.ptr &&
			handle_ptr != UIView2D.hwnd.ptr) return; // Scalable handles
		if (handle_ptr == UIView2D.hwnd.ptr && side != BorderSide.SideLeft) return;
		if (handle_ptr == UINodes.hwnd.ptr && side == BorderSide.SideTop && !UIView2D.show) return;
		if (handle_ptr == UIBase.hwnds[TabArea.TabSidebar0].ptr && side == BorderSide.SideTop) return;
		///end

		///if is_lab
		if (handle_ptr != UIBase.hwnds[TabArea.TabStatus].ptr &&
			handle_ptr != UINodes.hwnd.ptr &&
			handle_ptr != UIView2D.hwnd.ptr) return; // Scalable handles
		if (handle_ptr == UIView2D.hwnd.ptr && side != BorderSide.SideLeft) return;
		if (handle_ptr == UINodes.hwnd.ptr && side == BorderSide.SideTop && !UIView2D.show) return;
		///end

		if (handle_ptr == UINodes.hwnd.ptr && side != BorderSide.SideLeft && side != BorderSide.SideTop) return;
		if (handle_ptr == UIBase.hwnds[TabArea.TabStatus].ptr && side != BorderSide.SideTop) return;
		if (side == BorderSide.SideRight) return; // UI is snapped to the right side

		side == BorderSide.SideLeft || side == BorderSide.SideRight ?
			krom_set_mouse_cursor(3) : // Horizontal
			krom_set_mouse_cursor(4);  // Vertical

		if (zui_current.input_started) {
			UIBase.borderStarted = side;
			UIBase.borderHandle_ptr = handle_ptr;
			Base.isResizing = true;
		}
	}

	static onTextHover = () => {
		krom_set_mouse_cursor(2); // I-cursor
	}

	static onDeselectText = () => {
		///if krom_ios
		keyboard_up_listener(key_code_t.SHIFT);
		///end
	}

	static onTabDrop = (to_ptr: i32, toPosition: i32, from_ptr: i32, fromPosition: i32) => {
		let i = -1;
		let j = -1;
		for (let k = 0; k < UIBase.htabs.length; ++k) {
			if (UIBase.htabs[k].ptr == to_ptr) i = k;
			if (UIBase.htabs[k].ptr == from_ptr) j = k;
		}
		if (i > -1 && j > -1) {
			let element = UIBase.hwndTabs[j][fromPosition];
			UIBase.hwndTabs[j].splice(fromPosition, 1);
			UIBase.hwndTabs[i].splice(toPosition, 0, element);
			UIBase.hwnds[i].redraws = 2;
			UIBase.hwnds[j].redraws = 2;
		}
	}

	static tagUIRedraw = () => {
		UIHeader.headerHandle.redraws = 2;
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		UIMenubar.workspaceHandle.redraws = 2;
		UIMenubar.menuHandle.redraws = 2;
		///if (is_paint || is_sculpt)
		UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
		UIToolbar.toolbarHandle.redraws = 2;
		///end
	}
}
