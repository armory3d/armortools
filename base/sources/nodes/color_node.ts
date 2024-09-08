
type color_node_t = {
	base?: logic_node_t;
	value?: vec4_t;
	image?: image_t;
};

function color_node_create(args: f32_array_t): color_node_t {
	let r: f32 = args == null ? 0.8 : args[0];
	let g: f32 = args == null ? 0.8 : args[1];
	let b: f32 = args == null ? 0.8 : args[2];
	let a: f32 = args == null ? 1.0 : args[3];
	let n: color_node_t = {};
	n.base = logic_node_create();
	n.base.get = color_node_get;
	n.base.get_as_image = color_node_get_as_image;
	n.base.set = color_node_set;
	n.value = vec4_create(r, g, b, a);
	return n;
}

function color_node_get(self: color_node_t, from: i32): logic_node_value_t {
	if (self.base.inputs.length > 0) {
		return logic_node_input_get(self.base.inputs[0]);
	}
	else {
		let v: logic_node_value_t = { _vec4: self.value };
		return v;
	}
}

function color_node_get_as_image(self: color_node_t, from: i32): image_t {
	if (self.base.inputs.length > 0) {
		return logic_node_input_get_as_image(self.base.inputs[0]);
	}
	if (self.image != null) {
		image_unload(self.image);
	}
	let b: buffer_t = buffer_create(16);
	buffer_set_f32(b, 0, self.value.x);
	buffer_set_f32(b, 4, self.value.y);
	buffer_set_f32(b, 8, self.value.z);
	buffer_set_f32(b, 12, self.value.w);
	self.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
	return self.image;
}

function color_node_set(self: color_node_t, value: f32_array_t) {
	if (self.base.inputs.length > 0) {
		logic_node_input_set(self.base.inputs[0], value);
	}
	else {
		self.value.x = value[0];
		self.value.y = value[1];
		self.value.z = value[2];
		self.value.w = value[3];
	}
}
