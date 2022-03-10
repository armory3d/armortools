package arm.node.brush;

import arm.ui.UIToolbar;
import arm.ui.UISidebar;
import arm.ui.UIView2D;
import arm.Enums;

@:keep
class BrushOutputNode extends LogicNode {

	public var Directional = false; // button 0

	public function new(tree: LogicTree) {
		super(tree);
		Context.runBrush = run;
		Context.parseBrushInputs = parseInputs;
	}

	function parseInputs() {
		var lastMask = Context.brushMaskImage;
		var lastStencil = Context.brushStencilImage;

		var input0: Dynamic;
		var input1: Dynamic;
		var input2: Dynamic;
		var input3: Dynamic;
		var input4: Dynamic;
		var input5: Dynamic;
		var input6: Dynamic;
		try {
			input0 = inputs[0].get();
			input1 = inputs[1].get();
			input2 = inputs[2].get();
			input3 = inputs[3].get();
			input4 = inputs[4].get();
			input5 = inputs[5].get();
			input6 = inputs[6].get();
		}
		catch (_) {
			return;
		}

		Context.paintVec = input0;
		Context.brushNodesRadius = input1;
		Context.brushNodesScale = input2;
		Context.brushNodesAngle = input3;

		var opac: Dynamic = input4; // Float or texture name
		if (opac == null) opac = 1.0;
		if (Std.isOfType(opac, String)) {
			Context.brushMaskImageIsAlpha = opac.endsWith(".a");
			opac = opac.substr(0, opac.lastIndexOf("."));
			Context.brushNodesOpacity = 1.0;
			var index = Project.assetNames.indexOf(opac);
			var asset = Project.assets[index];
			Context.brushMaskImage = Project.getImage(asset);
		}
		else {
			Context.brushNodesOpacity = opac;
			Context.brushMaskImage = null;
		}

		Context.brushNodesHardness = input5;

		var stencil: Dynamic = input6; // Float or texture name
		if (stencil == null) stencil = 1.0;
		if (Std.isOfType(stencil, String)) {
			Context.brushStencilImageIsAlpha = stencil.endsWith(".a");
			stencil = stencil.substr(0, stencil.lastIndexOf("."));
			var index = Project.assetNames.indexOf(stencil);
			var asset = Project.assets[index];
			Context.brushStencilImage = Project.getImage(asset);
		}
		else {
			Context.brushStencilImage = null;
		}

		if (lastMask != Context.brushMaskImage ||
			lastStencil != Context.brushStencilImage) {
			MakeMaterial.parsePaintMaterial();
		}

		Context.brushDirectional = Directional;
	}

	override function run(from: Int) {
		var left = 0.0;
		var right = 1.0;
		if (Context.paint2d) {
			left = 1.0;
			right = (Context.splitView ? 2.0 : 1.0) + UIView2D.inst.ww / App.w();
		}

		// First time init
		if (Context.lastPaintX < 0 || Context.lastPaintY < 0) {
			Context.lastPaintVecX = Context.paintVec.x;
			Context.lastPaintVecY = Context.paintVec.y;
		}

		// Do not paint over fill layer
		var fillLayer = Context.layer.fill_layer != null && Context.tool != ToolPicker && Context.tool != ToolColorId;

		// Do not paint over groups
		var groupLayer = Context.layer.isGroup();

		// Paint bounds
		if (Context.paintVec.x > left &&
			Context.paintVec.x < right &&
			Context.paintVec.y > 0 &&
			Context.paintVec.y < 1 &&
			!fillLayer &&
			!groupLayer &&
			(Context.layer.isVisible() || Context.paint2d) &&
			!UISidebar.inst.ui.isHovered &&
			!arm.App.isDragging &&
			!arm.App.isResizing &&
			!arm.App.isScrolling() &&
			!arm.App.isComboSelected()) {

			// Set color pick
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			if (down && Context.tool == ToolColorId && Project.assets.length > 0) {
				Context.colorIdPicked = true;
				UIToolbar.inst.toolbarHandle.redraws = 1;
			}

			// Prevent painting the same spot
			var sameSpot = Context.paintVec.x == Context.lastPaintX && Context.paintVec.y == Context.lastPaintY;
			var lazy = Context.tool == ToolBrush && Context.brushLazyRadius > 0;
			if (down && (sameSpot || lazy)) {
				Context.painted++;
			}
			else {
				Context.painted = 0;
			}
			Context.lastPaintX = Context.paintVec.x;
			Context.lastPaintY = Context.paintVec.y;

			if (Context.tool == ToolParticle) {
				Context.painted = 0; // Always paint particles
			}

			if (Context.painted == 0) {
				parseInputs();
			}

			if (Context.painted <= 1) {
				Context.pdirty = 1;
				Context.rdirty = 2;
			}
		}
	}
}
