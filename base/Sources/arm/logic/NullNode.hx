package arm.logic;

import arm.logic.LogicNode;

@:keep
class NullNode extends LogicNode {

	public function new() {
		super();
	}

	override function get(from: Int, done: Dynamic->Void) {
		done(null);
	}
}
