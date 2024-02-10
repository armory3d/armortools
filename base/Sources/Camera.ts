
class Camera {

	static origins: vec4_t[];
	static views: mat4_t[];
	static redraws = 0;
	static dir = vec4_create();
	static ease = 1.0;
	static controlsDown = false;

	constructor() {
		Camera.reset();
	}

	static update = () => {
		let camera = scene_camera;

		if (mouse_view_x() < 0 ||
			mouse_view_x() > app_w() ||
			mouse_view_y() < 0 ||
			mouse_view_y() > app_h()) {

			if (Config.raw.wrap_mouse && Camera.controlsDown) {
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

		let modifKey = keyboard_down("alt") || keyboard_down("shift") || keyboard_down("control");
		let modif = modifKey || Config.keymap.action_rotate == "middle";
		let defaultKeymap = Config.raw.keymap == "default.json";

		if (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutStarted) ||
			(mouse_started("right") && !modif) ||
			(mouse_started("middle") && !modif) ||
			(mouse_wheel_delta != 0 && !modifKey)) {
			Camera.controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown) &&
			!(mouse_down("right") && !modif) &&
			!(mouse_down("middle") && !modif) &&
			(mouse_wheel_delta == 0 && !modifKey)) {
			Camera.controlsDown = false;
		}

		if (_input_occupied || !Base.uiEnabled || Base.isDragging || Base.isScrolling() || Base.isComboSelected() || !Camera.controlsDown) {
			return;
		}

		let controls = Context.raw.cameraControls;
		if (controls == CameraControls.ControlsOrbit && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (mouse_down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let dist = Camera.distance();
			transform_move(camera.base.transform, camera_object_look_world(camera), dist);
			transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 100 * Config.raw.camera_rotation_speed);
			transform_rotate(camera.base.transform, camera_object_right_world(camera), -mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			if (camera_object_up_world(camera).z < 0) {
				transform_rotate(camera.base.transform, camera_object_right_world(camera), mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			}
			transform_move(camera.base.transform, camera_object_look_world(camera), -dist);
		}
		else if (controls == CameraControls.ControlsRotate && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (mouse_down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let t = Context.mainObject().base.transform;
			let up = vec4_normalize(transform_up(t));
			transform_rotate(t, up, mouse_movement_x / 100 * Config.raw.camera_rotation_speed);
			let right = vec4_normalize(camera_object_right_world(camera));
			transform_rotate(t, right, mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			transform_build_matrix(t);
			if (transform_up(t).z < 0) {
				transform_rotate(t, right, -mouse_movement_y / 100 * Config.raw.camera_rotation_speed);
			}
		}

		if (controls == CameraControls.ControlsRotate || controls == CameraControls.ControlsOrbit) {
			Camera.panAction(modif, defaultKeymap);

			if (Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown)) {
				Camera.redraws = 2;
				let f = Camera.getZoomDelta() / 150;
				f *= Camera.getCameraZoomSpeed();
				transform_move(camera.base.transform, camera_object_look(camera), f);
			}

			if (mouse_wheel_delta != 0 && !modifKey) {
				Camera.redraws = 2;
				let f = mouse_wheel_delta * (-0.1);
				f *= Camera.getCameraZoomSpeed();
				transform_move(camera.base.transform, camera_object_look(camera), f);
			}
		}
		else if (controls == CameraControls.ControlsFly && mouse_down("right")) {
			let moveForward = keyboard_down("w") || keyboard_down("up") || mouse_wheel_delta < 0;
			let moveBackward = keyboard_down("s") || keyboard_down("down") || mouse_wheel_delta > 0;
			let strafeLeft = keyboard_down("a") || keyboard_down("left");
			let strafeRight = keyboard_down("d") || keyboard_down("right");
			let strafeUp = keyboard_down("e");
			let strafeDown = keyboard_down("q");
			let fast = keyboard_down("shift") ? 2.0 : (keyboard_down("alt") ? 0.5 : 1.0);
			if (mouse_wheel_delta != 0) {
				fast *= Math.abs(mouse_wheel_delta) * 4.0;
			}

			if (moveForward || moveBackward || strafeRight || strafeLeft || strafeUp || strafeDown) {
				Camera.ease += time_delta() * 15;
				if (Camera.ease > 1.0) Camera.ease = 1.0;
				vec4_set(Camera.dir, 0, 0, 0);
				if (moveForward) vec4_add_f(Camera.dir, camera_object_look(camera).x, camera_object_look(camera).y, camera_object_look(camera).z);
				if (moveBackward) vec4_add_f(Camera.dir, -camera_object_look(camera).x, -camera_object_look(camera).y, -camera_object_look(camera).z);
				if (strafeRight) vec4_add_f(Camera.dir, camera_object_right(camera).x, camera_object_right(camera).y, camera_object_right(camera).z);
				if (strafeLeft) vec4_add_f(Camera.dir, -camera_object_right(camera).x, -camera_object_right(camera).y, -camera_object_right(camera).z);
				if (strafeUp) vec4_add_f(Camera.dir, 0, 0, 1);
				if (strafeDown) vec4_add_f(Camera.dir, 0, 0, -1);
			}
			else {
				Camera.ease -= time_delta() * 20.0 * Camera.ease;
				if (Camera.ease < 0.0) Camera.ease = 0.0;
			}


			let d = time_delta() * fast * Camera.ease * 2.0 * ((moveForward || moveBackward) ? Config.raw.camera_zoom_speed : Config.raw.camera_pan_speed);
			if (d > 0.0) {
				transform_move(camera.base.transform, Camera.dir, d);
				if (Context.raw.cameraType == CameraType.CameraOrthographic) {
					Viewport.updateCameraType(Context.raw.cameraType);
				}
			}

			Camera.redraws = 2;
			transform_rotate(camera.base.transform, vec4_z_axis(), -mouse_movement_x / 200 * Config.raw.camera_rotation_speed);
			transform_rotate(camera.base.transform, camera_object_right(camera), -mouse_movement_y / 200 * Config.raw.camera_rotation_speed);
		}

		if (Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			let light = scene_lights[0];
			Context.raw.lightAngle = (Context.raw.lightAngle + ((mouse_movement_x / 100) % (2 * Math.PI) + 2 * Math.PI)) % (2 * Math.PI);
			let m = mat4_rot_z(mouse_movement_x / 100);
			mat4_mult_mat(light.base.transform.local, m);
			transform_decompose(light.base.transform);
		}

		if (Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			Context.raw.envmapAngle -= mouse_movement_x / 100;
		}

		if (Camera.redraws > 0) {
			Camera.redraws--;
			Context.raw.ddirty = 2;

			if (Context.raw.cameraType == CameraType.CameraOrthographic) {
				Viewport.updateCameraType(Context.raw.cameraType);
			}
		}
	}

	static distance = (): f32 => {
		let camera = scene_camera;
		return vec4_dist(Camera.origins[Camera.index()], camera.base.transform.loc);
	}

	static index = (): i32 => {
		return Context.raw.viewIndexLast > 0 ? 1 : 0;
	}

	static getCameraZoomSpeed = (): f32 => {
		let sign = Config.raw.zoom_direction == ZoomDirection.ZoomVerticalInverted ||
				   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontalInverted ||
				   Config.raw.zoom_direction == ZoomDirection.ZoomVerticalAndHorizontalInverted ? -1 : 1;
		return Config.raw.camera_zoom_speed * sign;
	}

	static reset = (viewIndex = -1) => {
		let camera = scene_camera;
		if (viewIndex == -1) {
			Camera.origins = [vec4_create(0, 0, 0), vec4_create(0, 0, 0)];
			Camera.views = [mat4_clone(camera.base.transform.local), mat4_clone(camera.base.transform.local)];
		}
		else {
			Camera.origins[viewIndex] = vec4_create(0, 0, 0);
			Camera.views[viewIndex] = mat4_clone(camera.base.transform.local);
		}
	}

	static panAction = (modif: bool, defaultKeymap: bool) => {
		let camera = scene_camera;
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) || (mouse_down("middle") && !modif && defaultKeymap)) {
			Camera.redraws = 2;
			let look = vec4_mult(vec4_normalize(transform_look(camera.base.transform)), mouse_movement_y / 150 * Config.raw.camera_pan_speed);
			let right = vec4_mult(vec4_normalize(transform_right(camera.base.transform)), -mouse_movement_x / 150 * Config.raw.camera_pan_speed);
			vec4_add(camera.base.transform.loc, look);
			vec4_add(camera.base.transform.loc, right);
			vec4_add(Camera.origins[Camera.index()], look);
			vec4_add(Camera.origins[Camera.index()], right);
			camera_object_build_mat(camera);
		}
	}

	static getZoomDelta = (): f32 => {
		return Config.raw.zoom_direction == ZoomDirection.ZoomVertical ? -mouse_movement_y :
			   Config.raw.zoom_direction == ZoomDirection.ZoomVerticalInverted ? -mouse_movement_y :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontal ? mouse_movement_x :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontalInverted ? mouse_movement_x :
			   -(mouse_movement_y - mouse_movement_x);
	}
}
