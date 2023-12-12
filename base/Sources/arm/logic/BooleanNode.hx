package arm.logic;

import arm.logic.LogicNode;

@:keep
class BooleanNode extends LogicNode {

	public var value: Bool;

	public function new(value = false) {
		super();
		this.value = value;
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (inputs.length > 0) inputs[0].get(done);
		else done(value);
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
