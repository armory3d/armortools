
type tab_draw_t = {
	f: (h: ui_handle_t)=>void;
};
type tab_draw_array_t = tab_draw_t[];

let ui_base_show: bool = true;
let ui_base_ui: ui_t;
let ui_base_border_started: i32 = 0;
let ui_base_border_handle: ui_handle_t = null;
let ui_base_action_paint_remap: string = "";
let ui_base_operator_search_offset: i32 = 0;
let ui_base_undo_tap_time: f32 = 0.0;
let ui_base_redo_tap_time: f32 = 0.0;
let ui_base_hwnds: ui_handle_t[] = ui_base_init_hwnds();
let ui_base_htabs: ui_handle_t[] = ui_base_init_htabs();
let ui_base_hwnd_tabs: tab_draw_array_t[] = ui_base_init_hwnd_tabs();
let ui_base_viewport_col: i32;

///if is_lab
let ui_base_default_sidebar_mini_w: i32 = 0;
let ui_base_default_sidebar_full_w: i32 = 0;
///else
let ui_base_default_sidebar_mini_w: i32 = 56;
let ui_base_default_sidebar_full_w: i32 = 280;
///end

///if (arm_android || arm_ios)
let ui_base_default_sidebar_w: i32 = ui_base_default_sidebar_mini_w;
///else
let ui_base_default_sidebar_w: i32 = ui_base_default_sidebar_full_w;
///end

let ui_base_tabx: i32 = 0;
let ui_base_hminimized: ui_handle_t = ui_handle_create();
let ui_base_sidebar_mini_w: i32 = ui_base_default_sidebar_mini_w;
let _ui_base_operator_search_first: bool;

function ui_base_init_hwnds(): ui_handle_t[] {
	let hwnds: ui_handle_t[] = [ui_handle_create(), ui_handle_create(), ui_handle_create()];
	return hwnds;
}

function ui_base_init_htabs(): ui_handle_t[] {
	let htabs: ui_handle_t[] = [ui_handle_create(), ui_handle_create(), ui_handle_create()];
	return htabs;
}

function _draw_callback_create(f: (h: ui_handle_t)=>void): tab_draw_t {
	let cb: tab_draw_t = { f: f };
	return cb;
}

function ui_base_init_hwnd_tabs(): tab_draw_array_t[] {
	return ui_base_ext_init_hwnd_tabs();
}

function ui_base_init() {
	///if (is_paint || is_sculpt)
	ui_toolbar_init();
	ui_toolbar_w = math_floor(ui_toolbar_default_w * config_raw.window_scale);
	context_raw.text_tool_text = tr("Text");
	///end

	ui_header_init();
	ui_status_init();
	ui_menubar_init();

	ui_header_h = math_floor(ui_header_default_h * config_raw.window_scale);
	ui_menubar_w = math_floor(ui_menubar_default_w * config_raw.window_scale);

	///if (is_paint || is_sculpt)
	if (project_materials == null) {
		project_materials = [];
		let m: material_data_t = data_get_material("Scene", "Material");
		array_push(project_materials, slot_material_create(m));
		context_raw.material = project_materials[0];
	}

	if (project_brushes == null) {
		project_brushes = [];
		array_push(project_brushes, slot_brush_create());
		context_raw.brush = project_brushes[0];
		make_material_parse_brush();
	}

	if (project_fonts == null) {
		project_fonts = [];
		array_push(project_fonts, slot_font_create("default.ttf", base_font));
		context_raw.font = project_fonts[0];
	}

	if (project_layers == null) {
		project_layers = [];
		array_push(project_layers, slot_layer_create());
		context_raw.layer = project_layers[0];
	}
	///end

	///if is_lab
	if (project_material_data == null) {
		let m: material_data_t = data_get_material("Scene", "Material");
		project_material_data = m;
	}

	if (project_default_canvas == null) { // Synchronous
		let b: buffer_t = data_get_blob("default_brush.arm");
		project_default_canvas = b;
	}

	project_nodes = ui_nodes_create();
	project_canvas = armpack_decode(project_default_canvas);
	project_canvas.name = "Brush 1";

	brush_output_node_parse_inputs();

	parser_logic_parse(project_canvas);
	///end

	if (project_raw.swatches == null) {
		project_set_default_swatches();
		context_raw.swatch = project_raw.swatches[0];
	}

	if (context_raw.empty_envmap == null) {
		ui_base_make_empty_envmap(base_theme.VIEWPORT_COL);
	}
	if (context_raw.preview_envmap == null) {
		let b: u8_array_t = u8_array_create(4);
		b[0] = 0;
		b[1] = 0;
		b[2] = 0;
		b[3] = 255;
		context_raw.preview_envmap = image_from_bytes(b, 1, 1);
	}

	let world: world_data_t = scene_world;
	if (context_raw.saved_envmap == null) {
		// raw.saved_envmap = world._envmap;
		context_raw.default_irradiance = world._.irradiance;
		context_raw.default_radiance = world._.radiance;
		context_raw.default_radiance_mipmaps = world._.radiance_mipmaps;
	}
	world._.envmap = context_raw.show_envmap ? context_raw.saved_envmap : context_raw.empty_envmap;
	context_raw.ddirty = 1;

	history_reset();

	let scale: f32 = config_raw.window_scale;
	let ops: ui_options_t = {
		theme: base_theme,
		font: base_font,
		scale_factor: scale,
		color_wheel: base_color_wheel.texture_,
		black_white_gradient: base_color_wheel_gradient.texture_
	};
	ui_base_ui = ui_create(ops);
	ui_on_border_hover = ui_base_on_border_hover;
	ui_on_text_hover = ui_base_on_text_hover;
	ui_on_deselect_text = ui_base_on_deselect_text;
	ui_on_tab_drop = ui_base_on_tab_drop;

	///if (is_paint || is_sculpt)
	let resources: string[] = ["cursor.k", "icons.k"];
	///end
	///if is_lab
	let resources: string[] = ["cursor.k", "icons.k", "placeholder.k"];
	///end

	///if (is_paint || is_sculpt)
	context_raw.gizmo = scene_get_child(".Gizmo");
	context_raw.gizmo_translate_x = object_get_child(context_raw.gizmo, ".TranslateX");
	context_raw.gizmo_translate_y = object_get_child(context_raw.gizmo, ".TranslateY");
	context_raw.gizmo_translate_z = object_get_child(context_raw.gizmo, ".TranslateZ");
	context_raw.gizmo_scale_x = object_get_child(context_raw.gizmo, ".ScaleX");
	context_raw.gizmo_scale_y = object_get_child(context_raw.gizmo, ".ScaleY");
	context_raw.gizmo_scale_z = object_get_child(context_raw.gizmo, ".ScaleZ");
	context_raw.gizmo_rotate_x = object_get_child(context_raw.gizmo, ".RotateX");
	context_raw.gizmo_rotate_y = object_get_child(context_raw.gizmo, ".RotateY");
	context_raw.gizmo_rotate_z = object_get_child(context_raw.gizmo, ".RotateZ");
	///end

	resource_load(resources);

	if (ui_SCALE(ui_base_ui) > 1) {
		ui_base_set_icon_scale();
	}

	context_raw.paint_object = scene_get_child(".Cube").ext;
	project_paint_objects = [context_raw.paint_object];

	if (project_filepath == "") {
		app_notify_on_init(layers_init);
	}

	context_raw.project_objects = [];
	for (let i: i32 = 0; i < scene_meshes.length; ++i) {
		let m: mesh_object_t = scene_meshes[i];
		array_push(context_raw.project_objects, m);
	}

	operator_register("view_top", ui_base_view_top);
}

