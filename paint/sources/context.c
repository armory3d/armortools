
#include "global.h"

context_t *context_create() {
	context_t *c               = GC_ALLOC_INIT(context_t, {0});
	c->merged_object_is_atlas  = false; // Only objects referenced by atlas are merged
	c->ddirty                  = 0;     // depth
	c->pdirty                  = 0;     // paint
	c->rdirty                  = 0;     // render
	c->brush_blend_dirty       = true;
	c->split_view              = false;
	c->view_index              = -1;
	c->view_index_last         = -1;
	c->picked_color            = project_make_swatch(0xffffffff);
	c->envmap_loaded           = false;
	c->show_envmap             = false;
	c->show_envmap_handle      = ui_handle_create();
	c->show_envmap_blur        = false;
	c->show_envmap_blur_handle = ui_handle_create();
	c->envmap_angle            = 0.0;
	c->light_angle             = 0.0;
	c->cull_backfaces          = true;
	c->texture_filter          = true;
	c->format_type             = TEXTURE_LDR_FORMAT_PNG;
	c->format_quality          = 100.0;
	c->layers_destination      = EXPORT_DESTINATION_DISK;
	c->split_by                = SPLIT_TYPE_OBJECT;
	c->select_time             = 0.0;
	c->viewport_mode           = config_raw->viewport_mode == 0 ? VIEWPORT_MODE_LIT : VIEWPORT_MODE_PATH_TRACE;
	c->hscale_was_changed      = false;
	c->export_mesh_format      = MESH_FORMAT_OBJ;
	c->export_mesh_index       = 0;
	c->pack_assets_on_export   = true;
	c->paint_vec               = vec4_create(0.0, 0.0, 0.0, 1.0);
	c->last_paint_x            = -1.0;
	c->last_paint_y            = -1.0;
	c->foreground_event        = false;
	c->painted                 = 0;
	c->brush_time              = 0.0;
	c->clone_start_x           = -1.0;
	c->clone_start_y           = -1.0;
	c->clone_delta_x           = 0.0;
	c->clone_delta_y           = 0.0;
	c->show_compass            = true;
	c->project_aspect_ratio    = 0; // 1:1, 2:1, 1:2
	c->last_paint_vec_x        = -1.0;
	c->last_paint_vec_y        = -1.0;
	c->prev_paint_vec_x        = -1.0;
	c->prev_paint_vec_y        = -1.0;
	c->frame                   = 0;
	c->paint2d_view            = false;
	c->brush_locked            = false;
	c->camera_type             = CAMERA_TYPE_PERSPECTIVE;
	c->cam_handle              = ui_handle_create();
	c->fov_handle              = ui_handle_create();
	c->texture_export_path     = "";
	c->last_status_position    = 0;
	c->camera_controls         = CAMERA_CONTROLS_ORBIT;
	c->pen_painting_only       = false; // Reject painting with finger when using pen
	c->layer_preview_dirty     = true;
	c->layers_preview_dirty    = false;
	c->node_preview_name       = "";
	c->node_preview_socket_map = any_map_create();
	c->node_preview_map        = any_map_create();
	c->selected_node_preview   = true;
	c->colorid_picked          = false;
	c->material_preview        = false; // Drawing material previews
	c->saved_camera            = mat4_identity();
	c->materialid_picked       = 0;
	c->uvx_picked              = 0.0;
	c->uvy_picked              = 0.0;
	c->picker_select_material  = false;
	c->picker_mask_handle      = ui_handle_create();
	c->pick_pos_nor_tex        = false;
	c->posx_picked             = 0.0;
	c->posy_picked             = 0.0;
	c->posz_picked             = 0.0;
	c->norx_picked             = 0.0;
	c->nory_picked             = 0.0;
	c->norz_picked             = 0.0;
	c->draw_wireframe          = false;
	c->wireframe_handle        = ui_handle_create();
	c->draw_texels             = false;
	c->texels_handle           = ui_handle_create();
	c->colorid_handle          = ui_handle_create();
	c->layers_export           = EXPORT_MODE_VISIBLE;
	c->decal_preview           = false;
	c->decal_x                 = 0.0;
	c->decal_y                 = 0.0;
	// c.cache_draws = false;
	c->write_icon_on_export              = false;
	c->particle_hit_x                    = 0.0;
	c->particle_hit_y                    = 0.0;
	c->particle_hit_z                    = 0.0;
	c->last_particle_hit_x               = 0.0;
	c->last_particle_hit_y               = 0.0;
	c->last_particle_hit_z               = 0.0;
	c->layer_filter                      = 0;
	c->gizmo_started                     = false;
	c->gizmo_offset                      = 0.0;
	c->gizmo_drag                        = 0.0;
	c->gizmo_drag_last                   = 0.0;
	c->translate_x                       = false;
	c->translate_y                       = false;
	c->translate_z                       = false;
	c->scale_x                           = false;
	c->scale_y                           = false;
	c->scale_z                           = false;
	c->rotate_x                          = false;
	c->rotate_y                          = false;
	c->rotate_z                          = false;
	c->brush_nodes_radius                = 1.0;
	c->brush_nodes_opacity               = 1.0;
	c->brush_mask_image_is_alpha         = false;
	c->brush_stencil_image_is_alpha      = false;
	c->brush_stencil_x                   = 0.02;
	c->brush_stencil_y                   = 0.02;
	c->brush_stencil_scale               = 0.9;
	c->brush_stencil_scaling             = false;
	c->brush_stencil_angle               = 0.0;
	c->brush_stencil_rotating            = false;
	c->brush_nodes_scale                 = 1.0;
	c->brush_nodes_angle                 = 0.0;
	c->brush_nodes_hardness              = 1.0;
	c->brush_directional                 = false;
	c->brush_radius_handle               = ui_handle_create();
	c->brush_scale_x                     = 1.0;
	c->brush_decal_mask_radius           = 0.5;
	c->brush_decal_mask_radius_handle    = ui_handle_create();
	c->brush_decal_mask_radius_handle->f = 0.5;
	c->brush_scale_x_handle              = ui_handle_create();
	c->brush_scale_x_handle->f           = 1.0;
	c->brush_blending                    = BLEND_TYPE_MIX;
	c->brush_opacity                     = 1.0;
	c->brush_opacity_handle              = ui_handle_create();
	c->brush_opacity_handle->f           = 1.0;
	c->brush_scale                       = 1.0;
	c->brush_angle                       = 0.0;
	c->brush_angle_handle                = ui_handle_create();
	c->brush_angle_handle->f             = 0.0;
	c->brush_lazy_radius                 = 0.0;
	c->brush_lazy_step                   = 0.0;
	c->brush_lazy_x                      = 0.0;
	c->brush_lazy_y                      = 0.0;
	c->brush_paint                       = UV_TYPE_UVMAP;
	c->brush_angle_reject_dot            = 0.5;
	c->bake_type                         = BAKE_TYPE_CURVATURE;
	c->bake_axis                         = BAKE_AXIS_XYZ;
	c->bake_up_axis                      = BAKE_UP_AXIS_Z;
	c->bake_samples                      = 128;
	c->bake_ao_strength                  = 1.0;
	c->bake_ao_radius                    = 1.0;
	c->bake_ao_offset                    = 1.0;
	c->bake_curv_strength                = 1.0;
	c->bake_curv_radius                  = 1.0;
	c->bake_curv_offset                  = 0.0;
	c->bake_curv_smooth                  = 1;
	c->bake_high_poly                    = 0;
	c->xray                              = false;
	c->sym_x                             = false;
	c->sym_y                             = false;
	c->sym_z                             = false;
	c->fill_type_handle                  = ui_handle_create();
	c->paint2d                           = false;
	c->maximized_sidebar_width           = 0;
	c->drag_dest                         = 0;
	return c;
}

