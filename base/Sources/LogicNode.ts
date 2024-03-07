
class LogicNode {
	inputs: LogicNodeInput[] = [];
	outputs: LogicNode[][] = [];

	constructor() {}

	add_input = (node: LogicNode, from: i32) => {
		this.inputs.push(new LogicNodeInput(node, from));
	}

	add_outputs = (nodes: LogicNode[]) => {
		this.outputs.push(nodes);
	}

	get = (from: i32, done: (a: any)=>void) => {
		done(null);
	}

	get_as_image = (from: i32, done: (img: image_t)=>void) => {
		done(null);
	}

	get_cached_image = (): image_t => {
		return null;
	}

	set = (value: any) => {}
}

class LogicNodeInput {
	node: LogicNode;
	from: i32; // Socket index

	constructor(node: LogicNode, from: i32) {
		this.node = node;
		this.from = from;
	}

	get = (done: (a: any)=>void) => {
		this.node.get(this.from, done);
	}

	get_as_image = (done: (img: image_t)=>void) => {
		this.node.get_as_image(this.from, done);
	}

	set = (value: any) => {
		this.node.set(value);
	}
}
