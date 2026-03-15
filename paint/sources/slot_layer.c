
#include "global.h"

slot_layer_t *slot_layer_create(char *ext, layer_slot_type_t type, slot_layer_t *parent) {
	slot_layer_t *raw       = GC_ALLOC_INIT(slot_layer_t, {0});
	raw->id                 = 0;
	raw->ext                = "";
	raw->visible            = true;
	raw->mask_opacity       = 1.0; // Opacity mask
	raw->show_panel         = true;
	raw->blending           = BLEND_TYPE_MIX;
	raw->object_mask        = 0;
	raw->scale              = 1.0;
	raw->angle              = 0.0;
	raw->uv_type            = UV_TYPE_UVMAP;
	raw->paint_base         = true;
	raw->paint_opac         = true;
	raw->paint_occ          = true;
	raw->paint_rough        = true;
	raw->paint_met          = true;
	raw->paint_nor          = true;
	raw->paint_nor_blend    = true;
	raw->paint_height       = true;
	raw->paint_height_blend = true;
	raw->paint_emis         = true;
	raw->paint_subs         = true;
	raw->decal_mat          = mat4_identity(); // Decal layer

	if (string_equals(ext, "")) {
		raw->id = 0;
		for (i32 i = 0; i < project_layers->length; ++i) {
			slot_layer_t *l = project_layers->buffer[i];
			if (l->id >= raw->id) {
				raw->id = l->id + 1;
			}
		}
		ext = string("%d", raw->id);
	}
	raw->ext    = string_copy(ext);
	raw->parent = parent;

	if (type == LAYER_SLOT_TYPE_GROUP) {
		i32 id    = (raw->id + 1);
		raw->name = string("Group %d", id);
	}
	else if (type == LAYER_SLOT_TYPE_LAYER) {
		i32 id       = (raw->id + 1);
		raw->name    = string("Layer %d", id);
		char *format = base_bits_handle->i == TEXTURE_BITS_BITS8 ? "RGBA32" : base_bits_handle->i == TEXTURE_BITS_BITS16 ? "RGBA64" : "RGBA128";

		{
			render_target_t *t = render_target_create();
			t->name            = string("texpaint%s", ext);
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = string_copy(format);
			raw->texpaint      = render_path_create_render_target(t)->_image;
		}

		{
			render_target_t *t = render_target_create();
			t->name            = string("texpaint_nor%s", ext);
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = string_copy(format);
			raw->texpaint_nor  = render_path_create_render_target(t)->_image;
		}
		{
			render_target_t *t = render_target_create();
			t->name            = string("texpaint_pack%s", ext);
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = string_copy(format);
			raw->texpaint_pack = render_path_create_render_target(t)->_image;
		}

		raw->texpaint_preview = gpu_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, GPU_TEXTURE_FORMAT_RGBA32);
	}

	else { // Mask
		i32 id        = (raw->id + 1);
		raw->name     = string("Mask %d", id);
		char *format  = "RGBA32"; // Full bits for undo support, R8 is used
		raw->blending = BLEND_TYPE_ADD;

		{
			render_target_t *t = render_target_create();
			t->name            = string("texpaint%s", ext);
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = string_copy(format);
			raw->texpaint      = render_path_create_render_target(t)->_image;
		}

		raw->texpaint_preview = gpu_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, GPU_TEXTURE_FORMAT_RGBA32);
	}

	return raw;
}

void slot_layer_delete(slot_layer_t *raw) {
	slot_layer_unload(raw);

	if (slot_layer_is_layer(raw)) {
		slot_layer_t_array_t *masks = slot_layer_get_masks(raw, false); // Prevents deleting group masks
		if (masks != NULL) {
			for (i32 i = 0; i < masks->length; ++i) {
				slot_layer_t *m = masks->buffer[i];
				slot_layer_delete(m);
			}
		}
	}
	else if (slot_layer_is_group(raw)) {
		slot_layer_t_array_t *children = slot_layer_get_children(raw);
		if (children != NULL) {
			for (i32 i = 0; i < children->length; ++i) {
				slot_layer_t *c = children->buffer[i];
				slot_layer_delete(c);
			}
		}
		slot_layer_t_array_t *masks = slot_layer_get_masks(raw, true);
		if (masks != NULL) {
			for (i32 i = 0; i < masks->length; ++i) {
				slot_layer_t *m = masks->buffer[i];
				slot_layer_delete(m);
			}
		}
	}

	i32 lpos = array_index_of(project_layers, raw);
	array_remove(project_layers, raw);
	// Undo can remove base layer and then restore it from undo layers
	if (project_layers->length > 0) {
		context_set_layer(project_layers->buffer[lpos > 0 ? lpos - 1 : 0]);
	}

	// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
}

void slot_layer_unload(slot_layer_t *raw) {
	if (slot_layer_is_group(raw)) {
		return;
	}

	gpu_texture_t *_texpaint         = raw->texpaint;
	gpu_texture_t *_texpaint_nor     = raw->texpaint_nor;
	gpu_texture_t *_texpaint_pack    = raw->texpaint_pack;
	gpu_texture_t *_texpaint_preview = raw->texpaint_preview;

	gpu_delete_texture(_texpaint);
	if (_texpaint_nor != NULL) {
		gpu_delete_texture(_texpaint_nor);
	}
	if (_texpaint_pack != NULL) {
		gpu_delete_texture(_texpaint_pack);
	}
	if (_texpaint_preview != NULL) {
		gpu_delete_texture(_texpaint_preview);
	}

	map_delete(render_path_render_targets, string("texpaint%s", raw->ext));
	if (slot_layer_is_layer(raw)) {
		map_delete(render_path_render_targets, string("texpaint_nor%s", raw->ext));
		map_delete(render_path_render_targets, string("texpaint_pack%s", raw->ext));
	}
}

void slot_layer_swap(slot_layer_t *raw, slot_layer_t *other) {
	if ((slot_layer_is_layer(raw) || slot_layer_is_mask(raw)) && (slot_layer_is_layer(other) || slot_layer_is_mask(other))) {
		render_target_t *rt0     = any_map_get(render_path_render_targets, string("texpaint%s", raw->ext));
		render_target_t *rt1     = any_map_get(render_path_render_targets, string("texpaint%s", other->ext));
		rt0->_image              = other->texpaint;
		rt1->_image              = raw->texpaint;
		gpu_texture_t *_texpaint = raw->texpaint;
		raw->texpaint            = other->texpaint;
		other->texpaint          = _texpaint;

		gpu_texture_t *_texpaint_preview = raw->texpaint_preview;
		raw->texpaint_preview            = other->texpaint_preview;
		other->texpaint_preview          = _texpaint_preview;
	}

	if (slot_layer_is_layer(raw) && slot_layer_is_layer(other)) {
		render_target_t *nor0         = any_map_get(render_path_render_targets, string("texpaint_nor%s", raw->ext));
		nor0->_image                  = other->texpaint_nor;
		render_target_t *pack0        = any_map_get(render_path_render_targets, string("texpaint_pack%s", raw->ext));
		pack0->_image                 = other->texpaint_pack;
		render_target_t *nor1         = any_map_get(render_path_render_targets, string("texpaint_nor%s", other->ext));
		nor1->_image                  = raw->texpaint_nor;
		render_target_t *pack1        = any_map_get(render_path_render_targets, string("texpaint_pack%s", other->ext));
		pack1->_image                 = raw->texpaint_pack;
		gpu_texture_t *_texpaint_nor  = raw->texpaint_nor;
		gpu_texture_t *_texpaint_pack = raw->texpaint_pack;
		raw->texpaint_nor             = other->texpaint_nor;
		raw->texpaint_pack            = other->texpaint_pack;
		other->texpaint_nor           = _texpaint_nor;
		other->texpaint_pack          = _texpaint_pack;
	}
}

