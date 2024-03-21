
type boolean_node_t = {
	base?: logic_node_t;
	value?: bool;
};

function boolean_node_create(arg: bool): boolean_node_t {
	let n: boolean_node_t = {};
	n.base = logic_node_create();
	n.base.get = boolean_node_get;
	n.base.set = boolean_node_set;
	n.value = arg;
	return n;
}

function boolean_node_get(self: boolean_node_t, from: i32, done: (a: any)=>void) {
	if (self.base.inputs.length > 0) {
		logic_node_input_get(self.base.inputs[0], done);
	}
	else {
		done(self.value);
	}
}

function boolean_node_set(self: boolean_node_t, value: any) {
	if (self.base.inputs.length > 0) {
		logic_node_input_set(self.base.inputs[0], value);
	}
	else {
		self.value = value;
	}
}