function ui_base_update() {
	ui_base_update_ui();
	operator_update();

	let keys: string[] = map_keys(plugin_map);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let p: plugin_t = map_get(plugin_map, keys[i]);
		if (p.on_update != null) {
			js_call(p.on_update);
		}
	}

	if (!base_ui_enabled) {
		return;
	}

	if (!ui_nodes_ui.is_typing && !ui_base_ui.is_typing) {
		if (operator_shortcut(map_get(config_keymap, "toggle_node_editor"))) {
			///if (is_paint || is_sculpt)
			ui_nodes_canvas_type == canvas_type_t.MATERIAL ? ui_base_show_material_nodes() : ui_base_show_brush_nodes();
			///end
			///if is_lab
			ui_base_show_material_nodes();
			///end
		}
		else if (operator_shortcut(map_get(config_keymap, "toggle_browser"))) {
			ui_base_toggle_browser();
		}

		else if (operator_shortcut(map_get(config_keymap, "toggle_2d_view"))) {
			///if (is_paint || is_sculpt)
			ui_base_show_2d_view(view_2d_type_t.LAYER);
			///else
			ui_base_show_2d_view(view_2d_type_t.ASSET);
			///end
		}
	}

	if (operator_shortcut(map_get(config_keymap, "file_save_as"))) {
		project_save_as();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_save"))) {
		project_save();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_open"))) {
		project_open();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_open_recent"))) {
		box_projects_show();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_reimport_mesh"))) {
		project_reimport_mesh();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_reimport_textures"))) {
		project_reimport_textures();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_new"))) {
		project_new_box();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_export_textures"))) {
		if (context_raw.texture_export_path == "") { // First export, ask for path
			context_raw.layers_export = export_mode_t.VISIBLE;
			box_export_show_textures();
		}
		else {
			app_notify_on_init(function () {
				export_texture_run(context_raw.texture_export_path);
			});
		}
	}
	else if (operator_shortcut(map_get(config_keymap, "file_export_textures_as"))) {
		context_raw.layers_export = export_mode_t.VISIBLE;
		box_export_show_textures();
	}
	else if (operator_shortcut(map_get(config_keymap, "file_import_assets"))) {
		project_import_asset();
	}
	else if (operator_shortcut(map_get(config_keymap, "edit_prefs"))) {
		box_preferences_show();
	}

	if (keyboard_started(map_get(config_keymap, "view_distract_free")) || (keyboard_started("escape") && !ui_base_show && !ui_box_show)) {
		ui_base_toggle_distract_free();
	}

	///if arm_linux
	if (operator_shortcut("alt+enter", shortcut_type_t.STARTED)) {
		base_toggle_fullscreen();
	}
	///end

	///if (is_paint || is_sculpt)
	let decal: bool = context_is_decal();
	let decal_mask: bool = context_is_decal_mask();

	if ((context_raw.brush_can_lock || context_raw.brush_locked) && mouse_moved) {
		if (operator_shortcut(map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN) ||
			operator_shortcut(map_get(config_keymap, "brush_opacity"), shortcut_type_t.DOWN) ||
			operator_shortcut(map_get(config_keymap, "brush_angle"), shortcut_type_t.DOWN) ||
			(decal_mask && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN))) {
			if (context_raw.brush_locked) {
				if (operator_shortcut(map_get(config_keymap, "brush_opacity"), shortcut_type_t.DOWN)) {
					context_raw.brush_opacity += mouse_movement_x / 500;
					context_raw.brush_opacity = math_max(0.0, math_min(1.0, context_raw.brush_opacity));
					context_raw.brush_opacity = math_round(context_raw.brush_opacity * 100) / 100;
					context_raw.brush_opacity_handle.value = context_raw.brush_opacity;
				}
				else if (operator_shortcut(map_get(config_keymap, "brush_angle"), shortcut_type_t.DOWN)) {
					context_raw.brush_angle += mouse_movement_x / 5;
					let i: i32 = math_floor(context_raw.brush_angle);
					context_raw.brush_angle = i % 360;
					if (context_raw.brush_angle < 0) context_raw.brush_angle += 360;
					context_raw.brush_angle_handle.value = context_raw.brush_angle;
					make_material_parse_paint_material();
				}
				else if (decal_mask && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN)) {
					context_raw.brush_decal_mask_radius += mouse_movement_x / 150;
					context_raw.brush_decal_mask_radius = math_max(0.01, math_min(4.0, context_raw.brush_decal_mask_radius));
					context_raw.brush_decal_mask_radius = math_round(context_raw.brush_decal_mask_radius * 100) / 100;
					context_raw.brush_decal_mask_radius_handle.value = context_raw.brush_decal_mask_radius;
				}
				else {
					context_raw.brush_radius += mouse_movement_x / 150;
					context_raw.brush_radius = math_max(0.01, math_min(4.0, context_raw.brush_radius));
					context_raw.brush_radius = math_round(context_raw.brush_radius * 100) / 100;
					context_raw.brush_radius_handle.value = context_raw.brush_radius;
				}
				ui_header_handle.redraws = 2;
			}
			else if (context_raw.brush_can_lock) {
				context_raw.brush_can_lock = false;
				context_raw.brush_locked = true;
			}
		}
	}
	///end

	///if is_lab
	if ((context_raw.brush_can_lock || context_raw.brush_locked) && mouse_moved) {
		if (operator_shortcut(map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN)) {
			if (context_raw.brush_locked) {
				context_raw.brush_radius += mouse_movement_x / 150;
				context_raw.brush_radius = math_max(0.01, math_min(4.0, context_raw.brush_radius));
				context_raw.brush_radius = math_round(context_raw.brush_radius * 100) / 100;
				context_raw.brush_radius_handle.value = context_raw.brush_radius;
				ui_header_handle.redraws = 2;
			}
			else if (context_raw.brush_can_lock) {
				context_raw.brush_can_lock = false;
				context_raw.brush_locked = true;
			}
		}
	}
	///end

	let is_typing: bool = ui_base_ui.is_typing || ui_view2d_ui.is_typing || ui_nodes_ui.is_typing;

	///if (is_paint || is_sculpt)
	if (!is_typing) {
		if (operator_shortcut(map_get(config_keymap, "select_material"), shortcut_type_t.DOWN)) {
			ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			for (let i: i32 = 1; i < 10; ++i) {
				if (keyboard_started(i + "")) {
					context_select_material(i - 1);
				}
			}
		}
		else if (operator_shortcut(map_get(config_keymap, "select_layer"), shortcut_type_t.DOWN)) {
			ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
			for (let i: i32 = 1; i < 10; ++i) {
				if (keyboard_started(i + "")) {
					context_select_layer(i - 1);
				}
			}
		}
	}
	///end

	// Viewport shortcuts
	if (context_in_paint_area() && !is_typing) {

		///if is_paint
		if (!mouse_down("right")) { // Fly mode off
			if (operator_shortcut(map_get(config_keymap, "tool_brush"))) {
				context_select_tool(workspace_tool_t.BRUSH);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_eraser"))) {
				context_select_tool(workspace_tool_t.ERASER);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_fill"))) {
				context_select_tool(workspace_tool_t.FILL);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_colorid"))) {
				context_select_tool(workspace_tool_t.COLORID);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_decal"))) {
				context_select_tool(workspace_tool_t.DECAL);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_text"))) {
				context_select_tool(workspace_tool_t.TEXT);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_clone"))) {
				context_select_tool(workspace_tool_t.CLONE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_blur"))) {
				context_select_tool(workspace_tool_t.BLUR);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_smudge"))) {
				context_select_tool(workspace_tool_t.SMUDGE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_particle"))) {
				context_select_tool(workspace_tool_t.PARTICLE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_picker"))) {
				context_select_tool(workspace_tool_t.PICKER);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_bake"))) {
				context_select_tool(workspace_tool_t.BAKE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_gizmo"))) {
				context_select_tool(workspace_tool_t.GIZMO);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_material"))) {
				context_select_tool(workspace_tool_t.MATERIAL);
			}
			else if (operator_shortcut(map_get(config_keymap, "swap_brush_eraser"))) {
				context_select_tool(context_raw.tool == workspace_tool_t.BRUSH ? workspace_tool_t.ERASER : workspace_tool_t.BRUSH);
			}
		}

		// Radius
		if (context_raw.tool == workspace_tool_t.BRUSH  ||
			context_raw.tool == workspace_tool_t.ERASER ||
			context_raw.tool == workspace_tool_t.DECAL  ||
			context_raw.tool == workspace_tool_t.TEXT   ||
			context_raw.tool == workspace_tool_t.CLONE  ||
			context_raw.tool == workspace_tool_t.BLUR   ||
			context_raw.tool == workspace_tool_t.SMUDGE   ||
			context_raw.tool == workspace_tool_t.PARTICLE) {
			if (operator_shortcut(map_get(config_keymap, "brush_radius")) ||
				operator_shortcut(map_get(config_keymap, "brush_opacity")) ||
				operator_shortcut(map_get(config_keymap, "brush_angle")) ||
				(decal_mask && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius")))) {
				context_raw.brush_can_lock = true;
				if (!pen_connected) {
					mouse_lock();
				}
				context_raw.lock_started_x = mouse_x;
				context_raw.lock_started_y = mouse_y;
			}
			else if (operator_shortcut(map_get(config_keymap, "brush_radius_decrease"), shortcut_type_t.REPEAT)) {
				context_raw.brush_radius -= ui_base_get_radius_increment();
				context_raw.brush_radius = math_max(math_round(context_raw.brush_radius * 100) / 100, 0.01);
				context_raw.brush_radius_handle.value = context_raw.brush_radius;
				ui_header_handle.redraws = 2;
			}
			else if (operator_shortcut(map_get(config_keymap, "brush_radius_increase"), shortcut_type_t.REPEAT)) {
				context_raw.brush_radius += ui_base_get_radius_increment();
				context_raw.brush_radius = math_round(context_raw.brush_radius * 100) / 100;
				context_raw.brush_radius_handle.value = context_raw.brush_radius;
				ui_header_handle.redraws = 2;
			}
			else if (decal_mask) {
				if (operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius_decrease"), shortcut_type_t.REPEAT)) {
					context_raw.brush_decal_mask_radius -= ui_base_get_radius_increment();
					context_raw.brush_decal_mask_radius = math_max(math_round(context_raw.brush_decal_mask_radius * 100) / 100, 0.01);
					context_raw.brush_decal_mask_radius_handle.value = context_raw.brush_decal_mask_radius;
					ui_header_handle.redraws = 2;
				}
				else if (operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius_increase"), shortcut_type_t.REPEAT)) {
					context_raw.brush_decal_mask_radius += ui_base_get_radius_increment();
					context_raw.brush_decal_mask_radius = math_round(context_raw.brush_decal_mask_radius * 100) / 100;
					context_raw.brush_decal_mask_radius_handle.value = context_raw.brush_decal_mask_radius;
					ui_header_handle.redraws = 2;
				}
			}
		}

		if (decal_mask && (operator_shortcut(map_get(config_keymap, "decal_mask"), shortcut_type_t.STARTED) || operator_shortcut(map_get(config_keymap, "decal_mask"), shortcut_type_t.RELEASED))) {
			ui_header_handle.redraws = 2;
		}
		///end

		///if is_lab
		if (ui_header_worktab.position == space_type_t.SPACE3D) {
			// Radius
			if (context_raw.tool == workspace_tool_t.ERASER ||
				context_raw.tool == workspace_tool_t.CLONE  ||
				context_raw.tool == workspace_tool_t.BLUR   ||
				context_raw.tool == workspace_tool_t.SMUDGE) {
				if (operator_shortcut(map_get(config_keymap, "brush_radius"))) {
					context_raw.brush_can_lock = true;
					if (!pen_connected) {
						mouse_lock();
					}
					context_raw.lock_started_x = mouse_x;
					context_raw.lock_started_y = mouse_y;
				}
				else if (operator_shortcut(map_get(config_keymap, "brush_radius_decrease"), shortcut_type_t.REPEAT)) {
					context_raw.brush_radius -= ui_base_get_radius_increment();
					context_raw.brush_radius = math_max(math_round(context_raw.brush_radius * 100) / 100, 0.01);
					context_raw.brush_radius_handle.value = context_raw.brush_radius;
					ui_header_handle.redraws = 2;
				}
				else if (operator_shortcut(map_get(config_keymap, "brush_radius_increase"), shortcut_type_t.REPEAT)) {
					context_raw.brush_radius += ui_base_get_radius_increment();
					context_raw.brush_radius = math_round(context_raw.brush_radius * 100) / 100;
					context_raw.brush_radius_handle.value = context_raw.brush_radius;
					ui_header_handle.redraws = 2;
				}
			}
		}
		///end

		// Viewpoint
		if (mouse_view_x() < app_w()) {
			if (operator_shortcut(map_get(config_keymap, "view_reset"))) {
				viewport_reset();
				viewport_scale_to_bounds();
			}
			else if (operator_shortcut(map_get(config_keymap, "view_back"))) {
				viewport_set_view(0, 1, 0, math_pi() / 2, 0, math_pi());
			}
			else if (operator_shortcut(map_get(config_keymap, "view_front"))) {
				viewport_set_view(0, -1, 0, math_pi() / 2, 0, 0);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_left"))) {
				viewport_set_view(-1, 0, 0, math_pi() / 2, 0, -math_pi() / 2);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_right"))) {
				viewport_set_view(1, 0, 0, math_pi() / 2, 0, math_pi() / 2);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_bottom"))) {
				viewport_set_view(0, 0, -1, math_pi(), 0, math_pi());
			}
			else if (operator_shortcut(map_get(config_keymap, "view_camera_type"))) {
				context_raw.camera_type = context_raw.camera_type == camera_type_t.PERSPECTIVE ? camera_type_t.ORTHOGRAPHIC : camera_type_t.PERSPECTIVE;
				context_raw.cam_handle.position = context_raw.camera_type;
				viewport_update_camera_type(context_raw.camera_type);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_orbit_left"), shortcut_type_t.REPEAT)) {
				viewport_orbit(-math_pi() / 12, 0);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_orbit_right"), shortcut_type_t.REPEAT)) {
				viewport_orbit(math_pi() / 12, 0);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_orbit_up"), shortcut_type_t.REPEAT)) {
				viewport_orbit(0, -math_pi() / 12);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_orbit_down"), shortcut_type_t.REPEAT)) {
				viewport_orbit(0, math_pi() / 12);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_orbit_opposite"))) {
				viewport_orbit_opposite();
			}
			else if (operator_shortcut(map_get(config_keymap, "view_zoom_in"), shortcut_type_t.REPEAT)) {
				viewport_zoom(0.2);
			}
			else if (operator_shortcut(map_get(config_keymap, "view_zoom_out"), shortcut_type_t.REPEAT)) {
				viewport_zoom(-0.2);
			}
			else if (operator_shortcut(map_get(config_keymap, "viewport_mode"))) {
				base_ui_menu.is_key_pressed = false;
				ui_menu_draw(function (ui: ui_t) {
					let mode_handle: ui_handle_t = ui_handle(__ID__);
					mode_handle.position = context_raw.viewport_mode;
					ui_text(tr("Viewport Mode"), ui_align_t.RIGHT);
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

					if (iron_raytrace_supported()) {
						array_push(modes, tr("Path Traced"));
						array_push(shortcuts, "p");
					}

					for (let i: i32 = 0; i < modes.length; ++i) {
						ui_radio(mode_handle, i, modes[i], shortcuts[i]);
					}

					let index: i32 = array_index_of(shortcuts, keyboard_key_code(ui.key_code));
					if (ui.is_key_pressed && index != -1) {
						mode_handle.position = index;
						ui.changed = true;
						context_set_viewport_mode(mode_handle.position);
					}
					else if (mode_handle.changed) {
						context_set_viewport_mode(mode_handle.position);
						ui.changed = true;
					}
				});
			}
		}

		if (operator_shortcut(map_get(config_keymap, "operator_search"))) {
			ui_base_operator_search();
		}
	}

	if (context_raw.brush_can_lock || context_raw.brush_locked) {
		if (mouse_moved && context_raw.brush_can_unlock) {
			context_raw.brush_locked = false;
			context_raw.brush_can_unlock = false;
		}

		///if (is_paint || is_sculpt)
		let b: bool = (context_raw.brush_can_lock || context_raw.brush_locked) &&
			!operator_shortcut(map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN) &&
			!operator_shortcut(map_get(config_keymap, "brush_opacity"), shortcut_type_t.DOWN) &&
			!operator_shortcut(map_get(config_keymap, "brush_angle"), shortcut_type_t.DOWN) &&
			!(decal_mask && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN));
		///end
		///if is_lab
		let b: bool = (context_raw.brush_can_lock || context_raw.brush_locked) &&
			!operator_shortcut(map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN);
		///end

		if (b) {
			mouse_unlock();
			context_raw.last_paint_x = -1;
			context_raw.last_paint_y = -1;
			if (context_raw.brush_can_lock) {
				context_raw.brush_can_lock = false;
				context_raw.brush_can_unlock = false;
				context_raw.brush_locked = false;
			}
			else {
				context_raw.brush_can_unlock = true;
			}
		}
	}

	if (ui_base_border_handle != null) {
		if (ui_base_border_handle == ui_nodes_hwnd || ui_base_border_handle == ui_view2d_hwnd) {
			if (ui_base_border_started == border_side_t.LEFT) {
				config_raw.layout[layout_size_t.NODES_W] -= math_floor(mouse_movement_x);
				if (config_raw.layout[layout_size_t.NODES_W] < 32) {
					config_raw.layout[layout_size_t.NODES_W] = 32;
				}
				else if (config_raw.layout[layout_size_t.NODES_W] > sys_width() * 0.7) {
					config_raw.layout[layout_size_t.NODES_W] = math_floor(sys_width() * 0.7);
				}
			}
			else { // UINodes / UIView2D ratio
				config_raw.layout[layout_size_t.NODES_H] -= math_floor(mouse_movement_y);
				if (config_raw.layout[layout_size_t.NODES_H] < 32) {
					config_raw.layout[layout_size_t.NODES_H] = 32;
				}
				else if (config_raw.layout[layout_size_t.NODES_H] > app_h() * 0.95) {
					config_raw.layout[layout_size_t.NODES_H] = math_floor(app_h() * 0.95);
				}
			}
		}
		else if (ui_base_border_handle == ui_base_hwnds[tab_area_t.STATUS]) {
			let my: i32 = math_floor(mouse_movement_y);
			if (config_raw.layout[layout_size_t.STATUS_H] - my >= ui_status_default_status_h * config_raw.window_scale && config_raw.layout[layout_size_t.STATUS_H] - my < sys_height() * 0.7) {
				config_raw.layout[layout_size_t.STATUS_H] -= my;
			}
		}
		else {
			if (ui_base_border_started == border_side_t.LEFT) {
				config_raw.layout[layout_size_t.SIDEBAR_W] -= math_floor(mouse_movement_x);
				if (config_raw.layout[layout_size_t.SIDEBAR_W] < ui_base_sidebar_mini_w) {
					config_raw.layout[layout_size_t.SIDEBAR_W] = ui_base_sidebar_mini_w;
				}
				else if (config_raw.layout[layout_size_t.SIDEBAR_W] > sys_width() - ui_base_sidebar_mini_w) {
					config_raw.layout[layout_size_t.SIDEBAR_W] = sys_width() - ui_base_sidebar_mini_w;
				}
			}
			else {
				let my: i32 = math_floor(mouse_movement_y);
				if (ui_base_border_handle == ui_base_hwnds[tab_area_t.SIDEBAR1] && ui_base_border_started == border_side_t.TOP) {
					if (config_raw.layout[layout_size_t.SIDEBAR_H0] + my > 32 && config_raw.layout[layout_size_t.SIDEBAR_H1] - my > 32) {
						config_raw.layout[layout_size_t.SIDEBAR_H0] += my;
						config_raw.layout[layout_size_t.SIDEBAR_H1] -= my;
					}
				}
			}
		}
	}

	if (!mouse_down()) {
		ui_base_border_handle = null;
		base_is_resizing = false;
	}

	///if arm_physics
	if (context_raw.tool == workspace_tool_t.PARTICLE && context_in_paint_area() && !context_raw.paint2d) {
		util_particle_init_physics();
		let world: physics_world_t = physics_world_active;
		physics_world_update(world);
		context_raw.ddirty = 2;
		context_raw.rdirty = 2;
		if (mouse_started()) {
			if (context_raw.particle_timer != null) {
				tween_stop(context_raw.particle_timer);
				let timer: tween_anim_t = context_raw.particle_timer;
				timer.done(timer.done_data);
				context_raw.particle_timer = null;
			}
			history_push_undo = true;
			context_raw.particle_hit_x = context_raw.particle_hit_y = context_raw.particle_hit_z = 0;
			let o: object_t = scene_spawn_object(".Sphere");
			let md: material_data_t = data_get_material("Scene", ".Gizmo");
			let mo: mesh_object_t = o.ext;
			mo.base.name = ".Bullet";
			mo.materials[0] = md;
			mo.base.visible = true;

			let camera: camera_object_t = scene_camera;
			let ct: transform_t = camera.base.transform;
			mo.base.transform.loc = vec4_create(transform_world_x(ct), transform_world_y(ct), transform_world_z(ct));
			mo.base.transform.scale = vec4_create(context_raw.brush_radius * 0.2, context_raw.brush_radius * 0.2, context_raw.brush_radius * 0.2);
			transform_build_matrix(mo.base.transform);

			let body: physics_body_t = physics_body_create();
			body.shape = physics_shape_t.SPHERE;
			body.mass = 1.0;
			physics_body_init(body, mo.base);

			let ray: ray_t = raycast_get_ray(mouse_view_x(), mouse_view_y(), camera);
			physics_body_apply_impulse(body, vec4_mult(ray.dir, 0.15));

			context_raw.particle_timer = tween_timer(5, function (mo: mesh_object_t) {
				mesh_object_remove(mo);
			}, mo);
		}

		let pairs: physics_pair_t[] = physics_world_get_contact_pairs(world, context_raw.paint_body);
		if (pairs != null) {
			for (let i: i32 = 0; i < pairs.length; ++i) {
				let p: physics_pair_t = pairs[i];
				context_raw.last_particle_hit_x = context_raw.particle_hit_x != 0 ? context_raw.particle_hit_x : p.pos_a_x;
				context_raw.last_particle_hit_y = context_raw.particle_hit_y != 0 ? context_raw.particle_hit_y : p.pos_a_y;
				context_raw.last_particle_hit_z = context_raw.particle_hit_z != 0 ? context_raw.particle_hit_z : p.pos_a_z;
				context_raw.particle_hit_x = p.pos_a_x;
				context_raw.particle_hit_y = p.pos_a_y;
				context_raw.particle_hit_z = p.pos_a_z;
				context_raw.pdirty = 1;
				break; // 1 pair for now
			}
		}
	}
	///end
}

function ui_base_view_top() {
	let is_typing: bool = ui_base_ui.is_typing || ui_view2d_ui.is_typing || ui_nodes_ui.is_typing;

	if (context_in_paint_area() && !is_typing) {
		if (mouse_view_x() < app_w()) {
			viewport_set_view(0, 0, 1, 0, 0, 0);
		}
	}
}

function ui_base_operator_search() {
	_ui_base_operator_search_first = true;

	ui_menu_draw(function (ui: ui_t) {
		ui_menu_h = ui_ELEMENT_H(ui) * 8;
		let search_handle: ui_handle_t = ui_handle(__ID__);
		let search: string = ui_text_input(search_handle, "", ui_align_t.LEFT, true, true);
		ui.changed = false;
		if (_ui_base_operator_search_first) {
			_ui_base_operator_search_first = false;
			search_handle.text = "";
			ui_start_text_edit(search_handle); // Focus search bar
		}

		if (search_handle.changed) {
			ui_base_operator_search_offset = 0;
		}

		if (ui.is_key_pressed) { // Move selection
			if (ui.key_code == key_code_t.DOWN && ui_base_operator_search_offset < 6) {
				ui_base_operator_search_offset++;
			}
			if (ui.key_code == key_code_t.UP && ui_base_operator_search_offset > 0) {
				ui_base_operator_search_offset--;
			}
		}
		let enter: bool = keyboard_down("enter");
		let count: i32 = 0;
		let BUTTON_COL: i32 = ui.ops.theme.BUTTON_COL;

		let keys: string[] = map_keys(config_keymap);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let n: string = keys[i];
			if (string_index_of(n, search) >= 0) {
				ui.ops.theme.BUTTON_COL = count == ui_base_operator_search_offset ? ui.ops.theme.HIGHLIGHT_COL : ui.ops.theme.SEPARATOR_COL;
				if (ui_button(n, ui_align_t.LEFT, map_get(config_keymap, n)) || (enter && count == ui_base_operator_search_offset)) {
					if (enter) {
						ui.changed = true;
						count = 6; // Trigger break
					}
					operator_run(n);
				}
				if (++count > 6) {
					break;
				}
			}
		}

		if (enter && count == 0) { // Hide popup on enter when command is not found
			ui.changed = true;
			search_handle.text = "";
		}
		ui.ops.theme.BUTTON_COL = BUTTON_COL;
	});
}

function ui_base_toggle_distract_free() {
	ui_base_show = !ui_base_show;
	base_resize();
}

function ui_base_get_radius_increment(): f32 {
	return 0.1;
}

function ui_base_hit_rect(mx: f32, my: f32, x: i32, y: i32, w: i32, h: i32): bool {
	return mx > x && mx < x + w && my > y && my < y + h;
}

function ui_base_get_brush_stencil_rect(): rect_t {
	let w: i32 = math_floor(context_raw.brush_stencil_image.width * (base_h() / context_raw.brush_stencil_image.height) * context_raw.brush_stencil_scale);
	let h: i32 = math_floor(base_h() * context_raw.brush_stencil_scale);
	let x: i32 = math_floor(base_x() + context_raw.brush_stencil_x * base_w());
	let y: i32 = math_floor(base_y() + context_raw.brush_stencil_y * base_h());
	let r: rect_t = {
		w: w,
		h: h,
		x: x,
		y: y
	};
	return r;
}

function ui_base_update_ui() {
	if (console_message_timer > 0) {
		console_message_timer -= time_delta();
		if (console_message_timer <= 0) {
			ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
		}
	}

	ui_base_sidebar_mini_w = math_floor(ui_base_default_sidebar_mini_w * ui_SCALE(ui_base_ui));

	if (!base_ui_enabled) {
		return;
	}

	// Same mapping for paint and rotate (predefined in touch keymap)
	if (context_in_viewport()) {
		if (mouse_started() && map_get(config_keymap, "action_paint") == map_get(config_keymap, "action_rotate")) {
			ui_base_action_paint_remap = map_get(config_keymap, "action_paint");
			util_render_pick_pos_nor_tex();
			let is_mesh: bool = math_abs(context_raw.posx_picked) < 50 && math_abs(context_raw.posy_picked) < 50 && math_abs(context_raw.posz_picked) < 50;
			///if arm_android
			// Allow rotating with both pen and touch, because hovering a pen prevents touch input on android
			let pen_only: bool = false;
			///else
			let pen_only: bool = context_raw.pen_painting_only;
			///end
			let is_pen: bool = pen_only && pen_down();
			// Mesh picked - disable rotate
			// Pen painting only - rotate with touch, paint with pen
			if ((is_mesh && !pen_only) || is_pen) {
				map_set(config_keymap, "action_rotate", "");
				map_set(config_keymap, "action_paint", ui_base_action_paint_remap);
			}
			// World sphere picked - disable paint
			else {
				map_set(config_keymap, "action_paint", "");
				map_set(config_keymap, "action_rotate", ui_base_action_paint_remap);
			}
		}
		else if (!mouse_down() && ui_base_action_paint_remap != "") {
			map_set(config_keymap, "action_rotate", ui_base_action_paint_remap);
			map_set(config_keymap, "action_paint", ui_base_action_paint_remap);
			ui_base_action_paint_remap = "";
		}
	}

	if (context_raw.brush_stencil_image != null && operator_shortcut(map_get(config_keymap, "stencil_transform"), shortcut_type_t.DOWN)) {
		let r: rect_t = ui_base_get_brush_stencil_rect();
		if (mouse_started("left")) {
			context_raw.brush_stencil_scaling =
				ui_base_hit_rect(mouse_x, mouse_y, r.x - 8,       r.y - 8,       16, 16) ||
				ui_base_hit_rect(mouse_x, mouse_y, r.x - 8,       r.h + r.y - 8, 16, 16) ||
				ui_base_hit_rect(mouse_x, mouse_y, r.w + r.x - 8, r.y - 8,       16, 16) ||
				ui_base_hit_rect(mouse_x, mouse_y, r.w + r.x - 8, r.h + r.y - 8, 16, 16);
			let cosa: f32 = math_cos(-context_raw.brush_stencil_angle);
			let sina: f32 = math_sin(-context_raw.brush_stencil_angle);
			let ox: f32 = 0;
			let oy: f32 = -r.h / 2;
			let x: f32 = ox * cosa - oy * sina;
			let y: f32 = ox * sina + oy * cosa;
			x += r.x + r.w / 2;
			y += r.y + r.h / 2;
			context_raw.brush_stencil_rotating =
				ui_base_hit_rect(mouse_x, mouse_y, math_floor(x - 16), math_floor(y - 16), 32, 32);
		}
		let _scale: f32 = context_raw.brush_stencil_scale;
		if (mouse_down("left")) {
			if (context_raw.brush_stencil_scaling) {
				let mult: i32 = mouse_x > r.x + r.w / 2 ? 1 : -1;
				context_raw.brush_stencil_scale += mouse_movement_x / 400 * mult;
			}
			else if (context_raw.brush_stencil_rotating) {
				let gizmo_x: f32 = r.x + r.w / 2;
				let gizmo_y: f32 = r.y + r.h / 2;
				context_raw.brush_stencil_angle = -math_atan2(mouse_y - gizmo_y, mouse_x - gizmo_x) - math_pi() / 2;
			}
			else {
				context_raw.brush_stencil_x += mouse_movement_x / base_w();
				context_raw.brush_stencil_y += mouse_movement_y / base_h();
			}
		}
		else {
			context_raw.brush_stencil_scaling = false;
		}
		if (mouse_wheel_delta != 0) {
			context_raw.brush_stencil_scale -= mouse_wheel_delta / 10;
		}
		// Center after scale
		let ratio: f32 = base_h() / context_raw.brush_stencil_image.height;
		let old_w: f32 = _scale * context_raw.brush_stencil_image.width * ratio;
		let new_w: f32 = context_raw.brush_stencil_scale * context_raw.brush_stencil_image.width * ratio;
		let old_h: f32 = _scale * base_h();
		let new_h: f32 = context_raw.brush_stencil_scale * base_h();
		context_raw.brush_stencil_x += (old_w - new_w) / base_w() / 2;
		context_raw.brush_stencil_y += (old_h - new_h) / base_h() / 2;
	}

	let set_clone_source: bool = context_raw.tool == workspace_tool_t.CLONE && operator_shortcut(map_get(config_keymap, "set_clone_source") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);

	let decal: bool = context_is_decal();
	let decal_mask: bool = context_is_decal_mask_paint();

	///if (is_paint || is_sculpt)
	let down: bool = operator_shortcut(map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
					 decal_mask ||
					 set_clone_source ||
					 operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
					 (pen_down() && !keyboard_down("alt"));
	///end
	///if is_lab
	let down: bool = operator_shortcut(map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
					 set_clone_source ||
					 operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
					 (pen_down() && !keyboard_down("alt"));
	///end

	if (config_raw.touch_ui) {
		if (pen_down()) {
			context_raw.pen_painting_only = true;
		}
		else if (context_raw.pen_painting_only) {
			down = false;
		}
	}

	///if arm_physics
	if (context_raw.tool == workspace_tool_t.PARTICLE) {
		down = false;
	}
	///end

	///if arm_ios
	// No hover on iPad, decals are painted by pen release
	if (decal) {
		down = pen_released();
		if (!context_raw.pen_painting_only) {
			down = down || mouse_released();
		}
	}
	///end

	if (down) {
		let mx: i32 = mouse_view_x();
		let my: i32 = mouse_view_y();
		let ww: i32 = app_w();

		if (context_raw.paint2d) {
			mx -= app_w();
			ww = ui_view2d_ww;
		}

		if (mx < ww &&
			mx > app_x() &&
			my < app_h() &&
			my > app_y()) {

			if (set_clone_source) {
				context_raw.clone_start_x = mx;
				context_raw.clone_start_y = my;
			}
			else {
				if (context_raw.brush_time == 0 &&
					!base_is_dragging &&
					!base_is_resizing &&
					!base_is_combo_selected()) { // Paint started

					// Draw line
					if (operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN)) {
						context_raw.last_paint_vec_x = context_raw.last_paint_x;
						context_raw.last_paint_vec_y = context_raw.last_paint_y;
					}

					history_push_undo = true;

					if (context_raw.tool == workspace_tool_t.CLONE && context_raw.clone_start_x >= 0.0) { // Clone delta
						context_raw.clone_delta_x = (context_raw.clone_start_x - mx) / ww;
						context_raw.clone_delta_y = (context_raw.clone_start_y - my) / app_h();
						context_raw.clone_start_x = -1;
					}
					else if (context_raw.tool == workspace_tool_t.FILL && context_raw.fill_type_handle.position == fill_type_t.UV_ISLAND) {
						util_uv_uvislandmap_cached = false;
					}
				}

				context_raw.brush_time += time_delta();

				if (context_raw.run_brush != null) {
					context_raw.run_brush(context_raw.brush_output_node_inst, 0);
				}
			}
		}
	}
	else if (context_raw.brush_time > 0) { // Brush released
		context_raw.brush_time = 0;
		context_raw.prev_paint_vec_x = -1;
		context_raw.prev_paint_vec_y = -1;
		///if (arm_opengl || arm_direct3d11) // Keep accumulated samples for D3D12
		context_raw.ddirty = 3;
		///end
		context_raw.brush_blend_dirty = true; // Update brush mask

		context_raw.layer_preview_dirty = true; // Update layer preview

		// New color id picked, update fill layer
		if (context_raw.tool == workspace_tool_t.COLORID && context_raw.layer.fill_layer != null) {
			app_notify_on_next_frame(function () {
				layers_update_fill_layer();
				make_material_parse_paint_material(false);
			});
		}
	}

	if (context_raw.layers_preview_dirty) {
		context_raw.layers_preview_dirty = false;
		context_raw.layer_preview_dirty = false;
		context_raw.mask_preview_last = null;
		// Update all layer previews
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (slot_layer_is_group(l)) {
				continue;
			}

			let target: image_t = l.texpaint_preview;
			if (target == null) {
				continue;
			}

			let source: image_t = l.texpaint;
			g2_begin(target);
			g2_clear(0x00000000);
			// g2_set_pipeline(l.is_mask() ? pipes_copy8 : pipes_copy);
			g2_set_pipeline(pipes_copy); // texpaint_preview is always RGBA32 for now
			g2_draw_scaled_image(source, 0, 0, target.width, target.height);
			g2_set_pipeline(null);
			g2_end();
		}
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	}
	if (context_raw.layer != null && context_raw.layer_preview_dirty && !slot_layer_is_group(context_raw.layer)) {
		context_raw.layer_preview_dirty = false;
		context_raw.mask_preview_last = null;
		// Update layer preview
		let l: slot_layer_t = context_raw.layer;

		let target: image_t = l.texpaint_preview;
		if (target != null) {

			let source: image_t = l.texpaint;
			g2_begin(target);
			g2_clear(0x00000000);
			// g2_set_pipeline(raw.layer.is_mask() ? pipes_copy8 : pipes_copy);
			g2_set_pipeline(pipes_copy); // texpaint_preview is always RGBA32 for now
			g2_draw_scaled_image(source, 0, 0, target.width, target.height);
			g2_set_pipeline(null);
			g2_end();
			ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		}
	}

	let undo_pressed: bool = operator_shortcut(map_get(config_keymap, "edit_undo"));
	let redo_pressed: bool = operator_shortcut(map_get(config_keymap, "edit_redo")) ||
							 (keyboard_down("control") && keyboard_started("y"));

	// Two-finger tap to undo, three-finger tap to redo
	if (context_in_viewport() && config_raw.touch_ui) {
		if (mouse_started("middle")) {
			ui_base_redo_tap_time = time_time();
		}
		else if (mouse_started("right")) {
			ui_base_undo_tap_time = time_time();
		}
		else if (mouse_released("middle") && time_time() - ui_base_redo_tap_time < 0.1) {
			ui_base_redo_tap_time = ui_base_undo_tap_time = 0;
			redo_pressed = true;
		}
		else if (mouse_released("right") && time_time() - ui_base_undo_tap_time < 0.1) {
			ui_base_redo_tap_time = ui_base_undo_tap_time = 0;
			undo_pressed = true;
		}
	}

	if (undo_pressed) {
		history_undo();
	}
	else if (redo_pressed) {
		history_redo();
	}

	///if (is_paint || is_sculpt)
	gizmo_update();
	///end
}

function ui_base_render() {
	if (!ui_base_show && config_raw.touch_ui) {
		ui_base_ui.input_enabled = true;
		g2_end();
		ui_begin(ui_base_ui);
		if (ui_window(ui_handle(__ID__), 0, 0, 150, math_floor(ui_ELEMENT_H(ui_base_ui) + ui_ELEMENT_OFFSET(ui_base_ui) + 1))) {
			if (ui_button(tr("Close"))) {
				ui_base_toggle_distract_free();
			}
		}
		ui_end();
		g2_begin(null);
	}

	if (!ui_base_show || sys_width() == 0 || sys_height() == 0) {
		return;
	}

	ui_base_ui.input_enabled = base_ui_enabled;

	// Remember last tab positions
	for (let i: i32 = 0; i < ui_base_htabs.length; ++i) {
		if (ui_base_htabs[i].changed) {
			config_raw.layout_tabs[i] = ui_base_htabs[i].position;
			config_save();
		}
	}

	// Set tab positions
	for (let i: i32 = 0; i < ui_base_htabs.length; ++i) {
		ui_base_htabs[i].position = config_raw.layout_tabs[i];
	}

	g2_end();
	ui_begin(ui_base_ui);

	///if (is_paint || is_sculpt)
	ui_toolbar_render_ui();
	///end
	ui_menubar_render_ui();
	ui_header_render_ui();
	ui_status_render_ui();

	///if (is_paint || is_sculpt)
	ui_base_draw_sidebar();
	///end

	ui_end();
	g2_begin(null);
}

function ui_base_draw_sidebar() {
	// Tabs
	let mini: bool = config_raw.layout[layout_size_t.SIDEBAR_W] <= ui_base_sidebar_mini_w;
	let expand_button_offset: i32 = config_raw.touch_ui ? math_floor(ui_ELEMENT_H(ui_base_ui) + ui_ELEMENT_OFFSET(ui_base_ui)) : 0;
	ui_base_tabx = sys_width() - config_raw.layout[layout_size_t.SIDEBAR_W];

	let _SCROLL_W: i32 = ui_base_ui.ops.theme.SCROLL_W;
	if (mini) {
		ui_base_ui.ops.theme.SCROLL_W = ui_base_ui.ops.theme.SCROLL_MINI_W;
	}

	if (ui_window(ui_base_hwnds[tab_area_t.SIDEBAR0], ui_base_tabx, 0, config_raw.layout[layout_size_t.SIDEBAR_W], config_raw.layout[layout_size_t.SIDEBAR_H0])) {
		let tabs: tab_draw_t[] = ui_base_hwnd_tabs[tab_area_t.SIDEBAR0];
		for (let i: i32 = 0; i < (mini ? 1 : tabs.length); ++i) {
			tabs[i].f(ui_base_htabs[tab_area_t.SIDEBAR0]);
		}
	}
	if (ui_window(ui_base_hwnds[tab_area_t.SIDEBAR1], ui_base_tabx, config_raw.layout[layout_size_t.SIDEBAR_H0], config_raw.layout[layout_size_t.SIDEBAR_W], config_raw.layout[layout_size_t.SIDEBAR_H1] - expand_button_offset)) {
		let tabs: tab_draw_t[] = ui_base_hwnd_tabs[tab_area_t.SIDEBAR1];
		for (let i: i32 = 0; i < (mini ? 1 : tabs.length); ++i) {
			tabs[i].f(ui_base_htabs[tab_area_t.SIDEBAR1]);
		}
	}

	ui_end_window();
	ui_base_ui.ops.theme.SCROLL_W = _SCROLL_W;

	// Collapse / expand button for mini sidebar
	if (config_raw.touch_ui) {
		let width: i32 = config_raw.layout[layout_size_t.SIDEBAR_W];
		let height: i32 = math_floor(ui_ELEMENT_H(ui_base_ui) + ui_ELEMENT_OFFSET(ui_base_ui));
		if (ui_window(ui_handle(__ID__), sys_width() - width, sys_height() - height, width, height + 1)) {
			ui_base_ui._w = width;
			let _BUTTON_H: i32 = ui_base_ui.ops.theme.BUTTON_H;
			let _BUTTON_COL: i32 = ui_base_ui.ops.theme.BUTTON_COL;
			ui_base_ui.ops.theme.BUTTON_H = ui_base_ui.ops.theme.ELEMENT_H;
			ui_base_ui.ops.theme.BUTTON_COL = ui_base_ui.ops.theme.WINDOW_BG_COL;
			if (ui_button(mini ? "<<" : ">>")) {
				config_raw.layout[layout_size_t.SIDEBAR_W] = mini ? ui_base_default_sidebar_full_w : ui_base_default_sidebar_mini_w;
				config_raw.layout[layout_size_t.SIDEBAR_W] = math_floor(config_raw.layout[layout_size_t.SIDEBAR_W] * ui_SCALE(ui_base_ui));
			}
			ui_base_ui.ops.theme.BUTTON_H = _BUTTON_H;
			ui_base_ui.ops.theme.BUTTON_COL = _BUTTON_COL;
		}
	}

	// Expand button
	if (config_raw.layout[layout_size_t.SIDEBAR_W] == 0) {
		let width: i32 = math_floor(g2_font_width(ui_base_ui.ops.font, ui_base_ui.font_size, "<<") + 25 * ui_SCALE(ui_base_ui));
		if (ui_window(ui_base_hminimized, sys_width() - width, 0, width, math_floor(ui_ELEMENT_H(ui_base_ui) + ui_ELEMENT_OFFSET(ui_base_ui) + 1))) {
			ui_base_ui._w = width;
			let _BUTTON_H: i32 = ui_base_ui.ops.theme.BUTTON_H;
			let _BUTTON_COL: i32 = ui_base_ui.ops.theme.BUTTON_COL;
			ui_base_ui.ops.theme.BUTTON_H = ui_base_ui.ops.theme.ELEMENT_H;
			ui_base_ui.ops.theme.BUTTON_COL = ui_base_ui.ops.theme.SEPARATOR_COL;

			if (ui_button("<<")) {
				config_raw.layout[layout_size_t.SIDEBAR_W] = context_raw.maximized_sidebar_width != 0 ? context_raw.maximized_sidebar_width : math_floor(ui_base_default_sidebar_w * config_raw.window_scale);
			}
			ui_base_ui.ops.theme.BUTTON_H = _BUTTON_H;
			ui_base_ui.ops.theme.BUTTON_COL = _BUTTON_COL;
		}
	}
	else if (ui_base_htabs[tab_area_t.SIDEBAR0].changed && ui_base_htabs[tab_area_t.SIDEBAR0].position == context_raw.last_htab0_pos) {
		if (time_time() - context_raw.select_time < 0.25) {
			context_raw.maximized_sidebar_width = config_raw.layout[layout_size_t.SIDEBAR_W];
			config_raw.layout[layout_size_t.SIDEBAR_W] = 0;
		}
		context_raw.select_time = time_time();
	}
	context_raw.last_htab0_pos = ui_base_htabs[tab_area_t.SIDEBAR0].position;
}

function ui_base_render_cursor() {
	if (!base_ui_enabled) {
		return;
	}

	if (context_raw.tool == workspace_tool_t.MATERIAL || context_raw.tool == workspace_tool_t.BAKE) {
		return;
	}

	g2_set_color(0xffffffff);

	context_raw.view_index = context_raw.view_index_last;
	let mx: i32 = base_x() + context_raw.paint_vec.x * base_w();
	let my: i32 = base_y() + context_raw.paint_vec.y * base_h();
	context_raw.view_index = -1;

	// Radius being scaled
	if (context_raw.brush_locked) {
		mx += context_raw.lock_started_x - sys_width() / 2;
		my += context_raw.lock_started_y - sys_height() / 2;
	}

	if (context_raw.brush_stencil_image != null &&
		// @ts-ignore
		context_raw.tool != workspace_tool_t.BAKE &&
		context_raw.tool != workspace_tool_t.PICKER &&
		// @ts-ignore
		context_raw.tool != workspace_tool_t.MATERIAL &&
		context_raw.tool != workspace_tool_t.COLORID) {
		let r: rect_t = ui_base_get_brush_stencil_rect();
		if (!operator_shortcut(map_get(config_keymap, "stencil_hide"), shortcut_type_t.DOWN)) {
			g2_set_color(0x88ffffff);
			let angle: f32 = context_raw.brush_stencil_angle;
			let cx: f32 = r.x + r.w / 2;
			let cy: f32 = r.y + r.h / 2;
			g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(-angle)), mat3_translation(-cx, -cy)));
			g2_draw_scaled_image(context_raw.brush_stencil_image, r.x, r.y, r.w, r.h);
			g2_set_transformation(mat3_nan());
			g2_set_color(0xffffffff);
		}
		let transform: bool = operator_shortcut(map_get(config_keymap, "stencil_transform"), shortcut_type_t.DOWN);
		if (transform) {
			// Outline
			g2_draw_rect(r.x, r.y, r.w, r.h);
			// Scale
			g2_draw_rect(r.x - 8,       r.y - 8,       16, 16);
			g2_draw_rect(r.x - 8 + r.w, r.y - 8,       16, 16);
			g2_draw_rect(r.x - 8,       r.y - 8 + r.h, 16, 16);
			g2_draw_rect(r.x - 8 + r.w, r.y - 8 + r.h, 16, 16);
			// Rotate
			let angle: f32 = context_raw.brush_stencil_angle;
			let cx: f32 = r.x + r.w / 2;
			let cy: f32 = r.y + r.h / 2;
			g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(-angle)), mat3_translation(-cx, -cy)));
			g2_fill_circle(r.x + r.w / 2, r.y - 4, 8);
			g2_set_transformation(mat3_nan());
		}
	}

	// Show picked material next to cursor
	if (context_raw.tool == workspace_tool_t.PICKER && context_raw.picker_select_material && context_raw.color_picker_callback == null) {
		let img: image_t = context_raw.material.image_icon;
		g2_draw_image(img, mx + 10, my + 10);
	}
	if (context_raw.tool == workspace_tool_t.PICKER && context_raw.color_picker_callback != null) {
		let img: image_t = resource_get("icons.k");
		let rect: rect_t = resource_tile50(img, workspace_tool_t.PICKER, 0);
		g2_draw_sub_image(img, mx + 10, my + 10, rect.x, rect.y, rect.w, rect.h);
	}

	let cursor_img: image_t = resource_get("cursor.k");
	let psize: i32 = math_floor(182 * (context_raw.brush_radius * context_raw.brush_nodes_radius) * ui_SCALE(ui_base_ui));

	// Clone source cursor
	if (context_raw.tool == workspace_tool_t.CLONE && !keyboard_down("alt") && (mouse_down() || pen_down())) {
		g2_set_color(0x66ffffff);
		g2_draw_scaled_image(cursor_img, mx + context_raw.clone_delta_x * app_w() - psize / 2, my + context_raw.clone_delta_y * app_h() - psize / 2, psize, psize);
		g2_set_color(0xffffffff);
	}

	let decal: bool = context_is_decal();

	if (!config_raw.brush_3d || context_in_2d_view() || decal) {
		let decal_mask: bool = context_is_decal_mask();
		if (decal && !context_in_nodes()) {
			let decal_alpha: f32 = 0.5;
			if (!decal_mask) {
				context_raw.decal_x = context_raw.paint_vec.x;
				context_raw.decal_y = context_raw.paint_vec.y;
				decal_alpha = context_raw.brush_opacity;

				// Radius being scaled
				if (context_raw.brush_locked) {
					context_raw.decal_x += (context_raw.lock_started_x - sys_width() / 2) / base_w();
					context_raw.decal_y += (context_raw.lock_started_y - sys_height() / 2) / base_h();
				}
			}

			if (!config_raw.brush_live) {
				let psizex: i32 = math_floor(256 * ui_SCALE(ui_base_ui) * (context_raw.brush_radius * context_raw.brush_nodes_radius * context_raw.brush_scale_x));
				let psizey: i32 = math_floor(256 * ui_SCALE(ui_base_ui) * (context_raw.brush_radius * context_raw.brush_nodes_radius));

				context_raw.view_index = context_raw.view_index_last;
				let decalx: f32 = base_x() + context_raw.decal_x * base_w() - psizex / 2;
				let decaly: f32 = base_y() + context_raw.decal_y * base_h() - psizey / 2;
				context_raw.view_index = -1;

				g2_set_color(color_from_floats(1, 1, 1, decal_alpha));
				let angle: f32 = (context_raw.brush_angle + context_raw.brush_nodes_angle) * (math_pi() / 180);
				let cx: f32 = decalx + psizex / 2;
				let cy: f32 = decaly + psizey / 2;
				g2_set_transformation(mat3_multmat(mat3_multmat(mat3_translation(cx, cy), mat3_rotation(angle)), mat3_translation(-cx, -cy)));
				g2_draw_scaled_image(context_raw.decal_image, decalx, decaly, psizex, psizey);
				g2_set_transformation(mat3_nan());
				g2_set_color(0xffffffff);
			}
		}
		if (context_raw.tool == workspace_tool_t.BRUSH  ||
			context_raw.tool == workspace_tool_t.ERASER ||
			context_raw.tool == workspace_tool_t.CLONE  ||
			context_raw.tool == workspace_tool_t.BLUR   ||
			context_raw.tool == workspace_tool_t.SMUDGE   ||
			context_raw.tool == workspace_tool_t.PARTICLE ||
			(decal_mask && !config_raw.brush_3d) ||
			(decal_mask && context_in_2d_view())) {
			if (decal_mask) {
				psize = math_floor(cursor_img.width * (context_raw.brush_decal_mask_radius * context_raw.brush_nodes_radius) * ui_SCALE(ui_base_ui));
			}
			if (config_raw.brush_3d && context_in_2d_view()) {
				psize = math_floor(psize * ui_view2d_pan_scale);
			}
			g2_draw_scaled_image(cursor_img, mx - psize / 2, my - psize / 2, psize, psize);
		}
	}

	if (context_raw.brush_lazy_radius > 0 && !context_raw.brush_locked &&
		(context_raw.tool == workspace_tool_t.BRUSH ||
			context_raw.tool == workspace_tool_t.ERASER ||
			context_raw.tool == workspace_tool_t.DECAL ||
			context_raw.tool == workspace_tool_t.TEXT ||
			context_raw.tool == workspace_tool_t.CLONE ||
			context_raw.tool == workspace_tool_t.BLUR ||
			context_raw.tool == workspace_tool_t.SMUDGE ||
			context_raw.tool == workspace_tool_t.PARTICLE)) {
		g2_fill_rect(mx - 1, my - 1, 2, 2);
		mx = context_raw.brush_lazy_x * base_w() + base_x();
		my = context_raw.brush_lazy_y * base_h() + base_y();
		let radius: f32 = context_raw.brush_lazy_radius * 180;
		g2_set_color(0xff666666);
		g2_draw_scaled_image(cursor_img, mx - radius / 2, my - radius / 2, radius, radius);
		g2_set_color(0xffffffff);
	}
}

