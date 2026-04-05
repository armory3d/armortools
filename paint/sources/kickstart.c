

#include "global.h"

#ifdef WITH_PLUGINS
void plugins_init();
#endif

void _kickstart() {
	_render_path_cached_shader_contexts = any_map_create();
	gc_root(_render_path_cached_shader_contexts);

	ui_children = any_map_create();
	gc_root(ui_children);

	ui_nodes_custom_buttons = any_map_create();
	gc_root(ui_nodes_custom_buttons);

	operator_ops = any_map_create();
	gc_root(operator_ops);

	physics_body_object_map = any_imap_create();
	gc_root(physics_body_object_map);

	box_export_htab = ui_handle_create();
	gc_root(box_export_htab);

	box_export_mesh_handle = ui_handle_create();
	gc_root(box_export_mesh_handle);

	box_export_hpreset = ui_handle_create();
	gc_root(box_export_hpreset);

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

	box_export_color_spaces = any_array_create_from_raw(
	    (void *[]){
	        "linear",
	        "srgb",
	    },
	    2);
	gc_root(box_export_color_spaces);

	box_export_h_export_player_target = ui_handle_create();
	gc_root(box_export_h_export_player_target);

	import_texture_importers = any_map_create();
	gc_root(import_texture_importers);

	ui_files_path = ui_files_default_path;
	gc_root(ui_files_path);

	base_res_handle = ui_handle_create();
	gc_root(base_res_handle);

	base_bits_handle = ui_handle_create();
	gc_root(base_bits_handle);

	base_drop_paths = any_array_create_from_raw((void *[]){}, 0);
	gc_root(base_drop_paths);

	ui_base_hwnds = ui_base_init_hwnds();
	gc_root(ui_base_hwnds);

	ui_base_htabs = ui_base_init_htabs();
	gc_root(ui_base_htabs);

	ui_base_hwnd_tabs = ui_base_init_hwnd_tabs();
	gc_root(ui_base_hwnd_tabs);
	gizmo_v  = vec4_create(0.0, 0.0, 0.0, 1.0);
	gizmo_v0 = vec4_create(0.0, 0.0, 0.0, 1.0);
	gizmo_q  = quat_create(0.0, 0.0, 0.0, 1.0);
	gizmo_q0 = quat_create(0.0, 0.0, 0.0, 1.0);

	ui_toolbar_handle = ui_handle_create();
	gc_root(ui_toolbar_handle);

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

	box_projects_htab = ui_handle_create();
	gc_root(box_projects_htab);

	box_projects_hsearch = ui_handle_create();
	gc_root(box_projects_hsearch);

	tab_scripts_hscript = ui_handle_create();
	gc_root(tab_scripts_hscript);

	import_mesh_importers = any_map_create();
	gc_root(import_mesh_importers);

	ui_menubar_hwnd = ui_handle_create();
	gc_root(ui_menubar_hwnd);

	ui_menubar_menu_handle = ui_handle_create();
	gc_root(ui_menubar_menu_handle);

	ui_menubar_tab = ui_handle_create();
	gc_root(ui_menubar_tab);
	ui_menubar_w             = ui_menubar_default_w;

	translator_translations = any_map_create();
	gc_root(translator_translations);

	g_project = GC_ALLOC_INIT(project_t, {0});
	gc_root(g_project);

	project_assets = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_assets);

	project_asset_names = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_asset_names);

	project_mesh_assets = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_mesh_assets);

	project_material_groups = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_material_groups);

	project_asset_map = any_imap_create();
	gc_root(project_asset_map);

	project_materials = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_materials);

	project_brushes = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_brushes);

	project_layers = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_layers);

	project_fonts = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_fonts);

	ui_view2d_hwnd = ui_handle_create();
	gc_root(ui_view2d_hwnd);

	ui_view2d_htab = ui_handle_create();
	gc_root(ui_view2d_htab);

	sim_object_script_map = any_map_create();
	gc_root(sim_object_script_map);

	parser_material_node_values = any_map_create();
	gc_root(parser_material_node_values);

	parser_material_node_vectors = any_map_create();
	gc_root(parser_material_node_vectors);

	parser_material_custom_nodes = any_map_create();
	gc_root(parser_material_custom_nodes);

	parser_material_parsed_map = any_map_create();
	gc_root(parser_material_parsed_map);

	parser_material_texture_map = any_map_create();
	gc_root(parser_material_texture_map);

	tab_browser_hpath = ui_handle_create();
	gc_root(tab_browser_hpath);

	tab_browser_hsearch = ui_handle_create();
	gc_root(tab_browser_hsearch);

	util_mesh_unwrappers = any_map_create();
	gc_root(util_mesh_unwrappers);

	ui_header_h = ui_header_default_h;

	ui_header_handle = ui_handle_create();
	gc_root(ui_header_handle);

	plugin_map = any_map_create();
	gc_root(plugin_map);

	parser_logic_custom_nodes = any_map_create();
	gc_root(parser_logic_custom_nodes);

	resource_bundled = any_map_create();
	gc_root(resource_bundled);

	ui_nodes_hwnd = ui_handle_create();
	gc_root(ui_nodes_hwnd);

	ui_nodes_group_stack = any_array_create_from_raw((void *[]){}, 0);
	gc_root(ui_nodes_group_stack);

	ui_nodes_htab = ui_handle_create();
	gc_root(ui_nodes_htab);

	_ui_nodes_htype = ui_handle_create();
	gc_root(_ui_nodes_htype);

	_ui_nodes_hname = ui_handle_create();
	gc_root(_ui_nodes_hname);

	_ui_nodes_hmin = ui_handle_create();
	gc_root(_ui_nodes_hmin);

	_ui_nodes_hmax = ui_handle_create();
	gc_root(_ui_nodes_hmax);

	_ui_nodes_hval0 = ui_handle_create();
	gc_root(_ui_nodes_hval0);

	_ui_nodes_hval1 = ui_handle_create();
	gc_root(_ui_nodes_hval1);

	_ui_nodes_hval2 = ui_handle_create();
	gc_root(_ui_nodes_hval2);

	_ui_nodes_hval3 = ui_handle_create();
	gc_root(_ui_nodes_hval3);

	nodes_brush_categories = any_array_create_from_raw(
	    (void *[]){
	        _tr("Nodes"),
	    },
	    1);
	gc_root(nodes_brush_categories);

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

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	ui_sidebar_default_w = ui_sidebar_default_w_mini;
#else
	ui_sidebar_default_w = ui_sidebar_default_w_full;
