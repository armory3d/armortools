
class BrushOutputNode extends LogicNode {

	Directional = false; // button 0

	constructor() {
		super();
		Context.raw.run_brush = this.run;
		Context.raw.parse_brush_inputs = this.parse_inputs;
	}

	parse_inputs = () => {
		let lastMask = Context.raw.brush_mask_image;
		let lastStencil = Context.raw.brush_stencil_image;

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

		Context.raw.paint_vec = input0;
		Context.raw.brush_nodes_radius = input1;
		Context.raw.brush_nodes_scale = input2;
		Context.raw.brush_nodes_angle = input3;

		let opac: any = input4; // Float or texture name
		if (opac == null) opac = 1.0;
		if (typeof opac == "string") {
			Context.raw.brush_mask_image_is_alpha = opac.endsWith(".a");
			opac = opac.substr(0, opac.lastIndexOf("."));
			Context.raw.brush_nodes_opacity = 1.0;
			let index = Project.asset_names.indexOf(opac);
			let asset = Project.assets[index];
			Context.raw.brush_mask_image = Project.get_image(asset);
		}
		else {
			Context.raw.brush_nodes_opacity = opac;
			Context.raw.brush_mask_image = null;
		}

		Context.raw.brush_nodes_hardness = input5;

		let stencil: any = input6; // Float or texture name
		if (stencil == null) stencil = 1.0;
		if (typeof stencil == "string") {
			Context.raw.brush_stencil_image_is_alpha = stencil.endsWith(".a");
			stencil = stencil.substr(0, stencil.lastIndexOf("."));
			let index = Project.asset_names.indexOf(stencil);
			let asset = Project.assets[index];
			Context.raw.brush_stencil_image = Project.get_image(asset);
		}
		else {
			Context.raw.brush_stencil_image = null;
		}

		if (lastMask != Context.raw.brush_mask_image ||
			lastStencil != Context.raw.brush_stencil_image) {
			MakeMaterial.parse_paint_material();
		}

		Context.raw.brush_directional = this.Directional;
	}

	run = (from: i32) => {
		let left = 0.0;
		let right = 1.0;
		if (Context.raw.paint2d) {
			left = 1.0;
			right = (Context.raw.split_view ? 2.0 : 1.0) + UIView2D.ww / Base.w();
		}

		// First time init
		if (Context.raw.last_paint_x < 0 || Context.raw.last_paint_y < 0) {
			Context.raw.last_paint_vec_x = Context.raw.paint_vec.x;
			Context.raw.last_paint_vec_y = Context.raw.paint_vec.y;
		}

		// Do not paint over fill layer
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != workspace_tool_t.PICKER && Context.raw.tool != workspace_tool_t.MATERIAL && Context.raw.tool != workspace_tool_t.COLORID;

		// Do not paint over groups
		let groupLayer = SlotLayer.is_group(Context.raw.layer);

		// Paint bounds
		if (Context.raw.paint_vec.x > left &&
			Context.raw.paint_vec.x < right &&
			Context.raw.paint_vec.y > 0 &&
			Context.raw.paint_vec.y < 1 &&
			!fillLayer &&
			!groupLayer &&
			(SlotLayer.is_visible(Context.raw.layer) || Context.raw.paint2d) &&
			!UIBase.ui.is_hovered &&
			!Base.is_dragging &&
			!Base.is_resizing &&
			!Base.is_scrolling() &&
			!Base.is_combo_selected()) {

			// Set color pick
			let down = mouse_down() || pen_down();
			if (down && Context.raw.tool == workspace_tool_t.COLORID && Project.assets.length > 0) {
				Context.raw.colorid_picked = true;
				UIToolbar.toolbar_handle.redraws = 1;
			}

			// Prevent painting the same spot
			let sameSpot = Context.raw.paint_vec.x == Context.raw.last_paint_x && Context.raw.paint_vec.y == Context.raw.last_paint_y;
			let lazy = Context.raw.tool == workspace_tool_t.BRUSH && Context.raw.brush_lazy_radius > 0;
			if (down && (sameSpot || lazy)) {
				Context.raw.painted++;
			}
			else {
				Context.raw.painted = 0;
			}
			Context.raw.last_paint_x = Context.raw.paint_vec.x;
			Context.raw.last_paint_y = Context.raw.paint_vec.y;

			if (Context.raw.tool == workspace_tool_t.PARTICLE) {
				Context.raw.painted = 0; // Always paint particles
			}

			if (Context.raw.painted == 0) {
				this.parse_inputs();
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
