
let camera_origins: vec4_box_t[];
let camera_views: mat4_box_t[];
let camera_redraws: i32 = 0;
let camera_dir: vec4_t = vec4_create();
let camera_ease: f32 = 1.0;
let camera_controls_down: bool = false;

function camera_init() {
	camera_reset();
}

function camera_update() {
	let camera: camera_object_t = scene_camera;

	if (mouse_view_x() < 0 ||
		mouse_view_x() > app_w() ||
		mouse_view_y() < 0 ||
		mouse_view_y() > app_h()) {

		if (config_raw.wrap_mouse && camera_controls_down) {
			if (mouse_view_x() < 0) {
				mouse_x = mouse_last_x = app_x() + app_w();
				iron_set_mouse_position(math_floor(mouse_x), math_floor(mouse_y));
			}
			else if (mouse_view_x() > app_w()) {
				mouse_x = mouse_last_x = app_x();
				iron_set_mouse_position(math_floor(mouse_x), math_floor(mouse_y));
			}
			else if (mouse_view_y() < 0) {
				mouse_y = mouse_last_y = app_y() + app_h();
				iron_set_mouse_position(math_floor(mouse_x), math_floor(mouse_y));
			}
			else if (mouse_view_y() > app_h()) {
				mouse_y = mouse_last_y = app_y();
				iron_set_mouse_position(math_floor(mouse_x), math_floor(mouse_y));
			}
		}
		else {
			return;
		}
	}

	let modif_key: bool = keyboard_down("alt") || keyboard_down("shift") || keyboard_down("control");
	let modif: bool = modif_key || map_get(config_keymap, "action_rotate") == "middle";
	let default_keymap: bool = config_raw.keymap == "default.json";

	if (operator_shortcut(map_get(config_keymap, "action_rotate"), shortcut_type_t.STARTED) ||
		operator_shortcut(map_get(config_keymap, "action_zoom"), shortcut_type_t.STARTED) ||
		operator_shortcut(map_get(config_keymap, "action_pan"), shortcut_type_t.STARTED) ||
		operator_shortcut(map_get(config_keymap, "rotate_envmap"), shortcut_type_t.STARTED) ||
		(mouse_started("right") && !modif) ||
		(mouse_started("middle") && !modif) ||
		(mouse_wheel_delta != 0 && !modif_key)) {
		camera_controls_down = true;
	}
	else if (!operator_shortcut(map_get(config_keymap, "action_rotate"), shortcut_type_t.DOWN) &&
		!operator_shortcut(map_get(config_keymap, "action_zoom"), shortcut_type_t.DOWN) &&
		!operator_shortcut(map_get(config_keymap, "action_pan"), shortcut_type_t.DOWN) &&
		!operator_shortcut(map_get(config_keymap, "rotate_envmap"), shortcut_type_t.DOWN) &&
		!(mouse_down("right") && !modif) &&
		!(mouse_down("middle") && !modif) &&
		(mouse_wheel_delta == 0 && !modif_key)) {
		camera_controls_down = false;
	}

	if (_input_occupied || !base_ui_enabled || base_is_dragging || base_is_scrolling() || base_is_combo_selected() || !camera_controls_down) {
		return;
	}

	let controls: camera_controls_t = context_raw.camera_controls;
	if (controls == camera_controls_t.ORBIT && (operator_shortcut(map_get(config_keymap, "action_rotate"), shortcut_type_t.DOWN) || (mouse_down("right") && !modif && default_keymap))) {
		camera_redraws = 2;
		let dist: f32 = camera_distance();
		transform_move(camera.base.transform, camera_object_look_world(camera), dist);
		transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 100 * config_raw.camera_rotation_speed);
		transform_rotate(camera.base.transform,  camera_object_right_world(camera), -mouse_movement_y / 100 * config_raw.camera_rotation_speed);
		let up_world: vec4_t = camera_object_up_world(camera);
		if (up_world.z < 0) {
			transform_rotate(camera.base.transform, camera_object_right_world(camera), mouse_movement_y / 100 * config_raw.camera_rotation_speed);
		}
		transform_move(camera.base.transform, camera_object_look_world(camera), -dist);
	}
	else if (controls == camera_controls_t.ROTATE && (operator_shortcut(map_get(config_keymap, "action_rotate"), shortcut_type_t.DOWN) || (mouse_down("right") && !modif && default_keymap))) {
		camera_redraws = 2;
		let t: transform_t = context_main_object().base.transform;
		let up: vec4_t = transform_up(t);
		transform_rotate(t, up, mouse_movement_x / 100 * config_raw.camera_rotation_speed);
		let right: vec4_t = camera_object_right_world(camera);
		transform_rotate(t, right, mouse_movement_y / 100 * config_raw.camera_rotation_speed);
		transform_build_matrix(t);
		let tup: vec4_t = transform_up(t);
		if (tup.z < 0) {
			transform_rotate(t, right, -mouse_movement_y / 100 * config_raw.camera_rotation_speed);
		}
	}

	if (controls == camera_controls_t.ROTATE || controls == camera_controls_t.ORBIT) {
		camera_pan_action(modif, default_keymap);

		if (operator_shortcut(map_get(config_keymap, "action_zoom"), shortcut_type_t.DOWN)) {
			camera_redraws = 2;
			let f: f32 = camera_get_zoom_delta() / 150;
			f *= camera_get_zoom_speed();
			transform_move(camera.base.transform, camera_object_look(camera), f);
		}

		if (mouse_wheel_delta != 0 && !modif_key) {
			camera_redraws = 2;
			let f: f32 = mouse_wheel_delta * (-0.1);
			f *= camera_get_zoom_speed();
			transform_move(camera.base.transform, camera_object_look(camera), f);
		}
	}
	else if (controls == camera_controls_t.FLY && mouse_down("right")) {
		let move_forward: bool = keyboard_down("w") || keyboard_down("up") || mouse_wheel_delta < 0;
		let move_backward: bool = keyboard_down("s") || keyboard_down("down") || mouse_wheel_delta > 0;
		let strafe_left: bool = keyboard_down("a") || keyboard_down("left");
		let strafe_right: bool = keyboard_down("d") || keyboard_down("right");
		let strafe_up: bool = keyboard_down("e");
		let strafe_down: bool = keyboard_down("q");
		let fast: f32 = keyboard_down("shift") ? 2.0 : (keyboard_down("alt") ? 0.5 : 1.0);
		if (mouse_wheel_delta != 0) {
			fast *= math_abs(mouse_wheel_delta) * 4.0;
		}

		if (move_forward || move_backward || strafe_right || strafe_left || strafe_up || strafe_down) {
			camera_ease += time_delta() * 15;
			if (camera_ease > 1.0) {
				camera_ease = 1.0;
			}
			camera_dir = vec4_create(0, 0, 0);
			let look: vec4_t = camera_object_look(camera);
			let right: vec4_t = camera_object_right(camera);
			if (move_forward) {
				camera_dir = vec4_fadd(camera_dir, look.x, look.y, look.z);
			}
			if (move_backward) {
				camera_dir = vec4_fadd(camera_dir, -look.x, -look.y, -look.z);
			}
			if (strafe_right) {
				camera_dir = vec4_fadd(camera_dir, right.x, right.y, right.z);
			}
			if (strafe_left) {
				camera_dir = vec4_fadd(camera_dir, -right.x, -right.y, -right.z);
			}
			if (strafe_up) {
				camera_dir = vec4_fadd(camera_dir, 0, 0, 1);
			}
			if (strafe_down) {
				camera_dir = vec4_fadd(camera_dir, 0, 0, -1);
			}
		}
		else {
			camera_ease -= time_delta() * 20.0 * camera_ease;
			if (camera_ease < 0.0) {
				camera_ease = 0.0;
			}
		}


		let d: f32 = time_delta() * fast * camera_ease * 2.0 * ((move_forward || move_backward) ? config_raw.camera_zoom_speed : config_raw.camera_pan_speed);
		if (d > 0.0) {
			transform_move(camera.base.transform, camera_dir, d);
			if (context_raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
				viewport_update_camera_type(context_raw.camera_type);
			}
		}

		camera_redraws = 2;
		transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 200 * config_raw.camera_rotation_speed);
		transform_rotate(camera.base.transform, camera_object_right(camera), -mouse_movement_y / 200 * config_raw.camera_rotation_speed);
	}

	if (operator_shortcut(map_get(config_keymap, "rotate_envmap"), shortcut_type_t.DOWN)) {
		camera_redraws = 2;
		context_raw.envmap_angle -= mouse_movement_x / 100;
	}

	if (camera_redraws > 0) {
		camera_redraws--;
		context_raw.ddirty = 2;

		if (context_raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
			viewport_update_camera_type(context_raw.camera_type);
		}
	}
}

