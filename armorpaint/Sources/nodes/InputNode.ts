
class InputNode extends LogicNode {

	static coords = vec4_create();

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
			app_notify_on_update(this.update);
		}
	}

	update = () => {
		if (Context.raw.split_view) {
			Context.raw.view_index = mouse_view_x() > Base.w() / 2 ? 1 : 0;
		}

		let decal = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		let lazyPaint = Context.raw.brush_lazy_radius > 0 &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 decalMask);

		let paintX = mouse_view_x() / app_w();
		let paintY = mouse_view_y() / app_h();
		if (mouse_started()) {
			InputNode.startX = mouse_view_x() / app_w();
			InputNode.startY = mouse_view_y() / app_h();
		}

		if (pen_down()) {
			paintX = pen_view_x() / app_w();
			paintY = pen_view_y() / app_h();
		}
		if (pen_started()) {
			InputNode.startX = pen_view_x() / app_w();
			InputNode.startY = pen_view_y() / app_h();
		}

		if (Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown)) {
			if (InputNode.lockX) paintX = InputNode.startX;
			if (InputNode.lockY) paintY = InputNode.startY;
		}

		if (Context.raw.brush_lazy_radius > 0) {
			Context.raw.brush_lazy_x = paintX;
			Context.raw.brush_lazy_y = paintY;
		}
		if (!lazyPaint) {
			InputNode.coords.x = paintX;
			InputNode.coords.y = paintY;
		}

		if (Context.raw.split_view) {
			Context.raw.view_index = -1;
		}

		if (InputNode.lockBegin) {
			let dx = Math.abs(InputNode.lockStartX - mouse_view_x());
			let dy = Math.abs(InputNode.lockStartY - mouse_view_y());
			if (dx > 1 || dy > 1) {
				InputNode.lockBegin = false;
				dx > dy ? InputNode.lockY = true : InputNode.lockX = true;
			}
		}

		if (keyboard_started(Config.keymap.brush_ruler)) {
			InputNode.lockStartX = mouse_view_x();
			InputNode.lockStartY = mouse_view_y();
			InputNode.lockBegin = true;
		}
		else if (keyboard_released(Config.keymap.brush_ruler)) {
			InputNode.lockX = InputNode.lockY = InputNode.lockBegin = false;
		}

		if (Context.raw.brush_lazy_radius > 0) {
			let v1 = vec4_create(Context.raw.brush_lazy_x * app_w(), Context.raw.brush_lazy_y * app_h(), 0.0);
			let v2 = vec4_create(InputNode.coords.x * app_w(), InputNode.coords.y * app_h(), 0.0);
			let d = vec4_dist(v1, v2);
			let r = Context.raw.brush_lazy_radius * 85;
			if (d > r) {
				let v3 = vec4_create();
				vec4_sub_vecs(v3, v2, v1);
				vec4_normalize(v3, );
				vec4_mult(v3, 1.0 - Context.raw.brush_lazy_step);
				vec4_mult(v3, r);
				vec4_add_vecs(v2, v1, v3);
				InputNode.coords.x = v2.x / app_w();
				InputNode.coords.y = v2.y / app_h();
				// Parse brush inputs once on next draw
				Context.raw.painted = -1;
			}
			Context.raw.last_paint_x = -1;
			Context.raw.last_paint_y = -1;
		}

		Context.raw.parse_brush_inputs();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((value) => {
			Context.raw.brush_lazy_radius = value;
			this.inputs[1].get((value) => {
				Context.raw.brush_lazy_step = value;
				done(InputNode.coords);
			});
		});
	}

	static def: zui_node_t = {
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
