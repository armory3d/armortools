
#pragma once

#include "enums.h"
#include "types.h"
#include <iron.h>

any_map_t     *ui_children;
any_map_t     *ui_nodes_custom_buttons;
gpu_texture_t *_tab_swatches_empty;
i32            tab_swatches_drag_pos = -1;
i32            _tab_swatches_draw_i;
buffer_t      *slot_brush_default_canvas = NULL;
gpu_texture_t *layers_temp_image         = NULL;
gpu_texture_t *layers_expa               = NULL;
gpu_texture_t *layers_expb               = NULL;
gpu_texture_t *layers_expc               = NULL;
f32            layers_default_base       = 0.5;
f32            layers_default_rough      = 0.4;
#if defined(IRON_ANDROID) || defined(IRON_IOS)
i32 layers_max_layers = 18;
#else
i32 layers_max_layers = 255;
#endif
uv_type_t                _layers_uv_type;
mat4_t                   _layers_decal_mat;
i32                      _layers_position;
i32                      _layers_base_color;
f32                      _layers_occlusion;
f32                      _layers_roughness;
f32                      _layers_metallic;
any_map_t               *operator_ops;
object_t                *_compass_hitbox_x;
object_t                *_compass_hitbox_y;
object_t                *_compass_hitbox_z;
object_t                *_compass_hovered      = NULL;
object_t                *_compass_hovered_last = NULL;
config_t                *config_raw            = NULL;
any_map_t               *config_keymap;
bool                     config_loaded         = false;
ui_align_t               config_button_align   = UI_ALIGN_LEFT;
char                    *config_button_spacing = "       ";
any_imap_t              *physics_body_object_map;
ui_handle_t             *box_export_htab;
string_t_array_t        *box_export_files = NULL;
ui_handle_t             *box_export_mesh_handle;
ui_handle_t             *box_export_hpreset;
export_preset_t         *box_export_preset = NULL;
string_t_array_t        *box_export_channels;
string_t_array_t        *box_export_color_spaces;
bool                     _box_export_bake_material;
export_preset_texture_t *_box_export_t;
bool                     _box_export_apply_displacement;
bool                     _box_export_merge_vertices;
gpu_texture_t           *_tab_textures_draw_img;
char                    *_tab_textures_draw_path;
asset_t                 *_tab_textures_draw_asset;
i32                      _tab_textures_draw_i;
bool                     _tab_textures_draw_is_packed;
buffer_t                *slot_material_default_canvas = NULL;
i32                      _tab_brushes_draw_i;
gpu_pipeline_t          *pipes_copy;
gpu_pipeline_t          *pipes_copy8;
gpu_pipeline_t          *pipes_copy64;
gpu_pipeline_t          *pipes_copy128;
gpu_pipeline_t          *pipes_copy_bgra;
gpu_pipeline_t          *pipes_copy_rgb = NULL;
// let pipes_copy_r: gpu_pipeline_t;
// let pipes_copy_g: gpu_pipeline_t;
// let pipes_copy_a: gpu_pipeline_t;
// let pipes_copy_a_tex: i32;
gpu_pipeline_t *pipes_merge   = NULL;
gpu_pipeline_t *pipes_merge_r = NULL;
gpu_pipeline_t *pipes_merge_g = NULL;
gpu_pipeline_t *pipes_merge_b = NULL;
gpu_pipeline_t *pipes_merge_mask;
gpu_pipeline_t *pipes_invert8;
gpu_pipeline_t *pipes_apply_mask;
gpu_pipeline_t *pipes_colorid_to_mask;
i32             pipes_tex0;
i32             pipes_tex1;
i32             pipes_texmask;
i32             pipes_texa;
i32             pipes_opac;
i32             pipes_blending;
i32             pipes_tex1w;
i32             pipes_tex0_mask;
i32             pipes_texa_mask;
i32             pipes_tex0_merge_mask;
i32             pipes_texa_merge_mask;
i32             pipes_tex_colorid;
i32             pipes_texpaint_colorid;
i32             pipes_opac_merge_mask;
i32             pipes_blending_merge_mask;
gpu_texture_t  *pipes_temp_mask_image = NULL;
gpu_pipeline_t *pipes_inpaint_preview;
i32             pipes_tex0_inpaint_preview;
i32             pipes_texa_inpaint_preview;
gpu_pipeline_t *pipes_cursor;
i32             pipes_cursor_vp;
i32             pipes_cursor_inv_vp;
i32             pipes_cursor_mouse;
i32             pipes_cursor_tex_step;
i32             pipes_cursor_radius;
i32             pipes_cursor_camera_right;
i32             pipes_cursor_tint;
i32             pipes_cursor_gbufferd;
i32             pipes_offset;
char           *_import_blend_material_path;
any_map_t      *import_texture_importers;

