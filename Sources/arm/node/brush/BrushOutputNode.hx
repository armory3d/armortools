package arm.node.brush;

import arm.ui.UISidebar;
import arm.ui.UIView2D;
import arm.Enums;

@:keep
class BrushOutputNode extends LogicNode {

	public var Directional = false; // button 0

	public function new(tree: LogicTree) {
		super(tree);
		UISidebar.inst.runBrush = run;
		UISidebar.inst.parseBrushInputs = parseInputs;
	}

	function parseInputs() {
		var lastMask = UISidebar.inst.brushMaskImage;
		var lastStencil = UISidebar.inst.brushStencilImage;

		UISidebar.inst.paintVec = inputs[0].get();
		UISidebar.inst.brushNodesRadius = inputs[1].get();
		UISidebar.inst.brushNodesScale = inputs[2].get();
		UISidebar.inst.brushNodesAngle = inputs[3].get();

		var opac: Dynamic = inputs[4].get(); // Float or texture name
		if (opac == null) opac = 1.0;
		if (Std.is(opac, String)) {
			UISidebar.inst.brushNodesOpacity = 1.0;
			var index = Project.assetNames.indexOf(opac);
			var asset = Project.assets[index];
			UISidebar.inst.brushMaskImage = UISidebar.inst.getImage(asset);
		}
		else {
			UISidebar.inst.brushNodesOpacity = opac;
			UISidebar.inst.brushMaskImage = null;
		}

		UISidebar.inst.brushNodesHardness = inputs[5].get();

		var stencil: Dynamic = inputs[6].get(); // Float or texture name
		if (stencil == null) stencil = 1.0;
		if (Std.is(stencil, String)) {
			var index = Project.assetNames.indexOf(stencil);
			var asset = Project.assets[index];
			UISidebar.inst.brushStencilImage = UISidebar.inst.getImage(asset);
		}
		else {
			UISidebar.inst.brushStencilImage = null;
		}

		if (lastMask != UISidebar.inst.brushMaskImage ||
			lastStencil != UISidebar.inst.brushStencilImage) {
			MaterialParser.parsePaintMaterial();
		}

		UISidebar.inst.brushDirectional = Directional;
	}

	override function run(from: Int) {

		parseInputs();

		var left = 0;
		var right = 1;
		if (UISidebar.inst.paint2d) {
			left = 1;
			right = 2;
		}

		// First time init
		if (UISidebar.inst.lastPaintX < 0 || UISidebar.inst.lastPaintY < 0) {
			UISidebar.inst.lastPaintVecX = UISidebar.inst.paintVec.x;
			UISidebar.inst.lastPaintVecY = UISidebar.inst.paintVec.y;
		}

		// Do not paint over fill layer
		var fillLayer = Context.layer.material_mask != null && Context.tool != ToolPicker && !Context.layerIsMask;

		// Do not paint over groups
		var groupLayer = Context.layer.getChildren() != null;

		// Paint bounds
		if (UISidebar.inst.paintVec.x < right && UISidebar.inst.paintVec.x > left &&
			UISidebar.inst.paintVec.y < 1 && UISidebar.inst.paintVec.y > 0 &&
			!UISidebar.inst.ui.isHovered &&
			!UISidebar.inst.ui.isScrolling &&
			!fillLayer &&
			!groupLayer &&
			(Context.layer.isVisible() || UISidebar.inst.paint2d) &&
			!arm.App.isDragging &&
			!arm.App.isResizing &&
			@:privateAccess UISidebar.inst.ui.comboSelectedHandle == null &&
			@:privateAccess UIView2D.inst.ui.comboSelectedHandle == null) { // Header combos are in use

			// Set color pick
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			if (down && Context.tool == ToolColorId && Project.assets.length > 0) {
				UISidebar.inst.colorIdPicked = true;
			}
			// Prevent painting the same spot
			if (down && UISidebar.inst.paintVec.x == UISidebar.inst.lastPaintX && UISidebar.inst.paintVec.y == UISidebar.inst.lastPaintY) {
				UISidebar.inst.painted++;
			}
			else {
				UISidebar.inst.painted = 0;
			}
			UISidebar.inst.lastPaintX = UISidebar.inst.paintVec.x;
			UISidebar.inst.lastPaintY = UISidebar.inst.paintVec.y;

			if (Context.tool == ToolParticle) {
				UISidebar.inst.painted = 0; // Always paint particles
			}

			var decal = Context.tool == ToolDecal || Context.tool == ToolText;
			var paintFrames = decal ? 1 : 4;

			if (UISidebar.inst.painted <= paintFrames) {
				Context.pdirty = 1;
				Context.rdirty = 2;
			}
		}
	}
}
