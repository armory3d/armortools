package arm.node;

class LogicNode {

	var tree: LogicTree;
	var inputs: Array<LogicNodeInput> = [];
	var outputs: Array<Array<LogicNode>> = [];

	public function new(tree: LogicTree) {
		this.tree = tree;
	}

	public function addInput(node: LogicNode, from: Int) {
		inputs.push(new LogicNodeInput(node, from));
	}

	public function addOutputs(nodes: Array<LogicNode>) {
		outputs.push(nodes);
	}

	@:allow(arm.node.LogicNodeInput)
	function get(from: Int, done: Dynamic->Void) {
		done(this);
	}

	@:allow(arm.node.LogicNodeInput)
	function set(value: Dynamic) {}

	public function getImage(): kha.Image {
		return null;
	}
}

class LogicNodeInput {

	@:allow(arm.node.LogicNode)
	var node: LogicNode;
	var from: Int; // Socket index

	public function new(node: LogicNode, from: Int) {
		this.node = node;
		this.from = from;
	}

	@:allow(arm.node.LogicNode)
	function get(done: Dynamic->Void) {
		node.get(from, done);
	}

	@:allow(arm.node.LogicNode)
	function set(value: Dynamic) {
		node.set(value);
	}
}
