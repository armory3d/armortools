
class LogicNode {
	inputs: LogicNodeInput[] = [];
	outputs: LogicNode[][] = [];

	constructor() {}

	addInput = (node: LogicNode, from: i32) => {
		this.inputs.push(new LogicNodeInput(node, from));
	}

	addOutputs = (nodes: LogicNode[]) => {
		this.outputs.push(nodes);
	}

	get = (from: i32, done: (a: any)=>void) => {
		done(null);
	}

	getAsImage = (from: i32, done: (img: image_t)=>void) => {
		done(null);
	}

	getCachedImage = (): image_t => {
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

	getAsImage = (done: (img: image_t)=>void) => {
		this.node.getAsImage(this.from, done);
	}

	set = (value: any) => {
		this.node.set(value);
	}
}
