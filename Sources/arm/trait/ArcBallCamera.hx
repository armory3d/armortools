package arm.trait;

import arm.UITrait;

class ArcBallCamera extends iron.Trait {

	public static var inst:ArcBallCamera;

	var redraws = 0;

	public function new() {
		super();
		inst = this;
		
		notifyOnUpdate(function() {
			if (iron.system.Input.occupied) return;
			if (!arm.App.uienabled) return;
			if (UITrait.inst.isScrolling) return;
			if (arm.App.isDragging) return;
			if (UITrait.inst.cameraType != 0) return;

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
				if (!kb.down("alt")) {
					var v = UITrait.inst.selectedObject.transform.up();
					v.normalize();
					UITrait.inst.selectedObject.transform.rotate(v, mouse.movementX / 100);
				}
				
				// Rotate Y
				if (!kb.down("shift")) {
					var v = camera.rightWorld();
					v.normalize();
					UITrait.inst.selectedObject.transform.rotate(v, mouse.movementY / 100);
					UITrait.inst.selectedObject.transform.buildMatrix();

					if (UITrait.inst.selectedObject.transform.up().z < 0) {
						UITrait.inst.selectedObject.transform.rotate(v, -mouse.movementY / 100);
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
