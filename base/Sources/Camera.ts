
class Camera {

	static origins: vec4_t[];
	static views: mat4_t[];
	static redraws: i32 = 0;
	static dir: vec4_t = vec4_create();
	static ease: f32 = 1.0;
	static controls_down: bool = false;

	constructor() {
		Camera.reset();
	}

	static update = () => {
		let camera: camera_object_t = scene_camera;

		if (mouse_view_x() < 0 ||
			mouse_view_x() > app_w() ||
			mouse_view_y() < 0 ||
			mouse_view_y() > app_h()) {

			if (Config.raw.wrap_mouse && Camera.controls_down) {
				if (mouse_view_x() < 0) {
					mouse_x = mouse_last_x = app_x() + app_w();
					krom_set_mouse_position(Math.floor(mouse_x), Math.floor(mouse_y));
				}
				else if (mouse_view_x() > app_w()) {
					mouse_x = mouse_last_x = app_x();
					krom_set_mouse_position(Math.floor(mouse_x), Math.floor(mouse_y));
				}
				else if (mouse_view_y() < 0) {
					mouse_y = mouse_last_y = app_y() + app_h();
					krom_set_mouse_position(Math.floor(mouse_x), Math.floor(mouse_y));
				}
				else if (mouse_view_y() > app_h()) {
					mouse_y = mouse_last_y = app_y();
					krom_set_mouse_position(Math.floor(mouse_x), Math.floor(mouse_y));
				}
			}
			else {
				return;
			}
		}

		let modif_key: bool = keyboard_down("alt") || keyboard_down("shift") || keyboard_down("control");
		let modif: bool = modif_key || Config.keymap.action_rotate == "middle";
		let default_keymap: bool = Config.raw.keymap == "default.json";

		if (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutStarted) ||
			(mouse_started("right") && !modif) ||
			(mouse_started("middle") && !modif) ||
			(mouse_wheel_delta != 0 && !modif_key)) {
			Camera.controls_down = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown) &&
			!(mouse_down("right") && !modif) &&
			!(mouse_down("middle") && !modif) &&
			(mouse_wheel_delta == 0 && !modif_key)) {
			Camera.controls_down = false;
		}

		if (_input_occupied || !base_ui_enabled || base_is_dragging || base_is_scrolling() || base_is_combo_selected() || !Camera.controls_down) {
			return;
		}

		let controls: camera_controls_t = Context.raw.camera_controls;
		if (controls == camera_controls_t.ORBIT && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (mouse_down("right") && !modif && default_keymap))) {
			Camera.redraws = 2;
			let dist: f32 = Camera.distance();
			transform_move(camera.base.transform, camera_object_look_world(camera), dist);
			transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 100 * Config.raw.camera_rotation_speed);
			transform_rotate(camera.base.transform, camera_object_right_world(camera), -mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			if (camera_object_up_world(camera).z < 0) {
				transform_rotate(camera.base.transform, camera_object_right_world(camera), mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			}
			transform_move(camera.base.transform, camera_object_look_world(camera), -dist);
		}
		else if (controls == camera_controls_t.ROTATE && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (mouse_down("right") && !modif && default_keymap))) {
			Camera.redraws = 2;
			let t: transform_t = Context.main_object().base.transform;
			let up: vec4_t = vec4_normalize(transform_up(t));
			transform_rotate(t, up, mouse_movement_x / 100 * Config.raw.camera_rotation_speed);
			let right: vec4_t = vec4_normalize(camera_object_right_world(camera));
			transform_rotate(t, right, mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			transform_build_matrix(t);
			if (transform_up(t).z < 0) {
				transform_rotate(t, right, -mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			}
		}

		if (controls == camera_controls_t.ROTATE || controls == camera_controls_t.ORBIT) {
			Camera.pan_action(modif, default_keymap);

			if (Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown)) {
				Camera.redraws = 2;
				let f: f32 = Camera.get_zoom_delta() / 150;
				f *= Camera.get_camera_zoom_speed();
				transform_move(camera.base.transform, camera_object_look(camera), f);
			}

			if (mouse_wheel_delta != 0 && !modif_key) {
				Camera.redraws = 2;
				let f: f32 = mouse_wheel_delta * (-0.1);
				f *= Camera.get_camera_zoom_speed();
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
				fast *= Math.abs(mouse_wheel_delta) * 4.0;
			}

			if (move_forward || move_backward || strafe_right || strafe_left || strafe_up || strafe_down) {
				Camera.ease += time_delta() * 15;
				if (Camera.ease > 1.0) Camera.ease = 1.0;
				vec4_set(Camera.dir, 0, 0, 0);
				if (move_forward) vec4_add_f(Camera.dir, camera_object_look(camera).x, camera_object_look(camera).y, camera_object_look(camera).z);
				if (move_backward) vec4_add_f(Camera.dir, -camera_object_look(camera).x, -camera_object_look(camera).y, -camera_object_look(camera).z);
				if (strafe_right) vec4_add_f(Camera.dir, camera_object_right(camera).x, camera_object_right(camera).y, camera_object_right(camera).z);
				if (strafe_left) vec4_add_f(Camera.dir, -camera_object_right(camera).x, -camera_object_right(camera).y, -camera_object_right(camera).z);
				if (strafe_up) vec4_add_f(Camera.dir, 0, 0, 1);
				if (strafe_down) vec4_add_f(Camera.dir, 0, 0, -1);
			}
			else {
				Camera.ease -= time_delta() * 20.0 * Camera.ease;
				if (Camera.ease < 0.0) Camera.ease = 0.0;
			}


			let d: f32 = time_delta() * fast * Camera.ease * 2.0 * ((move_forward || move_backward) ? Config.raw.camera_zoom_speed : Config.raw.camera_pan_speed);
			if (d > 0.0) {
				transform_move(camera.base.transform, Camera.dir, d);
				if (Context.raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
					Viewport.update_camera_type(Context.raw.camera_type);
				}
			}

			Camera.redraws = 2;
			transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 200 * Config.raw.camera_rotation_speed);
			transform_rotate(camera.base.transform, camera_object_right(camera), -mouse_movement_y / 200 * Config.raw.camera_rotation_speed);
		}

		if (Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			let light: light_object_t = scene_lights[0];
			Context.raw.light_angle = (Context.raw.light_angle + ((mouse_movement_x / 100) % (2 * Math.PI) + 2 * Math.PI)) % (2 * Math.PI);
			let m: mat4_t = mat4_rot_z(mouse_movement_x / 100);
			mat4_mult_mat(light.base.transform.local, m);
			transform_decompose(light.base.transform);
		}

		if (Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			Context.raw.envmap_angle -= mouse_movement_x / 100;
		}

		if (Camera.redraws > 0) {
			Camera.redraws--;
			Context.raw.ddirty = 2;

			if (Context.raw.camera_type == camera_type_t.ORTHOGRAPHIC) {
				Viewport.update_camera_type(Context.raw.camera_type);
			}
		}
	}

	static distance = (): f32 => {
		let camera: camera_object_t = scene_camera;
		return vec4_dist(Camera.origins[Camera.index()], camera.base.transform.loc);
	}

	static index = (): i32 => {
		return Context.raw.view_index_last > 0 ? 1 : 0;
	}

	static get_camera_zoom_speed = (): f32 => {
		let sign: i32 = Config.raw.zoom_direction == zoom_direction_t.VERTICAL_INVERTED ||
						Config.raw.zoom_direction == zoom_direction_t.HORIZONTAL_INVERTED ||
				   		Config.raw.zoom_direction == zoom_direction_t.VERTICAL_HORIZONTAL_INVERTED ? -1 : 1;
		return Config.raw.camera_zoom_speed * sign;
	}

	static reset = (viewIndex: i32 = -1) => {
		let camera: camera_object_t = scene_camera;
		if (viewIndex == -1) {
			Camera.origins = [vec4_create(0, 0, 0), vec4_create(0, 0, 0)];
			Camera.views = [mat4_clone(camera.base.transform.local), mat4_clone(camera.base.transform.local)];
		}
		else {
			Camera.origins[viewIndex] = vec4_create(0, 0, 0);
			Camera.views[viewIndex] = mat4_clone(camera.base.transform.local);
		}
	}

	static pan_action = (modif: bool, defaultKeymap: bool) => {
		let camera: camera_object_t = scene_camera;
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) || (mouse_down("middle") && !modif && defaultKeymap)) {
			Camera.redraws = 2;
			let look: vec4_t = vec4_mult(vec4_normalize(transform_look(camera.base.transform)), mouse_movement_y / 150 * Config.raw.camera_pan_speed);
			let right: vec4_t = vec4_mult(vec4_normalize(transform_right(camera.base.transform)), -mouse_movement_x / 150 * Config.raw.camera_pan_speed);
			vec4_add(camera.base.transform.loc, look);
			vec4_add(camera.base.transform.loc, right);
			vec4_add(Camera.origins[Camera.index()], look);
			vec4_add(Camera.origins[Camera.index()], right);
			camera_object_build_mat(camera);
		}
	}

	static get_zoom_delta = (): f32 => {
		return Config.raw.zoom_direction == zoom_direction_t.VERTICAL ? -mouse_movement_y :
			   Config.raw.zoom_direction == zoom_direction_t.VERTICAL_INVERTED ? -mouse_movement_y :
			   Config.raw.zoom_direction == zoom_direction_t.HORIZONTAL ? mouse_movement_x :
			   Config.raw.zoom_direction == zoom_direction_t.HORIZONTAL_INVERTED ? mouse_movement_x :
			   -(mouse_movement_y - mouse_movement_x);
	}
}
