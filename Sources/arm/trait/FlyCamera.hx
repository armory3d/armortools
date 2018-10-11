package arm.trait;

import iron.Trait;
import iron.system.Input;
import iron.system.Time;
import iron.object.CameraObject;
import iron.math.Vec4;
import arm.UITrait;

class FlyCamera extends Trait {

	public static var inst:FlyCamera;
	public var enabled = false;

	static inline var speed = 2.0;
	var dir = new Vec4();
	var xvec = new Vec4();
	var yvec = new Vec4();
	var easing = true;
	var ease = 1.0;

	public function new(easing = true) {
		super();
		inst = this;

		this.easing = easing;
		notifyOnUpdate(update);
	}

	function update() {
		if (Input.occupied) return;
		if (!arm.App.uienabled) return;
		if (UITrait.inst.isScrolling) return;
		if (arm.App.isDragging) return;
		if (UITrait.inst.cameraType != 1) return;
		
		var keyboard = Input.getKeyboard();
		var gamepad = Input.getGamepad();
		var mouse = Input.getMouse();
		var camera = iron.Scene.active.camera;

		if (mouse.x > iron.App.w()) return;

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
			if (easing) {
				ease += Time.delta * 15;
				if (ease > 1.0) ease = 1.0;
			}
			else ease = 1.0;
			dir.set(0, 0, 0);
			if (moveForward) dir.addf(camera.look().x, camera.look().y, camera.look().z);
			if (moveBackward) dir.addf(-camera.look().x, -camera.look().y, -camera.look().z);
			if (strafeRight) dir.addf(camera.right().x, camera.right().y, camera.right().z);
			if (strafeLeft) dir.addf(-camera.right().x, -camera.right().y, -camera.right().z);
			if (strafeUp) dir.addf(0, 0, 1);
			if (strafeDown) dir.addf(0, 0, -1);
		}
		else {
			if (easing) {
				ease -= Time.delta * 20.0 * ease;
				if (ease < 0.0) ease = 0.0;
			}
			else ease = 0.0;
		}

		var d = Time.delta * speed * fast * ease;
		if (d > 0.0) {
			UITrait.inst.dirty = 2;
			camera.move(dir, d);
		}

		if (mouse.down("right")) {
			UITrait.inst.dirty = 2;
			camera.rotate(Vec4.zAxis(), -mouse.movementX / 200);
			camera.rotate(camera.right(), -mouse.movementY / 200);
		}
	}
}