void slot_layer_clear(slot_layer_t *raw, i32 base_color, gpu_texture_t *base_image, f32 occlusion, f32 roughness, f32 metallic) {
	// Base
	_gpu_begin(raw->texpaint, NULL, NULL, GPU_CLEAR_COLOR, base_color, 0.0);
	gpu_end();
	if (base_image != NULL) {
		draw_begin(raw->texpaint, false, 0);
		draw_scaled_image(base_image, 0, 0, raw->texpaint->width, raw->texpaint->height);
		draw_end();
	}

	if (slot_layer_is_layer(raw)) {
		// Nor
		_gpu_begin(raw->texpaint_nor, NULL, NULL, GPU_CLEAR_COLOR, color_from_floats(0.5, 0.5, 1.0, 0.0), 0.0);
		gpu_end();
		// Occ, rough, met
		_gpu_begin(raw->texpaint_pack, NULL, NULL, GPU_CLEAR_COLOR, color_from_floats(occlusion, roughness, metallic, 0.0), 0.0);
		gpu_end();
	}

	context_raw->layer_preview_dirty = true;
	context_raw->ddirty              = 3;
}

void slot_layer_invert_mask(slot_layer_t *raw) {
	gpu_texture_t *inverted = gpu_create_render_target(raw->texpaint->width, raw->texpaint->height, GPU_TEXTURE_FORMAT_RGBA32);
	draw_begin(inverted, false, 0);
	draw_set_pipeline(pipes_invert8);
	draw_image(raw->texpaint, 0, 0);
	draw_set_pipeline(NULL);
	draw_end();
	gpu_texture_t *_texpaint = raw->texpaint;
	gpu_delete_texture(_texpaint);
	render_target_t *rt = any_map_get(render_path_render_targets, string("texpaint%d", raw->id));
	raw->texpaint = rt->_image       = inverted;
	context_raw->layer_preview_dirty = true;
	context_raw->ddirty              = 3;
}

void slot_layer_apply_mask(slot_layer_t *raw) {
	if (raw->parent->fill_layer != NULL) {
		slot_layer_to_paint_layer(raw->parent);
	}
	if (slot_layer_is_group(raw->parent)) {
		for (i32 i = 0; i < slot_layer_get_children(raw->parent)->length; ++i) {
			slot_layer_t *c = slot_layer_get_children(raw->parent)->buffer[i];
			layers_apply_mask(c, raw);
		}
	}
	else {
		layers_apply_mask(raw->parent, raw);
	}
	slot_layer_delete(raw);
}

slot_layer_t *slot_layer_duplicate(slot_layer_t *raw) {
	slot_layer_t_array_t *layers = project_layers;
	i32                   i      = array_index_of(layers, raw) + 1;
	slot_layer_t         *l      = slot_layer_create("",
                                        slot_layer_is_layer(raw)  ? LAYER_SLOT_TYPE_LAYER
	                                                 : slot_layer_is_mask(raw) ? LAYER_SLOT_TYPE_MASK
	                                                                           : LAYER_SLOT_TYPE_GROUP,
	                                                 raw->parent);
	array_insert(layers, i, l);

	if (slot_layer_is_layer(raw)) {
		draw_begin(l->texpaint, false, 0);
		draw_set_pipeline(pipes_copy);
		draw_image(raw->texpaint, 0, 0);
		draw_set_pipeline(NULL);
		draw_end();

		if (l->texpaint_nor != NULL) {
			draw_begin(l->texpaint_nor, false, 0);
			draw_set_pipeline(pipes_copy);
			draw_image(raw->texpaint_nor, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();
		}

		if (l->texpaint_pack != NULL) {
			draw_begin(l->texpaint_pack, false, 0);
			draw_set_pipeline(pipes_copy);
			draw_image(raw->texpaint_pack, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();
		}
	}
	else if (slot_layer_is_mask(raw)) {
		draw_begin(l->texpaint, false, 0);
		draw_set_pipeline(pipes_copy8);
		draw_image(raw->texpaint, 0, 0);
		draw_set_pipeline(NULL);
		draw_end();
	}

	if (l->texpaint_preview != NULL) {
		draw_begin(l->texpaint_preview, true, 0x00000000);
		draw_set_pipeline(pipes_copy);
		draw_scaled_image(raw->texpaint_preview, 0, 0, raw->texpaint_preview->width, raw->texpaint_preview->height);
		draw_set_pipeline(NULL);
		draw_end();
	}

	l->visible            = raw->visible;
	l->mask_opacity       = raw->mask_opacity;
	l->fill_layer         = raw->fill_layer;
	l->object_mask        = raw->object_mask;
	l->blending           = raw->blending;
	l->uv_type            = raw->uv_type;
	l->scale              = raw->scale;
	l->angle              = raw->angle;
	l->paint_base         = raw->paint_base;
	l->paint_opac         = raw->paint_opac;
	l->paint_occ          = raw->paint_occ;
	l->paint_rough        = raw->paint_rough;
	l->paint_met          = raw->paint_met;
	l->paint_nor          = raw->paint_nor;
	l->paint_nor_blend    = raw->paint_nor_blend;
	l->paint_height       = raw->paint_height;
	l->paint_height_blend = raw->paint_height_blend;
	l->paint_emis         = raw->paint_emis;
	l->paint_subs         = raw->paint_subs;

	return l;
}

void slot_layer_resize_and_set_bits(slot_layer_t *raw) {
	i32        res_x = config_get_texture_res_x();
	i32        res_y = config_get_texture_res_y();
	any_map_t *rts   = render_path_render_targets;

	if (slot_layer_is_layer(raw)) {
		gpu_texture_format_t format = base_bits_handle->i == TEXTURE_BITS_BITS8    ? GPU_TEXTURE_FORMAT_RGBA32
		                              : base_bits_handle->i == TEXTURE_BITS_BITS16 ? GPU_TEXTURE_FORMAT_RGBA64
		                                                                           : GPU_TEXTURE_FORMAT_RGBA128;

		gpu_pipeline_t *pipe = format == GPU_TEXTURE_FORMAT_RGBA32 ? pipes_copy : format == GPU_TEXTURE_FORMAT_RGBA64 ? pipes_copy64 : pipes_copy128;

		gpu_texture_t *_texpaint = raw->texpaint;
		raw->texpaint            = gpu_create_render_target(res_x, res_y, format);
		draw_begin(raw->texpaint, false, 0);
		draw_set_pipeline(pipe);
		draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
		draw_set_pipeline(NULL);
		draw_end();

		gpu_texture_t *_texpaint_nor = raw->texpaint_nor;
		if (raw->texpaint_nor != NULL) {
			raw->texpaint_nor = gpu_create_render_target(res_x, res_y, format);
			draw_begin(raw->texpaint_nor, false, 0);
			draw_set_pipeline(pipe);
			draw_scaled_image(_texpaint_nor, 0, 0, res_x, res_y);
			draw_set_pipeline(NULL);
			draw_end();
		}

		gpu_texture_t *_texpaint_pack = raw->texpaint_pack;
		if (raw->texpaint_pack != NULL) {
			raw->texpaint_pack = gpu_create_render_target(res_x, res_y, format);
			draw_begin(raw->texpaint_pack, false, 0);
			draw_set_pipeline(pipe);
			draw_scaled_image(_texpaint_pack, 0, 0, res_x, res_y);
			draw_set_pipeline(NULL);
			draw_end();
		}

		gpu_delete_texture(_texpaint);
		if (_texpaint_nor != NULL) {
			gpu_delete_texture(_texpaint_nor);
		}
		if (_texpaint_pack != NULL) {
			gpu_delete_texture(_texpaint_pack);
		}

		render_target_t *rt = any_map_get(rts, string("texpaint%s", raw->ext));
		rt->_image          = raw->texpaint;

		if (raw->texpaint_nor != NULL) {
			render_target_t *rt_nor = any_map_get(rts, string("texpaint_nor%s", raw->ext));
			rt_nor->_image          = raw->texpaint_nor;
		}

		if (raw->texpaint_pack != NULL) {
			render_target_t *rt_pack = any_map_get(rts, string("texpaint_pack%s", raw->ext));
			rt_pack->_image          = raw->texpaint_pack;
		}
	}
	else if (slot_layer_is_mask(raw)) {
		gpu_texture_t *_texpaint = raw->texpaint;
		raw->texpaint            = gpu_create_render_target(res_x, res_y, GPU_TEXTURE_FORMAT_RGBA32);

		draw_begin(raw->texpaint, false, 0);
		draw_set_pipeline(pipes_copy8);
		draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
		draw_set_pipeline(NULL);
		draw_end();

		gpu_delete_texture(_texpaint);

		render_target_t *rt = any_map_get(rts, string("texpaint%s", raw->ext));
		rt->_image          = raw->texpaint;
	}
}

void slot_layer_to_fill_layer_on_next_frame(void *_) {
	make_material_parse_paint_material(true);
	context_raw->layer_preview_dirty                  = true;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
}

void slot_layer_to_fill_layer(slot_layer_t *raw) {
	context_set_layer(raw);
	raw->fill_layer = context_raw->material;
	layers_update_fill_layer(true);
	sys_notify_on_next_frame(&slot_layer_to_fill_layer_on_next_frame, NULL);
}

void slot_layer_to_paint_layer(slot_layer_t *raw) {
	context_set_layer(raw);
	raw->fill_layer = NULL;
	make_material_parse_paint_material(true);
	context_raw->layer_preview_dirty                  = true;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
}

bool slot_layer_is_visible(slot_layer_t *raw) {
	return raw->visible && (raw->parent == NULL || raw->parent->visible);
}

slot_layer_t_array_t *slot_layer_get_children(slot_layer_t *raw) {
	slot_layer_t_array_t *children = NULL; // Child layers of a group
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->parent == raw && slot_layer_is_layer(l)) {
			if (children == NULL) {
				children = any_array_create_from_raw((void *[]){}, 0);
			}
			any_array_push(children, l);
		}
	}
	return children;
}

