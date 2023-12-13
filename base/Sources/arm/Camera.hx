package arm;

import iron.App;
import iron.Input;
import iron.Time;
import iron.Vec4;
import iron.Mat4;
import iron.Scene;
import arm.Viewport;

class Camera {

	public static var inst: Camera;
	public var origins: Array<Vec4>;
	public var views: Array<Mat4>;
	var redraws = 0;
	var first = true;
	var dir = new Vec4();
	var ease = 1.0;
	var controlsDown = false;

	public function new() {
		inst = this;
	}

	public function update() {
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();
		var camera = Scene.active.camera;

		if (first) {
			first = false;
			reset();
		}

		if (mouse.viewX < 0 ||
			mouse.viewX > App.w() ||
			mouse.viewY < 0 ||
			mouse.viewY > App.h()) {

			if (Config.raw.wrap_mouse && controlsDown) {
				if (mouse.viewX < 0) {
					mouse.x = mouse.lastX = App.x() + App.w();
					Krom.setMousePosition(Std.int(mouse.x), Std.int(mouse.y));
				}
				else if (mouse.viewX > App.w()) {
					mouse.x = mouse.lastX = App.x();
					Krom.setMousePosition(Std.int(mouse.x), Std.int(mouse.y));
				}
				else if (mouse.viewY < 0) {
					mouse.y = mouse.lastY = App.y() + App.h();
					Krom.setMousePosition(Std.int(mouse.x), Std.int(mouse.y));
				}
				else if (mouse.viewY > App.h()) {
					mouse.y = mouse.lastY = App.y();
					Krom.setMousePosition(Std.int(mouse.x), Std.int(mouse.y));
				}
			}
			else {
				return;
			}
		}

		var modifKey = kb.down("alt") || kb.down("shift") || kb.down("control");
		var modif = modifKey || Config.keymap.action_rotate == "middle";
		var defaultKeymap = Config.raw.keymap == "default.json";

		if (Operator.shortcut(Config.keymap.action_rotate, ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_pan, ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_envmap, ShortcutStarted) ||
			Operator.shortcut(Config.keymap.rotate_light, ShortcutStarted) ||
			(mouse.started("right") && !modif) ||
			(mouse.started("middle") && !modif) ||
			(mouse.wheelDelta != 0 && !modifKey)) {
			controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_rotate, ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_pan, ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_envmap, ShortcutDown) &&
			!Operator.shortcut(Config.keymap.rotate_light, ShortcutDown) &&
			!(mouse.down("right") && !modif) &&
			!(mouse.down("middle") && !modif) &&
			(mouse.wheelDelta == 0 && !modifKey)) {
			controlsDown = false;
		}

		if (Input.occupied ||
			!Base.uiEnabled ||
			Base.isDragging ||
			Base.isScrolling() ||
			Base.isComboSelected() ||
			!controlsDown) {
			return;
		}

