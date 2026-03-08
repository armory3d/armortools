
void _kickstart() {
	_world_data_empty_irr        = NULL;
	_render_path_last_frame_time = 0.0;
	_render_path_loading         = 0;
	gc_unroot(_render_path_cached_shader_contexts);
	_render_path_cached_shader_contexts = any_map_create();
	gc_root(_render_path_cached_shader_contexts);
	gc_unroot(ui_children);
	ui_children = any_map_create();
	gc_root(ui_children);
	gc_unroot(ui_nodes_custom_buttons);
	ui_nodes_custom_buttons = any_map_create();
	gc_root(ui_nodes_custom_buttons);
	tab_swatches_drag_pos     = -1;
	slot_brush_default_canvas = NULL;
	layers_temp_image         = NULL;
	layers_expa               = NULL;
	layers_expb               = NULL;
	layers_expc               = NULL;
	layers_default_base       = 0.5;
	layers_default_rough      = 0.4;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	layers_max_layers = 18;
#else
	layers_max_layers = 255;
#endif
	gc_unroot(operator_ops);
	operator_ops = any_map_create();
	gc_root(operator_ops);
	_compass_hovered      = NULL;
	_compass_hovered_last = NULL;
	config_raw            = NULL;
	config_loaded         = false;
	config_button_align   = UI_ALIGN_LEFT;
	config_button_spacing = "       ";
	gc_unroot(physics_body_object_map);
	physics_body_object_map = any_imap_create();
	gc_root(physics_body_object_map);
	gc_unroot(box_export_htab);
	box_export_htab = ui_handle_create();
	gc_root(box_export_htab);
	box_export_files = NULL;
	gc_unroot(box_export_mesh_handle);
	box_export_mesh_handle = ui_handle_create();
	gc_root(box_export_mesh_handle);
	gc_unroot(box_export_hpreset);
	box_export_hpreset = ui_handle_create();
	gc_root(box_export_hpreset);
	box_export_preset = NULL;
	gc_unroot(box_export_channels);
	box_export_channels = any_array_create_from_raw(
	    (void *[]){
	        "base_r",
	        "base_g",
	        "base_b",
	        "height",
	        "metal",
	        "nor_r",
	        "nor_g",
	        "nor_g_directx",
	        "nor_b",
	        "occ",
	        "opac",
	        "rough",
	        "smooth",
	        "emis",
	        "subs",
	        "0.0",
	        "1.0",
	    },
	    17);
	gc_root(box_export_channels);
	gc_unroot(box_export_color_spaces);
	box_export_color_spaces = any_array_create_from_raw(
	    (void *[]){
	        "linear",
	        "srgb",
	    },
	    2);
	gc_root(box_export_color_spaces);
	slot_material_default_canvas = NULL;
	pipes_copy_rgb               = NULL;
	pipes_merge                  = NULL;
	pipes_merge_r                = NULL;
	pipes_merge_g                = NULL;
	pipes_merge_b                = NULL;
	pipes_temp_mask_image        = NULL;
	gc_unroot(import_texture_importers);
	import_texture_importers = any_map_create();
	gc_root(import_texture_importers);
	ui_files_default_path =
#ifdef IRON_WINDOWS
	    "C:\\Users"
#elif defined(IRON_ANDROID)
	    "/storage/emulated/0/Download"
#elif defined(IRON_MACOS)
	    "/Users"
#else
	    "/"
#endif
	    ;

	gc_unroot(ui_files_path);
	ui_files_path = ui_files_default_path;
	gc_root(ui_files_path);
	ui_files_last_path       = "";
	ui_files_last_search     = "";
	ui_files_files           = NULL;
	ui_files_icon_map        = NULL;
	ui_files_icon_file_map   = NULL;
	ui_files_selected        = -1;
	ui_files_show_extensions = false;
	ui_files_offline         = false;
	base_ui_enabled          = true;
	base_view3d_show         = true;
	base_is_dragging         = false;
	base_is_resizing         = false;
	base_drag_asset          = NULL;
	base_drag_swatch         = NULL;
	base_drag_file           = NULL;
	base_drag_file_icon      = NULL;
	base_drag_tint           = 0xffffffff;
	base_drag_size           = -1;
	base_drag_rect           = NULL;
	base_drag_off_x          = 0.0;
	base_drag_off_y          = 0.0;
	base_drag_start          = 0.0;
	base_drop_x              = 0.0;
	base_drop_y              = 0.0;
	base_font                = NULL;
	base_default_element_w   = 100;
	base_default_element_h   = 28;
	base_default_font_size   = 13;
	gc_unroot(base_res_handle);
	base_res_handle = ui_handle_create();
	gc_root(base_res_handle);
	gc_unroot(base_bits_handle);
	base_bits_handle = ui_handle_create();
	gc_root(base_bits_handle);
	gc_unroot(base_drop_paths);
	base_drop_paths = any_array_create_from_raw((void *[]){}, 0);
	gc_root(base_drop_paths);
	base_appx                      = 0;
	base_appy                      = 0;
	base_last_window_width         = 0;
	base_last_window_height        = 0;
	base_drag_material             = NULL;
	base_drag_layer                = NULL;
	ui_base_show                   = true;
	ui_base_border_started         = 0;
	ui_base_border_handle          = NULL;
	ui_base_action_paint_remap     = "";
	ui_base_operator_search_offset = 0;
	ui_base_undo_tap_time          = 0.0;
	ui_base_redo_tap_time          = 0.0;
	gc_unroot(ui_base_hwnds);
	ui_base_hwnds = ui_base_init_hwnds();
	gc_root(ui_base_hwnds);
	gc_unroot(ui_base_htabs);
	ui_base_htabs = ui_base_init_htabs();
	gc_root(ui_base_htabs);
	gc_unroot(ui_base_hwnd_tabs);
	ui_base_hwnd_tabs = ui_base_init_hwnd_tabs();
	gc_root(ui_base_hwnd_tabs);
	gizmo_v              = vec4_create(0.0, 0.0, 0.0, 1.0);
	gizmo_v0             = vec4_create(0.0, 0.0, 0.0, 1.0);
	gizmo_q              = quat_create(0.0, 0.0, 0.0, 1.0);
	gizmo_q0             = quat_create(0.0, 0.0, 0.0, 1.0);
	ui_toolbar_default_w = 36;
	gc_unroot(ui_toolbar_handle);
	ui_toolbar_handle = ui_handle_create();
	gc_root(ui_toolbar_handle);
	ui_toolbar_last_tool = 0;
	gc_unroot(ui_toolbar_tool_names);
	ui_toolbar_tool_names = any_array_create_from_raw(
	    (void *[]){
	        _tr("Brush"),
	        _tr("Eraser"),
	        _tr("Fill"),
	        _tr("Decal"),
	        _tr("Text"),
	        _tr("Clone"),
	        _tr("Blur"),
	        _tr("Smudge"),
	        _tr("Particle"),
	        _tr("ColorID"),
	        _tr("Picker"),
	        _tr("Bake"),
	        _tr("Material"),
	        _tr("Gizmo"),
	    },
	    14);
	gc_root(ui_toolbar_tool_names);
	gc_unroot(ui_toolbar_tooltip_extras);
	ui_toolbar_tooltip_extras = any_array_create_from_raw(
	    (void *[]){
	        _tr("Hold {action_paint} to paint\nHold {brush_ruler} and press {action_paint} to paint a straight line (ruler mode)"),
	        _tr("Hold {action_paint} to erase\nHold {brush_ruler} and press {action_paint} to erase a straight line (ruler mode)"),
	        "",
	        _tr("Hold {decal_mask} to paint on a decal mask"),
	        _tr("Hold {decal_mask} to use the text as a mask"),
	        _tr("Hold {set_clone_source} to set source"),
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	        "",
	    },
	    14);
	gc_root(ui_toolbar_tooltip_extras);
	uniforms_ext_ortho_p = mat4_ortho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);
	gc_unroot(box_projects_htab);
	box_projects_htab = ui_handle_create();
	gc_root(box_projects_htab);
	gc_unroot(box_projects_hsearch);
	box_projects_hsearch = ui_handle_create();
	gc_root(box_projects_hsearch);
	box_projects_icon_map = NULL;
	history_undo_i        = 0;
	history_undos         = 0;
	history_redos         = 0;
	history_push_undo     = false;
	history_undo_layers   = NULL;
	gc_unroot(tab_scripts_hscript);
	tab_scripts_hscript = ui_handle_create();
	gc_root(tab_scripts_hscript);
	tab_scripts_text_coloring    = NULL;
	import_mesh_clear_layers     = true;
	import_mesh_meshes_to_unwrap = NULL;
	gc_unroot(import_mesh_importers);
	import_mesh_importers = any_map_create();
	gc_root(import_mesh_importers);
	ui_menubar_default_w = 406;
	gc_unroot(ui_menubar_hwnd);
	ui_menubar_hwnd = ui_handle_create();
	gc_root(ui_menubar_hwnd);
	gc_unroot(ui_menubar_menu_handle);
	ui_menubar_menu_handle = ui_handle_create();
	gc_root(ui_menubar_menu_handle);
	gc_unroot(ui_menubar_tab);
	ui_menubar_tab = ui_handle_create();
	gc_root(ui_menubar_tab);
	ui_menubar_w             = ui_menubar_default_w;
	ui_menubar_category      = 0;
	_ui_menubar_saved_camera = mat4_nan();
	_ui_menubar_plane        = NULL;
	gc_unroot(translator_translations);
	translator_translations = any_map_create();
	gc_root(translator_translations);
	translator_cjk_font_indices        = NULL;
	translator_last_locale             = "en";
	ui_menu_show                       = false;
	ui_menu_nested                     = false;
	ui_menu_x                          = 0;
	ui_menu_y                          = 0;
	ui_menu_h                          = 0;
	ui_menu_keep_open                  = false;
	ui_menu_commands                   = NULL;
	ui_menu_show_first                 = true;
	ui_menu_hide_flag                  = false;
	ui_menu_sub_x                      = 0;
	ui_menu_sub_y                      = 0;
	ui_menu_sub_handle                 = NULL;
	render_path_base_taa_frame         = 0;
	render_path_base_super_sample      = 1.0;
	render_path_base_last_x            = -1.0;
	render_path_base_last_y            = -1.0;
	render_path_base_bloom_current_mip = 0;
	render_path_base_buf_swapped       = false;
	gc_unroot(project_raw);
	project_raw = GC_ALLOC_INIT(project_format_t, {0});
	gc_root(project_raw);
	project_filepath = "";
	gc_unroot(project_assets);
	project_assets = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_assets);
	gc_unroot(project_asset_names);
	project_asset_names = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_asset_names);
	project_asset_id = 0;
	gc_unroot(project_mesh_assets);
	project_mesh_assets = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_mesh_assets);
	gc_unroot(project_material_groups);
	project_material_groups = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_material_groups);
	project_paint_objects = NULL;
	gc_unroot(project_asset_map);
	project_asset_map = any_imap_create();
	gc_root(project_asset_map);
	project_mesh_list = NULL;
	gc_unroot(project_materials);
	project_materials = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_materials);
	gc_unroot(project_brushes);
	project_brushes = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_brushes);
	gc_unroot(project_layers);
	project_layers = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_layers);
	gc_unroot(project_fonts);
	project_fonts = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_fonts);
	project_atlas_objects      = NULL;
	project_atlas_names        = NULL;
	_project_unwrap_by         = 0;
	ui_view2d_text_input_hover = false;
	ui_view2d_uvmap_show       = false;
	ui_view2d_tex_type         = PAINT_TEX_BASE;
	ui_view2d_layer_mode       = VIEW_2D_LAYER_MODE_SELECTED;
	ui_view2d_type             = VIEW_2D_TYPE_LAYER;
	ui_view2d_show             = false;
	gc_unroot(ui_view2d_hwnd);
	ui_view2d_hwnd = ui_handle_create();
	gc_root(ui_view2d_hwnd);
	ui_view2d_pan_x         = 0.0;
	ui_view2d_pan_y         = 0.0;
	ui_view2d_pan_scale     = 1.0;
	ui_view2d_tiled_show    = false;
	ui_view2d_controls_down = false;
	ui_view2d_grid          = NULL;
	ui_view2d_grid_redraw   = true;
	gc_unroot(ui_view2d_htab);
	ui_view2d_htab = ui_handle_create();
	gc_root(ui_view2d_htab);
	ui_view2d_layer_touched    = false;
	str_get_pos_nor_from_depth = "\
fun get_pos_from_depth(uv: float2, invVP: float4x4): float3 { \
	var depth: float = sample_lod(gbufferD, sampler_linear, float2(uv.x, 1.0 - uv.y), 0.0).r; \
	var wpos: float4 = float4(uv * 2.0 - 1.0, depth, 1.0); \
	wpos = invVP * wpos; \
	return wpos.xyz / wpos.w; \
} \
fun get_nor_from_depth(p0: float3, uv: float2, invVP: float4x4, tex_step: float2): float3 { \
	var p1: float3 = get_pos_from_depth(uv + float2(tex_step.x * 4.0, 0.0), invVP); \
	var p2: float3 = get_pos_from_depth(uv + float2(0.0, tex_step.y * 4.0), invVP); \
	return normalize(cross(p2 - p0, p1 - p0)); \
} \
";
	sim_running                = false;
	gc_unroot(sim_object_script_map);
	sim_object_script_map = any_map_create();
	gc_root(sim_object_script_map);
	sim_record                = false;
	sim_initialized           = false;
	parser_material_tex_coord = "tex_coord";
	parser_material_eps       = 0.000001;
	gc_unroot(parser_material_node_values);
	parser_material_node_values = any_map_create();
	gc_root(parser_material_node_values);
	gc_unroot(parser_material_node_vectors);
	parser_material_node_vectors = any_map_create();
	gc_root(parser_material_node_vectors);
	gc_unroot(parser_material_custom_nodes);
	parser_material_custom_nodes = any_map_create();
	gc_root(parser_material_custom_nodes);
	parser_material_parse_surface           = true;
	parser_material_parse_opacity           = true;
	parser_material_parse_height            = false;
	parser_material_parse_height_as_channel = false;
	parser_material_parse_emission          = false;
	parser_material_parse_subsurface        = false;
	parser_material_parsing_basecolor       = false;
	parser_material_triplanar               = false;
	parser_material_sample_keep_aspect      = false;
	parser_material_sample_uv_scale         = "1.0";
	parser_material_transform_color_space   = true;
	parser_material_blur_passthrough        = false;
	parser_material_warp_passthrough        = false;
	parser_material_bake_passthrough        = false;
	parser_material_start_group             = NULL;
	parser_material_start_parents           = NULL;
	parser_material_start_node              = NULL;
	parser_material_arm_export_tangents     = true;
	parser_material_script_links            = NULL;
	gc_unroot(parser_material_parsed_map);
	parser_material_parsed_map = any_map_create();
	gc_root(parser_material_parsed_map);
	gc_unroot(parser_material_texture_map);
	parser_material_texture_map = any_map_create();
	gc_root(parser_material_texture_map);
	parser_material_is_frag            = true;
	args_use                           = false;
	args_asset_path                    = "";
	args_background                    = false;
	args_export_textures               = false;
	args_export_textures_type          = "";
	args_export_textures_preset        = "";
	args_export_textures_path          = "";
	args_reimport_mesh                 = false;
	args_export_mesh                   = false;
	args_export_mesh_path              = "";
	args_export_material               = false;
	args_export_material_path          = "";
	util_render_material_preview_size  = 256;
	util_render_node_preview_size      = 512;
	util_render_decal_preview_size     = 512;
	util_render_layer_preview_size     = 200;
	util_render_font_preview_size      = 200;
	util_render_screen_aligned_full_vb = NULL;
	util_render_screen_aligned_full_ib = NULL;
	gc_unroot(tab_browser_hpath);
	tab_browser_hpath = ui_handle_create();
	gc_root(tab_browser_hpath);
	gc_unroot(tab_browser_hsearch);
	tab_browser_hsearch = ui_handle_create();
	gc_root(tab_browser_hsearch);
	tab_browser_known     = false;
	tab_browser_last_path = "";
	tab_browser_refresh   = false;
	gc_unroot(util_mesh_unwrappers);
	util_mesh_unwrappers = any_map_create();
	gc_root(util_mesh_unwrappers);
	ui_header_default_h = 30;
	ui_header_h         = ui_header_default_h;
	gc_unroot(ui_header_handle);
	ui_header_handle = ui_handle_create();
	gc_root(ui_header_handle);
	gc_unroot(plugin_map);
	plugin_map = any_map_create();
	gc_root(plugin_map);
	gc_unroot(parser_logic_custom_nodes);
	parser_logic_custom_nodes = any_map_create();
	gc_root(parser_logic_custom_nodes);
	parser_logic_parsed_nodes = NULL;
	gc_unroot(resource_bundled);
	resource_bundled = any_map_create();
	gc_root(resource_bundled);
	render_path_raytrace_bake_rays_pix        = 0;
	render_path_raytrace_bake_rays_sec        = 0;
	render_path_raytrace_bake_current_sample  = 0;
	render_path_raytrace_bake_rays_timer      = 0.0;
	render_path_raytrace_bake_rays_counter    = 0;
	render_path_raytrace_bake_last_layer      = NULL;
	render_path_raytrace_bake_last_bake_type  = 0;
	render_path_raytrace_bake_last_bake_type2 = 0;
	make_material_default_scon                = NULL;
	make_material_default_mcon                = NULL;
	make_material_opac_used                   = false;
	make_material_height_used                 = false;
	make_material_emis_used                   = false;
	make_material_subs_used                   = false;
	ui_nodes_show                             = false;
	ui_nodes_canvas_type                      = CANVAS_TYPE_MATERIAL;
	ui_nodes_show_menu                        = false;
	ui_nodes_show_menu_first                  = true;
	ui_nodes_hide_menu                        = false;
	ui_nodes_menu_category                    = 0;
	ui_nodes_popup_x                          = 0.0;
	ui_nodes_popup_y                          = 0.0;
	ui_nodes_uichanged_last                   = false;
	ui_nodes_recompile_mat                    = false;
	ui_nodes_recompile_mat_final              = false;
	ui_nodes_node_search_spawn                = NULL;
	ui_nodes_node_search_offset               = 0;
	ui_nodes_last_canvas                      = NULL;
	ui_nodes_last_node_selected_id            = -1;
	ui_nodes_release_link                     = false;
	ui_nodes_is_node_menu_op                  = false;
	ui_nodes_grid                             = NULL;
	ui_nodes_grid_redraw                      = true;
	ui_nodes_grid_cell_w                      = 200;
	ui_nodes_grid_small_cell_w                = 40;
	gc_unroot(ui_nodes_hwnd);
	ui_nodes_hwnd = ui_handle_create();
	gc_root(ui_nodes_hwnd);
	gc_unroot(ui_nodes_group_stack);
	ui_nodes_group_stack = any_array_create_from_raw((void *[]){}, 0);
	gc_root(ui_nodes_group_stack);
	ui_nodes_controls_down = false;
	ui_nodes_tabs          = NULL;
	gc_unroot(ui_nodes_htab);
	ui_nodes_htab = ui_handle_create();
	gc_root(ui_nodes_htab);
	ui_nodes_last_zoom = 1.0;
	gc_unroot(_ui_nodes_htype);
	_ui_nodes_htype = ui_handle_create();
	gc_root(_ui_nodes_htype);
	gc_unroot(_ui_nodes_hname);
	_ui_nodes_hname = ui_handle_create();
	gc_root(_ui_nodes_hname);
	gc_unroot(_ui_nodes_hmin);
	_ui_nodes_hmin = ui_handle_create();
	gc_root(_ui_nodes_hmin);
	gc_unroot(_ui_nodes_hmax);
	_ui_nodes_hmax = ui_handle_create();
	gc_root(_ui_nodes_hmax);
	gc_unroot(_ui_nodes_hval0);
	_ui_nodes_hval0 = ui_handle_create();
	gc_root(_ui_nodes_hval0);
	gc_unroot(_ui_nodes_hval1);
	_ui_nodes_hval1 = ui_handle_create();
	gc_root(_ui_nodes_hval1);
	gc_unroot(_ui_nodes_hval2);
	_ui_nodes_hval2 = ui_handle_create();
	gc_root(_ui_nodes_hval2);
	gc_unroot(_ui_nodes_hval3);
	_ui_nodes_hval3 = ui_handle_create();
	gc_root(_ui_nodes_hval3);
	ui_nodes_node_changed = NULL;
	gc_unroot(nodes_brush_categories);
	nodes_brush_categories = any_array_create_from_raw(
	    (void *[]){
	        _tr("Nodes"),
	    },
	    1);
	gc_root(nodes_brush_categories);
	str_get_smudge_tool_weight = "\
fun get_smudge_tool_weight(i: int): float { \
	if (i == 0) { return 1.0 / 28.0; } \
	if (i == 1) { return 2.0 / 28.0; } \
	if (i == 2) { return 3.0 / 28.0; } \
	if (i == 3) { return 4.0 / 28.0; } \
	if (i == 4) { return 5.0 / 28.0; } \
	if (i == 5) { return 6.0 / 28.0; } \
	return 7.0 / 28.0; \
} \
";
	str_get_blur_tool_weight   = "\
fun get_blur_tool_weight(i: int): float { \
	if (i == 0) { return 0.034619 / 2.0; } \
	if (i == 1) { return 0.044859 / 2.0; } \
	if (i == 2) { return 0.055857 / 2.0; } \
	if (i == 3) { return 0.066833 / 2.0; } \
	if (i == 4) { return 0.076841 / 2.0; } \
	if (i == 5) { return 0.084894 / 2.0; } \
	if (i == 6) { return 0.090126 / 2.0; } \
	if (i == 7) { return 0.09194 / 2.0; } \
	if (i == 8) { return 0.090126 / 2.0; } \
	if (i == 9) { return 0.084894 / 2.0; } \
	if (i == 10) { return 0.076841 / 2.0; } \
	if (i == 11) { return 0.066833 / 2.0; } \
	if (i == 12) { return 0.055857 / 2.0; } \
	if (i == 13) { return 0.044859 / 2.0; } \
	return 0.034619 / 2.0; \
} \
";
	manifest_title             = "ArmorPaint";
	manifest_version           = "1.0 alpha";
	manifest_version_project   = "4";
	manifest_version_config    = "1";
	manifest_url               = "https://armorpaint.org";
	manifest_url_android       = "https://play.google.com/store/apps/details?id=org.armorpaint";
	manifest_url_ios           = "https://apps.apple.com/app/armorpaint/id1533967534";

	gc_unroot(nodes_material_categories);
