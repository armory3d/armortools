
#include "global.h"

static slot_layer_t *path_layer_current      = NULL;
static object_t    **path_point_spheres      = NULL;
static i32           path_point_sphere_count = 0;
static i32           path_point_dragging     = -1;
static i32           path_layer_last_active  = -1;

bool util_layer_is_path_point_dragging() {
	return path_point_dragging >= 0;
}

static void path_layer_destroy_spheres() {
	for (i32 i = 0; i < path_point_sphere_count; i++) {
		mesh_object_remove((mesh_object_t *)path_point_spheres[i]->ext);
	}
	gc_free(path_point_spheres);
	path_point_spheres      = NULL;
	path_point_sphere_count = 0;
}

static void path_layer_push_camera(f32_array_t *ar) {
	vec4_t loc = scene_camera->base->transform->loc;
	quat_t rot = scene_camera->base->transform->rot;
	f32_array_push(ar, loc.x);
	f32_array_push(ar, loc.y);
	f32_array_push(ar, loc.z);
	f32_array_push(ar, loc.w);
	f32_array_push(ar, rot.x);
	f32_array_push(ar, rot.y);
	f32_array_push(ar, rot.z);
	f32_array_push(ar, rot.w);
	f32_array_push(ar, sys_w() / (f32)sys_h());
}

static void path_set_camera(f32_array_t *points_camera, i32 num_camera, i32 ci) {
	if (ci < num_camera) {
		scene_camera->base->transform->loc = (vec4_t){points_camera->buffer[ci * 9 + 0], points_camera->buffer[ci * 9 + 1], points_camera->buffer[ci * 9 + 2],
		                                              points_camera->buffer[ci * 9 + 3]};
		scene_camera->base->transform->rot = (quat_t){points_camera->buffer[ci * 9 + 4], points_camera->buffer[ci * 9 + 5], points_camera->buffer[ci * 9 + 6],
		                                              points_camera->buffer[ci * 9 + 7]};
		camera_object_build_proj(scene_camera, points_camera->buffer[ci * 9 + 8]);
		camera_object_build_mat(scene_camera);
		render_path_base_draw_gbuffer();
	}
}

static void path_paint(f32 px, f32 py, bool is_decal, f32 *prev_px, f32 *prev_py) {
	if (is_decal) {
		g_context->decal_x = px;
		g_context->decal_y = py;
	}
	g_context->paint_vec.x      = px;
	g_context->paint_vec.y      = py;
	g_context->last_paint_vec_x = *prev_px;
	g_context->last_paint_vec_y = *prev_py;
	g_context->pdirty           = 1;
	render_path_paint_commands_paint(false);
	*prev_px = px;
	*prev_py = py;
}

