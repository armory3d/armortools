
type string_node_t = {
	base?: logic_node_t;
	value?: string;
};

function string_node_create(raw: ui_node_t, args: f32_array_t): string_node_t {
	let n: string_node_t = {};
	n.base = logic_node_create(n);
	n.base.get = string_node_get;
	n.base.set = string_node_set;
	n.value = args == null ? "" : sys_buffer_to_string(args.buffer);
	return n;
}

function string_node_get(self: string_node_t, from: i32): logic_node_value_t {
	if (self.base.inputs.length > 0) {
		return logic_node_input_get(self.base.inputs[0]);
	}
	else {
		let v: logic_node_value_t = { _str: self.value };
		return v;
	}
}

function string_node_set(self: string_node_t, value: any) {
	if (self.base.inputs.length > 0) {
		logic_node_input_set(self.base.inputs[0], value);
	}
	else {
		self.value = value;
	}
}