void context_init() {
	context_raw = context_create();
	gc_root(context_raw);
	context_raw->tool                       = TOOL_TYPE_BRUSH;
	context_raw->color_picker_previous_tool = TOOL_TYPE_BRUSH;
	context_raw->brush_radius               = 0.5;
	context_raw->brush_radius_handle->f     = 0.5;
	context_raw->brush_hardness             = 0.8;
}

bool context_use_deferred() {
	return config_raw->render_mode != RENDER_MODE_FORWARD &&
	       (context_raw->viewport_mode == VIEWPORT_MODE_LIT || context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE) &&
	       context_raw->tool != TOOL_TYPE_COLORID;
}

void context_select_material(i32 i) {
	if (project_materials->length <= i) {
		return;
	}
	context_set_material(project_materials->buffer[i]);
}

void context_set_material_on_next_frame(void *_) {
	util_render_make_decal_preview();
}

void context_set_material(slot_material_t *m) {
	if (array_index_of(project_materials, m) == -1) {
		return;
	}
	context_raw->material = m;
	make_material_parse_paint_material(true);
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
	ui_header_handle->redraws                         = 2;
	ui_nodes_hwnd->redraws                            = 2;
	gc_unroot(ui_nodes_group_stack);
	ui_nodes_group_stack = any_array_create_from_raw((void *[]){}, 0);
	gc_root(ui_nodes_group_stack);

	bool decal = context_is_decal();
	if (decal) {
		sys_notify_on_next_frame(&context_set_material_on_next_frame, NULL);
	}
}

