
#include "global.h"

void gizmo_update() {
	bool      is_object = g_context->tool == TOOL_TYPE_GIZMO;
	bool      is_decal  = base_is_decal_layer();

	object_t *gizmo     = g_context->gizmo;
	bool      hide      = operator_shortcut(any_map_get(config_keymap, "stencil_hide"), SHORTCUT_TYPE_DOWN);
	gizmo->visible      = (is_object || is_decal) && !hide && g_config->workspace != WORKSPACE_PLAYER;
	if (!gizmo->visible) {
		return;
	}

	object_t *paint_object = g_context->paint_object->base;

	if (g_context->tool == TOOL_TYPE_GIZMO && g_context->selected_object != NULL) {
		paint_object = g_context->selected_object;
	}

	if (is_object) {
		gizmo->transform->loc = vec4_clone(paint_object->transform->loc);
	}
	else if (is_decal) {
		gizmo->transform->loc = vec4_create(g_context->layer->decal_mat.m30, g_context->layer->decal_mat.m31, g_context->layer->decal_mat.m32, 1.0);
	}

	camera_object_t *cam                             = scene_camera;
	f32              fov                             = cam->data->fov;
	f32              dist                            = vec4_dist(cam->base->transform->loc, gizmo->transform->loc) / 8.0 * fov;
	gizmo->transform->scale                          = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_translate_x->transform->scale = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_translate_y->transform->scale = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_translate_z->transform->scale = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_scale_x->transform->scale     = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_scale_y->transform->scale     = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_scale_z->transform->scale     = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_rotate_x->transform->scale    = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_rotate_y->transform->scale    = vec4_create(dist, dist, dist, 1.0);
	g_context->gizmo_rotate_z->transform->scale    = vec4_create(dist, dist, dist, 1.0);
	transform_build_matrix(gizmo->transform);

	// Scene control
	if (is_object) {
		if (g_context->translate_x || g_context->translate_y || g_context->translate_z || g_context->scale_x || g_context->scale_y ||
		    g_context->scale_z || g_context->rotate_x || g_context->rotate_y || g_context->rotate_z) {
			if (g_context->translate_x) {
				paint_object->transform->loc.x = g_context->gizmo_drag;
			}
			else if (g_context->translate_y) {
				paint_object->transform->loc.y = g_context->gizmo_drag;
			}
			else if (g_context->translate_z) {
				paint_object->transform->loc.z = g_context->gizmo_drag;
			}
			else if (g_context->scale_x) {
				paint_object->transform->scale.x += g_context->gizmo_drag - g_context->gizmo_drag_last;
			}
			else if (g_context->scale_y) {
				paint_object->transform->scale.y += g_context->gizmo_drag - g_context->gizmo_drag_last;
			}
			else if (g_context->scale_z) {
				paint_object->transform->scale.z += g_context->gizmo_drag - g_context->gizmo_drag_last;
			}
			else if (g_context->rotate_x) {
				gizmo_q0                     = quat_from_axis_angle(vec4_x_axis(), -(g_context->gizmo_drag - g_context->gizmo_drag_last));
				paint_object->transform->rot = quat_mult(paint_object->transform->rot, gizmo_q0);
			}
			else if (g_context->rotate_y) {
				gizmo_q0                     = quat_from_axis_angle(vec4_y_axis(), -(g_context->gizmo_drag - g_context->gizmo_drag_last));
				paint_object->transform->rot = quat_mult(paint_object->transform->rot, gizmo_q0);
			}
			else if (g_context->rotate_z) {
				gizmo_q0                     = quat_from_axis_angle(vec4_z_axis(), g_context->gizmo_drag - g_context->gizmo_drag_last);
				paint_object->transform->rot = quat_mult(paint_object->transform->rot, gizmo_q0);
			}
			g_context->gizmo_drag_last = g_context->gizmo_drag;

			transform_build_matrix(paint_object->transform);

			physics_body_t *pb = any_imap_get(physics_body_object_map, paint_object->uid);
			if (pb != NULL) {
				physics_body_sync_transform(pb);
			}
		}
	}
	// Decal layer control
	else if (is_decal) {
		if (g_context->translate_x || g_context->translate_y || g_context->translate_z || g_context->scale_x || g_context->scale_y ||
		    g_context->scale_z || g_context->rotate_x || g_context->rotate_y || g_context->rotate_z) {
			if (g_context->translate_x) {
				g_context->layer->decal_mat.m30 = g_context->gizmo_drag;
			}
			else if (g_context->translate_y) {
				g_context->layer->decal_mat.m31 = g_context->gizmo_drag;
			}
			else if (g_context->translate_z) {
				g_context->layer->decal_mat.m32 = g_context->gizmo_drag;
			}
			else if (g_context->scale_x) {
				mat4_decomposed_t *dec = mat4_decompose(g_context->layer->decal_mat);
				gizmo_v                = dec->loc;
				gizmo_q                = dec->rot;
				gizmo_v0               = dec->scl;
				gizmo_v0.x += g_context->gizmo_drag - g_context->gizmo_drag_last;
				g_context->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (g_context->scale_y) {
				mat4_decomposed_t *dec = mat4_decompose(g_context->layer->decal_mat);
				gizmo_v                = dec->loc;
				gizmo_q                = dec->rot;
				gizmo_v0               = dec->scl;
				gizmo_v0.y += g_context->gizmo_drag - g_context->gizmo_drag_last;
				g_context->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (g_context->scale_z) {
				mat4_decomposed_t *dec = mat4_decompose(g_context->layer->decal_mat);
				gizmo_v                = dec->loc;
				gizmo_q                = dec->rot;
				gizmo_v0               = dec->scl;
				gizmo_v0.z += g_context->gizmo_drag - g_context->gizmo_drag_last;
				g_context->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (g_context->rotate_x) {
				mat4_decomposed_t *dec        = mat4_decompose(g_context->layer->decal_mat);
				gizmo_v                       = dec->loc;
				gizmo_q                       = dec->rot;
				gizmo_v0                      = dec->scl;
				gizmo_q0                      = quat_from_axis_angle(vec4_x_axis(), -g_context->gizmo_drag + g_context->gizmo_drag_last);
				gizmo_q                       = quat_mult(gizmo_q0, gizmo_q);
				g_context->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (g_context->rotate_y) {
				mat4_decomposed_t *dec        = mat4_decompose(g_context->layer->decal_mat);
				gizmo_v                       = dec->loc;
				gizmo_q                       = dec->rot;
				gizmo_v0                      = dec->scl;
				gizmo_q0                      = quat_from_axis_angle(vec4_y_axis(), -g_context->gizmo_drag + g_context->gizmo_drag_last);
				gizmo_q                       = quat_mult(gizmo_q0, gizmo_q);
				g_context->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (g_context->rotate_z) {
				mat4_decomposed_t *dec        = mat4_decompose(g_context->layer->decal_mat);
				gizmo_v                       = dec->loc;
				gizmo_q                       = dec->rot;
				gizmo_v0                      = dec->scl;
				gizmo_q0                      = quat_from_axis_angle(vec4_z_axis(), g_context->gizmo_drag - g_context->gizmo_drag_last);
				gizmo_q                       = quat_mult(gizmo_q0, gizmo_q);
				g_context->layer->decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			g_context->gizmo_drag_last = g_context->gizmo_drag;

			if (g_context->material != g_context->layer->fill_layer) {
				context_set_material(g_context->layer->fill_layer);
			}
			layers_update_fill_layer(g_context->gizmo_started);
		}
	}

	g_context->gizmo_started = false;
	if (mouse_started("left") && !string_equals(paint_object->name, "Scene")) {
		// Translate, scale
		transform_t_array_t *trs = any_array_create_from_raw(
		    (void *[]){
		        g_context->gizmo_translate_x->transform,
		        g_context->gizmo_translate_y->transform,
		        g_context->gizmo_translate_z->transform,
		        g_context->gizmo_scale_x->transform,
		        g_context->gizmo_scale_y->transform,
		        g_context->gizmo_scale_z->transform,
		    },
		    6);
		transform_t *hit = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
		if (hit != NULL) {
			if (hit->object == g_context->gizmo_translate_x) {
				g_context->translate_x = true;
			}
			else if (hit->object == g_context->gizmo_translate_y) {
				g_context->translate_y = true;
			}
			else if (hit->object == g_context->gizmo_translate_z) {
				g_context->translate_z = true;
			}
			else if (hit->object == g_context->gizmo_scale_x) {
				g_context->scale_x = true;
			}
			else if (hit->object == g_context->gizmo_scale_y) {
				g_context->scale_y = true;
			}
			else if (hit->object == g_context->gizmo_scale_z) {
				g_context->scale_z = true;
			}
			if (g_context->translate_x || g_context->translate_y || g_context->translate_z || g_context->scale_x || g_context->scale_y ||
			    g_context->scale_z) {
				g_context->gizmo_offset  = 0.0;
				g_context->gizmo_started = true;
			}
		}
		else {
			// Rotate
			transform_t_array_t *trs = any_array_create_from_raw(
			    (void *[]){
			        g_context->gizmo_rotate_x->transform,
			        g_context->gizmo_rotate_y->transform,
			        g_context->gizmo_rotate_z->transform,
			    },
			    3);
			transform_t *hit = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != NULL) {
				if (hit->object == g_context->gizmo_rotate_x) {
					g_context->rotate_x = true;
				}
				else if (hit->object == g_context->gizmo_rotate_y) {
					g_context->rotate_y = true;
				}
				else if (hit->object == g_context->gizmo_rotate_z) {
					g_context->rotate_z = true;
				}
				if (g_context->rotate_x || g_context->rotate_y || g_context->rotate_z) {
					g_context->gizmo_offset  = 0.0;
					g_context->gizmo_started = true;
				}
			}
		}
	}
	else if (mouse_released("left")) {
		g_context->translate_x = g_context->translate_y = g_context->translate_z = false;
		g_context->scale_x = g_context->scale_y = g_context->scale_z = false;
		g_context->rotate_x = g_context->rotate_y = g_context->rotate_z = false;
	}

	if (g_context->translate_x || g_context->translate_y || g_context->translate_z || g_context->scale_x || g_context->scale_y ||
	    g_context->scale_z || g_context->rotate_x || g_context->rotate_y || g_context->rotate_z) {
		g_context->rdirty = 2;
		if (is_object) {
			transform_t *t = paint_object->transform;
			gizmo_v        = vec4_create(transform_world_x(t), transform_world_y(t), transform_world_z(t), 1.0);
		}
		else if (is_decal) {
			gizmo_v = vec4_create(g_context->layer->decal_mat.m30, g_context->layer->decal_mat.m31, g_context->layer->decal_mat.m32, 1.0);
		}

		if (g_context->translate_x || g_context->scale_x) {
			vec4_t hit = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (g_context->gizmo_started) {
					g_context->gizmo_offset = hit.x - gizmo_v.x;
				}
				g_context->gizmo_drag = hit.x - g_context->gizmo_offset;
			}
		}
		else if (g_context->translate_y || g_context->scale_y) {
			vec4_t hit = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (g_context->gizmo_started) {
					g_context->gizmo_offset = hit.y - gizmo_v.y;
				}
				g_context->gizmo_drag = hit.y - g_context->gizmo_offset;
			}
		}
		else if (g_context->translate_z || g_context->scale_z) {
			vec4_t hit = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (g_context->gizmo_started) {
					g_context->gizmo_offset = hit.z - gizmo_v.z;
				}
				g_context->gizmo_drag = hit.z - g_context->gizmo_offset;
			}
		}
		else if (g_context->rotate_x) {
			vec4_t hit = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (g_context->gizmo_started) {
					mat4_decomposed_t *dec    = mat4_decompose(g_context->layer->decal_mat);
					gizmo_v                   = dec->loc;
					gizmo_q                   = dec->rot;
					gizmo_v0                  = dec->scl;
					g_context->gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z);
				}
				g_context->gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z) - g_context->gizmo_offset;
			}
		}
		else if (g_context->rotate_y) {
			vec4_t hit = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (g_context->gizmo_started) {
					mat4_decomposed_t *dec    = mat4_decompose(g_context->layer->decal_mat);
					gizmo_v                   = dec->loc;
					gizmo_q                   = dec->rot;
					gizmo_v0                  = dec->scl;
					g_context->gizmo_offset = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x);
				}
				g_context->gizmo_drag = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x) - g_context->gizmo_offset;
			}
		}
		else if (g_context->rotate_z) {
			vec4_t hit = raycast_plane_intersect(vec4_z_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (g_context->gizmo_started) {
					mat4_decomposed_t *dec    = mat4_decompose(g_context->layer->decal_mat);
					gizmo_v                   = dec->loc;
					gizmo_q                   = dec->rot;
					gizmo_v0                  = dec->scl;
					g_context->gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x);
				}
				g_context->gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x) - g_context->gizmo_offset;
			}
		}

		if (g_context->gizmo_started) {
			g_context->gizmo_drag_last = g_context->gizmo_drag;
		}
	}

	_input_occupied = (g_context->translate_x || g_context->translate_y || g_context->translate_z || g_context->scale_x || g_context->scale_y ||
	                   g_context->scale_z || g_context->rotate_x || g_context->rotate_y || g_context->rotate_z) &&
	                  mouse_view_x() < base_w();
}
