
#include "global.h"

void uniforms_ext_init() {
	gc_unroot(uniforms_i32_links);
	uniforms_i32_links = uniforms_ext_i32_link;
	gc_root(uniforms_i32_links);
	gc_unroot(uniforms_f32_links);
	uniforms_f32_links = uniforms_ext_f32_link;
	gc_root(uniforms_f32_links);
	gc_unroot(uniforms_vec2_links);
	uniforms_vec2_links = uniforms_ext_vec2_link;
	gc_root(uniforms_vec2_links);
	gc_unroot(uniforms_vec3_links);
	uniforms_vec3_links = uniforms_ext_vec3_link;
	gc_root(uniforms_vec3_links);
	gc_unroot(uniforms_vec4_links);
	uniforms_vec4_links = uniforms_ext_vec4_link;
	gc_root(uniforms_vec4_links);
	gc_unroot(uniforms_mat4_links);
	uniforms_mat4_links = uniforms_ext_mat4_link;
	gc_root(uniforms_mat4_links);
	gc_unroot(uniforms_tex_links);
	uniforms_tex_links = uniforms_ext_tex_link;
	gc_root(uniforms_tex_links);
}

i32 uniforms_ext_i32_link(object_t *object, material_data_t *mat, char *link) {
	if (string_equals(link, "_bloom_current_mip")) {
		return render_path_base_bloom_current_mip;
	}
	return INT_MAX;
}

