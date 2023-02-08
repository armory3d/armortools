package arm.node.brush;

@:keep
class RandomNode extends LogicNode {

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int): Dynamic {
		var min = inputs[0].get();
		var max = inputs[1].get();
		return min + (Math.random() * (max - min));
	}
}
