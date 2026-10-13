
#include "../global.h"

#define PATH_TOOL_SIDES       12
#define PATH_TOOL_MIN_DIST    0.0001
#define PATH_TOOL_MIN_RADIUS  0.001
#define PATH_TOOL_CURVE_STEPS 16
#define PATH_TOOL_SMOOTH_DOT  0.8 // Joints of a curve bending less than this are shaded smooth

static vec4_t        *_path_tool_points   = NULL; // World space, placed on the ground plane
static i32            _path_tool_count    = 0;
static i32            _path_tool_capacity = 0;
static vec4_t        *_path_tool_samples  = NULL;  // Curve tool - points along the beziers
static i32            _path_tool_tool     = -1;    // Tool the points were placed with
static mesh_object_t *_path_tool_object   = NULL;  // Created with the first segment, or the selected mesh being edited
static bool           _path_tool_open     = false; // Clicks add points
static i32            _path_tool_dragging = -1;
static any_imap_t    *_path_tool_datas    = NULL; // Object uid -> f32_array_t, curve flag, radius and the points in the object space
static f32            _path_tool_radius;          // Radius the mesh was built with
static f32            _path_tool_right_x;
static f32            _path_tool_right_y;

static void _path_tool_vertex(mesh_geo_t *g, i32 i, vec4_t pos, vec4_t nor, f32 u, f32 v) {
	g->pos[i * 3]     = pos.x;
	g->pos[i * 3 + 1] = pos.y;
	g->pos[i * 3 + 2] = pos.z;
	g->nor[i * 3]     = nor.x;
	g->nor[i * 3 + 1] = nor.y;
	g->nor[i * 3 + 2] = nor.z;
	g->tex[i * 2]     = u;
	g->tex[i * 2 + 1] = v;
}

// Segments stay separate where the path turns back
static bool _path_tool_mitered(vec4_t dir, vec4_t neighbor_dir) {
	return vec4_len(vec4_add(dir, neighbor_dir)) >= 0.5;
}

// Shifts a ring point along the segment onto the plane shared with the neighbor segment
static vec4_t _path_tool_miter(vec4_t offset, vec4_t dir, vec4_t neighbor_dir) {
	if (!_path_tool_mitered(dir, neighbor_dir)) {
		return offset;
	}
	vec4_t m = vec4_norm(vec4_add(dir, neighbor_dir));
	return vec4_sub(offset, vec4_mult(dir, vec4_dot(offset, m) / vec4_dot(dir, m)));
}

static void _path_tool_cap(mesh_geo_t *g, i32 *vi, i32 *ii, vec4_t center, vec4_t n, vec4_t b, vec4_t nor, f32 radius, bool flip) {
	i32 sides = PATH_TOOL_SIDES;
	i32 base  = *vi;
	_path_tool_vertex(g, base, center, nor, 0.5, 0.5);
	for (i32 k = 0; k < sides; ++k) {
		f32    a = k / (f32)sides * math_pi() * 2.0;
		vec4_t o = vec4_add(vec4_mult(n, math_cos(a) * radius), vec4_mult(b, math_sin(a) * radius));
		_path_tool_vertex(g, base + 1 + k, vec4_add(center, o), nor, 0.5 + math_cos(a) * 0.5, 0.5 + math_sin(a) * 0.5);
	}
	for (i32 k = 0; k < sides; ++k) {
		i32 k1          = (k + 1) % sides;
		g->ind[(*ii)++] = base;
		g->ind[(*ii)++] = base + 1 + (flip ? k1 : k);
		g->ind[(*ii)++] = base + 1 + (flip ? k : k1);
	}
	*vi += sides + 1;
}

