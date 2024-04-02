
type brush_output_node_t = {
	base?: logic_node_t;
	Directional?: bool; // button 0
};

function brush_output_node_create(arg: any): brush_output_node_t {
	let n: brush_output_node_t = {};
	n.base = logic_node_create();
	context_raw.run_brush = brush_output_node_run;
	context_raw.parse_brush_inputs = brush_output_node_parse_inputs;
	context_raw.brush_output_node_inst = n;
	return n;
}

function brush_output_node_parse_inputs(self: brush_output_node_t) {

	let last_mask: image_t = context_raw.brush_mask_image;
	let last_stencil: image_t = context_raw.brush_stencil_image;

	let input0: any = logic_node_input_get(self.base.inputs[0]);
	let input1: any = logic_node_input_get(self.base.inputs[1]);
	let input2: any = logic_node_input_get(self.base.inputs[2]);
	let input3: any = logic_node_input_get(self.base.inputs[3]);
	let input4: any = logic_node_input_get(self.base.inputs[4]);
	let input5: any = logic_node_input_get(self.base.inputs[5]);
	let input6: any = logic_node_input_get(self.base.inputs[6]);

	context_raw.paint_vec = input0;
	context_raw.brush_nodes_radius = input1;
	context_raw.brush_nodes_scale = input2;
	context_raw.brush_nodes_angle = input3;

	let opac: any = input4; // Float or texture name
	if (opac == null) {
		opac = 1.0;
	}
	if (typeof opac == "string") {
		context_raw.brush_mask_image_is_alpha = ends_with(opac, ".a");
		opac = substring(opac, 0, string_last_index_of(opac, "."));
		context_raw.brush_nodes_opacity = 1.0;
		let index: i32 = array_index_of(project_asset_names, opac);
		let asset: asset_t = project_assets[index];
		context_raw.brush_mask_image = project_get_image(asset);
	}
	else {
		context_raw.brush_nodes_opacity = opac;
		context_raw.brush_mask_image = null;
	}

	context_raw.brush_nodes_hardness = input5;

	let stencil: any = input6; // Float or texture name
	if (stencil == null) {
		stencil = 1.0;
	}
	if (typeof stencil == "string") {
		context_raw.brush_stencil_image_is_alpha = ends_with(stencil, ".a");
		stencil = substring(stencil, 0, string_last_index_of(stencil, "."));
		let index: i32 = array_index_of(project_asset_names, stencil);
		let asset: asset_t = project_assets[index];
		context_raw.brush_stencil_image = project_get_image(asset);
	}
	else {
		context_raw.brush_stencil_image = null;
	}

	if (last_mask != context_raw.brush_mask_image ||
		last_stencil != context_raw.brush_stencil_image) {
		make_material_parse_paint_material();
	}

	context_raw.brush_directional = self.Directional;
}

function brush_output_node_run(self: brush_output_node_t, from: i32) {
	let left: f32 = 0.0;
	let right: f32 = 1.0;
	if (context_raw.paint2d) {
		left = 1.0;
		right = (context_raw.split_view ? 2.0 : 1.0) + ui_view2d_ww / base_w();
	}

	// First time init
	if (context_raw.last_paint_x < 0 || context_raw.last_paint_y < 0) {
		context_raw.last_paint_vec_x = context_raw.paint_vec.x;
		context_raw.last_paint_vec_y = context_raw.paint_vec.y;
	}

	// Do not paint over fill layer
	let fill_layer: bool = context_raw.layer.fill_layer != null && context_raw.tool != workspace_tool_t.PICKER && context_raw.tool != workspace_tool_t.MATERIAL && context_raw.tool != workspace_tool_t.COLORID;

	// Do not paint over groups
	let group_layer: bool = slot_layer_is_group(context_raw.layer);

	// Paint bounds
	if (context_raw.paint_vec.x > left &&
		context_raw.paint_vec.x < right &&
		context_raw.paint_vec.y > 0 &&
		context_raw.paint_vec.y < 1 &&
		!fill_layer &&
		!group_layer &&
		(slot_layer_is_visible(context_raw.layer) || context_raw.paint2d) &&
		!ui_base_ui.is_hovered &&
		!base_is_dragging &&
		!base_is_resizing &&
		!base_is_scrolling() &&
		!base_is_combo_selected()) {

		// Set color pick
		let down: bool = mouse_down() || pen_down();
		if (down && context_raw.tool == workspace_tool_t.COLORID && project_assets.length > 0) {
			context_raw.colorid_picked = true;
			ui_toolbar_handle.redraws = 1;
		}

		// Prevent painting the same spot
		let same_spot: bool = context_raw.paint_vec.x == context_raw.last_paint_x && context_raw.paint_vec.y == context_raw.last_paint_y;
		let lazy: bool = context_raw.tool == workspace_tool_t.BRUSH && context_raw.brush_lazy_radius > 0;
		if (down && (same_spot || lazy)) {
			context_raw.painted++;
		}
		else {
			context_raw.painted = 0;
		}
		context_raw.last_paint_x = context_raw.paint_vec.x;
		context_raw.last_paint_y = context_raw.paint_vec.y;

		if (context_raw.tool == workspace_tool_t.PARTICLE) {
			context_raw.painted = 0; // Always paint particles
		}

		if (context_raw.painted == 0) {
			brush_output_node_parse_inputs(self);
		}

		if (context_raw.painted <= 1) {
			context_raw.pdirty = 1;
			context_raw.rdirty = 2;
		}
	}
}

// let brush_output_node_def: node_t = {
// 	id: 0,
// 	name: _tr("Brush Output"),
// 	type: "brush_output_node",
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