#ifdef IRON_WINDOWS
char *ui_files_default_path = "C:\\Users";
#elif defined(IRON_ANDROID)
char *ui_files_default_path = "/storage/emulated/0/Download";
#elif defined(IRON_MACOS)
char *ui_files_default_path = "/Users";
#else
char *ui_files_default_path = "/";
#endif

char                     *ui_files_filename;
char                     *ui_files_path;
char                     *ui_files_last_path       = "";
char                     *ui_files_last_search     = "";
string_t_array_t         *ui_files_files           = NULL;
any_map_t                *ui_files_icon_map        = NULL;
any_map_t                *ui_files_icon_file_map   = NULL;
i32                       ui_files_selected        = -1;
bool                      ui_files_show_extensions = false;
bool                      ui_files_offline         = false;
ui_handle_t              *_ui_files_file_browser_handle;
char                     *_ui_files_file_browser_f;
bool                      base_ui_enabled     = true;
bool                      base_view3d_show    = true;
bool                      base_is_dragging    = false;
bool                      base_is_resizing    = false;
asset_t                  *base_drag_asset     = NULL;
swatch_color_t           *base_drag_swatch    = NULL;
char                     *base_drag_file      = NULL;
gpu_texture_t            *base_drag_file_icon = NULL;
i32                       base_drag_tint      = 0xffffffff;
i32                       base_drag_size      = -1;
rect_t                   *base_drag_rect      = NULL;
f32                       base_drag_off_x     = 0.0;
f32                       base_drag_off_y     = 0.0;
f32                       base_drag_start     = 0.0;
f32                       base_drop_x         = 0.0;
f32                       base_drop_y         = 0.0;
draw_font_t              *base_font           = NULL;
ui_theme_t               *base_theme;
gpu_texture_t            *base_color_wheel;
gpu_texture_t            *base_color_wheel_gradient;
i32                       base_default_element_w = 100;
i32                       base_default_element_h = 28;
i32                       base_default_font_size = 13;
ui_handle_t              *base_res_handle;
ui_handle_t              *base_bits_handle;
string_t_array_t         *base_drop_paths;
i32                       base_appx               = 0;
i32                       base_appy               = 0;
i32                       base_last_window_width  = 0;
i32                       base_last_window_height = 0;
slot_material_t          *base_drag_material      = NULL;
slot_layer_t             *base_drag_layer         = NULL;
bool                      base_player_lock        = false;
i32                       _base_material_count;
ui_t                     *ui;
bool                      ui_base_show                   = true;
i32                       ui_base_border_started         = 0;
ui_handle_t              *ui_base_border_handle          = NULL;
char                     *ui_base_action_paint_remap     = "";
i32                       ui_base_operator_search_offset = 0;
f32                       ui_base_undo_tap_time          = 0.0;
f32                       ui_base_redo_tap_time          = 0.0;
i32                       ui_base_viewport_col;
bool                      _ui_base_operator_search_first;
ui_handle_t_array_t      *ui_base_hwnds;
ui_handle_t_array_t      *ui_base_htabs;
tab_draw_array_t_array_t *ui_base_hwnd_tabs;
vec4_t                    gizmo_v;
vec4_t                    gizmo_v0;
quat_t                    gizmo_q;
quat_t                    gizmo_q0;
context_t                *context_raw;
i32                       ui_toolbar_default_w = 36;
ui_handle_t              *ui_toolbar_handle;
i32                       ui_toolbar_last_tool = 0;
string_t_array_t         *ui_toolbar_tool_names;
string_t_array_t         *ui_toolbar_tooltip_extras;
i32                       _ui_toolbar_i;
f32                       _import_asset_drop_x;
f32                       _import_asset_drop_y;
bool                      _import_asset_show_box;
bool                      _import_asset_hdr_as_envmap;
void (*_import_asset_done)(void);
mat4_t                  uniforms_ext_ortho_p;
ui_handle_t            *box_projects_htab;
ui_handle_t            *box_projects_hsearch;
any_map_t              *box_projects_icon_map = NULL;
char                   *_box_projects_path;
char                   *_box_projects_icon_path;
i32                     _box_projects_i;
history_step_t_array_t *history_steps;
i32                     history_undo_i      = 0;     // Undo layer
i32                     history_undos       = 0;     // Undos available
i32                     history_redos       = 0;     // Redos available
bool                    history_push_undo   = false; // Store undo on next paint
slot_layer_t_array_t   *history_undo_layers = NULL;
ui_handle_t            *tab_scripts_hscript;
ui_text_coloring_t     *tab_scripts_text_coloring    = NULL;
bool                    import_mesh_clear_layers     = true;
any_array_t            *import_mesh_meshes_to_unwrap = NULL;
any_map_t              *import_mesh_importers;
i32                     ui_menubar_default_w = 406;
ui_handle_t            *ui_menubar_hwnd;
ui_handle_t            *ui_menubar_menu_handle;
ui_handle_t            *ui_menubar_tab;
i32                     ui_menubar_w;
i32                     ui_menubar_category = 0;
mat4_t                  _ui_menubar_saved_camera;
mesh_object_t          *_ui_menubar_plane = NULL;
any_map_t              *translator_translations;
// The font index is a value specific to font_cjk.ttc
i32_map_t       *translator_cjk_font_indices = NULL;
char            *translator_last_locale      = "en";
char            *_translator_load_translations_cjk_font_path;
char            *_translator_load_translations_cjk_font_disk_path;
bool             _translator_init_font_cjk;
char            *_translator_init_font_font_path;
f32              _translator_init_font_font_scale;
scene_t         *scene_raw_gc;
i32              _tab_fonts_draw_i;
physics_world_t *physics_world_active;
bool             ui_menu_show               = false;
bool             ui_menu_nested             = false;
i32              ui_menu_x                  = 0;
i32              ui_menu_y                  = 0;
i32              ui_menu_h                  = 0;
bool             ui_menu_keep_open          = false;
void (*ui_menu_commands)(void)              = NULL;
bool                     ui_menu_show_first = true;
bool                     ui_menu_hide_flag  = false;
i32                      ui_menu_sub_x      = 0;
i32                      ui_menu_sub_y      = 0;
ui_handle_t             *ui_menu_sub_handle = NULL;
char                    *_ui_menu_render_msg;
i32                      render_path_base_taa_frame    = 0;
f32                      render_path_base_super_sample = 1.0;
f32                      render_path_base_last_x       = -1.0;
f32                      render_path_base_last_y       = -1.0;
render_target_t_array_t *render_path_base_bloom_mipmaps;
i32                      render_path_base_bloom_current_mip = 0;
f32                      render_path_base_bloom_sample_scale;
bool                     render_path_base_buf_swapped = false;
project_format_t        *project_raw;
char                    *project_filepath = "";
asset_t_array_t         *project_assets;
string_t_array_t        *project_asset_names;
i32                      project_asset_id = 0;
string_t_array_t        *project_mesh_assets;
node_group_t_array_t    *project_material_groups;
mesh_object_t_array_t   *project_paint_objects = NULL;
any_imap_t              *project_asset_map; // gpu_texture_t | font_t
string_t_array_t        *project_mesh_list = NULL;
slot_material_t_array_t *project_materials;
slot_brush_t_array_t    *project_brushes;
slot_layer_t_array_t    *project_layers;
slot_font_t_array_t     *project_fonts;
i32_array_t             *project_atlas_objects = NULL;
string_t_array_t        *project_atlas_names   = NULL;
bool                     _project_save_and_quit;
bool                     _project_import_mesh_replace_existing;
void (*_project_import_mesh_done)(void);
char *_project_import_mesh_box_path;
bool  _project_import_mesh_box_replace_existing;
bool  _project_import_mesh_box_clear_layers;
void (*_project_import_mesh_box_done)(void);
raw_mesh_t *_project_unwrap_mesh_box_mesh;
void (*_project_unwrap_mesh_box_done)(raw_mesh_t *);
bool                      _project_unwrap_mesh_box_skip_ui;
bool                      _project_import_asset_hdr_as_envmap;
bool                      _project_import_swatches_replace_existing;
asset_t                  *_project_reimport_texture_asset;
scene_t                  *_project_scene_mesh_gc;
i32                       _project_unwrap_by = 0;
gpu_pipeline_t           *ui_view2d_pipe;
i32                       ui_view2d_channel_loc;
bool                      ui_view2d_text_input_hover = false;
bool                      ui_view2d_uvmap_show       = false;
paint_tex_t               ui_view2d_tex_type         = PAINT_TEX_BASE;
view_2d_layer_mode_t      ui_view2d_layer_mode       = VIEW_2D_LAYER_MODE_SELECTED;
view_2d_type_t            ui_view2d_type             = VIEW_2D_TYPE_LAYER;
bool                      ui_view2d_show             = false;
i32                       ui_view2d_wx;
i32                       ui_view2d_wy;
i32                       ui_view2d_ww;
i32                       ui_view2d_wh;
ui_handle_t              *ui_view2d_hwnd;
f32                       ui_view2d_pan_x         = 0.0;
f32                       ui_view2d_pan_y         = 0.0;
f32                       ui_view2d_pan_scale     = 1.0;
bool                      ui_view2d_tiled_show    = false;
bool                      ui_view2d_controls_down = false;
gpu_texture_t            *_ui_view2d_render_tex;
f32                       _ui_view2d_render_x;
f32                       _ui_view2d_render_y;
f32                       _ui_view2d_render_tw;
f32                       _ui_view2d_render_th;
gpu_texture_t            *ui_view2d_grid        = NULL;
bool                      ui_view2d_grid_redraw = true;
ui_handle_t              *ui_view2d_htab;
bool                      ui_view2d_layer_touched    = false;
char                     *str_get_pos_nor_from_depth = "\
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
bool                      sim_running                = false;
mat4_box_t_array_t       *sim_transforms;
any_map_t                *sim_object_script_map;
bool                      sim_record         = false;
bool                      sim_initialized    = false;
bool                      viewport_recording = false;
node_shader_context_t    *parser_material_con;
node_shader_t            *parser_material_kong;
material_context_t       *parser_material_matcon;
string_t_array_t         *parser_material_parsed;
ui_node_t_array_t        *parser_material_parents;
ui_node_canvas_t_array_t *parser_material_canvases;
ui_node_t_array_t        *parser_material_nodes;
ui_node_link_t_array_t   *parser_material_links;
bool                      parser_material_cotangent_frame_written;
char                     *parser_material_tex_coord = "tex_coord";
f32                       parser_material_eps       = 0.000001;
any_map_t                *parser_material_node_values;
any_map_t                *parser_material_node_vectors;
any_map_t                *parser_material_custom_nodes;
bool                      parser_material_parse_surface           = true;
bool                      parser_material_parse_opacity           = true;
bool                      parser_material_parse_height            = false;
bool                      parser_material_parse_height_as_channel = false;
bool                      parser_material_parse_emission          = false;
bool                      parser_material_parse_subsurface        = false;
bool                      parser_material_parsing_basecolor       = false;
bool                      parser_material_triplanar               = false; // Sample using tex_coord/1/2 & tex_coord_blend
bool                      parser_material_sample_keep_aspect      = false; // Adjust uvs to preserve texture aspect ratio
char                     *parser_material_sample_uv_scale         = "1.0";
bool                      parser_material_transform_color_space   = true;
bool                      parser_material_blur_passthrough        = false;
bool                      parser_material_warp_passthrough        = false;
bool                      parser_material_bake_passthrough        = false;
ui_node_canvas_t         *parser_material_start_group             = NULL;
ui_node_t_array_t        *parser_material_start_parents           = NULL;
ui_node_t                *parser_material_start_node              = NULL;
bool                      parser_material_arm_export_tangents     = true;
char                     *parser_material_out_normaltan; // Raw tangent space normal parsed from normal map
any_map_t                *parser_material_script_links = NULL;
any_map_t                *parser_material_parsed_map;
any_map_t                *parser_material_texture_map;
bool                      parser_material_is_frag            = true;
bool                      args_use                           = false;
char                     *args_asset_path                    = "";
bool                      args_background                    = false;
bool                      args_export_textures               = false;
char                     *args_export_textures_type          = "";
char                     *args_export_textures_preset        = "";
char                     *args_export_textures_path          = "";
bool                      args_reimport_mesh                 = false;
bool                      args_export_mesh                   = false;
char                     *args_export_mesh_path              = "";
bool                      args_export_material               = false;
char                     *args_export_material_path          = "";
i32                       util_render_material_preview_size  = 256;
i32                       util_render_node_preview_size      = 512;
i32                       util_render_decal_preview_size     = 512;
i32                       util_render_layer_preview_size     = 200;
i32                       util_render_font_preview_size      = 200;
gpu_buffer_t             *util_render_screen_aligned_full_vb = NULL;
gpu_buffer_t             *util_render_screen_aligned_full_ib = NULL;
i32                       _tab_materials_draw_slots;
ui_handle_t              *tab_browser_hpath;
ui_handle_t              *tab_browser_hsearch;
bool                      tab_browser_known     = false;
char                     *tab_browser_last_path = "";
char                     *_tab_browser_draw_file;
char                     *_tab_browser_draw_b;
bool                      tab_browser_refresh = false;
any_map_t                *util_mesh_unwrappers;
i16_array_t              *util_mesh_va0;
i32_array_t              *util_mesh_quantized;
i32                       ui_header_default_h = 30;
i32                       ui_header_h;
ui_handle_t              *ui_header_handle;
ui_handle_t              *_ui_header_draw_tool_properties_h;
any_map_t                *plugin_map;
char                     *_plugin_name;
any_map_t                *parser_logic_custom_nodes;
ui_node_t_array_t        *parser_logic_nodes;
ui_node_link_t_array_t   *parser_logic_links;
string_t_array_t         *parser_logic_parsed_nodes = NULL;
any_map_t                *parser_logic_node_map;
any_map_t                *resource_bundled;
i32                       render_path_raytrace_bake_rays_pix        = 0;
i32                       render_path_raytrace_bake_rays_sec        = 0;
i32                       render_path_raytrace_bake_current_sample  = 0;
f32                       render_path_raytrace_bake_rays_timer      = 0.0;
i32                       render_path_raytrace_bake_rays_counter    = 0;
gpu_texture_t            *render_path_raytrace_bake_last_layer      = NULL;
i32                       render_path_raytrace_bake_last_bake_type  = 0;
i32                       render_path_raytrace_bake_last_bake_type2 = 0;
shader_context_t         *make_material_default_scon                = NULL;
material_context_t       *make_material_default_mcon                = NULL;
bool                      make_material_opac_used                   = false;
bool                      make_material_height_used                 = false;
bool                      make_material_emis_used                   = false;
bool                      make_material_subs_used                   = false;
bool                      ui_nodes_show                             = false;
i32                       ui_nodes_wx;
i32                       ui_nodes_wy;
i32                       ui_nodes_ww;
i32                       ui_nodes_wh;
canvas_type_t             ui_nodes_canvas_type     = CANVAS_TYPE_MATERIAL;
bool                      ui_nodes_show_menu       = false;
bool                      ui_nodes_show_menu_first = true;
bool                      ui_nodes_hide_menu       = false;
i32                       ui_nodes_menu_category   = 0;
f32                       ui_nodes_popup_x         = 0.0;
f32                       ui_nodes_popup_y         = 0.0;
i32                       ui_nodes_node_search_x;
i32                       ui_nodes_node_search_y;
bool                      ui_nodes_uichanged_last        = false;
bool                      ui_nodes_recompile_mat         = false; // Mat preview
bool                      ui_nodes_recompile_mat_final   = false;
ui_node_t                *ui_nodes_node_search_spawn     = NULL;
i32                       ui_nodes_node_search_offset    = 0;
ui_node_canvas_t         *ui_nodes_last_canvas           = NULL;
i32                       ui_nodes_last_node_selected_id = -1;
bool                      ui_nodes_release_link          = false;
bool                      ui_nodes_is_node_menu_op       = false;
gpu_texture_t            *ui_nodes_grid                  = NULL;
bool                      ui_nodes_grid_redraw           = true;
i32                       ui_nodes_grid_cell_w           = 200;
i32                       ui_nodes_grid_small_cell_w     = 40;
ui_handle_t              *ui_nodes_hwnd;
node_group_t_array_t     *ui_nodes_group_stack;
bool                      ui_nodes_controls_down = false;
slot_material_t_array_t  *ui_nodes_tabs          = NULL;
ui_handle_t              *ui_nodes_htab;
f32                       ui_nodes_last_zoom = 1.0;
ui_node_link_t           *_ui_nodes_on_link_drag_link_drag;
ui_node_t                *_ui_nodes_on_link_drag_node;
ui_node_socket_t         *_ui_nodes_on_socket_released_socket;
ui_node_t                *_ui_nodes_on_socket_released_node;
ui_handle_t              *_ui_nodes_htype;
ui_handle_t              *_ui_nodes_hname;
ui_handle_t              *_ui_nodes_hmin;
ui_handle_t              *_ui_nodes_hmax;
ui_handle_t              *_ui_nodes_hval0;
ui_handle_t              *_ui_nodes_hval1;
ui_handle_t              *_ui_nodes_hval2;
ui_handle_t              *_ui_nodes_hval3;
ui_node_t                *_ui_nodes_on_canvas_released_selected;
bool                      _ui_nodes_node_search_first;
void (*_ui_nodes_node_search_done)(void);
ui_node_t *ui_nodes_node_changed = NULL;
void (*_ui_nodes_render_tmp)(i32);
string_t_array_t    *nodes_brush_categories;
ui_node_t_array_t   *nodes_brush_category0;
node_list_t_array_t *nodes_brush_list;
any_map_t           *nodes_brush_creates;
char                *str_get_smudge_tool_weight = "\
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
char                *str_get_blur_tool_weight   = "\
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
char                *manifest_title             = "ArmorPaint";
char                *manifest_version           = "1.0 alpha";
char                *manifest_version_project   = "4";
char                *manifest_version_config    = "1";
char                *manifest_url               = "https://armorpaint.org";
char                *manifest_url_android       = "https://play.google.com/store/apps/details?id=org.armorpaint";
char                *manifest_url_ios           = "https://apps.apple.com/app/armorpaint/id1533967534";
i32                  _export_exr_width;
i32                  _export_exr_stride;
u8_array_t          *_export_exr_out;
buffer_t            *_export_exr_src_view;
void (*_export_exr_write_line)(i32);
string_t_array_t    *nodes_material_categories;
ui_node_t_array_t   *nodes_material_input;
ui_node_t_array_t   *nodes_material_texture;
ui_node_t_array_t   *nodes_material_color;
ui_node_t_array_t   *nodes_material_utilities;
ui_node_t_array_t   *nodes_material_neural;
ui_node_t_array_t   *nodes_material_group;
node_list_t_array_t *nodes_material_list        = NULL;
i32                  make_mesh_layer_pass_count = 1;
char                *str_cotangent_frame        = "\
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