// Normal shared by the ring points of both segments meeting at a joint
static vec4_t _path_tool_joint_normal(vec4_t nor, vec4_t dir, vec4_t neighbor_dir) {
	if (vec4_dot(dir, neighbor_dir) < PATH_TOOL_SMOOTH_DOT) {
		return nor;
	}
	vec4_t m = vec4_norm(vec4_add(dir, neighbor_dir));
	return vec4_norm(vec4_sub(nor, vec4_mult(m, vec4_dot(nor, m))));
}

static bool _path_tool_curved() {
	return _path_tool_tool == TOOL_TYPE_CURVE;
}

// Curves start at an anchor and take a control point and an anchor each
static bool _path_tool_has_mesh() {
	return _path_tool_curved() ? _path_tool_count > 2 : _path_tool_count > 1;
}

static vec4_t _path_tool_bezier(vec4_t p0, vec4_t pc, vec4_t p1, f32 t) {
	f32 u = 1.0 - t;
	return vec4_add(vec4_add(vec4_mult(p0, u * u), vec4_mult(pc, 2.0 * u * t)), vec4_mult(p1, t * t));
}

// Fills _path_tool_samples, a trailing control point has no curve yet
static i32 _path_tool_sample_curves() {
	i32 curves                  = (_path_tool_count - 1) / 2;
	_path_tool_samples          = realloc(_path_tool_samples, (curves * PATH_TOOL_CURVE_STEPS + 1) * sizeof(vec4_t));
	i32 count                   = 0;
	_path_tool_samples[count++] = _path_tool_points[0];
	for (i32 j = 2; j < _path_tool_count; j += 2) {
		for (i32 k = 1; k <= PATH_TOOL_CURVE_STEPS; ++k) {
			vec4_t p = _path_tool_bezier(_path_tool_points[j - 2], _path_tool_points[j - 1], _path_tool_points[j], k / (f32)PATH_TOOL_CURVE_STEPS);
			if (vec4_dist(p, _path_tool_samples[count - 1]) >= PATH_TOOL_MIN_DIST) {
				_path_tool_samples[count++] = p;
			}
		}
	}
	return count;
}

// Round tube with mitered joints and capped ends
static mesh_geo_t *_path_tool_make_geo(vec4_t *points, i32 count, f32 radius, bool smooth) {
	i32         sides    = PATH_TOOL_SIDES;
	i32         segments = count - 1;
	mesh_geo_t *g        = util_geo_create(segments * (sides + 1) * 4, segments * sides * 12);
	i32         vi       = 0;
	i32         ii       = 0;

	f32 total = 0.0;
	for (i32 s = 0; s < segments; ++s) {
		total += vec4_dist(points[s], points[s + 1]);
	}

	vec4_t n;
	vec4_t b;
	vec4_t prev_dir;
	f32    dist = 0.0;
	for (i32 s = 0; s < segments; ++s) {
		vec4_t p0  = points[s];
		vec4_t p1  = points[s + 1];
		vec4_t dir = vec4_norm(vec4_sub(p1, p0));
		if (s == 0) {
			vec4_t up = math_abs(dir.z) > 0.99 ? vec4_x_axis() : vec4_z_axis();
			n         = vec4_norm(vec4_cross(up, dir));
		}
		else {
			// Mirror the frame over the joint, ring points of both segments then meet
			if (_path_tool_mitered(prev_dir, dir)) {
				vec4_t m = vec4_norm(vec4_add(prev_dir, dir));
				n        = vec4_sub(n, vec4_mult(m, 2.0 * vec4_dot(n, m)));
			}
			n = vec4_norm(vec4_sub(n, vec4_mult(dir, vec4_dot(n, dir))));
		}
		b               = vec4_cross(dir, n);
		vec4_t next_dir = s < segments - 1 ? vec4_norm(vec4_sub(points[s + 2], points[s + 1])) : dir;

		if (s == 0 || !_path_tool_mitered(dir, prev_dir)) {
			_path_tool_cap(g, &vi, &ii, p0, n, b, vec4_mult(dir, -1.0), radius, true);
		}

		f32 len  = vec4_dist(p0, p1);
		i32 base = vi;
		for (i32 k = 0; k <= sides; ++k) {
			f32    a   = k / (f32)sides * math_pi() * 2.0;
			vec4_t nor = vec4_add(vec4_mult(n, math_cos(a)), vec4_mult(b, math_sin(a)));
			vec4_t o0  = vec4_mult(nor, radius);
			vec4_t o1  = o0;
			if (s > 0) {
				o0 = _path_tool_miter(o0, dir, prev_dir);
			}
			if (s < segments - 1) {
				o1 = _path_tool_miter(o1, dir, next_dir);
			}
			vec4_t nor0 = smooth && s > 0 ? _path_tool_joint_normal(nor, dir, prev_dir) : nor;
			vec4_t nor1 = smooth && s < segments - 1 ? _path_tool_joint_normal(nor, dir, next_dir) : nor;
			_path_tool_vertex(g, vi++, vec4_add(p0, o0), nor0, k / (f32)sides, dist / total);
			_path_tool_vertex(g, vi++, vec4_add(p1, o1), nor1, k / (f32)sides, (dist + len) / total);
		}
		for (i32 k = 0; k < sides; ++k) {
			i32 v00      = base + k * 2;
			i32 v01      = v00 + 1;
			i32 v10      = v00 + 2;
			i32 v11      = v00 + 3;
			g->ind[ii++] = v00;
			g->ind[ii++] = v10;
			g->ind[ii++] = v11;
			g->ind[ii++] = v00;
			g->ind[ii++] = v11;
			g->ind[ii++] = v01;
		}

		if (s == segments - 1 || !_path_tool_mitered(dir, next_dir)) {
			_path_tool_cap(g, &vi, &ii, p1, n, b, dir, radius, false);
		}
		dist += len;
		prev_dir = dir;
	}
	g->vertex_count = vi;
	g->index_count  = ii;
	return g;
}

