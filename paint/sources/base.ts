
let base_ui_enabled: bool = true;
let base_is_dragging: bool = false;
let base_is_resizing: bool = false;
let base_drag_asset: asset_t = null;
let base_drag_swatch: swatch_color_t = null;
let base_drag_file: string = null;
let base_drag_file_icon: gpu_texture_t = null;
let base_drag_tint: i32 = 0xffffffff;
let base_drag_size: i32 = -1;
let base_drag_rect: rect_t = null;
let base_drag_off_x: f32 = 0.0;
let base_drag_off_y: f32 = 0.0;
let base_drag_start: f32 = 0.0;
let base_drop_x: f32 = 0.0;
let base_drop_y: f32 = 0.0;
let base_font: draw_font_t = null;
let base_theme: ui_theme_t;
let base_color_wheel: gpu_texture_t;
let base_color_wheel_gradient: gpu_texture_t;
let base_default_element_w: i32 = 100;
let base_default_element_h: i32 = 28;
let base_default_font_size: i32 = 13;
let base_res_handle: ui_handle_t = ui_handle_create();
let base_bits_handle: ui_handle_t = ui_handle_create();
let base_drop_paths: string[] = [];
let base_appx: i32 = 0;
let base_appy: i32 = 0;
let base_last_window_width: i32 = 0;
let base_last_window_height: i32 = 0;
let base_drag_material: slot_material_t = null;
let base_drag_layer: slot_layer_t = null;
let base_default_fov: f32 = 0.69;
let _base_material_count: i32;

let ui: ui_t;
let ui_base_show: bool = true;
let ui_base_border_started: i32 = 0;
let ui_base_border_handle: ui_handle_t = null;
let ui_base_action_paint_remap: string = "";
let ui_base_operator_search_offset: i32 = 0;
let ui_base_undo_tap_time: f32 = 0.0;
let ui_base_redo_tap_time: f32 = 0.0;
let ui_base_viewport_col: i32;
let _ui_base_operator_search_first: bool;

type tab_draw_t = {
	f: (h: ui_handle_t)=>void;
};
type tab_draw_array_t = tab_draw_t[];

let ui_base_hwnds: ui_handle_t[] = ui_base_init_hwnds();
let ui_base_htabs: ui_handle_t[] = ui_base_init_htabs();
let ui_base_hwnd_tabs: tab_draw_array_t[] = ui_base_init_hwnd_tabs();

function base_init() {
	base_last_window_width = iron_window_width();
	base_last_window_height = iron_window_height();

	sys_notify_on_drop_files(function (drop_path: string) {
		///if arm_linux
		drop_path = uri_decode(drop_path);
		///end
		drop_path = trim_end(drop_path);
		array_push(base_drop_paths, drop_path);
	});

	sys_notify_on_app_state(
		function () { // Foreground
			context_raw.foreground_event = true;
			context_raw.last_paint_x = -1;
			context_raw.last_paint_y = -1;
		},
		function () {}, // Resume
		function () {}, // Pause
		function () { // Background
			// Release keys after alt-tab / win-tab
			keyboard_up_listener(key_code_t.ALT);
			keyboard_up_listener(key_code_t.WIN);
		},
		function () { // Shutdown
			///if (arm_android || arm_ios)
			project_save();
			///end
			config_save();
		}
	);

	iron_set_save_and_quit_callback(base_save_and_quit_callback);

	base_font = data_get_font("font.ttf");
	base_color_wheel = data_get_image("color_wheel.k");
	base_color_wheel_gradient = data_get_image("color_wheel_gradient.k");
	config_load_theme(config_raw.theme, false);
	base_default_element_w = base_theme.ELEMENT_W;
	base_default_element_h = base_theme.ELEMENT_H;
	base_default_font_size = base_theme.FONT_SIZE;
	translator_load_translations(config_raw.locale);
	ui_files_filename = tr("untitled");
	///if (arm_android || arm_ios)
	sys_title_set(tr("untitled"));
	///end

	// Baked font for fast startup
	if (config_raw.locale == "en") {
		draw_font_13(base_font);
	}
	else {
		draw_font_init(base_font);
	}

	ui_nodes_enum_texts = base_enum_texts;

	// Init plugins
	if (config_raw.plugins != null) {
		for (let i: i32 = 0; i < config_raw.plugins.length; ++i) {
			let plugin: string = config_raw.plugins[i];
			plugin_start(plugin);
		}
	}

	args_parse();
	camera_init();
	ui_base_init();
	ui_viewnodes_init();
	ui_view2d_init();

	sys_notify_on_update(base_update);
	sys_notify_on_update(ui_view2d_update);
	sys_notify_on_update(ui_nodes_update);
	sys_notify_on_update(ui_base_update);
	sys_notify_on_update(camera_update);

	sys_notify_on_render(ui_view2d_render);
	sys_notify_on_render(ui_base_render_cursor);
	sys_notify_on_render(ui_nodes_render);
	sys_notify_on_render(ui_base_render);
	sys_notify_on_render(base_render);

	base_appx = ui_toolbar_w(true);
	base_appy = 0;
	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		base_appy = ui_header_h * 2;
	}
	scene_camera.data.fov = math_floor(scene_camera.data.fov * 100) / 100;
	camera_object_build_proj(scene_camera);

	args_run();

	let has_projects: bool = config_raw.recent_projects.length > 0;
	if (config_raw.splash_screen && has_projects) {
		box_projects_show();
	}
}

function base_save_and_quit_callback(save: bool) {
	base_save_window_rect();
	if (save) {
		project_save(true);
	}
	else {
		iron_stop();
	}
}

