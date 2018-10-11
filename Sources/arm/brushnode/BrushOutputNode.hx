package arm.brushnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;

@:keep
class BrushOutputNode extends LogicNode {

	public function new(tree:LogicTree) {
		super(tree);
		arm.UITrait.inst.notifyOnBrush(run);
	}

	override function run() {
		arm.UITrait.inst.paint = true;
		arm.UITrait.inst.paintVec = inputs[0].get();
		arm.UITrait.inst.brushNodesRadius = inputs[1].get();
		arm.UITrait.inst.brushNodesOpacity = inputs[2].get();
		arm.UITrait.inst.brushNodesStrength = inputs[3].get();
		arm.UITrait.inst.brushNodesScale = inputs[4].get();

		super.run();
	}
}
