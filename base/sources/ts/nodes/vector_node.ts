
type vector_node_t = {
	base?: logic_node_t;
	value?: vec4_t;
	image?: image_t;
};

function vector_node_create(raw: ui_node_t, args: f32_array_t): vector_node_t {
	let n: vector_node_t = {};
	n.base = logic_node_create(n);
	n.base.get = vector_node_get;
	n.base.get_as_image = vector_node_get_as_image;
	n.base.set = vector_node_set;
	n.value = vec4_create();

	if (args != null) {
		logic_node_add_input(n.base, float_node_create(null, f32_array_create_x(args[0])), 0);
		logic_node_add_input(n.base, float_node_create(null, f32_array_create_x(args[1])), 0);
		logic_node_add_input(n.base, float_node_create(null, f32_array_create_x(args[2])), 0);
	}

	return n;
}

function vector_node_get(self: vector_node_t, from: i32): logic_node_value_t {
	let x: f32 = logic_node_input_get(self.base.inputs[0])._f32;
	let y: f32 = logic_node_input_get(self.base.inputs[1])._f32;
	let z: f32 = logic_node_input_get(self.base.inputs[2])._f32;
	self.value.x = x;
	self.value.y = y;
	self.value.z = z;
	let v: logic_node_value_t = { _vec4: self.value };
	return v;
}

function vector_node_get_as_image(self: vector_node_t, from: i32): image_t {
	// let x: f32 = logic_node_input_get(self.base.inputs[0]);
	// let y: f32 = logic_node_input_get(self.base.inputs[1]);
	// let z: f32 = logic_node_input_get(self.base.inputs[2]);
	if (self.image != null) {
		image_unload(self.image);
	}
	let b: buffer_t = buffer_create(16);
	let n0: float_node_t = self.base.inputs[0].node;
	let n1: float_node_t = self.base.inputs[1].node;
	let n2: float_node_t = self.base.inputs[2].node;
	buffer_set_f32(b, 0, n0.value);
	buffer_set_f32(b, 4, n1.value);
	buffer_set_f32(b, 8, n2.value);
	buffer_set_f32(b, 12, 1.0);
	self.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
	return self.image;
}

function vector_node_set(self: vector_node_t, value: f32_array_t) {
	logic_node_input_set(self.base.inputs[0], f32_array_create_x(value[0]));
	logic_node_input_set(self.base.inputs[1], f32_array_create_x(value[1]));
	logic_node_input_set(self.base.inputs[2], f32_array_create_x(value[2]));
}

let vector_node_def: ui_node_t = {
	id: 0,
	name: _tr("Vector"),
	type: "vector_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("X"),
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
			name: _tr("Y"),
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
			name: _tr("Z"),
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
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0
};
