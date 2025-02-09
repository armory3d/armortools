
let base_ui_enabled: bool = true;
let base_is_dragging: bool = false;
let base_is_resizing: bool = false;
let base_drag_asset: asset_t = null;
let base_drag_swatch: swatch_color_t = null;
let base_drag_file: string = null;
let base_drag_file_icon: image_t = null;
let base_drag_tint: i32 = 0xffffffff;
let base_drag_size: i32 = -1;
let base_drag_rect: rect_t = null;
let base_drag_off_x: f32 = 0.0;
let base_drag_off_y: f32 = 0.0;
let base_drag_start: f32 = 0.0;
let base_drop_x: f32 = 0.0;
let base_drop_y: f32 = 0.0;
let base_font: g2_font_t = null;
let base_theme: ui_theme_t;
let base_color_wheel: image_t;
let base_color_wheel_gradient: image_t;
let base_ui_box: ui_t;
let base_ui_menu: ui_t;
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

function base_init() {
	base_last_window_width = sys_width();
	base_last_window_height = sys_height();

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
		}
	);

	iron_set_save_and_quit_callback(base_save_and_quit_callback);

	let font: g2_font_t = data_get_font("font.ttf");
	let image_color_wheel: image_t = data_get_image("color_wheel.k");
	let image_color_wheel_gradient: image_t = data_get_image("color_wheel_gradient.k");

	base_font = font;
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
		base_font.font_ = kinc_g2_font_13(base_font.blob);
		base_font.glyphs = _g2_font_glyphs;
	}
	else {
		g2_font_init(base_font);
	}

	base_color_wheel = image_color_wheel;
	base_color_wheel_gradient = image_color_wheel_gradient;
	ui_nodes_enum_texts = base_enum_texts;
	let ops: ui_options_t = {
		theme: base_theme,
		font: font,
		scale_factor: config_raw.window_scale,
		color_wheel: base_color_wheel.texture_,
		black_white_gradient: base_color_wheel_gradient.texture_
	};
	base_ui_box = ui_create(ops);
	base_ui_menu = ui_create(ops);

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
	base_ext_init();

	app_notify_on_update(base_update);
	app_notify_on_render_2d(ui_view2d_render);
	app_notify_on_update(ui_view2d_update);
	app_notify_on_render_2d(ui_base_render_cursor);
	app_notify_on_update(ui_nodes_update);
	app_notify_on_render_2d(ui_nodes_render);
	app_notify_on_update(ui_base_update);
	app_notify_on_render_2d(ui_base_render);
	app_notify_on_update(camera_update);
	app_notify_on_render_2d(base_render);

	base_appx = ui_toolbar_w;
	base_appy = ui_header_h;
	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		base_appy += ui_header_h;
	}
	let cam: camera_object_t = scene_camera;
	cam.data.fov = math_floor(cam.data.fov * 100) / 100;
	camera_object_build_proj(cam);

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
		sys_stop();
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
		let sidebarw: i32 = ui_base_default_sidebar_w;
		res = sys_width() - sidebarw - ui_toolbar_default_w;
	}
	else if (ui_nodes_show || ui_view2d_show) {
		res = sys_width() - config_raw.layout[layout_size_t.SIDEBAR_W] - config_raw.layout[layout_size_t.NODES_W] - ui_toolbar_w;
	}
	else if (ui_base_show) {
		res = sys_width() - config_raw.layout[layout_size_t.SIDEBAR_W] - ui_toolbar_w;
	}
	else { // Distract free
		res = sys_width();
	}
	if (context_raw.view_index > -1) {
		res = math_floor(res / 2);
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

	let res: i32 = sys_height();

	if (config_raw.layout == null) {
		res -= ui_header_default_h * 2 + ui_status_default_status_h;
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
	if (sys_width() == 0 || sys_height() == 0) {
		return;
	}

	let ratio_w: f32 = sys_width() / base_last_window_width;
	base_last_window_width = sys_width();
	let ratio_h: f32 = sys_height() / base_last_window_height;
	base_last_window_height = sys_height();

	config_raw.layout[layout_size_t.NODES_W] = math_floor(config_raw.layout[layout_size_t.NODES_W] * ratio_w);
	config_raw.layout[layout_size_t.SIDEBAR_H0] = math_floor(config_raw.layout[layout_size_t.SIDEBAR_H0] * ratio_h);
	config_raw.layout[layout_size_t.SIDEBAR_H1] = sys_height() - config_raw.layout[layout_size_t.SIDEBAR_H0];

	base_resize();
	base_save_window_rect();
}

function base_save_window_rect() {
	config_raw.window_w = sys_width();
	config_raw.window_h = sys_height();
	config_raw.window_x = sys_x();
	config_raw.window_y = sys_y();
	config_save();
}

function base_resize() {
	if (sys_width() == 0 || sys_height() == 0) {
		return;
	}

	let cam: camera_object_t = scene_camera;
	if (cam.data.ortho != null) {
		cam.data.ortho[2] = -2 * (app_h() / app_w());
		cam.data.ortho[3] =  2 * (app_h() / app_w());
	}
	camera_object_build_proj(cam);

	if (context_raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
		viewport_update_camera_type(context_raw.camera_type);
	}

	context_raw.ddirty = 2;

	if (ui_base_show) {
		base_appx = ui_toolbar_w;
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
		iron_set_mouse_cursor(0); // Arrow
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
				base_drag_start += time_real_delta();
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
					let image: image_t = project_get_image(base_drag_asset);
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

				///if (is_paint || is_sculpt)
				_base_material_count = project_materials.length;
				import_asset_run(base_drag_file, base_drop_x, base_drop_y, true, true, function () {
					// Asset was material
					if (project_materials.length > _base_material_count) {
						base_drag_material = context_raw.material;
						base_material_dropped();
					}
				});
				///end

				///if is_lab
				import_asset_run(base_drag_file, base_drop_x, base_drop_y);
				///end
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

		iron_set_mouse_cursor(0); // Arrow
		base_is_dragging = false;
	}
	if (context_raw.color_picker_callback != null && (mouse_released() || mouse_released("right"))) {
		context_raw.color_picker_callback = null;
		context_select_tool(context_raw.color_picker_previous_tool);
	}

	base_handle_drop_paths();

	///if arm_windows
	let is_picker: bool = context_is_picker();
	let decal: bool = context_is_decal();
	ui_always_redraw_window = !context_raw.cache_draws ||
							  ui_menu_show ||
							  ui_box_show ||
							  base_is_dragging ||
							  is_picker ||
							  decal ||
							  ui_view2d_show ||
							  !config_raw.brush_3d ||
							  context_raw.frame < 3;
	///end

	if (ui_always_redraw_window && context_raw.ddirty < 0) {
		context_raw.ddirty = 0;
	}

	base_ext_update();
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
	let icons: image_t = resource_get("icons.k");
	if (base_drag_layer != null && !slot_layer_is_group(base_drag_layer) && base_drag_layer.fill_layer == null) {
		return resource_tile50(icons, 4, 1);
	}
	return null;
}

function base_get_drag_image(): image_t {
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
		let icons: image_t = resource_get("icons.k");
		base_drag_rect = string_index_of(base_drag_file, ".") > 0 ? resource_tile50(icons, 3, 1) : resource_tile50(icons, 2, 1);
		base_drag_tint = ui_base_ui.ops.theme.HIGHLIGHT_COL;
		return icons;
	}

	if (base_drag_material != null) {
		return base_drag_material.image_icon;
	}
	if (base_drag_layer != null && slot_layer_is_group(base_drag_layer)) {
		let icons: image_t = resource_get("icons.k");
		let folder_closed: rect_t = resource_tile50(icons, 2, 1);
		let folder_open: rect_t = resource_tile50(icons, 8, 1);
		base_drag_rect = base_drag_layer.show_panel ? folder_open : folder_closed;
		base_drag_tint = ui_base_ui.ops.theme.LABEL_COL - 0x00202020;
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
	if (sys_width() == 0 || sys_height() == 0) {
		return;
	}

	base_ext_render();

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
		iron_set_mouse_cursor(1); // Hand
		let img: image_t = base_get_drag_image();
		let scale_factor: f32 = ui_SCALE(ui_base_ui);
		let size: f32 = (base_drag_size == -1 ? 50 : base_drag_size) * scale_factor;
		let ratio: f32 = size / img.width;
		let h: f32 = img.height * ratio;
		let inv: i32 = 0;

		g2_set_color(base_drag_tint);

		let bg_rect: rect_t = base_get_drag_background();
		if (bg_rect != null) {
			g2_draw_scaled_sub_image(resource_get("icons.k"), bg_rect.x, bg_rect.y, bg_rect.w, bg_rect.h, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		}

		base_drag_rect == null ?
			g2_draw_scaled_image(img, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2) :
			g2_draw_scaled_sub_image(img, base_drag_rect.x, base_drag_rect.y, base_drag_rect.w, base_drag_rect.h, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		g2_set_color(0xffffffff);
	}

	let using_menu: bool = ui_menu_show && mouse_y > ui_header_h;
	base_ui_enabled = !ui_box_show && !using_menu && !base_is_combo_selected();
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
	if (sys_mode() == window_mode_t.WINDOWED) {
		config_raw.window_w = sys_width();
		config_raw.window_h = sys_height();
		config_raw.window_x = sys_x();
		config_raw.window_y = sys_y();
		sys_mode_set(window_mode_t.FULLSCREEN);
	}
	else {
		sys_mode_set(window_mode_t.WINDOWED);
		sys_resize(config_raw.window_w, config_raw.window_h);
		sys_move(config_raw.window_x, config_raw.window_y);
	}
}

function base_is_scrolling(): bool {
	for (let i: i32 = 0; i < base_get_uis().length; ++i) {
		let ui: ui_t = base_get_uis()[i];
		if (ui.is_scrolling) {
			return true;
		}
	}
	return false;
}

function base_is_combo_selected(): bool {
	for (let i: i32 = 0; i < base_get_uis().length; ++i) {
		let ui: ui_t = base_get_uis()[i];
		if (ui.combo_selected_handle != null) {
			return true;
		}
	}
	return false;
}

function base_get_uis(): ui_t[] {
	let uis: ui_t[] = [base_ui_box, base_ui_menu, ui_base_ui, ui_nodes_ui, ui_view2d_ui];
	return uis;
}

function base_is_decal_layer(): bool {
	///if (is_sculpt || is_lab)
	return false;
	///end

	let is_painting: bool = context_raw.tool != workspace_tool_t.MATERIAL && context_raw.tool != workspace_tool_t.BAKE;
	return is_painting && context_raw.layer.fill_layer != null && context_raw.layer.uv_type == uv_type_t.PROJECT;
}

function base_redraw_status() {
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
}

function base_redraw_console() {
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_base_ui != null && statush > ui_status_default_status_h * ui_SCALE(ui_base_ui)) {
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

	array_push(new_layout, math_floor(ui_base_default_sidebar_w * raw.window_scale)); // LayoutSidebarW
	array_push(new_layout, math_floor(sys_height() / 2)); // LayoutSidebarH0
	array_push(new_layout, math_floor(sys_height() / 2)); // LayoutSidebarH1

	///if arm_ios
	array_push(new_layout, show2d ? math_floor((app_w() + raw.layout[layout_size_t.NODES_W]) * 0.473) : math_floor(app_w() * 0.473)); // LayoutNodesW
	///elseif arm_android
	array_push(new_layout, show2d ? math_floor((app_w() + raw.layout[layout_size_t.NODES_W]) * 0.473) : math_floor(app_w() * 0.473));
	///else
	array_push(new_layout, show2d ? math_floor((app_w() + raw.layout[layout_size_t.NODES_W]) * 0.515) : math_floor(app_w() * 0.515)); // Align with ui header controls
	///end

	array_push(new_layout, math_floor(app_h() / 2)); // LayoutNodesH
	array_push(new_layout, math_floor(ui_status_default_status_h * raw.window_scale)); // LayoutStatusH

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

function base_init_config() {
	let raw: config_t = config_raw;
	raw.recent_projects = [];
	raw.bookmarks = [];
	raw.plugins = [];
	///if (arm_android || arm_ios)
	raw.keymap = "touch.json";
	///else
	raw.keymap = "default.json";
	///end
	raw.theme = "default.json";
	raw.server = "https://armorpaint.fra1.digitaloceanspaces.com";
	raw.undo_steps = 4;
	raw.pressure_radius = true;
	raw.pressure_sensitivity = 1.0;
	raw.camera_zoom_speed = 1.0;
	raw.camera_pan_speed = 1.0;
	raw.camera_rotation_speed = 1.0;
	raw.zoom_direction = zoom_direction_t.VERTICAL;
	raw.displace_strength = 0.0;
	raw.wrap_mouse = false;
	raw.workspace = space_type_t.SPACE3D;
	///if (arm_android || arm_ios)
	raw.camera_controls = camera_controls_t.ROTATE;
	///else
	raw.camera_controls = camera_controls_t.ORBIT;
	///end

	raw.layer_res = texture_res_t.RES2048;
	///if (arm_android || arm_ios)
	raw.touch_ui = true;
	raw.splash_screen = true;
	///else
	raw.touch_ui = false;
	raw.splash_screen = false;
	///end
	raw.node_preview = true;

	raw.pressure_hardness = true;
	raw.pressure_angle = false;
	raw.pressure_opacity = false;
	///if (arm_vulkan || arm_ios)
	raw.material_live = false;
	///else
	raw.material_live = true;
	///end
	raw.brush_3d = true;
	raw.brush_depth_reject = true;
	raw.brush_angle_reject = true;
	raw.brush_live = false;
	raw.show_asset_names = false;
	raw.dilate = dilate_type_t.INSTANT;
	raw.dilate_radius = 2;
	raw.gpu_inference = true;
	raw.blender = "";
	raw.atlas_res = 0;
	raw.pathtrace_mode = pathtrace_mode_t.FAST;
	raw.grid_snap = false;

	base_ext_init_config(raw);
}
