package arm.logicnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;
import armory.math.Vec4;

class BrushOutputNode extends LogicNode {

	public function new(tree:LogicTree) {
		super(tree);
		arm.UITrait.notifyOnBrush(run);
	}

	override function run() {
		arm.UITrait.paint = true;
		arm.UITrait.paintVec = inputs[0].get();
		arm.UITrait.brushNodesRadius = inputs[1].get();
		arm.UITrait.brushNodesOpacity = inputs[2].get();
		arm.UITrait.brushNodesStrength = inputs[3].get();
		arm.UITrait.brushNodesScale = inputs[4].get();

		super.run();
	}
}
