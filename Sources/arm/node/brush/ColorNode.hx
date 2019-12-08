package arm.node.brush;

import iron.math.Vec4;

@:keep
class ColorNode extends LogicNode {

	var value = new Vec4();

	public function new(tree: LogicTree, r = 0.8, g = 0.8, b = 0.8, a = 1.0) {
		super(tree);

		value.set(r, g, b, a);
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
