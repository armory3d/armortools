
class Camera {

	static origins: TVec4[];
	static views: TMat4[];
	static redraws = 0;
	static dir = Vec4.create();
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
			Transform.move(camera.base.transform, CameraObject.lookWorld(camera), dist);
			Transform.rotate(camera.base.transform, Vec4.zAxis(), -Mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			Transform.rotate(camera.base.transform, CameraObject.rightWorld(camera), -Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			if (CameraObject.upWorld(camera).z < 0) {
				Transform.rotate(camera.base.transform, CameraObject.rightWorld(camera), Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
			Transform.move(camera.base.transform, CameraObject.lookWorld(camera), -dist);
		}
		else if (controls == CameraControls.ControlsRotate && (Operator.shortcut(Config.keymap.action_rotate, ShortcutType.ShortcutDown) || (Mouse.down("right") && !modif && defaultKeymap))) {
			Camera.redraws = 2;
			let t = Context.mainObject().base.transform;
			let up = Vec4.normalize(Transform.up(t));
			Transform.rotate(t, up, Mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			let right = Vec4.normalize(CameraObject.rightWorld(camera));
			Transform.rotate(t, right, Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			Transform.buildMatrix(t);
			if (Transform.up(t).z < 0) {
				Transform.rotate(t, right, -Mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
		}

		if (controls == CameraControls.ControlsRotate || controls == CameraControls.ControlsOrbit) {
			Camera.panAction(modif, defaultKeymap);

			if (Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown)) {
				Camera.redraws = 2;
				let f = Camera.getZoomDelta() / 150;
				f *= Camera.getCameraZoomSpeed();
				Transform.move(camera.base.transform, CameraObject.look(camera), f);
			}

			if (Mouse.wheelDelta != 0 && !modifKey) {
				Camera.redraws = 2;
				let f = Mouse.wheelDelta * (-0.1);
				f *= Camera.getCameraZoomSpeed();
				Transform.move(camera.base.transform, CameraObject.look(camera), f);
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
				Vec4.set(Camera.dir, 0, 0, 0);
				if (moveForward) Vec4.addf(Camera.dir, CameraObject.look(camera).x, CameraObject.look(camera).y, CameraObject.look(camera).z);
				if (moveBackward) Vec4.addf(Camera.dir, -CameraObject.look(camera).x, -CameraObject.look(camera).y, -CameraObject.look(camera).z);
				if (strafeRight) Vec4.addf(Camera.dir, CameraObject.right(camera).x, CameraObject.right(camera).y, CameraObject.right(camera).z);
				if (strafeLeft) Vec4.addf(Camera.dir, -CameraObject.right(camera).x, -CameraObject.right(camera).y, -CameraObject.right(camera).z);
				if (strafeUp) Vec4.addf(Camera.dir, 0, 0, 1);
				if (strafeDown) Vec4.addf(Camera.dir, 0, 0, -1);
			}
			else {
				Camera.ease -= Time.delta * 20.0 * Camera.ease;
				if (Camera.ease < 0.0) Camera.ease = 0.0;
			}


			let d = Time.delta * fast * Camera.ease * 2.0 * ((moveForward || moveBackward) ? Config.raw.camera_zoom_speed : Config.raw.camera_pan_speed);
			if (d > 0.0) {
				Transform.move(camera.base.transform, Camera.dir, d);
				if (Context.raw.cameraType == CameraType.CameraOrthographic) {
					Viewport.updateCameraType(Context.raw.cameraType);
				}
			}

			Camera.redraws = 2;
			Transform.rotate(camera.base.transform, Vec4.zAxis(), -Mouse.movementX / 200 * Config.raw.camera_rotation_speed);
			Transform.rotate(camera.base.transform, CameraObject.right(camera), -Mouse.movementY / 200 * Config.raw.camera_rotation_speed);
		}

		if (Operator.shortcut(Config.keymap.rotate_light, ShortcutType.ShortcutDown)) {
			Camera.redraws = 2;
			let light = Scene.lights[0];
			Context.raw.lightAngle = (Context.raw.lightAngle + ((Mouse.movementX / 100) % (2 * Math.PI) + 2 * Math.PI)) % (2 * Math.PI);
			let m = Mat4.rotationZ(Mouse.movementX / 100);
			Mat4.multmat(light.base.transform.local, m);
			Transform.decompose(light.base.transform);
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
		return Vec4.distance(Camera.origins[Camera.index()], camera.base.transform.loc);
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
			Camera.origins = [Vec4.create(0, 0, 0), Vec4.create(0, 0, 0)];
			Camera.views = [Mat4.clone(camera.base.transform.local), Mat4.clone(camera.base.transform.local)];
		}
		else {
			Camera.origins[viewIndex] = Vec4.create(0, 0, 0);
			Camera.views[viewIndex] = Mat4.clone(camera.base.transform.local);
		}
	}

	static panAction = (modif: bool, defaultKeymap: bool) => {
		let camera = Scene.camera;
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) || (Mouse.down("middle") && !modif && defaultKeymap)) {
			Camera.redraws = 2;
			let look = Vec4.mult(Vec4.normalize(Transform.look(camera.base.transform)), Mouse.movementY / 150 * Config.raw.camera_pan_speed);
			let right = Vec4.mult(Vec4.normalize(Transform.right(camera.base.transform)), -Mouse.movementX / 150 * Config.raw.camera_pan_speed);
			Vec4.add(camera.base.transform.loc, look);
			Vec4.add(camera.base.transform.loc, right);
			Vec4.add(Camera.origins[Camera.index()], look);
			Vec4.add(Camera.origins[Camera.index()], right);
			CameraObject.buildMatrix(camera);
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