#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	nodes_material_categories = any_array_create_from_raw(
	    (void *[]){
	        _tr("Input"),
	        _tr("Texture"),
	        _tr("Color"),
	        _tr("Utilities"),
	        _tr("Neural"),
	        _tr("Group"),
	    },
	    6);
#else
	nodes_material_categories = any_array_create_from_raw(
	    (void *[]){
	        _tr("Input"),
	        _tr("Texture"),
	        _tr("Color"),
	        _tr("Utilities"),
	        _tr("Group"),
	    },
	    5);
#endif
	gc_root(nodes_material_categories);

	nodes_material_list        = NULL;
	make_mesh_layer_pass_count = 1;
	str_cotangent_frame        = "\
fun cotangent_frame(n: float3, p: float3, tex_coord: float2): float3x3 { \
	var duv1: float2 = ddx2(tex_coord); \
	var duv2: float2 = ddy2(tex_coord); \
	var dp1: float3 = ddx3(p); \
	var dp2: float3 = ddy3(p); \
	var dp2perp: float3 = cross(dp2, n); \
	var dp1perp: float3 = cross(n, dp1); \
	var t: float3 = dp2perp * duv1.x + dp1perp * duv2.x; \
	var b: float3 = dp2perp * duv1.y + dp1perp * duv2.y; \
	var invmax: float = rsqrt(max(dot(t, t), dot(b, b))); \
	return float3x3(t * invmax, b * invmax, n); \
} \
";
	str_octahedron_wrap        = "\
fun octahedron_wrap(v: float2): float2 { \
	var a: float2; \
	if (v.x >= 0.0) { a.x = 1.0; } else { a.x = -1.0; } \
	if (v.y >= 0.0) { a.y = 1.0; } else { a.y = -1.0; } \
	var r: float2; \
	r.x = abs(v.y); \
	r.y = abs(v.x); \
	r.x = 1.0 - r.x; \
	r.y = 1.0 - r.y; \
	return r * a; \
} \
";
	str_pack_float_int16       = "\
fun pack_f32_i16(f: float, i: uint): float { \
	return 0.062504762 * min(f, 0.9999) + 0.062519999 * float(i); \
} \
";
	str_sh_irradiance          = "\
fun sh_irradiance(nor: float3): float3 { \
	var c1: float = 0.429043; \
	var c2: float = 0.511664; \
	var c3: float = 0.743125; \
	var c4: float = 0.886227; \
	var c5: float = 0.247708; \
	var cl00: float3 = float3(constants.shirr0.x, constants.shirr0.y, constants.shirr0.z); \
	var cl1m1: float3 = float3(constants.shirr0.w, constants.shirr1.x, constants.shirr1.y); \
	var cl10: float3 = float3(constants.shirr1.z, constants.shirr1.w, constants.shirr2.x); \
	var cl11: float3 = float3(constants.shirr2.y, constants.shirr2.z, constants.shirr2.w); \
	var cl2m2: float3 = float3(constants.shirr3.x, constants.shirr3.y, constants.shirr3.z); \
	var cl2m1: float3 = float3(constants.shirr3.w, constants.shirr4.x, constants.shirr4.y); \
	var cl20: float3 = float3(constants.shirr4.z, constants.shirr4.w, constants.shirr5.x); \
	var cl21: float3 = float3(constants.shirr5.y, constants.shirr5.z, constants.shirr5.w); \
	var cl22: float3 = float3(constants.shirr6.x, constants.shirr6.y, constants.shirr6.z); \
	return ( \
		cl22 * c1 * (nor.y * nor.y - (-nor.z) * (-nor.z)) + \
		cl20 * c3 * nor.x * nor.x + \
		cl00 * c4 - \
		cl20 * c5 + \
		cl2m2 * 2.0 * c1 * nor.y * (-nor.z) + \
		cl21  * 2.0 * c1 * nor.y * nor.x + \
		cl2m1 * 2.0 * c1 * (-nor.z) * nor.x + \
		cl11  * 2.0 * c2 * nor.y + \
		cl1m1 * 2.0 * c2 * (-nor.z) + \
		cl10  * 2.0 * c2 * nor.x \
	); \
} \
";
	str_envmap_equirect        = "\
fun envmap_equirect(normal: float3, angle: float): float2 { \
	var PI: float = 3.1415926535; \
	var PI2: float = PI * 2.0; \
	var phi: float = acos(normal.z); \
	var theta: float = atan2(-normal.y, normal.x) + PI + angle; \
	return float2(theta / PI2, phi / PI); \
} \
";
	str_envmap_sample          = "\
fun envmap_sample(lod: float, coord: float2): float3 { \
	if (lod == 0.0) { \
		return sample_lod(senvmap_radiance, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 1.0) { \
		return sample_lod(senvmap_radiance0, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 2.0) { \
		return sample_lod(senvmap_radiance1, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 3.0) { \
		return sample_lod(senvmap_radiance2, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 4.0) { \
		return sample_lod(senvmap_radiance3, sampler_linear, coord, 0.0).rgb; \
	} \
	return sample_lod(senvmap_radiance4, sampler_linear, coord, 0.0).rgb; \
} \
";

	// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
	str_env_brdf_approx = "\
fun env_brdf_approx(specular: float3, roughness: float, dotnv: float): float3 { \
	var c0: float4 = float4(-1.0, -0.0275, -0.572, 0.022); \
	var c1: float4 = float4(1.0, 0.0425, 1.04, -0.04); \
	var r: float4 = c0 * roughness + c1; \
	var a004: float = min(r.x * r.x, exp((-9.28 * dotnv) * log(2.0))) * r.x + r.y; \
	var ab: float2 = float2(-1.04, 1.04) * a004 + r.zw; \
	return specular * ab.x + ab.y; \
} \
";
	str_dither_bayer    = "\
fun dither_bayer(uv: float2): float { \
	var x: int = int(uv.x % 4.0); \
	var y: int = int(uv.y % 4.0); \
	if (y == 0) { \
		if (x == 0) { \
			return 0.0 / 16.0; \
		} \
		if (x == 1) { \
			return 8.0 / 16.0; \
		} \
		if (x == 2) { \
			return 2.0 / 16.0; \
		} \
		return 10.0 / 16.0; \
	} \
	if (y == 1) { \
		if (x == 0) { \
			return 12.0 / 16.0; \
		} \
		if (x == 1) { \
			return 4.0 / 16.0; \
		} \
		if (x == 2) { \
			return 14.0 / 16.0; \
		} \
		return 6.0 / 16.0; \
	} \
	if (y == 2) { \
		if (x == 0) { \
			return 3.0 / 16.0; \
		} \
		if (x == 1) { \
			return 11.0 / 16.0; \
		} \
		if (x == 2) { \
			return 1.0 / 16.0; \
		} \
		return 9.0 / 16.0; \
	} \
	if (x == 0) { \
		return 15.0 / 16.0; \
	} \
	if (x == 1) { \
		return 7.0 / 16.0; \
	} \
	if (x == 2) { \
		return 13.0 / 16.0; \
	} \
	return 5.0 / 16.0; \
} \
";

	// let str_octahedron_wrap: string = "\
	// fun octahedron_wrap(v: float2): float2 { \
	// 	return (1.0 - abs(v.yx)) * (float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0)); \
	// } \
	// ";

	// let str_pack_float_int16: string = "\
	// fun pack_f32_i16(f: float, i: uint): float { \
	// 	var prec: float = float(1 << 16); \
	// 	var maxi: float = float(1 << 4); \
	// 	var prec_minus_one: float = prec - 1.0; \
	// 	var t1: float = ((prec / maxi) - 1.0) / prec_minus_one; \
	// 	var t2: float = (prec / maxi) / prec_minus_one; \
	// 	return t1 * f + t2 * float(i); \
	// } \
	// ";

	ui_sidebar_default_w_mini = 56;
	ui_sidebar_default_w_full = 280;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
	ui_sidebar_default_w = ui_sidebar_default_w_mini;