static void layers_repaint_path_layer(slot_layer_t *l) {
	if (l->path_material == NULL) {
		return;
	}
	f32_array_t *points_world = l->path_points_world;
	if (points_world == NULL || points_world->length == 0) {
		return;
	}

	slot_layer_t    *_layer     = g_context->layer;
	slot_material_t *_material  = g_context->material;
	tool_type_t      _tool      = g_context->tool;
	f32              _last_x    = g_context->last_paint_vec_x;
	f32              _last_y    = g_context->last_paint_vec_y;
	vec4_t           _paint_vec = g_context->paint_vec;
	g_context->layer            = l;
	g_context->material         = l->path_material;
	g_context->tool             = l->path_tool;

	make_material_parse_paint_material(false);
	layers_set_object_mask();

	slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);

	vec4_t _camera_loc = scene_camera->base->transform->loc;
	quat_t _camera_rot = scene_camera->base->transform->rot;

	f32_array_t *points        = l->path_points;
	f32_array_t *points_camera = l->path_points_camera;
	i32_array_t *points_parent = l->path_points_parent;
	i32          num_world     = points_world->length / 3;
	i32          num_camera    = points_camera->length / 9;
	i32          num_parent    = points_parent->length;
	bool         is_decal      = context_is_decal();

	if (l->path_curved && num_world >= 1) {
		f32 prev_px = 0.5f;
		f32 prev_py = 0.5f;

		// Paint the first anchor
		f32 pt0x = points->length >= 2 ? points->buffer[0] : 0.5f;
		f32 pt0y = points->length >= 2 ? points->buffer[1] : 0.5f;
		path_set_camera(points_camera, num_camera, 0);
		prev_px = pt0x;
		prev_py = pt0y;
		path_paint(pt0x, pt0y, is_decal, &prev_px, &prev_py);

		// Paint curve - anchor[p], control[j-1], anchor[j]
		for (i32 j = 2; j < num_world; j += 2) {
			i32 p   = (j < num_parent) ? points_parent->buffer[j] : j - 2;
			f32 ax1 = j * 2 + 1 < (i32)points->length ? points->buffer[j * 2] : 0.5f;
			f32 ay1 = j * 2 + 1 < (i32)points->length ? points->buffer[j * 2 + 1] : 0.5f;

			f32 wx0 = points_world->buffer[p * 3];
			f32 wy0 = points_world->buffer[p * 3 + 1];
			f32 wz0 = points_world->buffer[p * 3 + 2];
			f32 wcx = points_world->buffer[(j - 1) * 3];
			f32 wcy = points_world->buffer[(j - 1) * 3 + 1];
			f32 wcz = points_world->buffer[(j - 1) * 3 + 2];
			f32 wx1 = points_world->buffer[j * 3];
			f32 wy1 = points_world->buffer[j * 3 + 1];
			f32 wz1 = points_world->buffer[j * 3 + 2];

			// Restore the start anchor camera and re-project its position as prev
			if (p < num_camera) {
				path_set_camera(points_camera, num_camera, p);
				vec4_t pp = vec4_apply_mat4((vec4_t){wx0, wy0, wz0, 1.0f}, scene_camera->vp);
				if (pp.w > 0.0f) {
					prev_px = (pp.x / pp.w + 1.0f) * 0.5f;
					prev_py = (-pp.y / pp.w + 1.0f) * 0.5f;
				}
			}

			// 16 intermediate points + final anchor
			bool have_j   = j < num_camera;
			f32  prev_bwx = wx0;
			f32  prev_bwy = wy0;
			f32  prev_bwz = wz0;
			for (i32 s = 1; s <= 17; s++) {
				f32 t   = s / 17.0f;
				f32 t1  = 1.0f - t;
				f32 bwx = t1 * t1 * t1 * wx0 + 3.0f * t * t1 * wcx + t * t * t * wx1;
				f32 bwy = t1 * t1 * t1 * wy0 + 3.0f * t * t1 * wcy + t * t * t * wy1;
				f32 bwz = t1 * t1 * t1 * wz0 + 3.0f * t * t1 * wcz + t * t * t * wz1;
				if (s == 17 && have_j) {
					path_set_camera(points_camera, num_camera, j);
					// Re-project the previous world pos from the new camera so that
					// last_paint_vec and paint_vec are in the same screen space
					vec4_t pc = vec4_apply_mat4((vec4_t){prev_bwx, prev_bwy, prev_bwz, 1.0f}, scene_camera->vp);
					if (pc.w > 0.0f) {
						prev_px = (pc.x / pc.w + 1.0f) * 0.5f;
						prev_py = (-pc.y / pc.w + 1.0f) * 0.5f;
					}
				}
				vec4_t clip = vec4_apply_mat4((vec4_t){bwx, bwy, bwz, 1.0f}, scene_camera->vp);
				f32    bx, by;
				if (clip.w > 0.0f) {
					bx = (clip.x / clip.w + 1.0f) * 0.5f;
					by = (-clip.y / clip.w + 1.0f) * 0.5f;
				}
				else {
					bx = ax1;
					by = ay1;
				}
				path_paint(bx, by, is_decal, &prev_px, &prev_py);
				prev_bwx = bwx;
				prev_bwy = bwy;
				prev_bwz = bwz;
			}
		}

		render_path_paint_dilate(true, true);
	}
	else {
		for (i32 i = 0; i < num_world; i++) {
			path_set_camera(points_camera, num_camera, i);

			f32 cur_px = points->buffer[i * 2];
			f32 cur_py = points->buffer[i * 2 + 1];

			// Project parent world point for stroke connection
			f32 prev_px = cur_px;
			f32 prev_py = cur_py;
			if (!is_decal) {
				i32 parent = points_parent->buffer[i];
				if (parent >= 0) {
					f32    pwx  = points_world->buffer[parent * 3];
					f32    pwy  = points_world->buffer[parent * 3 + 1];
					f32    pwz  = points_world->buffer[parent * 3 + 2];
					vec4_t clip = vec4_apply_mat4((vec4_t){pwx, pwy, pwz, 1.0f}, scene_camera->vp);
					if (clip.w > 0.0f) {
						prev_px = (clip.x / clip.w + 1.0f) * 0.5f;
						prev_py = (-clip.y / clip.w + 1.0f) * 0.5f;
					}
				}
			}

			path_paint(cur_px, cur_py, is_decal, &prev_px, &prev_py);
		}

		render_path_paint_dilate(true, true);
	}

	g_context->tool = _tool;

	scene_camera->base->transform->loc = _camera_loc;
	scene_camera->base->transform->rot = _camera_rot;
	camera_object_build_proj(scene_camera, -1.0);
	camera_object_build_mat(scene_camera);
	render_path_base_draw_gbuffer();

	g_context->pdirty   = 0;
	g_context->rdirty   = 2;
	g_context->layer    = _layer;
	g_context->material = _material;
	make_material_parse_paint_material(false);
	g_context->last_paint_vec_x = _last_x;
	g_context->last_paint_vec_y = _last_y;
	g_context->paint_vec        = _paint_vec;
}