f32 uniforms_ext_f32_link(object_t *object, material_data_t *mat, char *link) {
	if (string_equals(link, "_brush_radius")) {
		bool decal      = context_is_decal();
		bool decal_mask = decal && operator_shortcut(string("%s+%s", any_map_get(config_keymap, "decal_mask"), any_map_get(config_keymap, "action_paint")),
		                                             SHORTCUT_TYPE_DOWN);
		f32  brush_decal_mask_radius = context_raw->brush_decal_mask_radius;
		brush_decal_mask_radius *= context_raw->paint2d ? 0.55 * ui_view2d_pan_scale : 2.0;
		f32 radius = decal_mask ? brush_decal_mask_radius : context_raw->brush_radius;
		f32 val    = (radius * context_raw->brush_nodes_radius) / 15.0;
		if (config_raw->pressure_radius && pen_down("tip")) {
			val *= pen_pressure * config_raw->pressure_sensitivity;
		}
		f32 scale2d = (900 / (float)base_h()) * config_raw->window_scale;
		if (!decal) {
			val *= context_raw->paint2d ? 0.55 * scale2d * ui_view2d_pan_scale : 2;
		}
		else {
			val *= scale2d; // Projection ratio
		}
		return val;
	}
	else if (string_equals(link, "_vignette_strength")) {
		return config_raw->rp_vignette;
	}
	else if (string_equals(link, "_grain_strength")) {
		return config_raw->rp_grain;
	}
	else if (string_equals(link, "_tonemap_strength")) {
		bool tonemap = context_raw->viewport_mode == VIEWPORT_MODE_LIT || context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE;
		return tonemap ? 1.0 : 0.0;
	}
	else if (string_equals(link, "_bloom_sample_scale")) {
		return render_path_base_bloom_sample_scale;
	}
	else if (string_equals(link, "_bloom_strength")) {
		return context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE ? 0.2 : 0.02;
	}
	else if (string_equals(link, "_brush_scale_x")) {
		return 1 / (float)context_raw->brush_scale_x;
	}
	else if (string_equals(link, "_brush_opacity")) {
		f32 val = context_raw->brush_opacity * context_raw->brush_nodes_opacity;
		if (config_raw->pressure_opacity && pen_down("tip")) {
			val *= pen_pressure * config_raw->pressure_sensitivity;
		}
		return val;
	}
	else if (string_equals(link, "_brush_hardness")) {
		bool decal_mask = context_is_decal_mask_paint();
		if (context_raw->tool != TOOL_TYPE_BRUSH && context_raw->tool != TOOL_TYPE_ERASER && context_raw->tool != TOOL_TYPE_CLONE && !decal_mask) {
			return 1.0;
		}
		f32 val = context_raw->brush_hardness * context_raw->brush_nodes_hardness;
		if (config_raw->pressure_hardness && pen_down("tip")) {
			val *= pen_pressure * config_raw->pressure_sensitivity;
		}
		if (context_raw->paint2d) {
			val *= 1.0 / (float)ui_view2d_pan_scale;
		}
		else {
			val *= val;
		}
		return val;
	}
	else if (string_equals(link, "_brush_scale")) {
		if (context_raw->tool == TOOL_TYPE_GIZMO) {
			i32 atlas_w      = config_get_scene_atlas_res();
			i32 item_w       = config_get_layer_res();
			i32 atlas_stride = atlas_w / (float)item_w;
			return atlas_stride;
		}
		bool fill = context_raw->layer->fill_layer != NULL;
		f32  val  = (fill ? context_raw->layer->scale : context_raw->brush_scale) * context_raw->brush_nodes_scale;
		return val;
	}
	else if (string_equals(link, "_object_id")) {
		return array_index_of(project_paint_objects, object->ext);
	}
	else if (string_equals(link, "_dilate_radius")) {
		return util_uv_dilatemap != NULL ? config_raw->dilate_radius : 0.0;
	}
	else if (string_equals(link, "_decal_layer_dim")) {
		vec4_t sc = mat4_get_scale(context_raw->layer->decal_mat);
		return sc.z * 0.5;
	}
	else if (string_equals(link, "_picker_opacity")) {
		return context_raw->picked_color->opacity;
	}
	else if (string_equals(link, "_picker_occlusion")) {
		return context_raw->picked_color->occlusion;
	}
	else if (string_equals(link, "_picker_roughness")) {
		return context_raw->picked_color->roughness;
	}
	else if (string_equals(link, "_picker_metallic")) {
		return context_raw->picked_color->metallic;
	}
	else if (string_equals(link, "_picker_height")) {
		return context_raw->picked_color->height;
	}
	else if (string_equals(link, "_taa_blend")) {
		if (render_path_base_taa_frame == 0) {
			return 0.0;
		}
		if (context_raw->ddirty > 1 || context_raw->pdirty > 0) {
			camera_object_taa_frames = 2;
			return 0.5;
		}
		camera_object_taa_frames = 6;
		return 0.833;
	}
	if (parser_material_script_links != NULL) {
		string_t_array_t *keys = map_keys(parser_material_script_links);
		for (i32 i = 0; i < keys->length; ++i) {
			char *key    = keys->buffer[i];
			char *script = any_map_get(parser_material_script_links, key);
			f32   result = NAN;
			if (!string_equals(script, "")) {
				result = js_eval(script);
			}
			return result;
		}
	}
	return NAN;
}

vec2_t uniforms_ext_vec2_link(object_t *object, material_data_t *mat, char *link) {
	if (string_equals(link, "_gbuffer_size")) {
		render_target_t *gbuffer2 = any_map_get(render_path_render_targets, "gbuffer2");
		return vec2_create(gbuffer2->_image->width, gbuffer2->_image->height);
	}
	else if (string_equals(link, "_clone_delta")) {
		return vec2_create(context_raw->clone_delta_x, context_raw->clone_delta_y);
	}
	else if (string_equals(link, "_texpaint_size")) {
		return vec2_create(config_get_texture_res_x(), config_get_texture_res_y());
	}
	else if (string_equals(link, "_brush_angle")) {
		f32 brush_angle = context_raw->brush_angle + context_raw->brush_nodes_angle;
		f32 angle       = context_raw->layer->fill_layer != NULL ? context_raw->layer->angle : brush_angle;
		angle *= (math_pi() / 180.0);
		if (config_raw->pressure_angle && pen_down("tip")) {
			angle *= pen_pressure * config_raw->pressure_sensitivity;
		}
		return vec2_create(math_cos(-angle), math_sin(-angle));
	}
	return vec2_nan();
}

