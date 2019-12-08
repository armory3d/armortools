package arm.node.brush;

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

	public function new(tree: LogicTree) {
		super(tree);
		tree.notifyOnUpdate(function() {

			if (arm.ui.UITrait.inst.splitView) {
				arm.ui.UITrait.inst.viewIndex = iron.system.Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
			}

			var mouse = iron.system.Input.getMouse();
			coords.x = mouse.viewX / iron.App.w();
			coords.y = mouse.viewY / iron.App.h();
			if (mouse.started()) {
				startX = mouse.viewX / iron.App.w();
				startY = mouse.viewY / iron.App.h();
			}

			var pen = iron.system.Input.getPen();
			if (pen.down()) {
				coords.x = pen.viewX / iron.App.w();
				coords.y = pen.viewY / iron.App.h();
			}
			if (pen.started()) {
				startX = pen.viewX / iron.App.w();
				startY = pen.viewY / iron.App.h();
			}

			if (arm.ui.UITrait.inst.splitView) {
				arm.ui.UITrait.inst.viewIndex = -1;
			}

			if (lockBegin) {
				var dx = Math.abs(lockStartX - mouse.viewX);
				var dy = Math.abs(lockStartY - mouse.viewY);
				if (dx > 1 || dy > 1) {
					lockBegin = false;
					if (dx > dy) lockY = true;
					else lockX = true;
				}
			}

			var kb = iron.system.Input.getKeyboard();
			if (kb.started(Config.keymap.brush_ruler)) { lockStartX = mouse.viewX; lockStartY = mouse.viewY; lockBegin = true; }
			else if (kb.released(Config.keymap.brush_ruler)) { lockX = lockY = lockBegin = false; }

			if (lockX) coords.x = startX;
			if (lockY) coords.y = startY;
		});
	}

	override function get(from: Int): Dynamic {
		return coords;
	}
}
