#pragma once

#include "engine.h"
#include "iron_math.h"

extern mat4_t _raycast_vp_inv;
extern mat4_t _raycast_p_inv;
extern mat4_t _raycast_v_inv;

typedef struct ray {
	vec4_t origin;
	vec4_t dir;
} ray_t;

typedef struct plane {
	vec4_t normal;
	f32    constant;
} plane_t;

ray_t *ray_create(vec4_t origin, vec4_t dir);
vec4_t ray_at(ray_t *raw, f32 t);
f32    ray_dist_to_point(ray_t *raw, vec4_t point);
bool   ray_intersects_sphere(ray_t *raw, vec4_t sphere_center, f32 sphere_radius);
bool   ray_intersects_plane(ray_t *raw, plane_t *plane);
f32    ray_dist_to_plane(ray_t *raw, plane_t *plane);
vec4_t ray_intersect_plane(ray_t *raw, plane_t *plane);
vec4_t ray_intersect_box(ray_t *raw, vec4_t center, vec4_t dim);
vec4_t ray_intersect_triangle(ray_t *raw, vec4_t a, vec4_t b, vec4_t c, bool cull_backface);

plane_t plane_create(void);
f32     plane_dist_to_point(plane_t *raw, vec4_t point);
plane_t plane_set(plane_t *raw, vec4_t normal, vec4_t point);

ray_t       *raycast_get_ray(f32 input_x, f32 input_y, camera_object_t *camera);
vec4_t       raycast_box_intersect(transform_t *transform, f32 input_x, f32 input_y, camera_object_t *camera);
transform_t *raycast_closest_box_intersect(any_array_t *transforms, f32 input_x, f32 input_y, camera_object_t *camera);
vec4_t       raycast_plane_intersect(vec4_t normal, vec4_t a, f32 input_x, f32 input_y, camera_object_t *camera);