vec4_t uniforms_ext_vec3_link(object_t *object, material_data_t *mat, char *link) {
	vec4_t v = vec4_nan();
	if (string_equals(link, "_brush_direction")) {
		v = _uniforms_vec;
		// Discard first paint for directional brush
		bool allow_paint = context_raw->prev_paint_vec_x != context_raw->last_paint_vec_x && context_raw->prev_paint_vec_y != context_raw->last_paint_vec_y &&
		                   context_raw->prev_paint_vec_x > 0 && context_raw->prev_paint_vec_y > 0;
		f32 x     = context_raw->paint_vec.x;
		f32 y     = context_raw->paint_vec.y;
		f32 lastx = context_raw->prev_paint_vec_x;
		f32 lasty = context_raw->prev_paint_vec_y;
		if (context_raw->paint2d) {
			x     = uniforms_ext_vec2d(x);
			lastx = uniforms_ext_vec2d(lastx);
		}
		f32 angle                     = math_atan2(-y + lasty, x - lastx) - math_pi() / 2.0;
		v                             = vec4_create(math_cos(angle), math_sin(angle), allow_paint ? 1 : 0, 1.0);
		context_raw->prev_paint_vec_x = context_raw->last_paint_vec_x;
		context_raw->prev_paint_vec_y = context_raw->last_paint_vec_y;
		return v;
	}
	else if (string_equals(link, "_decal_layer_loc")) {
		v = _uniforms_vec;
		v = vec4_create(context_raw->layer->decal_mat.m30, context_raw->layer->decal_mat.m31, context_raw->layer->decal_mat.m32, 1.0);
		return v;
	}
	else if (string_equals(link, "_decal_layer_nor")) {
		v = _uniforms_vec;
		v = vec4_create(context_raw->layer->decal_mat.m20, context_raw->layer->decal_mat.m21, context_raw->layer->decal_mat.m22, 1.0);
		v = vec4_norm(v);
		return v;
	}
	else if (string_equals(link, "_picker_base")) {
		v = _uniforms_vec;
		v = vec4_create(color_get_rb(context_raw->picked_color->base) / 255.0, color_get_gb(context_raw->picked_color->base) / 255.0,
		                color_get_bb(context_raw->picked_color->base) / 255.0, 1.0);
		return v;
	}
	else if (string_equals(link, "_picker_normal")) {
		v = _uniforms_vec;
		v = vec4_create(color_get_rb(context_raw->picked_color->normal) / 255.0, color_get_gb(context_raw->picked_color->normal) / 255.0,
		                color_get_bb(context_raw->picked_color->normal) / 255.0, 1.0);
		return v;
	}
	else if (string_equals(link, "_particle_hit")) {
		v = _uniforms_vec;
		v = vec4_create(context_raw->particle_hit_x, context_raw->particle_hit_y, context_raw->particle_hit_z, 1.0);
		return v;
	}
	else if (string_equals(link, "_particle_hit_last")) {
		v = _uniforms_vec;
		v = vec4_create(context_raw->last_particle_hit_x, context_raw->last_particle_hit_y, context_raw->last_particle_hit_z, 1.0);
		return v;
	}
	return v;
}

f32 uniforms_ext_vec2d(f32 x) {
	// Transform from 3d viewport coord to 2d view coord
	context_raw->paint2d_view = false;
	f32 res                   = (x * base_w() - base_w()) / (float)ui_view2d_ww;
	context_raw->paint2d_view = true;
	return res;
}