slot_layer_t_array_t *slot_layer_get_recursive_children(slot_layer_t *raw) {
	slot_layer_t_array_t *children = NULL;
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->parent == raw) { // Child layers and group masks
			if (children == NULL) {
				children = any_array_create_from_raw((void *[]){}, 0);
			}
			any_array_push(children, l);
		}
		if (l->parent != NULL && l->parent->parent == raw) { // Layer masks
			if (children == NULL) {
				children = any_array_create_from_raw((void *[]){}, 0);
			}
			any_array_push(children, l);
		}
	}
	return children;
}

slot_layer_t_array_t *slot_layer_get_masks(slot_layer_t *raw, bool include_group_masks) {
	if (slot_layer_is_mask(raw)) {
		return NULL;
	}

	slot_layer_t_array_t *children = NULL;
	// Child masks of a layer
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->parent == raw && slot_layer_is_mask(l)) {
			if (children == NULL) {
				children = any_array_create_from_raw((void *[]){}, 0);
			}
			any_array_push(children, l);
		}
	}
	// Child masks of a parent group
	if (include_group_masks) {
		if (raw->parent != NULL && slot_layer_is_group(raw->parent)) {
			for (i32 i = 0; i < project_layers->length; ++i) {
				slot_layer_t *l = project_layers->buffer[i];
				if (l->parent == raw->parent && slot_layer_is_mask(l)) {
					if (children == NULL) {
						children = any_array_create_from_raw((void *[]){}, 0);
					}
					any_array_push(children, l);
				}
			}
		}
	}
	return children;
}

bool slot_layer_has_masks(slot_layer_t *raw, bool include_group_masks) {
	// Layer mask
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->parent == raw && slot_layer_is_mask(l)) {
			return true;
		}
	}

	// Group mask
	if (include_group_masks && raw->parent != NULL && slot_layer_is_group(raw->parent)) {
		for (i32 i = 0; i < project_layers->length; ++i) {
			slot_layer_t *l = project_layers->buffer[i];
			if (l->parent == raw->parent && slot_layer_is_mask(l)) {
				return true;
			}
		}
	}
	return false;
}

f32 slot_layer_get_opacity(slot_layer_t *raw) {
	f32 f = raw->mask_opacity;
	if (slot_layer_is_layer(raw) && raw->parent != NULL) {
		f *= raw->parent->mask_opacity;
	}
	return f;
}

i32 slot_layer_get_object_mask(slot_layer_t *raw) {
	return slot_layer_is_mask(raw) ? raw->parent->object_mask : raw->object_mask;
}

bool slot_layer_is_layer(slot_layer_t *raw) {
	return raw->texpaint != NULL && raw->texpaint_nor != NULL;
}

bool slot_layer_is_group(slot_layer_t *raw) {
	return raw->texpaint == NULL;
}

slot_layer_t *slot_layer_get_containing_group(slot_layer_t *raw) {
	if (raw->parent != NULL && slot_layer_is_group(raw->parent)) {
		return raw->parent;
	}
	else if (raw->parent != NULL && raw->parent->parent != NULL && slot_layer_is_group(raw->parent->parent)) {
		return raw->parent->parent;
	}
	else {
		return NULL;
	}
}

bool slot_layer_is_mask(slot_layer_t *raw) {
	return raw->texpaint != NULL && raw->texpaint_nor == NULL;
}

bool slot_layer_is_group_mask(slot_layer_t *raw) {
	return raw->texpaint != NULL && raw->texpaint_nor == NULL && slot_layer_is_group(raw->parent);
}

bool slot_layer_is_layer_mask(slot_layer_t *raw) {
	return raw->texpaint != NULL && raw->texpaint_nor == NULL && slot_layer_is_layer(raw->parent);
}

bool slot_layer_is_in_group(slot_layer_t *raw) {
	return raw->parent != NULL && (slot_layer_is_group(raw->parent) || (raw->parent->parent != NULL && slot_layer_is_group(raw->parent->parent)));
}

bool slot_layer_can_move(slot_layer_t *raw, i32 to) {
	i32 old_index = array_index_of(project_layers, raw);

	i32 delta = to - old_index; // If delta > 0 the layer is moved up, otherwise down
	if (to < 0 || to > project_layers->length - 1 || delta == 0) {
		return false;
	}

	// If the layer is moved up, all layers between the old position and the new one move one down
	// The layers above the new position stay where they are
	// If the new position is on top or on bottom no upper resp. lower layer exists
	slot_layer_t *new_upper_layer = delta > 0 ? (to < project_layers->length - 1 ? project_layers->buffer[to + 1] : NULL) : project_layers->buffer[to];

	// Group or layer is collapsed so we check below and update the upper layer
	if (new_upper_layer != NULL && !new_upper_layer->show_panel) {
		slot_layer_t_array_t *children = slot_layer_get_recursive_children(new_upper_layer);
		to -= children != NULL ? children->length : 0;
		delta           = to - old_index;
		new_upper_layer = delta > 0 ? (to < project_layers->length - 1 ? project_layers->buffer[to + 1] : NULL) : project_layers->buffer[to];
	}

	slot_layer_t *new_lower_layer = delta > 0 ? project_layers->buffer[to] : (to > 0 ? project_layers->buffer[to - 1] : NULL);

	if (slot_layer_is_mask(raw)) {
		// Masks can not be on top
		if (new_upper_layer == NULL) {
			return false;
		}
		// Masks should not be placed below a collapsed group - this condition can be savely removed
		if (slot_layer_is_in_group(new_upper_layer) && !slot_layer_get_containing_group(new_upper_layer)->show_panel) {
			return false;
		}
		// Masks should not be placed below a collapsed layer - this condition can be savely removed
		if (slot_layer_is_mask(new_upper_layer) && !new_upper_layer->parent->show_panel) {
			return false;
		}
	}

	if (slot_layer_is_layer(raw)) {
		// Layers can not be moved directly below its own mask(s)
		if (new_upper_layer != NULL && slot_layer_is_mask(new_upper_layer) && new_upper_layer->parent == raw) {
			return false;
		}
		// Layers can not be placed above a mask as the mask would be reparented
		if (new_lower_layer != NULL && slot_layer_is_mask(new_lower_layer)) {
			return false;
		}
	}

	// Currently groups can not be nested - thus valid positions for groups are:
	if (slot_layer_is_group(raw)) {
		// At the top
		if (new_upper_layer == NULL) {
			return true;
		}
		// NOT below its own children
		if (slot_layer_get_containing_group(new_upper_layer) == raw) {
			return false;
		}
		// At the bottom
		if (new_lower_layer == NULL) {
			return true;
		}
		// Above a group
		if (slot_layer_is_group(new_lower_layer)) {
			return true;
		}
		// Above a non-grouped layer
		if (slot_layer_is_layer(new_lower_layer) && !slot_layer_is_in_group(new_lower_layer)) {
			return true;
		}
		else {
			return false;
		}
	}

	return true;
}

