package arm.brushnode;

import armory.logicnode.LogicNode;
import armory.logicnode.LogicTree;
import arm.ui.UITrait;
import arm.ui.*;

@:keep
class BrushOutputNode extends LogicNode {

	public function new(tree:LogicTree) {
		super(tree);
		UITrait.inst.notifyOnBrush(run);
	}

	override function run(from:Int) {
		UITrait.inst.paintVec = inputs[0].get();
		UITrait.inst.brushNodesRadius = inputs[1].get();
		UITrait.inst.brushNodesOpacity = inputs[2].get();
		UITrait.inst.brushNodesHardness = inputs[3].get();
		UITrait.inst.brushNodesScale = inputs[4].get();

		var left = 0;
		var right = 1;
		if (UITrait.inst.paint2d) {
			left = 1;
			right = 2;
		}

		// First time init
		if (UITrait.inst.lastPaintX < 0 || UITrait.inst.lastPaintY < 0) {
			UITrait.inst.lastPaintVecX = UITrait.inst.paintVec.x;
			UITrait.inst.lastPaintVecY = UITrait.inst.paintVec.y;
		}

		// Paint bounds
		if (UITrait.inst.paintVec.x < right && UITrait.inst.paintVec.x > left &&
			UITrait.inst.paintVec.y < 1 && UITrait.inst.paintVec.y > 0 &&
			!UITrait.inst.ui.isHovered && !UITrait.inst.ui.isScrolling) { // Header combos are in use
			// Set color pick
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			if (UITrait.inst.selectedTool == ToolColorId && UITrait.inst.assets.length > 0 && down) {
				UITrait.inst.colorIdPicked = true;
			}
			// Prevent painting the same spot - save perf & reduce projection paint jittering caused by _sub offset
			if (down && UITrait.inst.paintVec.x == UITrait.inst.lastPaintX && UITrait.inst.paintVec.y == UITrait.inst.lastPaintY) {
				UITrait.inst.painted++;
			}
			else {
				UITrait.inst.painted = 0;
			}
			UITrait.inst.lastPaintX = UITrait.inst.paintVec.x;
			UITrait.inst.lastPaintY = UITrait.inst.paintVec.y;

			if (UITrait.inst.selectedTool == ToolParticle) {
				UITrait.inst.painted = 0; // Always paint particles
			}

			var decal = UITrait.inst.selectedTool == ToolDecal || UITrait.inst.selectedTool == ToolText;
			var paintFrames = decal ? 1 : 8;

			if (UITrait.inst.painted <= paintFrames) {
				UITrait.inst.pdirty = 1;
				UITrait.inst.rdirty = 2;
			}
		}

		runOutput(0);
	}
}
