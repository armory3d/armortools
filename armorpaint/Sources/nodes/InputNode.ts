
// @:keep
class InputNode extends LogicNode {

	static coords = new Vec4();

	static startX = 0.0;
	static startY = 0.0;

	// Brush ruler
	static lockBegin = false;
	static lockX = false;
	static lockY = false;
	static lockStartX = 0.0;
	static lockStartY = 0.0;

	static registered = false;

	constructor() {
		super();

		if (!InputNode.registered) {
			InputNode.registered = true;
			App.notifyOnUpdate(this.update);
		}
	}

	update = () => {
		if (Context.raw.splitView) {
			Context.raw.viewIndex = Input.getMouse().viewX > Base.w() / 2 ? 1 : 0;
		}

		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		let lazyPaint = Context.raw.brushLazyRadius > 0 &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 decalMask);

		let mouse = Input.getMouse();
		let paintX = mouse.viewX / App.w();
		let paintY = mouse.viewY / App.h();
		if (mouse.started()) {
			InputNode.startX = mouse.viewX / App.w();
			InputNode.startY = mouse.viewY / App.h();
		}

		let pen = Input.getPen();
		if (pen.down()) {
			paintX = pen.viewX / App.w();
			paintY = pen.viewY / App.h();
		}
		if (pen.started()) {
			InputNode.startX = pen.viewX / App.w();
			InputNode.startY = pen.viewY / App.h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown)) {
			if (InputNode.lockX) paintX = InputNode.startX;
			if (InputNode.lockY) paintY = InputNode.startY;
		}

		if (Context.raw.brushLazyRadius > 0) {
			Context.raw.brushLazyX = paintX;
			Context.raw.brushLazyY = paintY;
		}
		if (!lazyPaint) {
			InputNode.coords.x = paintX;
			InputNode.coords.y = paintY;
		}

		if (Context.raw.splitView) {
			Context.raw.viewIndex = -1;
		}

		if (InputNode.lockBegin) {
			let dx = Math.abs(InputNode.lockStartX - mouse.viewX);
			let dy = Math.abs(InputNode.lockStartY - mouse.viewY);
			if (dx > 1 || dy > 1) {
				InputNode.lockBegin = false;
				dx > dy ? InputNode.lockY = true : InputNode.lockX = true;
			}
		}

		let kb = Input.getKeyboard();
		if (kb.started(Config.keymap.brush_ruler)) {
			InputNode.lockStartX = mouse.viewX;
			InputNode.lockStartY = mouse.viewY;
			InputNode.lockBegin = true;
		}
		else if (kb.released(Config.keymap.brush_ruler)) {
			InputNode.lockX = InputNode.lockY = InputNode.lockBegin = false;
		}

		if (Context.raw.brushLazyRadius > 0) {
			let v1 = new Vec4(Context.raw.brushLazyX * App.w(), Context.raw.brushLazyY * App.h(), 0.0);
			let v2 = new Vec4(InputNode.coords.x * App.w(), InputNode.coords.y * App.h(), 0.0);
			let d = Vec4.distance(v1, v2);
			let r = Context.raw.brushLazyRadius * 85;
			if (d > r) {
				let v3 = new Vec4();
				v3.subvecs(v2, v1);
				v3.normalize();
				v3.mult(1.0 - Context.raw.brushLazyStep);
				v3.mult(r);
				v2.addvecs(v1, v3);
				InputNode.coords.x = v2.x / App.w();
				InputNode.coords.y = v2.y / App.h();
				// Parse brush inputs once on next draw
				Context.raw.painted = -1;
			}
			Context.raw.lastPaintX = -1;
			Context.raw.lastPaintY = -1;
		}

		Context.raw.parseBrushInputs();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((value) => {
			Context.raw.brushLazyRadius = value;
			this.inputs[1].get((value) => {
				Context.raw.brushLazyStep = value;
				done(InputNode.coords);
			});
		});
	}

	static def: TNode = {
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