static f32_array_t *_path_tool_data(mesh_object_t *o) {
	return _path_tool_datas != NULL ? any_imap_get(_path_tool_datas, o->base->uid) : NULL;
}

static void _path_tool_set_data(mesh_object_t *o, f32_array_t *data) {
	if (_path_tool_datas == NULL) {
		_path_tool_datas = any_imap_create();
	}
	any_imap_set(_path_tool_datas, o->base->uid, data);
}

static void _path_tool_reserve(i32 count) {
	if (count > _path_tool_capacity) {
		_path_tool_capacity = math_max(count, math_max(_path_tool_capacity * 2, 16));
		_path_tool_points   = realloc(_path_tool_points, _path_tool_capacity * sizeof(vec4_t));
	}
}

// Keeps the points with the mesh
static void _path_tool_store() {
	f32_array_t *data = _path_tool_data(_path_tool_object);
	if (data == NULL || data->length != 2 + _path_tool_count * 3) {
		data = f32_array_create(2 + _path_tool_count * 3);
		_path_tool_set_data(_path_tool_object, data);
	}
	mat4_t inv      = mat4_inv(_path_tool_object->base->transform->world);
	data->buffer[0] = _path_tool_curved() ? 1.0 : 0.0;
	data->buffer[1] = _path_tool_radius;
	for (i32 i = 0; i < _path_tool_count; ++i) {
		vec4_t p                = _path_tool_points[i];
		p.w                     = 1.0;
		p                       = vec4_apply_mat4(p, inv);
		data->buffer[2 + i * 3] = p.x;
		data->buffer[3 + i * 3] = p.y;
		data->buffer[4 + i * 3] = p.z;
	}
}