function ui_base_show_material_nodes() {
	// Clear input state as ui receives input events even when not drawn
	_ui_end_input(ui_nodes_ui);

	///if (is_paint || is_sculpt)
	ui_nodes_show = !ui_nodes_show || ui_nodes_canvas_type != canvas_type_t.MATERIAL;
	ui_nodes_canvas_type = canvas_type_t.MATERIAL;
	///end
	///if is_lab
	ui_nodes_show = !ui_nodes_show;
	///end

	base_resize();
}

function ui_base_show_brush_nodes() {
	// Clear input state as ui receives input events even when not drawn
	_ui_end_input(ui_nodes_ui);
	ui_nodes_show = !ui_nodes_show || ui_nodes_canvas_type != canvas_type_t.BRUSH;
	ui_nodes_canvas_type = canvas_type_t.BRUSH;
	base_resize();
}

function ui_base_show_2d_view(type: view_2d_type_t) {
	// Clear input state as ui receives input events even when not drawn
	_ui_end_input(ui_view2d_ui);
	if (ui_view2d_type != type) {
		ui_view2d_show = true;
	}
	else {
		ui_view2d_show = !ui_view2d_show;
	}
	ui_view2d_type = type;
	ui_view2d_hwnd.redraws = 2;
	base_resize();
}

