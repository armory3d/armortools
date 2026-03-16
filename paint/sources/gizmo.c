
#include "global.h"

void gizmo_update() {
	bool      is_object = context_raw->tool == TOOL_TYPE_GIZMO;
	bool      is_decal  = base_is_decal_layer();

	object_t *gizmo     = context_raw->gizmo;
	bool      hide      = operator_shortcut(any_map_get(config_keymap, "stencil_hide"), SHORTCUT_TYPE_DOWN);
	gizmo->visible      = (is_object || is_decal) && !hide && config_raw->workspace != WORKSPACE_PLAYER;
	if (!gizmo->visible) {
		return;
	}

	object_t *paint_object = context_raw->paint_object->base;

	if (context_raw->tool == TOOL_TYPE_GIZMO && context_raw->selected_object != NULL) {
		paint_object = context_raw->selected_object;
	}

	if (is_object) {
		gizmo->transform->loc = vec4_clone(paint_object->transform->loc);
	}
	else if (is_decal) {
		gizmo->transform->loc = vec4_create(context_raw->layer->decal_mat.m30, context_raw->layer->decal_mat.m31, context_raw->layer->decal_mat.m32, 1.0);
	}

	camera_object_t *cam                             = scene_camera;
	f32              fov                             = cam->data->fov;
	f32              dist                            = vec4_dist(cam->base->transform->loc, gizmo->transform->loc) / 8.0 * fov;
	gizmo->transform->scale                          = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_translate_x->transform->scale = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_translate_y->transform->scale = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_translate_z->transform->scale = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_scale_x->transform->scale     = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_scale_y->transform->scale     = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_scale_z->transform->scale     = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_rotate_x->transform->scale    = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_rotate_y->transform->scale    = vec4_create(dist, dist, dist, 1.0);
	context_raw->gizmo_rotate_z->transform->scale    = vec4_create(dist, dist, dist, 1.0);
	transform_build_matrix(gizmo->transform);

	// Scene control
	if (is_object) {
		if (context_raw->translate_x || context_raw->translate_y || context_raw->translate_z || context_raw->scale_x || context_raw->scale_y ||
		    context_raw->scale_z || context_raw->rotate_x || context_raw->rotate_y || context_raw->rotate_z) {
			if (context_raw->translate_x) {
				paint_object->transform->loc.x = context_raw->gizmo_drag;
			}
			else if (context_raw->translate_y) {
				paint_object->transform->loc.y = context_raw->gizmo_drag;
			}
			else if (context_raw->translate_z) {
				paint_object->transform->loc.z = context_raw->gizmo_drag;
			}
			else if (context_raw->scale_x) {
				paint_object->transform->scale.x += context_raw->gizmo_drag - context_raw->gizmo_drag_last;
			}
			else if (context_raw->scale_y) {
				paint_object->transform->scale.y += context_raw->gizmo_drag - context_raw->gizmo_drag_last;
			}
			else if (context_raw->scale_z) {
				paint_object->transform->scale.z += context_raw->gizmo_drag - context_raw->gizmo_drag_last;
			}
			else if (context_raw->rotate_x) {
				gizmo_q0                     = quat_from_axis_angle(vec4_x_axis(), -(context_raw->gizmo_drag - context_raw->gizmo_drag_last));
				paint_object->transform->rot = quat_mult(paint_object->transform->rot, gizmo_q0);
			}
			else if (context_raw->rotate_y) {
				gizmo_q0                     = quat_from_axis_angle(vec4_y_axis(), -(context_raw->gizmo_drag - context_raw->gizmo_drag_last));
				paint_object->transform->rot = quat_mult(paint_object->transform->rot, gizmo_q0);
			}
			else if (context_raw->rotate_z) {
				gizmo_q0                     = quat_from_axis_angle(vec4_z_axis(), context_raw->gizmo_drag - context_raw->gizmo_drag_last);
				paint_object->transform->rot = quat_mult(paint_object->transform->rot, gizmo_q0);
			}
			context_raw->gizmo_drag_last = context_raw->gizmo_drag;

			transform_build_matrix(paint_object->transform);

			physics_body_t *pb = any_imap_get(physics_body_object_map, paint_object->uid);
			if (pb != NULL) {
				physics_body_sync_transform(pb);
			}
		}
	}
	// Decal layer control
	else if (is_decal) {
		if (context_raw->translate_x || context_raw->translate_y || context_raw->translate_z || context_raw->scale_x || context_raw->scale_y ||
		    context_raw->scale_z || context_raw->rotate_x || context_raw->rotate_y || context_raw->rotate_z) {
			if (context_raw->translate_x) {
				context_raw->layer->decal_mat.m30 = context_raw->gizmo_drag;
			}
			else if (context_raw->translate_y) {
				context_raw->layer->decal_mat.m31 = context_raw->gizmo_drag;
			}
			else if (context_raw->translate_z) {
				context_raw->layer->decal_mat.m32 = context_raw->gizmo_drag;
			}
			else if (context_raw->scale_x) {
				mat4_decomposed_t *dec = mat4_decompose(context_raw->layer->decal_mat);
				gizmo_v                = dec->loc;
				gizmo_q                = dec->rot;
				gizmo_v0               = dec->scl;
				gizmo_v0.x += context_raw->gizmo_drag - context_raw->gizmo_drag_last;
				context_raw->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw->scale_y) {
				mat4_decomposed_t *dec = mat4_decompose(context_raw->layer->decal_mat);
				gizmo_v                = dec->loc;
				gizmo_q                = dec->rot;
				gizmo_v0               = dec->scl;
				gizmo_v0.y += context_raw->gizmo_drag - context_raw->gizmo_drag_last;
				context_raw->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw->scale_z) {
				mat4_decomposed_t *dec = mat4_decompose(context_raw->layer->decal_mat);
				gizmo_v                = dec->loc;
				gizmo_q                = dec->rot;
				gizmo_v0               = dec->scl;
				gizmo_v0.z += context_raw->gizmo_drag - context_raw->gizmo_drag_last;
				context_raw->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw->rotate_x) {
				mat4_decomposed_t *dec        = mat4_decompose(context_raw->layer->decal_mat);
				gizmo_v                       = dec->loc;
				gizmo_q                       = dec->rot;
				gizmo_v0                      = dec->scl;
				gizmo_q0                      = quat_from_axis_angle(vec4_x_axis(), -context_raw->gizmo_drag + context_raw->gizmo_drag_last);
				gizmo_q                       = quat_mult(gizmo_q0, gizmo_q);
				context_raw->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw->rotate_y) {
				mat4_decomposed_t *dec        = mat4_decompose(context_raw->layer->decal_mat);
				gizmo_v                       = dec->loc;
				gizmo_q                       = dec->rot;
				gizmo_v0                      = dec->scl;
				gizmo_q0                      = quat_from_axis_angle(vec4_y_axis(), -context_raw->gizmo_drag + context_raw->gizmo_drag_last);
				gizmo_q                       = quat_mult(gizmo_q0, gizmo_q);
				context_raw->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw->rotate_z) {
				mat4_decomposed_t *dec        = mat4_decompose(context_raw->layer->decal_mat);
				gizmo_v                       = dec->loc;
				gizmo_q                       = dec->rot;
				gizmo_v0                      = dec->scl;
				gizmo_q0                      = quat_from_axis_angle(vec4_z_axis(), context_raw->gizmo_drag - context_raw->gizmo_drag_last);
				gizmo_q                       = quat_mult(gizmo_q0, gizmo_q);
				context_raw->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			context_raw->gizmo_drag_last = context_raw->gizmo_drag;

			if (context_raw->material != context_raw->layer->fill_layer) {
				context_set_material(context_raw->layer->fill_layer);
			}
			layers_update_fill_layer(context_raw->gizmo_started);
		}
	}

	context_raw->gizmo_started = false;
	if (mouse_started("left") && !string_equals(paint_object->name, "Scene")) {
		// Translate, scale
		transform_t_array_t *trs = any_array_create_from_raw(
		    (void *[]){
		        context_raw->gizmo_translate_x->transform,
		        context_raw->gizmo_translate_y->transform,
		        context_raw->gizmo_translate_z->transform,
		        context_raw->gizmo_scale_x->transform,
		        context_raw->gizmo_scale_y->transform,
		        context_raw->gizmo_scale_z->transform,
		    },
		    6);
		transform_t *hit = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
		if (hit != NULL) {
			if (hit->object == context_raw->gizmo_translate_x) {
				context_raw->translate_x = true;
			}
			else if (hit->object == context_raw->gizmo_translate_y) {
				context_raw->translate_y = true;
			}
			else if (hit->object == context_raw->gizmo_translate_z) {
				context_raw->translate_z = true;
			}
			else if (hit->object == context_raw->gizmo_scale_x) {
				context_raw->scale_x = true;
			}
			else if (hit->object == context_raw->gizmo_scale_y) {
				context_raw->scale_y = true;
			}
			else if (hit->object == context_raw->gizmo_scale_z) {
				context_raw->scale_z = true;
			}
			if (context_raw->translate_x || context_raw->translate_y || context_raw->translate_z || context_raw->scale_x || context_raw->scale_y ||
			    context_raw->scale_z) {
				context_raw->gizmo_offset  = 0.0;
				context_raw->gizmo_started = true;
			}
		}
		else {
			// Rotate
			transform_t_array_t *trs = any_array_create_from_raw(
			    (void *[]){
			        context_raw->gizmo_rotate_x->transform,
			        context_raw->gizmo_rotate_y->transform,
			        context_raw->gizmo_rotate_z->transform,
			    },
			    3);
			transform_t *hit = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != NULL) {
				if (hit->object == context_raw->gizmo_rotate_x) {
					context_raw->rotate_x = true;
				}
				else if (hit->object == context_raw->gizmo_rotate_y) {
					context_raw->rotate_y = true;
				}
				else if (hit->object == context_raw->gizmo_rotate_z) {
					context_raw->rotate_z = true;
				}
				if (context_raw->rotate_x || context_raw->rotate_y || context_raw->rotate_z) {
					context_raw->gizmo_offset  = 0.0;
					context_raw->gizmo_started = true;
				}
			}
		}
	}
	else if (mouse_released("left")) {
		context_raw->translate_x = context_raw->translate_y = context_raw->translate_z = false;
		context_raw->scale_x = context_raw->scale_y = context_raw->scale_z = false;
		context_raw->rotate_x = context_raw->rotate_y = context_raw->rotate_z = false;
	}

	if (context_raw->translate_x || context_raw->translate_y || context_raw->translate_z || context_raw->scale_x || context_raw->scale_y ||
	    context_raw->scale_z || context_raw->rotate_x || context_raw->rotate_y || context_raw->rotate_z) {
		context_raw->rdirty = 2;
		if (is_object) {
			transform_t *t = paint_object->transform;
			gizmo_v        = vec4_create(transform_world_x(t), transform_world_y(t), transform_world_z(t), 1.0);
		}
		else if (is_decal) {
			gizmo_v = vec4_create(context_raw->layer->decal_mat.m30, context_raw->layer->decal_mat.m31, context_raw->layer->decal_mat.m32, 1.0);
		}

		if (context_raw->translate_x || context_raw->scale_x) {
			vec4_t hit = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw->gizmo_started) {
					context_raw->gizmo_offset = hit.x - gizmo_v.x;
				}
				context_raw->gizmo_drag = hit.x - context_raw->gizmo_offset;
			}
		}
		else if (context_raw->translate_y || context_raw->scale_y) {
			vec4_t hit = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw->gizmo_started) {
					context_raw->gizmo_offset = hit.y - gizmo_v.y;
				}
				context_raw->gizmo_drag = hit.y - context_raw->gizmo_offset;
			}
		}
		else if (context_raw->translate_z || context_raw->scale_z) {
			vec4_t hit = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw->gizmo_started) {
					context_raw->gizmo_offset = hit.z - gizmo_v.z;
				}
				context_raw->gizmo_drag = hit.z - context_raw->gizmo_offset;
			}
		}
		else if (context_raw->rotate_x) {
			vec4_t hit = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw->gizmo_started) {
					mat4_decomposed_t *dec    = mat4_decompose(context_raw->layer->decal_mat);
					gizmo_v                   = dec->loc;
					gizmo_q                   = dec->rot;
					gizmo_v0                  = dec->scl;
					context_raw->gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z);
				}
				context_raw->gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z) - context_raw->gizmo_offset;
			}
		}
		else if (context_raw->rotate_y) {
			vec4_t hit = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw->gizmo_started) {
					mat4_decomposed_t *dec    = mat4_decompose(context_raw->layer->decal_mat);
					gizmo_v                   = dec->loc;
					gizmo_q                   = dec->rot;
					gizmo_v0                  = dec->scl;
					context_raw->gizmo_offset = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x);
				}
				context_raw->gizmo_drag = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x) - context_raw->gizmo_offset;
			}
		}
		else if (context_raw->rotate_z) {
			vec4_t hit = raycast_plane_intersect(vec4_z_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw->gizmo_started) {
					mat4_decomposed_t *dec    = mat4_decompose(context_raw->layer->decal_mat);
					gizmo_v                   = dec->loc;
					gizmo_q                   = dec->rot;
					gizmo_v0                  = dec->scl;
					context_raw->gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x);
				}
				context_raw->gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x) - context_raw->gizmo_offset;
			}
		}

		if (context_raw->gizmo_started) {
			context_raw->gizmo_drag_last = context_raw->gizmo_drag;
		}
	}

	_input_occupied = (context_raw->translate_x || context_raw->translate_y || context_raw->translate_z || context_raw->scale_x || context_raw->scale_y ||
	                   context_raw->scale_z || context_raw->rotate_x || context_raw->rotate_y || context_raw->rotate_z) &&
	                  mouse_view_x() < base_w();
}
