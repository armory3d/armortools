
type boolean_node_t = {
	base?: logic_node_t;
	value?: bool;
};

function boolean_node_create(args: f32_array_t): boolean_node_t {
	let n: boolean_node_t = {};
	n.base = logic_node_create();
	n.base.get = boolean_node_get;
	n.base.set = boolean_node_set;
	n.value = args[0] > 0.0;
	return n;
}

function boolean_node_get(self: boolean_node_t, from: i32): logic_node_value_t {
	if (self.base.inputs.length > 0) {
		return logic_node_input_get(self.base.inputs[0]);
	}
	else {
		let v: logic_node_value_t = {
			_f32: self.value ? 1.0 : 0.0
		};
		return v;
	}
}

function boolean_node_set(self: boolean_node_t, value: f32_array_t) {
	if (self.base.inputs.length > 0) {
		logic_node_input_set(self.base.inputs[0], value);
	}
	else {
		self.value = value[0] > 0.0;
	}
}