// let str_octahedron_wrap: string = "\
// fun octahedron_wrap(v: float2): float2 { \
// 	return (1.0 - abs(v.yx)) * (float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0)); \
// } \
// ";

char *str_octahedron_wrap = "\
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

char *str_pack_float_int16 = "\
fun pack_f32_i16(f: float, i: uint): float { \
	return 0.062504762 * min(f, 0.9999) + 0.062519999 * float(i); \
} \
";
char *str_sh_irradiance    = "\
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
char *str_envmap_equirect  = "\
fun envmap_equirect(normal: float3, angle: float): float2 { \
	var PI: float = 3.1415926535; \
	var PI2: float = PI * 2.0; \
	var phi: float = acos(normal.z); \
	var theta: float = atan2(-normal.y, normal.x) + PI + angle; \
	return float2(theta / PI2, phi / PI); \
} \
";
char *str_envmap_sample    = "\
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
char               *str_env_brdf_approx       = "\
fun env_brdf_approx(specular: float3, roughness: float, dotnv: float): float3 { \
	var c0: float4 = float4(-1.0, -0.0275, -0.572, 0.022); \
	var c1: float4 = float4(1.0, 0.0425, 1.04, -0.04); \
	var r: float4 = c0 * roughness + c1; \
	var a004: float = min(r.x * r.x, exp((-9.28 * dotnv) * log(2.0))) * r.x + r.y; \
	var ab: float2 = float2(-1.04, 1.04) * a004 + r.zw; \
	return specular * ab.x + ab.y; \
} \
";
char               *str_dither_bayer          = "\
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
i32                 ui_sidebar_default_w_mini = 56;
i32                 ui_sidebar_default_w_full = 280;
i32                 ui_sidebar_default_w;
i32                 ui_sidebar_tabx = 0;
ui_handle_t        *ui_sidebar_hminimized;
i32                 ui_sidebar_w_mini;
i32                 ui_sidebar_last_tab   = 0;
char               *console_message       = "";
f32                 console_message_timer = 0.0;
i32                 console_message_color = 0x00000000;
string_t_array_t   *console_last_traces;
char               *console_progress_text = NULL;
vec4_box_t_array_t *camera_origins;
mat4_box_t_array_t *camera_views;
i32                 camera_redraws = 0;
vec4_t              camera_dir;
f32                 camera_ease          = 1.0;
bool                camera_controls_down = false;
ui_handle_t        *box_preferences_htab;
string_t_array_t   *box_preferences_files_plugin = NULL;
string_t_array_t   *box_preferences_files_keymap = NULL;
ui_handle_t        *box_preferences_h_theme;
ui_handle_t        *box_preferences_h_preset;
string_t_array_t   *box_preferences_locales = NULL;
string_t_array_t   *box_preferences_themes  = NULL;
char               *_box_preferences_f;
ui_handle_t        *_box_preferences_h;
i32                 _box_preferences_i;
slot_layer_t       *render_path_paint_live_layer        = NULL;
i32                 render_path_paint_live_layer_drawn  = 0;
bool                render_path_paint_live_layer_locked = false;
bool                render_path_paint_dilated           = true;
bool                render_path_paint_push_undo_last;
mesh_object_t      *render_path_paint_painto                = NULL;
mesh_object_t      *render_path_paint_planeo                = NULL;
u8_array_t         *render_path_paint_visibles              = NULL;
bool                render_path_paint_merged_object_visible = false;
f32                 render_path_paint_saved_fov             = 0.0;
bool                render_path_paint_baking                = false;
render_target_t    *_render_path_paint_texpaint;
render_target_t    *_render_path_paint_texpaint_nor;
render_target_t    *_render_path_paint_texpaint_pack;
render_target_t    *_render_path_paint_texpaint_undo;
render_target_t    *_render_path_paint_texpaint_nor_undo;
render_target_t    *_render_path_paint_texpaint_pack_undo;
f32                 render_path_paint_last_x = -1.0;
f32                 render_path_paint_last_y = -1.0;
bake_type_t         _render_path_paint_bake_type;
f32                 export_texture_gamma = 1.0 / 2.2;
bool                ui_box_show          = false;
bool                ui_box_draggable     = true;
ui_handle_t        *ui_box_hwnd;
char               *ui_box_title   = "";
char               *ui_box_text    = "";
void (*ui_box_commands)(void)      = NULL;
bool ui_box_click_to_hide          = true;
i32  ui_box_modalw                 = 400;
i32  ui_box_modalh                 = 170;
void (*ui_box_modal_on_hide)(void) = NULL;
i32             ui_box_draws       = 0;
bool            ui_box_copyable    = false;
f32             ui_box_tween_alpha = 0.0;
i32             _tab_meshes_draw_i;
i32             tab_scene_line_counter         = 0;
i32             _tab_scene_paint_object_length = 1;
i32             tab_layers_layer_name_edit     = -1;
ui_handle_t    *tab_layers_layer_name_handle;
bool            tab_layers_show_context_menu = false;
slot_layer_t   *tab_layers_l;
bool            tab_layers_mini;
gpu_texture_t  *util_uv_uvmap                    = NULL;
bool            util_uv_uvmap_cached             = false;
gpu_texture_t  *util_uv_trianglemap              = NULL;
bool            util_uv_trianglemap_cached       = false;
gpu_texture_t  *util_uv_dilatemap                = NULL;
bool            util_uv_dilatemap_cached         = false;
gpu_texture_t  *util_uv_uvislandmap              = NULL;
bool            util_uv_uvislandmap_cached       = false;
buffer_t       *util_uv_dilate_bytes             = NULL;
gpu_pipeline_t *util_uv_pipe_dilate              = NULL;
i32             render_path_raytrace_frame       = 0;
bool            render_path_raytrace_ready       = false;
i32             render_path_raytrace_dirty       = 0;
f32             render_path_raytrace_uv_scale    = 1.0;
bool            render_path_raytrace_init_shader = true;
f32_array_t    *render_path_raytrace_f32a;
mat4_t          render_path_raytrace_help_mat;
mat4_t          render_path_raytrace_transform;
gpu_buffer_t   *render_path_raytrace_vb;
gpu_buffer_t   *render_path_raytrace_ib;
gpu_texture_t  *render_path_raytrace_last_envmap = NULL;
bool            render_path_raytrace_is_bake     = false;