void slot_layer_move(slot_layer_t *raw, i32 to) {
	if (!slot_layer_can_move(raw, to)) {
		return;
	}

	i32_map_t    *pointers        = tab_layers_init_layer_map();
	i32           old_index       = array_index_of(project_layers, raw);
	i32           delta           = to - old_index;
	slot_layer_t *new_upper_layer = delta > 0 ? (to < project_layers->length - 1 ? project_layers->buffer[to + 1] : NULL) : project_layers->buffer[to];

	// Group or layer is collapsed so we check below and update the upper layer
	if (new_upper_layer != NULL && !new_upper_layer->show_panel) {
		slot_layer_t_array_t *children = slot_layer_get_recursive_children(new_upper_layer);
		to -= children != NULL ? children->length : 0;
		delta           = to - old_index;
		new_upper_layer = delta > 0 ? (to < project_layers->length - 1 ? project_layers->buffer[to + 1] : NULL) : project_layers->buffer[to];
	}

	context_set_layer(raw);
	history_order_layers(to);
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;

	array_remove(project_layers, raw);
	array_insert(project_layers, to, raw);

	if (slot_layer_is_layer(raw)) {
		slot_layer_t *old_parent = raw->parent;

		if (new_upper_layer == NULL) {
			raw->parent = NULL; // Placed on top
		}
		else if (slot_layer_is_in_group(new_upper_layer) && !slot_layer_get_containing_group(new_upper_layer)->show_panel) {
			raw->parent = NULL; // Placed below a collapsed group
		}
		else if (slot_layer_is_layer(new_upper_layer)) {
			raw->parent = new_upper_layer->parent; // Placed below a layer, use the same parent
		}
		else if (slot_layer_is_group(new_upper_layer)) {
			raw->parent = new_upper_layer; // Placed as top layer in a group
		}
		else if (slot_layer_is_group_mask(new_upper_layer)) {
			raw->parent = new_upper_layer->parent; // Placed in a group below the lowest group mask
		}
		else if (slot_layer_is_layer_mask(new_upper_layer)) {
			raw->parent = slot_layer_get_containing_group(new_upper_layer); // Either the group the mask belongs to or NULL
		}

		// Layers can have masks as children
		// These have to be moved, too
		slot_layer_t_array_t *layer_masks = slot_layer_get_masks(raw, false);
		if (layer_masks != NULL) {
			for (i32 idx = 0; idx < layer_masks->length; ++idx) {
				slot_layer_t *mask = layer_masks->buffer[idx];
				array_remove(project_layers, mask);
				// If the masks are moved down each step increases the index below the layer by one.
				array_insert(project_layers, delta > 0 ? old_index + delta - 1 : old_index + delta + idx, mask);
			}
		}

		// The layer is the last layer in the group, remove it
		// Notice that this might remove group masks
		if (old_parent != NULL && slot_layer_get_children(old_parent) == NULL) {
			slot_layer_delete(old_parent);
		}
	}
	else if (slot_layer_is_mask(raw)) {
		// Precondition new_upper_layer != NULL, ensured in can_move
		if (slot_layer_is_layer(new_upper_layer) || slot_layer_is_group(new_upper_layer)) {
			raw->parent = new_upper_layer;
		}
		else if (slot_layer_is_mask(new_upper_layer)) { // Group mask or layer mask
			raw->parent = new_upper_layer->parent;
		}
	}
	else if (slot_layer_is_group(raw)) {
		slot_layer_t_array_t *children = slot_layer_get_recursive_children(raw);
		if (children != NULL) {
			for (i32 idx = 0; idx < children->length; ++idx) {
				slot_layer_t *child = children->buffer[idx];
				array_remove(project_layers, child);
				// If the children are moved down each step increases the index below the layer by one
				array_insert(project_layers, delta > 0 ? old_index + delta - 1 : old_index + delta + idx, child);
			}
		}
	}

	for (i32 i = 0; i < project_materials->length; ++i) {
		slot_material_t *m = project_materials->buffer[i];
		tab_layers_remap_layer_pointers(m->canvas->nodes, tab_layers_fill_layer_map(pointers));
	}
}

void layers_init() {
	slot_layer_clear(project_layers->buffer[0], color_from_floats(layers_default_base, layers_default_base, layers_default_base, 1.0), NULL, 1.0,
	                 layers_default_rough, 0.0);
}

void layers_resize() {
	if (base_res_handle->i >= math_floor(TEXTURE_RES_RES16384)) { // Save memory for >=16k
		config_raw->undo_steps = 1;
		while (history_undo_layers->length > config_raw->undo_steps) {
			slot_layer_t *l = array_pop(history_undo_layers);
			sys_notify_on_next_frame(&slot_layer_unload, l);
		}
	}
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		slot_layer_resize_and_set_bits(l);
	}
	for (i32 i = 0; i < history_undo_layers->length; ++i) {
		slot_layer_t *l = history_undo_layers->buffer[i];
		slot_layer_resize_and_set_bits(l);
	}

	any_map_t *rts = render_path_render_targets;

	render_target_t *blend0           = any_map_get(rts, "texpaint_blend0");
	gpu_texture_t   *_texpaint_blend0 = blend0->_image;
	gpu_delete_texture(_texpaint_blend0);
	blend0->width  = config_get_texture_res_x();
	blend0->height = config_get_texture_res_y();
	blend0->_image = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), GPU_TEXTURE_FORMAT_R8);

	render_target_t *blend1           = any_map_get(rts, "texpaint_blend1");
	gpu_texture_t   *_texpaint_blend1 = blend1->_image;
	gpu_delete_texture(_texpaint_blend1);
	blend1->width  = config_get_texture_res_x();
	blend1->height = config_get_texture_res_y();
	blend1->_image = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), GPU_TEXTURE_FORMAT_R8);

	context_raw->brush_blend_dirty = true;

	render_target_t *blur = any_map_get(rts, "texpaint_blur");
	if (blur != NULL) {
		gpu_texture_t *_texpaint_blur = blur->_image;
		gpu_delete_texture(_texpaint_blur);
		f32 size_x   = math_floor(config_get_texture_res_x() * 0.95);
		f32 size_y   = math_floor(config_get_texture_res_y() * 0.95);
		blur->width  = size_x;
		blur->height = size_y;
		blur->_image = gpu_create_render_target(size_x, size_y, GPU_TEXTURE_FORMAT_RGBA32);
	}
	if (render_path_paint_live_layer != NULL) {
		slot_layer_resize_and_set_bits(render_path_paint_live_layer);
	}
	render_path_raytrace_ready = false; // Rebuild baketex
	context_raw->ddirty        = 2;
}

void layers_set_bits() {
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		slot_layer_resize_and_set_bits(l);
	}
	for (i32 i = 0; i < history_undo_layers->length; ++i) {
		slot_layer_t *l = history_undo_layers->buffer[i];
		slot_layer_resize_and_set_bits(l);
	}
}

