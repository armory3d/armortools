
type brush_output_node_t = {
	base?: logic_node_t;
	Directional?: bool;
};

function brush_output_node_create(): brush_output_node_t {
	let n: brush_output_node_t = {};
	n.base = logic_node_create();
	context_raw.run_brush = brush_output_node_run;
	context_raw.parse_brush_inputs = brush_output_node_parse_inputs;
	return n;
}

function brush_output_node_parse_inputs(self: brush_output_node_t) {
	let last_mask = context_raw.brush_mask_image;
	let last_stencil = context_raw.brush_stencil_image;

	let input0: any;
	let input1: any;
	let input2: any;
	let input3: any;
	let input4: any;
	logic_node_input_get(self.base.inputs[0], function (value: any) { input0 = value; });
	logic_node_input_get(self.base.inputs[1], function (value: any) { input1 = value; });
	logic_node_input_get(self.base.inputs[2], function (value: any) { input2 = value; });
	logic_node_input_get(self.base.inputs[3], function (value: any) { input3 = value; });
	logic_node_input_get(self.base.inputs[4], function (value: any) { input4 = value; });

	context_raw.paint_vec = input0;
	context_raw.brush_nodes_radius = input1;

	let opac: any = input2; // Float or texture name
	if (opac == null) opac = 1.0;
	if (typeof opac == "string") {
		context_raw.brush_mask_image_is_alpha = ends_with(opac, ".a");
		opac = substring(opac, 0, string_last_index_of(opac, "."));
		context_raw.brush_nodes_opacity = 1.0;
		let index = array_index_of(project_asset_names, opac);
		let asset = project_assets[index];
		context_raw.brush_mask_image = project_get_image(asset);
	}
	else {
		context_raw.brush_nodes_opacity = opac;
		context_raw.brush_mask_image = null;
	}

	context_raw.brush_nodes_hardness = input3;

	let stencil: any = input4; // Float or texture name
	if (stencil == null) {
		stencil = 1.0;
	}
	if (typeof stencil == "string") {
		context_raw.brush_stencil_image_is_alpha = ends_with(stencil, ".a");
		stencil = substring(stencil, 0, string_last_index_of(stencil, "."));
		let index = array_index_of(project_asset_names, stencil);
		let asset = project_assets[index];
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
	let fill_layer: bool = context_raw.layer.fill_layer != null;

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

		let down = mouse_down() || pen_down();

		// Prevent painting the same spot
		let same_spot = context_raw.paint_vec.x == context_raw.last_paint_x && context_raw.paint_vec.y == context_raw.last_paint_y;
		let lazy = context_raw.tool == workspace_tool_t.BRUSH && context_raw.brush_lazy_radius > 0;
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

		if (context_raw.painted == 0) {
			context_raw.pdirty = 1;
			context_raw.rdirty = 2;
			history_push_undo2 = true; ////
		}
	}
}