#else
	ui_sidebar_default_w = ui_sidebar_default_w_full;
#endif
	ui_sidebar_tabx = 0;
	gc_unroot(ui_sidebar_hminimized);
	ui_sidebar_hminimized = ui_handle_create();
	gc_root(ui_sidebar_hminimized);
	ui_sidebar_w_mini     = ui_sidebar_default_w_mini;
	ui_sidebar_last_tab   = 0;
	console_message       = "";
	console_message_timer = 0.0;
	console_message_color = 0x00000000;
	gc_unroot(console_last_traces);
	console_last_traces = any_array_create_from_raw(
	    (void *[]){
	        "",
	    },
	    1);
	gc_root(console_last_traces);
	console_progress_text = NULL;
	camera_redraws        = 0;
	camera_dir            = vec4_create(0.0, 0.0, 0.0, 1.0);
	camera_ease           = 1.0;
	camera_controls_down  = false;
	gc_unroot(box_preferences_htab);
	box_preferences_htab = ui_handle_create();
	gc_root(box_preferences_htab);
	box_preferences_files_plugin            = NULL;
	box_preferences_files_keymap            = NULL;
	box_preferences_locales                 = NULL;
	box_preferences_themes                  = NULL;
	render_path_paint_live_layer            = NULL;
	render_path_paint_live_layer_drawn      = 0;
	render_path_paint_live_layer_locked     = false;
	render_path_paint_dilated               = true;
	render_path_paint_painto                = NULL;
	render_path_paint_planeo                = NULL;
	render_path_paint_visibles              = NULL;
	render_path_paint_merged_object_visible = false;
	render_path_paint_saved_fov             = 0.0;
	render_path_paint_baking                = false;
	render_path_paint_last_x                = -1.0;
	render_path_paint_last_y                = -1.0;
	export_texture_gamma                    = 1.0 / 2.2;
	ui_box_show                             = false;
	ui_box_draggable                        = true;
	gc_unroot(ui_box_hwnd);
	ui_box_hwnd = ui_handle_create();
	gc_root(ui_box_hwnd);
	ui_box_title                   = "";
	ui_box_text                    = "";
	ui_box_commands                = NULL;
	ui_box_click_to_hide           = true;
	ui_box_modalw                  = 400;
	ui_box_modalh                  = 170;
	ui_box_modal_on_hide           = NULL;
	ui_box_draws                   = 0;
	ui_box_copyable                = false;
	ui_box_tween_alpha             = 0.0;
	tab_scene_line_counter         = 0;
	_tab_scene_paint_object_length = 1;
	tab_layers_layer_name_edit     = -1;
	gc_unroot(tab_layers_layer_name_handle);
	tab_layers_layer_name_handle = ui_handle_create();
	gc_root(tab_layers_layer_name_handle);
	tab_layers_show_context_menu     = false;
	util_uv_uvmap                    = NULL;
	util_uv_uvmap_cached             = false;
	util_uv_trianglemap              = NULL;
	util_uv_trianglemap_cached       = false;
	util_uv_dilatemap                = NULL;
	util_uv_dilatemap_cached         = false;
	util_uv_uvislandmap              = NULL;
	util_uv_uvislandmap_cached       = false;
	util_uv_dilate_bytes             = NULL;
	util_uv_pipe_dilate              = NULL;
	render_path_raytrace_frame       = 0;
	render_path_raytrace_ready       = false;
	render_path_raytrace_dirty       = 0;
	render_path_raytrace_uv_scale    = 1.0;
	render_path_raytrace_init_shader = true;
	gc_unroot(render_path_raytrace_f32a);
	render_path_raytrace_f32a = f32_array_create(24);
	gc_root(render_path_raytrace_f32a);
	render_path_raytrace_help_mat    = mat4_identity();
	render_path_raytrace_last_envmap = NULL;
	render_path_raytrace_is_bake     = false;
#ifdef IRON_DIRECT3D12
	render_path_raytrace_ext = ".cso";
#elif defined(IRON_METAL)
	render_path_raytrace_ext = ".metal";
#else
	render_path_raytrace_ext = ".spirv";
