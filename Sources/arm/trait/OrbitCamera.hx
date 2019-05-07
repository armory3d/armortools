package arm.trait;

import iron.system.Input;
import arm.ui.*;

class OrbitCamera extends iron.Trait {

	public static var inst:OrbitCamera;
	public static var dist = 0.0;
	var redraws = 0;
	var first = true;

	public function new() {
		super();
		inst = this;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();
		
		notifyOnUpdate(function() {

			if (Input.occupied ||
				!arm.App.uienabled ||
				arm.App.isDragging  ||
				UITrait.inst.isScrolling ||
				UITrait.inst.cameraControls != 1 ||
				mouse.x < 0 ||
				mouse.x > iron.App.w()) return;
			
			var camera = iron.Scene.active.camera;

			if (first) {
				first = false;
				reset();
			}

			if (mouse.wheelDelta != 0) {
				redraws = 2;
				var f = mouse.wheelDelta * (-0.1);
				camera.transform.move(camera.look(), f);
				dist -= f;
			}

			if (mouse.down("middle") || (mouse.down("right") && kb.down("space"))) {
				redraws = 2;
				if (kb.down("control")) {
					var f = mouse.movementX / 75;
					camera.transform.move(camera.look(), f);
					dist -= f;
				}
				else {
					var look = camera.transform.look().normalize().mult(mouse.movementY / 150);
					var right = camera.transform.right().normalize().mult(-mouse.movementX / 150);
					camera.transform.loc.add(look);
					camera.transform.loc.add(right);
					camera.buildMatrix();
				}
			}

			if (mouse.down("right") || (mouse.down("left") && kb.down("control"))) {
				redraws = 2;
				camera.transform.move(camera.lookWorld(), dist);
				camera.transform.rotate(new iron.math.Vec4(0, 0, 1), -mouse.movementX / 100);
				
				// Rotate Y
				if (!kb.down("shift")) {
					camera.transform.rotate(camera.rightWorld(), -mouse.movementY / 100);
					if (camera.upWorld().z < 0) {
						camera.transform.rotate(camera.rightWorld(), mouse.movementY / 100);
					}
				}

				camera.transform.move(camera.lookWorld(), -dist);
			}

			if (redraws > 0) {
				redraws--;
				UITrait.inst.ddirty = 2;

				if (UITrait.inst.cameraType == 1) {
					UITrait.inst.updateCameraType(UITrait.inst.cameraType);
				}
			}
		});
	}

	public function reset() {
		var camera = iron.Scene.active.camera;
		dist = camera.transform.loc.length();
	}
}
