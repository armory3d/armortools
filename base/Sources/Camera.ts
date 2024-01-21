
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
		let mouse = Input.getMouse();
		let kb = Input.getKeyboard();
		let camera = Scene.active.camera;

		if (mouse.viewX < 0 ||
			mouse.viewX > App.w() ||
			mouse.viewY < 0 ||
			mouse.viewY > App.h()) {

			if (Config.raw.wrap_mouse && Camera.controlsDown) {
				if (mouse.viewX < 0) {
					mouse.x = mouse.lastX = App.x() + App.w();
					Krom.setMousePosition(Math.floor(mouse.x), Math.floor(mouse.y));
				}
				else if (mouse.viewX > App.w()) {
					mouse.x = mouse.lastX = App.x();
					Krom.setMousePosition(Math.floor(mouse.x), Math.floor(mouse.y));
				}
				else if (mouse.viewY < 0) {
					mouse.y = mouse.lastY = App.y() + App.h();
					Krom.setMousePosition(Math.floor(mouse.x), Math.floor(mouse.y));
				}
				else if (mouse.viewY > App.h()) {
					mouse.y = mouse.lastY = App.y();
					Krom.setMousePosition(Math.floor(mouse.x), Math.floor(mouse.y));
				}
			}
			else {
				return;
			}
		}

		let modifKey = kb.down("alt") || kb.down("shift") || kb.down("control");
		let modif = modifKey || Config.keymap.action_rotate == "middle";
		let defaultKeymap = Config.raw.keymap == "default.json";

		if (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutStarted) ||
			(mouse.started("right") && !modif) ||
			(mouse.started("middle") && !modif) ||
			(mouse.wheelDelta != 0 && !modifKey)) {
			Camera.controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown) &&
			!(mouse.down("right") && !modif) &&
			!(mouse.down("middle") && !modif) &&
			(mouse.wheelDelta == 0 && !modifKey)) {
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
		if (controls == CameraControls.ControlsOrbit && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (mouse.down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let dist = Camera.distance();
			camera.transform.move(camera.lookWorld(), dist);
			camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			camera.transform.rotate(camera.rightWorld(), -mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			if (camera.upWorld().z < 0) {
				camera.transform.rotate(camera.rightWorld(), mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
			camera.transform.move(camera.lookWorld(), -dist);
		}
		else if (controls == CameraControls.ControlsRotate && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (mouse.down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let t = Context.mainObject().transform;
			let up = t.up().normalize();
			t.rotate(up, mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			let right = camera.rightWorld().normalize();
			t.rotate(right, mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			t.buildMatrix();
			if (t.up().z < 0) {
				t.rotate(right, -mouse.movementY / 100 * Config.raw.camera_rotation_speed);
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

			if (mouse.wheelDelta != 0 && !modifKey) {
				Camera.redraws = 2;
				let f = mouse.wheelDelta * (-0.1);
				f *= Camera.getCameraZoomSpeed();
				camera.transform.move(camera.look(), f);
			}
		}
		else if (controls == CameraControls.ControlsFly && mouse.down("right")) {
			let moveForward = kb.down("w") || kb.down("up") || mouse.wheelDelta < 0;
			let moveBackward = kb.down("s") || kb.down("down") || mouse.wheelDelta > 0;
			let strafeLeft = kb.down("a") || kb.down("left");
			let strafeRight = kb.down("d") || kb.down("right");
			let strafeUp = kb.down("e");
			let strafeDown = kb.down("q");
			let fast = kb.down("shift") ? 2.0 : (kb.down("alt") ? 0.5 : 1.0);
			if (mouse.wheelDelta != 0) {
				fast *= Math.abs(mouse.wheelDelta) * 4.0;
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
			camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 200 * Config.raw.camera_rotation_speed);
			camera.transform.rotate(camera.right(), -mouse.movementY / 200 * Config.raw.camera_rotation_speed);
		}

		if (Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			let light = Scene.active.lights[0];
			Context.raw.lightAngle = (Context.raw.lightAngle + ((mouse.movementX / 100) % (2 * Math.PI) + 2 * Math.PI)) % (2 * Math.PI);
			let m = Mat4.rotationZ(mouse.movementX / 100);
			light.transform.local.multmat(m);
			light.transform.decompose();
		}

		if (Operator.shortcut(Config.keymap.rotate_envmap, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			Context.raw.envmapAngle -= mouse.movementX / 100;
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
		let camera = Scene.active.camera;
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
		let camera = Scene.active.camera;
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
		let camera = Scene.active.camera;
		let mouse = Input.getMouse();
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) || (mouse.down("middle") && !modif && defaultKeymap)) {
			Camera.redraws = 2;
			let look = camera.transform.look().normalize().mult(mouse.movementY / 150 * Config.raw.camera_pan_speed);
			let right = camera.transform.right().normalize().mult(-mouse.movementX / 150 * Config.raw.camera_pan_speed);
			camera.transform.loc.add(look);
			camera.transform.loc.add(right);
			Camera.origins[Camera.index()].add(look);
			Camera.origins[Camera.index()].add(right);
			camera.buildMatrix();
		}
	}

	static getZoomDelta = (): f32 => {
		let mouse = Input.getMouse();
		return Config.raw.zoom_direction == ZoomDirection.ZoomVertical ? -mouse.movementY :
			   Config.raw.zoom_direction == ZoomDirection.ZoomVerticalInverted ? -mouse.movementY :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontal ? mouse.movementX :
			   Config.raw.zoom_direction == ZoomDirection.ZoomHorizontalInverted ? mouse.movementX :
			   -(mouse.movementY - mouse.movementX);
	}
}