void context_select_brush(i32 i) {
	if (project_brushes->length <= i) {
		return;
	}
	context_set_brush(project_brushes->buffer[i]);
}

void context_set_brush(slot_brush_t *b) {
	if (array_index_of(project_brushes, b) == -1) {
		return;
	}
	context_raw->brush = b;
	make_material_parse_brush();
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
	ui_nodes_hwnd->redraws                            = 2;
}

void context_select_font(i32 i) {
	if (project_fonts->length <= i) {
		return;
	}
	context_set_font(project_fonts->buffer[i]);
}

void context_set_font(slot_font_t *f) {
	if (array_index_of(project_fonts, f) == -1) {
		return;
	}
	context_raw->font = f;
	util_render_make_text_preview();
	util_render_make_decal_preview();
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
	ui_view2d_hwnd->redraws                         = 2;
}

void context_select_layer(i32 i) {
	if (project_layers->length <= i) {
		return;
	}
	context_set_layer(project_layers->buffer[i]);
}

void context_set_layer(slot_layer_t *l) {
	if (l == context_raw->layer) {
		return;
	}
	context_raw->layer        = l;
	ui_header_handle->redraws = 2;

	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use)
		draw_end();

	layers_set_object_mask();
	make_material_parse_mesh_material();
	make_material_parse_paint_material(true);

	if (in_use)
		draw_begin(current, false, 0);

	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
	ui_view2d_hwnd->redraws                           = 2;
}

void context_select_tool(i32 i) {
	context_raw->tool = i;
	make_material_parse_paint_material(true);
	make_material_parse_mesh_material();
	context_raw->ddirty            = 3;
	viewport_mode_t _viewport_mode = context_raw->viewport_mode;
	context_raw->viewport_mode     = VIEWPORT_MODE_MINUS_ONE;
	context_set_viewport_mode(_viewport_mode);

	context_init_tool();
	ui_header_handle->redraws  = 2;
	ui_toolbar_handle->redraws = 2;
}