		var controls = Context.raw.cameraControls;
		if (controls == ControlsOrbit && (Operator.shortcut(Config.keymap.action_rotate, ShortcutDown) || (mouse.down("right") && !modif && defaultKeymap))) {
			redraws = 2;
			var dist = distance();
			camera.transform.move(camera.lookWorld(), dist);
			camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			camera.transform.rotate(camera.rightWorld(), -mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			if (camera.upWorld().z < 0) {
				camera.transform.rotate(camera.rightWorld(), mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
			camera.transform.move(camera.lookWorld(), -dist);
		}
		else if (controls == ControlsRotate && (Operator.shortcut(Config.keymap.action_rotate, ShortcutDown) || (mouse.down("right") && !modif && defaultKeymap))) {
			redraws = 2;
			var t = Context.mainObject().transform;
			var up = t.up().normalize();
			t.rotate(up, mouse.movementX / 100 * Config.raw.camera_rotation_speed);
			var right = camera.rightWorld().normalize();
			t.rotate(right, mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			t.buildMatrix();
			if (t.up().z < 0) {
				t.rotate(right, -mouse.movementY / 100 * Config.raw.camera_rotation_speed);
			}
		}

		if (controls == ControlsRotate || controls == ControlsOrbit) {
			panAction(modif, defaultKeymap);

			if (Operator.shortcut(Config.keymap.action_zoom, ShortcutDown)) {
				redraws = 2;
				var f = getZoomDelta() / 150;
				f *= getCameraZoomSpeed();
				camera.transform.move(camera.look(), f);
			}

			if (mouse.wheelDelta != 0 && !modifKey) {
				redraws = 2;
				var f = mouse.wheelDelta * (-0.1);
				f *= getCameraZoomSpeed();
				camera.transform.move(camera.look(), f);
			}
		}
		else if (controls == ControlsFly && mouse.down("right")) {
			var moveForward = kb.down("w") || kb.down("up") || mouse.wheelDelta < 0;
			var moveBackward = kb.down("s") || kb.down("down") || mouse.wheelDelta > 0;
			var strafeLeft = kb.down("a") || kb.down("left");
			var strafeRight = kb.down("d") || kb.down("right");
			var strafeUp = kb.down("e");
			var strafeDown = kb.down("q");
			var fast = kb.down("shift") ? 2.0 : (kb.down("alt") ? 0.5 : 1.0);
			if (mouse.wheelDelta != 0) {
				fast *= Math.abs(mouse.wheelDelta) * 4.0;
			}

			if (moveForward || moveBackward || strafeRight || strafeLeft || strafeUp || strafeDown) {
				ease += Time.delta * 15;
				if (ease > 1.0) ease = 1.0;
				dir.set(0, 0, 0);
				if (moveForward) dir.addf(camera.look().x, camera.look().y, camera.look().z);
				if (moveBackward) dir.addf(-camera.look().x, -camera.look().y, -camera.look().z);
				if (strafeRight) dir.addf(camera.right().x, camera.right().y, camera.right().z);
				if (strafeLeft) dir.addf(-camera.right().x, -camera.right().y, -camera.right().z);
				if (strafeUp) dir.addf(0, 0, 1);
				if (strafeDown) dir.addf(0, 0, -1);
			}
			else {
				ease -= Time.delta * 20.0 * ease;
				if (ease < 0.0) ease = 0.0;
			}


			var d = Time.delta * fast * ease * 2.0 * ((moveForward || moveBackward) ? Config.raw.camera_zoom_speed : Config.raw.camera_pan_speed);
			if (d > 0.0) {
				camera.transform.move(dir, d);
				if (Context.raw.cameraType == CameraOrthographic) {
					Viewport.updateCameraType(Context.raw.cameraType);
				}
			}

			redraws = 2;
			camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 200 * Config.raw.camera_rotation_speed);
			camera.transform.rotate(camera.right(), -mouse.movementY / 200 * Config.raw.camera_rotation_speed);
		}

		if (Operator.shortcut(Config.keymap.rotate_light, ShortcutDown)) {
			redraws = 2;
			var light = Scene.active.lights[0];
			Context.raw.lightAngle = (Context.raw.lightAngle + ((mouse.movementX / 100) % (2 * Math.PI) + 2 * Math.PI)) % (2 * Math.PI);
			var m = Mat4.rotationZ(mouse.movementX / 100);
			light.transform.local.multmat(m);
			light.transform.decompose();
		}

		if (Operator.shortcut(Config.keymap.rotate_envmap, ShortcutDown)) {
			redraws = 2;
			Context.raw.envmapAngle -= mouse.movementX / 100;
		}

		if (redraws > 0) {
			redraws--;
			Context.raw.ddirty = 2;

			if (Context.raw.cameraType == CameraOrthographic) {
				Viewport.updateCameraType(Context.raw.cameraType);
			}
		}
	}

	public function distance(): Float {
		var camera = Scene.active.camera;
		return Vec4.distance(origins[index()], camera.transform.loc);
	}

	public function index(): Int {
		return Context.raw.viewIndexLast > 0 ? 1 : 0;
	}

	function getCameraZoomSpeed(): Float {
		var sign = Config.raw.zoom_direction == ZoomVerticalInverted ||
				   Config.raw.zoom_direction == ZoomHorizontalInverted ||
				   Config.raw.zoom_direction == ZoomVerticalAndHorizontalInverted ? -1 : 1;
		return Config.raw.camera_zoom_speed * sign;
	}

	public function reset(viewIndex = -1) {
		var camera = Scene.active.camera;
		if (viewIndex == -1) {
			origins = [new Vec4(0, 0, 0), new Vec4(0, 0, 0)];
			views = [camera.transform.local.clone(), camera.transform.local.clone()];
		}
		else {
			origins[viewIndex] = new Vec4(0, 0, 0);
			views[viewIndex] = camera.transform.local.clone();
		}
	}

	function panAction(modif: Bool, defaultKeymap: Bool) {
		var camera = Scene.active.camera;
		var mouse = Input.getMouse();
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutDown) || (mouse.down("middle") && !modif && defaultKeymap)) {
			redraws = 2;
			var look = camera.transform.look().normalize().mult(mouse.movementY / 150 * Config.raw.camera_pan_speed);
			var right = camera.transform.right().normalize().mult(-mouse.movementX / 150 * Config.raw.camera_pan_speed);
			camera.transform.loc.add(look);
			camera.transform.loc.add(right);
			origins[index()].add(look);
			origins[index()].add(right);
			camera.buildMatrix();
		}
	}

	static function getZoomDelta(): Float {
		var mouse = Input.getMouse();
		return Config.raw.zoom_direction == ZoomVertical ? -mouse.movementY :
			   Config.raw.zoom_direction == ZoomVerticalInverted ? -mouse.movementY :
			   Config.raw.zoom_direction == ZoomHorizontal ? mouse.movementX :
			   Config.raw.zoom_direction == ZoomHorizontalInverted ? mouse.movementX :
			   -(mouse.movementY - mouse.movementX);
	}
}
