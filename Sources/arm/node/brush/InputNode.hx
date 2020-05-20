package arm.node.brush;

import iron.math.Vec4;
import arm.ui.UISidebar;

@:keep
class InputNode extends LogicNode {

	static var coords = new Vec4();

	static var startX = 0.0;
	static var startY = 0.0;

	// Brush ruler
	static var lockBegin = false;
	static var lockX = false;
	static var lockY = false;
	static var lockStartX = 0.0;
	static var lockStartY = 0.0;

	static var registered = false;

	public function new(tree: LogicTree) {
		super(tree);

		if (!registered) {
			registered = true;
			tree.notifyOnUpdate(update);
		}
	}

	function update() {
		if (Context.splitView) {
			Context.viewIndex = iron.system.Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
		}

		var lazyPaint = Context.brushLazyRadius > 0 &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown));

		var mouse = iron.system.Input.getMouse();
		var paintX = mouse.viewX / iron.App.w();
		var paintY = mouse.viewY / iron.App.h();
		if (mouse.started()) {
			startX = mouse.viewX / iron.App.w();
			startY = mouse.viewY / iron.App.h();
		}

		var pen = iron.system.Input.getPen();
		if (pen.down()) {
			paintX = pen.viewX / iron.App.w();
			paintY = pen.viewY / iron.App.h();
		}
		if (pen.started()) {
			startX = pen.viewX / iron.App.w();
			startY = pen.viewY / iron.App.h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
			if (lockX) paintX = startX;
			if (lockY) paintY = startY;
		}

		if (Context.brushLazyRadius > 0) {
			Context.brushLazyX = paintX;
			Context.brushLazyY = paintY;
		}
		if (!lazyPaint) {
			coords.x = paintX;
			coords.y = paintY;
		}

		if (Context.splitView) {
			Context.viewIndex = -1;
		}

		if (lockBegin) {
			var dx = Math.abs(lockStartX - mouse.viewX);
			var dy = Math.abs(lockStartY - mouse.viewY);
			if (dx > 1 || dy > 1) {
				lockBegin = false;
				dx > dy ? lockY = true : lockX = true;
			}
		}

		var kb = iron.system.Input.getKeyboard();
		if (kb.started(Config.keymap.brush_ruler)) { lockStartX = mouse.viewX; lockStartY = mouse.viewY; lockBegin = true; }
		else if (kb.released(Config.keymap.brush_ruler)) { lockX = lockY = lockBegin = false; }

		if (Context.brushLazyRadius > 0) {
			var v1 = new Vec4(Context.brushLazyX * iron.App.w(), Context.brushLazyY * iron.App.h(), 0.0);
			var v2 = new Vec4(coords.x * iron.App.w(), coords.y * iron.App.h(), 0.0);
			var d = Vec4.distance(v1, v2);
			var r = Context.brushLazyRadius * 85;
			if (d > r) {
				var v3 = new Vec4();
				v3.subvecs(v2, v1);
				v3.normalize();
				v3.mult(1.0 - Context.brushLazyStep);
				v3.mult(r);
				v2.addvecs(v1, v3);
				coords.x = v2.x / iron.App.w();
				coords.y = v2.y / iron.App.h();
				// Parse brush inputs once on next draw
				Context.painted = -1;
			}
			Context.lastPaintX = -1;
			Context.lastPaintY = -1;
		}
	}

	override function get(from: Int): Dynamic {
		Context.brushLazyRadius = inputs[0].get();
		Context.brushLazyStep = inputs[1].get();
		return coords;
	}
}