function camera_distance(): f32 {
	let camera: camera_object_t = scene_camera;
	return vec4_dist(camera_origins[camera_index()].v, camera.base.transform.loc);
}

function camera_index(): i32 {
	return context_raw.view_index_last > 0 ? 1 : 0;
}

function camera_get_zoom_speed(): f32 {
	let sign: i32 = config_raw.zoom_direction == zoom_direction_t.VERTICAL_INVERTED ||
					config_raw.zoom_direction == zoom_direction_t.HORIZONTAL_INVERTED ||
					config_raw.zoom_direction == zoom_direction_t.VERTICAL_HORIZONTAL_INVERTED ? -1 : 1;
	let camera: camera_object_t = scene_camera;
	let fov_adjust: f32 = camera.data.fov;
	return (config_raw.camera_zoom_speed * sign) / fov_adjust;
}

function camera_reset(view_index: i32 = -1) {
	let camera: camera_object_t = scene_camera;
	if (view_index == -1) {
		let v0: vec4_box_t = { v: vec4_create(0, 0, 0, 1) };
		let v1: vec4_box_t = { v: vec4_create(0, 0, 0, 1) };
		camera_origins = [v0, v1];

		let m0: mat4_box_t = { v: mat4_clone(camera.base.transform.local) };
		let m1: mat4_box_t = { v: mat4_clone(camera.base.transform.local) };
		camera_views = [m0, m1];
	}
	else {
		camera_origins[view_index].v = vec4_create(0, 0, 0);
		camera_views[view_index].v = mat4_clone(camera.base.transform.local);
	}
}

