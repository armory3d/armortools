package arm.logic;

import iron.math.Vec4;
import arm.logic.LogicNode;

@:keep
class SeparateVectorNode extends LogicNode {

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(vector: Vec4) {
			if (from == 0) done(vector.x);
			else if (from == 1) done(vector.y);
			else done(vector.z);
		});
	}
}
