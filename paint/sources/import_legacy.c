
#include "global.h"

ui_node_socket_t_array_t *import_arm_get_node_socket_array(any_map_t *old, char *key) {
	ui_node_socket_t_array_t *sockets = any_array_create_from_raw((void *[]){}, 0);
	any_array_t              *ias     = any_map_get(old, key);
	for (i32 i = 0; i < ias->length; ++i) {
		any_map_t        *old = ias->buffer[i];
		ui_node_socket_t *s   = GC_ALLOC_INIT(ui_node_socket_t, {0});
		s->id                 = armpack_map_get_i32(old, "id");
		s->node_id            = armpack_map_get_i32(old, "node_id");
		s->name               = string_copy(any_map_get(old, "name"));
		s->type               = string_copy(any_map_get(old, "type"));
		s->color              = armpack_map_get_i32(old, "color");
		s->default_value      = any_map_get(old, "default_value");
		s->min                = armpack_map_get_f32(old, "min");
		s->max                = armpack_map_get_f32(old, "max");
		s->precision          = armpack_map_get_f32(old, "precision");
		s->display            = armpack_map_get_i32(old, "display");
		any_array_push(sockets, s);
	}
	return sockets;
}

ui_node_canvas_t_array_t *import_arm_get_node_canvas_array(any_map_t *map, char *key) {
	any_array_t *cas = any_map_get(map, key);
	if (cas == NULL) {
		return NULL;
	}
	ui_node_canvas_t_array_t *ar = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < cas->length; ++i) {
		any_map_t        *old = cas->buffer[i];
		ui_node_canvas_t *c   = GC_ALLOC_INIT(ui_node_canvas_t, {0});
		c->name               = string_copy(any_map_get(old, "name"));
		c->nodes              = any_array_create_from_raw((void *[]){}, 0);
		any_array_t *ns       = any_map_get(old, "nodes");
		for (i32 i = 0; i < ns->length; ++i) {
			any_map_t *old   = ns->buffer[i];
			ui_node_t *n     = GC_ALLOC_INIT(ui_node_t, {0});
			n->id            = armpack_map_get_i32(old, "id");
			n->name          = string_copy(any_map_get(old, "name"));
			n->type          = string_copy(any_map_get(old, "type"));
			n->x             = armpack_map_get_f32(old, "x");
			n->y             = armpack_map_get_f32(old, "y");
			n->color         = armpack_map_get_i32(old, "color");
			n->inputs        = import_arm_get_node_socket_array(old, "inputs");
			n->outputs       = import_arm_get_node_socket_array(old, "outputs");
			n->buttons       = any_array_create_from_raw((void *[]){}, 0);
			any_array_t *bas = any_map_get(old, "buttons");
			for (i32 i = 0; i < bas->length; ++i) {
				any_map_t        *old = bas->buffer[i];
				ui_node_button_t *b   = GC_ALLOC_INIT(ui_node_button_t, {0});
				b->name               = string_copy(any_map_get(old, "name"));
				b->type               = string_copy(any_map_get(old, "type"));
				b->output             = armpack_map_get_i32(old, "output");
				b->default_value      = any_map_get(old, "default_value");
				b->data               = any_map_get(old, "data");
				b->min                = armpack_map_get_f32(old, "min");
				b->max                = armpack_map_get_f32(old, "max");
				b->precision          = armpack_map_get_f32(old, "precision");
				b->height             = armpack_map_get_f32(old, "height");
				any_array_push(n->buttons, b);
			}
			n->width = armpack_map_get_f32(old, "width");
			n->flags = armpack_map_get_i32(old, "flags");
			any_array_push(c->nodes, n);
		}
		c->links         = any_array_create_from_raw((void *[]){}, 0);
		any_array_t *las = any_map_get(old, "links");
		for (i32 i = 0; i < las->length; ++i) {
			any_map_t      *old = las->buffer[i];
			ui_node_link_t *l   = GC_ALLOC_INIT(ui_node_link_t, {0});
			l->id               = armpack_map_get_i32(old, "id");
			l->from_id          = armpack_map_get_i32(old, "from_id");
			l->from_socket      = armpack_map_get_i32(old, "from_socket");
			l->to_id            = armpack_map_get_i32(old, "to_id");
			l->to_socket        = armpack_map_get_i32(old, "to_socket");
			any_array_push(c->links, l);
		}
		any_array_push(ar, c);
	}
	return ar;
}

bool import_arm_is_version_4(buffer_t *b) {
	bool has_version = b->buffer[10] == 118; // 'v'
	bool has_four    = b->buffer[22] == 52;  // '4'
	if (has_version && has_four) {
		return true;
	}
	return false;
}

