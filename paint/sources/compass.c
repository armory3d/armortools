
#include "global.h"

void compass_render() {
	if (!context_raw->show_compass || config_raw->workspace == WORKSPACE_PLAYER) {
		return;
	}

	camera_object_t *cam      = scene_camera;
	mesh_object_t   *compass  = scene_get_child(".Compass")->ext;

	bool             _visible = compass->base->visible;
	object_t        *_parent  = compass->base->parent;
	quat_t           crot     = cam->base->transform->rot;
	f32              ratio    = sys_w() / (float)sys_h();
	mat4_t           _P       = cam->p;
	cam->p                    = mat4_ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
	compass->base->visible    = true;
	compass->base->parent     = cam->base;

	f32  compass_x            = 7.4;
	f32  compass_y            = 7.0;
	bool compass_down         = false;

	#ifdef IRON_IOS
	if (config_is_iphone()) {
		compass_down = true;
	}
	#endif

	if (compass_down) {
		compass_x = 6.0;
		compass_y = -compass_y;
	}

	compass->base->transform->loc   = vec4_create(compass_x * ratio, compass_y, -1, 1.0);
	compass->base->transform->rot   = quat_create(-crot.x, -crot.y, -crot.z, crot.w);
	compass->base->transform->scale = vec4_create(0.4, 0.4, 0.4, 1.0);
	transform_build_matrix(compass->base->transform);
	compass->frustum_culling = false;
	mesh_object_render(compass, "overlay", NULL);

	if (_compass_hovered != NULL) {
		line_draw_color    = _compass_hovered == _compass_hitbox_x ? 0xffff0000 : _compass_hovered == _compass_hitbox_y ? 0xff00ff00 : 0xff0000ff;
		line_draw_strength = 0.1;
		shape_draw_sphere(_compass_hovered->transform->world);
	}

	cam->p                 = _P;
	compass->base->visible = _visible;
	compass->base->parent  = _parent;
}

void compass_init_hitbox() {
	if (_compass_hitbox_x != NULL) {
		return;
	}

	gc_unroot(_compass_hitbox_x);
	_compass_hitbox_x = object_create(true);
	gc_root(_compass_hitbox_x);
	gc_unroot(_compass_hitbox_y);
	_compass_hitbox_y = object_create(true);
	gc_root(_compass_hitbox_y);
	gc_unroot(_compass_hitbox_z);
	_compass_hitbox_z = object_create(true);
	gc_root(_compass_hitbox_z);

	_compass_hitbox_x->transform->scale = vec4_create(0.15, 0.15, 0.15, 1.0);
	_compass_hitbox_y->transform->scale = vec4_create(0.15, 0.15, 0.15, 1.0);
	_compass_hitbox_z->transform->scale = vec4_create(0.15, 0.15, 0.15, 1.0);

	_compass_hitbox_x->transform->loc   = vec4_create(1.5, 0, 0, 1.0);
	_compass_hitbox_y->transform->loc   = vec4_create(0, 1.5, 0, 1.0);
	_compass_hitbox_z->transform->loc   = vec4_create(0, 0, 1.5, 1.0);

	object_t *compass                   = scene_get_child(".Compass");
	object_set_parent(_compass_hitbox_x, compass);
	object_set_parent(_compass_hitbox_y, compass);
	object_set_parent(_compass_hitbox_z, compass);
}

bool _compass_compare_quat(quat_t a, quat_t b) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

void compass_update() {
	if (!context_raw->show_compass) {
		return;
	}

	if (_compass_hovered_last != _compass_hovered) {
		gc_unroot(_compass_hovered_last);
		_compass_hovered_last = _compass_hovered;
		gc_root(_compass_hovered_last);
		context_raw->ddirty = 2;
	}
	gc_unroot(_compass_hovered);
	_compass_hovered = NULL;

	f32  x           = mouse_view_x() / (float)sys_w();
	f32  y           = mouse_view_y() / (float)sys_h();
	bool hover       = x > 0.9 && x < 1.0 && y < 0.14 && y > 0.0;
	if (hover) {

		compass_init_hitbox();

		f32    ratio    = sys_w() / (float)sys_h();
		mat4_t _P       = scene_camera->p;
		scene_camera->p = mat4_ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);

		transform_build_matrix(_compass_hitbox_x->transform);
		transform_build_matrix(_compass_hitbox_y->transform);
		transform_build_matrix(_compass_hitbox_z->transform);

		transform_t_array_t *ts = any_array_create_from_raw(
		    (void *[]){
		        _compass_hitbox_x->transform,
		        _compass_hitbox_y->transform,
		        _compass_hitbox_z->transform,
		    },
		    3);

		transform_t *t = raycast_closest_box_intersect(ts, mouse_view_x(), mouse_view_y(), scene_camera);
		if (t != NULL) {
			quat_t cq = scene_camera->base->transform->rot;

			if (t == _compass_hitbox_x->transform) {
				if (mouse_started("left")) {
					// Flip between left / right
					_compass_compare_quat(quat_from_euler(math_pi() / 2.0, 0, math_pi() / 2.0), cq)
					    ? viewport_set_view(-1, 0, 0, math_pi() / 2.0, 0, -math_pi() / 2.0) // Left
					    : viewport_set_view(1, 0, 0, math_pi() / 2.0, 0, math_pi() / 2.0); // Right
				}
				gc_unroot(_compass_hovered);
				_compass_hovered = _compass_hitbox_x;
				gc_root(_compass_hovered);
			}

			else if (t == _compass_hitbox_y->transform) {
				if (mouse_started("left")) {
					_compass_compare_quat(quat_from_euler(math_pi() / 2.0, 0, math_pi()), cq)
					    ? viewport_set_view(0, -1, 0, math_pi() / 2.0, 0, 0) // Front
					    : viewport_set_view(0, 1, 0, math_pi() / 2.0, 0, math_pi()); // Back
				}
				gc_unroot(_compass_hovered);
				_compass_hovered = _compass_hitbox_y;
				gc_root(_compass_hovered);
			}

			else {
				if (mouse_started("left")) {
					_compass_compare_quat(quat_from_euler(0, 0, 0), cq) ? viewport_set_view(0, 0, -1, math_pi(), 0, math_pi()) // Bottom
					                                                    : viewport_set_view(0, 0, 1, 0, 0, 0); // Top
				}
				gc_unroot(_compass_hovered);
				_compass_hovered = _compass_hitbox_z;
				gc_root(_compass_hovered);
			}
		}
		scene_camera->p = _P;
	}
}