function ui_base_toggle_browser() {
	let minimized: bool = config_raw.layout[layout_size_t.STATUS_H] <= (ui_status_default_status_h * config_raw.window_scale);
	config_raw.layout[layout_size_t.STATUS_H] = minimized ? 240 : ui_status_default_status_h;
	config_raw.layout[layout_size_t.STATUS_H] = math_floor(config_raw.layout[layout_size_t.STATUS_H] * config_raw.window_scale);
}

function ui_base_set_icon_scale() {
	if (ui_SCALE(ui_base_ui) > 1) {
		let res: string[] = ["icons2x.k"];
		resource_load(res);
		map_set(resource_bundled, "icons.k", resource_get("icons2x.k"));
	}
	else {
		let res: string[] = ["icons.k"];
		resource_load(res);
	}
}

function ui_base_on_border_hover(handle: ui_handle_t, side: i32) {
	if (!base_ui_enabled) return;

	if (handle != ui_base_hwnds[tab_area_t.SIDEBAR0] &&
		handle != ui_base_hwnds[tab_area_t.SIDEBAR1] &&
		handle != ui_base_hwnds[tab_area_t.STATUS] &&
		handle != ui_nodes_hwnd &&
		handle != ui_view2d_hwnd) {
		return; // Scalable handles
	}
	if (handle == ui_view2d_hwnd && side != border_side_t.LEFT) {
		return;
	}
	if (handle == ui_nodes_hwnd && side == border_side_t.TOP && !ui_view2d_show) {
		return;
	}
	if (handle == ui_base_hwnds[tab_area_t.SIDEBAR0] && side == border_side_t.TOP) {
		return;
	}

	if (handle == ui_nodes_hwnd && side != border_side_t.LEFT && side != border_side_t.TOP) {
		return;
	}
	if (handle == ui_base_hwnds[tab_area_t.STATUS] && side != border_side_t.TOP) {
		return;
	}
	if (side == border_side_t.RIGHT) {
		return; // UI is snapped to the right side
	}

	side == border_side_t.LEFT || side == border_side_t.RIGHT ?
		iron_set_mouse_cursor(3) : // Horizontal
		iron_set_mouse_cursor(4);  // Vertical

	if (ui_get_current().input_started) {
		ui_base_border_started = side;
		ui_base_border_handle = handle;
		base_is_resizing = true;
	}
}