void layers_make_temp_img() {
	slot_layer_t *l = project_layers->buffer[0];

	if (layers_temp_image != NULL && (layers_temp_image->width != l->texpaint->width || layers_temp_image->height != l->texpaint->height ||
	                                  layers_temp_image->format != l->texpaint->format)) {
		render_target_t *_temptex0 = any_map_get(render_path_render_targets, "temptex0");
		gpu_delete_texture(_temptex0->_image);
		map_delete(render_path_render_targets, "temptex0");
		gc_unroot(layers_temp_image);
		layers_temp_image = NULL;
	}

	if (layers_temp_image == NULL) {
		char            *format = base_bits_handle->i == TEXTURE_BITS_BITS8 ? "RGBA32" : base_bits_handle->i == TEXTURE_BITS_BITS16 ? "RGBA64" : "RGBA128";
		render_target_t *t      = render_target_create();
		t->name                 = "temptex0";
		t->width                = l->texpaint->width;
		t->height               = l->texpaint->height;
		t->format               = string_copy(format);
		render_target_t *rt     = render_path_create_render_target(t);
		gc_unroot(layers_temp_image);
		layers_temp_image = rt->_image;
		gc_root(layers_temp_image);
	}
}

void layers_make_temp_mask_img() {
	if (pipes_temp_mask_image != NULL &&
	    (pipes_temp_mask_image->width != config_get_texture_res_x() || pipes_temp_mask_image->height != config_get_texture_res_y())) {
		gpu_texture_t *_temp_mask_image = pipes_temp_mask_image;
		gpu_delete_texture(_temp_mask_image);
		gc_unroot(pipes_temp_mask_image);
		pipes_temp_mask_image = NULL;
	}

	if (pipes_temp_mask_image == NULL) {
		gc_unroot(pipes_temp_mask_image);
		pipes_temp_mask_image = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), GPU_TEXTURE_FORMAT_R8);
		gc_root(pipes_temp_mask_image);
	}
}

void layers_make_export_img() {
	slot_layer_t *l = project_layers->buffer[0];
	if (layers_expa != NULL &&
	    (layers_expa->width != l->texpaint->width || layers_expa->height != l->texpaint->height || layers_expa->format != l->texpaint->format)) {
		gpu_texture_t *_expa = layers_expa;
		gpu_texture_t *_expb = layers_expb;
		gpu_texture_t *_expc = layers_expc;
		gpu_delete_texture(_expa);
		gpu_delete_texture(_expb);
		gpu_delete_texture(_expc);
		gc_unroot(layers_expa);
		layers_expa = NULL;
		gc_unroot(layers_expb);
		layers_expb = NULL;
		gc_unroot(layers_expc);
		layers_expc = NULL;
		map_delete(render_path_render_targets, "expa");
		map_delete(render_path_render_targets, "expb");
		map_delete(render_path_render_targets, "expc");
	}
	if (layers_expa == NULL) {
		char *format = base_bits_handle->i == TEXTURE_BITS_BITS8 ? "RGBA32" : base_bits_handle->i == TEXTURE_BITS_BITS16 ? "RGBA64" : "RGBA128";
		{
			render_target_t *t  = render_target_create();
			t->name             = "expa";
			t->width            = l->texpaint->width;
			t->height           = l->texpaint->height;
			t->format           = string_copy(format);
			render_target_t *rt = render_path_create_render_target(t);
			gc_unroot(layers_expa);
			layers_expa = rt->_image;
			gc_root(layers_expa);
		}
		{
			render_target_t *t  = render_target_create();
			t->name             = "expb";
			t->width            = l->texpaint->width;
			t->height           = l->texpaint->height;
			t->format           = string_copy(format);
			render_target_t *rt = render_path_create_render_target(t);
			gc_unroot(layers_expb);
			layers_expb = rt->_image;
			gc_root(layers_expb);
		}
		{
			render_target_t *t  = render_target_create();
			t->name             = "expc";
			t->width            = l->texpaint->width;
			t->height           = l->texpaint->height;
			t->format           = string_copy(format);
			render_target_t *rt = render_path_create_render_target(t);
			gc_unroot(layers_expc);
			layers_expc = rt->_image;
			gc_root(layers_expc);
		}
	}
}

void layers_apply_mask(slot_layer_t *l, slot_layer_t *m) {
	if (!slot_layer_is_layer(l) || !slot_layer_is_mask(m)) {
		return;
	}

	layers_make_temp_img();

	// Copy layer to temp
	draw_begin(layers_temp_image, false, 0);
	draw_set_pipeline(pipes_copy);
	draw_image(l->texpaint, 0, 0);
	draw_set_pipeline(NULL);
	draw_end();

	// Apply mask
	_gpu_begin(l->texpaint, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
	gpu_set_pipeline(pipes_apply_mask);
	gpu_set_texture(pipes_tex0_mask, layers_temp_image);
	gpu_set_texture(pipes_texa_mask, m->texpaint);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	gpu_end();
}

void layers_commands_merge_pack(gpu_pipeline_t *pipe, gpu_texture_t *i0, gpu_texture_t *i1, gpu_texture_t *i1pack, f32 i1mask_opacity, gpu_texture_t *i1texmask,
                                i32 i1blending) {
	_gpu_begin(i0, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
	gpu_set_pipeline(pipe);
	gpu_set_texture(pipes_tex0, i1);
	gpu_set_texture(pipes_tex1, i1pack);
	gpu_set_texture(pipes_texmask, i1texmask);
	gpu_set_texture(pipes_texa, layers_temp_image);
	gpu_set_float(pipes_opac, i1mask_opacity);
	gpu_set_float(pipes_tex1w, i1pack->width);
	gpu_set_int(pipes_blending, i1blending);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	gpu_end();
}

bool layers_is_fill_material() {
	if (context_raw->tool == TOOL_TYPE_MATERIAL) {
		return true;
	}

	slot_material_t *m = context_raw->material;
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->fill_layer == m) {
			return true;
		}
	}
	return false;
}

void layers_update_fill_layers() {
	slot_layer_t  *_layer     = context_raw->layer;
	tool_type_t    _tool      = context_raw->tool;
	i32            _fill_type = context_raw->fill_type_handle->i;
	gpu_texture_t *current    = NULL;

	if (context_raw->tool == TOOL_TYPE_MATERIAL) {
		if (render_path_paint_live_layer == NULL) {
			gc_unroot(render_path_paint_live_layer);
			render_path_paint_live_layer = slot_layer_create("_live", LAYER_SLOT_TYPE_LAYER, NULL);
			gc_root(render_path_paint_live_layer);
		}

		current     = _draw_current;
		bool in_use = gpu_in_use;
		if (in_use)
			draw_end();

		context_raw->tool                = TOOL_TYPE_FILL;
		context_raw->fill_type_handle->i = FILL_TYPE_OBJECT;
		render_path_paint_set_plane_mesh();
		make_material_parse_paint_material(false);
		context_raw->pdirty = 1;
		render_path_paint_use_live_layer(true);
		render_path_paint_commands_paint(false);
		render_path_paint_dilate(true, true);
		render_path_paint_use_live_layer(false);
		context_raw->tool                = _tool;
		context_raw->fill_type_handle->i = _fill_type;
		context_raw->pdirty              = 0;
		context_raw->rdirty              = 2;
		render_path_paint_restore_plane_mesh();
		make_material_parse_paint_material(true);
		ui_view2d_hwnd->redraws = 2;

		if (in_use)
			draw_begin(current, false, 0);
		return;
	}

	bool has_fill_layer = false;
	bool has_fill_mask  = false;
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (slot_layer_is_layer(l) && l->fill_layer == context_raw->material) {
			has_fill_layer = true;
		}
	}
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (slot_layer_is_mask(l) && l->fill_layer == context_raw->material) {
			has_fill_mask = true;
		}
	}

	if (has_fill_layer || has_fill_mask) {
		current     = _draw_current;
		bool in_use = gpu_in_use;
		if (in_use)
			draw_end();
		context_raw->pdirty              = 1;
		context_raw->tool                = TOOL_TYPE_FILL;
		context_raw->fill_type_handle->i = FILL_TYPE_OBJECT;

		if (has_fill_layer) {
			bool first = true;
			for (i32 i = 0; i < project_layers->length; ++i) {
				slot_layer_t *l = project_layers->buffer[i];
				if (slot_layer_is_layer(l) && l->fill_layer == context_raw->material) {
					context_raw->layer = l;
					if (first) {
						first = false;
						make_material_parse_paint_material(false);
					}
					layers_set_object_mask();
					slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
					render_path_paint_commands_paint(false);
					render_path_paint_dilate(true, true);
				}
			}
		}
		if (has_fill_mask) {
			bool first = true;
			for (i32 i = 0; i < project_layers->length; ++i) {
				slot_layer_t *l = project_layers->buffer[i];
				if (slot_layer_is_mask(l) && l->fill_layer == context_raw->material) {
					context_raw->layer = l;
					if (first) {
						first = false;
						make_material_parse_paint_material(false);
					}
					layers_set_object_mask();
					slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
					render_path_paint_commands_paint(false);
					render_path_paint_dilate(true, true);
				}
			}
		}
		context_raw->pdirty               = 0;
		context_raw->ddirty               = 2;
		context_raw->rdirty               = 2;
		context_raw->layers_preview_dirty = true; // Repaint all layer previews as multiple layers might have changed.
		if (in_use)
			draw_begin(current, false, 0);
		context_raw->layer = _layer;
		layers_set_object_mask();
		context_raw->tool                = _tool;
		context_raw->fill_type_handle->i = _fill_type;
		make_material_parse_paint_material(false);
	}
}

