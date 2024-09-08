
type input_node_t = {
	base?: logic_node_t;
};

let input_node_coords: vec4_t = vec4_create();
let input_node_start_x: f32 = 0.0;
let input_node_start_y: f32 = 0.0;
// Brush ruler
let input_node_lock_begin: bool = false;
let input_node_lock_x: bool = false;
let input_node_lock_y: bool = false;
let input_node_lock_start_x: f32 = 0.0;
let input_node_lock_start_y: f32 = 0.0;
let input_node_registered: bool = false;

function input_node_create(arg: any): input_node_t {
	let n: float_node_t = {};
	n.base = logic_node_create();
	n.base.get = input_node_get;

	if (!input_node_registered) {
		input_node_registered = true;
		app_notify_on_update(input_node_update, n);
	}

	return n;
}

function input_node_update(self: float_node_t) {
	if (context_raw.split_view) {
		context_raw.view_index = mouse_view_x() > base_w() / 2 ? 1 : 0;
	}

	let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
	let decal_mask: bool = decal && operator_shortcut(map_get(config_keymap, "decal_mask") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);

	let lazy_paint: bool = context_raw.brush_lazy_radius > 0 &&
		(operator_shortcut(map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
			operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
			decal_mask);

	let paint_x: f32 = mouse_view_x() / app_w();
	let paint_y: f32 = mouse_view_y() / app_h();
	if (mouse_started()) {
		input_node_start_x = mouse_view_x() / app_w();
		input_node_start_y = mouse_view_y() / app_h();
	}

	if (pen_down()) {
		paint_x = pen_view_x() / app_w();
		paint_y = pen_view_y() / app_h();
	}
	if (pen_started()) {
		input_node_start_x = pen_view_x() / app_w();
		input_node_start_y = pen_view_y() / app_h();
	}

	if (operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN)) {
		if (input_node_lock_x) {
			paint_x = input_node_start_x;
		}
		if (input_node_lock_y) {
			paint_y = input_node_start_y;
		}
	}

	if (context_raw.brush_lazy_radius > 0) {
		context_raw.brush_lazy_x = paint_x;
		context_raw.brush_lazy_y = paint_y;
	}
	if (!lazy_paint) {
		input_node_coords.x = paint_x;
		input_node_coords.y = paint_y;
	}

	if (context_raw.split_view) {
		context_raw.view_index = -1;
	}

	if (input_node_lock_begin) {
		let dx: f32 = math_abs(input_node_lock_start_x - mouse_view_x());
		let dy: f32 = math_abs(input_node_lock_start_y - mouse_view_y());
		if (dx > 1 || dy > 1) {
			input_node_lock_begin = false;
			if (dx > dy) {
				input_node_lock_y = true;
			}
			else {
				input_node_lock_x = true;
			}
		}
	}

	if (keyboard_started(map_get(config_keymap, "brush_ruler"))) {
		input_node_lock_start_x = mouse_view_x();
		input_node_lock_start_y = mouse_view_y();
		input_node_lock_begin = true;
	}
	else if (keyboard_released(map_get(config_keymap, "brush_ruler"))) {
		input_node_lock_x = input_node_lock_y = input_node_lock_begin = false;
	}

	if (context_raw.brush_lazy_radius > 0) {
		let v1: vec4_t = vec4_create(context_raw.brush_lazy_x * app_w(), context_raw.brush_lazy_y * app_h(), 0.0);
		let v2: vec4_t = vec4_create(input_node_coords.x * app_w(), input_node_coords.y * app_h(), 0.0);
		let d: f32 = vec4_dist(v1, v2);
		let r: f32 = context_raw.brush_lazy_radius * 85;
		if (d > r) {
			let v3: vec4_t = vec4_create();
			v3 = vec4_sub(v2, v1);
			v3 = vec4_norm(v3);
			v3 = vec4_mult(v3, 1.0 - context_raw.brush_lazy_step);
			v3 = vec4_mult(v3, r);
			v2 = vec4_add(v1, v3);
			input_node_coords.x = v2.x / app_w();
			input_node_coords.y = v2.y / app_h();
			// Parse brush inputs once on next draw
			context_raw.painted = -1;
		}
		context_raw.last_paint_x = -1;
		context_raw.last_paint_y = -1;
	}

	context_raw.parse_brush_inputs(context_raw.brush_output_node_inst);
}

function input_node_get(self: input_node_t, from: i32): logic_node_value_t {
	context_raw.brush_lazy_radius = logic_node_input_get(self.base.inputs[0])._f32;
	context_raw.brush_lazy_step = logic_node_input_get(self.base.inputs[1])._f32;
	let v: logic_node_value_t = { _vec4: input_node_coords };
	return v;
}

let input_node_def: ui_node_t = {
	id: 0,
	name: _tr("Input"),
	type: "input_node",
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
			default_value: f32_array_create_x(0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Lazy Step"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Position"),
			type: "VECTOR",
			color: 0xff63c763,
			default_value: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0
};
