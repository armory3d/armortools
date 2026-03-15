
#include "global.h"

void camera_init() {
	camera_reset(-1);
}

void camera_update(void * _) {
	camera_object_t *camera = scene_camera;

	if (mouse_view_x() < 0 || mouse_view_x() > sys_w() || mouse_view_y() < 0 || mouse_view_y() > sys_h()) {

		if (config_raw->wrap_mouse && camera_controls_down) {
			if (mouse_view_x() < 0) {
				mouse_x = mouse_last_x = sys_x() + sys_w();
				iron_mouse_set_position(math_floor(mouse_x), math_floor(mouse_y));
			}
			else if (mouse_view_x() > sys_w()) {
				mouse_x = mouse_last_x = sys_x();
				iron_mouse_set_position(math_floor(mouse_x), math_floor(mouse_y));
			}
			else if (mouse_view_y() < 0) {
				mouse_y = mouse_last_y = sys_y() + sys_h();
				iron_mouse_set_position(math_floor(mouse_x), math_floor(mouse_y));
			}
			else if (mouse_view_y() > sys_h()) {
				mouse_y = mouse_last_y = sys_y();
				iron_mouse_set_position(math_floor(mouse_x), math_floor(mouse_y));
			}
		}
		else {
			return;
		}
	}

	bool modif_key      = keyboard_down("alt") || keyboard_down("shift") || keyboard_down("control");
	bool modif          = modif_key || string_equals(any_map_get(config_keymap, "action_rotate"), "middle");
	bool default_keymap = string_equals(config_raw->keymap, "default.json");

	if (operator_shortcut(any_map_get(config_keymap, "action_rotate"), SHORTCUT_TYPE_STARTED) ||
	    operator_shortcut(any_map_get(config_keymap, "action_zoom"), SHORTCUT_TYPE_STARTED) ||
	    operator_shortcut(any_map_get(config_keymap, "action_pan"), SHORTCUT_TYPE_STARTED) ||
	    operator_shortcut(any_map_get(config_keymap, "rotate_envmap"), SHORTCUT_TYPE_STARTED) || (mouse_started("right") && !modif) ||
	    (mouse_started("middle") && !modif) || (mouse_wheel_delta != 0 && !modif_key)) {
		camera_controls_down = true;
	}
	else if (!operator_shortcut(any_map_get(config_keymap, "action_rotate"), SHORTCUT_TYPE_DOWN) &&
	         !operator_shortcut(any_map_get(config_keymap, "action_zoom"), SHORTCUT_TYPE_DOWN) &&
	         !operator_shortcut(any_map_get(config_keymap, "action_pan"), SHORTCUT_TYPE_DOWN) &&
	         !operator_shortcut(any_map_get(config_keymap, "rotate_envmap"), SHORTCUT_TYPE_DOWN) && !(mouse_down("right") && !modif) &&
	         !(mouse_down("middle") && !modif) && (mouse_wheel_delta == 0 && !modif_key)) {
		camera_controls_down = false;
	}

	if (_input_occupied || !base_ui_enabled || base_is_dragging || ui->is_scrolling || ui->combo_selected_handle != NULL || !camera_controls_down) {
		return;
	}

	camera_controls_t controls = context_raw->camera_controls;
	if (controls == CAMERA_CONTROLS_ORBIT &&
	    (operator_shortcut(any_map_get(config_keymap, "action_rotate"), SHORTCUT_TYPE_DOWN) || (mouse_down("right") && !modif && default_keymap))) {
		camera_redraws = 2;
		f32 dist       = camera_distance();
		transform_move(camera->base->transform, camera_object_look_world(camera), dist);
		transform_rotate(camera->base->transform, vec4_z_axis(), -mouse_movement_x / 100.0 * config_raw->camera_rotation_speed);
		transform_rotate(camera->base->transform, camera_object_right_world(camera), -mouse_movement_y / 100.0 * config_raw->camera_rotation_speed);
		vec4_t up_world = camera_object_up_world(camera);
		if (up_world.z < 0 && !config_raw->camera_upside_down) {
			transform_rotate(camera->base->transform, camera_object_right_world(camera), mouse_movement_y / 100.0 * config_raw->camera_rotation_speed);
		}
		transform_move(camera->base->transform, camera_object_look_world(camera), -dist);
	}
	else if (controls == CAMERA_CONTROLS_ROTATE &&
	         (operator_shortcut(any_map_get(config_keymap, "action_rotate"), SHORTCUT_TYPE_DOWN) || (mouse_down("right") && !modif && default_keymap))) {
		camera_redraws  = 2;
		transform_t *t  = context_main_object()->base->transform;
		vec4_t       up = transform_up(t);
		transform_rotate(t, up, (mouse_movement_x / 120.0) * config_raw->camera_rotation_speed);
		vec4_t right = camera_object_right_world(camera);
		transform_rotate(t, right, (mouse_movement_y / 120.0) * config_raw->camera_rotation_speed);
		transform_build_matrix(t);
		vec4_t tup = transform_up(t);
		if (tup.z < 0 && !config_raw->camera_upside_down) {
			transform_rotate(t, right, -(mouse_movement_y / 120.0) * config_raw->camera_rotation_speed);
		}
	}

	if (controls == CAMERA_CONTROLS_ROTATE || controls == CAMERA_CONTROLS_ORBIT) {
		camera_pan_action(modif, default_keymap);

		if (operator_shortcut(any_map_get(config_keymap, "action_zoom"), SHORTCUT_TYPE_DOWN)) {
			camera_redraws = 2;
			f32 f          = camera_get_zoom_delta() / (float)(150.0 * (1.0 / (camera_distance() / 2.0)));
			f *= camera_get_zoom_speed();
			transform_move(camera->base->transform, camera_object_look(camera), f);
		}

		if (mouse_wheel_delta != 0 && !modif_key) {
			camera_redraws = 2;
			f32 f          = mouse_wheel_delta * (-0.2) * ((camera_distance() / 4.0));
			f *= camera_get_zoom_speed();
			transform_move(camera->base->transform, camera_object_look(camera), f);
		}
	}
	else if (controls == CAMERA_CONTROLS_FLY && mouse_down("right")) {
		bool move_forward  = keyboard_down("w") || keyboard_down("up") || mouse_wheel_delta < 0;
		bool move_backward = keyboard_down("s") || keyboard_down("down") || mouse_wheel_delta > 0;
		bool strafe_left   = keyboard_down("a") || keyboard_down("left");
		bool strafe_right  = keyboard_down("d") || keyboard_down("right");
		bool strafe_up     = keyboard_down("e");
		bool strafe_down   = keyboard_down("q");
		f32  fast          = keyboard_down("shift") ? 2.0 : (keyboard_down("alt") ? 0.5 : 1.0);
		if (mouse_wheel_delta != 0) {
			fast *= math_abs(mouse_wheel_delta) * 4.0;
		}

		if (move_forward || move_backward || strafe_right || strafe_left || strafe_up || strafe_down) {
			camera_ease += sys_delta() * 15;
			if (camera_ease > 1.0) {
				camera_ease = 1.0;
			}
			camera_dir   = vec4_create(0, 0, 0, 1.0);
			vec4_t look  = camera_object_look(camera);
			vec4_t right = camera_object_right(camera);
			if (move_forward) {
				camera_dir = vec4_fadd(camera_dir, look.x, look.y, look.z, 0.0);
			}
			if (move_backward) {
				camera_dir = vec4_fadd(camera_dir, -look.x, -look.y, -look.z, 0.0);
			}
			if (strafe_right) {
				camera_dir = vec4_fadd(camera_dir, right.x, right.y, right.z, 0.0);
			}
			if (strafe_left) {
				camera_dir = vec4_fadd(camera_dir, -right.x, -right.y, -right.z, 0.0);
			}
			if (strafe_up) {
				camera_dir = vec4_fadd(camera_dir, 0, 0, 1, 0.0);
			}
			if (strafe_down) {
				camera_dir = vec4_fadd(camera_dir, 0, 0, -1, 0.0);
			}
		}
		else {
			camera_ease -= sys_delta() * 20.0 * camera_ease;
			if (camera_ease < 0.0) {
				camera_ease = 0.0;
			}
		}

		f32 d = sys_delta() * fast * camera_ease * 2.0 * ((move_forward || move_backward) ? config_raw->camera_zoom_speed : config_raw->camera_pan_speed);
		if (d > 0.0) {
			transform_move(camera->base->transform, camera_dir, d);
			if (context_raw->camera_type == CAMERA_TYPE_ORTHOGRAPHIC) {
				viewport_update_camera_type(context_raw->camera_type);
			}
		}

		camera_redraws = 2;
		transform_rotate(camera->base->transform, vec4_z_axis(), -mouse_movement_x / 200.0 * config_raw->camera_rotation_speed);
		transform_rotate(camera->base->transform, camera_object_right(camera), -mouse_movement_y / 200.0 * config_raw->camera_rotation_speed);
	}

	if (operator_shortcut(any_map_get(config_keymap, "view_pivot_center"), SHORTCUT_TYPE_STARTED)) {
		camera_set_pivot_center_to_mouse();
	}

	if (operator_shortcut(any_map_get(config_keymap, "rotate_envmap"), SHORTCUT_TYPE_DOWN)) {
		camera_redraws = 2;
		context_raw->envmap_angle -= mouse_movement_x / 100.0;
	}

	if (camera_redraws > 0) {
		camera_redraws--;
		context_raw->ddirty = 2;

		if (context_raw->camera_type == CAMERA_TYPE_ORTHOGRAPHIC) {
			viewport_update_camera_type(context_raw->camera_type);
		}
	}
}

