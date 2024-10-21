
type float_node_t = {
	base?: logic_node_t;
	value?: f32;
	image?: image_t;
};

function float_node_create(raw: ui_node_t, args: f32_array_t): float_node_t {
	let n: float_node_t = {};
	n.base = logic_node_create(n);
	n.base.get = float_node_get;
	n.base.get_as_image = float_node_get_as_image;
	n.base.set = float_node_set;
	n.value = args == null ? 0.5 : args[0];
	return n;
}

function float_node_get(self: float_node_t, from: i32): logic_node_value_t {
	if (self.base.inputs.length > 0) {
		return logic_node_input_get(self.base.inputs[0]);
	}
	else {
		let v: logic_node_value_t = { _f32: self.value };
		return v;
	}
}

function float_node_get_as_image(self: float_node_t, from: i32): image_t {
	if (self.base.inputs.length > 0) {
		return logic_node_input_get_as_image(self.base.inputs[0]);
	}
	if (self.image != null) {
		image_unload(self.image);
	}
	let b: buffer_t = buffer_create(16);
	buffer_set_f32(b, 0, self.value);
	buffer_set_f32(b, 4, self.value);
	buffer_set_f32(b, 8, self.value);
	buffer_set_f32(b, 12, 1.0);
	self.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
	return self.image;
}

function float_node_set(self: float_node_t, value: f32_array_t) {
	if (self.base.inputs.length > 0) {
		logic_node_input_set(self.base.inputs[0], value);
	}
	else {
		self.value = value[0];
	}
}

let float_node_def: ui_node_t = {
	id: 0,
	name: _tr("Value"),
	type: "float_node",
	x: 0,
	y: 0,
	color: 0xffb34f5a,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Value"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.5),
			min: 0.0,
			max: 10.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Value"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.5),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0
};
