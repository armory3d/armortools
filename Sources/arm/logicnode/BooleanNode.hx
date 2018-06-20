package arm.logicnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;

@:keep
class BooleanNode extends LogicNode {

	public var value:Bool;

	public function new(tree:LogicTree, value = false) {
		super(tree);
		this.value = value;
	}

	override function get(from:Int):Dynamic {
		if (inputs.length > 0) return inputs[0].get();
		return value;
	}

	override function set(value:Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