f32 camera_distance() {
	camera_object_t *camera = scene_camera;
	return vec4_dist(camera_origins->buffer[camera_index()]->v, camera->base->transform->loc);
}

i32 camera_index() {
	return context_raw->view_index_last > 0 ? 1 : 0;
}

f32 camera_get_zoom_speed() {
	i32 sign = config_raw->zoom_direction == ZOOM_DIRECTION_VERTICAL_INVERTED || config_raw->zoom_direction == ZOOM_DIRECTION_HORIZONTAL_INVERTED ||
	                   config_raw->zoom_direction == ZOOM_DIRECTION_VERTICAL_HORIZONTAL_INVERTED
	               ? -1
	               : 1;
	camera_object_t *camera     = scene_camera;
	f32              fov_adjust = camera->data->fov;
	return (config_raw->camera_zoom_speed * sign) / (float)fov_adjust;
}

void camera_reset(i32 view_index) {
	camera_object_t *camera = scene_camera;
	if (view_index == -1) {
		vec4_box_t *v0 = GC_ALLOC_INIT(vec4_box_t, {.v = vec4_create(0, 0, 0, 1)});
		vec4_box_t *v1 = GC_ALLOC_INIT(vec4_box_t, {.v = vec4_create(0, 0, 0, 1)});
		gc_unroot(camera_origins);
		camera_origins = any_array_create_from_raw(
		    (void *[]){
		        v0,
		        v1,
		    },
		    2);
		gc_root(camera_origins);
		mat4_box_t *m0 = GC_ALLOC_INIT(mat4_box_t, {.v = mat4_clone(camera->base->transform->local)});
		mat4_box_t *m1 = GC_ALLOC_INIT(mat4_box_t, {.v = mat4_clone(camera->base->transform->local)});
		gc_unroot(camera_views);
		camera_views = any_array_create_from_raw(
		    (void *[]){
		        m0,
		        m1,
		    },
		    2);
		gc_root(camera_views);
	}
	else {
		camera_origins->buffer[view_index]->v = vec4_create(0, 0, 0, 1.0);
		camera_views->buffer[view_index]->v   = mat4_clone(camera->base->transform->local);
	}

	#ifdef IRON_IOS
	if (config_is_iphone()) {
		viewport_zoom(-2.8);
		vec4_t right                              = vec4_mult(transform_right(camera->base->transform), -0.135);
		camera->base->transform->loc              = vec4_add(camera->base->transform->loc, right);
		camera_origins->buffer[camera_index()]->v = vec4_add(camera_origins->buffer[camera_index()]->v, right);
		camera_object_build_mat(camera);
	}
	#endif
}

