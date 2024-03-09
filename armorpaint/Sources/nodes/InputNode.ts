
class InputNode extends LogicNode {

	static coords: vec4_t = vec4_create();

	static startX: f32 = 0.0;
	static startY: f32 = 0.0;

	// Brush ruler
	static lock_begin: bool = false;
	static lock_x: bool = false;
	static lock_y: bool = false;
	static lock_start_x: f32 = 0.0;
	static lock_start_y: f32 = 0.0;

	static registered: bool = false;

	constructor() {
		super();

		if (!InputNode.registered) {
			InputNode.registered = true;
			app_notify_on_update(this.update);
		}
	}

	update = () => {
		if (context_raw.split_view) {
			context_raw.view_index = mouse_view_x() > base_w() / 2 ? 1 : 0;
		}

		let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
		let decal_mask: bool = decal && operator_shortcut(config_keymap.decal_mask + "+" + config_keymap.action_paint, shortcut_type_t.DOWN);

		let lazy_paint: bool = context_raw.brush_lazy_radius > 0 &&
			(operator_shortcut(config_keymap.action_paint, shortcut_type_t.DOWN) ||
			 operator_shortcut(config_keymap.brush_ruler + "+" + config_keymap.action_paint, shortcut_type_t.DOWN) ||
			 decal_mask);

		let paint_x: f32 = mouse_view_x() / app_w();
		let paint_y: f32 = mouse_view_y() / app_h();
		if (mouse_started()) {
			InputNode.startX = mouse_view_x() / app_w();
			InputNode.startY = mouse_view_y() / app_h();
		}

		if (pen_down()) {
			paint_x = pen_view_x() / app_w();
			paint_y = pen_view_y() / app_h();
		}
		if (pen_started()) {
			InputNode.startX = pen_view_x() / app_w();
			InputNode.startY = pen_view_y() / app_h();
		}

		if (operator_shortcut(config_keymap.brush_ruler + "+" + config_keymap.action_paint, shortcut_type_t.DOWN)) {
			if (InputNode.lock_x) paint_x = InputNode.startX;
			if (InputNode.lock_y) paint_y = InputNode.startY;
		}

		if (context_raw.brush_lazy_radius > 0) {
			context_raw.brush_lazy_x = paint_x;
			context_raw.brush_lazy_y = paint_y;
		}
		if (!lazy_paint) {
			InputNode.coords.x = paint_x;
			InputNode.coords.y = paint_y;
		}

		if (context_raw.split_view) {
			context_raw.view_index = -1;
		}

		if (InputNode.lock_begin) {
			let dx: f32 = Math.abs(InputNode.lock_start_x - mouse_view_x());
			let dy: f32 = Math.abs(InputNode.lock_start_y - mouse_view_y());
			if (dx > 1 || dy > 1) {
				InputNode.lock_begin = false;
				dx > dy ? InputNode.lock_y = true : InputNode.lock_x = true;
			}
		}

		if (keyboard_started(config_keymap.brush_ruler)) {
			InputNode.lock_start_x = mouse_view_x();
			InputNode.lock_start_y = mouse_view_y();
			InputNode.lock_begin = true;
		}
		else if (keyboard_released(config_keymap.brush_ruler)) {
			InputNode.lock_x = InputNode.lock_y = InputNode.lock_begin = false;
		}

		if (context_raw.brush_lazy_radius > 0) {
			let v1: vec4_t = vec4_create(context_raw.brush_lazy_x * app_w(), context_raw.brush_lazy_y * app_h(), 0.0);
			let v2: vec4_t = vec4_create(InputNode.coords.x * app_w(), InputNode.coords.y * app_h(), 0.0);
			let d: f32 = vec4_dist(v1, v2);
			let r: f32 = context_raw.brush_lazy_radius * 85;
			if (d > r) {
				let v3: vec4_t = vec4_create();
				vec4_sub_vecs(v3, v2, v1);
				vec4_normalize(v3, );
				vec4_mult(v3, 1.0 - context_raw.brush_lazy_step);
				vec4_mult(v3, r);
				vec4_add_vecs(v2, v1, v3);
				InputNode.coords.x = v2.x / app_w();
				InputNode.coords.y = v2.y / app_h();
				// Parse brush inputs once on next draw
				context_raw.painted = -1;
			}
			context_raw.last_paint_x = -1;
			context_raw.last_paint_y = -1;
		}

		context_raw.parse_brush_inputs();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((value) => {
			context_raw.brush_lazy_radius = value;
			this.inputs[1].get((value) => {
				context_raw.brush_lazy_step = value;
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
