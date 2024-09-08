
type logic_node_t = {
	inputs?: logic_node_input_t[];
	outputs?: logic_node_t[][];
	get?: (self: any, from: i32)=>logic_node_value_t;
	get_as_image?: (self: any, from: i32)=>image_t;
	get_cached_image?: (self: any)=>image_t;
	set?: (self: any, value: f32_array_t)=>void;
};

type logic_node_ext_t = {
	base?: logic_node_t;
}

type logic_node_value_t = {
	_f32?: f32;
	_vec4?: vec4_t;
	_str?: string;
};

type logic_node_input_t = {
	node?: logic_node_ext_t;
	from?: i32; // Socket index
};

function logic_node_create(): logic_node_t {
	let n: logic_node_t = {};
	n.inputs = [];
	n.outputs = [];
	n.get = logic_node_get;
	n.get_as_image = logic_node_get_as_image;
	n.get_cached_image = logic_node_get_cached_image;
	n.set = logic_node_set;
	return n;
}

function logic_node_add_input(self: logic_node_t, node: logic_node_ext_t, from: i32) {
	array_push(self.inputs, logic_node_input_create(node, from));
}

function logic_node_add_outputs(self: logic_node_t, nodes: logic_node_t[]) {
	array_push(self.outputs, nodes);
}

function logic_node_get(self: logic_node_t, from: i32): any {
	return null;
}

function logic_node_get_as_image(self: logic_node_t, from: i32): image_t {
	return null;
}

function logic_node_get_cached_image(self: logic_node_t): image_t {
	return null;
}

function logic_node_set(self: logic_node_t, value: any) {}

function logic_node_input_create(node: logic_node_ext_t, from: i32): logic_node_input_t {
	let inp: logic_node_input_t = {};
	inp.node = node;
	inp.from = from;
	return inp;
}

function logic_node_input_get(self: logic_node_input_t): logic_node_value_t {
	return self.node.base.get(self.node, self.from);
}

function logic_node_input_get_as_image(self: logic_node_input_t): image_t {
	return self.node.base.get_as_image(self.node, self.from);
}

function logic_node_input_set(self: logic_node_input_t, value: f32_array_t) {
	self.node.base.set(self.node, value);
}
