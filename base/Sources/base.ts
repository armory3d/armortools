
let base_ui_enabled: bool = true;
let base_is_dragging: bool = false;
let base_is_resizing: bool = false;
let base_drag_asset: asset_t = null;
let base_drag_swatch: swatch_color_t = null;
let base_drag_file: string = null;
let base_drag_file_icon: image_t = null;
let base_drag_tint = 0xffffffff;
let base_drag_size: i32 = -1;
let base_drag_rect: rect_t = null;
let base_drag_off_x: f32 = 0.0;
let base_drag_off_y: f32 = 0.0;
let base_drag_start: f32 = 0.0;
let base_drop_x: f32 = 0.0;
let base_drop_y: f32 = 0.0;
let base_font: g2_font_t = null;
let base_theme: theme_t;
let base_color_wheel: image_t;
let base_color_wheel_gradient: image_t;
let base_ui_box: zui_t;
let base_ui_menu: zui_t;
let base_default_element_w: i32 = 100;
let base_default_element_h: i32 = 28;
let base_default_font_size: i32 = 13;
let base_res_handle: zui_handle_t = zui_handle_create();
let base_bits_handle: zui_handle_t = zui_handle_create();
let base_drop_paths: string[] = [];
let base_appx: i32 = 0;
let base_appy: i32 = 0;
let base_last_window_width: i32 = 0;
let base_last_window_height: i32 = 0;
///if (is_paint || is_sculpt)
let base_drag_material: SlotMaterialRaw = null;
let base_drag_layer: SlotLayerRaw = null;
///end

let base_pipe_copy: pipeline_t;
let base_pipe_copy8: pipeline_t;
let base_pipe_copy128: pipeline_t;
let base_pipe_copy_bgra: pipeline_t;
let base_pipe_copy_rgb: pipeline_t = null;
///if (is_paint || is_sculpt)
let base_pipe_merge: pipeline_t = null;
let base_pipe_merge_r: pipeline_t = null;
let base_pipe_merge_g: pipeline_t = null;
let base_pipe_merge_b: pipeline_t = null;
let base_pipe_merge_a: pipeline_t = null;
let base_pipe_invert8: pipeline_t;
let base_pipe_apply_mask: pipeline_t;
let base_pipe_merge_mask: pipeline_t;
let base_pipe_colorid_to_mask: pipeline_t;
let base_tex0: kinc_tex_unit_t;
let base_tex1: kinc_tex_unit_t;
let base_texmask: kinc_tex_unit_t;
let base_texa: kinc_tex_unit_t;
let base_opac: kinc_const_loc_t;
let base_blending: kinc_const_loc_t;
let base_tex0_mask: kinc_tex_unit_t;
let base_texa_mask: kinc_tex_unit_t;
let base_tex0_merge_mask: kinc_tex_unit_t;
let base_texa_merge_mask: kinc_tex_unit_t;
let base_tex_colorid: kinc_tex_unit_t;
let base_texpaint_colorid: kinc_tex_unit_t;
let base_opac_merge_mask: kinc_const_loc_t;
let base_blending_merge_mask: kinc_const_loc_t;
let base_temp_mask_image: image_t = null;
///end
///if is_lab
let base_pipe_copy_r: pipeline_t;
let base_pipe_copy_g: pipeline_t;
let base_pipe_copy_b: pipeline_t;
let base_pipe_copy_a: pipeline_t;
let base_pipe_copy_a_tex: kinc_tex_unit_t;
let base_pipe_inpaint_preview: pipeline_t;
let base_tex0_inpaint_preview: kinc_tex_unit_t;
let base_texa_inpaint_preview: kinc_tex_unit_t;
///end
let base_temp_image: image_t = null;
let base_expa: image_t = null;
let base_expb: image_t = null;
let base_expc: image_t = null;
let base_pipe_cursor: pipeline_t;
let base_cursor_vp: kinc_const_loc_t;
let base_cursor_inv_vp: kinc_const_loc_t;
let base_cursor_mouse: kinc_const_loc_t;
let base_cursor_tex_step: kinc_const_loc_t;
let base_cursor_radius: kinc_const_loc_t;
let base_cursor_camera_right: kinc_const_loc_t;
let base_cursor_tint: kinc_const_loc_t;
let base_cursor_tex: kinc_tex_unit_t;
let base_cursor_gbufferd: kinc_tex_unit_t;

///if (is_paint || is_sculpt)
let base_default_base: f32 = 0.5;
let base_default_rough: f32 = 0.4;
///if (krom_android || krom_ios)
let base_max_layers: i32 = 18;
///else
let base_max_layers: i32 = 255;
///end
///end
let base_default_fov: f32 = 0.69;

let base_default_keymap: any = {
	action_paint: "left",
	action_rotate: "alt+left",
	action_pan: "alt+middle",
	action_zoom: "alt+right",
	rotate_light: "shift+middle",
	rotate_envmap: "ctrl+middle",
	set_clone_source: "alt",
	stencil_transform: "ctrl",
	stencil_hide: "z",
	brush_radius: "f",
	brush_radius_decrease: "[",
	brush_radius_increase: "]",
	brush_ruler: "shift",
	file_new: "ctrl+n",
	file_open: "ctrl+o",
	file_open_recent: "ctrl+shift+o",
	file_save: "ctrl+s",
	file_save_as: "ctrl+shift+s",
	file_reimport_mesh: "ctrl+r",
	file_reimport_textures: "ctrl+shift+r",
	file_import_assets: "ctrl+i",
	file_export_textures: "ctrl+e",
	file_export_textures_as: "ctrl+shift+e",
	edit_undo: "ctrl+z",
	edit_redo: "ctrl+shift+z",
	edit_prefs: "ctrl+k",
	view_reset: "0",
	view_front: "1",
	view_back: "ctrl+1",
	view_right: "3",
	view_left: "ctrl+3",
	view_top: "7",
	view_bottom: "ctrl+7",
	view_camera_type: "5",
	view_orbit_left: "4",
	view_orbit_right: "6",
	view_orbit_up: "8",
	view_orbit_down: "2",
	view_orbit_opposite: "9",
	view_zoom_in: "",
	view_zoom_out: "",
	view_distract_free: "f11",
	viewport_mode: "ctrl+m",
	toggle_node_editor: "tab",
	toggle_2d_view: "shift+tab",
	toggle_browser: "`",
	node_search: "space",
	operator_search: "space",
	///if (is_paint || is_sculpt)
	decal_mask: "ctrl",
	select_material: "shift+number",
	select_layer: "alt+number",
	brush_opacity: "shift+f",
	brush_angle: "alt+f",
	tool_brush: "b",
	tool_eraser: "e",
	tool_fill: "g",
	tool_decal: "d",
	tool_text: "t",
	tool_clone: "l",
	tool_blur: "u",
	tool_smudge: "m",
	tool_particle: "p",
	tool_colorid: "c",
	tool_picker: "v",
	tool_bake: "k",
	tool_gizmo: "",
	tool_material: "",
	swap_brush_eraser: "",
	///end
};

function base_init() {
	base_last_window_width = sys_width();
	base_last_window_height = sys_height();

	sys_notify_on_drop_files(function(drop_path: string) {
		///if krom_linux
		drop_path = decodeURIComponent(drop_path);
		///end
		drop_path = trim_end(drop_path);
		base_drop_paths.push(drop_path);
	});

	sys_notify_on_app_state(
		function() { // Foreground
			Context.raw.foreground_event = true;
			Context.raw.last_paint_x = -1;
			Context.raw.last_paint_y = -1;
		},
		function() {}, // Resume
		function() {}, // Pause
		function() { // Background
			// Release keys after alt-tab / win-tab
			keyboard_up_listener(key_code_t.ALT);
			keyboard_up_listener(key_code_t.WIN);
		},
		function() { // Shutdown
			///if (krom_android || krom_ios)
			Project.project_save();
			///end
		}
	);

	krom_set_save_and_quit_callback(base_save_and_quit_callback);

	let f: g2_font_t = data_get_font("font.ttf");
	let image_color_wheel: image_t = data_get_image("color_wheel.k");
	let image_color_wheel_gradient: image_t = data_get_image("color_wheel_gradient.k");

	base_font = f;
	Config.load_theme(Config.raw.theme, false);
	base_default_element_w = base_theme.ELEMENT_W;
	base_default_font_size = base_theme.FONT_SIZE;
	Translator.load_translations(Config.raw.locale);
	UIFiles.filename = tr("untitled");
	///if (krom_android || krom_ios)
	sys_title_set(tr("untitled"));
	///end

	// Baked font for fast startup
	if (Config.raw.locale == "en") {
		base_font.font_ = krom_g2_font_13(base_font.blob);
		base_font.glyphs = _g2_font_glyphs;
	}
	else g2_font_init(base_font);

	base_color_wheel = image_color_wheel;
	base_color_wheel_gradient = image_color_wheel_gradient;
	zui_set_enum_texts(base_enum_texts);
	zui_tr = tr;
	base_ui_box = zui_create({ theme: base_theme, font: f, scale_factor: Config.raw.window_scale, color_wheel: base_color_wheel, black_white_gradient: base_color_wheel_gradient });
	base_ui_menu = zui_create({ theme: base_theme, font: f, scale_factor: Config.raw.window_scale, color_wheel: base_color_wheel, black_white_gradient: base_color_wheel_gradient });
	base_default_element_h = base_ui_menu.t.ELEMENT_H;

	// Init plugins
	if (Config.raw.plugins != null) {
		for (let plugin of Config.raw.plugins) {
			Plugin.start(plugin);
		}
	}

	args_parse();

	new Camera();
	new UIBase();
	new UINodes();
	new UIView2D();

	///if is_lab
	RandomNode.setSeed(Math.floor(time_time() * 4294967295));
	///end

	app_notify_on_update(base_update);
	app_notify_on_render_2d(UIView2D.render);
	app_notify_on_update(UIView2D.update);
	///if (is_paint || is_sculpt)
	app_notify_on_render_2d(UIBase.render_cursor);
	///end
	app_notify_on_update(UINodes.update);
	app_notify_on_render_2d(UINodes.render);
	app_notify_on_update(UIBase.update);
	app_notify_on_render_2d(UIBase.render);
	app_notify_on_update(Camera.update);
	app_notify_on_render_2d(base_render);

	///if (is_paint || is_sculpt)
	base_appx = UIToolbar.toolbar_w;
	///end
	///if is_lab
	base_appx = 0;
	///end

	base_appy = UIHeader.headerh;
	if (Config.raw.layout[layout_size_t.HEADER] == 1) base_appy += UIHeader.headerh;
	let cam: camera_object_t = scene_camera;
	cam.data.fov = Math.floor(cam.data.fov * 100) / 100;
	camera_object_build_proj(cam);

	args_run();

	///if (krom_android || krom_ios)
	let has_projects: bool = Config.raw.recent_projects.length > 0;
	///else
	let has_projects: bool = true;
	///end

	if (Config.raw.splash_screen && has_projects) {
		BoxProjects.show();
	}
}