void util_layer_clear_path_points(slot_layer_t *l) {
	if (l->path_points == NULL) {
		return;
	}
	l->path_points->length        = 0;
	l->path_points_world->length  = 0;
	l->path_points_camera->length = 0;
	l->path_points_parent->length = 0;
	l->path_tool                  = -1;
	path_layer_last_active        = -1;
}

void util_layer_add_path_point(slot_layer_t *l, f32 screen_x, f32 screen_y) {
	if (l->path_tool == -1) {
		l->path_tool = g_context->tool;
	}
	if ((i32)g_context->tool != l->path_tool) {
		return;
	}

	for (i32 j = 0; j < path_point_sphere_count; j++) {
		path_point_spheres[j]->visible = false;
	}

	// Re-render gbuffer without spheres so gbufferD is clean
	render_path_base_draw_gbuffer();

	f32_array_t *points        = l->path_points;
	f32_array_t *points_world  = l->path_points_world;
	f32_array_t *points_camera = l->path_points_camera;
	i32_array_t *points_parent = l->path_points_parent;

	f32_array_push(points, screen_x);
	f32_array_push(points, screen_y);
	f32 saved_pvx          = g_context->paint_vec.x;
	f32 saved_pvy          = g_context->paint_vec.y;
	g_context->paint_vec.x = screen_x;
	g_context->paint_vec.y = screen_y;
	util_render_pick_pos_nor_tex();
	g_context->paint_vec.x = saved_pvx;
	g_context->paint_vec.y = saved_pvy;
	f32_array_push(points_world, g_context->posx_picked);
	f32_array_push(points_world, g_context->posy_picked);
	f32_array_push(points_world, g_context->posz_picked);
	path_layer_push_camera(points_camera);

	// For curved paths the parent must be an anchor
	i32 parent = path_layer_last_active;
	if (l->path_curved && parent % 2 == 1) {
		if (parent < (i32)points_parent->length) {
			parent = points_parent->buffer[parent];
		}
		else {
			parent -= 1;
		}
	}
	i32_array_push(points_parent, parent);

	// New point becomes the parent for the next stroke
	i32 new_index          = points->length / 2 - 1;
	path_layer_last_active = new_index;

	layers_repaint_path_layer(l);

	for (i32 j = 0; j < path_point_sphere_count; j++) {
		path_point_spheres[j]->visible = true;
	}
}

