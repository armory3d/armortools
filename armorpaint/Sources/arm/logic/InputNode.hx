package arm.logic;

import iron.App;
import iron.Input;
import iron.Vec4;
import zui.Zui.Nodes;
import zui.Zui.TNode;
import arm.logic.LogicNode;
import arm.Translator._tr;

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

	public function new() {
		super();

		if (!registered) {
			registered = true;
			App.notifyOnUpdate(update);
		}
	}

	function update() {
		if (Context.raw.splitView) {
			Context.raw.viewIndex = Input.getMouse().viewX > Base.w() / 2 ? 1 : 0;
		}

		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);

		var lazyPaint = Context.raw.brushLazyRadius > 0 &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
			 decalMask);

		var mouse = Input.getMouse();
		var paintX = mouse.viewX / App.w();
		var paintY = mouse.viewY / App.h();
		if (mouse.started()) {
			startX = mouse.viewX / App.w();
			startY = mouse.viewY / App.h();
		}

		var pen = Input.getPen();
		if (pen.down()) {
			paintX = pen.viewX / App.w();
			paintY = pen.viewY / App.h();
		}
		if (pen.started()) {
			startX = pen.viewX / App.w();
			startY = pen.viewY / App.h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown)) {
			if (lockX) paintX = startX;
			if (lockY) paintY = startY;
		}

		if (Context.raw.brushLazyRadius > 0) {
			Context.raw.brushLazyX = paintX;
			Context.raw.brushLazyY = paintY;
		}
		if (!lazyPaint) {
			coords.x = paintX;
			coords.y = paintY;
		}

		if (Context.raw.splitView) {
			Context.raw.viewIndex = -1;
		}

		if (lockBegin) {
			var dx = Math.abs(lockStartX - mouse.viewX);
			var dy = Math.abs(lockStartY - mouse.viewY);
			if (dx > 1 || dy > 1) {
				lockBegin = false;
				dx > dy ? lockY = true : lockX = true;
			}
		}

		var kb = Input.getKeyboard();
		if (kb.started(Config.keymap.brush_ruler)) {
			lockStartX = mouse.viewX;
			lockStartY = mouse.viewY;
			lockBegin = true;
		}
		else if (kb.released(Config.keymap.brush_ruler)) {
			lockX = lockY = lockBegin = false;
		}

		if (Context.raw.brushLazyRadius > 0) {
			var v1 = new Vec4(Context.raw.brushLazyX * App.w(), Context.raw.brushLazyY * App.h(), 0.0);
			var v2 = new Vec4(coords.x * App.w(), coords.y * App.h(), 0.0);
			var d = Vec4.distance(v1, v2);
			var r = Context.raw.brushLazyRadius * 85;
			if (d > r) {
				var v3 = new Vec4();
				v3.subvecs(v2, v1);
				v3.normalize();
				v3.mult(1.0 - Context.raw.brushLazyStep);
				v3.mult(r);
				v2.addvecs(v1, v3);
				coords.x = v2.x / App.w();
				coords.y = v2.y / App.h();
				// Parse brush inputs once on next draw
				Context.raw.painted = -1;
			}
			Context.raw.lastPaintX = -1;
			Context.raw.lastPaintY = -1;
		}

		Context.raw.parseBrushInputs();
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(value) {
			Context.raw.brushLazyRadius = value;
			inputs[1].get(function(value) {
				Context.raw.brushLazyStep = value;
				done(coords);
			});
		});
	}

	public static var def: TNode = {
		id: 0,
		name: _tr("Input"),
		type: "InputNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Lazy Radius"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Lazy Step"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Position"),
				type: "VECTOR",
				color: 0xff63c763,
				default_value: null
			}
		],
		buttons: []
	};
}
