package arm.trait;

import iron.system.Input;
import arm.ui.*;

class OrbitCamera extends iron.Trait {

	public static var inst:OrbitCamera;
	var redraws = 0;

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

			if (mouse.wheelDelta != 0) {
				redraws = 2;
				camera.transform.move(camera.look(), mouse.wheelDelta * (-0.1));
			}

			if (mouse.down("middle") || (mouse.down("right") && kb.down("space"))) {
				redraws = 2;
				if (kb.down("control")) {
					camera.transform.move(camera.look(), mouse.movementX / 75);
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
				var dist = camera.transform.loc.length();
				redraws = 2;
				camera.transform.move(camera.lookWorld(), dist);
				camera.transform.rotate(new iron.math.Vec4(0, 0, 1), -mouse.movementX / 100);
				camera.transform.move(camera.lookWorld(), -dist);
				
				// Rotate Y
				if (!kb.down("shift")) {
					camera.transform.move(camera.lookWorld(), dist);
					camera.transform.rotate(camera.rightWorld(), -mouse.movementY / 100);

					if (camera.upWorld().z < 0) {
						camera.transform.rotate(camera.rightWorld(), mouse.movementY / 100);
					}
					
					camera.transform.move(camera.lookWorld(), -dist);
				}
			}

			if (redraws > 0) {
				redraws--;
				UITrait.inst.ddirty = 2;
			}
		});
	}
}
