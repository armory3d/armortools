
type logic_node_t = {
	inputs?: logic_node_input_t[];
	outputs?: logic_node_t[][];
	get?: (self: any, from: i32, done: (a: any)=>void)=>void;
	get_as_image?: (self: any, from: i32, done: (img: image_t)=>void)=>void;
	get_cached_image?: (self: any)=>image_t ;
	set?: (self: any, value: any)=>void;
};

type logic_node_input_t = {
	node?: any; // logic_node_t
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

function logic_node_add_input(self: logic_node_t, node: logic_node_t, from: i32) {
	self.inputs.push(logic_node_input_create(node, from));
}

function logic_node_add_outputs(self: logic_node_t, nodes: logic_node_t[]) {
	self.outputs.push(nodes);
}

function logic_node_get(self: logic_node_t, from: i32, done: (a: any)=>void) {
	done(null);
}

function logic_node_get_as_image(self: logic_node_t, from: i32, done: (img: image_t)=>void) {
	done(null);
}

function logic_node_get_cached_image(self: logic_node_t): image_t {
	return null;
}

function logic_node_set(self: logic_node_t, value: any) {}

function logic_node_input_create(node: logic_node_t, from: i32): logic_node_input_t {
	let inp: logic_node_input_t = {};
	inp.node = node;
	inp.from = from;
	return inp;
}

function logic_node_input_get(self: logic_node_input_t, done: (a: any)=>void) {
	self.node.base.get(self.node, self.from, done);
}

function logic_node_input_get_as_image(self: logic_node_input_t, done: (img: image_t)=>void) {
	self.node.base.get_as_image(self.node, self.from, done);
}

function logic_node_input_set(self: logic_node_input_t, value: any) {
	self.node.base.set(self.node, value);
}
