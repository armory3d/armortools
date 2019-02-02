package arm.trait;

import iron.system.Input;
import arm.UITrait;

class ArcBallCamera extends iron.Trait {

	public static var inst:ArcBallCamera;
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
				UITrait.inst.cameraControls != 0 ||
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
					camera.transform.loc.addf(-mouse.movementX / 150, 0.0, mouse.movementY / 150);
					camera.buildMatrix();
				}
			}

			if ((mouse.down("right") && !kb.down("space")) || (mouse.down("left") && kb.down("control"))) {
				redraws = 2;
				var t = UITrait.inst.selectedObject.transform;

				// Rotate X
				var up = t.up().normalize();
				t.rotate(up, mouse.movementX / 100);
				
				// Rotate Y
				if (!kb.down("shift")) {
					var right = camera.rightWorld().normalize();
					t.rotate(right, mouse.movementY / 100);
					t.buildMatrix();

					if (t.up().z < 0) {
						t.rotate(right, -mouse.movementY / 100);
					}
				}
			}

			if (redraws > 0) {
				redraws--;
				UITrait.inst.ddirty = 2;
			}
		});
	}
}