bool import_arm_is_version_3(buffer_t *b) {
	bool has_version = b->buffer[10] == 118; // 'v'
	bool has_three   = b->buffer[22] == 51;  // '3'
	if (has_version && has_three) {
		return true;
	}
	return false;
}

bool import_arm_is_version_2(buffer_t *b) {
	bool has_version = b->buffer[10] == 118; // 'v'
	bool has_two     = b->buffer[22] == 50;  // '2'
	if (has_version && has_two) {
		return true;
	}
	return false;
}

bool import_arm_is_old(buffer_t *b) {
	return import_arm_is_version_2(b) || import_arm_is_version_3(b) || import_arm_is_version_4(b);
}

project_t *import_arm_from_map_to_arm(any_map_t *old) {
	project_t *project = GC_ALLOC_INIT(project_t, {0});
	project->version   = string_copy(manifest_version_project);
	project->assets    = any_map_get(old, "assets");
	project->is_bgra   = armpack_map_get_i32(old, "is_bgra") > 0;

	any_array_t *pas = any_map_get(old, "packed_assets");
	if (pas != NULL) {
		project->packed_assets = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < pas->length; ++i) {
			any_map_t      *old = pas->buffer[i];
			packed_asset_t *pa  = GC_ALLOC_INIT(packed_asset_t, {0});
			pa->name            = string_copy(any_map_get(old, "name"));
			pa->bytes           = any_map_get(old, "bytes");
			any_array_push(project->packed_assets, pa);
		}
	}
	project->envmap          = string_copy(any_map_get(old, "envmap"));
	project->envmap_strength = armpack_map_get_f32(old, "envmap_strength");
	project->envmap_angle    = armpack_map_get_f32(old, "envmap_angle");
	project->envmap_blur     = armpack_map_get_i32(old, "envmap_blur");
	project->camera_world    = any_map_get(old, "camera_world");
	project->camera_origin   = any_map_get(old, "camera_origin");
	project->camera_fov      = armpack_map_get_f32(old, "camera_fov");

	any_array_t *ss = any_map_get(old, "swatches");
	if (ss != NULL) {
		project->swatches = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < ss->length; ++i) {
			any_map_t      *old = ss->buffer[i];
			swatch_color_t *s   = GC_ALLOC_INIT(swatch_color_t, {0});
			s->base             = armpack_map_get_i32(old, "base");
			s->opacity          = armpack_map_get_f32(old, "opacity");
			s->occlusion        = armpack_map_get_f32(old, "occlusion");
			s->roughness        = armpack_map_get_f32(old, "roughness");
			s->metallic         = armpack_map_get_f32(old, "metallic");
			s->normal           = armpack_map_get_i32(old, "normal");
			s->emission         = armpack_map_get_f32(old, "emission");
			s->height           = armpack_map_get_f32(old, "height");
			s->subsurface       = armpack_map_get_f32(old, "subsurface");
			any_array_push(project->swatches, s);
		}
	}
	project->brush_nodes    = import_arm_get_node_canvas_array(old, "brush_nodes");
	project->brush_icons    = any_map_get(old, "brush_icons");
	project->material_nodes = import_arm_get_node_canvas_array(old, "material_nodes");
	if (any_map_get(old, "material_groups") != NULL) {
		project->material_groups = import_arm_get_node_canvas_array(old, "material_groups");
	}
	project->material_icons = any_map_get(old, "material_icons");
	project->font_assets    = any_map_get(old, "font_assets");

	any_array_t *lds = any_map_get(old, "layer_datas");
	if (lds != NULL) {
		project->layer_datas = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < lds->length; ++i) {
			any_map_t    *old      = lds->buffer[i];
			layer_data_t *ld       = GC_ALLOC_INIT(layer_data_t, {0});
			ld->name               = string_copy(any_map_get(old, "name"));
			ld->res                = armpack_map_get_i32(old, "res");
			ld->bpp                = armpack_map_get_i32(old, "bpp");
			ld->texpaint           = any_map_get(old, "texpaint");
			ld->uv_scale           = armpack_map_get_f32(old, "uv_scale");
			ld->uv_rot             = armpack_map_get_f32(old, "uv_rot");
			ld->uv_type            = armpack_map_get_i32(old, "uv_type");
			ld->decal_mat          = any_map_get(old, "decal_mat");
			ld->opacity_mask       = armpack_map_get_f32(old, "opacity_mask");
			ld->fill_layer         = armpack_map_get_i32(old, "fill_layer");
			ld->object_mask        = armpack_map_get_i32(old, "object_mask");
			ld->blending           = armpack_map_get_i32(old, "blending");
			ld->parent             = armpack_map_get_i32(old, "parent");
			ld->visible            = armpack_map_get_i32(old, "visible") > 0;
			ld->texpaint_nor       = any_map_get(old, "texpaint_nor");
			ld->texpaint_pack      = any_map_get(old, "texpaint_pack");
			ld->paint_base         = armpack_map_get_i32(old, "paint_base") > 0;
			ld->paint_opac         = armpack_map_get_i32(old, "paint_opac") > 0;
			ld->paint_occ          = armpack_map_get_i32(old, "paint_occ") > 0;
			ld->paint_rough        = armpack_map_get_i32(old, "paint_rough") > 0;
			ld->paint_met          = armpack_map_get_i32(old, "paint_met") > 0;
			ld->paint_nor          = armpack_map_get_i32(old, "paint_nor") > 0;
			ld->paint_nor_blend    = armpack_map_get_i32(old, "paint_nor_blend") > 0;
			ld->paint_height       = armpack_map_get_i32(old, "paint_height") > 0;
			ld->paint_height_blend = armpack_map_get_i32(old, "paint_height_blend") > 0;
			ld->paint_emis         = armpack_map_get_i32(old, "paint_emis") > 0;
			ld->paint_subs         = armpack_map_get_i32(old, "paint_subs") > 0;
			ld->uv_map             = armpack_map_get_i32(old, "uv_map");
			any_array_push(project->layer_datas, ld);
		}
	}

	any_array_t *ms = any_map_get(old, "mesh_datas");
	if (ms != NULL) {
		project->mesh_datas = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < ms->length; ++i) {
			any_map_t   *old  = ms->buffer[i];
			mesh_data_t *md   = GC_ALLOC_INIT(mesh_data_t, {0});
			md->name          = string_copy(any_map_get(old, "name"));
			md->scale_pos     = armpack_map_get_f32(old, "scale_pos");
			md->scale_tex     = armpack_map_get_f32(old, "scale_tex");
			any_array_t *vas  = any_map_get(old, "vertex_arrays");
			md->vertex_arrays = any_array_create_from_raw((void *[]){}, 0);
			for (i32 i = 0; i < vas->length; ++i) {
				any_map_t      *old = vas->buffer[i];
				vertex_array_t *va  = GC_ALLOC_INIT(vertex_array_t, {0});
				va->attrib          = string_copy(any_map_get(old, "attrib"));
				va->data            = string_copy(any_map_get(old, "data"));
				va->values          = any_map_get(old, "values");
				any_array_push(md->vertex_arrays, va);
			}
			md->index_array = any_map_get(old, "index_array");
			any_array_push(project->mesh_datas, md);
		}
	}

	project->mesh_assets   = any_map_get(old, "mesh_assets");
	project->mesh_icons    = any_map_get(old, "mesh_icons");
	project->atlas_objects = any_map_get(old, "atlas_objects");
	project->atlas_names   = any_map_get(old, "atlas_names");

	return project;
}

