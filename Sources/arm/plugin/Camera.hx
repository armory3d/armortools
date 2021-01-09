package arm.plugin;

import iron.system.Input;
import iron.system.Time;
import iron.math.Vec4;
import iron.math.Mat4;
import arm.ui.UISidebar;
import arm.util.ViewportUtil;
import arm.Enums;

class Camera {

	public static var inst: Camera;
	public var origins: Array<Vec4>;
	public var views: Array<Mat4>;
	var redraws = 0;
	var first = true;
	var dir = new Vec4();
	var ease = 1.0;

	public function new() {
		inst = this;
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();
		var camera = iron.Scene.active.camera;

		iron.App.notifyOnUpdate(function() {

			if (first) {
				first = false;
				reset();
			}

			if (Input.occupied ||
				!App.uiEnabled ||
				App.isDragging  ||
				UISidebar.inst.isScrolling ||
				mouse.viewX < 0 ||
				mouse.viewX > iron.App.w() ||
				mouse.viewY < 0 ||
				mouse.viewY > iron.App.h()) {
				return;
			}

			var modifKey = kb.down("alt") || kb.down("shift") || kb.down("control");
			var modif = modifKey || Config.keymap.action_rotate == "middle";
			var controls = Context.cameraControls;
			if (controls == ControlsOrbit) {
				if (Operator.shortcut(Config.keymap.action_rotate, ShortcutDown) || (mouse.down("right") && !modif)) {
					redraws = 2;
					var dist = distance();
					camera.transform.move(camera.lookWorld(), dist);
					camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 100);
					camera.transform.rotate(camera.rightWorld(), -mouse.movementY / 100);
					if (camera.upWorld().z < 0) {
						camera.transform.rotate(camera.rightWorld(), mouse.movementY / 100);
					}
					camera.transform.move(camera.lookWorld(), -dist);
				}

				panAction(modif);

				if (Operator.shortcut(Config.keymap.action_zoom, ShortcutDown)) {
					redraws = 2;
					var f = getZoomDelta() / 150;
					f *= getCameraSpeed();
					camera.transform.move(camera.look(), f);
				}

				if (mouse.wheelDelta != 0 && !modifKey) {
					redraws = 2;
					var f = mouse.wheelDelta * (-0.1);
					f *= getCameraSpeed();
					camera.transform.move(camera.look(), f);
				}

				if (Operator.shortcut(Config.keymap.rotate_light, ShortcutDown)) {
					redraws = 2;
					var light = iron.Scene.active.lights[0];
					var m = iron.math.Mat4.identity();
					m.self = kha.math.FastMatrix4.rotationZ(mouse.movementX / 100);
					light.transform.local.multmat(m);
					light.transform.decompose();
				}

				if (Operator.shortcut(Config.keymap.rotate_envmap, ShortcutDown)) {
					redraws = 2;
					Context.envmapAngle -= mouse.movementX / 100;
				}
			}
			else if (controls == ControlsRotate) {
				if (Operator.shortcut(Config.keymap.action_rotate, ShortcutDown) || (mouse.down("right") && !modif)) {
					redraws = 2;
					var t = Context.mainObject().transform;
					var up = t.up().normalize();
					t.rotate(up, mouse.movementX / 100);
					var right = camera.rightWorld().normalize();
					t.rotate(right, mouse.movementY / 100);
					t.buildMatrix();
					if (t.up().z < 0) {
						t.rotate(right, -mouse.movementY / 100);
					}
				}

				panAction(modif);

				if (Operator.shortcut(Config.keymap.action_zoom, ShortcutDown)) {
					redraws = 2;
					var f = getZoomDelta() / 150;
					f *= getCameraSpeed();
					camera.transform.move(camera.look(), f);
				}

				if (mouse.wheelDelta != 0) {
					redraws = 2;
					var f = mouse.wheelDelta * (-0.1);
					f *= getCameraSpeed();
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

				var d = Time.delta * fast * ease * 2.0 * Config.raw.camera_speed;
				if (d > 0.0) {
					camera.transform.move(dir, d);
					if (Context.cameraType == CameraOrthographic) {
						ViewportUtil.updateCameraType(Context.cameraType);
					}
				}

				redraws = 2;
				camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 200);
				camera.transform.rotate(camera.right(), -mouse.movementY / 200);
			}

			if (redraws > 0) {
				redraws--;
				Context.ddirty = 2;

				if (Context.cameraType == CameraOrthographic) {
					ViewportUtil.updateCameraType(Context.cameraType);
				}
			}
		});
	}

	public function distance(): Float {
		var camera = iron.Scene.active.camera;
		return Vec4.distance(origins[index()], camera.transform.loc);
	}

	public function index(): Int {
		return Context.viewIndexLast > 0 ? 1 : 0;
	}

	function getCameraSpeed(): Float {
		var sign = Config.raw.zoom_direction == ZoomVerticalInverted ||
				   Config.raw.zoom_direction == ZoomHorizontalInverted ||
				   Config.raw.zoom_direction == ZoomVerticalAndHorizontalInverted ? -1 : 1;
		return Config.raw.camera_speed * sign;
	}

	public function reset(viewIndex = -1) {
		var camera = iron.Scene.active.camera;
		if (viewIndex == -1) {
			origins = [new Vec4(0, 0, 0), new Vec4(0, 0, 0)];
			views = [camera.transform.local.clone(), camera.transform.local.clone()];
		}
		else {
			origins[viewIndex] = new Vec4(0, 0, 0);
			views[viewIndex] = camera.transform.local.clone();
		}
	}

	function panAction(modif: Bool) {
		var camera = iron.Scene.active.camera;
		var mouse = Input.getMouse();
		if (Operator.shortcut(Config.keymap.action_pan, ShortcutDown) || (mouse.down("middle") && !modif)) {
			redraws = 2;
			var look = camera.transform.look().normalize().mult(mouse.movementY / 150);
			var right = camera.transform.right().normalize().mult(-mouse.movementX / 150);
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