vec4_t uniforms_ext_vec4_link(object_t *object, material_data_t *mat, char *link) {
	if (string_equals(link, "_input_brush")) {
		bool   down = mouse_down("left") || pen_down("tip");
		vec4_t v    = vec4_create(context_raw->paint_vec.x, context_raw->paint_vec.y, down ? 1.0 : 0.0, context_raw->paint2d ? 1.0 : 0.0);
		if (context_raw->paint2d) {
			v.x = uniforms_ext_vec2d(v.x);
		}

		return v;
	}
	else if (string_equals(link, "_input_brush_last")) {
		bool   down = mouse_down("left") || pen_down("tip");
		vec4_t v    = vec4_create(context_raw->last_paint_vec_x, context_raw->last_paint_vec_y, down ? 1.0 : 0.0, context_raw->paint2d ? 1.0 : 0.0);
		if (context_raw->paint2d) {
			v.x = uniforms_ext_vec2d(v.x);
		}

		return v;
	}
	else if (string_equals(link, "_envmap_data")) {
		return vec4_create(context_raw->envmap_angle, math_sin(-context_raw->envmap_angle), math_cos(-context_raw->envmap_angle), scene_world->strength * 2.0);
	}
	else if (string_equals(link, "_envmap_data_world")) {
		bool tonemap = context_raw->viewport_mode == VIEWPORT_MODE_LIT || context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE;
		return vec4_create(context_raw->envmap_angle, tonemap ? 0.0 : 1.0, 0.0, context_raw->show_envmap ? scene_world->strength : 1.0);
	}
	else if (string_equals(link, "_stencil_transform")) {
		vec4_t v = vec4_create(context_raw->brush_stencil_x, context_raw->brush_stencil_y, context_raw->brush_stencil_scale, context_raw->brush_stencil_angle);
		if (context_raw->paint2d) {
			v.x = uniforms_ext_vec2d(v.x);
		}

		return v;
	}
	else if (string_equals(link, "_decal_mask")) {
		bool decal_mask = context_is_decal_mask_paint();
		f32  val        = (context_raw->brush_radius * context_raw->brush_nodes_radius) / 15.0;
		f32  scale2d    = (900 / (float)base_h()) * config_raw->window_scale;
		val *= scale2d; // Projection ratio
		vec4_t v = vec4_create(context_raw->decal_x, context_raw->decal_y, decal_mask ? 1 : 0, val);
		if (context_raw->paint2d) {
			v.x = uniforms_ext_vec2d(v.x);
		}

		return v;
	}

	return vec4_nan();
}

mat4_t uniforms_ext_mat4_link(object_t *object, material_data_t *mat, char *link) {
	if (string_equals(link, "_decal_layer_matrix")) { // Decal layer
		mat4_t m = mat4_inv(context_raw->layer->decal_mat);
		f32    f = object->parent->transform->scale.x * object->transform->scale_world;
		m        = mat4_scale(m, vec4_create(f, f, f, 1.0));
		m        = mat4_mult_mat(m, uniforms_ext_ortho_p);
		return m;
	}

	return mat4_nan();
}

void uniforms_ext_cache_uv_island_map(void *_) {
	util_uv_cache_uv_island_map();
}

void uniforms_ext_cache_triangle_map(void *_) {
	util_uv_cache_triangle_map();
}

void uniforms_ext_cache_uv_map(void *_) {
	util_uv_cache_uv_map();
}

