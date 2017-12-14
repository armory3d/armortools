package arm.logicnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;
import armory.math.Vec4;

class OnBrushNode extends LogicNode {

	var coords = new Vec4();

	public function new(tree:LogicTree) {
		super(tree);
		arm.UITrait.notifyOnBrush(onBrush);
	}

	override function get(from:Int):Dynamic {
		var mouse = armory.system.Input.getMouse();
		coords.x = mouse.x;
		coords.y = mouse.y;
		return coords;
	}

	function onBrush() {
		run();
	}
}