static void _path_tool_rebuild() {
	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use) {
		draw_end();
	}

	// The object may have been deleted in the meantime
	if (_path_tool_object != NULL && array_index_of(g_project->_->paint_objects, _path_tool_object) < 0) {
		_path_tool_object = NULL;
	}

	_path_tool_radius = util_path_tool_radius;
	f32     radius    = math_max(_path_tool_radius, PATH_TOOL_MIN_RADIUS); // Typed values are not clamped
	bool    curved    = _path_tool_curved();
	i32     count     = curved ? _path_tool_sample_curves() : _path_tool_count; // Reallocates the samples
	vec4_t *world     = curved ? _path_tool_samples : _path_tool_points;
	// The mesh is built in the object space, a new object sits at the first point
	vec4_t *local = malloc(count * sizeof(vec4_t));
	mat4_t  inv   = _path_tool_object != NULL ? mat4_inv(_path_tool_object->base->transform->world) : mat4_identity();
	for (i32 i = 0; i < count; ++i) {
		vec4_t p = world[i];
		p.w      = 1.0;
		local[i] = _path_tool_object != NULL ? vec4_apply_mat4(p, inv) : vec4_sub(p, _path_tool_points[0]);
	}
	mesh_geo_t  *g  = _path_tool_make_geo(local, count, radius, curved);
	mesh_data_t *md = nodes_mesh_geo_to_data(g, curved ? "Curve" : "Path");
	util_geo_free(g);
	free(local);

	if (_path_tool_object == NULL) {
		_path_tool_object                       = tab_meshes_append_data(md, NULL);
		_path_tool_object->base->transform->loc = _path_tool_points[0];
		transform_build_matrix(_path_tool_object->base->transform);
		util_mesh_transform_changed();
	}
	else {
		mesh_data_t *old = _path_tool_object->data;
		if (!util_mesh_data_is_shared(old) && !nodes_mesh_holds(old)) {
			sys_notify_on_next_frame(&util_mesh_delete_data_uncache, old);
		}
		mesh_object_set_data(_path_tool_object, md);
		transform_build_matrix(_path_tool_object->base->transform);
		util_mesh_merge(NULL);
		nodes_mesh_request_refresh();
		render_path_raytrace_ready = false;
	}
	_path_tool_store();
	tab_meshes_reset_preview_map();

	if (in_use) {
		draw_begin(current, false, 0);
	}

	ui_dock_redraw(TAB_AREA_SIDEBAR0);
	ui_dock_redraw(TAB_AREA_SIDEBAR1);
	ui_header_handle->redraws = 2;
	g_context->ddirty         = 2;
}

static void _path_tool_end() {
	_path_tool_count    = 0;
	_path_tool_object   = NULL;
	_path_tool_open     = false;
	_path_tool_dragging = -1;
}

static f32 _path_tool_sphere_scale(vec4_t p) {
	f32 dist = vec4_dist(scene_camera->base->transform->loc, p);
	return dist / 8.0 * scene_camera->data->fov * 0.1;
}

// Points of the path being drawn or edited, drawn as spheres by render_pathsphere_tool
vec4_t *util_path_tool_get_points(i32 *count, i32 *active) {
	bool tool = (g_context->tool == TOOL_TYPE_PATH || g_context->tool == TOOL_TYPE_CURVE) && !base_player;
	*count    = tool && g_context->tool == _path_tool_tool ? _path_tool_count : 0;
	*active   = _path_tool_dragging;
	return _path_tool_points;
}

