package arm.node.brush;

@:keep
class StringNode extends LogicNode {

	public var value: String;

	public function new(tree: LogicTree, value = "") {
		super(tree);
		this.value = value;
	}

	override function get(from: Int): Dynamic {
		if (inputs.length > 0) return inputs[0].get();
		return value;
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
