package arm.brushnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;

@:keep
class BrushOutputNode extends LogicNode {

	public function new(tree:LogicTree) {
		super(tree);
		arm.UITrait.inst.notifyOnBrush(run);
	}

	override function run(from:Int) {
		arm.UITrait.inst.paintVec = inputs[0].get();
		arm.UITrait.inst.brushNodesRadius = inputs[1].get();
		arm.UITrait.inst.brushNodesOpacity = inputs[2].get();
		arm.UITrait.inst.brushNodesStrength = inputs[3].get();
		arm.UITrait.inst.brushNodesScale = inputs[4].get();

		// Paint bounds
		if (arm.UITrait.inst.paintVec.x < 1 && arm.UITrait.inst.paintVec.x > 0 &&
			arm.UITrait.inst.paintVec.y < 1 && arm.UITrait.inst.paintVec.y > 0 &&
			!arm.UITrait.inst.ui.isHovered) { // Header combos are hovered
			// Set color pick
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			if (arm.UITrait.inst.brushType == 4 && arm.UITrait.inst.assets.length > 0 && down) {
				arm.UITrait.inst.colorIdPicked = true;
			}
			// Prevent painting the same spot - save perf & reduce projection paint jittering caused by _sub offset
			if (down && arm.UITrait.inst.paintVec.x == arm.UITrait.inst.lastPaintX && arm.UITrait.inst.paintVec.y == arm.UITrait.inst.lastPaintY) {
				arm.UITrait.inst.painted++;
			}
			else {
				arm.UITrait.inst.painted = 0;
			}
			arm.UITrait.inst.lastPaintX = arm.UITrait.inst.paintVec.x;
			arm.UITrait.inst.lastPaintY = arm.UITrait.inst.paintVec.y;
			if (arm.UITrait.inst.painted <= 8) {
				arm.UITrait.inst.pdirty = 1;
				arm.UITrait.inst.rdirty = 2;
			}
		}

		runOutput(0);
	}
}
