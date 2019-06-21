package arm.nodes.brush;

import iron.math.Vec4;

@:keep
class InputNode extends LogicNode {

	var coords = new Vec4();

	var startX = 0.0;
	var startY = 0.0;

	// Brush ruler
	var lockBegin = false;
	var lockX = false;
	var lockY = false;
	var lockStartX = 0.0;
	var lockStartY = 0.0;

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

			if (lockBegin) {
				var dx = Math.abs(lockStartX - mouse.x);
				var dy = Math.abs(lockStartY - mouse.y);
				if (dx > 1 || dy > 1) {
					lockBegin = false;
					if (dx > dy) lockY = true;
					else lockX = true;
				}
			}

			var kb = iron.system.Input.getKeyboard();
			if (kb.started(App.K.brush_ruler)) { lockStartX = mouse.x; lockStartY = mouse.y; lockBegin = true; }
			else if (kb.released(App.K.brush_ruler)) { lockX = lockY = lockBegin = false; }

			if (lockX) coords.x = startX;
			if (lockY) coords.y = startY;
		});
	}

	override function get(from:Int):Dynamic {
		return coords;
	}
}