gpu_texture_t *uniforms_ext_tex_link(object_t *object, material_data_t *mat, char *link) {
	if (string_equals(link, "_texpaint_undo")) {
		i32              i  = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
		render_target_t *rt = any_map_get(render_path_render_targets, string("texpaint_undo%d", i));
		return rt->_image;
	}
	else if (string_equals(link, "_texpaint_nor_undo")) {
		i32              i  = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
		render_target_t *rt = any_map_get(render_path_render_targets, string("texpaint_nor_undo%d", i));
		return rt->_image;
	}
	else if (string_equals(link, "_texpaint_pack_undo")) {
		i32              i  = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
		render_target_t *rt = any_map_get(render_path_render_targets, string("texpaint_pack_undo%d", i));
		return rt->_image;
	}
	else if (string_equals(link, "_texpaint_sculpt_undo")) {
		i32              i  = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
		render_target_t *rt = any_map_get(render_path_render_targets, string("texpaint_sculpt_undo%d", i));
		return rt->_image;
	}
	else if (string_equals(link, "_texcolorid")) {
		if (project_assets->length == 0) {
			render_target_t *rt = any_map_get(render_path_render_targets, "empty_white");
			return rt->_image;
		}
		else {
			return project_get_image(project_assets->buffer[context_raw->colorid_handle->i]);
		}
	}
	else if (string_equals(link, "_textexttool")) { // Opacity map for text
		return context_raw->text_tool_image;
	}
	else if (string_equals(link, "_texbrushmask")) {
		return context_raw->brush_mask_image;
	}
	else if (string_equals(link, "_texbrushstencil")) {
		return context_raw->brush_stencil_image;
	}
	else if (string_equals(link, "_texparticle")) {
		render_target_t *rt = any_map_get(render_path_render_targets, "texparticle");
		return rt->_image;
	}
	else if (string_equals(link, "_texuvmap")) {
		if (!util_uv_uvmap_cached) {
			sys_notify_on_next_frame(&uniforms_ext_cache_uv_map, NULL);
		}
		return util_uv_uvmap;
	}
	else if (string_equals(link, "_textrianglemap")) {
		if (!util_uv_trianglemap_cached) {
			sys_notify_on_next_frame(&uniforms_ext_cache_triangle_map, NULL);
		}
		return util_uv_trianglemap;
	}
	else if (string_equals(link, "_texuvislandmap")) {
		sys_notify_on_next_frame(&uniforms_ext_cache_uv_island_map, NULL);
		if (util_uv_uvislandmap_cached) {
			return util_uv_uvislandmap;
		}
		else {
			render_target_t *rt = any_map_get(render_path_render_targets, "empty_black");
			return rt->_image;
		}
	}
	else if (string_equals(link, "_texdilatemap")) {
		return util_uv_dilatemap;
	}
	if (starts_with(link, "_texpaint_pack_vert")) {
		char            *tid = substring(link, string_length(link) - 1, string_length(link));
		render_target_t *rt  = any_map_get(render_path_render_targets, string("texpaint_pack%s", tid));
		return rt->_image;
	}
	if (starts_with(link, "_texpaint_vert")) {
		i32 tid = parse_int(substring(link, string_length(link) - 1, string_length(link)));
		return tid < project_layers->length ? project_layers->buffer[tid]->texpaint : NULL;
	}
	if (starts_with(link, "_texpaint_nor")) {
		i32 tid = parse_int(substring(link, string_length(link) - 1, string_length(link)));
		return tid < project_layers->length ? project_layers->buffer[tid]->texpaint_nor : NULL;
	}
	if (starts_with(link, "_texpaint_pack")) {
		i32 tid = parse_int(substring(link, string_length(link) - 1, string_length(link)));
		return tid < project_layers->length ? project_layers->buffer[tid]->texpaint_pack : NULL;
	}
	if (starts_with(link, "_texpaint_sculpt")) {
		i32 tid = parse_int(substring(link, string_length(link) - 1, string_length(link)));
		return tid < project_layers->length ? project_layers->buffer[tid]->texpaint_sculpt : NULL;
	}
	if (starts_with(link, "_texpaint")) {
		i32 tid = parse_int(substring(link, string_length(link) - 1, string_length(link)));
		return tid < project_layers->length ? project_layers->buffer[tid]->texpaint : NULL;
	}
	if (starts_with(link, "_texblur_")) {
		char *id = substring(link, 9, string_length(link));
		if (context_raw->node_previews != NULL) {
			return any_map_get(context_raw->node_previews, id);
		}
		else {
			render_target_t *rt = any_map_get(render_path_render_targets, "empty_black");
			return rt->_image;
		}
	}
	if (starts_with(link, "_texwarp_")) {
		char *id = substring(link, 9, string_length(link));
		if (context_raw->node_previews != NULL) {
			return any_map_get(context_raw->node_previews, id);
		}
		else {
			render_target_t *rt = any_map_get(render_path_render_targets, "empty_black");
			return rt->_image;
		}
	}
	if (starts_with(link, "_texbake_")) {
		char *id = substring(link, 9, string_length(link));
		if (context_raw->node_previews != NULL) {
			return any_map_get(context_raw->node_previews, id);
		}
		else {
			render_target_t *rt = any_map_get(render_path_render_targets, "empty_black");
			return rt->_image;
		}
	}
	if (string_equals(link, "_camera_texture")) {
		render_target_t *rt = any_map_get(render_path_render_targets, "last");
		return rt->_image;
	}
	return NULL;
}