void util_layer_repaint_path(slot_layer_t *l) {
	for (i32 j = 0; j < path_point_sphere_count; j++) {
		path_point_spheres[j]->visible = false;
	}
	layers_repaint_path_layer(l);
	for (i32 j = 0; j < path_point_sphere_count; j++) {
		path_point_spheres[j]->visible = true;
	}
}

void util_layer_update_path() {
	if (g_config->workspace == WORKSPACE_PLAYER || g_context->paint2d) {
		return;
	}

	slot_layer_t *l       = g_context->layer;
	bool          is_path = slot_layer_is_path(l);

	// Clear spheres when switching away from path layer
	if (path_layer_current != l) {
		path_layer_destroy_spheres();
		path_point_dragging    = -1;
		path_layer_current     = is_path ? l : NULL;
		path_layer_last_active = -1;
		if (is_path) {
			// Resume from the last point when switching to a path layer
			i32 n                  = l->path_points->length / 2;
			path_layer_last_active = n > 0 ? n - 1 : -1;
		}
	}

	if (!is_path) {
		return;
	}

	f32_array_t *points       = l->path_points;
	f32_array_t *points_world = l->path_points_world;
	i32          num_points   = points->length / 2;
	i32          num_world    = points_world->length / 3;
	i32          vis_points   = num_points;

	// Sync sphere count with number of visible path points
	if (vis_points < path_point_sphere_count) {
		while (path_point_sphere_count > vis_points) {
			path_point_sphere_count--;
			mesh_object_remove((mesh_object_t *)path_point_spheres[path_point_sphere_count]->ext);
			path_point_spheres[path_point_sphere_count] = NULL;
		}
	}
	else if (vis_points > path_point_sphere_count) {
		path_point_spheres = (object_t **)gc_realloc(path_point_spheres, vis_points * sizeof(object_t *));
		gc_root(path_point_spheres);
		for (i32 i = path_point_sphere_count; i < vis_points; i++) {
			object_t *o           = scene_spawn_object(".Sphere", NULL, true);
			o->visible            = true;
			path_point_spheres[i] = o;
		}
		path_point_sphere_count = vis_points;
	}

	// Update sphere transforms
	for (i32 i = 0; i < vis_points; i++) {
		if (path_point_spheres[i] == NULL) {
			continue;
		}
		f32 wx    = points_world->buffer[i * 3];
		f32 wy    = points_world->buffer[i * 3 + 1];
		f32 wz    = points_world->buffer[i * 3 + 2];
		f32 dist  = vec4_dist(scene_camera->base->transform->loc, (vec4_t){wx, wy, wz, 1.0});
		f32 fov   = scene_camera->data->fov;
		f32 scale = dist / 8.0f * fov * 0.1f * (i == path_layer_last_active ? 1.5f : 1.0f);

		path_point_spheres[i]->transform->loc   = (vec4_t){wx, wy, wz, 1.0};
		path_point_spheres[i]->transform->scale = (vec4_t){scale, scale, scale, 1.0};
		transform_build_matrix(path_point_spheres[i]->transform);
	}

	// Release drag - the dragged point becomes the parent for the next new point
	if (mouse_released("left") && path_point_dragging >= 0) {
		for (i32 j = 0; j < path_point_sphere_count; j++) {
			path_point_spheres[j]->visible = true;
		}
		path_layer_last_active          = path_point_dragging;
		path_point_dragging             = -1;
		g_context->layer_preview_dirty  = true;
		g_context->layers_preview_dirty = true;
	}

	// Drag point
	if (mouse_down("left") && !mouse_started("left") && path_point_dragging >= 0) {
		f32 new_x = g_context->paint_vec.x;
		f32 new_y = g_context->paint_vec.y;
		f32 old_x = points->buffer[path_point_dragging * 2];
		f32 old_y = points->buffer[path_point_dragging * 2 + 1];

		if (math_abs(new_x - old_x) > 0.0005f || math_abs(new_y - old_y) > 0.0005f) {
			points->buffer[path_point_dragging * 2]     = new_x;
			points->buffer[path_point_dragging * 2 + 1] = new_y;

			// Pick world position at new screen location
			f32 saved_pvx          = g_context->paint_vec.x;
			f32 saved_pvy          = g_context->paint_vec.y;
			g_context->paint_vec.x = new_x;
			g_context->paint_vec.y = new_y;
			util_render_pick_pos_nor_tex();
			g_context->paint_vec.x = saved_pvx;
			g_context->paint_vec.y = saved_pvy;

			if (math_abs(g_context->posx_picked) < 50.0f) {
				points_world->buffer[path_point_dragging * 3]     = g_context->posx_picked;
				points_world->buffer[path_point_dragging * 3 + 1] = g_context->posy_picked;
				points_world->buffer[path_point_dragging * 3 + 2] = g_context->posz_picked;
				f32_array_t *points_camera                        = l->path_points_camera;
				if (points_camera->length >= (path_point_dragging + 1) * 9) {
					vec4_t loc                                         = scene_camera->base->transform->loc;
					quat_t rot                                         = scene_camera->base->transform->rot;
					points_camera->buffer[path_point_dragging * 9 + 0] = loc.x;
					points_camera->buffer[path_point_dragging * 9 + 1] = loc.y;
					points_camera->buffer[path_point_dragging * 9 + 2] = loc.z;
					points_camera->buffer[path_point_dragging * 9 + 3] = loc.w;
					points_camera->buffer[path_point_dragging * 9 + 4] = rot.x;
					points_camera->buffer[path_point_dragging * 9 + 5] = rot.y;
					points_camera->buffer[path_point_dragging * 9 + 6] = rot.z;
					points_camera->buffer[path_point_dragging * 9 + 7] = rot.w;
					points_camera->buffer[path_point_dragging * 9 + 8] = sys_w() / (f32)sys_h();
				}
			}

			util_layer_repaint_path(l);
			for (i32 j = 0; j < path_point_sphere_count; j++) {
				path_point_spheres[j]->visible = false;
			}
		}
		return;
	}

	// Check if mouse ray hits any point sphere
	if (mouse_started("left") && !ui->is_hovered && !base_is_dragging && vis_points > 0) {
		f32 min_dist = 1e10f;
		i32 closest  = -1;

		for (i32 i = 0; i < vis_points; i++) {
			f32    wx         = points_world->buffer[i * 3];
			f32    wy         = points_world->buffer[i * 3 + 1];
			f32    wz         = points_world->buffer[i * 3 + 2];
			f32    dist       = vec4_dist(scene_camera->base->transform->loc, (vec4_t){wx, wy, wz, 1.0});
			f32    fov        = scene_camera->data->fov;
			f32    hit_radius = dist / 8.0f * fov * 0.25f;
			ray_t *ray        = raycast_get_ray(mouse_view_x(), mouse_view_y(), scene_camera);
			if (ray_intersects_sphere(ray, (vec4_t){wx, wy, wz, 1.0}, hit_radius) && dist < min_dist) {
				min_dist = dist;
				closest  = i;
			}
		}

		if (closest >= 0) {
			path_point_dragging = closest;
			for (i32 j = 0; j < path_point_sphere_count; j++) {
				if (path_point_spheres[j] != NULL) {
					path_point_spheres[j]->visible = false;
				}
			}
		}
	}
}
