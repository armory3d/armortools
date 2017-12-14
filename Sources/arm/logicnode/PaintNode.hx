package arm.logicnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;
import armory.math.Vec4;

class PaintNode extends LogicNode {

	public function new(tree:LogicTree) {
		super(tree);
	}

	override function run() {
		arm.UITrait.paint = true;

		var coords:Vec4 = inputs[1].get();
		arm.UITrait.paintX = coords.x;
		arm.UITrait.paintY = coords.y;

		super.run();
	}
}
