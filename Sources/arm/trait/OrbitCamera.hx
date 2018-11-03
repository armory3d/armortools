package arm.trait;

import arm.UITrait;

class OrbitCamera extends iron.Trait {

	public static var inst:OrbitCamera;

	var redraws = 0;

	public function new() {
		super();
		inst = this;
		
		notifyOnUpdate(function() {
			if (iron.system.Input.occupied) return;
			if (!arm.App.uienabled) return;
			if (UITrait.inst.isScrolling) return;
			if (arm.App.isDragging) return;
			if (UITrait.inst.cameraControls != 1) return;

			var mouse = iron.system.Input.getMouse();
			if (mouse.x < 0 || mouse.x > iron.App.w()) return;
			
			var kb = iron.system.Input.getKeyboard();
			var camera = iron.Scene.active.camera;

			if (mouse.wheelDelta != 0) {
				redraws = 2;
				camera.transform.move(camera.look(), mouse.wheelDelta * (-0.1));
			}

			if (mouse.down("middle") || (mouse.down("right") && kb.down("space"))) {
				redraws = 2;
				camera.transform.loc.addf(-mouse.movementX / 150, 0.0, mouse.movementY / 150);
				camera.buildMatrix();
			}

			if (mouse.down("right") || (mouse.down("left") && kb.down("control"))) {
				redraws = 2;
				
				// Rotate X
				// if (!kb.down("alt")) {
					var d = camera.transform.loc.length();
					camera.transform.loc.set(0, 0, 0);
					camera.transform.rotate(new iron.math.Vec4(0, 0, 1), -mouse.movementX / 100);
					camera.transform.move(camera.lookWorld(), -d);
				// }
				
				// Rotate Y
				if (!kb.down("shift")) {
					var d = camera.transform.loc.length();
					camera.transform.loc.set(0, 0, 0);
					camera.transform.rotate(camera.rightWorld(), -mouse.movementY / 100);

					if (camera.upWorld().z < 0) {
						camera.transform.rotate(camera.rightWorld(), mouse.movementY / 100);
					}
					
					camera.transform.move(camera.lookWorld(), -d);
				}
			}

			if (redraws > 0) {
				redraws--;
				UITrait.inst.ddirty = 2;
			}
		});
	}
}