function camera_pan_action(modif: bool, default_keymap: bool) {
	let camera: camera_object_t = scene_camera;
	if (operator_shortcut(map_get(config_keymap, "action_pan"), shortcut_type_t.DOWN) || (mouse_down("middle") && !modif && default_keymap)) {
		camera_redraws = 2;
		let look: vec4_t = vec4_mult(transform_look(camera.base.transform), mouse_movement_y / 150 * config_raw.camera_pan_speed);
		let right: vec4_t = vec4_mult(transform_right(camera.base.transform), -mouse_movement_x / 150 * config_raw.camera_pan_speed);
		camera.base.transform.loc = vec4_add(camera.base.transform.loc, look);
		camera.base.transform.loc = vec4_add(camera.base.transform.loc, right);
		camera_origins[camera_index()].v = vec4_add(camera_origins[camera_index()].v, look);
		camera_origins[camera_index()].v = vec4_add(camera_origins[camera_index()].v, right);
		camera_object_build_mat(camera);
	}
}

function camera_get_zoom_delta(): f32 {
	return config_raw.zoom_direction == zoom_direction_t.VERTICAL ? -mouse_movement_y :
		   config_raw.zoom_direction == zoom_direction_t.VERTICAL_INVERTED ? -mouse_movement_y :
		   config_raw.zoom_direction == zoom_direction_t.HORIZONTAL ? mouse_movement_x :
		   config_raw.zoom_direction == zoom_direction_t.HORIZONTAL_INVERTED ? mouse_movement_x :
		   -(mouse_movement_y - mouse_movement_x);
}