void layers_update_fill_layer(bool parse_paint) {
	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use)
		draw_end();

	tool_type_t _tool                = context_raw->tool;
	i32         _fill_type           = context_raw->fill_type_handle->i;
	context_raw->tool                = TOOL_TYPE_FILL;
	context_raw->fill_type_handle->i = FILL_TYPE_OBJECT;
	context_raw->pdirty              = 1;

	slot_layer_clear(context_raw->layer, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);

	if (parse_paint) {
		make_material_parse_paint_material(false);
	}
	render_path_paint_commands_paint(false);
	render_path_paint_dilate(true, true);

	context_raw->rdirty              = 2;
	context_raw->tool                = _tool;
	context_raw->fill_type_handle->i = _fill_type;
	if (in_use)
		draw_begin(current, false, 0);
}

void layers_set_object_mask() {
	string_t_array_t *ar = any_array_create_from_raw(
	    (void *[]){
	        tr("None"),
	    },
	    1);
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *p = project_paint_objects->buffer[i];
		any_array_push(ar, p->base->name);
	}

	i32 mask = context_object_mask_used() ? slot_layer_get_object_mask(context_raw->layer) : 0;
	if (context_layer_filter_used()) {
		mask = context_raw->layer_filter;
	}
	if (mask > 0) {
		if (context_raw->merged_object != NULL) {
			context_raw->merged_object->base->visible = false;
		}
		mesh_object_t *o = project_paint_objects->buffer[0];
		for (i32 i = 0; i < project_paint_objects->length; ++i) {
			mesh_object_t *p         = project_paint_objects->buffer[i];
			char          *mask_name = ar->buffer[mask];
			if (string_equals(p->base->name, mask_name)) {
				o = p;
				break;
			}
		}
		context_select_paint_object(o);
	}
	else {
		bool is_atlas = slot_layer_get_object_mask(context_raw->layer) > 0 && slot_layer_get_object_mask(context_raw->layer) <= project_paint_objects->length;
		if (context_raw->merged_object == NULL || is_atlas || context_raw->merged_object_is_atlas) {
			mesh_object_t_array_t *visibles = is_atlas ? project_get_atlas_objects(slot_layer_get_object_mask(context_raw->layer)) : NULL;
			util_mesh_merge(visibles);
		}
		context_select_paint_object(context_main_object());
		context_raw->paint_object->skip_context   = "paint";
		context_raw->merged_object->base->visible = true;
	}
	util_uv_dilatemap_cached = false;
}

void layers_new_layer_clear(slot_layer_t *l) {
	slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
}

slot_layer_t *layers_new_layer(bool clear, i32 position) {
	if (project_layers->length > layers_max_layers) {
		return NULL;
	}

	slot_layer_t *l = slot_layer_create("", LAYER_SLOT_TYPE_LAYER, NULL);
	l->object_mask  = context_raw->layer_filter;

	if (position == -1) {
		if (slot_layer_is_mask(context_raw->layer))
			context_set_layer(context_raw->layer->parent);
		array_insert(project_layers, array_index_of(project_layers, context_raw->layer) + 1, l);
	}
	else {
		array_insert(project_layers, position, l);
	}

	context_set_layer(l);
	i32 li = array_index_of(project_layers, context_raw->layer);
	if (li > 0) {
		slot_layer_t *below = project_layers->buffer[li - 1];
		if (slot_layer_is_layer(below)) {
			context_raw->layer->parent = below->parent;
		}
	}
	if (clear) {
		sys_notify_on_next_frame(&layers_new_layer_clear, l);
	}
	context_raw->layer_preview_dirty = true;
	return l;
}

void layers_new_mask_clear(slot_layer_t *l) {
	slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
}

slot_layer_t *layers_new_mask(bool clear, slot_layer_t *parent, i32 position) {
	if (project_layers->length > layers_max_layers) {
		return NULL;
	}

	slot_layer_t *l = slot_layer_create("", LAYER_SLOT_TYPE_MASK, parent);
	if (position == -1) {
		position = array_index_of(project_layers, parent);
	}
	array_insert(project_layers, position, l);
	context_set_layer(l);
	if (clear) {
		sys_notify_on_next_frame(&layers_new_mask_clear, l);
	}
	context_raw->layer_preview_dirty = true;
	return l;
}

slot_layer_t *layers_new_group() {
	if (project_layers->length > layers_max_layers) {
		return NULL;
	}

	slot_layer_t *l = slot_layer_create("", LAYER_SLOT_TYPE_GROUP, NULL);
	any_array_push(project_layers, l);
	context_set_layer(l);
	return l;
}

void layers_create_fill_layer_on_next_frame(void *_) {
	slot_layer_t *l = layers_new_layer(false, _layers_position);
	history_new_layer();
	l->uv_type = _layers_uv_type;
	if (!mat4_isnan(_layers_decal_mat)) {
		l->decal_mat = _layers_decal_mat;
	}
	l->object_mask = context_raw->layer_filter;
	history_to_fill_layer();
	slot_layer_to_fill_layer(l);
}

void layers_create_fill_layer(uv_type_t uv_type, mat4_t decal_mat, i32 position) {
	if (context_raw->tool == TOOL_TYPE_GIZMO) {
		return;
	}

	_layers_uv_type   = uv_type;
	_layers_decal_mat = decal_mat;
	_layers_position  = position;
	sys_notify_on_next_frame(&layers_create_fill_layer_on_next_frame, NULL);
}

void layers_create_image_mask(asset_t *asset) {
	slot_layer_t *l = context_raw->layer;
	if (slot_layer_is_mask(l) || slot_layer_is_group(l)) {
		return;
	}

	history_new_layer();
	slot_layer_t *m = layers_new_mask(false, l, -1);
	slot_layer_clear(m, 0x00000000, project_get_image(asset), 1.0, layers_default_rough, 0.0);
	context_raw->layer_preview_dirty = true;
}

void layers_create_color_layer_on_next_frame(void *_) {
	slot_layer_t *l = layers_new_layer(false, _layers_position);
	history_new_layer();
	l->uv_type     = UV_TYPE_UVMAP;
	l->object_mask = context_raw->layer_filter;
	slot_layer_clear(l, _layers_base_color, NULL, _layers_occlusion, _layers_roughness, _layers_metallic);
}

void layers_create_color_layer(i32 base_color, f32 occlusion, f32 roughness, f32 metallic, i32 position) {
	_layers_base_color = base_color;
	_layers_occlusion  = occlusion;
	_layers_roughness  = roughness;
	_layers_metallic   = metallic;
	_layers_position   = position;

	sys_notify_on_next_frame(&layers_create_color_layer_on_next_frame, NULL);
}