#ifdef IRON_DIRECT3D12
char *render_path_raytrace_ext = ".cso";
#elif defined(IRON_METAL)
char *render_path_raytrace_ext = ".metal";
#else
char *render_path_raytrace_ext = ".spirv";
#endif

gpu_texture_t               *render_path_raytrace_last_texpaint = NULL;
bool                         sculpt_push_undo                   = false;
i32                          ui_statusbar_default_h             = 33;
i32                          ui_statusbar_last_tab              = 0;
gpu_pipeline_t              *import_envmap_pipeline             = NULL;
i32                          import_envmap_params_loc;
vec4_t                       import_envmap_params;
vec4_t                       import_envmap_n;
i32                          import_envmap_radiance_loc;
gpu_texture_t               *import_envmap_radiance = NULL;
i32                          import_envmap_noise_loc;
gpu_texture_t_array_t       *import_envmap_mips = NULL;
ui_node_t                   *image_texture_node_def;
ui_node_t                   *material_node_def;
ui_node_t                   *uv_map_node_def;
char                        *str_tex_wave = "\
fun tex_wave_f(p: float3): float { \
	return 1.0 - sin((p.x + p.y) * 10.0); \
} \
";
ui_node_t                   *wave_texture_node_def;
ui_node_t                   *clamp_node_def;
ui_node_t                   *tangent_node_def;
char                        *parser_material_bake_passthrough_strength = "0.0";
char                        *parser_material_bake_passthrough_radius   = "0.0";
char                        *parser_material_bake_passthrough_offset   = "0.0";
ui_node_t                   *curvature_bake_node_def;
ui_node_t                   *blur_node_def;
ui_node_t                   *text_texture_node_def;
ui_node_t                   *gradient_texture_node_def;
ui_node_t                   *object_info_node_def;
char                        *str_tex_magic = "\
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
ui_node_t                   *magic_texture_node_def;
ui_node_t                   *fresnel_node_def;
ui_node_t                   *warp_node_def;
ui_node_t                   *normal_node_def;
ui_node_t                   *mapping_node_def;
ui_node_t                   *normal_map_node_def;
ui_node_t                   *invert_color_node_def;
ui_node_t                   *wireframe_node_def;
ui_node_t                   *gamma_node_def;
ui_node_t                   *vector_math2_node_def;
char                        *str_tex_checker = "\
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
ui_node_t                   *checker_texture_node_def;
char                        *str_hue_sat = "\
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
ui_node_t                   *hue_saturation_value_node_def;
ui_node_t                   *separate_xyz_node_def;
ui_node_t                   *math2_node_def;
ui_node_t                   *geometry_node_def;
ui_node_t                   *mix_color_node_def;
ui_node_t                   *quantize_node_def;
ui_node_t                   *layer_node_def;
ui_node_t                   *map_range_node_def;
char                        *str_tex_voronoi = "\
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
ui_node_t                   *voronoi_texture_node_def;
ui_node_t                   *color_ramp_node_def;
ui_node_t                   *picker_node_def;
ui_node_t                   *layer_weight_node_def;
ui_node_t                   *replace_color_node_def;
ui_node_t                   *bump_node_def;
char                        *str_tex_gabor = "\
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
	intensity = intensity * 0.5 + 0.5; \
	var phase: float = atan2(phase_sin, phase_cos) / (2.0 * pi) + 0.5; \
	return float3(value, phase, intensity); \
} \
";
ui_node_t                   *gabor_texture_node_def;
ui_node_t                   *mix_normal_map_node_def;
ui_node_t                   *camera_data_node_def;
ui_node_t                   *script_node_def;
ui_node_t                   *combine_xyz_node_def;
char                        *str_tex_noise = "\
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
ui_node_t                   *noise_texture_node_def;
char                        *str_brightcontrast = "\
fun brightcontrast(col: float3, bright: float, contr: float): float3 { \
	var a: float = 1.0 + contr; \
	var b: float = bright - contr * 0.5; \
	return max3(a * col + b, float3(0.0, 0.0, 0.0)); \
} \
";
ui_node_t                   *brightness_contrast_node_def;
ui_node_t                   *color_mask_node_def;
ui_node_t                   *vector_curves_node_def;
ui_node_t                   *layer_mask_node_def;
ui_node_t                   *camera_texture_node_def;
ui_node_t                   *value_node_def;
ui_node_t                   *texture_coordinate_node_def;
ui_nodes_t                  *_nodes_material_nodes;
ui_node_t                   *_nodes_material_node;
ui_node_socket_t_array_t    *_nodes_material_sockets;
ui_node_t                   *group_node_def;
char                        *str_tex_brick = "\
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
ui_node_t                   *brick_texture_node_def;
ui_node_t                   *rgb_node_def;
ui_node_t                   *rgb_to_bw_node_def;
ui_node_t                   *attribute_node_def;
ui_node_t                   *shader_node_def;
ui_node_t                   *upscale_image_node_def;
gpu_texture_t               *image_to_pbr_node_result_base      = NULL;
gpu_texture_t               *image_to_pbr_node_result_normal    = NULL;
gpu_texture_t               *image_to_pbr_node_result_occlusion = NULL;
gpu_texture_t               *image_to_pbr_node_result_height    = NULL;
gpu_texture_t               *image_to_pbr_node_result_roughness = NULL;
gpu_texture_t               *image_to_pbr_node_result_metallic  = NULL;
ui_node_t                   *image_to_pbr_node_def;
ui_node_t                   *inpaint_image_node_def;
ui_node_t                   *text_to_image_node_def;
ui_node_t                   *outpaint_image_node_def;
any_imap_t                  *neural_node_results;
ui_node_t                   *neural_node_current;
i32                          neural_node_downloading = 0;
neural_node_model_t_array_t *neural_node_models      = NULL;
ui_node_t                   *edit_image_node_def;
ui_node_t                   *tile_image_node_def;
ui_node_t                   *image_to_3d_mesh_node_def;
ui_node_t                   *vary_image_node_def;
ui_node_t                   *image_to_normal_map_node_def;
ui_node_t                   *image_to_depth_node_def;
ui_node_t                   *tex_image_node_def;
i32                          random_node_a;
i32                          random_node_b;
i32                          random_node_c;
i32                          random_node_d = -1;
ui_node_t                   *random_node_def;
vec4_t                       input_node_coords;
f32                          input_node_start_x = 0.0;
f32                          input_node_start_y = 0.0;
// Brush ruler
bool       input_node_lock_begin   = false;
bool       input_node_lock_x       = false;
bool       input_node_lock_y       = false;
f32        input_node_lock_start_x = 0.0;
f32        input_node_lock_start_y = 0.0;
bool       input_node_registered   = false;
ui_node_t *input_node_def;
ui_node_t *math_node_def;
ui_node_t *vector_node_def;
ui_node_t *time_node_def;
ui_node_t *vector_math_node_def;
ui_node_t *float_node_def;
ui_node_t *separate_vector_node_def;