void context_init_tool() {
	bool decal = context_is_decal();
	if (decal) {
		if (context_raw->tool == TOOL_TYPE_TEXT) {
			util_render_make_text_preview();
		}
		util_render_make_decal_preview();
	}
	else if (context_raw->tool == TOOL_TYPE_PARTICLE) {
		util_particle_init();
	}
	else if (context_raw->tool == TOOL_TYPE_BAKE) {
		// Bake in lit mode for now
		if (context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE) {
			context_raw->viewport_mode = VIEWPORT_MODE_LIT;
		}
	}
	else if (context_raw->tool == TOOL_TYPE_MATERIAL) {
		layers_update_fill_layers();
		context_main_object()->skip_context = NULL;
	}
}

void context_select_paint_object(mesh_object_t *o) {
	ui_header_handle->redraws = 2;
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *p = project_paint_objects->buffer[i];
		p->skip_context  = "paint";
	}

	// #ifdef is_forge
	// context_raw->paint_object->skip_context = "";
	// #endif

	context_raw->paint_object = o;

	i32 mask = slot_layer_get_object_mask(context_raw->layer);
	if (context_layer_filter_used()) {
		mask = context_raw->layer_filter;
	}

	if (context_raw->merged_object == NULL || mask > 0) {
		context_raw->paint_object->skip_context = "";
	}
	util_uv_uvmap_cached       = false;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached   = false;
}

mesh_object_t *context_main_object() {
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *po = project_paint_objects->buffer[i];
		if (po->base->children->length > 0) {
			return po;
		}
	}
	return project_paint_objects->buffer[0];
}

bool context_layer_filter_used() {
	return context_raw->layer_filter > 0 && context_raw->layer_filter <= project_paint_objects->length;
}

bool context_object_mask_used() {
	return slot_layer_get_object_mask(context_raw->layer) > 0 && slot_layer_get_object_mask(context_raw->layer) <= project_paint_objects->length;
}

bool context_in_3d_view() {
	return context_raw->paint_vec.x < 1 && context_raw->paint_vec.x > 0 && context_raw->paint_vec.y < 1 && context_raw->paint_vec.y > 0;
}

bool context_in_paint_area() {
	return context_in_3d_view() || context_in_2d_view(VIEW_2D_TYPE_LAYER);
}

bool context_in_layers() {
	char *tab = ui_hovered_tab_name();
	return string_equals(tab, tr("Layers"));
}

bool context_in_materials() {
	char *tab = ui_hovered_tab_name();
	return string_equals(tab, tr("Materials"));
}

bool context_in_2d_view(view_2d_type_t type) {
	return ui_view2d_show && ui_view2d_type == type && mouse_x > ui_view2d_wx && mouse_x < ui_view2d_wx + ui_view2d_ww && mouse_y > ui_view2d_wy &&
	       mouse_y < ui_view2d_wy + ui_view2d_wh;
}

bool context_in_nodes() {
	return ui_nodes_show && mouse_x > ui_nodes_wx && mouse_x < ui_nodes_wx + ui_nodes_ww && mouse_y > ui_nodes_wy && mouse_y < ui_nodes_wy + ui_nodes_wh;
}

bool context_in_swatches() {
	char *tab = ui_hovered_tab_name();
	return string_equals(tab, tr("Swatches"));
}

bool context_in_browser() {
	char *tab = ui_hovered_tab_name();
	return string_equals(tab, tr("Browser"));
}

bool context_is_picker() {
	return context_raw->tool == TOOL_TYPE_PICKER || context_raw->tool == TOOL_TYPE_MATERIAL;
}

bool context_is_decal() {
	return context_raw->tool == TOOL_TYPE_DECAL || context_raw->tool == TOOL_TYPE_TEXT;
}

bool context_is_decal_mask() {
	return context_is_decal() && operator_shortcut(any_map_get(config_keymap, "decal_mask"), SHORTCUT_TYPE_DOWN);
}

bool context_is_decal_mask_paint() {
	return context_is_decal() &&
	       operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "action_paint")), SHORTCUT_TYPE_DOWN);
}

bool context_is_floating_toolbar() {
	// Header is off -> floating toolbar
	return config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 0 || (!base_view3d_show && ui_view2d_show);
}