#endif

	ui_sidebar_hminimized = ui_handle_create();
	gc_root(ui_sidebar_hminimized);
	ui_sidebar_w_mini = ui_sidebar_default_w_mini;

	console_last_traces = any_array_create(0);
	gc_root(console_last_traces);
	camera_dir = vec4_create(0.0, 0.0, 0.0, 1.0);

	box_preferences_htab = ui_handle_create();
	gc_root(box_preferences_htab);

	ui_box_hwnd = ui_handle_create();
	gc_root(ui_box_hwnd);

	tab_layers_layer_name_handle = ui_handle_create();
	gc_root(tab_layers_layer_name_handle);

	render_path_raytrace_f32a = f32_array_create(24);
	gc_root(render_path_raytrace_f32a);
	render_path_raytrace_help_mat = mat4_identity();

	import_envmap_params = vec4_create(0.0, 0.0, 0.0, 1.0);
	import_envmap_n      = vec4_create(0.0, 0.0, 0.0, 1.0);

	neural_node_results = any_imap_create();
	gc_root(neural_node_results);
	input_node_coords = vec4_create(0.0, 0.0, 0.0, 1.0);

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

#ifdef is_debug
	double t_start = iron_time();
#endif

	if (g_config->layout == NULL) {
		config_init_layout();
	}
	iron_set_app_name(manifest_title);
	scene_set_active("Scene");
	uniforms_ext_init();
	render_path_base_init();
	render_path_deferred_init(); // Allocate gbuffer
	if (g_config->render_mode == RENDER_MODE_FORWARD) {
		render_path_forward_init();
		render_path_commands = render_path_forward_commands;
		gc_root(render_path_commands);
	}
	else {

		render_path_commands = render_path_deferred_commands;
		gc_root(render_path_commands);
	}

#ifdef WITH_PLUGINS
	plugins_init();
#endif

	base_init();

#ifdef is_debug
	iron_log("Started in %fs\n", iron_time() - t_start);
#endif

	iron_start();
}
