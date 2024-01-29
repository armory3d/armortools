
class Camera {

	static origins: Vec4[];
	static views: Mat4[];
	static redraws = 0;
	static dir = new Vec4();
	static ease = 1.0;
	static controlsDown = false;

	constructor() {
		Camera.reset();
	}

	static update = () => {
		let camera = Scene.camera;

		if (Mouse.viewX < 0 ||
			Mouse.viewX > App.w() ||
			Mouse.viewY < 0 ||
			Mouse.viewY > App.h()) {

			if (Config.raw.wrap_mouse && Camera.controlsDown) {
				if (Mouse.viewX < 0) {
					Mouse.x = Mouse.lastX = App.x() + App.w();
					Krom.setMousePosition(Math.floor(Mouse.x), Math.floor(Mouse.y));
				}
				else if (Mouse.viewX > App.w()) {
					Mouse.x = Mouse.lastX = App.x();
					Krom.setMousePosition(Math.floor(Mouse.x), Math.floor(Mouse.y));
				}
				else if (Mouse.viewY < 0) {
					Mouse.y = Mouse.lastY = App.y() + App.h();
					Krom.setMousePosition(Math.floor(Mouse.x), Math.floor(Mouse.y));
				}
				else if (Mouse.viewY > App.h()) {
					Mouse.y = Mouse.lastY = App.y();
					Krom.setMousePosition(Math.floor(Mouse.x), Math.floor(Mouse.y));
				}
			}
			else {
				return;
			}
		}

		let modifKey = Keyboard.down("alt") || Keyboard.down("shift") || Keyboard.down("control");
		let modif = modifKey || Config.keymap.action_rotate == "middle";
		let defaultKeymap = Config.raw.keymap == "default.json";

		if (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutStarted) ||
			(Mouse.started("right") && !modif) ||
			(Mouse.started("middle") && !modif) ||
			(Mouse.wheelDelta != 0 && !modifKey)) {
			Camera.controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown) &&
			!(Mouse.down("right") && !modif) &&
			!(Mouse.down("middle") && !modif) &&
			(Mouse.wheelDelta == 0 && !modifKey)) {
			Camera.controlsDown = false;
		}

		if (Input.occupied ||
			!Base.uiEnabled ||
			Base.isDragging ||
			Base.isScrolling() ||
			Base.isComboSelected() ||
			!Camera.controlsDown) {
			return;
		}

		let controls = Context.raw.cameraControls;
		if (controls == CameraControls.ControlsOrbit && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (Mouse.down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let dist = Camera.distance();
			camera.transform.move(camera.lookWorld(), dist);
			camera.transform.rotate(Vec4.zAxis(), -Mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			camera.transform.rotate(camera.rightWorld(), -Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			if (camera.upWorld().z < 0) {
				camera.transform.rotate(camera.rightWorld(), Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
			camera.transform.move(camera.lookWorld(), -dist);
		}
		else if (controls == CameraControls.ControlsRotate && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (Mouse.down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let t = Context.mainObject().transform;
			let up = t.up().normalize();
			t.rotate(up, Mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			let right = camera.rightWorld().normalize();
			t.rotate(right, Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			t.buildMatrix();
			if (t.up().z < 0) {
				t.rotate(right, -Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
		}

		if (controls == CameraControls.ControlsRotate || controls == CameraControls.ControlsOrbit) {
			Camera.panAction(modif, defaultKeymap);

			if (Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown)) {
				Camera.redraws = 2;
				let f = Camera.getZoomDelta() / 150;
				f *= Camera.getCameraZoomSpeed();
				camera.transform.move(camera.look(), f);
			}

			if (Mouse.wheelDelta != 0 && !modifKey) {
				Camera.redraws = 2;
				let f = Mouse.wheelDelta * (-0.1);
				f *= Camera.getCameraZoomSpeed();
				camera.transform.move(camera.look(), f);
			}
		}
		else if (controls == CameraControls.ControlsFly && Mouse.down("right")) {
			let moveForward = Keyboard.down("w") || Keyboard.down("up") || Mouse.wheelDelta < 0;
			let moveBackward = Keyboard.down("s") || Keyboard.down("down") || Mouse.wheelDelta > 0;
			let strafeLeft = Keyboard.down("a") || Keyboard.down("left");
			let strafeRight = Keyboard.down("d") || Keyboard.down("right");
			let strafeUp = Keyboard.down("e");
			let strafeDown = Keyboard.down("q");
			let fast = Keyboard.down("shift") ? 2.0 : (Keyboard.down("alt") ? 0.5 : 1.0);
			if (Mouse.wheelDelta != 0) {
				fast *= Math.abs(Mouse.wheelDelta) * 4.0;
			}

			if (moveForward || moveBackward || strafeRight || strafeLeft || strafeUp || strafeDown) {
				Camera.ease += Time.delta * 15;
				if (Camera.ease > 1.0) Camera.ease = 1.0;
				Camera.dir.set(0, 0, 0);
				if (moveForward) Camera.dir.addf(camera.look().x, camera.look().y, camera.look().z);
				if (moveBackward) Camera.dir.addf(-camera.look().x, -camera.look().y, -camera.look().z);
				if (strafeRight) Camera.dir.addf(camera.right().x, camera.right().y, camera.right().z);
				if (strafeLeft) Camera.dir.addf(-camera.right().x, -camera.right().y, -camera.right().z);
				if (strafeUp) Camera.dir.addf(0, 0, 1);
				if (strafeDown) Camera.dir.addf(0, 0, -1);
			}
			else {
				Camera.ease -= Time.delta * 20.0 * Camera.ease;
				if (Camera.ease < 0.0) Camera.ease = 0.0;
			}


			let d = Time.delta * fast * Camera.ease * 2.0 * ((moveForward || moveBackward) ? Config.raw.camera_zoom_speed : Config.raw.camera_pan_speed);
			if (d > 0.0) {
				camera.transform.move(Camera.dir, d);
				if (Context.raw.cameraType == CameraType.CameraOrthographic) {
					Viewport.updateCameraType(Context.raw.cameraType);
				}
			}

			Camera.redraws = 2;
			camera.transform.rotate(Vec4.zAxis(), -Mouse.movementX / 200 * Config.raw.camera_rotation_speed);
			camera.transform.rotate(camera.right(), -Mouse.movementY / 200 * Config.raw.camera_rotation_speed);
		}

		if (Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			let light = Scene.lights[0];
			Context.raw.lightAngle = (Context.raw.lightAngle + ((Mouse.movementX / 100) % (2 * Math.PI) + 2 * Math.PI)) % (2 * Math.PI);
			let m = Mat4.rotationZ(Mouse.movementX / 100);
			light.transform.local.multmat(m);
			light.transform.decompose();
		}

		if (Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			Context.raw.envmapAngle -= Mouse.movementX / 100;
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
		let camera = Scene.camera;
		return Vec4.distance(Camera.origins[Camera.index()], camera.transform.loc);
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
		let camera = Scene.camera;
		if (viewIndex == -1) {
			Camera.origins = [new Vec4(0, 0, 0), new Vec4(0, 0, 0)];
			Camera.views = [camera.transform.local.clone(), camera.transform.local.clone()];
		}
		else {
			Camera.origins[viewIndex] = new Vec4(0, 0, 0);
			Camera.views[viewIndex] = camera.transform.local.clone();
		}
	}

	static panAction = (modif: bool, defaultKeymap: bool) => {
		let camera = Scene.camera;
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) || (Mouse.down("middle") && !modif && defaultKeymap)) {
			Camera.redraws = 2;
			let look = camera.transform.look().normalize().mult(Mouse.movementY / 150 * Config.raw.camera_pan_speed);
			let right = camera.transform.right().normalize().mult(-Mouse.movementX / 150 * Config.raw.camera_pan_speed);
			camera.transform.loc.add(look);
			camera.transform.loc.add(right);
			Camera.origins[Camera.index()].add(look);
			Camera.origins[Camera.index()].add(right);
			camera.buildMatrix();
		}
	}

	static getZoomDelta = (): f32 => {
		return Config.raw.zoom_direction == ZoomDirection.ZoomVertical ? -Mouse.movementY :
			   Config.raw.zoom_direction == ZoomDirection.ZoomVerticalInverted ? -Mouse.movementY :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontal ? Mouse.movementX :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontalInverted ? Mouse.movementX :
			   -(Mouse.movementY - Mouse.movementX);
	}
}