function ui_base_on_text_hover() {
	iron_set_mouse_cursor(2); // I-cursor
}

function ui_base_on_deselect_text() {
	///if arm_ios
	keyboard_up_listener(key_code_t.SHIFT);
	///end
}

function ui_base_on_tab_drop(to: ui_handle_t, to_position: i32, from: ui_handle_t, from_position: i32) {
	let i: i32 = -1;
	let j: i32 = -1;
	for (let k: i32 = 0; k < ui_base_htabs.length; ++k) {
		if (ui_base_htabs[k] == to) {
			i = k;
		}
		if (ui_base_htabs[k] == from) {
			j = k;
		}
	}
	if (i == j && to_position == from_position) {
		return;
	}
	if (i > -1 && j > -1) {
		let tabsi: tab_draw_t[] = ui_base_hwnd_tabs[i];
		let tabsj: tab_draw_t[] = ui_base_hwnd_tabs[j];
		let element: tab_draw_t = tabsj[from_position];
		array_splice(tabsj, from_position, 1);
		array_insert(tabsi, to_position, element);
		ui_base_hwnds[i].redraws = 2;
		ui_base_hwnds[j].redraws = 2;
	}
}

function ui_base_tag_ui_redraw() {
	ui_header_handle.redraws = 2;
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	ui_menubar_workspace_handle.redraws = 2;
	ui_menubar_menu_handle.redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	ui_toolbar_handle.redraws = 2;
}

function ui_base_make_empty_envmap(col: i32) {
	ui_base_viewport_col = col;
	let b: u8_array_t = u8_array_create(4);
	b[0] = color_get_rb(col);
	b[1] = color_get_gb(col);
	b[2] = color_get_bb(col);
	b[3] = 255;
	context_raw.empty_envmap = image_from_bytes(b, 1, 1);
}

function ui_base_set_viewport_col(col: i32) {
	ui_base_make_empty_envmap(col);
	context_raw.ddirty = 2;
	if (!context_raw.show_envmap) {
		scene_world._.envmap = context_raw.empty_envmap;
	}
}
