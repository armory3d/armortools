package arm.plugin;

import iron.system.Input;
import iron.system.Time;
import iron.math.Vec4;
import arm.ui.UITrait;
import arm.util.ViewportUtil;

class FlyCamera extends iron.Trait {

	public static var inst:FlyCamera;
	static inline var speed = 2.0;
	var dir = new Vec4();
	var xvec = new Vec4();
	var yvec = new Vec4();
	var ease = 1.0;

	public function new() {
		super();
		inst = this;

		var keyboard = Input.getKeyboard();
		var mouse = Input.getMouse();
		// var gamepad = Input.getGamepad();

		notifyOnUpdate(function() {

			if (Input.occupied ||
				!arm.App.uienabled ||
				arm.App.isDragging  ||
				UITrait.inst.isScrolling ||
				UITrait.inst.cameraControls != 2 ||
				mouse.x < 0 ||
				mouse.x > iron.App.w() ||
				!mouse.down("right")) return;

			var camera = iron.Scene.active.camera;

			var moveForward = keyboard.down("w") || keyboard.down("up") || mouse.wheelDelta < 0;
			var moveBackward = keyboard.down("s") || keyboard.down("down") || mouse.wheelDelta > 0;
			var strafeLeft = keyboard.down("a") || keyboard.down("left");
			var strafeRight = keyboard.down("d") || keyboard.down("right");
			var strafeUp = keyboard.down("e");
			var strafeDown = keyboard.down("q");
			var fast = keyboard.down("shift") ? 2.0 : (keyboard.down("alt") ? 0.5 : 1.0);
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

			var d = Time.delta * speed * fast * ease;
			if (d > 0.0) {
				UITrait.inst.ddirty = 2;
				camera.transform.move(dir, d);

				if (UITrait.inst.cameraType == 1) {
					ViewportUtil.updateCameraType(UITrait.inst.cameraType);
				}
			}

			if (mouse.down("right")) {
				UITrait.inst.ddirty = 2;
				camera.transform.rotate(Vec4.zAxis(), -mouse.movementX / 200);
				camera.transform.rotate(camera.right(), -mouse.movementY / 200);
			}
		});
	}
}