// Picks up the points of the selected mesh when no path is being drawn
static void _path_tool_sync() {
	mesh_object_t *po = g_context->paint_object;
	if (_path_tool_open) {
		if (_path_tool_object == NULL || _path_tool_object == po) {
			return;
		}
		// Another mesh was selected
		_path_tool_end();
	}
	if (_path_tool_dragging >= 0) {
		return;
	}
	bool         selected = po != NULL && !context_object_deselected() && array_index_of(g_project->_->paint_objects, po) >= 0;
	f32_array_t *data     = selected ? _path_tool_data(po) : NULL;
	if (data == NULL || (data->buffer[0] != 0.0) != _path_tool_curved()) {
		_path_tool_count  = 0;
		_path_tool_object = NULL;
		return;
	}
	if (_path_tool_object != po) {
		_path_tool_radius         = data->buffer[1];
		util_path_tool_radius     = _path_tool_radius;
		ui_header_handle->redraws = 2;
	}
	_path_tool_object = po;
	_path_tool_count  = (data->length - 2) / 3;
	_path_tool_reserve(_path_tool_count);
	for (i32 i = 0; i < _path_tool_count; ++i) {
		vec4_t p             = (vec4_t){data->buffer[2 + i * 3], data->buffer[3 + i * 3], data->buffer[4 + i * 3], 1.0};
		_path_tool_points[i] = vec4_apply_mat4(p, po->base->transform->world);
	}
}

// Point under the mouse
static i32 _path_tool_pick() {
	ray_t *ray = raycast_get_ray(mouse_view_x(), mouse_view_y(), scene_camera);
	if (ray == NULL) {
		return -1;
	}
	i32 closest  = -1;
	f32 min_dist = 0.0;
	for (i32 i = 0; i < _path_tool_count; ++i) {
		vec4_t p      = _path_tool_points[i];
		f32    dist   = vec4_dist(scene_camera->base->transform->loc, p);
		f32    radius = _path_tool_sphere_scale(p) * 2.5;
		if (ray_intersects_sphere(ray, p, radius) && (closest < 0 || dist < min_dist)) {
			min_dist = dist;
			closest  = i;
		}
	}
	free(ray);
	return closest;
}

static void _path_tool_drag() {
	if (!mouse_down("left")) {
		_path_tool_dragging = -1;
		return;
	}
	vec4_t from = _path_tool_points[_path_tool_dragging];
	vec4_t p;
	if (!util_tool_plane_point_at(from, &p) || vec4_dist(p, from) < PATH_TOOL_MIN_DIST) {
		return;
	}
	if (keyboard_down("control")) {
		// Move all points
		vec4_t d = vec4_sub(p, from);
		for (i32 i = 0; i < _path_tool_count; ++i) {
			_path_tool_points[i] = vec4_add(_path_tool_points[i], d);
		}
	}
	_path_tool_points[_path_tool_dragging] = p;
	if (_path_tool_has_mesh()) {
		_path_tool_rebuild();
	}
	g_context->ddirty = 2;
}

static void _path_tool_input() {
	// Right click ends the path, right drag rotates the camera
	if (mouse_started("right")) {
		_path_tool_right_x = mouse_view_x();
		_path_tool_right_y = mouse_view_y();
	}
	if (mouse_released("right") && math_abs(mouse_view_x() - _path_tool_right_x) < 4 && math_abs(mouse_view_y() - _path_tool_right_y) < 4) {
		_path_tool_open = false;
		if (!_path_tool_has_mesh()) {
			_path_tool_end();
		}
		return;
	}

	_path_tool_sync();

	if (_path_tool_dragging >= 0) {
		_path_tool_drag();
		return;
	}

	if (_path_tool_has_mesh() && _path_tool_object != NULL && _path_tool_radius != util_path_tool_radius && !mouse_down("left")) {
		_path_tool_rebuild();
	}

	if (!util_tool_input_enabled() || !mouse_started("left")) {
		return;
	}
	_path_tool_dragging = _path_tool_pick();
	if (_path_tool_dragging >= 0) {
		return;
	}
	vec4_t p;
	if (!util_tool_plane_point(&p)) {
		return;
	}
	if (!_path_tool_open) {
		// Start a new path, the edited one is done
		_path_tool_end();
		_path_tool_open = true;
	}
	if (_path_tool_count > 0 && vec4_dist(p, _path_tool_points[_path_tool_count - 1]) < PATH_TOOL_MIN_DIST) {
		return;
	}
	_path_tool_reserve(_path_tool_count + 1);
	_path_tool_points[_path_tool_count++] = p;
	// A control point adds no curve until its anchor is placed
	if (_path_tool_has_mesh() && !(_path_tool_curved() && _path_tool_count % 2 == 0)) {
		_path_tool_rebuild();
	}
}

