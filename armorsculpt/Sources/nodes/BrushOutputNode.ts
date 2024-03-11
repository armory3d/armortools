
class BrushOutputNode extends LogicNode {

	Directional = false; // button 0

	constructor() {
		super();
		context_raw.runBrush = this.run;
		context_raw.parseBrushInputs = this.parseInputs;
	}

	parseInputs = () => {
		let lastMask = context_raw.brushMaskImage;
		let lastStencil = context_raw.brushStencilImage;

		let input0: any;
		let input1: any;
		let input2: any;
		let input3: any;
		let input4: any;
		try {
			this.inputs[0].get((value) => { input0 = value; });
			this.inputs[1].get((value) => { input1 = value; });
			this.inputs[2].get((value) => { input2 = value; });
			this.inputs[3].get((value) => { input3 = value; });
			this.inputs[4].get((value) => { input4 = value; });
		}
		catch (_) {
			return;
		}

		context_raw.paintVec = input0;
		context_raw.brushNodesRadius = input1;

		let opac: any = input2; // Float or texture name
		if (opac == null) opac = 1.0;
		if (typeof opac == "string") {
			context_raw.brushMaskImageIsAlpha = opac.endsWith(".a");
			opac = opac.substr(0, opac.lastIndexOf("."));
			context_raw.brushNodesOpacity = 1.0;
			let index = project_assetNames.indexOf(opac);
			let asset = project_assets[index];
			context_raw.brushMaskImage = project_getImage(asset);
		}
		else {
			context_raw.brushNodesOpacity = opac;
			context_raw.brushMaskImage = null;
		}

		context_raw.brushNodesHardness = input3;

		let stencil: any = input4; // Float or texture name
		if (stencil == null) stencil = 1.0;
		if (typeof stencil == "string") {
			context_raw.brushStencilImageIsAlpha = stencil.endsWith(".a");
			stencil = stencil.substr(0, stencil.lastIndexOf("."));
			let index = project_assetNames.indexOf(stencil);
			let asset = project_assets[index];
			context_raw.brushStencilImage = project_getImage(asset);
		}
		else {
			context_raw.brushStencilImage = null;
		}

		if (lastMask != context_raw.brushMaskImage ||
			lastStencil != context_raw.brushStencilImage) {
			MakeMaterial.parsePaintMaterial();
		}

		context_raw.brushDirectional = this.Directional;
	}

	run = (from: i32) => {
		let left = 0.0;
		let right = 1.0;
		if (context_raw.paint2d) {
			left = 1.0;
			right = (context_raw.splitView ? 2.0 : 1.0) + ui_view2d_ww / base_w();
		}

		// First time init
		if (context_raw.lastPaintX < 0 || context_raw.lastPaintY < 0) {
			context_raw.lastPaintVecX = context_raw.paintVec.x;
			context_raw.lastPaintVecY = context_raw.paintVec.y;
		}

		// Do not paint over fill layer
		let fillLayer = context_raw.layer.fill_layer != null;

		// Do not paint over groups
		let groupLayer = SlotLayer.isGroup(context_raw.layer);

		// Paint bounds
		if (context_raw.paintVec.x > left &&
			context_raw.paintVec.x < right &&
			context_raw.paintVec.y > 0 &&
			context_raw.paintVec.y < 1 &&
			!fillLayer &&
			!groupLayer &&
			(SlotLayer.isVisible(context_raw.layer) || context_raw.paint2d) &&
			!ui_base_ui.is_hovered &&
			!base_isDragging &&
			!base_isResizing &&
			!base_isScrolling() &&
			!base_isComboSelected()) {

			let down = mouse_down() || pen_down();

			// Prevent painting the same spot
			let sameSpot = context_raw.paintVec.x == context_raw.lastPaintX && context_raw.paintVec.y == context_raw.lastPaintY;
			let lazy = context_raw.tool == WorkspaceTool.ToolBrush && context_raw.brushLazyRadius > 0;
			if (down && (sameSpot || lazy)) {
				context_raw.painted++;
			}
			else {
				context_raw.painted = 0;
			}
			context_raw.lastPaintX = context_raw.paintVec.x;
			context_raw.lastPaintY = context_raw.paintVec.y;

			if (context_raw.tool == WorkspaceTool.ToolParticle) {
				context_raw.painted = 0; // Always paint particles
			}

			if (context_raw.painted == 0) {
				this.parseInputs();
			}

			if (context_raw.painted == 0) {
				context_raw.pdirty = 1;
				context_raw.rdirty = 2;
				history_push_undo2 = true; ////
			}
		}
	}
}
