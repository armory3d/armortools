package arm.logicnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;

@:keep
class NullNode extends LogicNode {

	public function new(tree:LogicTree) {
		super(tree);
	}

	override function get(from:Int):Dynamic { return null; }
}
