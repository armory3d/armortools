
type separate_vector_node_t = {
	base?: logic_node_t;
};

function separate_vector_node_create(arg: any): separate_vector_node_t {
	let n: separate_vector_node_t = {};
	n.base = logic_node_create();
	n.base.get = separate_vector_node_get;
	return n;
}

function separate_vector_node_get(self: separate_vector_node_t, from: i32): any {
	let vector: vec4_t = logic_node_input_get(self.base.inputs[0]);
	if (from == 0) {
		return vector.x;
	}
	else if (from == 1) {
		return vector.y;
	}
	else {
		return vector.z;
	}
}

let separate_vector_node_def: zui_node_t = {
	id: 0,
	name: _tr("Separate Vector"),
	type: "separate_vector_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: f32_array_create_xyz(0.0, 0.0, 0.0)
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("X"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Y"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Z"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		}
	],
	buttons: []
};
