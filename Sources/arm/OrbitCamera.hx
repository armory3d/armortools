package arm;

class OrbitCamera extends armory.Trait {

	public static var enabled = false;

	public function new() {
		super();
		
		notifyOnUpdate(function() {
			if (iron.system.Input.occupied) return;
			if (!UITrait.uienabled) return;
			if (UITrait.isScrolling) return;
			if (UITrait.isDragging) return;
			if (UITrait.cameraType != 0) return;

			var mouse = armory.system.Input.getMouse();

			if (mouse.x > iron.App.w()) return;
			// if (UINodes.show && mouse.y > UINodes.wy) return;
			
			var keyboard = armory.system.Input.getKeyboard();
			var camera = cast(object, iron.object.CameraObject);

			if (mouse.wheelDelta != 0) {
				UITrait.dirty = true;

				camera.move(camera.look(), mouse.wheelDelta * (-0.1));
			}

			if (mouse.down("middle") || (mouse.down("right") && keyboard.down("space"))) {
				UITrait.dirty = true;

				camera.transform.loc.addf(-mouse.movementX / 150, 0.0, mouse.movementY / 150);
				camera.buildMatrix();
			}
		});
	}
}