area_type_t context_get_area_type() {
	if (context_in_3d_view()) {
		return AREA_TYPE_VIEW3D;
	}
	if (context_in_nodes()) {
		return AREA_TYPE_NODES;
	}
	if (context_in_browser()) {
		return AREA_TYPE_BROWSER;
	}
	if (context_in_2d_view(VIEW_2D_TYPE_LAYER)) {
		return AREA_TYPE_VIEW2D;
	}
	if (context_in_layers()) {
		return AREA_TYPE_LAYERS;
	}
	if (context_in_materials()) {
		return AREA_TYPE_MATERIALS;
	}
	return AREA_TYPE_MINUS_ONE;
}

void context_set_viewport_mode(viewport_mode_t mode) {
	if (mode == context_raw->viewport_mode) {
		return;
	}

	context_raw->viewport_mode = mode;
	if (context_use_deferred()) {
		gc_unroot(render_path_commands);
		render_path_commands = render_path_deferred_commands;
		gc_root(render_path_commands);
	}
	else {
		gc_unroot(render_path_commands);
		render_path_commands = render_path_forward_commands;
		gc_root(render_path_commands);
	}
	make_material_parse_mesh_material();

	// Rotate mode is not supported for path tracing yet
	if (context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE && context_raw->camera_controls == CAMERA_CONTROLS_ROTATE) {
		context_raw->camera_controls = CAMERA_CONTROLS_ORBIT;
		viewport_reset();
	}

	// Bake in lit mode for now
	if (context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE && context_raw->tool == TOOL_TYPE_BAKE) {
		context_raw->viewport_mode = VIEWPORT_MODE_LIT;
	}
}

void context_load_envmap() {
	if (!context_raw->envmap_loaded) {
		// TODO: Unable to share texture for both radiance and envmap - reload image
		context_raw->envmap_loaded = true;
		map_delete(data_cached_images, "World_radiance.k");
	}
	world_data_load_envmap(scene_world);
	if (context_raw->saved_envmap == NULL) {
		context_raw->saved_envmap = scene_world->_->envmap;
	}
}

void context_update_envmap() {
	if (context_raw->show_envmap) {
		scene_world->_->envmap = context_raw->show_envmap_blur ? scene_world->_->radiance_mipmaps->buffer[0] : context_raw->saved_envmap;
	}
	else {
		scene_world->_->envmap = context_raw->empty_envmap;
	}
}

void context_set_viewport_shader(void *viewport_shader) { // JSValue * -> (ns: node_shader_t)=>void
	context_raw->viewport_shader = viewport_shader;
	context_set_render_path();
}

void context_set_render_path_on_next_frame(void *_) {
	make_material_parse_mesh_material();
}

void context_set_render_path() {
	if (config_raw->render_mode == RENDER_MODE_FORWARD || context_raw->viewport_shader != NULL) {
		gc_unroot(render_path_commands);
		render_path_commands = render_path_forward_commands;
		gc_root(render_path_commands);
	}
	else {
		gc_unroot(render_path_commands);
		render_path_commands = render_path_deferred_commands;
		gc_root(render_path_commands);
	}
	sys_notify_on_next_frame(&context_set_render_path_on_next_frame, NULL);
}

bool context_enable_import_plugin(char *file) {
	// Return plugin name suitable for importing the specified file
	if (box_preferences_files_plugin == NULL) {
		box_preferences_fetch_plugins();
	}
	char *ext = substring(file, string_last_index_of(file, ".") + 1, string_length(file));
	for (i32 i = 0; i < box_preferences_files_plugin->length; ++i) {
		char *f = box_preferences_files_plugin->buffer[i];
		if (starts_with(f, "import_") && string_index_of(f, ext) >= 0) {
			config_enable_plugin(f);
			console_info(string("%s %s", f, tr("plugin enabled")));
			return true;
		}
	}
	return false;
}

void context_set_swatch(swatch_color_t *s) {
	context_raw->swatch = s;
}
