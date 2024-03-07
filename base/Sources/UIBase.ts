
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

	static show: bool = true;
	static ui: zui_t;
	static border_started: i32 = 0;
	static border_handle_ptr: i32 = 0;
	static action_paint_remap: string = "";
	static operator_search_offset: i32 = 0;
	static undo_tap_time: f32 = 0.0;
	static redo_tap_time: f32 = 0.0;

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
			[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabSwatches.draw, TabScript.draw, TabConsole.draw, UIStatus.draw_version_tab]
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
			[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabFonts.draw, TabScript.draw, TabConsole.draw, UIStatus.draw_version_tab]
		];
		///end
		///if is_lab
		return [
			[TabBrowser.draw, TabTextures.draw, TabMeshes.draw, TabSwatches.draw, TabPlugins.draw, TabScript.draw, TabConsole.draw, UIStatus.draw_version_tab]
		];
		///end
	}

	static hwnds: zui_handle_t[] = UIBase.init_hwnds();
	static htabs: zui_handle_t[] = UIBase.init_htabs();
	static hwnd_tabs: any[] = UIBase.init_hwnd_tabs();

	///if (is_paint || is_sculpt)
	static default_sidebar_mini_w: i32 = 56;
	static default_sidebar_full_w: i32 = 280;
	///if (krom_android || krom_ios)
	static default_sidebar_w: i32 = UIBase.default_sidebar_mini_w;
	///else
	static default_sidebar_w: i32 = UIBase.default_sidebar_full_w;
	///end
	static tabx: i32 = 0;
	static hminimized: zui_handle_t = zui_handle_create();
	static sidebar_mini_w: i32 = UIBase.default_sidebar_mini_w;
	///end

	constructor() {
		///if (is_paint || is_sculpt)
		new UIToolbar();
		UIToolbar.toolbar_w = Math.floor(UIToolbar.default_toolbar_w * Config.raw.window_scale);
		Context.raw.text_tool_text = tr("Text");
		///end

		new UIHeader();
		new UIStatus();
		new UIMenubar();

		UIHeader.headerh = Math.floor(UIHeader.default_header_h * Config.raw.window_scale);
		UIMenubar.menubarw = Math.floor(UIMenubar.default_menubar_w * Config.raw.window_scale);

		///if (is_paint || is_sculpt)
		if (Project.materials == null) {
			Project.materials = [];
			let m: material_data_t = data_get_material("Scene", "Material");
			Project.materials.push(SlotMaterial.create(m));
			Context.raw.material = Project.materials[0];
		}

		if (Project.brushes == null) {
			Project.brushes = [];
			Project.brushes.push(SlotBrush.create());
			Context.raw.brush = Project.brushes[0];
			MakeMaterial.parse_brush();
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
		if (Project.material_data == null) {
			let m: material_data_t = data_get_material("Scene", "Material");
			Project.material_data = m;
		}

		if (Project.default_canvas == null) { // Synchronous
			let b: ArrayBuffer = data_get_blob("default_brush.arm");
			Project.default_canvas = b;
		}

		Project.nodes = zui_nodes_create();
		Project.canvas = armpack_decode(Project.default_canvas);
		Project.canvas.name = "Brush 1";

		Context.parse_brush_inputs();

		ParserLogic.parse(Project.canvas);
		///end

		if (Project.raw.swatches == null) {
			Project.set_default_swatches();
			Context.raw.swatch = Project.raw.swatches[0];
		}

		if (Context.raw.empty_envmap == null) {
			let b: Uint8Array = new Uint8Array(4);
			b[0] = 8;
			b[1] = 8;
			b[2] = 8;
			b[3] = 255;
			Context.raw.empty_envmap = image_from_bytes(b.buffer, 1, 1);
		}
		if (Context.raw.preview_envmap == null) {
			let b: Uint8Array = new Uint8Array(4);
			b[0] = 0;
			b[1] = 0;
			b[2] = 0;
			b[3] = 255;
			Context.raw.preview_envmap = image_from_bytes(b.buffer, 1, 1);
		}

		let world: world_data_t = scene_world;
		if (Context.raw.saved_envmap == null) {
			// Context.raw.savedEnvmap = world._envmap;
			Context.raw.default_irradiance = world._.irradiance;
			Context.raw.default_radiance = world._.radiance;
			Context.raw.default_radiance_mipmaps = world._.radiance_mipmaps;
		}
		world._.envmap = Context.raw.show_envmap ? Context.raw.saved_envmap : Context.raw.empty_envmap;
		Context.raw.ddirty = 1;

		History.reset();

		let scale: f32 = Config.raw.window_scale;
		UIBase.ui = zui_create({ theme: Base.theme, font: Base.font, scale_factor: scale, color_wheel: Base.color_wheel, black_white_gradient: Base.color_wheel_gradient });
		zui_set_on_border_hover(UIBase.on_border_hover);
		zui_set_on_text_hover(UIBase.on_text_hover);
		zui_set_on_deselect_text(UIBase.on_deselect_text);
		zui_set_on_tab_drop(UIBase.on_tab_drop);

		///if (is_paint || is_sculpt)
		let resources: string[] = ["cursor.k", "icons.k"];
		///end
		///if is_lab
		let resources: string[] = ["cursor.k", "icons.k", "placeholder.k"];
		///end

		///if (is_paint || is_sculpt)
		Context.raw.gizmo = scene_get_child(".Gizmo");
		Context.raw.gizmo_translate_x = object_get_child(Context.raw.gizmo, ".TranslateX");
		Context.raw.gizmo_translate_y = object_get_child(Context.raw.gizmo, ".TranslateY");
		Context.raw.gizmo_translate_z = object_get_child(Context.raw.gizmo, ".TranslateZ");
		Context.raw.gizmo_scale_x = object_get_child(Context.raw.gizmo, ".ScaleX");
		Context.raw.gizmo_scale_y = object_get_child(Context.raw.gizmo, ".ScaleY");
		Context.raw.gizmo_scale_z = object_get_child(Context.raw.gizmo, ".ScaleZ");
		Context.raw.gizmo_rotate_x = object_get_child(Context.raw.gizmo, ".RotateX");
		Context.raw.gizmo_rotate_y = object_get_child(Context.raw.gizmo, ".RotateY");
		Context.raw.gizmo_rotate_z = object_get_child(Context.raw.gizmo, ".RotateZ");
		///end

		Res.load(resources, () => {});

		if (zui_SCALE(UIBase.ui) > 1) UIBase.set_icon_scale();

		Context.raw.paint_object = scene_get_child(".Cube").ext;
		Project.paint_objects = [Context.raw.paint_object];

		if (Project.filepath == "") {
			app_notify_on_init(Base.init_layers);
		}

		Context.raw.project_objects = [];
		for (let m of scene_meshes) Context.raw.project_objects.push(m);

		Operator.register("view_top", UIBase.view_top);
	}

	static update = () => {
		UIBase.update_ui();
		Operator.update();

		for (let p of Plugin.plugins.values()) if (p.update != null) p.update();

		if (!Base.ui_enabled) return;

		if (!UINodes.ui.is_typing && !UIBase.ui.is_typing) {
			if (Operator.shortcut(Config.keymap.toggle_node_editor)) {
				///if (is_paint || is_sculpt)
				UINodes.canvas_type == canvas_type_t.MATERIAL ? UIBase.show_material_nodes() : UIBase.show_brush_nodes();
				///end
				///if is_lab
				UIBase.show_material_nodes();
				///end
			}
			else if (Operator.shortcut(Config.keymap.toggle_browser)) {
				UIBase.toggle_browser();
			}

			else if (Operator.shortcut(Config.keymap.toggle_2d_view)) {
				///if (is_paint || is_sculpt)
				UIBase.show_2d_view(view_2d_type_t.LAYER);
				///else
				UIBase.show_2d_view(view_2d_type_t.ASSET);
				///end
			}
		}

		if (Operator.shortcut(Config.keymap.file_save_as)) Project.project_save_as();
		else if (Operator.shortcut(Config.keymap.file_save)) Project.project_save();
		else if (Operator.shortcut(Config.keymap.file_open)) Project.project_open();
		else if (Operator.shortcut(Config.keymap.file_open_recent)) BoxProjects.show();
		else if (Operator.shortcut(Config.keymap.file_reimport_mesh)) Project.reimport_mesh();
		else if (Operator.shortcut(Config.keymap.file_reimport_textures)) Project.reimport_textures();
		else if (Operator.shortcut(Config.keymap.file_new)) Project.project_new_box();
		///if (is_paint || is_lab)
		else if (Operator.shortcut(Config.keymap.file_export_textures)) {
			if (Context.raw.texture_export_path == "") { // First export, ask for path
				///if is_paint
				Context.raw.layers_export = export_mode_t.VISIBLE;
				///end
				BoxExport.show_textures();
			}
			else {
				let _init = () => {
					ExportTexture.run(Context.raw.texture_export_path);
				}
				app_notify_on_init(_init);
			}
		}
		else if (Operator.shortcut(Config.keymap.file_export_textures_as)) {
			///if (is_paint || is_sculpt)
			Context.raw.layers_export = export_mode_t.VISIBLE;
			///end
			BoxExport.show_textures();
		}
		///end
		else if (Operator.shortcut(Config.keymap.file_import_assets)) Project.import_asset();
		else if (Operator.shortcut(Config.keymap.edit_prefs)) BoxPreferences.show();

		if (keyboard_started(Config.keymap.view_distract_free) ||
		   (keyboard_started("escape") && !UIBase.show && !UIBox.show)) {
			UIBase.toggle_distract_free();
		}

		///if krom_linux
		if (Operator.shortcut("alt+enter", ShortcutType.ShortcutStarted)) {
			Base.toggle_fullscreen();
		}
		///end

		///if (is_paint || is_sculpt)
		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		let decalMask: bool = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);

		if ((Context.raw.brush_can_lock || Context.raw.brush_locked) && mouse_moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_opacity, ShortcutType.ShortcutDown) ||
				Operator.shortcut(Config.keymap.brush_angle, ShortcutType.ShortcutDown) ||
				(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutType.ShortcutDown))) {
				if (Context.raw.brush_locked) {
					if (Operator.shortcut(Config.keymap.brush_opacity, ShortcutType.ShortcutDown)) {
						Context.raw.brush_opacity += mouse_movement_x / 500;
						Context.raw.brush_opacity = Math.max(0.0, Math.min(1.0, Context.raw.brush_opacity));
						Context.raw.brush_opacity = Math.round(Context.raw.brush_opacity * 100) / 100;
						Context.raw.brush_opacity_handle.value = Context.raw.brush_opacity;
					}
					else if (Operator.shortcut(Config.keymap.brush_angle, ShortcutType.ShortcutDown)) {
						Context.raw.brush_angle += mouse_movement_x / 5;
						Context.raw.brush_angle = Math.floor(Context.raw.brush_angle) % 360;
						if (Context.raw.brush_angle < 0) Context.raw.brush_angle += 360;
						Context.raw.brush_angle_handle.value = Context.raw.brush_angle;
						MakeMaterial.parse_paint_material();
					}
					else if (decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutType.ShortcutDown)) {
						Context.raw.brush_decal_mask_radius += mouse_movement_x / 150;
						Context.raw.brush_decal_mask_radius = Math.max(0.01, Math.min(4.0, Context.raw.brush_decal_mask_radius));
						Context.raw.brush_decal_mask_radius = Math.round(Context.raw.brush_decal_mask_radius * 100) / 100;
						Context.raw.brush_decal_mask_radius_handle.value = Context.raw.brush_decal_mask_radius;
					}
					else {
						Context.raw.brush_radius += mouse_movement_x / 150;
						Context.raw.brush_radius = Math.max(0.01, Math.min(4.0, Context.raw.brush_radius));
						Context.raw.brush_radius = Math.round(Context.raw.brush_radius * 100) / 100;
						Context.raw.brush_radius_handle.value = Context.raw.brush_radius;
					}
					UIHeader.header_handle.redraws = 2;
				}
				else if (Context.raw.brush_can_lock) {
					Context.raw.brush_can_lock = false;
					Context.raw.brush_locked = true;
				}
			}
		}
		///end

		///if is_lab
		if ((Context.raw.brush_can_lock || Context.raw.brush_locked) && mouse_moved) {
			if (Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown)) {
				if (Context.raw.brush_locked) {
					Context.raw.brush_radius += mouse_movement_x / 150;
					Context.raw.brush_radius = Math.max(0.01, Math.min(4.0, Context.raw.brush_radius));
					Context.raw.brush_radius = Math.round(Context.raw.brush_radius * 100) / 100;
					Context.raw.brush_radius_handle.value = Context.raw.brush_radius;
					UIHeader.header_handle.redraws = 2;
				}
				else if (Context.raw.brush_can_lock) {
					Context.raw.brush_can_lock = false;
					Context.raw.brush_locked = true;
				}
			}
		}
		///end

		let isTyping: bool = UIBase.ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;

		///if (is_paint || is_sculpt)
		if (!isTyping) {
			if (Operator.shortcut(Config.keymap.select_material, ShortcutType.ShortcutDown)) {
				UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
				for (let i: i32 = 1; i < 10; ++i) if (keyboard_started(i + "")) Context.select_material(i - 1);
			}
			else if (Operator.shortcut(Config.keymap.select_layer, ShortcutType.ShortcutDown)) {
				UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
				for (let i: i32 = 1; i < 10; ++i) if (keyboard_started(i + "")) Context.select_layer(i - 1);
			}
		}
		///end

		// Viewport shortcuts
		if (Context.in_paint_area() && !isTyping) {

			///if is_paint
			if (!mouse_down("right")) { // Fly mode off
				if (Operator.shortcut(Config.keymap.tool_brush)) Context.select_tool(workspace_tool_t.BRUSH);
				else if (Operator.shortcut(Config.keymap.tool_eraser)) Context.select_tool(workspace_tool_t.ERASER);
				else if (Operator.shortcut(Config.keymap.tool_fill)) Context.select_tool(workspace_tool_t.FILL);
				else if (Operator.shortcut(Config.keymap.tool_colorid)) Context.select_tool(workspace_tool_t.COLORID);
				else if (Operator.shortcut(Config.keymap.tool_decal)) Context.select_tool(workspace_tool_t.DECAL);
				else if (Operator.shortcut(Config.keymap.tool_text)) Context.select_tool(workspace_tool_t.TEXT);
				else if (Operator.shortcut(Config.keymap.tool_clone)) Context.select_tool(workspace_tool_t.CLONE);
				else if (Operator.shortcut(Config.keymap.tool_blur)) Context.select_tool(workspace_tool_t.BLUR);
				else if (Operator.shortcut(Config.keymap.tool_smudge)) Context.select_tool(workspace_tool_t.SMUDGE);
				else if (Operator.shortcut(Config.keymap.tool_particle)) Context.select_tool(workspace_tool_t.PARTICLE);
				else if (Operator.shortcut(Config.keymap.tool_picker)) Context.select_tool(workspace_tool_t.PICKER);
				else if (Operator.shortcut(Config.keymap.tool_bake)) Context.select_tool(workspace_tool_t.BAKE);
				else if (Operator.shortcut(Config.keymap.tool_gizmo)) Context.select_tool(workspace_tool_t.GIZMO);
				else if (Operator.shortcut(Config.keymap.tool_material)) Context.select_tool(workspace_tool_t.MATERIAL);
				else if (Operator.shortcut(Config.keymap.swap_brush_eraser)) Context.select_tool(Context.raw.tool == workspace_tool_t.BRUSH ? workspace_tool_t.ERASER : workspace_tool_t.BRUSH);
			}

			// Radius
			if (Context.raw.tool == workspace_tool_t.BRUSH  ||
				Context.raw.tool == workspace_tool_t.ERASER ||
				Context.raw.tool == workspace_tool_t.DECAL  ||
				Context.raw.tool == workspace_tool_t.TEXT   ||
				Context.raw.tool == workspace_tool_t.CLONE  ||
				Context.raw.tool == workspace_tool_t.BLUR   ||
				Context.raw.tool == workspace_tool_t.SMUDGE   ||
				Context.raw.tool == workspace_tool_t.PARTICLE) {
				if (Operator.shortcut(Config.keymap.brush_radius) ||
					Operator.shortcut(Config.keymap.brush_opacity) ||
					Operator.shortcut(Config.keymap.brush_angle) ||
					(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius))) {
					Context.raw.brush_can_lock = true;
					if (!pen_connected) mouse_lock();
					Context.raw.lock_started_x = mouse_x;
					Context.raw.lock_started_y = mouse_y;
				}
				else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutType.ShortcutRepeat)) {
					Context.raw.brush_radius -= UIBase.get_radius_increment();
					Context.raw.brush_radius = Math.max(Math.round(Context.raw.brush_radius * 100) / 100, 0.01);
					Context.raw.brush_radius_handle.value = Context.raw.brush_radius;
					UIHeader.header_handle.redraws = 2;
				}
				else if (Operator.shortcut(Config.keymap.brush_radius_increase, ShortcutType.ShortcutRepeat)) {
					Context.raw.brush_radius += UIBase.get_radius_increment();
					Context.raw.brush_radius = Math.round(Context.raw.brush_radius * 100) / 100;
					Context.raw.brush_radius_handle.value = Context.raw.brush_radius;
					UIHeader.header_handle.redraws = 2;
				}
				else if (decalMask) {
					if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_decrease, ShortcutType.ShortcutRepeat)) {
						Context.raw.brush_decal_mask_radius -= UIBase.get_radius_increment();
						Context.raw.brush_decal_mask_radius = Math.max(Math.round(Context.raw.brush_decal_mask_radius * 100) / 100, 0.01);
						Context.raw.brush_decal_mask_radius_handle.value = Context.raw.brush_decal_mask_radius;
						UIHeader.header_handle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius_increase, ShortcutType.ShortcutRepeat)) {
						Context.raw.brush_decal_mask_radius += UIBase.get_radius_increment();
						Context.raw.brush_decal_mask_radius = Math.round(Context.raw.brush_decal_mask_radius * 100) / 100;
						Context.raw.brush_decal_mask_radius_handle.value = Context.raw.brush_decal_mask_radius;
						UIHeader.header_handle.redraws = 2;
					}
				}
			}

			if (decalMask && (Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutStarted) || Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutReleased))) {
				UIHeader.header_handle.redraws = 2;
			}
			///end

			///if is_lab
			if (UIHeader.worktab.position == space_type_t.SPACE3D) {
				// Radius
				if (Context.raw.tool == workspace_tool_t.ERASER ||
					Context.raw.tool == workspace_tool_t.CLONE  ||
					Context.raw.tool == workspace_tool_t.BLUR   ||
					Context.raw.tool == workspace_tool_t.SMUDGE) {
					if (Operator.shortcut(Config.keymap.brush_radius)) {
						Context.raw.brush_can_lock = true;
						if (!pen_connected) mouse_lock();
						Context.raw.lock_started_x = mouse_x;
						Context.raw.lock_started_y = mouse_y;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_decrease, ShortcutType.ShortcutRepeat)) {
						Context.raw.brush_radius -= UIBase.get_radius_increment();
						Context.raw.brush_radius = Math.max(Math.round(Context.raw.brush_radius * 100) / 100, 0.01);
						Context.raw.brush_radius_handle.value = Context.raw.brush_radius;
						UIHeader.header_handle.redraws = 2;
					}
					else if (Operator.shortcut(Config.keymap.brush_radius_increase, ShortcutType.ShortcutRepeat)) {
						Context.raw.brush_radius += UIBase.get_radius_increment();
						Context.raw.brush_radius = Math.round(Context.raw.brush_radius * 100) / 100;
						Context.raw.brush_radius_handle.value = Context.raw.brush_radius;
						UIHeader.header_handle.redraws = 2;
					}
				}
			}
			///end

			// Viewpoint
			if (mouse_view_x() < app_w()) {
				if (Operator.shortcut(Config.keymap.view_reset)) {
					Viewport.reset();
					Viewport.scale_to_bounds();
				}
				else if (Operator.shortcut(Config.keymap.view_back)) Viewport.set_view(0, 1, 0, Math.PI / 2, 0, Math.PI);
				else if (Operator.shortcut(Config.keymap.view_front)) Viewport.set_view(0, -1, 0, Math.PI / 2, 0, 0);
				else if (Operator.shortcut(Config.keymap.view_left)) Viewport.set_view(-1, 0, 0, Math.PI / 2, 0, -Math.PI / 2);
				else if (Operator.shortcut(Config.keymap.view_right)) Viewport.set_view(1, 0, 0, Math.PI / 2, 0, Math.PI / 2);
				else if (Operator.shortcut(Config.keymap.view_bottom)) Viewport.set_view(0, 0, -1, Math.PI, 0, Math.PI);
				else if (Operator.shortcut(Config.keymap.view_camera_type)) {
					Context.raw.camera_type = Context.raw.camera_type == camera_type_t.PERSPECTIVE ? camera_type_t.ORTHOGRAPHIC : camera_type_t.PERSPECTIVE;
					Context.raw.cam_handle.position = Context.raw.camera_type;
					Viewport.update_camera_type(Context.raw.camera_type);
				}
				else if (Operator.shortcut(Config.keymap.view_orbit_left, ShortcutType.ShortcutRepeat)) Viewport.orbit(-Math.PI / 12, 0);
				else if (Operator.shortcut(Config.keymap.view_orbit_right, ShortcutType.ShortcutRepeat)) Viewport.orbit(Math.PI / 12, 0);
				else if (Operator.shortcut(Config.keymap.view_orbit_up, ShortcutType.ShortcutRepeat)) Viewport.orbit(0, -Math.PI / 12);
				else if (Operator.shortcut(Config.keymap.view_orbit_down, ShortcutType.ShortcutRepeat)) Viewport.orbit(0, Math.PI / 12);
				else if (Operator.shortcut(Config.keymap.view_orbit_opposite)) Viewport.orbit_opposite();
				else if (Operator.shortcut(Config.keymap.view_zoom_in, ShortcutType.ShortcutRepeat)) Viewport.zoom(0.2);
				else if (Operator.shortcut(Config.keymap.view_zoom_out, ShortcutType.ShortcutRepeat)) Viewport.zoom(-0.2);
				else if (Operator.shortcut(Config.keymap.viewport_mode)) {

					let count: i32;

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
						let modeHandle: zui_handle_t = zui_handle("uibase_0");
						modeHandle.position = Context.raw.viewport_mode;
						zui_text(tr("Viewport Mode"), zui_align_t.RIGHT, ui.t.HIGHLIGHT_COL);
						let modes: string[] = [
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

						let shortcuts: string[] = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];

						///if (krom_direct3d12 || krom_vulkan || krom_metal)
						if (krom_raytrace_supported()) {
							modes.push(tr("Path Traced"));
							shortcuts.push("p");
						}
						///end

						for (let i: i32 = 0; i < modes.length; ++i) {
							zui_radio(modeHandle, i, modes[i], shortcuts[i]);
						}

						let index: i32 = shortcuts.indexOf(keyboard_key_code(ui.key));
						if (ui.is_key_pressed && index != -1) {
							modeHandle.position = index;
							ui.changed = true;
							Context.set_viewport_mode(modeHandle.position);
						}
						else if (modeHandle.changed) {
							Context.set_viewport_mode(modeHandle.position);
							ui.changed = true;
						}
					}, count);
				}
			}

			if (Operator.shortcut(Config.keymap.operator_search)) UIBase.operator_search();
		}

		if (Context.raw.brush_can_lock || Context.raw.brush_locked) {
			if (mouse_moved && Context.raw.brush_can_unlock) {
				Context.raw.brush_locked = false;
				Context.raw.brush_can_unlock = false;
			}

			///if (is_paint || is_sculpt)
			let b: bool = (Context.raw.brush_can_lock || Context.raw.brush_locked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_opacity, ShortcutType.ShortcutDown) &&
				!Operator.shortcut(Config.keymap.brush_angle, ShortcutType.ShortcutDown) &&
				!(decalMask && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.brush_radius, ShortcutType.ShortcutDown));
			///end
			///if is_lab
			let b: bool = (Context.raw.brush_can_lock || Context.raw.brush_locked) &&
				!Operator.shortcut(Config.keymap.brush_radius, ShortcutType.ShortcutDown);
			///end

			if (b) {
				mouse_unlock();
				Context.raw.last_paint_x = -1;
				Context.raw.last_paint_y = -1;
				if (Context.raw.brush_can_lock) {
					Context.raw.brush_can_lock = false;
					Context.raw.brush_can_unlock = false;
					Context.raw.brush_locked = false;
				}
				else {
					Context.raw.brush_can_unlock = true;
				}
			}
		}

		///if (is_paint || is_sculpt)
		if (UIBase.border_handle_ptr != 0) {
			if (UIBase.border_handle_ptr == UINodes.hwnd.ptr || UIBase.border_handle_ptr == UIView2D.hwnd.ptr) {
				if (UIBase.border_started == border_side_t.LEFT) {
					Config.raw.layout[layout_size_t.NODES_W] -= Math.floor(mouse_movement_x);
					if (Config.raw.layout[layout_size_t.NODES_W] < 32) Config.raw.layout[layout_size_t.NODES_W] = 32;
					else if (Config.raw.layout[layout_size_t.NODES_W] > sys_width() * 0.7) Config.raw.layout[layout_size_t.NODES_W] = Math.floor(sys_width() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[layout_size_t.NODES_H] -= Math.floor(mouse_movement_y);
					if (Config.raw.layout[layout_size_t.NODES_H] < 32) Config.raw.layout[layout_size_t.NODES_H] = 32;
					else if (Config.raw.layout[layout_size_t.NODES_H] > app_h() * 0.95) Config.raw.layout[layout_size_t.NODES_H] = Math.floor(app_h() * 0.95);
				}
			}
			else if (UIBase.border_handle_ptr == UIBase.hwnds[tab_area_t.STATUS].ptr) {
				let my: i32 = Math.floor(mouse_movement_y);
				if (Config.raw.layout[layout_size_t.STATUS_H] - my >= UIStatus.default_status_h * Config.raw.window_scale && Config.raw.layout[layout_size_t.STATUS_H] - my < sys_height() * 0.7) {
					Config.raw.layout[layout_size_t.STATUS_H] -= my;
				}
			}
			else {
				if (UIBase.border_started == border_side_t.LEFT) {
					Config.raw.layout[layout_size_t.SIDEBAR_W] -= Math.floor(mouse_movement_x);
					if (Config.raw.layout[layout_size_t.SIDEBAR_W] < UIBase.sidebar_mini_w) Config.raw.layout[layout_size_t.SIDEBAR_W] = UIBase.sidebar_mini_w;
					else if (Config.raw.layout[layout_size_t.SIDEBAR_W] > sys_width() - UIBase.sidebar_mini_w) Config.raw.layout[layout_size_t.SIDEBAR_W] = sys_width() - UIBase.sidebar_mini_w;
				}
				else {
					let my: i32 = Math.floor(mouse_movement_y);
					if (UIBase.border_handle_ptr == UIBase.hwnds[tab_area_t.SIDEBAR1].ptr && UIBase.border_started == border_side_t.TOP) {
						if (Config.raw.layout[layout_size_t.SIDEBAR_H0] + my > 32 && Config.raw.layout[layout_size_t.SIDEBAR_H1] - my > 32) {
							Config.raw.layout[layout_size_t.SIDEBAR_H0] += my;
							Config.raw.layout[layout_size_t.SIDEBAR_H1] -= my;
						}
					}
				}
			}
		}
		///end

		///if is_lab
		if (UIBase.border_handle_ptr != 0) {
			if (UIBase.border_handle_ptr == UINodes.hwnd.ptr || UIBase.border_handle_ptr == UIView2D.hwnd.ptr) {
				if (UIBase.border_started == border_side_t.LEFT) {
					Config.raw.layout[layout_size_t.NODES_W] -= Math.floor(mouse_movement_x);
					if (Config.raw.layout[layout_size_t.NODES_W] < 32) Config.raw.layout[layout_size_t.NODES_W] = 32;
					else if (Config.raw.layout[layout_size_t.NODES_W] > sys_width() * 0.7) Config.raw.layout[layout_size_t.NODES_W] = Math.floor(sys_width() * 0.7);
				}
				else { // UINodes / UIView2D ratio
					Config.raw.layout[layout_size_t.NODES_H] -= Math.floor(mouse_movement_y);
					if (Config.raw.layout[layout_size_t.NODES_H] < 32) Config.raw.layout[layout_size_t.NODES_H] = 32;
					else if (Config.raw.layout[layout_size_t.NODES_H] > app_h() * 0.95) Config.raw.layout[layout_size_t.NODES_H] = Math.floor(app_h() * 0.95);
				}
			}
			else if (UIBase.border_handle_ptr == UIBase.hwnds[tab_area_t.STATUS].ptr) {
				let my: i32 = Math.floor(mouse_movement_y);
				if (Config.raw.layout[layout_size_t.STATUS_H] - my >= UIStatus.default_status_h * Config.raw.window_scale && Config.raw.layout[layout_size_t.STATUS_H] - my < sys_height() * 0.7) {
					Config.raw.layout[layout_size_t.STATUS_H] -= my;
				}
			}
		}
		///end

		if (!mouse_down()) {
			UIBase.border_handle_ptr = 0;
			Base.is_resizing = false;
		}

		///if arm_physics
		if (Context.raw.tool == workspace_tool_t.PARTICLE && Context.raw.particle_physics && Context.in_paint_area() && !Context.raw.paint2d) {
			UtilParticle.init_particle_physics();
			let world: PhysicsWorldRaw = PhysicsWorld.active;
			PhysicsWorld.late_update(world);
			Context.raw.ddirty = 2;
			Context.raw.rdirty = 2;
			if (mouse_started()) {
				if (Context.raw.particle_timer != null) {
					tween_stop(Context.raw.particle_timer);
					Context.raw.particle_timer.done();
					Context.raw.particle_timer = null;
				}
				History.push_undo = true;
				Context.raw.particle_hit_x = Context.raw.particle_hit_y = Context.raw.particle_hit_z = 0;
				let o: object_t = scene_spawn_object(".Sphere");
				let md: material_data_t = data_get_material("Scene", ".Gizmo");
				let mo: mesh_object_t = o.ext;
				mo.base.name = ".Bullet";
				mo.materials[0] = md;
				mo.base.visible = true;

				let camera: camera_object_t = scene_camera;
				let ct: transform_t = camera.base.transform;
				vec4_set(mo.base.transform.loc, transform_world_x(ct), transform_world_y(ct), transform_world_z(ct));
				vec4_set(mo.base.transform.scale, Context.raw.brush_radius * 0.2, Context.raw.brush_radius * 0.2, Context.raw.brush_radius * 0.2);
				transform_build_matrix(mo.base.transform);

				let body: PhysicsBodyRaw = PhysicsBody.create();
				body.shape = shape_type_t.SPHERE;
				body.mass = 1.0;
				body.ccd = true;
				mo.base.transform.radius /= 10; // Lower ccd radius
				PhysicsBody.init(body, mo.base);
				(mo.base as any).physicsBody = body;
				mo.base.transform.radius *= 10;

				let ray: ray_t = raycast_get_ray(mouse_view_x(), mouse_view_y(), camera);
				PhysicsBody.apply_impulse(body, vec4_mult(ray.dir, 0.15));

				Context.raw.particle_timer = tween_timer(5, function() { mesh_object_remove(mo); });
			}

			let pairs: pair_t[] = PhysicsWorld.get_contact_pairs(world, Context.raw.paint_body);
			if (pairs != null) {
				for (let p of pairs) {
					Context.raw.last_particle_hit_x = Context.raw.particle_hit_x != 0 ? Context.raw.particle_hit_x : p.posA.x;
					Context.raw.last_particle_hit_y = Context.raw.particle_hit_y != 0 ? Context.raw.particle_hit_y : p.posA.y;
					Context.raw.last_particle_hit_z = Context.raw.particle_hit_z != 0 ? Context.raw.particle_hit_z : p.posA.z;
					Context.raw.particle_hit_x = p.posA.x;
					Context.raw.particle_hit_y = p.posA.y;
					Context.raw.particle_hit_z = p.posA.z;
					Context.raw.pdirty = 1;
					break; // 1 pair for now
				}
			}
		}
		///end
	}

	static view_top = () => {
		let isTyping: bool = UIBase.ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;

		if (Context.in_paint_area() && !isTyping) {
			if (mouse_view_x() < app_w()) {
				Viewport.set_view(0, 0, 1, 0, 0, 0);
			}
		}
	}

	static operator_search = () => {
		let searchHandle: zui_handle_t = zui_handle("uibase_1");
		let first: bool = true;
		UIMenu.draw((ui: zui_t) => {
			zui_fill(0, 0, ui._w / zui_SCALE(ui), ui.t.ELEMENT_H * 8, ui.t.SEPARATOR_COL);
			let search: string = zui_text_input(searchHandle, "", zui_align_t.LEFT, true, true);
			ui.changed = false;
			if (first) {
				first = false;
				searchHandle.text = "";
				zui_start_text_edit(searchHandle); // Focus search bar
			}

			if (searchHandle.changed) UIBase.operator_search_offset = 0;

			if (ui.is_key_pressed) { // Move selection
				if (ui.key == key_code_t.DOWN && UIBase.operator_search_offset < 6) UIBase.operator_search_offset++;
				if (ui.key == key_code_t.UP && UIBase.operator_search_offset > 0) UIBase.operator_search_offset--;
			}
			let enter: bool = keyboard_down("enter");
			let count: i32 = 0;
			let BUTTON_COL: i32 = ui.t.BUTTON_COL;

			for (let n in Config.keymap) {
				if (n.indexOf(search) >= 0) {
					ui.t.BUTTON_COL = count == UIBase.operator_search_offset ? ui.t.HIGHLIGHT_COL : ui.t.SEPARATOR_COL;
					if (zui_button(n, zui_align_t.LEFT, Config.keymap[n]) || (enter && count == UIBase.operator_search_offset)) {
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

	static toggle_distract_free = () => {
		UIBase.show = !UIBase.show;
		Base.resize();
	}

	static get_radius_increment = (): f32 => {
		return 0.1;
	}

	static hit_rect = (mx: f32, my: f32, x: i32, y: i32, w: i32, h: i32) => {
		return mx > x && mx < x + w && my > y && my < y + h;
	}

	///if (is_paint || is_sculpt)
	static get_brush_stencil_rect = (): rect_t => {
		let w: i32 = Math.floor(Context.raw.brush_stencil_image.width * (Base.h() / Context.raw.brush_stencil_image.height) * Context.raw.brush_stencil_scale);
		let h: i32 = Math.floor(Base.h() * Context.raw.brush_stencil_scale);
		let x: i32 = Math.floor(Base.x() + Context.raw.brush_stencil_x * Base.w());
		let y: i32 = Math.floor(Base.y() + Context.raw.brush_stencil_y * Base.h());
		return { w: w, h: h, x: x, y: y };
	}
	///end

	static update_ui = () => {
		if (Console.message_timer > 0) {
			Console.message_timer -= time_delta();
			if (Console.message_timer <= 0) UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
		}

		///if (is_paint || is_sculpt)
		UIBase.sidebar_mini_w = Math.floor(UIBase.default_sidebar_mini_w * zui_SCALE(UIBase.ui));
		///end

		if (!Base.ui_enabled) return;

		///if (is_paint || is_sculpt)
		// Same mapping for paint and rotate (predefined in touch keymap)
		if (Context.in_viewport()) {
			if (mouse_started() && Config.keymap.action_paint == Config.keymap.action_rotate) {
				UIBase.action_paint_remap = Config.keymap.action_paint;
				UtilRender.pick_pos_nor_tex();
				let isMesh: bool = Math.abs(Context.raw.posx_picked) < 50 && Math.abs(Context.raw.posy_picked) < 50 && Math.abs(Context.raw.posz_picked) < 50;
				///if krom_android
				// Allow rotating with both pen and touch, because hovering a pen prevents touch input on android
				let penOnly: bool = false;
				///else
				let penOnly: bool = Context.raw.pen_painting_only;
				///end
				let isPen: bool = penOnly && pen_down();
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

		if (Context.raw.brush_stencil_image != null && Operator.shortcut(Config.keymap.stencil_transform, ShortcutType.ShortcutDown)) {
			let r: rect_t = UIBase.get_brush_stencil_rect();
			if (mouse_started("left")) {
				Context.raw.brush_stencil_scaling =
					UIBase.hit_rect(mouse_x, mouse_y, r.x - 8,       r.y - 8,       16, 16) ||
					UIBase.hit_rect(mouse_x, mouse_y, r.x - 8,       r.h + r.y - 8, 16, 16) ||
					UIBase.hit_rect(mouse_x, mouse_y, r.w + r.x - 8, r.y - 8,       16, 16) ||
					UIBase.hit_rect(mouse_x, mouse_y, r.w + r.x - 8, r.h + r.y - 8, 16, 16);
				let cosa: f32 = Math.cos(-Context.raw.brush_stencil_angle);
				let sina: f32 = Math.sin(-Context.raw.brush_stencil_angle);
				let ox: f32 = 0;
				let oy: f32 = -r.h / 2;
				let x: f32 = ox * cosa - oy * sina;
				let y: f32 = ox * sina + oy * cosa;
				x += r.x + r.w / 2;
				y += r.y + r.h / 2;
				Context.raw.brush_stencil_rotating =
					UIBase.hit_rect(mouse_x, mouse_y, Math.floor(x - 16), Math.floor(y - 16), 32, 32);
			}
			let _scale: f32 = Context.raw.brush_stencil_scale;
			if (mouse_down("left")) {
				if (Context.raw.brush_stencil_scaling) {
					let mult: i32 = mouse_x > r.x + r.w / 2 ? 1 : -1;
					Context.raw.brush_stencil_scale += mouse_movement_x / 400 * mult;
				}
				else if (Context.raw.brush_stencil_rotating) {
					let gizmoX: f32 = r.x + r.w / 2;
					let gizmoY: f32 = r.y + r.h / 2;
					Context.raw.brush_stencil_angle = -Math.atan2(mouse_y - gizmoY, mouse_x - gizmoX) - Math.PI / 2;
				}
				else {
					Context.raw.brush_stencil_x += mouse_movement_x / Base.w();
					Context.raw.brush_stencil_y += mouse_movement_y / Base.h();
				}
			}
			else Context.raw.brush_stencil_scaling = false;
			if (mouse_wheel_delta != 0) {
				Context.raw.brush_stencil_scale -= mouse_wheel_delta / 10;
			}
			// Center after scale
			let ratio: f32 = Base.h() / Context.raw.brush_stencil_image.height;
			let oldW: f32 = _scale * Context.raw.brush_stencil_image.width * ratio;
			let newW: f32 = Context.raw.brush_stencil_scale * Context.raw.brush_stencil_image.width * ratio;
			let oldH: f32 = _scale * Base.h();
			let newH: f32 = Context.raw.brush_stencil_scale * Base.h();
			Context.raw.brush_stencil_x += (oldW - newW) / Base.w() / 2;
			Context.raw.brush_stencil_y += (oldH - newH) / Base.h() / 2;
		}
		///end

		let setCloneSource: bool = Context.raw.tool == workspace_tool_t.CLONE && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		///if (is_paint || is_sculpt)
		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		let decalMask: bool = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
		let down: bool = Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   		 decalMask ||
				   		 setCloneSource ||
				   		 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   		 (pen_down() && !keyboard_down("alt"));
		///end
		///if is_lab
		let down: bool = Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   		 setCloneSource ||
				   		 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
				   		 (pen_down() && !keyboard_down("alt"));
		///end

		if (Config.raw.touch_ui) {
			if (pen_down()) {
				Context.raw.pen_painting_only = true;
			}
			else if (Context.raw.pen_painting_only) {
				down = false;
			}
		}

		///if arm_physics
		if (Context.raw.tool == workspace_tool_t.PARTICLE && Context.raw.particle_physics) {
			down = false;
		}
		///end

		///if (is_paint || is_sculpt)
		///if krom_ios
		// No hover on iPad, decals are painted by pen release
		if (decal) {
			down = pen_released();
			if (!Context.raw.pen_painting_only) {
				down = down || mouse_released();
			}
		}
		///end
		///end

		if (down) {
			let mx: i32 = mouse_view_x();
			let my: i32 = mouse_view_y();
			let ww: i32 = app_w();

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
					Context.raw.clone_start_x = mx;
					Context.raw.clone_start_y = my;
				}
				else {
					if (Context.raw.brush_time == 0 &&
						!Base.is_dragging &&
						!Base.is_resizing &&
						!Base.is_combo_selected()) { // Paint started

						// Draw line
						if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown)) {
							Context.raw.last_paint_vec_x = Context.raw.last_paint_x;
							Context.raw.last_paint_vec_y = Context.raw.last_paint_y;
						}

						///if (is_paint || is_sculpt)
						History.push_undo = true;

						if (Context.raw.tool == workspace_tool_t.CLONE && Context.raw.clone_start_x >= 0.0) { // Clone delta
							Context.raw.clone_delta_x = (Context.raw.clone_start_x - mx) / ww;
							Context.raw.clone_delta_y = (Context.raw.clone_start_y - my) / app_h();
							Context.raw.clone_start_x = -1;
						}
						else if (Context.raw.tool == workspace_tool_t.PARTICLE) {
							// Reset particles
							///if arm_particles
							let emitter: mesh_object_t = scene_get_child(".ParticleEmitter").ext;
							let psys: particle_sys_t = emitter.particle_systems[0];
							psys.time = 0;
							// psys.time = psys.seed * psys.animtime;
							// psys.seed++;
							///end
						}
						else if (Context.raw.tool == workspace_tool_t.FILL && Context.raw.fill_type_handle.position == fill_type_t.UV_ISLAND) {
							UtilUV.uvislandmap_cached = false;
						}
						///end
					}

					Context.raw.brush_time += time_delta();

					///if (is_paint || is_sculpt)
					if (Context.raw.run_brush != null) {
						Context.raw.run_brush(0);
					}
					///end
					///if is_lab
					if (Context.run_brush != null) {
						Context.run_brush(0);
					}
					///end
				}
			}
		}
		else if (Context.raw.brush_time > 0) { // Brush released
			Context.raw.brush_time = 0;
			Context.raw.prev_paint_vec_x = -1;
			Context.raw.prev_paint_vec_y = -1;
			///if (!krom_direct3d12 && !krom_vulkan && !krom_metal) // Keep accumulated samples for D3D12
			Context.raw.ddirty = 3;
			///end
			Context.raw.brush_blend_dirty = true; // Update brush mask

			///if (is_paint || is_sculpt)
			Context.raw.layer_preview_dirty = true; // Update layer preview
			///end

			///if is_paint
			// New color id picked, update fill layer
			if (Context.raw.tool == workspace_tool_t.COLORID && Context.raw.layer.fill_layer != null) {
				Base.notify_on_next_frame(() => {
					Base.update_fill_layer();
					MakeMaterial.parse_paint_material(false);
				});
			}
			///end
		}

		///if is_paint
		if (Context.raw.layers_preview_dirty) {
			Context.raw.layers_preview_dirty = false;
			Context.raw.layer_preview_dirty = false;
			Context.raw.mask_preview_last = null;
			if (Base.pipe_merge == null) Base.make_pipe();
			// Update all layer previews
			for (let l of Project.layers) {
				if (SlotLayer.is_group(l)) continue;
				let target: image_t = l.texpaint_preview;
				let source: image_t = l.texpaint;
				g2_begin(target);
				g2_clear(0x00000000);
				// g2_set_pipeline(l.isMask() ? Base.pipeCopy8 : Base.pipeCopy);
				g2_set_pipeline(Base.pipe_copy); // texpaint_preview is always RGBA32 for now
				g2_draw_scaled_image(source, 0, 0, target.width, target.height);
				g2_set_pipeline(null);
				g2_end();
			}
			UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		}
		if (Context.raw.layer_preview_dirty && !SlotLayer.is_group(Context.raw.layer)) {
			Context.raw.layer_preview_dirty = false;
			Context.raw.mask_preview_last = null;
			if (Base.pipe_merge == null) Base.make_pipe();
			// Update layer preview
			let l: SlotLayerRaw = Context.raw.layer;
			let target: image_t = l.texpaint_preview;
			let source: image_t = l.texpaint;
			g2_begin(target);
			g2_clear(0x00000000);
			// g2_set_pipeline(Context.raw.layer.isMask() ? Base.pipeCopy8 : Base.pipeCopy);
			g2_set_pipeline(Base.pipe_copy); // texpaint_preview is always RGBA32 for now
			g2_draw_scaled_image(source, 0, 0, target.width, target.height);
			g2_set_pipeline(null);
			g2_end();
			UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		}
		///end

		let undoPressed: bool = Operator.shortcut(Config.keymap.edit_undo);
		let redoPressed: bool = Operator.shortcut(Config.keymap.edit_redo) ||
						  		(keyboard_down("control") && keyboard_started("y"));

		// Two-finger tap to undo, three-finger tap to redo
		if (Context.in_viewport() && Config.raw.touch_ui) {
			if (mouse_started("middle")) { UIBase.redo_tap_time = time_time(); }
			else if (mouse_started("right")) { UIBase.undo_tap_time = time_time(); }
			else if (mouse_released("middle") && time_time() - UIBase.redo_tap_time < 0.1) { UIBase.redo_tap_time = UIBase.undo_tap_time = 0; redoPressed = true; }
			else if (mouse_released("right") && time_time() - UIBase.undo_tap_time < 0.1) { UIBase.redo_tap_time = UIBase.undo_tap_time = 0; undoPressed = true; }
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
					UIBase.toggle_distract_free();
				}
			}
			zui_end();
			g2_begin(null);
		}

		if (!UIBase.show || sys_width() == 0 || sys_height() == 0) return;

		UIBase.ui.input_enabled = Base.ui_enabled;

		// Remember last tab positions
		for (let i: i32 = 0; i < UIBase.htabs.length; ++i) {
			if (UIBase.htabs[i].changed) {
				Config.raw.layout_tabs[i] = UIBase.htabs[i].position;
				Config.save();
			}
		}

		// Set tab positions
		for (let i: i32 = 0; i < UIBase.htabs.length; ++i) {
			UIBase.htabs[i].position = Config.raw.layout_tabs[i];
		}

		g2_end();
		zui_begin(UIBase.ui);

		///if (is_paint || is_sculpt)
		UIToolbar.render_ui();
		///end
		UIMenubar.render_ui();
		UIHeader.render_ui();
		UIStatus.render_ui();

		///if (is_paint || is_sculpt)
		UIBase.draw_sidebar();
		///end

		zui_end();
		g2_begin(null);
	}

	///if (is_paint || is_sculpt)
	static draw_sidebar = () => {
		// Tabs
		let mini: bool = Config.raw.layout[layout_size_t.SIDEBAR_W] <= UIBase.sidebar_mini_w;
		let expandButtonOffset: i32 = Config.raw.touch_ui ? Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui)) : 0;
		UIBase.tabx = sys_width() - Config.raw.layout[layout_size_t.SIDEBAR_W];

		let _SCROLL_W: i32 = UIBase.ui.t.SCROLL_W;
		if (mini) UIBase.ui.t.SCROLL_W = UIBase.ui.t.SCROLL_MINI_W;

		if (zui_window(UIBase.hwnds[tab_area_t.SIDEBAR0], UIBase.tabx, 0, Config.raw.layout[layout_size_t.SIDEBAR_W], Config.raw.layout[layout_size_t.SIDEBAR_H0])) {
			for (let i: i32 = 0; i < (mini ? 1 : UIBase.hwnd_tabs[tab_area_t.SIDEBAR0].length); ++i) UIBase.hwnd_tabs[tab_area_t.SIDEBAR0][i](UIBase.htabs[tab_area_t.SIDEBAR0]);
		}
		if (zui_window(UIBase.hwnds[tab_area_t.SIDEBAR1], UIBase.tabx, Config.raw.layout[layout_size_t.SIDEBAR_H0], Config.raw.layout[layout_size_t.SIDEBAR_W], Config.raw.layout[layout_size_t.SIDEBAR_H1] - expandButtonOffset)) {
			for (let i: i32 = 0; i < (mini ? 1 : UIBase.hwnd_tabs[tab_area_t.SIDEBAR1].length); ++i) UIBase.hwnd_tabs[tab_area_t.SIDEBAR1][i](UIBase.htabs[tab_area_t.SIDEBAR1]);
		}

		zui_end_window();
		UIBase.ui.t.SCROLL_W = _SCROLL_W;

		// Collapse / expand button for mini sidebar
		if (Config.raw.touch_ui) {
			let width: i32 = Config.raw.layout[layout_size_t.SIDEBAR_W];
			let height: i32 = Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui));
			if (zui_window(zui_handle("uibase_3"), sys_width() - width, sys_height() - height, width, height + 1)) {
				UIBase.ui._w = width;
				let _BUTTON_H: i32 = UIBase.ui.t.BUTTON_H;
				let _BUTTON_COL: i32 = UIBase.ui.t.BUTTON_COL;
				UIBase.ui.t.BUTTON_H = UIBase.ui.t.ELEMENT_H;
				UIBase.ui.t.BUTTON_COL = UIBase.ui.t.WINDOW_BG_COL;
				if (zui_button(mini ? "<<" : ">>")) {
					Config.raw.layout[layout_size_t.SIDEBAR_W] = mini ? UIBase.default_sidebar_full_w : UIBase.default_sidebar_mini_w;
					Config.raw.layout[layout_size_t.SIDEBAR_W] = Math.floor(Config.raw.layout[layout_size_t.SIDEBAR_W] * zui_SCALE(UIBase.ui));
				}
				UIBase.ui.t.BUTTON_H = _BUTTON_H;
				UIBase.ui.t.BUTTON_COL = _BUTTON_COL;
			}
		}

		// Expand button
		if (Config.raw.layout[layout_size_t.SIDEBAR_W] == 0) {
			let width: i32 = Math.floor(g2_font_width(UIBase.ui.font, UIBase.ui.font_size, "<<") + 25 * zui_SCALE(UIBase.ui));
			if (zui_window(UIBase.hminimized, sys_width() - width, 0, width, Math.floor(zui_ELEMENT_H(UIBase.ui) + zui_ELEMENT_OFFSET(UIBase.ui) + 1))) {
				UIBase.ui._w = width;
				let _BUTTON_H: i32 = UIBase.ui.t.BUTTON_H;
				let _BUTTON_COL: i32 = UIBase.ui.t.BUTTON_COL;
				UIBase.ui.t.BUTTON_H = UIBase.ui.t.ELEMENT_H;
				UIBase.ui.t.BUTTON_COL = UIBase.ui.t.SEPARATOR_COL;

				if (zui_button("<<")) {
					Config.raw.layout[layout_size_t.SIDEBAR_W] = Context.raw.maximized_sidebar_width != 0 ? Context.raw.maximized_sidebar_width : Math.floor(UIBase.default_sidebar_w * Config.raw.window_scale);
				}
				UIBase.ui.t.BUTTON_H = _BUTTON_H;
				UIBase.ui.t.BUTTON_COL = _BUTTON_COL;
			}
		}
		else if (UIBase.htabs[tab_area_t.SIDEBAR0].changed && UIBase.htabs[tab_area_t.SIDEBAR0].position == Context.raw.last_htab0_pos) {
			if (time_time() - Context.raw.select_time < 0.25) {
				Context.raw.maximized_sidebar_width = Config.raw.layout[layout_size_t.SIDEBAR_W];
				Config.raw.layout[layout_size_t.SIDEBAR_W] = 0;
			}
			Context.raw.select_time = time_time();
		}
		Context.raw.last_htab0_pos = UIBase.htabs[tab_area_t.SIDEBAR0].position;
	}

	static render_cursor = () => {
		if (!Base.ui_enabled) return;

		///if is_paint
		if (Context.raw.tool == workspace_tool_t.MATERIAL || Context.raw.tool == workspace_tool_t.BAKE) return;
		///end

		g2_set_color(0xffffffff);

		Context.raw.view_index = Context.raw.view_index_last;
		let mx: i32 = Base.x() + Context.raw.paint_vec.x * Base.w();
		let my: i32 = Base.y() + Context.raw.paint_vec.y * Base.h();
		Context.raw.view_index = -1;

		// Radius being scaled
		if (Context.raw.brush_locked) {
			mx += Context.raw.lock_started_x - sys_width() / 2;
			my += Context.raw.lock_started_y - sys_height() / 2;
		}

		let tool: workspace_tool_t = Context.raw.tool as workspace_tool_t;

		///if is_paint
		if (Context.raw.brush_stencil_image != null &&
			tool != workspace_tool_t.BAKE &&
			tool != workspace_tool_t.PICKER &&
			tool != workspace_tool_t.MATERIAL &&
			tool != workspace_tool_t.COLORID) {
			let r: rect_t = UIBase.get_brush_stencil_rect();
			if (!Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown)) {
				g2_set_color(0x88ffffff);
				let angle: f32 = Context.raw.brush_stencil_angle;
				let cx: f32 = r.x + r.w / 2;
				let cy: f32 = r.y + r.h / 2;
				g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(-angle)), mat3_translation(-cx, -cy)));
				g2_draw_scaled_image(Context.raw.brush_stencil_image, r.x, r.y, r.w, r.h);
				g2_set_transformation(null);
				g2_set_color(0xffffffff);
			}
			let transform: bool = Operator.shortcut(Config.keymap.stencil_transform, ShortcutType.ShortcutDown);
			if (transform) {
				// Outline
				g2_draw_rect(r.x, r.y, r.w, r.h);
				// Scale
				g2_draw_rect(r.x - 8,       r.y - 8,       16, 16);
				g2_draw_rect(r.x - 8 + r.w, r.y - 8,       16, 16);
				g2_draw_rect(r.x - 8,       r.y - 8 + r.h, 16, 16);
				g2_draw_rect(r.x - 8 + r.w, r.y - 8 + r.h, 16, 16);
				// Rotate
				let angle: f32 = Context.raw.brush_stencil_angle;
				let cx: f32 = r.x + r.w / 2;
				let cy: f32 = r.y + r.h / 2;
				g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(-angle)), mat3_translation(-cx, -cy)));
				g2_fill_circle(r.x + r.w / 2, r.y - 4, 8);
				g2_set_transformation(null);
			}
		}
		///end

		// Show picked material next to cursor
		if (Context.raw.tool == workspace_tool_t.PICKER && Context.raw.picker_select_material && Context.raw.color_picker_callback == null) {
			let img: image_t = Context.raw.material.image_icon;
			///if krom_opengl
			g2_draw_scaled_image(img, mx + 10, my + 10 + img.height, img.width, -img.height);
			///else
			g2_draw_image(img, mx + 10, my + 10);
			///end
		}
		if (Context.raw.tool == workspace_tool_t.PICKER && Context.raw.color_picker_callback != null) {
			let img: image_t = Res.get("icons.k");
			let rect: rect_t = Res.tile50(img, workspace_tool_t.PICKER, 0);
			g2_draw_sub_image(img, mx + 10, my + 10, rect.x, rect.y, rect.w, rect.h);
		}

		let cursorImg: image_t = Res.get("cursor.k");
		let psize: i32 = Math.floor(cursorImg.width * (Context.raw.brush_radius * Context.raw.brush_nodes_radius) * zui_SCALE(UIBase.ui));

		// Clone source cursor
		if (Context.raw.tool == workspace_tool_t.CLONE && !keyboard_down("alt") && (mouse_down() || pen_down())) {
			g2_set_color(0x66ffffff);
			g2_draw_scaled_image(cursorImg, mx + Context.raw.clone_delta_x * app_w() - psize / 2, my + Context.raw.clone_delta_y * app_h() - psize / 2, psize, psize);
			g2_set_color(0xffffffff);
		}

		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;

		if (!Config.raw.brush_3d || Context.in_2d_view() || decal) {
			let decalMask: bool = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
			if (decal && !Context.in_nodes()) {
				let decalAlpha: f32 = 0.5;
				if (!decalMask) {
					Context.raw.decal_x = Context.raw.paint_vec.x;
					Context.raw.decal_y = Context.raw.paint_vec.y;
					decalAlpha = Context.raw.brush_opacity;

					// Radius being scaled
					if (Context.raw.brush_locked) {
						Context.raw.decal_x += (Context.raw.lock_started_x - sys_width() / 2) / Base.w();
						Context.raw.decal_y += (Context.raw.lock_started_y - sys_height() / 2) / Base.h();
					}
				}

				if (!Config.raw.brush_live) {
					let psizex: i32 = Math.floor(256 * zui_SCALE(UIBase.ui) * (Context.raw.brush_radius * Context.raw.brush_nodes_radius * Context.raw.brush_scale_x));
					let psizey: i32 = Math.floor(256 * zui_SCALE(UIBase.ui) * (Context.raw.brush_radius * Context.raw.brush_nodes_radius));

					Context.raw.view_index = Context.raw.view_index_last;
					let decalX: f32 = Base.x() + Context.raw.decal_x * Base.w() - psizex / 2;
					let decalY: f32 = Base.y() + Context.raw.decal_y * Base.h() - psizey / 2;
					Context.raw.view_index = -1;

					g2_set_color(color_from_floats(1, 1, 1, decalAlpha));
					let angle: f32 = (Context.raw.brush_angle + Context.raw.brush_nodes_angle) * (Math.PI / 180);
					let cx: f32 = decalX + psizex / 2;
					let cy: f32 = decalY + psizey / 2;
					g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(angle)), mat3_translation(-cx, -cy)));
					///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
					g2_draw_scaled_image(Context.raw.decal_image, decalX, decalY, psizex, psizey);
					///else
					g2_draw_scaled_image(Context.raw.decal_image, decalX, decalY + psizey, psizex, -psizey);
					///end
					g2_set_transformation(null);
					g2_set_color(0xffffffff);
				}
			}
			if (Context.raw.tool == workspace_tool_t.BRUSH  ||
				Context.raw.tool == workspace_tool_t.ERASER ||
				Context.raw.tool == workspace_tool_t.CLONE  ||
				Context.raw.tool == workspace_tool_t.BLUR   ||
				Context.raw.tool == workspace_tool_t.SMUDGE   ||
				Context.raw.tool == workspace_tool_t.PARTICLE ||
				(decalMask && !Config.raw.brush_3d) ||
				(decalMask && Context.in_2d_view())) {
				if (decalMask) {
					psize = Math.floor(cursorImg.width * (Context.raw.brush_decal_mask_radius * Context.raw.brush_nodes_radius) * zui_SCALE(UIBase.ui));
				}
				if (Config.raw.brush_3d && Context.in_2d_view()) {
					psize = Math.floor(psize * UIView2D.pan_scale);
				}
				g2_draw_scaled_image(cursorImg, mx - psize / 2, my - psize / 2, psize, psize);
			}
		}

		if (Context.raw.brush_lazy_radius > 0 && !Context.raw.brush_locked &&
			(Context.raw.tool == workspace_tool_t.BRUSH ||
			 Context.raw.tool == workspace_tool_t.ERASER ||
			 Context.raw.tool == workspace_tool_t.DECAL ||
			 Context.raw.tool == workspace_tool_t.TEXT ||
			 Context.raw.tool == workspace_tool_t.CLONE ||
			 Context.raw.tool == workspace_tool_t.BLUR ||
			 Context.raw.tool == workspace_tool_t.SMUDGE ||
			 Context.raw.tool == workspace_tool_t.PARTICLE)) {
			g2_fill_rect(mx - 1, my - 1, 2, 2);
			mx = Context.raw.brush_lazy_x * Base.w() + Base.x();
			my = Context.raw.brush_lazy_y * Base.h() + Base.y();
			let radius: f32 = Context.raw.brush_lazy_radius * 180;
			g2_set_color(0xff666666);
			g2_draw_scaled_image(cursorImg, mx - radius / 2, my - radius / 2, radius, radius);
			g2_set_color(0xffffffff);
		}
	}
	///end

	static show_material_nodes = () => {
		// Clear input state as ui receives input events even when not drawn
		zui_end_input();

		///if (is_paint || is_sculpt)
		UINodes.show = !UINodes.show || UINodes.canvas_type != canvas_type_t.MATERIAL;
		UINodes.canvas_type = canvas_type_t.MATERIAL;
		///end
		///if is_lab
		UINodes.show = !UINodes.show;
		///end

		Base.resize();
	}

	///if (is_paint || is_sculpt)
	static show_brush_nodes = () => {
		// Clear input state as ui receives input events even when not drawn
		zui_end_input();
		UINodes.show = !UINodes.show || UINodes.canvas_type != canvas_type_t.BRUSH;
		UINodes.canvas_type = canvas_type_t.BRUSH;
		Base.resize();
	}
	///end

	static show_2d_view = (type: view_2d_type_t) => {
		// Clear input state as ui receives input events even when not drawn
		zui_end_input();
		if (UIView2D.type != type) UIView2D.show = true;
		else UIView2D.show = !UIView2D.show;
		UIView2D.type = type;
		UIView2D.hwnd.redraws = 2;
		Base.resize();
	}

	static toggle_browser = () => {
		let minimized: bool = Config.raw.layout[layout_size_t.STATUS_H] <= (UIStatus.default_status_h * Config.raw.window_scale);
		Config.raw.layout[layout_size_t.STATUS_H] = minimized ? 240 : UIStatus.default_status_h;
		Config.raw.layout[layout_size_t.STATUS_H] = Math.floor(Config.raw.layout[layout_size_t.STATUS_H] * Config.raw.window_scale);
	}

	static set_icon_scale = () => {
		if (zui_SCALE(UIBase.ui) > 1) {
			Res.load(["icons2x.k"], () => {
				Res.bundled.set("icons.k", Res.get("icons2x.k"));
			});
		}
		else {
			Res.load(["icons.k"], () => {});
		}
	}

	static on_border_hover = (handle_ptr: i32, side: i32) => {
		if (!Base.ui_enabled) return;

		///if (is_paint || is_sculpt)
		if (handle_ptr != UIBase.hwnds[tab_area_t.SIDEBAR0].ptr &&
			handle_ptr != UIBase.hwnds[tab_area_t.SIDEBAR1].ptr &&
			handle_ptr != UIBase.hwnds[tab_area_t.STATUS].ptr &&
			handle_ptr != UINodes.hwnd.ptr &&
			handle_ptr != UIView2D.hwnd.ptr) return; // Scalable handles
		if (handle_ptr == UIView2D.hwnd.ptr && side != border_side_t.LEFT) return;
		if (handle_ptr == UINodes.hwnd.ptr && side == border_side_t.TOP && !UIView2D.show) return;
		if (handle_ptr == UIBase.hwnds[tab_area_t.SIDEBAR0].ptr && side == border_side_t.TOP) return;
		///end

		///if is_lab
		if (handle_ptr != UIBase.hwnds[tab_area_t.STATUS].ptr &&
			handle_ptr != UINodes.hwnd.ptr &&
			handle_ptr != UIView2D.hwnd.ptr) return; // Scalable handles
		if (handle_ptr == UIView2D.hwnd.ptr && side != border_side_t.LEFT) return;
		if (handle_ptr == UINodes.hwnd.ptr && side == border_side_t.TOP && !UIView2D.show) return;
		///end

		if (handle_ptr == UINodes.hwnd.ptr && side != border_side_t.LEFT && side != border_side_t.TOP) return;
		if (handle_ptr == UIBase.hwnds[tab_area_t.STATUS].ptr && side != border_side_t.TOP) return;
		if (side == border_side_t.RIGHT) return; // UI is snapped to the right side

		side == border_side_t.LEFT || side == border_side_t.RIGHT ?
			krom_set_mouse_cursor(3) : // Horizontal
			krom_set_mouse_cursor(4);  // Vertical

		if (zui_current.input_started) {
			UIBase.border_started = side;
			UIBase.border_handle_ptr = handle_ptr;
			Base.is_resizing = true;
		}
	}

	static on_text_hover = () => {
		krom_set_mouse_cursor(2); // I-cursor
	}

	static on_deselect_text = () => {
		///if krom_ios
		keyboard_up_listener(key_code_t.SHIFT);
		///end
	}

	static on_tab_drop = (to_ptr: i32, toPosition: i32, from_ptr: i32, fromPosition: i32) => {
		let i: i32 = -1;
		let j: i32 = -1;
		for (let k: i32 = 0; k < UIBase.htabs.length; ++k) {
			if (UIBase.htabs[k].ptr == to_ptr) i = k;
			if (UIBase.htabs[k].ptr == from_ptr) j = k;
		}
		if (i > -1 && j > -1) {
			let element: any = UIBase.hwnd_tabs[j][fromPosition];
			UIBase.hwnd_tabs[j].splice(fromPosition, 1);
			UIBase.hwnd_tabs[i].splice(toPosition, 0, element);
			UIBase.hwnds[i].redraws = 2;
			UIBase.hwnds[j].redraws = 2;
		}
	}

	static tag_ui_redraw = () => {
		UIHeader.header_handle.redraws = 2;
		UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
		UIMenubar.workspace_handle.redraws = 2;
		UIMenubar.menu_handle.redraws = 2;
		///if (is_paint || is_sculpt)
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		UIToolbar.toolbar_handle.redraws = 2;
		///end
	}
}