void util_path_tool_update() {
	bool active = (g_context->tool == TOOL_TYPE_PATH || g_context->tool == TOOL_TYPE_CURVE) && !base_player;
	if (!active || g_context->tool != _path_tool_tool || keyboard_started("escape")) {
		_path_tool_end();
		_path_tool_tool = g_context->tool;
		if (!active) {
			return;
		}
	}
	_path_tool_input();
}

void util_path_tool_reset() {
	_path_tool_datas = NULL;
	_path_tool_end();
}

void util_path_tool_copy(mesh_object_t *from, mesh_object_t *to) {
	f32_array_t *data = _path_tool_data(from);
	if (data != NULL) {
		_path_tool_set_data(to, f32_array_create_from_array(data));
	}
}

// Fills project mesh_paths, one array for each mesh, empty when it is not a path
void util_path_tool_export() {
	g_project->mesh_paths      = NULL;
	f32_array_t_array_t *paths = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
		f32_array_t *data = _path_tool_data(g_project->_->paint_objects->buffer[i]);
		if (data != NULL) {
			g_project->mesh_paths = paths;
		}
		any_array_push(paths, data != NULL ? data : f32_array_create(0));
	}
}

void util_path_tool_import() {
	util_path_tool_reset();
	f32_array_t_array_t *paths = g_project->mesh_paths;
	if (paths == NULL) {
		return;
	}
	for (i32 i = 0; i < paths->length && i < g_project->_->paint_objects->length; ++i) {
		if (paths->buffer[i] != NULL && paths->buffer[i]->length >= 2) {
			// Decoded arrays live in the project blob
			_path_tool_set_data(g_project->_->paint_objects->buffer[i], f32_array_create_from_array(paths->buffer[i]));
		}
	}
}

// Line or curve from the last point to the mouse and the control lines, called while drawing the cursor
void util_path_tool_render() {
	if (g_context->tool != _path_tool_tool || _path_tool_count == 0) {
		return;
	}

	draw_scissor(base_x(), base_y(), base_w(), base_h());
	draw_set_color(0xffffffff);
	bool   curved  = _path_tool_curved();
	vec4_t last    = _path_tool_points[_path_tool_count - 1];
	bool   control = curved && _path_tool_count % 2 == 0; // The next point is an anchor ending a curve
	vec4_t p;
	if (_path_tool_open && _path_tool_dragging < 0 && util_tool_input_enabled() && util_tool_plane_point(&p)) {
		if (control) {
			vec4_t prev = _path_tool_points[_path_tool_count - 2];
			vec4_t a    = prev;
			for (i32 k = 1; k <= PATH_TOOL_CURVE_STEPS; ++k) {
				vec4_t b = _path_tool_bezier(prev, last, p, k / (f32)PATH_TOOL_CURVE_STEPS);
				util_tool_draw_line(a, b);
				a = b;
			}
		}
		else {
			util_tool_draw_line(last, p);
		}
	}
	// The path stays visible no matter how thin the tube is
	if (curved) {
		i32 count = _path_tool_sample_curves();
		for (i32 i = 1; i < count; ++i) {
			util_tool_draw_line(_path_tool_samples[i - 1], _path_tool_samples[i]);
		}
		// Control points are off the curve
		for (i32 i = 1; i < _path_tool_count; ++i) {
			util_tool_draw_line(_path_tool_points[i - 1], _path_tool_points[i]);
		}
	}
	else {
		for (i32 i = 1; i < _path_tool_count; ++i) {
			util_tool_draw_line(_path_tool_points[i - 1], _path_tool_points[i]);
		}
	}
	draw_flush();
	gpu_disable_scissor();
}
