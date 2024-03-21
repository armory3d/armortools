
type time_node_t = {
	base?: logic_node_t;
};

function time_node_create(arg: any): time_node_t {
	let n: time_node_t = {};
	n.base = logic_node_create();
	n.base.get = time_node_get;
	return n;
}

function time_node_get(self: time_node_t, from: i32, done: (a: any)=>void) {
	if (from == 0) {
		done(time_time());
	}
	else if (from == 1) {
		done(time_delta());
	}
	else {
		done(context_raw.brush_time);
	}
}

let time_node_def: zui_node_t = {
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
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Delta"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Brush"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		}
	],
	buttons: []
};
