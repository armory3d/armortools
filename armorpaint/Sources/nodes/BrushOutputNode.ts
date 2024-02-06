
class BrushOutputNode extends LogicNode {

	Directional = false; // button 0

	constructor() {
		super();
		Context.raw.runBrush = this.run;
		Context.raw.parseBrushInputs = this.parseInputs;
	}

	parseInputs = () => {
		let lastMask = Context.raw.brushMaskImage;
		let lastStencil = Context.raw.brushStencilImage;

		let input0: any;
		let input1: any;
		let input2: any;
		let input3: any;
		let input4: any;
		let input5: any;
		let input6: any;
		try {
			this.inputs[0].get((value) => { input0 = value; });
			this.inputs[1].get((value) => { input1 = value; });
			this.inputs[2].get((value) => { input2 = value; });
			this.inputs[3].get((value) => { input3 = value; });
			this.inputs[4].get((value) => { input4 = value; });
			this.inputs[5].get((value) => { input5 = value; });
			this.inputs[6].get((value) => { input6 = value; });
		}
		catch (_) {
			return;
		}

		Context.raw.paintVec = input0;
		Context.raw.brushNodesRadius = input1;
		Context.raw.brushNodesScale = input2;
		Context.raw.brushNodesAngle = input3;

		let opac: any = input4; // Float or texture name
		if (opac == null) opac = 1.0;
		if (typeof opac == "string") {
			Context.raw.brushMaskImageIsAlpha = opac.endsWith(".a");
			opac = opac.substr(0, opac.lastIndexOf("."));
			Context.raw.brushNodesOpacity = 1.0;
			let index = Project.assetNames.indexOf(opac);
			let asset = Project.assets[index];
			Context.raw.brushMaskImage = Project.getImage(asset);
		}
		else {
			Context.raw.brushNodesOpacity = opac;
			Context.raw.brushMaskImage = null;
		}

		Context.raw.brushNodesHardness = input5;

		let stencil: any = input6; // Float or texture name
		if (stencil == null) stencil = 1.0;
		if (typeof stencil == "string") {
			Context.raw.brushStencilImageIsAlpha = stencil.endsWith(".a");
			stencil = stencil.substr(0, stencil.lastIndexOf("."));
			let index = Project.assetNames.indexOf(stencil);
			let asset = Project.assets[index];
			Context.raw.brushStencilImage = Project.getImage(asset);
		}
		else {
			Context.raw.brushStencilImage = null;
		}

		if (lastMask != Context.raw.brushMaskImage ||
			lastStencil != Context.raw.brushStencilImage) {
			MakeMaterial.parsePaintMaterial();
		}

		Context.raw.brushDirectional = this.Directional;
	}

	run = (from: i32) => {
		let left = 0.0;
		let right = 1.0;
		if (Context.raw.paint2d) {
			left = 1.0;
			right = (Context.raw.splitView ? 2.0 : 1.0) + UIView2D.ww / Base.w();
		}

		// First time init
		if (Context.raw.lastPaintX < 0 || Context.raw.lastPaintY < 0) {
			Context.raw.lastPaintVecX = Context.raw.paintVec.x;
			Context.raw.lastPaintVecY = Context.raw.paintVec.y;
		}

		// Do not paint over fill layer
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != WorkspaceTool.ToolPicker && Context.raw.tool != WorkspaceTool.ToolMaterial && Context.raw.tool != WorkspaceTool.ToolColorId;

		// Do not paint over groups
		let groupLayer = SlotLayer.isGroup(Context.raw.layer);

		// Paint bounds
		if (Context.raw.paintVec.x > left &&
			Context.raw.paintVec.x < right &&
			Context.raw.paintVec.y > 0 &&
			Context.raw.paintVec.y < 1 &&
			!fillLayer &&
			!groupLayer &&
			(SlotLayer.isVisible(Context.raw.layer) || Context.raw.paint2d) &&
			!UIBase.ui.is_hovered &&
			!Base.isDragging &&
			!Base.isResizing &&
			!Base.isScrolling() &&
			!Base.isComboSelected()) {

			// Set color pick
			let down = mouse_down() || pen_down();
			if (down && Context.raw.tool == WorkspaceTool.ToolColorId && Project.assets.length > 0) {
				Context.raw.colorIdPicked = true;
				UIToolbar.toolbarHandle.redraws = 1;
			}

			// Prevent painting the same spot
			let sameSpot = Context.raw.paintVec.x == Context.raw.lastPaintX && Context.raw.paintVec.y == Context.raw.lastPaintY;
			let lazy = Context.raw.tool == WorkspaceTool.ToolBrush && Context.raw.brushLazyRadius > 0;
			if (down && (sameSpot || lazy)) {
				Context.raw.painted++;
			}
			else {
				Context.raw.painted = 0;
			}
			Context.raw.lastPaintX = Context.raw.paintVec.x;
			Context.raw.lastPaintY = Context.raw.paintVec.y;

			if (Context.raw.tool == WorkspaceTool.ToolParticle) {
				Context.raw.painted = 0; // Always paint particles
			}

			if (Context.raw.painted == 0) {
				this.parseInputs();
			}

			if (Context.raw.painted <= 1) {
				Context.raw.pdirty = 1;
				Context.raw.rdirty = 2;
			}
		}
	}

	// static def: TNode = {
	// 	id: 0,
	// 	name: _tr("Brush Output"),
	// 	type: "BrushOutputNode",
	// 	x: 0,
	// 	y: 0,
	// 	color: 0xff4982a0,
	// 	inputs: [
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Position"),
	// 			type: "VECTOR",
	// 			color: 0xff63c763,
	// 			default_value: f32([0.0, 0.0, 0.0])
	// 		},
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Radius"),
	// 			type: "VALUE",
	// 			color: 0xffa1a1a1,
	// 			default_value: 1.0
	// 		},
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Scale"),
	// 			type: "VALUE",
	// 			color: 0xffa1a1a1,
	// 			default_value: 1.0
	// 		},
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Angle"),
	// 			type: "VALUE",
	// 			color: 0xffa1a1a1,
	// 			default_value: 0.0
	// 		},
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Opacity"),
	// 			type: "VALUE",
	// 			color: 0xffa1a1a1,
	// 			default_value: 1.0
	// 		},
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Hardness"),
	// 			type: "VALUE",
	// 			color: 0xffa1a1a1,
	// 			default_value: 1.0
	// 		},
	// 		{
	// 			id: 0,
	// 			node_id: 0,
	// 			name: _tr("Stencil"),
	// 			type: "VALUE",
	// 			color: 0xffa1a1a1,
	// 			default_value: 1.0
	// 		}
	// 	],
	// 	outputs: [],
	// 	buttons: [
	// 		{
	// 			name: _tr("Directional"),
	// 			type: "BOOL",
	// 			default_value: false,
	// 			output: 0
	// 		}
	// 	]
	// };
}
