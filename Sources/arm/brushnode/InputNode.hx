package arm.brushnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;
import iron.math.Vec4;

@:keep
class InputNode extends LogicNode {

	var coords = new Vec4();

	var startX = 0.0;
	var startY = 0.0;
	var lockX = false;
	var lockY = false;

	public function new(tree:LogicTree) {
		super(tree);
		tree.notifyOnUpdate(function() {
			var mouse = iron.system.Input.getMouse();
			coords.x = mouse.x / iron.App.w();
			coords.y = mouse.y / iron.App.h();
			if (mouse.started()) {
				startX = mouse.x / iron.App.w();
				startY = mouse.y / iron.App.h();
			}

			var pen = iron.system.Input.getPen();
			if (pen.down()) {
				coords.x = pen.x / iron.App.w();
				coords.y = pen.y / iron.App.h();
			}
			if (pen.started()) {
				startX = pen.x / iron.App.w();
				startY = pen.y / iron.App.h();
			}

			var kb = iron.system.Input.getKeyboard();
			lockY = kb.down("shift");
			if (lockY) coords.y = startY;
		});
	}

	override function get(from:Int):Dynamic {
		return coords;
	}
}