function base_w(): i32 {
	// Drawing material preview
	if (context_raw.material_preview) {
		return util_render_material_preview_size;
	}

	// Drawing decal preview
	if (context_raw.decal_preview) {
		return util_render_decal_preview_size;
	}

	let res: i32 = 0;
	if (config_raw.layout == null) {
		let sidebarw: i32 = ui_sidebar_default_w;
		res = iron_window_width() - sidebarw - ui_toolbar_default_w;
	}
	else if (ui_nodes_show || ui_view2d_show) {
		res = iron_window_width() - config_raw.layout[layout_size_t.SIDEBAR_W] - config_raw.layout[layout_size_t.NODES_W] - ui_toolbar_w(true);
	}
	else if (ui_base_show) {
		res = iron_window_width() - config_raw.layout[layout_size_t.SIDEBAR_W] - ui_toolbar_w(true);
	}
	else { // Distract free
		res = iron_window_width();
	}
	if (context_raw.view_index > -1) {
		res = math_ceil(res / 2);
	}
	if (context_raw.paint2d_view) {
		res = ui_view2d_ww;
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}

function base_h(): i32 {
	// Drawing material preview
	if (context_raw.material_preview) {
		return util_render_material_preview_size;
	}

	// Drawing decal preview
	if (context_raw.decal_preview) {
		return util_render_decal_preview_size;
	}

	let res: i32 = iron_window_height();

	if (config_raw.layout == null) {
		res -= ui_header_default_h * 2 + ui_statusbar_default_h;
		///if (arm_android || arm_ios)
		res += ui_header_h;
		///end
	}
	else if (ui_base_show && res > 0) {
		let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
		res -= math_floor(ui_header_default_h * 2 * config_raw.window_scale) + statush;

		if (config_raw.layout[layout_size_t.HEADER] == 0) {
			res += ui_header_h * 2;
		}
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}

function base_x(): i32 {
	return context_raw.view_index == 1 ? base_appx + base_w() : base_appx;
}

function base_y(): i32 {
	return base_appy;
}

function base_on_resize() {
	if (iron_window_width() == 0 || iron_window_height() == 0) {
		return;
	}

	let ratio_w: f32 = iron_window_width() / base_last_window_width;
	base_last_window_width = iron_window_width();
	let ratio_h: f32 = iron_window_height() / base_last_window_height;
	base_last_window_height = iron_window_height();

	config_raw.layout[layout_size_t.NODES_W] = math_floor(config_raw.layout[layout_size_t.NODES_W] * ratio_w);
	config_raw.layout[layout_size_t.SIDEBAR_H0] = math_floor(config_raw.layout[layout_size_t.SIDEBAR_H0] * ratio_h);
	config_raw.layout[layout_size_t.SIDEBAR_H1] = iron_window_height() - config_raw.layout[layout_size_t.SIDEBAR_H0];

	base_resize();
	base_save_window_rect();
}

function base_save_window_rect() {
	config_raw.window_w = iron_window_width();
	config_raw.window_h = iron_window_height();
	config_raw.window_x = iron_window_x();
	config_raw.window_y = iron_window_y();
	config_save();
}

function base_resize() {
	if (iron_window_width() == 0 || iron_window_height() == 0) {
		return;
	}

	let cam: camera_object_t = scene_camera;
	if (cam.data.ortho != null) {
		cam.data.ortho[2] = -2 * (sys_h() / sys_w());
		cam.data.ortho[3] =  2 * (sys_h() / sys_w());
	}
	camera_object_build_proj(cam);
	render_path_base_taa_frame = 0;

	if (context_raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
		viewport_update_camera_type(context_raw.camera_type);
	}

	context_raw.ddirty = 2;

	if (ui_base_show) {
		base_appx = ui_toolbar_w(true);
		base_appy = 0;
		if (config_raw.layout[layout_size_t.HEADER] == 1) {
			base_appy = ui_header_h * 2;
		}
	}
	else {
		base_appx = 0;
		base_appy = 0;
	}

	ui_nodes_grid_redraw = true;
	ui_view2d_grid_redraw = true;

	base_redraw_ui();
}

function base_redraw_ui() {
	ui_header_handle.redraws = 2;
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	ui_menubar_menu_handle.redraws = 2;
	ui_menubar_workspace_handle.redraws = 2;
	ui_nodes_hwnd.redraws = 2;
	ui_box_hwnd.redraws = 2;
	ui_view2d_hwnd.redraws = 2;
	// Redraw viewport
	if (context_raw.ddirty < 0) {
		context_raw.ddirty = 0;
	}
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	ui_toolbar_handle.redraws = 2;
	if (context_raw.split_view) {
		context_raw.ddirty = 1;
	}
}

function base_update() {
	if (mouse_movement_x != 0 || mouse_movement_y != 0) {
		iron_mouse_set_cursor(cursor_t.ARROW);
	}

	let has_drag: bool = base_drag_asset != null ||
						 base_drag_material != null ||
						 base_drag_layer != null ||
						 base_drag_file != null ||
						 base_drag_swatch != null;

	if (config_raw.touch_ui) {
		// Touch and hold to activate dragging
		if (base_drag_start < 0.2) {
			if (has_drag && mouse_down()) {
				base_drag_start += sys_real_delta();
			}
			else {
				base_drag_start = 0;
			}
			has_drag = false;
		}
		if (mouse_released()) {
			base_drag_start = 0;
		}
		let moved: bool = math_abs(mouse_movement_x) > 1 && math_abs(mouse_movement_y) > 1;
		if ((mouse_released() || moved) && !has_drag) {
			base_drag_asset = null;
			base_drag_swatch = null;
			base_drag_file = null;
			base_drag_file_icon = null;
			base_is_dragging = false;
			base_drag_material = null;
			base_drag_layer = null;
		}
		// Disable touch scrolling while dragging is active
		ui_touch_scroll = !base_is_dragging;
	}

	if (has_drag && (mouse_movement_x != 0 || mouse_movement_y != 0)) {
		base_is_dragging = true;
	}
	if (mouse_released() && has_drag) {
		if (base_drag_asset != null) {

			// Create image texture
			if (context_in_nodes()) {
				ui_nodes_accept_asset_drag(array_index_of(project_assets, base_drag_asset));
			}
			else if (context_in_viewport()) {
				if (ends_with(to_lower_case(base_drag_asset.file), ".hdr")) {
					let image: gpu_texture_t = project_get_image(base_drag_asset);
					import_envmap_run(base_drag_asset.file, image);
				}
			}
			// Create mask
			else if (context_in_layers() || context_in_2d_view()) {
				layers_create_image_mask(base_drag_asset);
			}
			base_drag_asset = null;
		}
		else if (base_drag_swatch != null) {
			// Create RGB node
			if (context_in_nodes()) {
				ui_nodes_accept_swatch_drag(base_drag_swatch);
			}
			else if (context_in_swatches()) {
				tab_swatches_accept_swatch_drag(base_drag_swatch);
			}
			else if (context_in_materials()) {
				tab_materials_accept_swatch_drag(base_drag_swatch);
			}
			else if (context_in_viewport()) {
				let color: i32 = base_drag_swatch.base;
				color = color_set_ab(color, base_drag_swatch.opacity * 255);
				layers_create_color_layer(color, base_drag_swatch.occlusion, base_drag_swatch.roughness, base_drag_swatch.metallic);
			}
			else if (context_in_layers() && tab_layers_can_drop_new_layer(context_raw.drag_dest)) {
				let color: i32 = base_drag_swatch.base;
				color = color_set_ab(color, base_drag_swatch.opacity * 255);
				layers_create_color_layer(color, base_drag_swatch.occlusion, base_drag_swatch.roughness, base_drag_swatch.metallic, context_raw.drag_dest);
			}

			base_drag_swatch = null;
		}
		else if (base_drag_file != null) {
			if (!context_in_browser()) {
				base_drop_x = mouse_x;
				base_drop_y = mouse_y;

				_base_material_count = project_materials.length;
				import_asset_run(base_drag_file, base_drop_x, base_drop_y, true, true, function () {
					// Asset was material
					if (project_materials.length > _base_material_count) {
						base_drag_material = context_raw.material;
						base_material_dropped();
					}
				});
			}
			base_drag_file = null;
			base_drag_file_icon = null;
		}
		else if (base_drag_material != null) {
			base_material_dropped();
		}
		else if (base_drag_layer != null) {
			if (context_in_nodes()) {
				ui_nodes_accept_layer_drag(array_index_of(project_layers, base_drag_layer));
			}
			else if (context_in_layers() && base_is_dragging) {
				slot_layer_move(base_drag_layer, context_raw.drag_dest);
				make_material_parse_mesh_material();
			}
			base_drag_layer = null;
		}

		iron_mouse_set_cursor(cursor_t.ARROW);
		base_is_dragging = false;
	}
	if (context_raw.color_picker_callback != null && (mouse_released() || mouse_released("right"))) {
		context_raw.color_picker_callback = null;
		context_select_tool(context_raw.color_picker_previous_tool);
	}

	base_handle_drop_paths();

	if (context_raw.ddirty < 0) {
		context_raw.ddirty = 0;
	}

	if (context_raw.tool == tool_type_t.GIZMO) {
		if (keyboard_down("control") && keyboard_started("d")) {
			sim_duplicate();
		}

		if (keyboard_started("delete")) {
			sim_delete();
		}
	}

	compass_update();
}

function base_material_dropped() {
	// Material drag and dropped onto viewport or layers tab
	if (context_in_viewport()) {
		let uv_type: uv_type_t = keyboard_down("control") ? uv_type_t.PROJECT : uv_type_t.UVMAP;
		let decal_mat: mat4_t = uv_type == uv_type_t.PROJECT ? util_render_get_decal_mat() : mat4_nan();
		layers_create_fill_layer(uv_type, decal_mat);
	}
	if (context_in_layers() && tab_layers_can_drop_new_layer(context_raw.drag_dest)) {
		let uv_type: uv_type_t = keyboard_down("control") ? uv_type_t.PROJECT : uv_type_t.UVMAP;
		let decal_mat: mat4_t = uv_type == uv_type_t.PROJECT ? util_render_get_decal_mat() : mat4_nan();
		layers_create_fill_layer(uv_type, decal_mat, context_raw.drag_dest);
	}
	else if (context_in_nodes()) {
		ui_nodes_accept_material_drag(array_index_of(project_materials, base_drag_material));
	}
	base_drag_material = null;
}

function base_handle_drop_paths() {
	if (base_drop_paths.length > 0) {
		let wait: bool = false;
		///if (arm_linux || arm_macos)
		wait = !mouse_moved; // Mouse coords not updated during drag
		///end
		if (!wait) {
			base_drop_x = mouse_x;
			base_drop_y = mouse_y;
			let drop_path: string = array_shift(base_drop_paths);
			import_asset_run(drop_path, base_drop_x, base_drop_y);
		}
	}
}

function base_get_drag_background(): rect_t {
	let icons: gpu_texture_t = resource_get("icons.k");
	if (base_drag_layer != null && !slot_layer_is_group(base_drag_layer) && base_drag_layer.fill_layer == null) {
		return resource_tile50(icons, 4, 1);
	}
	return null;
}

function base_get_drag_image(): gpu_texture_t {
	base_drag_tint = 0xffffffff;
	base_drag_size = -1;
	base_drag_rect = null;
	if (base_drag_asset != null) {
		return project_get_image(base_drag_asset);
	}
	if (base_drag_swatch != null) {
		base_drag_tint = base_drag_swatch.base;
		base_drag_size = 26;
		return tab_swatches_empty_get();
	}
	if (base_drag_file != null) {
		if (base_drag_file_icon != null) {
			return base_drag_file_icon;
		}
		let icons: gpu_texture_t = resource_get("icons.k");
		base_drag_rect = string_index_of(base_drag_file, ".") > 0 ? resource_tile50(icons, 3, 1) : resource_tile50(icons, 2, 1);
		base_drag_tint = ui.ops.theme.HIGHLIGHT_COL;
		return icons;
	}

	if (base_drag_material != null) {
		return base_drag_material.image_icon;
	}
	if (base_drag_layer != null && slot_layer_is_group(base_drag_layer)) {
		let icons: gpu_texture_t = resource_get("icons.k");
		let folder_closed: rect_t = resource_tile50(icons, 2, 1);
		let folder_open: rect_t = resource_tile50(icons, 8, 1);
		base_drag_rect = base_drag_layer.show_panel ? folder_open : folder_closed;
		base_drag_tint = ui.ops.theme.LABEL_COL - 0x00202020;
		return icons;
	}
	if (base_drag_layer != null && slot_layer_is_mask(base_drag_layer) && base_drag_layer.fill_layer == null) {
		tab_layers_make_mask_preview_rgba32(base_drag_layer);
		return context_raw.mask_preview_rgba32;
	}
	if (base_drag_layer != null) {
		return base_drag_layer.fill_layer != null ? base_drag_layer.fill_layer.image_icon : base_drag_layer.texpaint_preview;
	}

	return null;
}

function base_render() {
	if (context_raw.frame == 2) {
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;

		base_init_undo_layers();
    }

    if (context_raw.tool == tool_type_t.GIZMO) {
		sim_init();
		sim_update();
    }

	if (context_raw.frame == 2) {
		make_material_parse_mesh_material();
		make_material_parse_paint_material();
		context_raw.ddirty = 0;

		// Default workspace
		if (config_raw.workspace != 0) {
			ui_header_worktab.position = config_raw.workspace;
			ui_menubar_workspace_handle.redraws = 2;
			ui_header_worktab.changed = true;
		}

		// Default camera controls
		context_raw.camera_controls = config_raw.camera_controls;
	}
	else if (context_raw.frame == 3) {
		context_raw.ddirty = 3;
	}

	context_raw.frame++;

	if (base_is_dragging) {
		iron_mouse_set_cursor(cursor_t.HAND);
		let img: gpu_texture_t = base_get_drag_image();
		let scale_factor: f32 = UI_SCALE();
		let size: f32 = (base_drag_size == -1 ? 50 : base_drag_size) * scale_factor;
		let ratio: f32 = size / img.width;
		let h: f32 = img.height * ratio;
		let inv: i32 = 0;

		draw_begin();
		draw_set_color(base_drag_tint);

		let bg_rect: rect_t = base_get_drag_background();
		if (bg_rect != null) {
			draw_scaled_sub_image(resource_get("icons.k"), bg_rect.x, bg_rect.y, bg_rect.w, bg_rect.h, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		}

		base_drag_rect == null ?
			draw_scaled_image(img, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2) :
			draw_scaled_sub_image(img, base_drag_rect.x, base_drag_rect.y, base_drag_rect.w, base_drag_rect.h, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		draw_set_color(0xffffffff);
		draw_end();
	}

	let using_menu: bool = ui_menu_show && mouse_y > ui_header_h;
	base_ui_enabled = !ui_box_show && !using_menu && ui.combo_selected_handle == null;
	if (ui_box_show) {
		ui_box_render();
	}
	if (ui_menu_show) {
		ui_menu_render();
	}

	// Save last pos for continuos paint
	context_raw.last_paint_vec_x = context_raw.paint_vec.x;
	context_raw.last_paint_vec_y = context_raw.paint_vec.y;

	///if (arm_android || arm_ios)
	// No mouse move events for touch, re-init last paint position on touch start
	if (!mouse_down()) {
		context_raw.last_paint_x = -1;
		context_raw.last_paint_y = -1;
	}
	///end
}

function base_enum_texts(node_type: string): string[] {
	if (node_type == "TEX_IMAGE") {
		if (project_asset_names.length > 0) {
			return project_asset_names;
		}
		else {
			let empty: string[] = [""];
			return empty;
		}
	}

	if (node_type == "LAYER" || node_type == "LAYER_MASK") {
		let layer_names: string[] = [];
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			array_push(layer_names, l.name);
		}
		return layer_names;
	}

	if (node_type == "MATERIAL") {
		let material_names: string[] = [];
		for (let i: i32 = 0; i < project_materials.length; ++i) {
			let m: slot_material_t = project_materials[i];
			array_push(material_names, m.canvas.name);
		}
		return material_names;
	}

	if (node_type == "image_texture_node") {
		if (project_asset_names.length > 0) {
			return project_asset_names;
		}
		else {
			let empty: string[] = [""];
			return empty;
		}
	}

	return null;
}

function base_get_asset_index(file_name: string): i32 {
	let i: i32 = array_index_of(project_asset_names, file_name);
	return i >= 0 ? i : 0;
}

function base_toggle_fullscreen() {
	if (iron_window_get_mode() == window_mode_t.WINDOWED) {
		config_raw.window_w = iron_window_width();
		config_raw.window_h = iron_window_height();
		config_raw.window_x = iron_window_x();
		config_raw.window_y = iron_window_y();
		iron_set_window_mode(window_mode_t.FULLSCREEN);
	}
	else {
		iron_set_window_mode(window_mode_t.WINDOWED);
		iron_window_resize(config_raw.window_w, config_raw.window_h);
		iron_window_move(config_raw.window_x, config_raw.window_y);
	}
}

function base_is_decal_layer(): bool {
	let is_painting: bool = context_raw.tool != tool_type_t.MATERIAL && context_raw.tool != tool_type_t.BAKE;
	return is_painting && context_raw.layer.fill_layer != null && context_raw.layer.uv_type == uv_type_t.PROJECT;
}

function base_redraw_status() {
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
}

function base_redraw_console() {
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui != null && statush > ui_statusbar_default_h * UI_SCALE()) {
		ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	}
}

function base_init_undo_layers() {
	if (history_undo_layers == null) {
		history_undo_layers = [];
		for (let i: i32 = 0; i < config_raw.undo_steps; ++i) {
			let len: i32 = history_undo_layers.length;
			let ext: string = "_undo" + len;
			let l: slot_layer_t = slot_layer_create(ext);
			array_push(history_undo_layers, l);
		}
	}
}

function base_init_layout() {
	let raw: config_t = config_raw;
	let show2d: bool = (ui_nodes_show || ui_view2d_show) && raw.layout != null;
	let new_layout: i32[] = [];

	array_push(new_layout, math_floor(ui_sidebar_default_w * raw.window_scale)); // LayoutSidebarW
	array_push(new_layout, math_floor(iron_window_height() / 2)); // LayoutSidebarH0
	array_push(new_layout, math_floor(iron_window_height() / 2)); // LayoutSidebarH1

	///if arm_ios
	array_push(new_layout, show2d ? math_floor((sys_w() + raw.layout[layout_size_t.NODES_W]) * 0.473) : math_floor(sys_w() * 0.473)); // LayoutNodesW
	///elseif arm_android
	array_push(new_layout, show2d ? math_floor((sys_w() + raw.layout[layout_size_t.NODES_W]) * 0.473) : math_floor(sys_w() * 0.473));
	///else
	array_push(new_layout, show2d ? math_floor((sys_w() + raw.layout[layout_size_t.NODES_W]) * 0.515) : math_floor(sys_w() * 0.515)); // Align with ui header controls
	///end

	array_push(new_layout, math_floor(sys_h() / 2)); // LayoutNodesH
	array_push(new_layout, math_floor(ui_statusbar_default_h * raw.window_scale)); // LayoutStatusH

	///if (arm_android || arm_ios)
	array_push(new_layout, 0); // LayoutHeader
	///else
	array_push(new_layout, 1);
	///end

	raw.layout_tabs = [
		0,
		0,
		0
	];

	raw.layout = new_layout;
}

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
	let a0: tab_draw_array_t = [
		_draw_callback_create(tab_layers_draw),
		_draw_callback_create(tab_history_draw),
		_draw_callback_create(tab_plugins_draw)
	];
	let a1: tab_draw_array_t = [
		_draw_callback_create(tab_materials_draw),
		_draw_callback_create(tab_brushes_draw),
		_draw_callback_create(tab_scripts_draw)

	];
	let a2: tab_draw_array_t = [
		_draw_callback_create(tab_browser_draw),
		_draw_callback_create(tab_meshes_draw),
		_draw_callback_create(tab_textures_draw),
		_draw_callback_create(tab_fonts_draw),
		_draw_callback_create(tab_swatches_draw),
		_draw_callback_create(tab_console_draw),
		_draw_callback_create(ui_statusbar_draw_version_tab)
	];

    let r: tab_draw_array_t[] = [];
	array_push(r, a0);
	array_push(r, a1);
	array_push(r, a2);
	return r;
}

function ui_base_init() {
	ui_toolbar_init();
	context_raw.text_tool_text = tr("Text");
	ui_header_init();
	ui_statusbar_init();
	ui_menubar_init();

	ui_header_h = math_floor(ui_header_default_h * config_raw.window_scale);
	ui_menubar_w = math_floor(ui_menubar_default_w * config_raw.window_scale);

	if (context_raw.empty_envmap == null) {
		ui_base_make_empty_envmap(base_theme.VIEWPORT_COL);
	}
	if (context_raw.preview_envmap == null) {
		let b: u8_array_t = u8_array_create(4);
		b[0] = 0;
		b[1] = 0;
		b[2] = 0;
		b[3] = 255;
		context_raw.preview_envmap = gpu_create_texture_from_bytes(b, 1, 1);
	}

	if (context_raw.saved_envmap == null) {
		// raw.saved_envmap = scene_world._envmap;
		context_raw.default_irradiance = scene_world._.irradiance;
		context_raw.default_radiance = scene_world._.radiance;
		context_raw.default_radiance_mipmaps = scene_world._.radiance_mipmaps;
	}
	scene_world._.envmap = context_raw.show_envmap ? context_raw.saved_envmap : context_raw.empty_envmap;
	context_raw.ddirty = 1;

	let resources: string[] = ["cursor.k", "icons.k"];
	resource_load(resources);

	let scale: f32 = config_raw.window_scale;
	let ops: ui_options_t = {
		theme: base_theme,
		font: base_font,
		scale_factor: scale,
		color_wheel: base_color_wheel,
		black_white_gradient: base_color_wheel_gradient
	};
	ui = ui_create(ops);
	ui_on_border_hover = ui_base_on_border_hover;
	ui_on_tab_drop = ui_base_on_tab_drop;
	if (UI_SCALE() > 1) {
		ui_base_set_icon_scale();
	}

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

	project_new(false);

	if (project_filepath == "") {
		sys_notify_on_next_frame(layers_init);
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

	if (!ui.is_typing) {
		if (operator_shortcut(map_get(config_keymap, "toggle_node_editor"))) {
			ui_nodes_canvas_type == canvas_type_t.MATERIAL ? ui_base_show_material_nodes() : ui_base_show_brush_nodes();
		}
		else if (operator_shortcut(map_get(config_keymap, "toggle_browser"))) {
			ui_base_toggle_browser();
		}

		else if (operator_shortcut(map_get(config_keymap, "toggle_2d_view"))) {
			ui_base_show_2d_view(view_2d_type_t.LAYER);
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
			sys_notify_on_next_frame(function () {
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

	let is_typing: bool = ui.is_typing;
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

	// Viewport shortcuts
	if (context_in_paint_area() && !is_typing) {

		if (!mouse_down("right")) { // Fly mode off
			if (operator_shortcut(map_get(config_keymap, "tool_brush"))) {
				context_select_tool(tool_type_t.BRUSH);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_eraser"))) {
				context_select_tool(tool_type_t.ERASER);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_fill"))) {
				context_select_tool(tool_type_t.FILL);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_colorid"))) {
				context_select_tool(tool_type_t.COLORID);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_decal"))) {
				context_select_tool(tool_type_t.DECAL);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_text"))) {
				context_select_tool(tool_type_t.TEXT);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_clone"))) {
				context_select_tool(tool_type_t.CLONE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_blur"))) {
				context_select_tool(tool_type_t.BLUR);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_smudge"))) {
				context_select_tool(tool_type_t.SMUDGE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_particle"))) {
				context_select_tool(tool_type_t.PARTICLE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_picker"))) {
				context_select_tool(tool_type_t.PICKER);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_bake"))) {
				context_select_tool(tool_type_t.BAKE);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_gizmo"))) {
				context_select_tool(tool_type_t.GIZMO);
			}
			else if (operator_shortcut(map_get(config_keymap, "tool_material"))) {
				context_select_tool(tool_type_t.MATERIAL);
			}
			else if (operator_shortcut(map_get(config_keymap, "swap_brush_eraser"))) {
				context_select_tool(context_raw.tool == tool_type_t.BRUSH ? tool_type_t.ERASER : tool_type_t.BRUSH);
			}
		}

		// Radius
		if (context_raw.tool == tool_type_t.BRUSH  ||
			context_raw.tool == tool_type_t.ERASER ||
			context_raw.tool == tool_type_t.DECAL  ||
			context_raw.tool == tool_type_t.TEXT   ||
			context_raw.tool == tool_type_t.CLONE  ||
			context_raw.tool == tool_type_t.BLUR   ||
			context_raw.tool == tool_type_t.SMUDGE   ||
			context_raw.tool == tool_type_t.PARTICLE) {
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

		// Viewpoint
		if (mouse_view_x() < sys_w()) {
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
				ui.is_key_pressed = false;
				ui_menu_draw(function () {
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
						tr("Emission"),
						tr("Subsurface"),
						tr("TexCoord"),
						tr("Object Normal"),
						tr("Material ID"),
						tr("Object ID"),
						tr("Mask")
					];

					let shortcuts: string[] = ["l", "b", "n", "o", "r", "m", "a", "h", "e", "s", "t", "1", "2", "3", "4"];

					if (gpu_raytrace_supported()) {
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

		let b: bool = (context_raw.brush_can_lock || context_raw.brush_locked) &&
			!operator_shortcut(map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN) &&
			!operator_shortcut(map_get(config_keymap, "brush_opacity"), shortcut_type_t.DOWN) &&
			!operator_shortcut(map_get(config_keymap, "brush_angle"), shortcut_type_t.DOWN) &&
			!(decal_mask && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "brush_radius"), shortcut_type_t.DOWN));

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

	// Resizing
	if (ui_base_border_handle != null) {
		if (ui_base_border_handle == ui_nodes_hwnd || ui_base_border_handle == ui_view2d_hwnd) {
			if (ui_base_border_started == border_side_t.LEFT) {
				config_raw.layout[layout_size_t.NODES_W] -= math_floor(mouse_movement_x);
				if (config_raw.layout[layout_size_t.NODES_W] < 32) {
					config_raw.layout[layout_size_t.NODES_W] = 32;
				}
				else if (config_raw.layout[layout_size_t.NODES_W] > iron_window_width() * 0.7) {
					config_raw.layout[layout_size_t.NODES_W] = math_floor(iron_window_width() * 0.7);
				}
			}
			else { // UINodes / UIView2D ratio
				config_raw.layout[layout_size_t.NODES_H] -= math_floor(mouse_movement_y);
				if (config_raw.layout[layout_size_t.NODES_H] < 32) {
					config_raw.layout[layout_size_t.NODES_H] = 32;
				}
				else if (config_raw.layout[layout_size_t.NODES_H] > sys_h() * 0.95) {
					config_raw.layout[layout_size_t.NODES_H] = math_floor(sys_h() * 0.95);
				}
			}
		}
		else if (ui_base_border_handle == ui_base_hwnds[tab_area_t.STATUS]) {
			let my: i32 = math_floor(mouse_movement_y);
			if (config_raw.layout[layout_size_t.STATUS_H] - my >= ui_statusbar_default_h * config_raw.window_scale && config_raw.layout[layout_size_t.STATUS_H] - my < iron_window_height() * 0.7) {
				config_raw.layout[layout_size_t.STATUS_H] -= my;
			}
		}
		else {
			if (ui_base_border_started == border_side_t.LEFT) {
				config_raw.layout[layout_size_t.SIDEBAR_W] -= math_floor(mouse_movement_x);
				if (config_raw.layout[layout_size_t.SIDEBAR_W] < ui_sidebar_w_mini) {
					config_raw.layout[layout_size_t.SIDEBAR_W] = ui_sidebar_w_mini;
				}
				else if (config_raw.layout[layout_size_t.SIDEBAR_W] > iron_window_width() - ui_sidebar_w_mini) {
					config_raw.layout[layout_size_t.SIDEBAR_W] = iron_window_width() - ui_sidebar_w_mini;
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

	if (context_raw.tool == tool_type_t.PARTICLE && context_in_paint_area() && !context_raw.paint2d) {
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
			let mo: mesh_object_t = o.ext;
			mo.base.name = ".Bullet";
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
}

function ui_base_view_top() {
	let is_typing: bool = ui.is_typing;

	if (context_in_paint_area() && !is_typing) {
		if (mouse_view_x() < sys_w()) {
			viewport_set_view(0, 0, 1, 0, 0, 0);
		}
	}
}

function ui_base_operator_search() {
	_ui_base_operator_search_first = true;

	ui_menu_draw(function () {
		ui_menu_h = UI_ELEMENT_H() * 8;
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
		console_message_timer -= sys_delta();
		if (console_message_timer <= 0) {
			ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
		}
	}

	ui_sidebar_w_mini = math_floor(ui_sidebar_default_w_mini * UI_SCALE());

	if (!base_ui_enabled) {
		return;
	}

	// Same mapping for paint and rotate (predefined in touch keymap)
	if (context_in_viewport()) {
		let paint_key: string = map_get(config_keymap, "action_paint");
		let rotate_key: string = map_get(config_keymap, "action_rotate");
		if (mouse_started() && paint_key == rotate_key) {
			ui_base_action_paint_remap = paint_key;
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

	let set_clone_source: bool = context_raw.tool == tool_type_t.CLONE && operator_shortcut(map_get(config_keymap, "set_clone_source") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);

	let decal: bool = context_is_decal();
	let decal_mask: bool = context_is_decal_mask_paint();

	let down: bool = operator_shortcut(map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
					 decal_mask ||
					 set_clone_source ||
					 operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
					 (pen_down() && !keyboard_down("alt"));

	if (config_raw.touch_ui) {
		if (pen_down()) {
			context_raw.pen_painting_only = true;
		}
		else if (context_raw.pen_painting_only) {
			down = false;
		}
	}

	if (context_raw.tool == tool_type_t.PARTICLE) {
		down = false;
	}

	if (down) {
		let mx: i32 = mouse_view_x();
		let my: i32 = mouse_view_y();
		let ww: i32 = sys_w();

		if (context_raw.paint2d) {
			mx -= sys_w();
			ww = ui_view2d_ww;
		}

		if (mx < ww &&
			mx > sys_x() &&
			my < sys_h() &&
			my > sys_y()) {

			if (set_clone_source) {
				context_raw.clone_start_x = mx;
				context_raw.clone_start_y = my;
			}
			else {
				if (context_raw.brush_time == 0 &&
					!base_is_dragging &&
					!base_is_resizing &&
					ui.combo_selected_handle == null) { // Paint started

					// Draw line
					if (operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN)) {
						context_raw.last_paint_vec_x = context_raw.last_paint_x;
						context_raw.last_paint_vec_y = context_raw.last_paint_y;
					}

					history_push_undo = true;

					if (context_raw.tool == tool_type_t.CLONE && context_raw.clone_start_x >= 0.0) { // Clone delta
						context_raw.clone_delta_x = (context_raw.clone_start_x - mx) / ww;
						context_raw.clone_delta_y = (context_raw.clone_start_y - my) / sys_h();
						context_raw.clone_start_x = -1;
					}
					else if (context_raw.tool == tool_type_t.FILL && context_raw.fill_type_handle.position == fill_type_t.UV_ISLAND) {
						util_uv_uvislandmap_cached = false;
					}
				}

				context_raw.brush_time += sys_delta();

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
		if (context_raw.tool == tool_type_t.COLORID && context_raw.layer.fill_layer != null) {
			sys_notify_on_next_frame(function () {
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

			let target: gpu_texture_t = l.texpaint_preview;
			if (target == null) {
				continue;
			}

			let source: gpu_texture_t = l.texpaint;
			draw_begin(target, true, 0x00000000);
			// draw_set_pipeline(l.is_mask() ? pipes_copy8 : pipes_copy);
			draw_set_pipeline(pipes_copy); // texpaint_preview is always RGBA32 for now
			draw_scaled_image(source, 0, 0, target.width, target.height);
			draw_set_pipeline(null);
			draw_end();
		}
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	}
	if (context_raw.layer != null && context_raw.layer_preview_dirty && !slot_layer_is_group(context_raw.layer)) {
		context_raw.layer_preview_dirty = false;
		context_raw.mask_preview_last = null;
		// Update layer preview
		let l: slot_layer_t = context_raw.layer;

		let target: gpu_texture_t = l.texpaint_preview;
		if (target != null) {

			let source: gpu_texture_t = l.texpaint;
			draw_begin(target, true, 0x00000000);
			// draw_set_pipeline(raw.layer.is_mask() ? pipes_copy8 : pipes_copy);
			draw_set_pipeline(pipes_copy); // texpaint_preview is always RGBA32 for now
			draw_scaled_image(source, 0, 0, target.width, target.height);
			draw_set_pipeline(null);
			draw_end();
			ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		}
	}

	let undo_pressed: bool = operator_shortcut(map_get(config_keymap, "edit_undo"));
	let redo_pressed: bool = operator_shortcut(map_get(config_keymap, "edit_redo")) ||
							 (keyboard_down("control") && keyboard_started("y"));

	// Two-finger tap to undo, three-finger tap to redo
	if (context_in_viewport() && config_raw.touch_ui) {
		if (mouse_started("middle")) {
			ui_base_redo_tap_time = sys_time();
		}
		else if (mouse_started("right")) {
			ui_base_undo_tap_time = sys_time();
		}
		else if (mouse_released("middle") && sys_time() - ui_base_redo_tap_time < 0.1) {
			ui_base_redo_tap_time = ui_base_undo_tap_time = 0;
			redo_pressed = true;
		}
		else if (mouse_released("right") && sys_time() - ui_base_undo_tap_time < 0.1) {
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

	gizmo_update();
}

function ui_base_render() {
	if (!ui_base_show && config_raw.touch_ui) {
		ui.input_enabled = true;
		ui_begin(ui);
		if (ui_window(ui_handle(__ID__), 0, 0, 150, math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET() + 1))) {
			if (ui_button(tr("Close"))) {
				ui_base_toggle_distract_free();
			}
		}
		ui_end();
	}

	if (!ui_base_show) {
		return;
	}

	ui.input_enabled = base_ui_enabled;

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

	ui_begin(ui);
	ui_toolbar_render_ui();
	ui_menubar_render_ui();
	ui_header_render_ui();
	ui_statusbar_render_ui();
	ui_sidebar_render_ui();
	ui_end();

	ui.input_enabled = true;
}

function ui_base_render_cursor() {
	if (!base_ui_enabled) {
		return;
	}

	if (context_raw.tool == tool_type_t.MATERIAL || context_raw.tool == tool_type_t.BAKE) {
		return;
	}

	draw_begin();
	draw_set_color(0xffffffff);

	context_raw.view_index = context_raw.view_index_last;
	let mx: i32 = base_x() + context_raw.paint_vec.x * base_w();
	let my: i32 = base_y() + context_raw.paint_vec.y * base_h();
	context_raw.view_index = -1;

	// Radius being scaled
	if (context_raw.brush_locked) {
		mx += context_raw.lock_started_x - iron_window_width() / 2;
		my += context_raw.lock_started_y - iron_window_height() / 2;
	}

	if (context_raw.brush_stencil_image != null &&
		context_raw.tool != tool_type_t.BAKE &&
		context_raw.tool != tool_type_t.PICKER &&
		context_raw.tool != tool_type_t.MATERIAL &&
		context_raw.tool != tool_type_t.COLORID) {
		let r: rect_t = ui_base_get_brush_stencil_rect();
		if (!operator_shortcut(map_get(config_keymap, "stencil_hide"), shortcut_type_t.DOWN)) {
			draw_set_color(0x88ffffff);
			let angle: f32 = context_raw.brush_stencil_angle;
			draw_set_transform(mat3_multmat(mat3_multmat(mat3_translation(0.5, 0.5), mat3_rotation(-angle)), mat3_translation(-0.5, -0.5)));
			draw_scaled_image(context_raw.brush_stencil_image, r.x, r.y, r.w, r.h);
			draw_set_transform(mat3_nan());
			draw_set_color(0xffffffff);
		}
		let transform: bool = operator_shortcut(map_get(config_keymap, "stencil_transform"), shortcut_type_t.DOWN);
		if (transform) {
			// Outline
			draw_rect(r.x, r.y, r.w, r.h);
			// Scale
			draw_rect(r.x - 8,       r.y - 8,       16, 16);
			draw_rect(r.x - 8 + r.w, r.y - 8,       16, 16);
			draw_rect(r.x - 8,       r.y - 8 + r.h, 16, 16);
			draw_rect(r.x - 8 + r.w, r.y - 8 + r.h, 16, 16);
			// Rotate
			let cosa: f32 = math_cos(-context_raw.brush_stencil_angle);
			let sina: f32 = math_sin(-context_raw.brush_stencil_angle);
			let ox: f32 = 0;
			let oy: f32 = -r.h / 2;
			let x: f32 = ox * cosa - oy * sina;
			let y: f32 = ox * sina + oy * cosa;
			x += r.x + r.w / 2;
			y += r.y + r.h / 2;
			draw_filled_circle(x, y, 8);
		}
	}

	// Show picked material next to cursor
	if (context_raw.tool == tool_type_t.PICKER && context_raw.picker_select_material && context_raw.color_picker_callback == null) {
		let img: gpu_texture_t = context_raw.material.image_icon;
		draw_image(img, mx + 10, my + 10);
	}
	if (context_raw.tool == tool_type_t.PICKER && context_raw.color_picker_callback != null) {
		let img: gpu_texture_t = resource_get("icons.k");
		let rect: rect_t = resource_tile50(img, tool_type_t.PICKER, 0);
		draw_sub_image(img, mx + 10, my + 10, rect.x, rect.y, rect.w, rect.h);
	}

	let cursor_img: gpu_texture_t = resource_get("cursor.k");
	let psize: i32 = math_floor(182 * (context_raw.brush_radius * context_raw.brush_nodes_radius) * UI_SCALE());

	// Clone source cursor
	if (context_raw.tool == tool_type_t.CLONE && !keyboard_down("alt") && (mouse_down() || pen_down())) {
		draw_set_color(0x66ffffff);
		draw_scaled_image(cursor_img, mx + context_raw.clone_delta_x * sys_w() - psize / 2, my + context_raw.clone_delta_y * sys_h() - psize / 2, psize, psize);
		draw_set_color(0xffffffff);
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
					context_raw.decal_x += (context_raw.lock_started_x - iron_window_width() / 2) / base_w();
					context_raw.decal_y += (context_raw.lock_started_y - iron_window_height() / 2) / base_h();
				}
			}

			if (!config_raw.brush_live) {
				let psizex: i32 = math_floor(256 * UI_SCALE() * (context_raw.brush_radius * context_raw.brush_nodes_radius * context_raw.brush_scale_x));
				let psizey: i32 = math_floor(256 * UI_SCALE() * (context_raw.brush_radius * context_raw.brush_nodes_radius));

				context_raw.view_index = context_raw.view_index_last;
				let decalx: f32 = base_x() + context_raw.decal_x * base_w() - psizex / 2;
				let decaly: f32 = base_y() + context_raw.decal_y * base_h() - psizey / 2;
				context_raw.view_index = -1;

				draw_set_color(color_from_floats(1, 1, 1, decal_alpha));
				let angle: f32 = (context_raw.brush_angle + context_raw.brush_nodes_angle) * (math_pi() / 180);
				draw_set_transform(mat3_multmat(mat3_multmat(mat3_translation(0.5, 0.5), mat3_rotation(angle)), mat3_translation(-0.5, -0.5)));
				draw_scaled_image(context_raw.decal_image, decalx, decaly, psizex, psizey);
				draw_set_transform(mat3_nan());
				draw_set_color(0xffffffff);
			}
		}
		if (context_raw.tool == tool_type_t.BRUSH  ||
			context_raw.tool == tool_type_t.ERASER ||
			context_raw.tool == tool_type_t.CLONE  ||
			context_raw.tool == tool_type_t.BLUR   ||
			context_raw.tool == tool_type_t.SMUDGE   ||
			context_raw.tool == tool_type_t.PARTICLE ||
			(decal_mask && !config_raw.brush_3d) ||
			(decal_mask && context_in_2d_view())) {
			if (decal_mask) {
				psize = math_floor(cursor_img.width * (context_raw.brush_decal_mask_radius * context_raw.brush_nodes_radius) * UI_SCALE());
			}
			if (config_raw.brush_3d && context_in_2d_view()) {
				psize = math_floor(psize * ui_view2d_pan_scale);
			}
			draw_scaled_image(cursor_img, mx - psize / 2, my - psize / 2, psize, psize);
		}
	}

	if (context_raw.brush_lazy_radius > 0 && !context_raw.brush_locked &&
		(context_raw.tool == tool_type_t.BRUSH ||
			context_raw.tool == tool_type_t.ERASER ||
			context_raw.tool == tool_type_t.DECAL ||
			context_raw.tool == tool_type_t.TEXT ||
			context_raw.tool == tool_type_t.CLONE ||
			context_raw.tool == tool_type_t.BLUR ||
			context_raw.tool == tool_type_t.SMUDGE ||
			context_raw.tool == tool_type_t.PARTICLE)) {
		draw_filled_rect(mx - 1, my - 1, 2, 2);
		mx = context_raw.brush_lazy_x * base_w() + base_x();
		my = context_raw.brush_lazy_y * base_h() + base_y();
		let radius: f32 = context_raw.brush_lazy_radius * 180;
		draw_set_color(0xff666666);
		draw_scaled_image(cursor_img, mx - radius / 2, my - radius / 2, radius, radius);
		draw_set_color(0xffffffff);
	}
	draw_end();
}

function ui_base_show_material_nodes() {
	// Clear input state as ui receives input events even when not drawn
	ui_end_input();

	ui_nodes_show = !ui_nodes_show || ui_nodes_canvas_type != canvas_type_t.MATERIAL;
	ui_nodes_canvas_type = canvas_type_t.MATERIAL;

	///if (arm_ios || arm_android)
	if (ui_view2d_show) {
		ui_view2d_show = false;
	}
	///end

	base_resize();
}

function ui_base_show_brush_nodes() {
	// Clear input state as ui receives input events even when not drawn
	ui_end_input();
	ui_nodes_show = !ui_nodes_show || ui_nodes_canvas_type != canvas_type_t.BRUSH;
	ui_nodes_canvas_type = canvas_type_t.BRUSH;

	///if (arm_ios || arm_android)
	if (ui_view2d_show) {
		ui_view2d_show = false;
	}
	///end

	base_resize();
}

function ui_base_show_2d_view(type: view_2d_type_t) {
	// Clear input state as ui receives input events even when not drawn
	ui_end_input();
	if (ui_view2d_type != type) {
		ui_view2d_show = true;
	}
	else {
		ui_view2d_show = !ui_view2d_show;
	}
	ui_view2d_type = type;
	ui_view2d_hwnd.redraws = 2;

	///if (arm_ios || arm_android)
	if (ui_nodes_show) {
		ui_nodes_show = false;
	}
	///end

	base_resize();
}

function ui_base_toggle_browser() {
	let minimized: bool = config_raw.layout[layout_size_t.STATUS_H] <= (ui_statusbar_default_h * config_raw.window_scale);
	config_raw.layout[layout_size_t.STATUS_H] = minimized ? 240 : ui_statusbar_default_h;
	config_raw.layout[layout_size_t.STATUS_H] = math_floor(config_raw.layout[layout_size_t.STATUS_H] * config_raw.window_scale);
}

function ui_base_set_icon_scale() {
	if (UI_SCALE() > 1) {
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
	if (!base_ui_enabled) {
		return;
	}

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
		iron_mouse_set_cursor(cursor_t.SIZEWE) :
		iron_mouse_set_cursor(cursor_t.SIZENS);

	if (ui.input_started) {
		ui_base_border_started = side;
		ui_base_border_handle = handle;
		base_is_resizing = true;
	}
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
	context_raw.empty_envmap = gpu_create_texture_from_bytes(b, 1, 1);
}

function ui_base_set_viewport_col(col: i32) {
	ui_base_make_empty_envmap(col);
	context_raw.ddirty = 2;
	if (!context_raw.show_envmap) {
		scene_world._.envmap = context_raw.empty_envmap;
	}
}
