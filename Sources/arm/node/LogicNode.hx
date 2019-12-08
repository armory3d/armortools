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

	/**
	  Called when this node is activated.
	  @param from impulse index
	**/
	function run(from: Int) {}

	/**
	  Call to activate node connected to the output.
	  @param i output index
	**/
	function runOutput(i: Int) {
		if (i >= outputs.length) return;
		for (o in outputs[i]) {
			// Check which input activated the node
			for (j in 0...o.inputs.length) {
				if (o.inputs[j].node == this) {
					o.run(j);
					break;
				}
			}
		}
	}

	@:allow(arm.node.LogicNodeInput)
	function get(from: Int): Dynamic { return this; }

	@:allow(arm.node.LogicNodeInput)
	function set(value: Dynamic) {}
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
	function get(): Dynamic {
		return node.get(from);
	}

	@:allow(arm.node.LogicNode)
	function set(value: Dynamic) {
		node.set(value);
	}
}