project_t *import_arm_from_version_4(any_map_t *old) {
	f32_array_t *camera_world = any_map_get(old, "camera_world");
	if (camera_world != NULL) {
		mat4_t m     = mat4_from_f32_array(camera_world, 0);
		m            = mat4_transpose(m);
		camera_world = mat4_to_f32_array(m);
		any_map_set(old, "camera_world", camera_world);
	}
	return import_arm_from_map_to_arm(old);
}

project_t *import_arm_from_version_3(any_map_t *old) {
	any_map_set(old, "envmap_angle", 0);
	any_map_set(old, "envmap_blur", 0);
	return import_arm_from_version_4(old);
}

project_t *import_arm_from_version_2(any_map_t *old) {
	any_array_t *lds = any_map_get(old, "layer_datas");
	if (lds != NULL) {
		for (i32 i = 0; i < lds->length; ++i) {
			any_map_t *ld = lds->buffer[i];
			any_map_set(ld, "uv_map", 0);
		}
	}
	return import_arm_from_version_3(old);
}

project_t *import_arm_from_old(buffer_t *b) {
	any_map_t *old = armpack_decode_to_map(b);
	if (import_arm_is_version_4(b)) {
		return import_arm_from_version_4(old);
	}
	if (import_arm_is_version_3(b)) {
		return import_arm_from_version_3(old);
	}
	if (import_arm_is_version_2(b)) {
		return import_arm_from_version_2(old);
	}
	return NULL;
}