void layers_duplicate_layer(slot_layer_t *l) {
	if (!slot_layer_is_group(l)) {
		slot_layer_t *new_layer = slot_layer_duplicate(l);
		context_set_layer(new_layer);
		slot_layer_t_array_t *masks = slot_layer_get_masks(l, false);
		if (masks != NULL) {
			for (i32 i = 0; i < masks->length; ++i) {
				slot_layer_t *m = masks->buffer[i];
				m               = slot_layer_duplicate(m);
				m->parent       = new_layer;
				array_remove(project_layers, m);
				array_insert(project_layers, array_index_of(project_layers, new_layer), m);
			}
		}
		context_set_layer(new_layer);
	}
	else {
		slot_layer_t *new_group = layers_new_group();
		array_remove(project_layers, new_group);
		array_insert(project_layers, array_index_of(project_layers, l) + 1, new_group);
		// group.show_panel = true;
		for (i32 i = 0; i < slot_layer_get_children(l)->length; ++i) {
			slot_layer_t         *c         = slot_layer_get_children(l)->buffer[i];
			slot_layer_t_array_t *masks     = slot_layer_get_masks(c, false);
			slot_layer_t         *new_layer = slot_layer_duplicate(c);
			new_layer->parent               = new_group;
			array_remove(project_layers, new_layer);
			array_insert(project_layers, array_index_of(project_layers, new_group), new_layer);
			if (masks != NULL) {
				for (i32 i = 0; i < masks->length; ++i) {
					slot_layer_t *m        = masks->buffer[i];
					slot_layer_t *new_mask = slot_layer_duplicate(m);
					new_mask->parent       = new_layer;
					array_remove(project_layers, new_mask);
					array_insert(project_layers, array_index_of(project_layers, new_layer), new_mask);
				}
			}
		}
		slot_layer_t_array_t *group_masks = slot_layer_get_masks(l, true);
		if (group_masks != NULL) {
			for (i32 i = 0; i < group_masks->length; ++i) {
				slot_layer_t *m        = group_masks->buffer[i];
				slot_layer_t *new_mask = slot_layer_duplicate(m);
				new_mask->parent       = new_group;
				array_remove(project_layers, new_mask);
				array_insert(project_layers, array_index_of(project_layers, new_group), new_mask);
			}
		}
		context_set_layer(new_group);
	}
}

void layers_apply_masks(slot_layer_t *l) {
	slot_layer_t_array_t *masks = slot_layer_get_masks(l, true);

	if (masks != NULL) {
		for (i32 i = 0; i < masks->length - 1; ++i) {
			layers_merge_layer(masks->buffer[i + 1], masks->buffer[i], false);
			slot_layer_delete(masks->buffer[i]);
		}
		slot_layer_apply_mask(masks->buffer[masks->length - 1]);
		context_raw->layer_preview_dirty = true;
	}
}

void layers_merge_down() {
	slot_layer_t *l1 = context_raw->layer;

	if (slot_layer_is_group(l1)) {
		l1 = layers_merge_group(l1);
	}
	else if (slot_layer_has_masks(l1, true)) { // It is a layer
		layers_apply_masks(l1);
		context_set_layer(l1);
	}

	slot_layer_t *l0 = project_layers->buffer[array_index_of(project_layers, l1) - 1];

	if (slot_layer_is_group(l0)) {
		l0 = layers_merge_group(l0);
	}
	else if (slot_layer_has_masks(l0, true)) { // It is a layer
		layers_apply_masks(l0);
		context_set_layer(l0);
	}

	layers_merge_layer(l0, l1, false);
	slot_layer_delete(l1);
	context_set_layer(l0);
	context_raw->layer_preview_dirty = true;
}

slot_layer_t *layers_merge_group(slot_layer_t *l) {
	if (!slot_layer_is_group(l)) {
		return NULL;
	}

	slot_layer_t_array_t *children = slot_layer_get_children(l);

	if (children->length == 1 && slot_layer_has_masks(children->buffer[0], false)) {
		layers_apply_masks(children->buffer[0]);
	}

	for (i32 i = 0; i < children->length - 1; ++i) {
		context_set_layer(children->buffer[children->length - 1 - i]);
		history_merge_layers();
		layers_merge_down();
	}

	// Now apply the group masks
	slot_layer_t_array_t *masks = slot_layer_get_masks(l, true);
	if (masks != NULL) {
		for (i32 i = 0; i < masks->length - 1; ++i) {
			layers_merge_layer(masks->buffer[i + 1], masks->buffer[i], false);
			slot_layer_delete(masks->buffer[i]);
		}
		layers_apply_mask(children->buffer[0], masks->buffer[masks->length - 1]);
	}

	children->buffer[0]->parent = NULL;
	children->buffer[0]->name   = l->name;
	if (children->buffer[0]->fill_layer != NULL) {
		slot_layer_to_paint_layer(children->buffer[0]);
	}
	slot_layer_delete(l);
	return children->buffer[0];
}