#endif
	render_path_raytrace_last_texpaint = NULL;
	sculpt_push_undo                   = false;
	ui_statusbar_default_h             = 33;
	ui_statusbar_last_tab              = 0;
	import_envmap_pipeline             = NULL;
	import_envmap_params               = vec4_create(0.0, 0.0, 0.0, 1.0);
	import_envmap_n                    = vec4_create(0.0, 0.0, 0.0, 1.0);
	import_envmap_radiance             = NULL;
	import_envmap_mips                 = NULL;
	gc_unroot(image_texture_node_def);
	char *image_texture_color_space_data = _tr("Auto");
	image_texture_color_space_data       = string_join(image_texture_color_space_data, "\n");
	image_texture_color_space_data       = string_join(image_texture_color_space_data, _tr("Linear"));
	image_texture_color_space_data       = string_join(image_texture_color_space_data, "\n");
	image_texture_color_space_data       = string_join(image_texture_color_space_data, _tr("sRGB"));
	image_texture_color_space_data       = string_join(image_texture_color_space_data, "\n");
	image_texture_color_space_data       = string_join(image_texture_color_space_data, _tr("DirectX Normal Map"));
	image_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Image Texture"),
	                              .type   = "TEX_IMAGE",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Alpha"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("File"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(""),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Color Space"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(image_texture_color_space_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  2),
	                              .width = 0,
	                              .flags = 0});
	gc_root(image_texture_node_def);
	gc_unroot(material_node_def);
	material_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                              .name    = _tr("Material"),
	                                              .type    = "MATERIAL", // extension
	                                              .x       = 0,
	                                              .y       = 0,
	                                              .color   = 0xff4982a0,
	                                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                              .outputs = any_array_create_from_raw(
	                                                  (void *[]){
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Base Color"),
	                                                                                       .type          = "RGBA",
	                                                                                       .color         = 0xffc7c729,
	                                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Opacity"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Occlusion"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Roughness"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Metallic"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Normal Map"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = -10238109,
	                                                                                       .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Emission"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Height"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Subsurface"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                  },
	                                                  9),
	                                              .buttons = any_array_create_from_raw(
	                                                  (void *[]){
	                                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Material"),
	                                                                                       .type          = "ENUM",
	                                                                                       .output        = -1,
	                                                                                       .default_value = f32_array_create_x(0),
	                                                                                       .data          = u8_array_create_from_string(""),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .height        = 0}),
	                                                  },
	                                                  1),
	                                              .width = 0,
	                                              .flags = 0});
	gc_root(material_node_def);
	gc_unroot(uv_map_node_def);
	char *uv_map_data = "uv0";
	uv_map_data       = string_join(uv_map_data, "\n");
	uv_map_data       = string_join(uv_map_data, "uv1");
	uv_map_node_def   = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                              .name    = _tr("UV Map"),
	                                              .type    = "UVMAP",
	                                              .x       = 0,
	                                              .y       = 0,
	                                              .color   = 0xffb34f5a,
	                                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                              .outputs = any_array_create_from_raw(
                                                    (void *[]){
                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("UV"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
                                                    },
                                                    1),
	                                              .buttons = any_array_create_from_raw(
                                                    (void *[]){
                                                        GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("UV Map"),
	                                                                                       .type          = "ENUM",
	                                                                                       .output        = -1,
	                                                                                       .default_value = f32_array_create_x(0),
	                                                                                       .data          = u8_array_create_from_string(uv_map_data),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .height        = 0}),
                                                    },
                                                    1),
	                                              .width = 0,
	                                              .flags = 0});
	gc_root(uv_map_node_def);
	str_tex_wave = "\
fun tex_wave_f(p: float3): float { \
	return 1.0 - sin((p.x + p.y) * 10.0); \
} \
";
	gc_unroot(wave_texture_node_def);
	wave_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Wave Texture"),
	                              .type   = "TEX_WAVE",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(wave_texture_node_def);
	gc_unroot(clamp_node_def);
	char *clamp_operation_data = _tr("Min Max");
	clamp_operation_data       = string_join(clamp_operation_data, "\n");
	clamp_operation_data       = string_join(clamp_operation_data, _tr("Range"));
	clamp_node_def             = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                       .name   = _tr("Clamp"),
	                                                       .type   = "CLAMP",
	                                                       .x      = 0,
	                                                       .y      = 0,
	                                                       .color  = 0xff62676d,
	                                                       .inputs = any_array_create_from_raw(
                                                   (void *[]){
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                .node_id       = 0,
	                                                                                                .name          = _tr("Value"),
	                                                                                                .type          = "VALUE",
	                                                                                                .color         = 0xffa1a1a1,
	                                                                                                .default_value = f32_array_create_x(0.5),
	                                                                                                .min           = 0.0,
	                                                                                                .max           = 1.0,
	                                                                                                .precision     = 100,
	                                                                                                .display       = 0}),
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                .node_id       = 0,
	                                                                                                .name          = _tr("Min"),
	                                                                                                .type          = "VALUE",
	                                                                                                .color         = 0xffa1a1a1,
	                                                                                                .default_value = f32_array_create_x(0.0),
	                                                                                                .min           = 0.0,
	                                                                                                .max           = 1.0,
	                                                                                                .precision     = 100,
	                                                                                                .display       = 0}),
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                .node_id       = 0,
	                                                                                                .name          = _tr("Max"),
	                                                                                                .type          = "VALUE",
	                                                                                                .color         = 0xffa1a1a1,
	                                                                                                .default_value = f32_array_create_x(1.0),
	                                                                                                .min           = 0.0,
	                                                                                                .max           = 1.0,
	                                                                                                .precision     = 100,
	                                                                                                .display       = 0}),
                                                   },
                                                   3),
	                                                       .outputs = any_array_create_from_raw(
                                                   (void *[]){
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                .node_id       = 0,
	                                                                                                .name          = _tr("Value"),
	                                                                                                .type          = "VALUE",
	                                                                                                .color         = 0xffa1a1a1,
	                                                                                                .default_value = f32_array_create_x(0.0),
	                                                                                                .min           = 0.0,
	                                                                                                .max           = 1.0,
	                                                                                                .precision     = 100,
	                                                                                                .display       = 0}),
                                                   },
                                                   1),
	                                                       .buttons = any_array_create_from_raw(
                                                   (void *[]){
                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                                                .type          = "ENUM",
	                                                                                                .output        = 0,
	                                                                                                .default_value = f32_array_create_x(0),
	                                                                                                .data          = u8_array_create_from_string(clamp_operation_data),
	                                                                                                .min           = 0.0,
	                                                                                                .max           = 1.0,
	                                                                                                .precision     = 100,
	                                                                                                .height        = 0}),
                                                   },
                                                   1),
	                                                       .width = 0,
	                                                       .flags = 0});
	gc_root(clamp_node_def);
	gc_unroot(tangent_node_def);
	tangent_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                             .name    = _tr("Tangent"),
	                                             .type    = "TANGENT",
	                                             .x       = 0,
	                                             .y       = 0,
	                                             .color   = 0xffb34f5a,
	                                             .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                             .outputs = any_array_create_from_raw(
	                                                 (void *[]){
	                                                     GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                      .node_id       = 0,
	                                                                                      .name          = _tr("Tangent"),
	                                                                                      .type          = "VECTOR",
	                                                                                      .color         = 0xff6363c7,
	                                                                                      .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                      .min           = 0.0,
	                                                                                      .max           = 1.0,
	                                                                                      .precision     = 100,
	                                                                                      .display       = 0}),
	                                                 },
	                                                 1),
	                                             .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                             .width   = 0,
	                                             .flags   = 0});
	gc_root(tangent_node_def);
	parser_material_bake_passthrough_strength = "0.0";
	parser_material_bake_passthrough_radius   = "0.0";
	parser_material_bake_passthrough_offset   = "0.0";
	gc_unroot(curvature_bake_node_def);
	curvature_bake_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Curvature Bake"),
	                              .type   = "BAKE_CURVATURE", // extension
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Strength"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 2.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Radius"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 2.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Offset"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = -2.0,
	                                                                       .max           = 2.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(curvature_bake_node_def);
	gc_unroot(blur_node_def);
	blur_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                          .name   = _tr("Blur"),
	                                          .type   = "BLUR", // extension
	                                          .x      = 0,
	                                          .y      = 0,
	                                          .color  = 0xff448c6d,
	                                          .inputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Color"),
	                                                                                   .type          = "RGBA",
	                                                                                   .color         = 0xffc7c729,
	                                                                                   .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Strength"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.5),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              2),
	                                          .outputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Color"),
	                                                                                   .type          = "RGBA",
	                                                                                   .color         = 0xffc7c729,
	                                                                                   .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              1),
	                                          .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                          .width   = 0,
	                                          .flags   = 0});
	gc_root(blur_node_def);
	gc_unroot(text_texture_node_def);
	text_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Text Texture"),
	                              .type   = "TEX_TEXT", // extension
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Alpha"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "text",
	                                                                       .type          = "STRING",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0), // "",
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(text_texture_node_def);
	gc_unroot(gradient_texture_node_def);
	char *gradient_type_data  = _tr("Linear");
	gradient_type_data        = string_join(gradient_type_data, "\n");
	gradient_type_data        = string_join(gradient_type_data, _tr("Diagonal"));
	gradient_type_data        = string_join(gradient_type_data, "\n");
	gradient_type_data        = string_join(gradient_type_data, _tr("Radial"));
	gradient_type_data        = string_join(gradient_type_data, "\n");
	gradient_type_data        = string_join(gradient_type_data, _tr("Spherical"));
	gradient_texture_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                      .name   = _tr("Gradient Texture"),
	                                                      .type   = "TEX_GRADIENT",
	                                                      .x      = 0,
	                                                      .y      = 0,
	                                                      .color  = 0xff4982a0,
	                                                      .inputs = any_array_create_from_raw(
	                                                          (void *[]){
	                                                              GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                               .node_id       = 0,
	                                                                                               .name          = _tr("Vector"),
	                                                                                               .type          = "VECTOR",
	                                                                                               .color         = 0xff6363c7,
	                                                                                               .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .display       = 0}),
	                                                          },
	                                                          1),
	                                                      .outputs = any_array_create_from_raw(
	                                                          (void *[]){
	                                                              GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                               .node_id       = 0,
	                                                                                               .name          = _tr("Color"),
	                                                                                               .type          = "RGBA",
	                                                                                               .color         = 0xffc7c729,
	                                                                                               .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .display       = 0}),
	                                                              GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                               .node_id       = 0,
	                                                                                               .name          = _tr("Factor"),
	                                                                                               .type          = "VALUE",
	                                                                                               .color         = 0xffa1a1a1,
	                                                                                               .default_value = f32_array_create_x(1.0),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .display       = 0}),
	                                                          },
	                                                          2),
	                                                      .buttons = any_array_create_from_raw(
	                                                          (void *[]){
	                                                              GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("gradient_type"),
	                                                                                               .type          = "ENUM",
	                                                                                               .output        = 0,
	                                                                                               .default_value = f32_array_create_x(0),
	                                                                                               .data      = u8_array_create_from_string(gradient_type_data),
	                                                                                               .min       = 0.0,
	                                                                                               .max       = 1.0,
	                                                                                               .precision = 100,
	                                                                                               .height    = 0}),
	                                                          },
	                                                          1),
	                                                      .width = 0,
	                                                      .flags = 0});
	gc_root(gradient_texture_node_def);
	gc_unroot(object_info_node_def);
	object_info_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                              .name    = _tr("Object Info"),
	                              .type    = "OBJECT_INFO",
	                              .x       = 0,
	                              .y       = 0,
	                              .color   = 0xffb34f5a,
	                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Location"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Alpha"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Object Index"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Material Index"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Random"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  6),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(object_info_node_def);
	str_tex_magic = "\
fun tex_magic(p: float3): float3 { \
	var a: float = 1.0 - (sin(p.x) + sin(p.y)); \
	var b: float = 1.0 - sin(p.x - p.y); \
	var c: float = 1.0 - sin(p.x + p.y); \
	return float3(a, b, c); \
} \
fun tex_magic_f(p: float3): float { \
	var c: float3 = tex_magic(p); \
	return (c.x + c.y + c.z) / 3.0; \
} \
";
	gc_unroot(magic_texture_node_def);
	magic_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Magic Texture"),
	                              .type   = "TEX_MAGIC",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Distortion"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(magic_texture_node_def);
	gc_unroot(fresnel_node_def);
	fresnel_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Fresnel"),
	                              .type   = "FRESNEL",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xffb34f5a,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("IOR"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 3.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(fresnel_node_def);
	gc_unroot(warp_node_def);
	warp_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                          .name   = _tr("Warp"),
	                                          .type   = "DIRECT_WARP", // extension
	                                          .x      = 0,
	                                          .y      = 0,
	                                          .color  = 0xff448c6d,
	                                          .inputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Color"),
	                                                                                   .type          = "RGBA",
	                                                                                   .color         = 0xffc7c729,
	                                                                                   .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Angle"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 360.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Mask"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.5),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              3),
	                                          .outputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Color"),
	                                                                                   .type          = "RGBA",
	                                                                                   .color         = 0xffc7c729,
	                                                                                   .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              1),
	                                          .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                          .width   = 0,
	                                          .flags   = 0});
	gc_root(warp_node_def);
	gc_unroot(normal_node_def);
	normal_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                            .name   = _tr("Normal"),
	                                            .type   = "NORMAL",
	                                            .x      = 0,
	                                            .y      = 0,
	                                            .color  = 0xff522c99,
	                                            .inputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Normal"),
	                                                                                     .type          = "VECTOR",
	                                                                                     .color         = 0xff6363c7,
	                                                                                     .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                1),
	                                            .outputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Normal"),
	                                                                                     .type          = "VECTOR",
	                                                                                     .color         = 0xff6363c7,
	                                                                                     .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Dot"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                2),
	                                            .buttons = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Vector"),
	                                                                                     .type          = "VECTOR",
	                                                                                     .output        = 0,
	                                                                                     .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                     .data          = NULL,
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .height        = 0}),
	                                                },
	                                                1),
	                                            .width = 0,
	                                            .flags = 0});
	gc_root(normal_node_def);
	gc_unroot(mapping_node_def);
	mapping_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Mapping"),
	                              .type   = "MAPPING",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff522c99,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Location"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Rotation"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 360.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(1.0, 1.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                  },
	                                  4),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(mapping_node_def);
	gc_unroot(normal_map_node_def);
	normal_map_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                .name   = _tr("Normal Map"),
	                                                .type   = "NORMAL_MAP",
	                                                .x      = 0,
	                                                .y      = 0,
	                                                .color  = 0xff522c99,
	                                                .inputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Strength"),
	                                                                                         .type          = "VALUE",
	                                                                                         .color         = 0xffa1a1a1,
	                                                                                         .default_value = f32_array_create_x(1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 2.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Normal Map"),
	                                                                                         .type          = "VECTOR",
	                                                                                         .color         = -10238109,
	                                                                                         .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    2),
	                                                .outputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Normal Map"),
	                                                                                         .type          = "VECTOR",
	                                                                                         .color         = -10238109,
	                                                                                         .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                                .width   = 0,
	                                                .flags   = 0});
	gc_root(normal_map_node_def);
	gc_unroot(invert_color_node_def);
	invert_color_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Invert Color"),
	                              .type   = "INVERT_COLOR",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff448c6d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(invert_color_node_def);
	gc_unroot(wireframe_node_def);
	wireframe_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                               .name   = _tr("Wireframe"),
	                                               .type   = "WIREFRAME",
	                                               .x      = 0,
	                                               .y      = 0,
	                                               .color  = 0xffb34f5a,
	                                               .inputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Size"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.01),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 0.1,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   1),
	                                               .outputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Factor"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   1),
	                                               .buttons = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Pixel Size"),
	                                                                                        .type          = "BOOL",
	                                                                                        .output        = 0,
	                                                                                        .default_value = f32_array_create_x(0),
	                                                                                        .data          = NULL,
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .height        = 0}),
	                                                   },
	                                                   1),
	                                               .width = 0,
	                                               .flags = 0});
	gc_root(wireframe_node_def);
	gc_unroot(gamma_node_def);
	gamma_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                           .name   = _tr("Gamma"),
	                                           .type   = "GAMMA",
	                                           .x      = 0,
	                                           .y      = 0,
	                                           .color  = 0xff448c6d,
	                                           .inputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Color"),
	                                                                                    .type          = "RGBA",
	                                                                                    .color         = 0xffc7c729,
	                                                                                    .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Gamma"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               2),
	                                           .outputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Color"),
	                                                                                    .type          = "RGBA",
	                                                                                    .color         = 0xffc7c729,
	                                                                                    .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               1),
	                                           .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                           .width   = 0,
	                                           .flags   = 0});
	gc_root(gamma_node_def);
	gc_unroot(vector_math2_node_def);
	char *vector_math_operation_data = _tr("Add");
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Subtract"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Multiply"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Divide"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Average"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Cross Product"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Project"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Reflect"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Dot Product"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Distance"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Length"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Scale"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Normalize"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Absolute"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Minimum"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Maximum"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Floor"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Ceil"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Fraction"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Modulo"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Snap"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Sine"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Cosine"));
	vector_math_operation_data       = string_join(vector_math_operation_data, "\n");
	vector_math_operation_data       = string_join(vector_math_operation_data, _tr("Tangent"));
	vector_math2_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Vector Math"),
	                              .type   = "VECT_MATH",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff62676d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(vector_math_operation_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(vector_math2_node_def);
	str_tex_checker = "\
fun tex_checker(co: float3, col1: float3, col2: float3, scale: float): float3 { \
	/* Prevent precision issues on unit coordinates */ \
	var p: float3 = (co + 0.000001 * 0.999999) * scale; \
	var xi: float = abs(floor(p.x)); \
	var yi: float = abs(floor(p.y)); \
	var zi: float = abs(floor(p.z)); \
	/* var check: bool = (xi % 2.0 == yi % 2.0) == zi % 2.0;*/ \
	var checka: int = 0; \
	var checkb: int = 0; \
	if (xi % 2.0 == yi % 2.0) { checka = 1; } \
	if (zi % 2.0 != 0.0) { checkb = 1; } \
	if (checka == checkb) { return col1; } return col2; \
} \
fun tex_checker_f(co: float3, scale: float): float { \
	var p: float3 = (co + 0.000001 * 0.999999) * scale; \
	var xi: float = abs(floor(p.x)); \
	var yi: float = abs(floor(p.y)); \
	var zi: float = abs(floor(p.z)); \
	/*return float((xi % 2.0 == yi % 2.0) == zi % 2.0);*/ \
	var checka: int = 0; \
	var checkb: int = 0; \
	if (xi % 2.0 == yi % 2.0) { checka = 1; } \
	if (zi % 2.0 != 0.0) { checkb = 1; } \
	if (checka == checkb) { return 1.0; } return 0.0; \
	\
} \
";
	gc_unroot(checker_texture_node_def);
	checker_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Checker Texture"),
	                              .type   = "TEX_CHECKER",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color 1"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.8, 0.8, 0.8),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color 2"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.2, 0.2, 0.2),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  4),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(checker_texture_node_def);
	str_hue_sat = "\
fun hsv_to_rgb(c: float3): float3 { \
	var K: float4 = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0); \
	var p: float3 = abs3(frac3(c.xxx + K.xyz) * 6.0 - K.www); \
	return lerp3(K.xxx, clamp3(p - K.xxx, float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0)), c.y) * c.z; \
} \
fun rgb_to_hsv(c: float3): float3 { \
	var K: float4 = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0); \
	var p: float4 = lerp4(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g)); \
	var q: float4 = lerp4(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r)); \
	var d: float = q.x - min(q.w, q.y); \
	var e: float = 0.0000000001; \
	return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x); \
} \
fun hue_sat(col: float3, shift: float4): float3 { \
	var hsv: float3 = rgb_to_hsv(col); \
	hsv.x += shift.x; \
	hsv.y *= shift.y; \
	hsv.z *= shift.z; \
	return lerp3(hsv_to_rgb(hsv), col, shift.w); \
} \
";
	gc_unroot(hue_saturation_value_node_def);
	hue_saturation_value_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Hue/Saturation/Value"),
	                              .type   = "HUE_SAT",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff448c6d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Hue"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Saturation"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  5),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(hue_saturation_value_node_def);
	gc_unroot(separate_xyz_node_def);
	separate_xyz_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Separate XYZ"),
	                              .type   = "SEPXYZ",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff62676d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("X"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Y"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Z"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(separate_xyz_node_def);
	gc_unroot(math2_node_def);
	char *math_operation_data = _tr("Add");
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Subtract"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Multiply"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Divide"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Power"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Logarithm"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Square Root"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Inverse Square Root"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Absolute"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Exponent"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Minimum"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Maximum"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Less Than"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Greater Than"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Sign"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Round"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Floor"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Ceil"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Truncate"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Fraction"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Modulo"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Snap"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Ping-Pong"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Sine"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Cosine"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Tangent"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Arcsine"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Arccosine"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Arctangent"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Arctan2"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Hyperbolic Sine"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Hyperbolic Cosine"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("Hyperbolic Tangent"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("To Radians"));
	math_operation_data       = string_join(math_operation_data, "\n");
	math_operation_data       = string_join(math_operation_data, _tr("To Degrees"));
	math2_node_def            = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                      .name   = _tr("Math"),
	                                                      .type   = "MATH",
	                                                      .x      = 0,
	                                                      .y      = 0,
	                                                      .color  = 0xff62676d,
	                                                      .inputs = any_array_create_from_raw(
                                                   (void *[]){
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                               .node_id       = 0,
	                                                                                               .name          = _tr("Value"),
	                                                                                               .type          = "VALUE",
	                                                                                               .color         = 0xffa1a1a1,
	                                                                                               .default_value = f32_array_create_x(0.5),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .display       = 0}),
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                               .node_id       = 0,
	                                                                                               .name          = _tr("Value"),
	                                                                                               .type          = "VALUE",
	                                                                                               .color         = 0xffa1a1a1,
	                                                                                               .default_value = f32_array_create_x(0.5),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .display       = 0}),
                                                   },
                                                   2),
	                                                      .outputs = any_array_create_from_raw(
                                                   (void *[]){
                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                               .node_id       = 0,
	                                                                                               .name          = _tr("Value"),
	                                                                                               .type          = "VALUE",
	                                                                                               .color         = 0xffa1a1a1,
	                                                                                               .default_value = f32_array_create_x(0.0),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .display       = 0}),
                                                   },
                                                   1),
	                                                      .buttons = any_array_create_from_raw(
                                                   (void *[]){
                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                                               .type          = "ENUM",
	                                                                                               .output        = 0,
	                                                                                               .default_value = f32_array_create_x(0),
	                                                                                               .data          = u8_array_create_from_string(math_operation_data),
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .height        = 0}),
                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Clamp"),
	                                                                                               .type          = "BOOL",
	                                                                                               .output        = 0,
	                                                                                               .default_value = f32_array_create_x(0),
	                                                                                               .data          = NULL,
	                                                                                               .min           = 0.0,
	                                                                                               .max           = 1.0,
	                                                                                               .precision     = 100,
	                                                                                               .height        = 0}),
                                                   },
                                                   2),
	                                                      .width = 0,
	                                                      .flags = 0});
	gc_root(math2_node_def);
	gc_unroot(geometry_node_def);
	geometry_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                              .name    = _tr("Geometry"),
	                                              .type    = "NEW_GEOMETRY",
	                                              .x       = 0,
	                                              .y       = 0,
	                                              .color   = 0xffb34f5a,
	                                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                              .outputs = any_array_create_from_raw(
	                                                  (void *[]){
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Position"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Normal"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Tangent"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("True Normal"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Incoming"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Parametric"),
	                                                                                       .type          = "VECTOR",
	                                                                                       .color         = 0xff6363c7,
	                                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Backfacing"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Pointiness"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Random Per Island"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(0.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                  },
	                                                  9),
	                                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                              .width   = 0,
	                                              .flags   = 0});
	gc_root(geometry_node_def);
	gc_unroot(mix_color_node_def);
	char *mix_color_blend_type_data = _tr("Mix");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Darken"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Multiply"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Burn"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Lighten"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Screen"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Dodge"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Add"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Overlay"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Soft Light"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Linear Light"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Difference"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Subtract"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Divide"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Hue"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Saturation"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Color"));
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, "\n");
	mix_color_blend_type_data       = string_join(mix_color_blend_type_data, _tr("Value"));
	mix_color_node_def              = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                            .name   = _tr("Mix Color"),
	                                                            .type   = "MIX_RGB",
	                                                            .x      = 0,
	                                                            .y      = 0,
	                                                            .color  = 0xff448c6d,
	                                                            .inputs = any_array_create_from_raw(
                                                       (void *[]){
                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                     .node_id       = 0,
	                                                                                                     .name          = _tr("Factor"),
	                                                                                                     .type          = "VALUE",
	                                                                                                     .color         = 0xffa1a1a1,
	                                                                                                     .default_value = f32_array_create_x(0.5),
	                                                                                                     .min           = 0.0,
	                                                                                                     .max           = 1.0,
	                                                                                                     .precision     = 100,
	                                                                                                     .display       = 0}),
                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                     .node_id       = 0,
	                                                                                                     .name          = _tr("Color 1"),
	                                                                                                     .type          = "RGBA",
	                                                                                                     .color         = 0xffc7c729,
	                                                                                                     .default_value = f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
	                                                                                                     .min           = 0.0,
	                                                                                                     .max           = 1.0,
	                                                                                                     .precision     = 100,
	                                                                                                     .display       = 0}),
                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                     .node_id       = 0,
	                                                                                                     .name          = _tr("Color 2"),
	                                                                                                     .type          = "RGBA",
	                                                                                                     .color         = 0xffc7c729,
	                                                                                                     .default_value = f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
	                                                                                                     .min           = 0.0,
	                                                                                                     .max           = 1.0,
	                                                                                                     .precision     = 100,
	                                                                                                     .display       = 0}),
                                                       },
                                                       3),
	                                                            .outputs = any_array_create_from_raw(
                                                       (void *[]){
                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                                     .node_id       = 0,
	                                                                                                     .name          = _tr("Color"),
	                                                                                                     .type          = "RGBA",
	                                                                                                     .color         = 0xffc7c729,
	                                                                                                     .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                                     .min           = 0.0,
	                                                                                                     .max           = 1.0,
	                                                                                                     .precision     = 100,
	                                                                                                     .display       = 0}),
                                                       },
                                                       1),
	                                                            .buttons = any_array_create_from_raw(
                                                       (void *[]){
                                                           GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("blend_type"),
	                                                                                                     .type          = "ENUM",
	                                                                                                     .output        = 0,
	                                                                                                     .default_value = f32_array_create_x(0),
	                                                                                                     .data      = u8_array_create_from_string(mix_color_blend_type_data),
	                                                                                                     .min       = 0.0,
	                                                                                                     .max       = 1.0,
	                                                                                                     .precision = 100,
	                                                                                                     .height    = 0}),
                                                           GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Clamp"),
	                                                                                                     .type          = "BOOL",
	                                                                                                     .output        = 0,
	                                                                                                     .default_value = f32_array_create_x(0),
	                                                                                                     .data          = NULL,
	                                                                                                     .min           = 0.0,
	                                                                                                     .max           = 1.0,
	                                                                                                     .precision     = 100,
	                                                                                                     .height        = 0}),
                                                       },
                                                       2),
	                                                            .width = 0,
	                                                            .flags = 0});
	gc_root(mix_color_node_def);
	gc_unroot(quantize_node_def);
	quantize_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                              .name   = _tr("Quantize"),
	                                              .type   = "QUANTIZE", // extension
	                                              .x      = 0,
	                                              .y      = 0,
	                                              .color  = 0xff448c6d,
	                                              .inputs = any_array_create_from_raw(
	                                                  (void *[]){
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Strength"),
	                                                                                       .type          = "VALUE",
	                                                                                       .color         = 0xffa1a1a1,
	                                                                                       .default_value = f32_array_create_x(0.1),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Color"),
	                                                                                       .type          = "RGBA",
	                                                                                       .color         = 0xffc7c729,
	                                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                  },
	                                                  2),
	                                              .outputs = any_array_create_from_raw(
	                                                  (void *[]){
	                                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                       .node_id       = 0,
	                                                                                       .name          = _tr("Color"),
	                                                                                       .type          = "RGBA",
	                                                                                       .color         = 0xffc7c729,
	                                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                       .min           = 0.0,
	                                                                                       .max           = 1.0,
	                                                                                       .precision     = 100,
	                                                                                       .display       = 0}),
	                                                  },
	                                                  1),
	                                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                              .width   = 0,
	                                              .flags   = 0});
	gc_root(quantize_node_def);
	gc_unroot(layer_node_def);
	layer_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                           .name    = _tr("Layer"),
	                                           .type    = "LAYER", // extension
	                                           .x       = 0,
	                                           .y       = 0,
	                                           .color   = 0xff4982a0,
	                                           .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                           .outputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Base Color"),
	                                                                                    .type          = "RGBA",
	                                                                                    .color         = 0xffc7c729,
	                                                                                    .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Opacity"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Occlusion"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Roughness"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Metallic"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Normal Map"),
	                                                                                    .type          = "VECTOR",
	                                                                                    .color         = -10238109,
	                                                                                    .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Emission"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Height"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Subsurface"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(1.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               9),
	                                           .buttons = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Layer"),
	                                                                                    .type          = "ENUM",
	                                                                                    .output        = -1,
	                                                                                    .default_value = f32_array_create_x(0),
	                                                                                    .data          = u8_array_create_from_string(""),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .height        = 0}),
	                                               },
	                                               1),
	                                           .width = 0,
	                                           .flags = 0});
	gc_root(layer_node_def);
	gc_unroot(map_range_node_def);
	map_range_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                               .name   = _tr("Map Range"),
	                                               .type   = "MAPRANGE",
	                                               .x      = 0,
	                                               .y      = 0,
	                                               .color  = 0xff62676d,
	                                               .inputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Value"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.5),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("From Min"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("From Max"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(1.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("To Min"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("To Max"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(1.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   5),
	                                               .outputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Value"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   1),
	                                               .buttons = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Clamp"),
	                                                                                        .type          = "BOOL",
	                                                                                        .output        = 0,
	                                                                                        .default_value = f32_array_create_x(0),
	                                                                                        .data          = NULL,
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .height        = 0}),
	                                                   },
	                                                   1),
	                                               .width = 0,
	                                               .flags = 0});
	gc_root(map_range_node_def);
	str_tex_voronoi = "\
fun tex_voronoi(x: float3): float4 { \
	var p: float3 = floor3(x); \
	var f: float3 = frac3(x); \
	var id: float = 0.0; \
	var res: float = 100.0; \
	for (var k: int = 0; k <= 2; k += 1) \
	for (var j: int = 0; j <= 2; j += 1) \
	for (var i: int = 0; i <= 2; i += 1) { \
		var b: float3 = float3(float(i - 1), float(j - 1), float(k - 1)); \
		var pb: float3 = p + b; \
		var snoise_sample: float3 = sample(snoise256, sampler_linear, (pb.xy + float2(3.0, 1.0) * pb.z + 0.5) / 256.0).xyz; \
		var r: float3 = b - f + snoise_sample; \
		var d: float = dot(r, r); \
		if (d < res) { \
			id = dot(p + b, float3(1.0, 57.0, 113.0)); \
			res = d; \
		} \
	} \
	/*var col: float3 = 0.5 + 0.5 * cos(id * 0.35 + float3(0.0, 1.0, 2.0));*/ \
	var col: float3; \
	col.x = 0.5 + 0.5 * cos(id * 0.35 + 0.0); \
	col.y = 0.5 + 0.5 * cos(id * 0.35 + 1.0); \
	col.z = 0.5 + 0.5 * cos(id * 0.35 + 2.0); \
	return float4(col, sqrt(res)); \
} \
";
	gc_unroot(voronoi_texture_node_def);
	char *voronoi_coloring_data = _tr("Intensity");
	voronoi_coloring_data       = string_join(voronoi_coloring_data, "\n");
	voronoi_coloring_data       = string_join(voronoi_coloring_data, _tr("Cells"));
	voronoi_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Voronoi Texture"),
	                              .type   = "TEX_VORONOI",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("coloring"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(voronoi_coloring_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(voronoi_texture_node_def);
	gc_unroot(color_ramp_node_def);
	color_ramp_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                .name   = _tr("Color Ramp"),
	                                                .type   = "VALTORGB",
	                                                .x      = 0,
	                                                .y      = 0,
	                                                .color  = 0xff62676d,
	                                                .inputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Factor"),
	                                                                                         .type          = "VALUE",
	                                                                                         .color         = 0xffa1a1a1,
	                                                                                         .default_value = f32_array_create_x(0.5),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .outputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Alpha"),
	                                                                                         .type          = "VALUE",
	                                                                                         .color         = 0xffa1a1a1,
	                                                                                         .default_value = f32_array_create_x(0.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    2),
	                                                .buttons = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_button_t, {.name          = "nodes_material_color_ramp_button",
	                                                                                         .type          = "CUSTOM",
	                                                                                         .output        = 0,
	                                                                                         .default_value = f32_array_create_xyzwv(1.0, 1.0, 1.0, 1.0, 0.0),
	                                                                                         .data          = u8_array_create(1),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .height        = 4.5}),
	                                                    },
	                                                    1),
	                                                .width = 0,
	                                                .flags = 0});
	gc_root(color_ramp_node_def);
	gc_unroot(picker_node_def);
	picker_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                            .name    = _tr("Picker"),
	                                            .type    = "PICKER", // extension
	                                            .x       = 0,
	                                            .y       = 0,
	                                            .color   = 0xff4982a0,
	                                            .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                            .outputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Base Color"),
	                                                                                     .type          = "RGBA",
	                                                                                     .color         = 0xffc7c729,
	                                                                                     .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Opacity"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Occlusion"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Roughness"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Metallic"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Normal Map"),
	                                                                                     .type          = "VECTOR",
	                                                                                     .color         = -10238109,
	                                                                                     .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Emission"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Height"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Subsurface"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                9),
	                                            .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                            .width   = 0,
	                                            .flags   = 0});
	gc_root(picker_node_def);
	gc_unroot(layer_weight_node_def);
	layer_weight_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Layer Weight"),
	                              .type   = "LAYER_WEIGHT",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xffb34f5a,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Blend"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Fresnel"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Facing"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(layer_weight_node_def);
	gc_unroot(replace_color_node_def);
	replace_color_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Replace Color"),
	                              .type   = "REPLACECOL", // extension
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff448c6d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Old Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("New Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Radius"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.1),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.74,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Fuzziness"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  5),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0)});
	gc_root(replace_color_node_def);
	gc_unroot(bump_node_def);
	bump_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                          .name   = _tr("Bump"),
	                                          .type   = "BUMP",
	                                          .x      = 0,
	                                          .y      = 0,
	                                          .color  = 0xff522c99,
	                                          .inputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Strength"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(1.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Distance"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Height"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(1.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Normal"),
	                                                                                   .type          = "VECTOR",
	                                                                                   .color         = 0xff6363c7,
	                                                                                   .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              4),
	                                          .outputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Normal Map"),
	                                                                                   .type          = "VECTOR",
	                                                                                   .color         = -10238109,
	                                                                                   .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              1),
	                                          .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                          .width   = 0,
	                                          .flags   = 0});
	gc_root(bump_node_def);
	str_tex_gabor = "\
fun gabor_hash3(p: float3): float3 { \
	var q: float3 = float3(dot(p, float3(127.1, 311.7, 74.7)), \
						   dot(p, float3(269.5, 183.3, 246.1)), \
						   dot(p, float3(113.5, 271.9, 124.6))); \
	return frac3(float3(sin(q.x) * 43758.5453, sin(q.y) * 43758.5453, sin(q.z) * 43758.5453)); \
} \
fun gabor_hash1(p: float3): float { \
	return frac(sin(dot(p, float3(12.9898, 78.233, 53.539))) * 43758.5453); \
} \
fun gabor_random_unit_vector(p: float3): float3 { \
	var h1: float = gabor_hash1(p); \
	var h2: float = gabor_hash1(p + float3(1.1, 1.1, 1.1)); \
	var theta: float = acos(2.0 * h1 - 1.0); \
	var phi: float = 2.0 * 3.14159 * h2; \
	var sin_theta: float = sin(theta); \
	return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta)); \
} \
fun tex_gabor(co: float3, scale: float, frequency: float, anisotropy: float, orientation: float3): float3 { \
	var p: float3 = co * scale; \
	var ip: float3 = floor3(p); \
	var fp: float3 = frac3(p); \
	var value: float = 0.0; \
	var intensity: float = 0.0; \
	var phase_sin: float = 0.0; \
	var phase_cos: float = 0.0; \
	var pi: float = 3.14159; \
	var a: float = 1.0; \
	for (var k: int = 0; k <= 2; k += 1) { \
		for (var j: int = 0; j <= 2; j += 1) { \
			for (var i: int = 0; i <= 2; i += 1) { \
				var b: float3 = float3(float(i - 1), float(j - 1), float(k - 1)); \
				var h: float3 = gabor_hash3(ip + b); \
				var r: float3 = b - fp + h; \
				var dir: float3 = normalize(orientation); \
				if (anisotropy < 1.0) { \
					var hr_p: float3 = ip + b + float3(2.2, 2.2, 2.2);\
					var hr: float = gabor_hash1(hr_p); \
					if (hr > anisotropy) { \
						var dir_p: float3 = ip + b + float3(3.3, 3.3, 3.3);\
						dir = gabor_random_unit_vector(dir_p); \
					} \
				} \
				var dot_rd: float = dot(r, dir); \
				var r_parallel: float3 = dot_rd * dir; \
				var r_perp: float3 = r - r_parallel; \
				var a_parallel: float = a * (1.0 - anisotropy) + 0.001; \
				var a_perp: float = a; \
				var d_eff: float = a_parallel * a_parallel * dot(r_parallel, r_parallel) + a_perp * a_perp * dot(r_perp, r_perp); \
				var g: float = exp(-pi * d_eff); \
				var random_phase: float = 2.0 * pi * gabor_hash1(ip + b + float3(1.1, 1.1, 1.1)); \
				var theta: float = 2.0 * pi * frequency * dot_rd + random_phase; \
				value += g * sin(theta); \
				intensity += g; \
				phase_sin += sin(theta); \
				phase_cos += cos(theta); \
			} \
		} \
	} \
	value = value * 0.5 + 0.5; \
	intensity = intensity * 0.5 + 0.5; \
	var phase: float = atan2(phase_sin, phase_cos) / (2.0 * pi) + 0.5; \
	return float3(value, phase, intensity); \
} \
";
	gc_unroot(gabor_texture_node_def);
	char *gabor_dimensions_data = _tr("2D");
	gabor_dimensions_data       = string_join(gabor_dimensions_data, "\n");
	gabor_dimensions_data       = string_join(gabor_dimensions_data, _tr("3D"));
	gabor_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Gabor Texture"),
	                              .type   = "TEX_GABOR",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Frequency"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Anisotropy"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Orientation"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(1.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  5),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Phase"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Intensity"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Dimensions"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(1),
	                                                                       .data          = u8_array_create_from_string(gabor_dimensions_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(gabor_texture_node_def);
	gc_unroot(mix_normal_map_node_def);
	char *mix_normal_map_blend_type_data = _tr("Partial Derivative");
	mix_normal_map_blend_type_data       = string_join(mix_normal_map_blend_type_data, "\n");
	mix_normal_map_blend_type_data       = string_join(mix_normal_map_blend_type_data, _tr("Whiteout"));
	mix_normal_map_blend_type_data       = string_join(mix_normal_map_blend_type_data, "\n");
	mix_normal_map_blend_type_data       = string_join(mix_normal_map_blend_type_data, _tr("Reoriented"));
	mix_normal_map_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Mix Normal Map"),
	                              .type   = "MIX_NORMAL_MAP", // extension
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff522c99,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal Map 1"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = -10238109,
	                                                                       .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal Map 2"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = -10238109,
	                                                                       .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal Map"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = -10238109,
	                                                                       .default_value = f32_array_create_xyz(0.5, 0.5, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("blend_type"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(mix_normal_map_blend_type_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(mix_normal_map_node_def);
	gc_unroot(camera_data_node_def);
	camera_data_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                              .name    = _tr("Camera Data"),
	                              .type    = "CAMERA",
	                              .x       = 0,
	                              .y       = 0,
	                              .color   = 0xffb34f5a,
	                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("View Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("View Z Depth"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("View Distance"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(camera_data_node_def);
	gc_unroot(script_node_def);
	script_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                            .name    = _tr("Script"),
	                                            .type    = "SCRIPT_CPU", // extension
	                                            .x       = 0,
	                                            .y       = 0,
	                                            .color   = 0xffb34f5a,
	                                            .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                            .outputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Value"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(0.5),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                1),
	                                            .buttons = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_button_t, {.name          = " ",
	                                                                                     .type          = "STRING",
	                                                                                     .output        = -1,
	                                                                                     .default_value = f32_array_create_x(0), // "",
	                                                                                     .data          = NULL,
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .height        = 0}),
	                                                },
	                                                1),
	                                            .width = 0,
	                                            .flags = 0});
	gc_root(script_node_def);
	gc_unroot(combine_xyz_node_def);
	combine_xyz_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Combine XYZ"),
	                              .type   = "COMBXYZ",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff62676d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("X"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Y"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Z"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(combine_xyz_node_def);
	str_tex_noise = "\
fun hash(n: float): float { return frac(sin(n) * 10000.0); } \
fun tex_noise_f(x: float3): float { \
    var step: float3 = float3(110.0, 241.0, 171.0); \
    var i: float3 = floor3(x); \
    var f: float3 = frac3(x); \
    var n: float = dot(i, step); \
    var u: float3 = f * f * (3.0 - 2.0 * f); \
    return lerp(lerp(lerp(hash(n + dot(step, float3(0.0, 0.0, 0.0))), hash(n + dot(step, float3(1.0, 0.0, 0.0))), u.x), \
                     lerp(hash(n + dot(step, float3(0.0, 1.0, 0.0))), hash(n + dot(step, float3(1.0, 1.0, 0.0))), u.x), u.y), \
                lerp(lerp(hash(n + dot(step, float3(0.0, 0.0, 1.0))), hash(n + dot(step, float3(1.0, 0.0, 1.0))), u.x), \
                     lerp(hash(n + dot(step, float3(0.0, 1.0, 1.0))), hash(n + dot(step, float3(1.0, 1.0, 1.0))), u.x), u.y), u.z); \
} \
fun tex_noise(p: float3): float { \
	p = p * 1.25; \
	var f: float = 0.5 * tex_noise_f(p); p = p * 2.01; \
	f += 0.25 * tex_noise_f(p); p = p * 2.02; \
	f += 0.125 * tex_noise_f(p); p = p * 2.03; \
	f += 0.0625 * tex_noise_f(p); \
	return 1.0 - f; \
} \
";
	gc_unroot(noise_texture_node_def);
	noise_texture_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                   .name   = _tr("Noise Texture"),
	                                                   .type   = "TEX_NOISE",
	                                                   .x      = 0,
	                                                   .y      = 0,
	                                                   .color  = 0xff4982a0,
	                                                   .inputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Vector"),
	                                                                                            .type          = "VECTOR",
	                                                                                            .color         = 0xff6363c7,
	                                                                                            .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Scale"),
	                                                                                            .type          = "VALUE",
	                                                                                            .color         = 0xffa1a1a1,
	                                                                                            .default_value = f32_array_create_x(5.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 10.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       2),
	                                                   .outputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Color"),
	                                                                                            .type          = "RGBA",
	                                                                                            .color         = 0xffc7c729,
	                                                                                            .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Factor"),
	                                                                                            .type          = "VALUE",
	                                                                                            .color         = 0xffa1a1a1,
	                                                                                            .default_value = f32_array_create_x(1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       2),
	                                                   .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                                   .width   = 0,
	                                                   .flags   = 0});
	gc_root(noise_texture_node_def);
	str_brightcontrast = "\
fun brightcontrast(col: float3, bright: float, contr: float): float3 { \
	var a: float = 1.0 + contr; \
	var b: float = bright - contr * 0.5; \
	return max3(a * col + b, float3(0.0, 0.0, 0.0)); \
} \
";
	gc_unroot(brightness_contrast_node_def);
	brightness_contrast_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Brightness/Contrast"),
	                              .type   = "BRIGHTCONTRAST",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff448c6d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Brightness"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Contrast"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(brightness_contrast_node_def);
	gc_unroot(color_mask_node_def);
	color_mask_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Color Mask"),
	                              .type   = "COLMASK", // extension
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff62676d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Mask Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Radius"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.1),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.74,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Fuzziness"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  4),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Mask"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(color_mask_node_def);
	gc_unroot(vector_curves_node_def);
	vector_curves_node_def = GC_ALLOC_INIT(
	    ui_node_t,
	    {.id     = 0,
	     .name   = _tr("Vector Curves"),
	     .type   = "CURVE_VEC",
	     .x      = 0,
	     .y      = 0,
	     .color  = 0xff522c99,
	     .inputs = any_array_create_from_raw(
	         (void *[]){
	             GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                              .node_id       = 0,
	                                              .name          = _tr("Factor"),
	                                              .type          = "VALUE",
	                                              .color         = 0xffa1a1a1,
	                                              .default_value = f32_array_create_x(1.0),
	                                              .min           = 0.0,
	                                              .max           = 1.0,
	                                              .precision     = 100,
	                                              .display       = 0}),
	             GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                              .node_id       = 0,
	                                              .name          = _tr("Vector"),
	                                              .type          = "VECTOR",
	                                              .color         = 0xff6363c7,
	                                              .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                              .min           = 0.0,
	                                              .max           = 1.0,
	                                              .precision     = 100,
	                                              .display       = 0}),
	         },
	         2),
	     .outputs = any_array_create_from_raw(
	         (void *[]){
	             GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                              .node_id       = 0,
	                                              .name          = _tr("Vector"),
	                                              .type          = "VECTOR",
	                                              .color         = 0xff6363c7,
	                                              .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                              .min           = 0.0,
	                                              .max           = 1.0,
	                                              .precision     = 100,
	                                              .display       = 0}),
	         },
	         1),
	     .buttons = any_array_create_from_raw(
	         (void *[]){
	             GC_ALLOC_INIT(ui_node_button_t, {.name          = "nodes_material_vector_curves_button",
	                                              .type          = "CUSTOM",
	                                              .output        = 0,
	                                              .default_value = f32_array_create(96 + 3), // x - [0, 32], y - [33, 64], z - [65, 96], x_len, y_len, z_len
	                                              .data          = NULL,
	                                              .min           = 0.0,
	                                              .max           = 1.0,
	                                              .precision     = 100,
	                                              .height        = 8.5}),
	         },
	         1),
	     .width = 0,
	     .flags = 0});
	gc_root(vector_curves_node_def);
	gc_unroot(layer_mask_node_def);
	layer_mask_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                                .name    = _tr("Layer Mask"),
	                                                .type    = "LAYER_MASK", // extension
	                                                .x       = 0,
	                                                .y       = 0,
	                                                .color   = 0xff4982a0,
	                                                .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                                .outputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Value"),
	                                                                                         .type          = "VALUE",
	                                                                                         .color         = 0xffa1a1a1,
	                                                                                         .default_value = f32_array_create_x(0.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .buttons = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Layer"),
	                                                                                         .type          = "ENUM",
	                                                                                         .output        = -1,
	                                                                                         .default_value = f32_array_create_x(0),
	                                                                                         .data          = u8_array_create_from_string(""),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .height        = 0}),
	                                                    },
	                                                    1),
	                                                .width = 0,
	                                                .flags = 0});
	gc_root(layer_mask_node_def);
	gc_unroot(camera_texture_node_def);
	camera_texture_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                                    .name    = _tr("Camera Texture"),
	                                                    .type    = "TEX_CAMERA", // extension
	                                                    .x       = 0,
	                                                    .y       = 0,
	                                                    .color   = 0xff4982a0,
	                                                    .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                                    .outputs = any_array_create_from_raw(
	                                                        (void *[]){
	                                                            GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                             .node_id       = 0,
	                                                                                             .name          = _tr("Color"),
	                                                                                             .type          = "RGBA",
	                                                                                             .color         = 0xffc7c729,
	                                                                                             .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                             .min           = 0.0,
	                                                                                             .max           = 1.0,
	                                                                                             .precision     = 100,
	                                                                                             .display       = 0}),
	                                                        },
	                                                        1),
	                                                    .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                                    .width   = 0,
	                                                    .flags   = 0});
	gc_root(camera_texture_node_def);
	gc_unroot(value_node_def);
	value_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                           .name    = _tr("Value"),
	                                           .type    = "VALUE",
	                                           .x       = 0,
	                                           .y       = 0,
	                                           .color   = 0xffb34f5a,
	                                           .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                           .outputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Value"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(0.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               1),
	                                           .buttons = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("default_value"),
	                                                                                    .type          = "VALUE",
	                                                                                    .output        = 0,
	                                                                                    .default_value = f32_array_create_x(0),
	                                                                                    .data          = NULL,
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 10.0,
	                                                                                    .precision     = 100,
	                                                                                    .height        = 0}),
	                                               },
	                                               1),
	                                           .width = 0,
	                                           .flags = 0});
	gc_root(value_node_def);
	gc_unroot(texture_coordinate_node_def);
	texture_coordinate_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                              .name    = _tr("Texture Coordinate"),
	                              .type    = "TEX_COORD",
	                              .x       = 0,
	                              .y       = 0,
	                              .color   = 0xffb34f5a,
	                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Generated"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("UV"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Object"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Camera"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Window"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Reflection"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  7),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0)});
	gc_root(texture_coordinate_node_def);
	gc_unroot(group_node_def);
	group_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                           .name    = _tr("New Group"),
	                                           .type    = "GROUP",
	                                           .x       = 0,
	                                           .y       = 0,
	                                           .color   = 0xffb34f5a,
	                                           .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                           .outputs = any_array_create_from_raw((void *[]){}, 0),
	                                           .buttons = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_button_t, {.name          = "nodes_material_new_group_button",
	                                                                                    .type          = "CUSTOM",
	                                                                                    .output        = -1,
	                                                                                    .default_value = f32_array_create_x(0),
	                                                                                    .data          = NULL,
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .height        = 1}),
	                                               },
	                                               1),
	                                           .width = 0,
	                                           .flags = 0});
	gc_root(group_node_def);
	str_tex_brick = "\
fun tex_brick_noise(n: int): float { \
	var nn: int; \
	n = (n >> 13) ^ n; \
	/*nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;*/ \
	nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 2147483647; \
	return 0.5 * float(nn) / 1073741824.0; \
} \
fun tex_brick(p: float3, c1: float3, c2: float3, c3: float3): float3 { \
	var brick_size: float3 = float3(0.9, 0.49, 0.49); \
	var mortar_size: float3 = float3(0.05, 0.1, 0.1); \
	p /= brick_size / 2.0; \
	if (frac(p.y * 0.5) > 0.5) { p.x += 0.5; } \
	var col: float = floor(p.x / (brick_size.x + (mortar_size.x * 2.0))); \
	var row: float = p.y; \
	p = frac3(p); \
	var b: float3 = step3(p, 1.0 - mortar_size); \
	/*var tint: float = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 0xffff)), 0.0), 1.0);*/ \
	var tint: float = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 65535)), 0.0), 1.0); \
	return lerp3(c3, lerp3(c1, c2, tint), b.x * b.y * b.z); \
} \
fun tex_brick_f(p: float3): float { \
	p /= float3(0.9, 0.49, 0.49) / 2.0; \
	if (frac(p.y * 0.5) > 0.5) { p.x += 0.5; } \
	p = frac3(p); \
	var b: float3 = step3(p, float3(0.95, 0.9, 0.9)); \
	return lerp(1.0, 0.0, b.x * b.y * b.z); \
} \
";
	gc_unroot(brick_texture_node_def);
	brick_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Brick Texture"),
	                              .type   = "TEX_BRICK",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color 1"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.8, 0.8, 0.8),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color 2"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.2, 0.2, 0.2),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Mortar"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  5),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(brick_texture_node_def);
	gc_unroot(rgb_node_def);
	rgb_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                         .name    = _tr("Color"),
	                                         .type    = "RGB",
	                                         .x       = 0,
	                                         .y       = 0,
	                                         .color   = 0xffb34f5a,
	                                         .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                         .outputs = any_array_create_from_raw(
	                                             (void *[]){
	                                                 GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                  .node_id       = 0,
	                                                                                  .name          = _tr("Color"),
	                                                                                  .type          = "RGBA",
	                                                                                  .color         = 0xffc7c729,
	                                                                                  .default_value = f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
	                                                                                  .min           = 0.0,
	                                                                                  .max           = 1.0,
	                                                                                  .precision     = 100,
	                                                                                  .display       = 0}),
	                                             },
	                                             1),
	                                         .buttons = any_array_create_from_raw(
	                                             (void *[]){
	                                                 GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("default_value"),
	                                                                                  .type          = "RGBA",
	                                                                                  .output        = 0,
	                                                                                  .default_value = f32_array_create_x(0),
	                                                                                  .data          = NULL,
	                                                                                  .min           = 0.0,
	                                                                                  .max           = 1.0,
	                                                                                  .precision     = 100,
	                                                                                  .height        = 0}),
	                                             },
	                                             1),
	                                         .width = 0,
	                                         .flags = 0});
	gc_root(rgb_node_def);
	gc_unroot(rgb_to_bw_node_def);
	rgb_to_bw_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                               .name   = _tr("RGB to BW"),
	                                               .type   = "RGBTOBW",
	                                               .x      = 0,
	                                               .y      = 0,
	                                               .color  = 0xff62676d,
	                                               .inputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Color"),
	                                                                                        .type          = "RGBA",
	                                                                                        .color         = 0xffc7c729,
	                                                                                        .default_value = f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   1),
	                                               .outputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Val"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   1),
	                                               .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                               .width   = 0,
	                                               .flags   = 0});
	gc_root(rgb_to_bw_node_def);
	gc_unroot(attribute_node_def);
	attribute_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                               .name    = _tr("Attribute"),
	                                               .type    = "ATTRIBUTE",
	                                               .x       = 0,
	                                               .y       = 0,
	                                               .color   = 0xffb34f5a,
	                                               .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                               .outputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Color"),
	                                                                                        .type          = "RGBA",
	                                                                                        .color         = 0xffc7c729,
	                                                                                        .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Vector"),
	                                                                                        .type          = "VECTOR",
	                                                                                        .color         = 0xff6363c7,
	                                                                                        .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Factor"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Alpha"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   4),
	                                               .buttons = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Name"),
	                                                                                        .type          = "STRING",
	                                                                                        .output        = -1,
	                                                                                        .default_value = f32_array_create_x(0),
	                                                                                        .data          = NULL,
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .height        = 0}),
	                                                   },
	                                                   1),
	                                               .width = 0,
	                                               .flags = 0});
	gc_root(attribute_node_def);
	gc_unroot(shader_node_def);
	shader_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                            .name    = _tr("Shader"),
	                                            .type    = "SHADER_GPU", // extension
	                                            .x       = 0,
	                                            .y       = 0,
	                                            .color   = 0xffb34f5a,
	                                            .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                            .outputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Value"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(0.5),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                1),
	                                            .buttons = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_button_t, {.name          = " ",
	                                                                                     .type          = "STRING",
	                                                                                     .output        = -1,
	                                                                                     .default_value = f32_array_create_x(0), // "",
	                                                                                     .data          = NULL,
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .height        = 0}),
	                                                },
	                                                1),
	                                            .width = 0,
	                                            .flags = 0});
	gc_root(shader_node_def);
	gc_unroot(upscale_image_node_def);
	upscale_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                   .name   = _tr("Upscale Image"),
	                                                   .type   = "NEURAL_UPSCALE_IMAGE",
	                                                   .x      = 0,
	                                                   .y      = 0,
	                                                   .color  = 0xff4982a0,
	                                                   .inputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Color"),
	                                                                                            .type          = "RGBA",
	                                                                                            .color         = 0xffc7c729,
	                                                                                            .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       1),
	                                                   .outputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Color"),
	                                                                                            .type          = "RGBA",
	                                                                                            .color         = 0xffc7c729,
	                                                                                            .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       1),
	                                                   .buttons = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_button_t, {.name          = "upscale_image_node_button",
	                                                                                            .type          = "CUSTOM",
	                                                                                            .output        = -1,
	                                                                                            .default_value = f32_array_create_x(0),
	                                                                                            .data          = NULL,
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .height        = 2}),
	                                                       },
	                                                       1),
	                                                   .width = 0,
	                                                   .flags = 0});
	gc_root(upscale_image_node_def);
	image_to_pbr_node_result_base      = NULL;
	image_to_pbr_node_result_normal    = NULL;
	image_to_pbr_node_result_occlusion = NULL;
	image_to_pbr_node_result_height    = NULL;
	image_to_pbr_node_result_roughness = NULL;
	image_to_pbr_node_result_metallic  = NULL;
	gc_unroot(image_to_pbr_node_def);
	image_to_pbr_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Image to PBR"),
	                              .type   = "NEURAL_IMAGE_TO_PBR",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Base Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Occlusion"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Roughness"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Metallic"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal Map"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Height"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  6),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "image_to_pbr_node_button",
	                                                                       .type          = "CUSTOM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 2}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(image_to_pbr_node_def);
	gc_unroot(inpaint_image_node_def);
	inpaint_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                   .name   = _tr("Inpaint Image"),
	                                                   .type   = "NEURAL_INPAINT_IMAGE",
	                                                   .x      = 0,
	                                                   .y      = 0,
	                                                   .color  = 0xff4982a0,
	                                                   .inputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Color"),
	                                                                                            .type          = "RGBA",
	                                                                                            .color         = 0xffc7c729,
	                                                                                            .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Mask"),
	                                                                                            .type          = "VALUE",
	                                                                                            .color         = 0xffa1a1a1,
	                                                                                            .default_value = f32_array_create_x(1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       2),
	                                                   .outputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Color"),
	                                                                                            .type          = "RGBA",
	                                                                                            .color         = 0xffc7c729,
	                                                                                            .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       1),
	                                                   .buttons = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_button_t, {.name          = "inpaint_image_node_button",
	                                                                                            .type          = "CUSTOM",
	                                                                                            .output        = -1,
	                                                                                            .default_value = f32_array_create_x(0),
	                                                                                            .data          = NULL,
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .height        = 2}),
	                                                       },
	                                                       1),
	                                                   .width = 0,
	                                                   .flags = 0});
	gc_root(inpaint_image_node_def);
	gc_unroot(text_to_image_node_def);
	text_to_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                                   .name    = _tr("Text to Image"),
	                                                   .type    = "NEURAL_TEXT_TO_IMAGE",
	                                                   .x       = 0,
	                                                   .y       = 0,
	                                                   .color   = 0xff4982a0,
	                                                   .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                                   .outputs = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                            .node_id       = 0,
	                                                                                            .name          = _tr("Color"),
	                                                                                            .type          = "RGBA",
	                                                                                            .color         = 0xffc7c729,
	                                                                                            .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .display       = 0}),
	                                                       },
	                                                       1),
	                                                   .buttons = any_array_create_from_raw(
	                                                       (void *[]){
	                                                           GC_ALLOC_INIT(ui_node_button_t, {.name          = "text_to_image_node_button",
	                                                                                            .type          = "CUSTOM",
	                                                                                            .output        = -1,
	                                                                                            .default_value = f32_array_create_x(0),
	                                                                                            .data          = NULL,
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .height        = 1}),
	                                                           GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Tiled"),
	                                                                                            .type          = "BOOL",
	                                                                                            .output        = 0,
	                                                                                            .default_value = f32_array_create_x(0),
	                                                                                            .data          = NULL,
	                                                                                            .min           = 0.0,
	                                                                                            .max           = 1.0,
	                                                                                            .precision     = 100,
	                                                                                            .height        = 0}),
	                                                       },
	                                                       2),
	                                                   .width = 0,
	                                                   .flags = 0});
	gc_root(text_to_image_node_def);
	gc_unroot(outpaint_image_node_def);
	outpaint_image_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Outpaint Image"),
	                              .type   = "NEURAL_OUTPAINT_IMAGE",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(1.0, 1.0, 1.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "outpaint_image_node_button",
	                                                                       .type          = "CUSTOM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(outpaint_image_node_def);
	gc_unroot(neural_node_results);
	neural_node_results = any_imap_create();
	gc_root(neural_node_results);
	neural_node_downloading = 0;
	neural_node_models      = NULL;
	gc_unroot(edit_image_node_def);
	edit_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                .name   = _tr("Edit Image"),
	                                                .type   = "NEURAL_EDIT_IMAGE",
	                                                .x      = 0,
	                                                .y      = 0,
	                                                .color  = 0xff4982a0,
	                                                .inputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(1.0, 1.0, 1.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .outputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .buttons = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_button_t, {.name          = "edit_image_node_button",
	                                                                                         .type          = "CUSTOM",
	                                                                                         .output        = -1,
	                                                                                         .default_value = f32_array_create_x(0),
	                                                                                         .data          = NULL,
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .height        = 0}),
	                                                    },
	                                                    1),
	                                                .width = 0,
	                                                .flags = 0});
	gc_root(edit_image_node_def);
	gc_unroot(tile_image_node_def);
	tile_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                .name   = _tr("Tile Image"),
	                                                .type   = "NEURAL_TILE_IMAGE",
	                                                .x      = 0,
	                                                .y      = 0,
	                                                .color  = 0xff4982a0,
	                                                .inputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .outputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .buttons = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_button_t, {.name          = "tile_image_node_button",
	                                                                                         .type          = "CUSTOM",
	                                                                                         .output        = -1,
	                                                                                         .default_value = f32_array_create_x(0),
	                                                                                         .data          = NULL,
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .height        = 2}),
	                                                    },
	                                                    1),
	                                                .width = 0,
	                                                .flags = 0});
	gc_root(tile_image_node_def);
	gc_unroot(image_to_3d_mesh_node_def);
	image_to_3d_mesh_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Image to 3D Mesh"),
	                              .type   = "NEURAL_IMAGE_TO_3D_MESH",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Mesh"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "image_to_3d_mesh_node_button",
	                                                                       .type          = "CUSTOM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 2}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(image_to_3d_mesh_node_def);
	gc_unroot(vary_image_node_def);
	vary_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                .name   = _tr("Vary Image"),
	                                                .type   = "NEURAL_VARY_IMAGE",
	                                                .x      = 0,
	                                                .y      = 0,
	                                                .color  = 0xff4982a0,
	                                                .inputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(1.0, 1.0, 1.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .outputs = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                         .node_id       = 0,
	                                                                                         .name          = _tr("Color"),
	                                                                                         .type          = "RGBA",
	                                                                                         .color         = 0xffc7c729,
	                                                                                         .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .display       = 0}),
	                                                    },
	                                                    1),
	                                                .buttons = any_array_create_from_raw(
	                                                    (void *[]){
	                                                        GC_ALLOC_INIT(ui_node_button_t, {.name          = "vary_image_node_button",
	                                                                                         .type          = "CUSTOM",
	                                                                                         .output        = -1,
	                                                                                         .default_value = f32_array_create_x(0),
	                                                                                         .data          = NULL,
	                                                                                         .min           = 0.0,
	                                                                                         .max           = 1.0,
	                                                                                         .precision     = 100,
	                                                                                         .height        = 0}),
	                                                    },
	                                                    1),
	                                                .width = 0,
	                                                .flags = 0});
	gc_root(vary_image_node_def);
	gc_unroot(image_to_normal_map_node_def);
	image_to_normal_map_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Image to Normal Map"),
	                              .type   = "NEURAL_IMAGE_TO_NORMAL_MAP",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Normal Map"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "image_to_normal_map_node_button",
	                                                                       .type          = "CUSTOM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 2}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(image_to_normal_map_node_def);
	gc_unroot(image_to_depth_node_def);
	image_to_depth_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Image to Depth"),
	                              .type   = "NEURAL_IMAGE_TO_DEPTH",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Depth"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "image_to_depth_node_button",
	                                                                       .type          = "CUSTOM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 2}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(image_to_depth_node_def);
	gc_unroot(tex_image_node_def);
	tex_image_node_def = GC_ALLOC_INIT(ui_node_t, {.id   = 0,
	                                               .name = _tr("Image Texture"),
	                                               // type: "tex_image_node",
	                                               .type   = "TEX_IMAGE",
	                                               .x      = 0,
	                                               .y      = 0,
	                                               .color  = 0xff4982a0,
	                                               .inputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Vector"),
	                                                                                        .type          = "VECTOR",
	                                                                                        .color         = 0xff6363c7,
	                                                                                        .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   1),
	                                               .outputs = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Color"),
	                                                                                        .type          = "VALUE", // Match brush output socket type
	                                                                                        .color         = 0xffc7c729,
	                                                                                        .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                       GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                        .node_id       = 0,
	                                                                                        .name          = _tr("Alpha"),
	                                                                                        .type          = "VALUE",
	                                                                                        .color         = 0xffa1a1a1,
	                                                                                        .default_value = f32_array_create_x(1.0),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .display       = 0}),
	                                                   },
	                                                   2),
	                                               .buttons = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("file"),
	                                                                                        .type          = "ENUM",
	                                                                                        .output        = -1,
	                                                                                        .default_value = f32_array_create_x(0),
	                                                                                        .data          = u8_array_create_from_string(""),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .height        = 0}),
	                                                       GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("color_space"),
	                                                                                        .type          = "ENUM",
	                                                                                        .output        = -1,
	                                                                                        .default_value = f32_array_create_x(0),
	                                                                                        .data          = u8_array_create_from_string("linear\nsrgb"),
	                                                                                        .min           = 0.0,
	                                                                                        .max           = 1.0,
	                                                                                        .precision     = 100,
	                                                                                        .height        = 0}),
	                                                   },
	                                                   2),
	                                               .width = 0,
	                                               .flags = 0});
	gc_root(tex_image_node_def);
	random_node_d = -1;
	gc_unroot(random_node_def);
	random_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                            .name   = _tr("Random"),
	                                            .type   = "random_node",
	                                            .x      = 0,
	                                            .y      = 0,
	                                            .color  = 0xffb34f5a,
	                                            .inputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Min"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(0.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Max"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(1.0),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                2),
	                                            .outputs = any_array_create_from_raw(
	                                                (void *[]){
	                                                    GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                     .node_id       = 0,
	                                                                                     .name          = _tr("Value"),
	                                                                                     .type          = "VALUE",
	                                                                                     .color         = 0xffa1a1a1,
	                                                                                     .default_value = f32_array_create_x(0.5),
	                                                                                     .min           = 0.0,
	                                                                                     .max           = 1.0,
	                                                                                     .precision     = 100,
	                                                                                     .display       = 0}),
	                                                },
	                                                1),
	                                            .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                            .width   = 0,
	                                            .flags   = 0});
	gc_root(random_node_def);
	input_node_coords       = vec4_create(0.0, 0.0, 0.0, 1.0);
	input_node_start_x      = 0.0;
	input_node_start_y      = 0.0;
	input_node_lock_begin   = false;
	input_node_lock_x       = false;
	input_node_lock_y       = false;
	input_node_lock_start_x = 0.0;
	input_node_lock_start_y = 0.0;
	input_node_registered   = false;
	gc_unroot(input_node_def);
	input_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                           .name   = _tr("Input"),
	                                           .type   = "input_node",
	                                           .x      = 0,
	                                           .y      = 0,
	                                           .color  = 0xff4982a0,
	                                           .inputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Lazy Radius"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(0.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Lazy Step"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(0.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               2),
	                                           .outputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Position"),
	                                                                                    .type          = "VECTOR",
	                                                                                    .color         = 0xff63c763,
	                                                                                    .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               1),
	                                           .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                           .width   = 0,
	                                           .flags   = 0});
	gc_root(input_node_def);
	gc_unroot(math_node_def);
	math_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Math"),
	                              .type   = "math_node",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(
                                                                               "Add\nSubtract\nMultiply\nDivide\nPower\nLogarithm\nSquare Root\nInverse "
	                                                                                    "Square Root\nAbsolute\nExponent\nMinimum\nMaximum\nLess Than\nGreater "
	                                                                                    "Than\nSign\nRound\nFloor\nCeil\nTruncate\nFraction\nModulo\nSnap\nPing-"
	                                                                                    "Pong\nSine\nCosine\nTangent\nArcsine\nArccosine\nArctangent\nArctan2\nHyperb"
	                                                                                    "olic Sine\nHyperbolic Cosine\nHyperbolic Tangent\nTo Radians\nTo Degrees"),
	                                                                       .min       = 0.0,
	                                                                       .max       = 1.0,
	                                                                       .precision = 100,
	                                                                       .height    = 0}),
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Clamp"),
	                                                                       .type          = "BOOL",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  2),
	                              .width = 0,
	                              .flags = 0});
	gc_root(math_node_def);
	gc_unroot(vector_node_def);
	vector_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Vector"),
	                              .type   = "vector_node",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("X"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Y"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Z"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(vector_node_def);
	gc_unroot(time_node_def);
	time_node_def = GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                                          .name    = _tr("Time"),
	                                          .type    = "time_node",
	                                          .x       = 0,
	                                          .y       = 0,
	                                          .color   = 0xff4982a0,
	                                          .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                                          .outputs = any_array_create_from_raw(
	                                              (void *[]){
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Time"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Delta"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                                  GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                   .node_id       = 0,
	                                                                                   .name          = _tr("Brush"),
	                                                                                   .type          = "VALUE",
	                                                                                   .color         = 0xffa1a1a1,
	                                                                                   .default_value = f32_array_create_x(0.0),
	                                                                                   .min           = 0.0,
	                                                                                   .max           = 1.0,
	                                                                                   .precision     = 100,
	                                                                                   .display       = 0}),
	                                              },
	                                              3),
	                                          .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                          .width   = 0,
	                                          .flags   = 0});
	gc_root(time_node_def);
	gc_unroot(vector_math_node_def);
	vector_math_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Vector Math"),
	                              .type   = "vector_math_node",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(
                                                                               "Add\nSubtract\nMultiply\nDivide\nAverage\nCross Product\nProject\nReflect\nDot "
	                                                                                    "Product\nDistance\nLength\nScale\nNormalize\nAbsolute\nMinimum\nMaximum\nFloor"
	                                                                                    "\nCeil\nFraction\nModulo\nSnap\nSine\nCosine\nTangent"),
	                                                                       .min       = 0.0,
	                                                                       .max       = 1.0,
	                                                                       .precision = 100,
	                                                                       .height    = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(vector_math_node_def);
	gc_unroot(float_node_def);
	float_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                           .name   = _tr("Value"),
	                                           .type   = "float_node",
	                                           .x      = 0,
	                                           .y      = 0,
	                                           .color  = 0xffb34f5a,
	                                           .inputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Value"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(0.5),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 10.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               1),
	                                           .outputs = any_array_create_from_raw(
	                                               (void *[]){
	                                                   GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                                    .node_id       = 0,
	                                                                                    .name          = _tr("Value"),
	                                                                                    .type          = "VALUE",
	                                                                                    .color         = 0xffa1a1a1,
	                                                                                    .default_value = f32_array_create_x(0.5),
	                                                                                    .min           = 0.0,
	                                                                                    .max           = 1.0,
	                                                                                    .precision     = 100,
	                                                                                    .display       = 0}),
	                                               },
	                                               1),
	                                           .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                           .width   = 0,
	                                           .flags   = 0});
	gc_root(float_node_def);
	gc_unroot(separate_vector_node_def);
	separate_vector_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Separate Vector"),
	                              .type   = "separate_vector_node",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("X"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Y"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Z"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(separate_vector_node_def);

	sys_on_resize = base_on_resize;
	sys_on_w      = base_w;
	sys_on_h      = base_h;
	sys_on_x      = base_x;
	sys_on_y      = base_y;

	iron_set_app_name(manifest_title); // Used to locate external application data folder
	config_load();
	config_init();
	context_init();
	sys_start(config_get_options());
	if (config_raw->layout == NULL) {
		config_init_layout();
	}
	iron_set_app_name(manifest_title);
	scene_set_active("Scene");
	uniforms_ext_init();
	render_path_base_init();
	render_path_deferred_init(); // Allocate gbuffer
	if (config_raw->render_mode == RENDER_MODE_FORWARD) {
		render_path_forward_init();
		gc_unroot(render_path_commands);
		render_path_commands = render_path_forward_commands;
		gc_root(render_path_commands);
	}
	else {
		gc_unroot(render_path_commands);
		render_path_commands = render_path_deferred_commands;
		gc_root(render_path_commands);
	}

	base_init();

	iron_start();
}
