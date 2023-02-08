package arm.node.brush;

import iron.math.Vec4;

@:keep
class VectorNode extends LogicNode {

	var value = new Vec4();

	public function new(tree: LogicTree, x: Null<Float> = null, y: Null<Float> = null, z: Null<Float> = null) {
		super(tree);

		if (x != null) {
			addInput(new FloatNode(tree, x), 0);
			addInput(new FloatNode(tree, y), 0);
			addInput(new FloatNode(tree, z), 0);
		}
	}

	override function get(from: Int): Dynamic {
		value.x = inputs[0].get();
		value.y = inputs[1].get();
		value.z = inputs[2].get();
		return value;
	}

	override function set(value: Dynamic) {
		inputs[0].set(value.x);
		inputs[1].set(value.y);
		inputs[2].set(value.z);
	}
}