void camera_pan_action(bool modif, bool default_keymap) {
	camera_object_t *camera = scene_camera;
	if (operator_shortcut(any_map_get(config_keymap, "action_pan"), SHORTCUT_TYPE_DOWN) || (mouse_down("middle") && !modif && default_keymap)) {
		camera_redraws               = 2;
		f32    f                     = 150 * (1.0 / (float)(camera_distance() / 4.0));
		vec4_t look                  = vec4_mult(transform_look(camera->base->transform), mouse_movement_y / (float)f * config_raw->camera_pan_speed);
		vec4_t right                 = vec4_mult(transform_right(camera->base->transform), -mouse_movement_x / (float)f * config_raw->camera_pan_speed);
		camera->base->transform->loc = vec4_add(camera->base->transform->loc, look);
		camera->base->transform->loc = vec4_add(camera->base->transform->loc, right);
		camera_origins->buffer[camera_index()]->v = vec4_add(camera_origins->buffer[camera_index()]->v, look);
		camera_origins->buffer[camera_index()]->v = vec4_add(camera_origins->buffer[camera_index()]->v, right);
		camera_object_build_mat(camera);
	}
}

f32 camera_get_zoom_delta() {
	return config_raw->zoom_direction == ZOOM_DIRECTION_VERTICAL              ? -mouse_movement_y
	       : config_raw->zoom_direction == ZOOM_DIRECTION_VERTICAL_INVERTED   ? -mouse_movement_y
	       : config_raw->zoom_direction == ZOOM_DIRECTION_HORIZONTAL          ? mouse_movement_x
	       : config_raw->zoom_direction == ZOOM_DIRECTION_HORIZONTAL_INVERTED ? mouse_movement_x
	                                                                          : -(mouse_movement_y - mouse_movement_x);
}

void camera_set_pivot_center_to_mouse() {
	util_render_pick_pos_nor_tex();
	bool is_mesh = math_abs(context_raw->posx_picked) < 50 && math_abs(context_raw->posy_picked) < 50 && math_abs(context_raw->posz_picked) < 50;
	if (!is_mesh) {
		return;
	}

	vec4_box_t *o                  = camera_origins->buffer[camera_index()];
	o->v.x                         = context_raw->posx_picked;
	o->v.y                         = context_raw->posy_picked;
	o->v.z                         = context_raw->posz_picked;

	camera_redraws                 = 2;
	camera_object_t *camera        = scene_camera;
	vec4_t           up            = vec4_mult(transform_up(camera->base->transform), camera_distance());
	camera->base->transform->loc.x = context_raw->posx_picked;
	camera->base->transform->loc.y = context_raw->posy_picked;
	camera->base->transform->loc.z = context_raw->posz_picked;
	camera->base->transform->loc   = vec4_add(camera->base->transform->loc, up);
	camera_object_build_mat(camera);
}