function base_save_and_quit_callback(save: bool) {
	base_save_window_rect();
	if (save) Project.project_save(true);
	else sys_stop();
}

///if (is_paint || is_sculpt)
function base_w(): i32 {
	// Drawing material preview
	if (Context.raw.material_preview) {
		return UtilRender.material_preview_size;
	}

	// Drawing decal preview
	if (Context.raw.decal_preview) {
		return UtilRender.decal_preview_size;
	}

	let res: i32 = 0;
	if (Config.raw.layout == null) {
		let sidebarw: i32 = UIBase.default_sidebar_w;
		res = sys_width() - sidebarw - UIToolbar.default_toolbar_w;
	}
	else if (UINodes.show || UIView2D.show) {
		res = sys_width() - Config.raw.layout[layout_size_t.SIDEBAR_W] - Config.raw.layout[layout_size_t.NODES_W] - UIToolbar.toolbar_w;
	}
	else if (UIBase.show) {
		res = sys_width() - Config.raw.layout[layout_size_t.SIDEBAR_W] - UIToolbar.toolbar_w;
	}
	else { // Distract free
		res = sys_width();
	}
	if (Context.raw.view_index > -1) {
		res = Math.floor(res / 2);
	}
	if (Context.raw.paint2d_view) {
		res = UIView2D.ww;
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}

function base_h(): i32 {
	// Drawing material preview
	if (Context.raw.material_preview) {
		return UtilRender.material_preview_size;
	}

	// Drawing decal preview
	if (Context.raw.decal_preview) {
		return UtilRender.decal_preview_size;
	}

	let res: i32 = sys_height();

	if (Config.raw.layout == null) {
		res -= UIHeader.default_header_h * 2 + UIStatus.default_status_h;

		///if (krom_android || krom_ios)
		let layout_header: i32 = 0;
		///else
		let layout_header: i32 = 1;
		///end
		if (layout_header == 0) {
			res += UIHeader.headerh;
		}
	}
	else if (UIBase.show && res > 0) {
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		res -= Math.floor(UIHeader.default_header_h * 2 * Config.raw.window_scale) + statush;

		if (Config.raw.layout[layout_size_t.HEADER] == 0) {
			res += UIHeader.headerh;
		}
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}
///end

///if is_lab
function base_w(): i32 {
	let res: i32 = 0;
	if (UINodes == null) {
		res = sys_width();
	}
	else if (UINodes.show || UIView2D.show) {
		res = sys_width() - Config.raw.layout[layout_size_t.NODES_W];
	}
	else { // Distract free
		res = sys_width();
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}

function base_h(): i32 {
	let res: i32 = sys_height();
	if (UIBase == null) {
		res -= UIHeader.default_header_h * 2 + UIStatus.default_status_h;
	}
	else if (res > 0) {
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		res -= Math.floor(UIHeader.default_header_h * 2 * Config.raw.window_scale) + statush;
	}

	return res > 0 ? res : 1; // App was minimized, force render path resize
}
///end

function base_x(): i32 {
	///if (is_paint || is_sculpt)
	return Context.raw.view_index == 1 ? base_appx + base_w() : base_appx;
	///end
	///if is_lab
	return base_appx;
	///end
}

function base_y(): i32 {
	return base_appy;
}

function base_on_resize() {
	if (sys_width() == 0 || sys_height() == 0) return;

	let ratio_w: f32 = sys_width() / base_last_window_width;
	base_last_window_width = sys_width();
	let ratio_h: f32 = sys_height() / base_last_window_height;
	base_last_window_height = sys_height();

	Config.raw.layout[layout_size_t.NODES_W] = Math.floor(Config.raw.layout[layout_size_t.NODES_W] * ratio_w);
	///if (is_paint || is_sculpt)
	Config.raw.layout[layout_size_t.SIDEBAR_H0] = Math.floor(Config.raw.layout[layout_size_t.SIDEBAR_H0] * ratio_h);
	Config.raw.layout[layout_size_t.SIDEBAR_H1] = sys_height() - Config.raw.layout[layout_size_t.SIDEBAR_H0];
	///end

	base_resize();

	///if (krom_linux || krom_darwin)
	base_save_window_rect();
	///end
}

function base_save_window_rect() {
	///if (krom_windows || krom_linux || krom_darwin)
	Config.raw.window_w = sys_width();
	Config.raw.window_h = sys_height();
	Config.raw.window_x = sys_x();
	Config.raw.window_y = sys_y();
	Config.save();
	///end
}

function base_resize() {
	if (sys_width() == 0 || sys_height() == 0) return;

	let cam: camera_object_t = scene_camera;
	if (cam.data.ortho != null) {
		cam.data.ortho[2] = -2 * (app_h() / app_w());
		cam.data.ortho[3] =  2 * (app_h() / app_w());
	}
	camera_object_build_proj(cam);

	if (Context.raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
		Viewport.update_camera_type(Context.raw.camera_type);
	}

	Context.raw.ddirty = 2;

	if (UIBase.show) {
		///if (is_paint || is_sculpt)
		base_appx = UIToolbar.toolbar_w;
		///end
		///if is_lab
		base_appx = 0;
		///end
		base_appy = UIHeader.headerh * 2;
		if (Config.raw.layout[layout_size_t.HEADER] == 0) {
			base_appy -= UIHeader.headerh;
		}
	}
	else {
		base_appx = 0;
		base_appy = 0;
	}

	if (UINodes.grid != null) {
		let _grid: image_t = UINodes.grid;
		let _next = function() {
			image_unload(_grid);
		}
		base_notify_on_next_frame(_next);
		UINodes.grid = null;
	}

	base_redraw_ui();
}

function base_redraw_ui() {
	UIHeader.header_handle.redraws = 2;
	UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
	UIMenubar.menu_handle.redraws = 2;
	UIMenubar.workspace_handle.redraws = 2;
	UINodes.hwnd.redraws = 2;
	UIBox.hwnd.redraws = 2;
	UIView2D.hwnd.redraws = 2;
	if (Context.raw.ddirty < 0) Context.raw.ddirty = 0; // Redraw viewport
	///if (is_paint || is_sculpt)
	UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	UIToolbar.toolbar_handle.redraws = 2;
	if (Context.raw.split_view) Context.raw.ddirty = 1;
	///end
}

function base_update() {
	if (mouse_movement_x != 0 || mouse_movement_y != 0) {
		krom_set_mouse_cursor(0); // Arrow
	}

	///if (is_paint || is_sculpt)
	let has_drag: bool = base_drag_asset != null || base_drag_material != null || base_drag_layer != null || base_drag_file != null || base_drag_swatch != null;
	///end
	///if is_lab
	let has_drag: bool = base_drag_asset != null || base_drag_file != null || base_drag_swatch != null;
	///end

	if (Config.raw.touch_ui) {
		// Touch and hold to activate dragging
		if (base_drag_start < 0.2) {
			if (has_drag && mouse_down()) base_drag_start += time_real_delta();
			else base_drag_start = 0;
			has_drag = false;
		}
		if (mouse_released()) {
			base_drag_start = 0;
		}
		let moved: bool = Math.abs(mouse_movement_x) > 1 && Math.abs(mouse_movement_y) > 1;
		if ((mouse_released() || moved) && !has_drag) {
			base_drag_asset = null;
			base_drag_swatch = null;
			base_drag_file = null;
			base_drag_file_icon = null;
			base_is_dragging = false;
			///if (is_paint || is_sculpt)
			base_drag_material = null;
			base_drag_layer = null;
			///end
		}
		// Disable touch scrolling while dragging is active
		zui_set_touch_scroll(!base_is_dragging);
	}

	if (has_drag && (mouse_movement_x != 0 || mouse_movement_y != 0)) {
		base_is_dragging = true;
	}
	if (mouse_released() && has_drag) {
		if (base_drag_asset != null) {
			if (Context.in_nodes()) { // Create image texture
				UINodes.accept_asset_drag(Project.assets.indexOf(base_drag_asset));
			}
			else if (Context.in_viewport()) {
				if (base_drag_asset.file.toLowerCase().endsWith(".hdr")) {
					let image: image_t = Project.get_image(base_drag_asset);
					ImportEnvmap.run(base_drag_asset.file, image);
				}
			}
			///if (is_paint || is_sculpt)
			else if (Context.in_layers() || Context.in_2d_view()) { // Create mask
				base_create_image_mask(base_drag_asset);
			}
			///end
			base_drag_asset = null;
		}
		else if (base_drag_swatch != null) {
			if (Context.in_nodes()) { // Create RGB node
				UINodes.accept_swatch_drag(base_drag_swatch);
			}
			else if (Context.in_swatches()) {
				TabSwatches.accept_swatch_drag(base_drag_swatch);
			}
			///if (is_paint || is_sculpt)
			else if (Context.in_materials()) {
				TabMaterials.accept_swatch_drag(base_drag_swatch);
			}
			else if (Context.in_viewport()) {
				let color: i32 = base_drag_swatch.base;
				color = color_set_ab(color, base_drag_swatch.opacity * 255);
				base_create_color_layer(color, base_drag_swatch.occlusion, base_drag_swatch.roughness, base_drag_swatch.metallic);
			}
			else if (Context.in_layers() && TabLayers.can_drop_new_layer(Context.raw.drag_dest)) {
				let color: i32 = base_drag_swatch.base;
				color = color_set_ab(color, base_drag_swatch.opacity * 255);
				base_create_color_layer(color, base_drag_swatch.occlusion, base_drag_swatch.roughness, base_drag_swatch.metallic, Context.raw.drag_dest);
			}
			///end

			base_drag_swatch = null;
		}
		else if (base_drag_file != null) {
			if (!Context.in_browser()) {
				base_drop_x = mouse_x;
				base_drop_y = mouse_y;

				///if (is_paint || is_sculpt)
				let material_count: i32 = Project.materials.length;
				ImportAsset.run(base_drag_file, base_drop_x, base_drop_y, true, true, function() {
					// Asset was material
					if (Project.materials.length > material_count) {
						base_drag_material = Context.raw.material;
						base_material_dropped();
					}
				});
				///end

				///if is_lab
				ImportAsset.run(base_drag_file, base_drop_x, base_drop_y);
				///end
			}
			base_drag_file = null;
			base_drag_file_icon = null;
		}
		///if (is_paint || is_sculpt)
		else if (base_drag_material != null) {
			base_material_dropped();
		}
		else if (base_drag_layer != null) {
			if (Context.in_nodes()) {
				UINodes.accept_layer_drag(Project.layers.indexOf(base_drag_layer));
			}
			else if (Context.in_layers() && base_is_dragging) {
				SlotLayer.move(base_drag_layer, Context.raw.drag_dest);
				MakeMaterial.parse_mesh_material();
			}
			base_drag_layer = null;
		}
		///end

		krom_set_mouse_cursor(0); // Arrow
		base_is_dragging = false;
	}
	if (Context.raw.color_picker_callback != null && (mouse_released() || mouse_released("right"))) {
		Context.raw.color_picker_callback = null;
		Context.select_tool(Context.raw.color_picker_previous_tool);
	}

	base_handle_drop_paths();

	///if (is_paint || is_sculpt)
	///if krom_windows
	let is_picker: bool = Context.raw.tool == workspace_tool_t.PICKER || Context.raw.tool == workspace_tool_t.MATERIAL;
	let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
	zui_set_always_redraw_window(!Context.raw.cache_draws ||
		UIMenu.show ||
		UIBox.show ||
		base_is_dragging ||
		is_picker ||
		decal ||
		UIView2D.show ||
		!Config.raw.brush_3d ||
		Context.raw.frame < 3);
	///end
	///end

	if (zui_always_redraw_window() && Context.raw.ddirty < 0) Context.raw.ddirty = 0;
}

///if (is_paint || is_sculpt)
function base_material_dropped() {
	// Material drag and dropped onto viewport or layers tab
	if (Context.in_viewport()) {
		let uv_type: uv_type_t = keyboard_down("control") ? uv_type_t.PROJECT : uv_type_t.UVMAP;
		let decal_mat: mat4_t = uv_type == uv_type_t.PROJECT ? UtilRender.get_decal_mat() : null;
		base_create_fill_layer(uv_type, decal_mat);
	}
	if (Context.in_layers() && TabLayers.can_drop_new_layer(Context.raw.drag_dest)) {
		let uv_type: uv_type_t = keyboard_down("control") ? uv_type_t.PROJECT : uv_type_t.UVMAP;
		let decal_mat: mat4_t = uv_type == uv_type_t.PROJECT ? UtilRender.get_decal_mat() : null;
		base_create_fill_layer(uv_type, decal_mat, Context.raw.drag_dest);
	}
	else if (Context.in_nodes()) {
		UINodes.accept_material_drag(Project.materials.indexOf(base_drag_material));
	}
	base_drag_material = null;
}
///end

function base_handle_drop_paths() {
	if (base_drop_paths.length > 0) {
		///if (krom_linux || krom_darwin)
		let wait: bool = !mouse_moved; // Mouse coords not updated during drag
		///else
		let wait: bool = false;
		///end
		if (!wait) {
			base_drop_x = mouse_x;
			base_drop_y = mouse_y;
			let drop_path: string = base_drop_paths.shift();
			ImportAsset.run(drop_path, base_drop_x, base_drop_y);
		}
	}
}

///if (is_paint || is_sculpt)
function base_get_drag_background(): rect_t {
	let icons: image_t = Res.get("icons.k");
	if (base_drag_layer != null && !SlotLayer.is_group(base_drag_layer) && base_drag_layer.fill_layer == null) {
		return Res.tile50(icons, 4, 1);
	}
	return null;
}
///end

function base_get_drag_image(): image_t {
	base_drag_tint = 0xffffffff;
	base_drag_size = -1;
	base_drag_rect = null;
	if (base_drag_asset != null) {
		return Project.get_image(base_drag_asset);
	}
	if (base_drag_swatch != null) {
		base_drag_tint = base_drag_swatch.base;
		base_drag_size = 26;
		return TabSwatches.empty;
	}
	if (base_drag_file != null) {
		if (base_drag_file_icon != null) return base_drag_file_icon;
		let icons: image_t = Res.get("icons.k");
		base_drag_rect = base_drag_file.indexOf(".") > 0 ? Res.tile50(icons, 3, 1) : Res.tile50(icons, 2, 1);
		base_drag_tint = UIBase.ui.t.HIGHLIGHT_COL;
		return icons;
	}

	///if is_paint
	if (base_drag_material != null) {
		return base_drag_material.image_icon;
	}
	if (base_drag_layer != null && SlotLayer.is_group(base_drag_layer)) {
		let icons: image_t = Res.get("icons.k");
		let folder_closed: rect_t = Res.tile50(icons, 2, 1);
		let folder_open: rect_t = Res.tile50(icons, 8, 1);
		base_drag_rect = base_drag_layer.show_panel ? folder_open : folder_closed;
		base_drag_tint = UIBase.ui.t.LABEL_COL - 0x00202020;
		return icons;
	}
	if (base_drag_layer != null && SlotLayer.is_mask(base_drag_layer) && base_drag_layer.fill_layer == null) {
		TabLayers.make_mask_preview_rgba32(base_drag_layer);
		return Context.raw.mask_preview_rgba32;
	}
	if (base_drag_layer != null) {
		return base_drag_layer.fill_layer != null ? base_drag_layer.fill_layer.image_icon : base_drag_layer.texpaint_preview;
	}
	///end

	return null;
}

function base_render() {
	if (sys_width() == 0 || sys_height() == 0) return;

	if (Context.raw.frame == 2) {
		///if (is_paint || is_sculpt)
		UtilRender.make_material_preview();
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		///end

		MakeMaterial.parse_mesh_material();
		MakeMaterial.parse_paint_material();
		Context.raw.ddirty = 0;

		///if (is_paint || is_sculpt)
		if (History.undo_layers == null) {
			History.undo_layers = [];
			for (let i: i32 = 0; i < Config.raw.undo_steps; ++i) {
				let l: SlotLayerRaw = SlotLayer.create("_undo" + History.undo_layers.length);
				History.undo_layers.push(l);
			}
		}
		///end

		// Default workspace
		if (Config.raw.workspace != 0) {
			UIHeader.worktab.position = Config.raw.workspace;
			UIMenubar.workspace_handle.redraws = 2;
			UIHeader.worktab.changed = true;
		}

		// Default camera controls
		Context.raw.camera_controls = Config.raw.camera_controls;

		///if is_lab
		base_notify_on_next_frame(function() {
			base_notify_on_next_frame(function() {
				TabMeshes.set_default_mesh(".Sphere");
			});
		});
		///end

		///if is_sculpt
		base_notify_on_next_frame(function() {
			base_notify_on_next_frame(function() {
				Context.raw.project_type = project_model_t.SPHERE;
				Project.project_new();
			});
		});
		///end
	}
	else if (Context.raw.frame == 3) {
		Context.raw.ddirty = 3;
	}
	Context.raw.frame++;

	if (base_is_dragging) {
		krom_set_mouse_cursor(1); // Hand
		let img: image_t = base_get_drag_image();

		///if (is_paint || is_sculpt)
		let scale_factor: f32 = zui_SCALE(UIBase.ui);
		///end
		///if is_lab
		let scale_factor: f32 = zui_SCALE(base_ui_box);
		///end

		let size: f32 = (base_drag_size == -1 ? 50 : base_drag_size) * scale_factor;
		let ratio: f32 = size / img.width;
		let h: f32 = img.height * ratio;

		///if (is_lab || krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		let inv: i32 = 0;
		///else
		let inv: i32 = (base_drag_material != null || (base_drag_layer != null && base_drag_layer.fill_layer != null)) ? h : 0;
		///end

		g2_set_color(base_drag_tint);

		///if (is_paint || is_sculpt)
		let bg_rect: rect_t = base_get_drag_background();
		if (bg_rect != null) {
			g2_draw_scaled_sub_image(Res.get("icons.k"), bg_rect.x, bg_rect.y, bg_rect.w, bg_rect.h, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		}
		///end

		base_drag_rect == null ?
			g2_draw_scaled_image(img, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2) :
			g2_draw_scaled_sub_image(img, base_drag_rect.x, base_drag_rect.y, base_drag_rect.w, base_drag_rect.h, mouse_x + base_drag_off_x, mouse_y + base_drag_off_y + inv, size, h - inv * 2);
		g2_set_color(0xffffffff);
	}

	let using_menu: bool = UIMenu.show && mouse_y > UIHeader.headerh;
	base_ui_enabled = !UIBox.show && !using_menu && !base_is_combo_selected();
	if (UIBox.show) UIBox.render();
	if (UIMenu.show) UIMenu.render();

	// Save last pos for continuos paint
	Context.raw.last_paint_vec_x = Context.raw.paint_vec.x;
	Context.raw.last_paint_vec_y = Context.raw.paint_vec.y;

	///if (krom_android || krom_ios)
	// No mouse move events for touch, re-init last paint position on touch start
	if (!mouse_down()) {
		Context.raw.last_paint_x = -1;
		Context.raw.last_paint_y = -1;
	}
	///end
}

function base_enum_texts(node_type: string): string[] {
	///if (is_paint || is_sculpt)
	if (node_type == "TEX_IMAGE") {
		return Project.asset_names.length > 0 ? Project.asset_names : [""];
	}
	if (node_type == "LAYER" || node_type == "LAYER_MASK") {
		let layer_names: string[] = [];
		for (let l of Project.layers) layer_names.push(l.name);
		return layer_names;
	}
	if (node_type == "MATERIAL") {
		let material_names: string[] = [];
		for (let m of Project.materials) material_names.push(m.canvas.name);
		return material_names;
	}
	///end

	///if is_lab
	if (node_type == "ImageTextureNode") {
		return Project.asset_names.length > 0 ? Project.asset_names : [""];
	}
	///end

	return null;
}

function base_get_asset_index(fileName: string): i32 {
	let i: i32 = Project.asset_names.indexOf(fileName);
	return i >= 0 ? i : 0;
}

function base_notify_on_next_frame(f: ()=>void) {
	let _render = function() {
		app_notify_on_init(function() {
			let _update = function() {
				app_notify_on_init(f);
				app_remove_update(_update);
			}
			app_notify_on_update(_update);
		});
		app_remove_render(_render);
	}
	app_notify_on_render(_render);
}

function base_toggle_fullscreen() {
	if (sys_mode() == window_mode_t.WINDOWED) {
		///if (krom_windows || krom_linux || krom_darwin)
		Config.raw.window_w = sys_width();
		Config.raw.window_h = sys_height();
		Config.raw.window_x = sys_x();
		Config.raw.window_y = sys_y();
		///end
		sys_mode_set(window_mode_t.FULLSCREEN);
	}
	else {
		sys_mode_set(window_mode_t.WINDOWED);
		sys_resize(Config.raw.window_w, Config.raw.window_h);
		sys_move(Config.raw.window_x, Config.raw.window_y);
	}
}

function base_is_scrolling(): bool {
	for (let ui of base_get_uis()) if (ui.is_scrolling) return true;
	return false;
}

function base_is_combo_selected(): bool {
	for (let ui of base_get_uis()) if (ui.combo_selected_handle_ptr != 0) return true;
	return false;
}

function base_get_uis(): zui_t[] {
	return [base_ui_box, base_ui_menu, UIBase.ui, UINodes.ui, UIView2D.ui];
}

function base_is_decal_layer(): bool {
	///if is_paint
	let is_paint: bool = Context.raw.tool != workspace_tool_t.MATERIAL && Context.raw.tool != workspace_tool_t.BAKE;
	return is_paint && Context.raw.layer.fill_layer != null && Context.raw.layer.uv_type == uv_type_t.PROJECT;
	///end

	///if (is_sculpt || is_lab)
	return false;
	///end
}

function base_redraw_status() {
	UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
}

function base_redraw_console() {
	let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
	if (UIBase.ui != null && statush > UIStatus.default_status_h * zui_SCALE(UIBase.ui)) {
		UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
	}
}

function base_init_layout() {
	let show2d: bool = (UINodes != null && UINodes.show) || (UIView2D != null && UIView2D.show);

	let raw: config_t = Config.raw;
	raw.layout = [
		///if (is_paint || is_sculpt)
		Math.floor(UIBase.default_sidebar_w * raw.window_scale), // LayoutSidebarW
		Math.floor(sys_height() / 2), // LayoutSidebarH0
		Math.floor(sys_height() / 2), // LayoutSidebarH1
		///end

		///if krom_ios
		show2d ? Math.floor((app_w() + raw.layout[layout_size_t.NODES_W]) * 0.473) : Math.floor(app_w() * 0.473), // LayoutNodesW
		///elseif krom_android
		show2d ? Math.floor((app_w() + raw.layout[layout_size_t.NODES_W]) * 0.473) : Math.floor(app_w() * 0.473),
		///else
		show2d ? Math.floor((app_w() + raw.layout[layout_size_t.NODES_W]) * 0.515) : Math.floor(app_w() * 0.515), // Align with ui header controls
		///end

		Math.floor(app_h() / 2), // LayoutNodesH
		Math.floor(UIStatus.default_status_h * raw.window_scale), // LayoutStatusH

		///if (krom_android || krom_ios)
		0, // LayoutHeader
		///else
		1,
		///end
	];

	raw.layout_tabs = [
		///if (is_paint || is_sculpt)
		0,
		0,
		///end
		0
	];
}

function base_init_config() {
	let raw: config_t = Config.raw;
	raw.recent_projects = [];
	raw.bookmarks = [];
	raw.plugins = [];
	///if (krom_android || krom_ios)
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
	///if (is_paint || is_sculpt)
	raw.displace_strength = 0.0;
	///else
	raw.displace_strength = 1.0;
	///end
	raw.wrap_mouse = false;
	///if is_paint
	raw.workspace = space_type_t.SPACE3D;
	///end
	///if is_sculpt
	raw.workspace = space_type_t.SPACE3D;
	///end
	///if is_lab
	raw.workspace = space_type_t.SPACE2D;
	///end
	///if (krom_android || krom_ios)
	raw.camera_controls = camera_controls_t.ROTATE;
	///else
	raw.camera_controls = camera_controls_t.ORBIT;
	///end
	raw.layer_res = texture_res_t.RES2048;
	///if (krom_android || krom_ios)
	raw.touch_ui = true;
	raw.splash_screen = true;
	///else
	raw.touch_ui = false;
	raw.splash_screen = false;
	///end
	///if (is_paint || is_sculpt)
	raw.node_preview = true;
	///else
	raw.node_preview = false;
	///end

	///if (is_paint || is_sculpt)
	raw.pressure_hardness = true;
	raw.pressure_angle = false;
	raw.pressure_opacity = false;
	///if (krom_vulkan || krom_ios)
	raw.material_live = false;
	///else
	raw.material_live = true;
	///end
	raw.brush_3d = true;
	raw.brush_depth_reject = true;
	raw.brush_angle_reject = true;
	raw.brush_live = false;
	raw.show_asset_names = false;
	///end

	///if is_paint
	raw.dilate = dilate_type_t.INSTANT;
	raw.dilate_radius = 2;
	///end

	///if is_lab
	raw.gpu_inference = true;
	///end
}

function base_init_layers() {
	///if (is_paint || is_sculpt)
	SlotLayer.clear(Project.layers[0], color_from_floats(base_default_base, base_default_base, base_default_base, 1.0));
	///end

	///if is_lab
	let texpaint: image_t = render_path_render_targets.get("texpaint")._image;
	let texpaint_nor: image_t = render_path_render_targets.get("texpaint_nor")._image;
	let texpaint_pack: image_t = render_path_render_targets.get("texpaint_pack")._image;
	g2_begin(texpaint);
	g2_draw_scaled_image(Res.get("placeholder.k"), 0, 0, Config.get_texture_res_x(), Config.get_texture_res_y()); // Base
	g2_end();
	g4_begin(texpaint_nor);
	g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
	g4_end();
	g4_begin(texpaint_pack);
	g4_clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
	g4_end();
	let texpaint_nor_empty: image_t = render_path_render_targets.get("texpaint_nor_empty")._image;
	let texpaint_pack_empty: image_t = render_path_render_targets.get("texpaint_pack_empty")._image;
	g4_begin(texpaint_nor_empty);
	g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
	g4_end();
	g4_begin(texpaint_pack_empty);
	g4_clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
	g4_end();
	///end
}

///if (is_paint || is_sculpt)
function base_resize_layers() {
	let conf: config_t = Config.raw;
	if (base_res_handle.position >= Math.floor(texture_res_t.RES16384)) { // Save memory for >=16k
		conf.undo_steps = 1;
		if (Context.raw.undo_handle != null) {
			Context.raw.undo_handle.value = conf.undo_steps;
		}
		while (History.undo_layers.length > conf.undo_steps) {
			let l: SlotLayerRaw = History.undo_layers.pop();
			base_notify_on_next_frame(function() {
				SlotLayer.unload(l);
			});
		}
	}
	for (let l of Project.layers) SlotLayer.resize_and_set_bits(l);
	for (let l of History.undo_layers) SlotLayer.resize_and_set_bits(l);
	let rts: map_t<string, render_target_t> = render_path_render_targets;
	let _texpaint_blend0: image_t = rts.get("texpaint_blend0")._image;
	base_notify_on_next_frame(function() {
		image_unload(_texpaint_blend0);
	});
	rts.get("texpaint_blend0").width = Config.get_texture_res_x();
	rts.get("texpaint_blend0").height = Config.get_texture_res_y();
	rts.get("texpaint_blend0")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8);
	let _texpaint_blend1: image_t = rts.get("texpaint_blend1")._image;
	base_notify_on_next_frame(function() {
		image_unload(_texpaint_blend1);
	});
	rts.get("texpaint_blend1").width = Config.get_texture_res_x();
	rts.get("texpaint_blend1").height = Config.get_texture_res_y();
	rts.get("texpaint_blend1")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8);
	Context.raw.brush_blend_dirty = true;
	if (rts.get("texpaint_blur") != null) {
		let _texpaint_blur: image_t = rts.get("texpaint_blur")._image;
		base_notify_on_next_frame(function() {
			image_unload(_texpaint_blur);
		});
		let size_x: f32 = Math.floor(Config.get_texture_res_x() * 0.95);
		let size_y: f32 = Math.floor(Config.get_texture_res_y() * 0.95);
		rts.get("texpaint_blur").width = size_x;
		rts.get("texpaint_blur").height = size_y;
		rts.get("texpaint_blur")._image = image_create_render_target(size_x, size_y);
	}
	if (RenderPathPaint.live_layer != null) SlotLayer.resize_and_set_bits(RenderPathPaint.live_layer);
	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	RenderPathRaytrace.ready = false; // Rebuild baketex
	///end
	Context.raw.ddirty = 2;
}

function base_set_layer_bits() {
	for (let l of Project.layers) SlotLayer.resize_and_set_bits(l);
	for (let l of History.undo_layers) SlotLayer.resize_and_set_bits(l);
}

function base_make_merge_pipe(red: bool, green: bool, blue: bool, alpha: bool): pipeline_t {
	let pipe: pipeline_t = g4_pipeline_create();
	pipe.vertex_shader = sys_get_shader("pass.vert");
	pipe.fragment_shader = sys_get_shader("layer_merge.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	pipe.input_layout = [vs];
	pipe.color_write_masks_red = [red];
	pipe.color_write_masks_green = [green];
	pipe.color_write_masks_blue = [blue];
	pipe.color_write_masks_alpha = [alpha];
	g4_pipeline_compile(pipe);
	return pipe;
}
///end

function base_make_pipe() {
	///if (is_paint || is_sculpt)
	base_pipe_merge = base_make_merge_pipe(true, true, true, true);
	base_pipe_merge_r = base_make_merge_pipe(true, false, false, false);
	base_pipe_merge_g = base_make_merge_pipe(false, true, false, false);
	base_pipe_merge_b = base_make_merge_pipe(false, false, true, false);
	base_pipe_merge_a = base_make_merge_pipe(false, false, false, true);
	base_tex0 =g4_pipeline_get_tex_unit(base_pipe_merge, "tex0"); // Always binding texpaint.a for blending
	base_tex1 =g4_pipeline_get_tex_unit(base_pipe_merge, "tex1");
	base_texmask =g4_pipeline_get_tex_unit(base_pipe_merge, "texmask");
	base_texa =g4_pipeline_get_tex_unit(base_pipe_merge, "texa");
	base_opac =g4_pipeline_get_const_loc(base_pipe_merge, "opac");
	base_blending =g4_pipeline_get_const_loc(base_pipe_merge, "blending");
	///end

	{
		base_pipe_copy = g4_pipeline_create();
		base_pipe_copy.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy.input_layout = [vs];
		g4_pipeline_compile(base_pipe_copy);
	}

	{
		base_pipe_copy_bgra = g4_pipeline_create();
		base_pipe_copy_bgra.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_bgra.fragment_shader = sys_get_shader("layer_copy_bgra.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_bgra.input_layout = [vs];
		g4_pipeline_compile(base_pipe_copy_bgra);
	}

	///if (krom_metal || krom_vulkan || krom_direct3d12)
	{
		base_pipe_copy8 = g4_pipeline_create();
		base_pipe_copy8.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy8.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy8.input_layout = [vs];
		base_pipe_copy8.color_attachment_count = 1;
		base_pipe_copy8.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(base_pipe_copy8);
	}

	{
		base_pipe_copy128 = g4_pipeline_create();
		base_pipe_copy128.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy128.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy128.input_layout = [vs];
		base_pipe_copy128.color_attachment_count = 1;
		base_pipe_copy128.color_attachments[0] = tex_format_t.RGBA128;
		g4_pipeline_compile(base_pipe_copy128);
	}
	///else
	base_pipe_copy8 = base_pipe_copy;
	base_pipe_copy128 = base_pipe_copy;
	///end

	///if (is_paint || is_sculpt)
	{
		base_pipe_invert8 = g4_pipeline_create();
		base_pipe_invert8.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_invert8.fragment_shader = sys_get_shader("layer_invert.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_invert8.input_layout = [vs];
		base_pipe_invert8.color_attachment_count = 1;
		base_pipe_invert8.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(base_pipe_invert8);
	}

	{
		base_pipe_apply_mask = g4_pipeline_create();
		base_pipe_apply_mask.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_apply_mask.fragment_shader = sys_get_shader("mask_apply.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_apply_mask.input_layout = [vs];
		g4_pipeline_compile(base_pipe_apply_mask);
		base_tex0_mask = g4_pipeline_get_tex_unit(base_pipe_apply_mask, "tex0");
		base_texa_mask = g4_pipeline_get_tex_unit(base_pipe_apply_mask, "texa");
	}

	{
		base_pipe_merge_mask = g4_pipeline_create();
		base_pipe_merge_mask.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_merge_mask.fragment_shader = sys_get_shader("mask_merge.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_merge_mask.input_layout = [vs];
		g4_pipeline_compile(base_pipe_merge_mask);
		base_tex0_merge_mask = g4_pipeline_get_tex_unit(base_pipe_merge_mask, "tex0");
		base_texa_merge_mask = g4_pipeline_get_tex_unit(base_pipe_merge_mask, "texa");
		base_opac_merge_mask = g4_pipeline_get_const_loc(base_pipe_merge_mask, "opac");
		base_blending_merge_mask = g4_pipeline_get_const_loc(base_pipe_merge_mask, "blending");
	}

	{
		base_pipe_colorid_to_mask = g4_pipeline_create();
		base_pipe_colorid_to_mask.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_colorid_to_mask.fragment_shader = sys_get_shader("mask_colorid.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_colorid_to_mask.input_layout = [vs];
		g4_pipeline_compile(base_pipe_colorid_to_mask);
		base_texpaint_colorid = g4_pipeline_get_tex_unit(base_pipe_colorid_to_mask, "texpaint_colorid");
		base_tex_colorid = g4_pipeline_get_tex_unit(base_pipe_colorid_to_mask, "texcolorid");
	}
	///end

	///if is_lab
	{
		base_pipe_copy_r = g4_pipeline_create();
		base_pipe_copy_r.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_r.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_r.input_layout = [vs];
		base_pipe_copy_r.color_write_masks_green = [false];
		base_pipe_copy_r.color_write_masks_blue = [false];
		base_pipe_copy_r.color_write_masks_alpha = [false];
		g4_pipeline_compile(base_pipe_copy_r);
	}

	{
		base_pipe_copy_g = g4_pipeline_create();
		base_pipe_copy_g.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_g.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_g.input_layout = [vs];
		base_pipe_copy_g.color_write_masks_red = [false];
		base_pipe_copy_g.color_write_masks_blue = [false];
		base_pipe_copy_g.color_write_masks_alpha = [false];
		g4_pipeline_compile(base_pipe_copy_g);
	}

	{
		base_pipe_copy_b = g4_pipeline_create();
		base_pipe_copy_b.vertex_shader = sys_get_shader("layer_view.vert");
		base_pipe_copy_b.fragment_shader = sys_get_shader("layer_copy.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		base_pipe_copy_b.input_layout = [vs];
		base_pipe_copy_b.color_write_masks_red = [false];
		base_pipe_copy_b.color_write_masks_green = [false];
		base_pipe_copy_b.color_write_masks_alpha = [false];
		g4_pipeline_compile(base_pipe_copy_b);
	}

	{
		base_pipe_inpaint_preview = g4_pipeline_create();
		base_pipe_inpaint_preview.vertex_shader = sys_get_shader("pass.vert");
		base_pipe_inpaint_preview.fragment_shader = sys_get_shader("inpaint_preview.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		base_pipe_inpaint_preview.input_layout = [vs];
		g4_pipeline_compile(base_pipe_inpaint_preview);
		base_tex0_inpaint_preview = g4_pipeline_get_tex_unit(base_pipe_inpaint_preview, "tex0");
		base_texa_inpaint_preview = g4_pipeline_get_tex_unit(base_pipe_inpaint_preview, "texa");
	}
	///end
}

function base_make_pipe_copy_rgb() {
	base_pipe_copy_rgb = g4_pipeline_create();
	base_pipe_copy_rgb.vertex_shader = sys_get_shader("layer_view.vert");
	base_pipe_copy_rgb.fragment_shader = sys_get_shader("layer_copy.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
	g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
	g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
	base_pipe_copy_rgb.input_layout = [vs];
	base_pipe_copy_rgb.color_write_masks_alpha = [false];
	g4_pipeline_compile(base_pipe_copy_rgb);
}

///if is_lab
function base_make_pipe_copy_a() {
	base_pipe_copy_a = g4_pipeline_create();
	base_pipe_copy_a.vertex_shader = sys_get_shader("pass.vert");
	base_pipe_copy_a.fragment_shader = sys_get_shader("layer_copy_rrrr.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	base_pipe_copy_a.input_layout = [vs];
	base_pipe_copy_a.color_write_masks_red = [false];
	base_pipe_copy_a.color_write_masks_green = [false];
	base_pipe_copy_a.color_write_masks_blue = [false];
	g4_pipeline_compile(base_pipe_copy_a);
	base_pipe_copy_a_tex = g4_pipeline_get_tex_unit(base_pipe_copy_a, "tex");
}
///end

function base_make_cursor_pipe() {
	base_pipe_cursor = g4_pipeline_create();
	base_pipe_cursor.vertex_shader = sys_get_shader("cursor.vert");
	base_pipe_cursor.fragment_shader = sys_get_shader("cursor.frag");
	let vs: vertex_struct_t = g4_vertex_struct_create();
	///if (krom_metal || krom_vulkan)
	g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
	///else
	g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
	g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
	g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
	///end
	base_pipe_cursor.input_layout = [vs];
	base_pipe_cursor.blend_source = blend_factor_t.SOURCE_ALPHA;
	base_pipe_cursor.blend_dest = blend_factor_t.INV_SOURCE_ALPHA;
	base_pipe_cursor.depth_write = false;
	base_pipe_cursor.depth_mode = compare_mode_t.ALWAYS;
	g4_pipeline_compile(base_pipe_cursor);
	base_cursor_vp = g4_pipeline_get_const_loc(base_pipe_cursor, "VP");
	base_cursor_inv_vp = g4_pipeline_get_const_loc(base_pipe_cursor, "invVP");
	base_cursor_mouse = g4_pipeline_get_const_loc(base_pipe_cursor, "mouse");
	base_cursor_tex_step = g4_pipeline_get_const_loc(base_pipe_cursor, "texStep");
	base_cursor_radius = g4_pipeline_get_const_loc(base_pipe_cursor, "radius");
	base_cursor_camera_right = g4_pipeline_get_const_loc(base_pipe_cursor, "cameraRight");
	base_cursor_tint = g4_pipeline_get_const_loc(base_pipe_cursor, "tint");
	base_cursor_gbufferd = g4_pipeline_get_tex_unit(base_pipe_cursor, "gbufferD");
	base_cursor_tex = g4_pipeline_get_tex_unit(base_pipe_cursor, "tex");
}

function base_make_temp_img() {
	///if (is_paint || is_sculpt)
	let l: SlotLayerRaw = Project.layers[0];
	///end
	///if is_lab
	let l: any = BrushOutputNode.inst;
	///end

	if (base_temp_image != null && (base_temp_image.width != l.texpaint.width || base_temp_image.height != l.texpaint.height || base_temp_image.format != l.texpaint.format)) {
		let _temptex0: render_target_t = render_path_render_targets.get("temptex0");
		base_notify_on_next_frame(function() {
			render_target_unload(_temptex0);
		});
		render_path_render_targets.delete("temptex0");
		base_temp_image = null;
	}
	if (base_temp_image == null) {
		///if (is_paint || is_sculpt)
		let format: string = base_bits_handle.position == texture_bits_t.BITS8  ? "RGBA32" :
								base_bits_handle.position == texture_bits_t.BITS16 ? "RGBA64" :
																				"RGBA128";
		///end
		///if is_lab
		let format: string = "RGBA32";
		///end

		let t: render_target_t = render_target_create();
		t.name = "temptex0";
		t.width = l.texpaint.width;
		t.height = l.texpaint.height;
		t.format = format;
		let rt: render_target_t = render_path_create_render_target(t);
		base_temp_image = rt._image;
	}
}

///if (is_paint || is_sculpt)
function base_make_temp_mask_img() {
	if (base_temp_mask_image != null && (base_temp_mask_image.width != Config.get_texture_res_x() || base_temp_mask_image.height != Config.get_texture_res_y())) {
		let _temp_mask_image: image_t = base_temp_mask_image;
		base_notify_on_next_frame(function() {
			image_unload(_temp_mask_image);
		});
		base_temp_mask_image = null;
	}
	if (base_temp_mask_image == null) {
		base_temp_mask_image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8);
	}
}
///end

function base_make_export_img() {
	///if (is_paint || is_sculpt)
	let l: SlotLayerRaw = Project.layers[0];
	///end
	///if is_lab
	let l: any = BrushOutputNode.inst;
	///end

	if (base_expa != null && (base_expa.width != l.texpaint.width || base_expa.height != l.texpaint.height || base_expa.format != l.texpaint.format)) {
		let _expa: image_t = base_expa;
		let _expb: image_t = base_expb;
		let _expc: image_t = base_expc;
		base_notify_on_next_frame(function() {
			image_unload(_expa);
			image_unload(_expb);
			image_unload(_expc);
		});
		base_expa = null;
		base_expb = null;
		base_expc = null;
		render_path_render_targets.delete("expa");
		render_path_render_targets.delete("expb");
		render_path_render_targets.delete("expc");
	}
	if (base_expa == null) {
		///if (is_paint || is_sculpt)
		let format: string = base_bits_handle.position == texture_bits_t.BITS8  ? "RGBA32" :
						base_bits_handle.position == texture_bits_t.BITS16 		? "RGBA64" :
																				"RGBA128";
		///end
		///if is_lab
		let format: string = "RGBA32";
		///end

		{
			let t: render_target_t = render_target_create();
			t.name = "expa";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt: render_target_t = render_path_create_render_target(t);
			base_expa = rt._image;
		}

		{
			let t: render_target_t = render_target_create();
			t.name = "expb";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt: render_target_t = render_path_create_render_target(t);
			base_expb = rt._image;
		}

		{
			let t: render_target_t = render_target_create();
			t.name = "expc";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt: render_target_t = render_path_create_render_target(t);
			base_expc = rt._image;
		}
	}
}

///if (is_paint || is_sculpt)
function base_duplicate_layer(l: SlotLayerRaw) {
	if (!SlotLayer.is_group(l)) {
		let new_layer: SlotLayerRaw = SlotLayer.duplicate(l);
		Context.set_layer(new_layer);
		let masks: SlotLayerRaw[] = SlotLayer.get_masks(l, false);
		if (masks != null) {
			for (let m of masks) {
				m = SlotLayer.duplicate(m);
				m.parent = new_layer;
				array_remove(Project.layers, m);
				Project.layers.splice(Project.layers.indexOf(new_layer), 0, m);
			}
		}
		Context.set_layer(new_layer);
	}
	else {
		let new_group: SlotLayerRaw = base_new_group();
		array_remove(Project.layers, new_group);
		Project.layers.splice(Project.layers.indexOf(l) + 1, 0, new_group);
		// group.show_panel = true;
		for (let c of SlotLayer.get_children(l)) {
			let masks: SlotLayerRaw[] = SlotLayer.get_masks(c, false);
			let new_layer: SlotLayerRaw = SlotLayer.duplicate(c);
			new_layer.parent = new_group;
			array_remove(Project.layers, new_layer);
			Project.layers.splice(Project.layers.indexOf(new_group), 0, new_layer);
			if (masks != null) {
				for (let m of masks) {
					let new_mask: SlotLayerRaw = SlotLayer.duplicate(m);
					new_mask.parent = new_layer;
					array_remove(Project.layers, new_mask);
					Project.layers.splice(Project.layers.indexOf(new_layer), 0, new_mask);
				}
			}
		}
		let group_masks: SlotLayerRaw[] = SlotLayer.get_masks(l);
		if (group_masks != null) {
			for (let m of group_masks) {
				let new_mask: SlotLayerRaw = SlotLayer.duplicate(m);
				new_mask.parent = new_group;
				array_remove(Project.layers, new_mask);
				Project.layers.splice(Project.layers.indexOf(new_group), 0, new_mask);
			}
		}
		Context.set_layer(new_group);
	}
}

function base_apply_masks(l: SlotLayerRaw) {
	let masks: SlotLayerRaw[] = SlotLayer.get_masks(l);

	if (masks != null) {
		for (let i: i32 = 0; i < masks.length - 1; ++i) {
			base_merge_layer(masks[i + 1], masks[i]);
			SlotLayer.delete(masks[i]);
		}
		SlotLayer.apply_mask(masks[masks.length - 1]);
		Context.raw.layer_preview_dirty = true;
	}
}

function base_merge_down() {
	let l1: SlotLayerRaw = Context.raw.layer;

	if (SlotLayer.is_group(l1)) {
		l1 = base_merge_group(l1);
	}
	else if (SlotLayer.has_masks(l1)) { // It is a layer
		base_apply_masks(l1);
		Context.set_layer(l1);
	}

	let l0: SlotLayerRaw = Project.layers[Project.layers.indexOf(l1) - 1];

	if (SlotLayer.is_group(l0)) {
		l0 = base_merge_group(l0);
	}
	else if (SlotLayer.has_masks(l0)) { // It is a layer
		base_apply_masks(l0);
		Context.set_layer(l0);
	}

	base_merge_layer(l0, l1);
	SlotLayer.delete(l1);
	Context.set_layer(l0);
	Context.raw.layer_preview_dirty = true;
}

function base_merge_group(l: SlotLayerRaw) {
	if (!SlotLayer.is_group(l)) return null;

	let children: SlotLayerRaw[] = SlotLayer.get_children(l);

	if (children.length == 1 && SlotLayer.has_masks(children[0], false)) {
		base_apply_masks(children[0]);
	}

	for (let i: i32 = 0; i < children.length - 1; ++i) {
		Context.set_layer(children[children.length - 1 - i]);
		History.merge_layers();
		base_merge_down();
	}

	// Now apply the group masks
	let masks: SlotLayerRaw[] = SlotLayer.get_masks(l);
	if (masks != null) {
		for (let i: i32 = 0; i < masks.length - 1; ++i) {
			base_merge_layer(masks[i + 1], masks[i]);
			SlotLayer.delete(masks[i]);
		}
		base_apply_mask(children[0], masks[masks.length - 1]);
	}

	children[0].parent = null;
	children[0].name = l.name;
	if (children[0].fill_layer != null) SlotLayer.to_paint_layer(children[0]);
	SlotLayer.delete(l);
	return children[0];
}

function base_merge_layer(l0 : SlotLayerRaw, l1: SlotLayerRaw, use_mask: bool = false) {
	if (!l1.visible || SlotLayer.is_group(l1)) return;

	if (base_pipe_merge == null) base_make_pipe();
	base_make_temp_img();
	if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();

	g2_begin(base_temp_image); // Copy to temp
	g2_set_pipeline(base_pipe_copy);
	g2_draw_image(l0.texpaint, 0, 0);
	g2_set_pipeline(null);
	g2_end();

	let empty: image_t = render_path_render_targets.get("empty_white")._image;
	let mask: image_t = empty;
	let l1masks: SlotLayerRaw[] =  use_mask ? SlotLayer.get_masks(l1) : null;
	if (l1masks != null) {
		// for (let i: i32 = 1; i < l1masks.length - 1; ++i) {
		// 	mergeLayer(l1masks[i + 1], l1masks[i]);
		// }
		mask = l1masks[0].texpaint;
	}

	if (SlotLayer.is_mask(l1)) {
		g4_begin(l0.texpaint);
		g4_set_pipeline(base_pipe_merge_mask);
		g4_set_tex(base_tex0_merge_mask, l1.texpaint);
		g4_set_tex(base_texa_merge_mask, base_temp_image);
		g4_set_float(base_opac_merge_mask, SlotLayer.get_opacity(l1));
		g4_set_int(base_blending_merge_mask, l1.blending);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
	}

	if (SlotLayer.is_layer(l1)) {
		if (l1.paint_base) {
			g4_begin(l0.texpaint);
			g4_set_pipeline(base_pipe_merge);
			g4_set_tex(base_tex0, l1.texpaint);
			g4_set_tex(base_tex1, empty);
			g4_set_tex(base_texmask, mask);
			g4_set_tex(base_texa, base_temp_image);
			g4_set_float(base_opac, SlotLayer.get_opacity(l1));
			g4_set_int(base_blending, l1.blending);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		///if is_paint
		g2_begin(base_temp_image);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_image(l0.texpaint_nor, 0, 0);
		g2_set_pipeline(null);
		g2_end();

		if (l1.paint_nor) {
			g4_begin(l0.texpaint_nor);
			g4_set_pipeline(base_pipe_merge);
			g4_set_tex(base_tex0, l1.texpaint);
			g4_set_tex(base_tex1, l1.texpaint_nor);
			g4_set_tex(base_texmask, mask);
			g4_set_tex(base_texa, base_temp_image);
			g4_set_float(base_opac, SlotLayer.get_opacity(l1));
			g4_set_int(base_blending, l1.paint_nor_blend ? -2 : -1);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		g2_begin(base_temp_image);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_image(l0.texpaint_pack, 0, 0);
		g2_set_pipeline(null);
		g2_end();

		if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
			if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
				base_commands_merge_pack(base_pipe_merge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask, l1.paint_height_blend ? -3 : -1);
			}
			else {
				if (l1.paint_occ) base_commands_merge_pack(base_pipe_merge_r, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
				if (l1.paint_rough) base_commands_merge_pack(base_pipe_merge_g, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
				if (l1.paint_met) base_commands_merge_pack(base_pipe_merge_b, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
			}
		}
		///end
	}
}

function base_flatten(height_to_normal: bool = false, layers: SlotLayerRaw[] = null): any {
	if (layers == null) layers = Project.layers;
	base_make_temp_img();
	base_make_export_img();
	if (base_pipe_merge == null) base_make_pipe();
	if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
	let empty: image_t = render_path_render_targets.get("empty_white")._image;

	// Clear export layer
	g4_begin(base_expa);
	g4_clear(color_from_floats(0.0, 0.0, 0.0, 0.0));
	g4_end();
	g4_begin(base_expb);
	g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0));
	g4_end();
	g4_begin(base_expc);
	g4_clear(color_from_floats(1.0, 0.0, 0.0, 0.0));
	g4_end();

	// Flatten layers
	for (let l1 of layers) {
		if (!SlotLayer.is_visible(l1)) continue;
		if (!SlotLayer.is_layer(l1)) continue;

		let mask: image_t = empty;
		let l1masks: SlotLayerRaw[] = SlotLayer.get_masks(l1);
		if (l1masks != null) {
			if (l1masks.length > 1) {
				base_make_temp_mask_img();
				g2_begin(base_temp_mask_image);
				g2_clear(0x00000000);
				g2_end();
				let l1: any = { texpaint: base_temp_mask_image };
				for (let i: i32 = 0; i < l1masks.length; ++i) {
					base_merge_layer(l1, l1masks[i]);
				}
				mask = base_temp_mask_image;
			}
			else mask = l1masks[0].texpaint;
		}

		if (l1.paint_base) {
			g2_begin(base_temp_image); // Copy to temp
			g2_set_pipeline(base_pipe_copy);
			g2_draw_image(base_expa, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			g4_begin(base_expa);
			g4_set_pipeline(base_pipe_merge);
			g4_set_tex(base_tex0, l1.texpaint);
			g4_set_tex(base_tex1, empty);
			g4_set_tex(base_texmask, mask);
			g4_set_tex(base_texa, base_temp_image);
			g4_set_float(base_opac, SlotLayer.get_opacity(l1));
			g4_set_int(base_blending, layers.length > 1 ? l1.blending : 0);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		///if is_paint
		if (l1.paint_nor) {
			g2_begin(base_temp_image);
			g2_set_pipeline(base_pipe_copy);
			g2_draw_image(base_expb, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			g4_begin(base_expb);
			g4_set_pipeline(base_pipe_merge);
			g4_set_tex(base_tex0, l1.texpaint);
			g4_set_tex(base_tex1, l1.texpaint_nor);
			g4_set_tex(base_texmask, mask);
			g4_set_tex(base_texa, base_temp_image);
			g4_set_float(base_opac, SlotLayer.get_opacity(l1));
			g4_set_int(base_blending, l1.paint_nor_blend ? -2 : -1);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
			g2_begin(base_temp_image);
			g2_set_pipeline(base_pipe_copy);
			g2_draw_image(base_expc, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
				base_commands_merge_pack(base_pipe_merge, base_expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask, l1.paint_height_blend ? -3 : -1);
			}
			else {
				if (l1.paint_occ) base_commands_merge_pack(base_pipe_merge_r, base_expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
				if (l1.paint_rough) base_commands_merge_pack(base_pipe_merge_g, base_expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
				if (l1.paint_met) base_commands_merge_pack(base_pipe_merge_b, base_expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
			}
		}
		///end
	}

	///if krom_metal
	// Flush command list
	g2_begin(base_expa);
	g2_end();
	g2_begin(base_expb);
	g2_end();
	g2_begin(base_expc);
	g2_end();
	///end

	let l0: any = { texpaint: base_expa, texpaint_nor: base_expb, texpaint_pack: base_expc };

	// Merge height map into normal map
	if (height_to_normal && MakeMaterial.height_used) {

		g2_begin(base_temp_image);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_image(l0.texpaint_nor, 0, 0);
		g2_set_pipeline(null);
		g2_end();

		g4_begin(l0.texpaint_nor);
		g4_set_pipeline(base_pipe_merge);
		g4_set_tex(base_tex0, base_temp_image);
		g4_set_tex(base_tex1, l0.texpaint_pack);
		g4_set_tex(base_texmask, empty);
		g4_set_tex(base_texa, empty);
		g4_set_float(base_opac, 1.0);
		g4_set_int(base_blending, -4);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
	}

	return l0;
}

function base_apply_mask(l: SlotLayerRaw, m: SlotLayerRaw) {
	if (!SlotLayer.is_layer(l) || !SlotLayer.is_mask(m)) return;

	if (base_pipe_merge == null) base_make_pipe();
	base_make_temp_img();

	// Copy layer to temp
	g2_begin(base_temp_image);
	g2_set_pipeline(base_pipe_copy);
	g2_draw_image(l.texpaint, 0, 0);
	g2_set_pipeline(null);
	g2_end();

	// Apply mask
	if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
	g4_begin(l.texpaint);
	g4_set_pipeline(base_pipe_apply_mask);
	g4_set_tex(base_tex0_mask, base_temp_image);
	g4_set_tex(base_texa_mask, m.texpaint);
	g4_set_vertex_buffer(const_data_screen_aligned_vb);
	g4_set_index_buffer(const_data_screen_aligned_ib);
	g4_draw();
	g4_end();
}

function base_commands_merge_pack(pipe: pipeline_t, i0: image_t, i1: image_t, i1pack: image_t, i1mask_opacity: f32, i1texmask: image_t, i1blending: i32 = -1) {
	g4_begin(i0);
	g4_set_pipeline(pipe);
	g4_set_tex(base_tex0, i1);
	g4_set_tex(base_tex1, i1pack);
	g4_set_tex(base_texmask, i1texmask);
	g4_set_tex(base_texa, base_temp_image);
	g4_set_float(base_opac, i1mask_opacity);
	g4_set_int(base_blending, i1blending);
	g4_set_vertex_buffer(const_data_screen_aligned_vb);
	g4_set_index_buffer(const_data_screen_aligned_ib);
	g4_draw();
	g4_end();
}

function base_is_fill_material(): bool {
	///if is_paint
	if (Context.raw.tool == workspace_tool_t.MATERIAL) return true;
	///end

	let m: SlotMaterialRaw = Context.raw.material;
	for (let l of Project.layers) if (l.fill_layer == m) return true;
	return false;
}

function base_update_fill_layers() {
	let _layer: SlotLayerRaw = Context.raw.layer;
	let _tool: workspace_tool_t = Context.raw.tool;
	let _fill_type: i32 = Context.raw.fill_type_handle.position;
	let current: image_t = null;

	///if is_paint
	if (Context.raw.tool == workspace_tool_t.MATERIAL) {
		if (RenderPathPaint.live_layer == null) {
			RenderPathPaint.live_layer = SlotLayer.create("_live");
		}

		current = _g2_current;
		if (current != null) g2_end();

		Context.raw.tool = workspace_tool_t.FILL;
		Context.raw.fill_type_handle.position = fill_type_t.OBJECT;
		MakeMaterial.parse_paint_material(false);
		Context.raw.pdirty = 1;
		RenderPathPaint.use_live_layer(true);
		RenderPathPaint.commands_paint(false);
		RenderPathPaint.dilate(true, true);
		RenderPathPaint.use_live_layer(false);
		Context.raw.tool = _tool;
		Context.raw.fill_type_handle.position = _fill_type;
		Context.raw.pdirty = 0;
		Context.raw.rdirty = 2;

		if (current != null) g2_begin(current);
		return;
	}
	///end

	let has_fill_layer: bool = false;
	let has_fill_mask: bool = false;
	for (let l of Project.layers) if (SlotLayer.is_layer(l) && l.fill_layer == Context.raw.material) has_fill_layer = true;
	for (let l of Project.layers) if (SlotLayer.is_mask(l) && l.fill_layer == Context.raw.material) has_fill_mask = true;

	if (has_fill_layer || has_fill_mask) {
		current = _g2_current;
		if (current != null) g2_end();
		Context.raw.pdirty = 1;
		Context.raw.tool = workspace_tool_t.FILL;
		Context.raw.fill_type_handle.position = fill_type_t.OBJECT;

		if (has_fill_layer) {
			let first: bool = true;
			for (let l of Project.layers) {
				if (SlotLayer.is_layer(l) && l.fill_layer == Context.raw.material) {
					Context.raw.layer = l;
					if (first) {
						first = false;
						MakeMaterial.parse_paint_material(false);
					}
					base_set_object_mask();
					SlotLayer.clear(l);
					RenderPathPaint.commands_paint(false);
					RenderPathPaint.dilate(true, true);
				}
			}
		}
		if (has_fill_mask) {
			let first: bool = true;
			for (let l of Project.layers) {
				if (SlotLayer.is_mask(l) && l.fill_layer == Context.raw.material) {
					Context.raw.layer = l;
					if (first) {
						first = false;
						MakeMaterial.parse_paint_material(false);
					}
					base_set_object_mask();
					SlotLayer.clear(l);
					RenderPathPaint.commands_paint(false);
					RenderPathPaint.dilate(true, true);
				}
			}
		}

		Context.raw.pdirty = 0;
		Context.raw.ddirty = 2;
		Context.raw.rdirty = 2;
		Context.raw.layers_preview_dirty = true; // Repaint all layer previews as multiple layers might have changed.
		if (current != null) g2_begin(current);
		Context.raw.layer = _layer;
		base_set_object_mask();
		Context.raw.tool = _tool;
		Context.raw.fill_type_handle.position = _fill_type;
		MakeMaterial.parse_paint_material(false);
	}
}

function base_update_fill_layer(parse_paint: bool = true) {
	let current: image_t = _g2_current;
	if (current != null) g2_end();

	let _tool: workspace_tool_t = Context.raw.tool;
	let _fill_type: i32 = Context.raw.fill_type_handle.position;
	Context.raw.tool = workspace_tool_t.FILL;
	Context.raw.fill_type_handle.position = fill_type_t.OBJECT;
	Context.raw.pdirty = 1;

	SlotLayer.clear(Context.raw.layer);

	if (parse_paint) MakeMaterial.parse_paint_material(false);
	RenderPathPaint.commands_paint(false);
	RenderPathPaint.dilate(true, true);

	Context.raw.rdirty = 2;
	Context.raw.tool = _tool;
	Context.raw.fill_type_handle.position = _fill_type;
	if (current != null) g2_begin(current);
}

function base_set_object_mask() {
	///if is_sculpt
	return;
	///end

	let ar: string[] = [tr("None")];
	for (let p of Project.paint_objects) ar.push(p.base.name);

	let mask: i32 = Context.object_mask_used() ? SlotLayer.get_object_mask(Context.raw.layer) : 0;
	if (Context.layer_filter_used()) mask = Context.raw.layer_filter;
	if (mask > 0) {
		if (Context.raw.merged_object != null) {
			Context.raw.merged_object.base.visible = false;
		}
		let o: mesh_object_t = Project.paint_objects[0];
		for (let p of Project.paint_objects) {
			if (p.base.name == ar[mask]) {
				o = p;
				break;
			}
		}
		Context.select_paint_object(o);
	}
	else {
		let is_atlas: bool = SlotLayer.get_object_mask(Context.raw.layer) > 0 && SlotLayer.get_object_mask(Context.raw.layer) <= Project.paint_objects.length;
		if (Context.raw.merged_object == null || is_atlas || Context.raw.merged_object_is_atlas) {
			let visibles: mesh_object_t[] = is_atlas ? Project.get_atlas_objects(SlotLayer.get_object_mask(Context.raw.layer)) : null;
			UtilMesh.merge_mesh(visibles);
		}
		Context.select_paint_object(Context.main_object());
		Context.raw.paint_object.skip_context = "paint";
		Context.raw.merged_object.base.visible = true;
	}
	UtilUV.dilatemap_cached = false;
}

function base_new_layer(clear: bool = true, position: i32 = -1): SlotLayerRaw {
	if (Project.layers.length > base_max_layers) return null;
	let l: SlotLayerRaw = SlotLayer.create();
	l.object_mask = Context.raw.layer_filter;
	if (position == -1) {
		if (SlotLayer.is_mask(Context.raw.layer)) Context.set_layer(Context.raw.layer.parent);
		Project.layers.splice(Project.layers.indexOf(Context.raw.layer) + 1, 0, l);
	}
	else {
		Project.layers.splice(position, 0, l);
	}

	Context.set_layer(l);
	let li: i32 = Project.layers.indexOf(Context.raw.layer);
	if (li > 0) {
		let below: SlotLayerRaw = Project.layers[li - 1];
		if (SlotLayer.is_layer(below)) {
			Context.raw.layer.parent = below.parent;
		}
	}
	if (clear) app_notify_on_init(function() { SlotLayer.clear(l); });
	Context.raw.layer_preview_dirty = true;
	return l;
}

function base_new_mask(clear: bool = true, parent: SlotLayerRaw, position: i32 = -1): SlotLayerRaw {
	if (Project.layers.length > base_max_layers) return null;
	let l: SlotLayerRaw = SlotLayer.create("", layer_slot_type_t.MASK, parent);
	if (position == -1) position = Project.layers.indexOf(parent);
	Project.layers.splice(position, 0, l);
	Context.set_layer(l);
	if (clear) app_notify_on_init(function() { SlotLayer.clear(l); });
	Context.raw.layer_preview_dirty = true;
	return l;
}

function base_new_group(): SlotLayerRaw {
	if (Project.layers.length > base_max_layers) return null;
	let l: SlotLayerRaw = SlotLayer.create("", layer_slot_type_t.GROUP);
	Project.layers.push(l);
	Context.set_layer(l);
	return l;
}

function base_create_fill_layer(uv_type: uv_type_t = uv_type_t.UVMAP, decal_mat: mat4_t = null, position: i32 = -1) {
	let _init = function() {
		let l: SlotLayerRaw = base_new_layer(false, position);
		History.new_layer();
		l.uv_type = uv_type;
		if (decal_mat != null) l.decal_mat = decal_mat;
		l.object_mask = Context.raw.layer_filter;
		History.to_fill_layer();
		SlotLayer.to_fill_layer(l);
	}
	app_notify_on_init(_init);
}

function base_create_image_mask(asset: asset_t) {
	let l: SlotLayerRaw = Context.raw.layer;
	if (SlotLayer.is_mask(l) || SlotLayer.is_group(l)) {
		return;
	}

	History.new_layer();
	let m: SlotLayerRaw = base_new_mask(false, l);
	SlotLayer.clear(m, 0x00000000, Project.get_image(asset));
	Context.raw.layer_preview_dirty = true;
}

function base_create_color_layer(baseColor: i32, occlusion: f32 = 1.0, roughness: f32 = base_default_rough, metallic: f32 = 0.0, position: i32 = -1) {
	let _init = function() {
		let l: SlotLayerRaw = base_new_layer(false, position);
		History.new_layer();
		l.uv_type = uv_type_t.UVMAP;
		l.object_mask = Context.raw.layer_filter;
		SlotLayer.clear(l, baseColor, null, occlusion, roughness, metallic);
	}
	app_notify_on_init(_init);
}

function base_on_layers_resized() {
	app_notify_on_init(function() {
		base_resize_layers();
		let _layer: SlotLayerRaw = Context.raw.layer;
		let _material: SlotMaterialRaw = Context.raw.material;
		for (let l of Project.layers) {
			if (l.fill_layer != null) {
				Context.raw.layer = l;
				Context.raw.material = l.fill_layer;
				base_update_fill_layer();
			}
		}
		Context.raw.layer = _layer;
		Context.raw.material = _material;
		MakeMaterial.parse_paint_material();
	});
	UtilUV.uvmap = null;
	UtilUV.uvmap_cached = false;
	UtilUV.trianglemap = null;
	UtilUV.trianglemap_cached = false;
	UtilUV.dilatemap_cached = false;
	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	RenderPathRaytrace.ready = false;
	///end
}
///end

///if is_lab
function base_flatten(heightToNormal: bool = false): any {
	let texpaint: image_t = BrushOutputNode.inst.texpaint;
	let texpaint_nor: image_t = BrushOutputNode.inst.texpaint_nor;
	let texpaint_pack: image_t = BrushOutputNode.inst.texpaint_pack;

	let nodes: zui_nodes_t = UINodes.get_nodes();
	let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
	if (nodes.nodes_selected_id.length > 0) {
		let node: zui_node_t = zui_get_node(canvas.nodes, nodes.nodes_selected_id[0]);
		let brush_node: LogicNode = ParserLogic.get_logic_node(node);
		if (brush_node != null && brush_node.get_cached_image() != null) {
			texpaint = brush_node.get_cached_image();
			texpaint_nor = render_path_render_targets.get("texpaint_nor_empty")._image;
			texpaint_pack = render_path_render_targets.get("texpaint_pack_empty")._image;
		}
	}

	return { texpaint: texpaint, texpaint_nor: texpaint_nor, texpaint_pack: texpaint_pack };
}

function base_on_layers_resized() {
	image_unload(BrushOutputNode.inst.texpaint);
	BrushOutputNode.inst.texpaint = render_path_render_targets.get("texpaint")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y());
	image_unload(BrushOutputNode.inst.texpaint_nor);
	BrushOutputNode.inst.texpaint_nor = render_path_render_targets.get("texpaint_nor")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y());
	image_unload(BrushOutputNode.inst.texpaint_pack);
	BrushOutputNode.inst.texpaint_pack = render_path_render_targets.get("texpaint_pack")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y());

	if (InpaintNode.image != null) {
		image_unload(InpaintNode.image);
		InpaintNode.image = null;
		image_unload(InpaintNode.mask);
		InpaintNode.mask = null;
		InpaintNode.init();
	}

	if (PhotoToPBRNode.images != null) {
		for (let image of PhotoToPBRNode.images) image_unload(image);
		PhotoToPBRNode.images = null;
		PhotoToPBRNode.init();
	}

	if (TilingNode.image != null) {
		image_unload(TilingNode.image);
		TilingNode.image = null;
		TilingNode.init();
	}

	image_unload(render_path_render_targets.get("texpaint_blend0")._image);
	render_path_render_targets.get("texpaint_blend0")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8);
	image_unload(render_path_render_targets.get("texpaint_blend1")._image);
	render_path_render_targets.get("texpaint_blend1")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8);

	if (render_path_render_targets.get("texpaint_node") != null) {
		render_path_render_targets.delete("texpaint_node");
	}
	if (render_path_render_targets.get("texpaint_node_target") != null) {
		render_path_render_targets.delete("texpaint_node_target");
	}

	base_notify_on_next_frame(function() {
		base_init_layers();
	});

	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	RenderPathRaytrace.ready = false;
	///end
}
///end
