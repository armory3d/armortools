
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
string_array_t        *box_export_files = NULL;
ui_handle_t             *box_export_mesh_handle;
ui_handle_t             *box_export_hpreset;
export_preset_t         *box_export_preset = NULL;
string_array_t        *box_export_channels;
string_array_t        *box_export_color_spaces;
ui_handle_t             *box_export_h_export_player_target;
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
string_array_t         *ui_files_files           = NULL;
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
string_array_t         *base_drop_paths;
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
string_array_t         *ui_toolbar_tool_names;
string_array_t         *ui_toolbar_tooltip_extras;
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
string_array_t        *project_asset_names;
i32                      project_asset_id = 0;
string_array_t        *project_mesh_assets;
node_group_t_array_t    *project_material_groups;
mesh_object_t_array_t   *project_paint_objects = NULL;
any_imap_t              *project_asset_map; // gpu_texture_t | font_t
string_array_t        *project_mesh_list = NULL;
slot_material_t_array_t *project_materials;
slot_brush_t_array_t    *project_brushes;
slot_layer_t_array_t    *project_layers;
slot_font_t_array_t     *project_fonts;
i32_array_t             *project_atlas_objects = NULL;
string_array_t        *project_atlas_names   = NULL;
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
bool                      sim_running                = false;
mat4_box_t_array_t       *sim_transforms;
any_map_t                *sim_object_script_map;
bool                      sim_record         = false;
bool                      sim_initialized    = false;
bool                      viewport_recording = false;
node_shader_context_t    *parser_material_con;
node_shader_t            *parser_material_kong;
material_context_t       *parser_material_matcon;
string_array_t         *parser_material_parsed;
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
bool                      args_player                        = false;
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
string_array_t         *parser_logic_parsed_nodes = NULL;
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
string_array_t    *nodes_brush_categories;
ui_node_t_array_t   *nodes_brush_category0;
node_list_t_array_t *nodes_brush_list;
any_map_t           *nodes_brush_creates;
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
string_array_t    *nodes_material_categories;
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
string_array_t   *console_last_traces;
char               *console_progress_text = NULL;
vec4_box_t_array_t *camera_origins;
mat4_box_t_array_t *camera_views;
i32                 camera_redraws = 0;
vec4_t              camera_dir;
f32                 camera_ease          = 1.0;
bool                camera_controls_down = false;
ui_handle_t        *box_preferences_htab;
string_array_t   *box_preferences_files_plugin = NULL;
string_array_t   *box_preferences_files_keymap = NULL;
ui_handle_t        *box_preferences_h_theme;
ui_handle_t        *box_preferences_h_preset;
string_array_t   *box_preferences_locales = NULL;
string_array_t   *box_preferences_themes  = NULL;
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
ui_node_t                   *wave_texture_node_def;
ui_node_t                   *clamp_node_def;
char                        *parser_material_bake_passthrough_strength = "0.0";
char                        *parser_material_bake_passthrough_radius   = "0.0";
char                        *parser_material_bake_passthrough_offset   = "0.0";
ui_node_t                   *curvature_bake_node_def;
ui_node_t                   *blur_node_def;
ui_node_t                   *text_texture_node_def;
ui_node_t                   *gradient_texture_node_def;
ui_node_t                   *object_info_node_def;
ui_node_t                   *magic_texture_node_def;
ui_node_t                   *warp_node_def;
ui_node_t                   *normal_node_def;
ui_node_t                   *mapping_node_def;
ui_node_t                   *normal_map_node_def;
ui_node_t                   *invert_color_node_def;
ui_node_t                   *wireframe_node_def;
ui_node_t                   *gamma_node_def;
ui_node_t                   *vector_math2_node_def;
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
ui_node_t                   *voronoi_texture_node_def;
ui_node_t                   *color_ramp_node_def;
ui_node_t                   *picker_node_def;
ui_node_t                   *replace_color_node_def;
ui_node_t                   *bump_node_def;
ui_node_t                   *gabor_texture_node_def;
ui_node_t                   *mix_normal_map_node_def;
ui_node_t                   *script_node_def;
ui_node_t                   *combine_xyz_node_def;
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
