
type time_node_t = {
	base?: logic_node_t;
};

function time_node_create(args: f32_array_t): time_node_t {
	let n: time_node_t = {};
	n.base = logic_node_create();
	n.base.get = time_node_get;
	return n;
}

function time_node_get(self: time_node_t, from: i32): logic_node_value_t {
	if (from == 0) {
		let v: logic_node_value_t = { _f32: time_time() };
		return v;
	}
	else if (from == 1) {
		let v: logic_node_value_t = { _f32: time_delta() };
		return v;
	}
	else {
		let v: logic_node_value_t = { _f32: context_raw.brush_time };
		return v;
	}
}

let time_node_def: ui_node_t = {
	id: 0,
	name: _tr("Time"),
	type: "time_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Time"),
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
			name: _tr("Delta"),
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
			name: _tr("Brush"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0
};
