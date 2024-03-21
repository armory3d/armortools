
type string_node_t = {
	base?: logic_node_t;
	value?: string;
};

function string_node_create(arg: string): string_node_t {
	let n: string_node_t = {};
	n.base = logic_node_create();
	n.base.get = string_node_get;
	n.base.set = string_node_set;
	n.value = arg;
	return n;
}

function string_node_get(self: string_node_t, from: i32, done: (a: any)=>void) {
	if (self.base.inputs.length > 0) {
		logic_node_input_get(self.base.inputs[0], done);
	}
	else {
		done(self.value);
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