void layers_merge_layer(slot_layer_t *l0, slot_layer_t *l1, bool use_mask) {
	if (!l1->visible || slot_layer_is_group(l1)) {
		return;
	}

	layers_make_temp_img();

	draw_begin(layers_temp_image, false, 0); // Copy to temp
	draw_set_pipeline(pipes_copy);
	draw_image(l0->texpaint, 0, 0);
	draw_set_pipeline(NULL);
	draw_end();

	render_target_t      *empty_rt = any_map_get(render_path_render_targets, "empty_white");
	gpu_texture_t        *empty    = empty_rt->_image;
	gpu_texture_t        *mask     = empty;
	slot_layer_t_array_t *l1masks  = use_mask ? slot_layer_get_masks(l1, true) : NULL;
	if (l1masks != NULL) {
		// for (let i: i32 = 1; i < l1masks.length - 1; ++i) {
		// 	merge_layer(l1masks[i + 1], l1masks[i]);
		// }
		mask = l1masks->buffer[0]->texpaint;
	}

	if (slot_layer_is_mask(l1)) {
		_gpu_begin(l0->texpaint, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
		gpu_set_pipeline(pipes_merge_mask);
		gpu_set_texture(pipes_tex0_merge_mask, l1->texpaint);
		gpu_set_texture(pipes_texa_merge_mask, layers_temp_image);
		gpu_set_float(pipes_opac_merge_mask, slot_layer_get_opacity(l1));
		gpu_set_int(pipes_blending_merge_mask, l1->blending);
		gpu_set_vertex_buffer(const_data_screen_aligned_vb);
		gpu_set_index_buffer(const_data_screen_aligned_ib);
		gpu_draw();
		gpu_end();
	}

	if (slot_layer_is_layer(l1)) {
		if (l1->paint_base) {
			_gpu_begin(l0->texpaint, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
			gpu_set_pipeline(pipes_merge);
			gpu_set_texture(pipes_tex0, l1->texpaint);
			gpu_set_texture(pipes_tex1, empty);
			gpu_set_texture(pipes_texmask, mask);
			gpu_set_texture(pipes_texa, layers_temp_image);
			gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
			gpu_set_float(pipes_tex1w, empty->width);
			gpu_set_int(pipes_blending, l1->blending);
			gpu_set_vertex_buffer(const_data_screen_aligned_vb);
			gpu_set_index_buffer(const_data_screen_aligned_ib);
			gpu_draw();
			gpu_end();
		}

		if (l0->texpaint_nor != NULL) {
			draw_begin(layers_temp_image, false, 0);
			draw_set_pipeline(pipes_copy);
			draw_image(l0->texpaint_nor, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();

			if (l1->paint_nor) {
				_gpu_begin(l0->texpaint_nor, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
				gpu_set_pipeline(pipes_merge);
				gpu_set_texture(pipes_tex0, l1->texpaint);
				gpu_set_texture(pipes_tex1, l1->texpaint_nor);
				gpu_set_texture(pipes_texmask, mask);
				gpu_set_texture(pipes_texa, layers_temp_image);
				gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
				gpu_set_float(pipes_tex1w, l1->texpaint_nor->width);
				gpu_set_int(pipes_blending, l1->paint_nor_blend ? 102 : 101);
				gpu_set_vertex_buffer(const_data_screen_aligned_vb);
				gpu_set_index_buffer(const_data_screen_aligned_ib);
				gpu_draw();
				gpu_end();
			}
		}

		if (l0->texpaint_pack != NULL) {
			draw_begin(layers_temp_image, false, 0);
			draw_set_pipeline(pipes_copy);
			draw_image(l0->texpaint_pack, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();

			if (l1->paint_occ || l1->paint_rough || l1->paint_met || l1->paint_height) {
				if (l1->paint_occ && l1->paint_rough && l1->paint_met && l1->paint_height) {
					layers_commands_merge_pack(pipes_merge, l0->texpaint_pack, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask,
					                           l1->paint_height_blend ? 103 : 101);
				}
				else {
					if (l1->paint_occ) {
						layers_commands_merge_pack(pipes_merge_r, l0->texpaint_pack, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask, 101);
					}
					if (l1->paint_rough) {
						layers_commands_merge_pack(pipes_merge_g, l0->texpaint_pack, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask, 101);
					}
					if (l1->paint_met) {
						layers_commands_merge_pack(pipes_merge_b, l0->texpaint_pack, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask, 101);
					}
				}
			}
		}
	}
}

slot_layer_t *layers_flatten(bool height_to_normal, slot_layer_t_array_t *layers) {
	if (layers == NULL) {
		layers = project_layers;
	}
	layers_make_temp_img();
	layers_make_export_img();
	render_target_t *empty_rt = any_map_get(render_path_render_targets, "empty_white");
	gpu_texture_t   *empty    = empty_rt->_image;

	// Clear export layer
	_gpu_begin(layers_expa, NULL, NULL, GPU_CLEAR_COLOR, color_from_floats(0.0, 0.0, 0.0, 0.0), 0.0);
	gpu_end();
	_gpu_begin(layers_expb, NULL, NULL, GPU_CLEAR_COLOR, color_from_floats(0.5, 0.5, 1.0, 0.0), 0.0);
	gpu_end();
	_gpu_begin(layers_expc, NULL, NULL, GPU_CLEAR_COLOR, color_from_floats(1.0, 0.0, 0.0, 0.0), 0.0);
	gpu_end();

	// Flatten layers
	for (i32 i = 0; i < layers->length; ++i) {
		slot_layer_t *l1 = layers->buffer[i];
		if (!slot_layer_is_visible(l1)) {
			continue;
		}
		if (!slot_layer_is_layer(l1)) {
			continue;
		}

		gpu_texture_t        *mask    = empty;
		slot_layer_t_array_t *l1masks = slot_layer_get_masks(l1, true);
		if (l1masks != NULL) {
			if (l1masks->length > 1) {
				layers_make_temp_mask_img();
				draw_begin(pipes_temp_mask_image, GPU_CLEAR_COLOR, 0x00000000);
				draw_end();
				slot_layer_t *l1 = GC_ALLOC_INIT(slot_layer_t, {.texpaint = pipes_temp_mask_image});
				for (i32 i = 0; i < l1masks->length; ++i) {
					layers_merge_layer(l1, l1masks->buffer[i], false);
				}
				mask = pipes_temp_mask_image;
			}
			else {
				mask = l1masks->buffer[0]->texpaint;
			}
		}

		if (l1->paint_base) {
			draw_begin(layers_temp_image, false, 0); // Copy to temp
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expa, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();

			if (context_raw->tool == TOOL_TYPE_GIZMO) {
				// Do not multiply basecol by alpha
				draw_begin(layers_expa, false, 0); // Copy to temp
				draw_set_pipeline(pipes_copy);
				draw_image(l1->texpaint, 0, 0);
				draw_set_pipeline(NULL);
				draw_end();
			}
			else {
				_gpu_begin(layers_expa, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
				gpu_set_pipeline(pipes_merge);
				gpu_set_texture(pipes_tex0, l1->texpaint);
				gpu_set_texture(pipes_tex1, empty);
				gpu_set_texture(pipes_texmask, mask);
				gpu_set_texture(pipes_texa, layers_temp_image);
				gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
				gpu_set_float(pipes_tex1w, empty->width);
				gpu_set_int(pipes_blending, layers->length > 1 ? l1->blending : 0);
				gpu_set_vertex_buffer(const_data_screen_aligned_vb);
				gpu_set_index_buffer(const_data_screen_aligned_ib);
				gpu_draw();
				gpu_end();
			}
		}

		if (l1->paint_nor) {
			draw_begin(layers_temp_image, false, 0);
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expb, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();

			_gpu_begin(layers_expb, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
			gpu_set_pipeline(pipes_merge);
			gpu_set_texture(pipes_tex0, l1->texpaint);
			gpu_set_texture(pipes_tex1, l1->texpaint_nor);
			gpu_set_texture(pipes_texmask, mask);
			gpu_set_texture(pipes_texa, layers_temp_image);
			gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
			gpu_set_float(pipes_tex1w, l1->texpaint_nor->width);
			gpu_set_int(pipes_blending, l1->paint_nor_blend ? 102 : 101);
			gpu_set_vertex_buffer(const_data_screen_aligned_vb);
			gpu_set_index_buffer(const_data_screen_aligned_ib);
			gpu_draw();
			gpu_end();
		}

		if (l1->paint_occ || l1->paint_rough || l1->paint_met || l1->paint_height) {
			draw_begin(layers_temp_image, false, 0);
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expc, 0, 0);
			draw_set_pipeline(NULL);
			draw_end();

			if (l1->paint_occ && l1->paint_rough && l1->paint_met && l1->paint_height) {
				layers_commands_merge_pack(pipes_merge, layers_expc, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask,
				                           l1->paint_height_blend ? 103 : 101);
			}
			else {
				if (l1->paint_occ) {
					layers_commands_merge_pack(pipes_merge_r, layers_expc, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask, 101);
				}
				if (l1->paint_rough) {
					layers_commands_merge_pack(pipes_merge_g, layers_expc, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask, 101);
				}
				if (l1->paint_met) {
					layers_commands_merge_pack(pipes_merge_b, layers_expc, l1->texpaint, l1->texpaint_pack, slot_layer_get_opacity(l1), mask, 101);
				}
			}
		}
	}

	slot_layer_t *l0 = GC_ALLOC_INIT(slot_layer_t, {.texpaint = layers_expa, .texpaint_nor = layers_expb, .texpaint_pack = layers_expc});

	// Merge height map into normal map
	if (height_to_normal && make_material_height_used) {

		draw_begin(layers_temp_image, false, 0);
		draw_set_pipeline(pipes_copy);
		draw_image(l0->texpaint_nor, 0, 0);
		draw_set_pipeline(NULL);
		draw_end();

		_gpu_begin(l0->texpaint_nor, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
		gpu_set_pipeline(pipes_merge);
		gpu_set_texture(pipes_tex0, layers_temp_image);
		gpu_set_texture(pipes_tex1, l0->texpaint_pack);
		gpu_set_texture(pipes_texmask, empty);
		gpu_set_texture(pipes_texa, empty);
		gpu_set_float(pipes_opac, 1.0);
		gpu_set_float(pipes_tex1w, l0->texpaint_pack->width);
		gpu_set_int(pipes_blending, 104);
		gpu_set_vertex_buffer(const_data_screen_aligned_vb);
		gpu_set_index_buffer(const_data_screen_aligned_ib);
		gpu_draw();
		gpu_end();
	}

	return l0;
}

void layers_on_resized_on_next_frame(void *_) {
	layers_resize();
	slot_layer_t    *_layer    = context_raw->layer;
	slot_material_t *_material = context_raw->material;
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->fill_layer != NULL) {
			context_raw->layer    = l;
			context_raw->material = l->fill_layer;
			layers_update_fill_layer(true);
		}
	}
	context_raw->layer    = _layer;
	context_raw->material = _material;
	make_material_parse_paint_material(true);
}

void layers_on_resized() {
	sys_notify_on_next_frame(&layers_on_resized_on_next_frame, NULL);
	gc_unroot(util_uv_uvmap);
	util_uv_uvmap        = NULL;
	util_uv_uvmap_cached = false;
	gc_unroot(util_uv_trianglemap);
	util_uv_trianglemap        = NULL;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached   = false;
	render_path_raytrace_ready = false;
}
