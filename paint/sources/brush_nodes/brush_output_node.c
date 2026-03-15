
#include "../global.h"

brush_output_node_t *brush_output_node_create(ui_node_t *raw, f32_array_t *args) {
	context_raw->run_brush          = brush_output_node_run;
	context_raw->parse_brush_inputs = brush_output_node_parse_inputs;

	brush_output_node_t *n = GC_ALLOC_INIT(brush_output_node_t, {0});
	n->base                = logic_node_create(n);
	n->raw                 = raw;
	context_raw->brush_output_node_inst = n;
	return n;
}

void brush_output_node_parse_inputs(brush_output_node_t *self) {
	gpu_texture_t *last_mask    = context_raw->brush_mask_image;
	gpu_texture_t *last_stencil = context_raw->brush_stencil_image;

	logic_node_value_t *input0 = logic_node_input_get(self->base->inputs->buffer[0]);
	logic_node_value_t *input1 = logic_node_input_get(self->base->inputs->buffer[1]);
	logic_node_value_t *input2 = logic_node_input_get(self->base->inputs->buffer[2]);
	logic_node_value_t *input3 = logic_node_input_get(self->base->inputs->buffer[3]);
	logic_node_value_t *input4 = logic_node_input_get(self->base->inputs->buffer[4]);
	logic_node_value_t *input5 = logic_node_input_get(self->base->inputs->buffer[5]);
	logic_node_value_t *input6 = logic_node_input_get(self->base->inputs->buffer[6]);

	context_raw->paint_vec          = input0->_vec4;
	context_raw->brush_nodes_radius = input1->_f32;
	context_raw->brush_nodes_scale  = input2->_f32;
	context_raw->brush_nodes_angle  = input3->_f32;

	logic_node_value_t *opac = input4; // Float or texture name
	if (opac == NULL) {
		opac = GC_ALLOC_INIT(logic_node_value_t, {._f32 = 1.0});
	}
	if (opac->_str != NULL) { // string
		context_raw->brush_mask_image_is_alpha = ends_with(opac->_str, ".a");
		opac->_str                             = string_copy(substring(opac->_str, 0, string_last_index_of(opac->_str, ".")));
		context_raw->brush_nodes_opacity       = 1.0;
		i32 index                              = string_array_index_of(project_asset_names, opac->_str);
		if (index != -1) {
			asset_t *asset                = project_assets->buffer[index];
			context_raw->brush_mask_image = project_get_image(asset);
		}
	}
	else {
		context_raw->brush_nodes_opacity = opac->_f32;
		context_raw->brush_mask_image    = NULL;
	}

	context_raw->brush_nodes_hardness = input5->_f32;

	logic_node_value_t *stencil = input6; // Float or texture name
	if (stencil == NULL) {
		stencil = GC_ALLOC_INIT(logic_node_value_t, {._f32 = 1.0});
	}
	if (stencil->_str != NULL) { // string
		context_raw->brush_stencil_image_is_alpha = ends_with(stencil->_str, ".a");
		stencil->_str                             = string_copy(substring(stencil->_str, 0, string_last_index_of(stencil->_str, ".")));
		i32 index                                 = string_array_index_of(project_asset_names, stencil->_str);
		if (index != -1) {
			asset_t *asset                   = project_assets->buffer[index];
			context_raw->brush_stencil_image = project_get_image(asset);
		}
	}
	else {
		context_raw->brush_stencil_image = NULL;
	}

	if (last_mask != context_raw->brush_mask_image || last_stencil != context_raw->brush_stencil_image) {
		make_material_parse_paint_material(true);
	}

	context_raw->brush_directional = self->raw->buttons->buffer[0]->default_value->buffer[0] > 0.0;
}

void brush_output_node_run(brush_output_node_t *self, i32 from) {
	f32 left   = 0.0;
	f32 right  = 1.0;
	f32 top    = 0.0;
	f32 bottom = 1.0;

	if (context_raw->paint2d) {
		left  = 1.0;
		right = (context_raw->split_view ? 2.0 : 1.0) + ui_view2d_ww / (float)base_w();
	}

	// Do not paint over floating toolbar
	if (context_is_floating_toolbar()) {
		i32 w = ui_toolbar_x() + ui_toolbar_w(false);
		left += w / (float)sys_w();
		top += w / (float)sys_h();

		#ifdef IRON_IOS
		if (config_is_iphone()) {
			top += w / (float)sys_h();
		}
		#endif
	}

	// First time init
	if (context_raw->last_paint_x < 0 || context_raw->last_paint_y < 0) {
		context_raw->last_paint_vec_x = context_raw->paint_vec.x;
		context_raw->last_paint_vec_y = context_raw->paint_vec.y;
	}

	// Paint bounds
	if (context_raw->paint_vec.x < left || context_raw->paint_vec.x > right || context_raw->paint_vec.y < top || context_raw->paint_vec.y > bottom) {
		return;
	}

	// Do not paint over fill layer
	bool fill_layer = context_raw->layer->fill_layer != NULL && context_raw->tool != TOOL_TYPE_PICKER && context_raw->tool != TOOL_TYPE_MATERIAL &&
	                  context_raw->tool != TOOL_TYPE_COLORID;
	if (fill_layer) {
		return;
	}

	// Do not paint over groups
	if (slot_layer_is_group(context_raw->layer)) {
		return;
	}

	if (context_raw->brush_locked) {
		return;
	}

	if (!slot_layer_is_visible(context_raw->layer) && !context_raw->paint2d) {
		return;
	}

	if (ui->is_hovered || base_is_dragging || base_is_resizing || ui->is_scrolling || ui->combo_selected_handle != NULL) {
		return;
	}

	brush_output_paint(self);
}

void brush_output_paint(brush_output_node_t *self) {
	bool down = mouse_down("left") || pen_down("tip");

	// Set color pick
	if (down && context_raw->tool == TOOL_TYPE_COLORID && project_assets->length > 0) {
		context_raw->colorid_picked = true;
		ui_toolbar_handle->redraws  = 1;
	}

	// Prevent painting the same spot
	bool same_spot = context_raw->paint_vec.x == context_raw->last_paint_x && context_raw->paint_vec.y == context_raw->last_paint_y;
	bool lazy      = context_raw->tool == TOOL_TYPE_BRUSH && context_raw->brush_lazy_radius > 0;
	if (down && (same_spot || lazy)) {
		context_raw->painted++;
	}
	else {
		context_raw->painted = 0;
	}
	context_raw->last_paint_x = context_raw->paint_vec.x;
	context_raw->last_paint_y = context_raw->paint_vec.y;

	if (context_raw->tool == TOOL_TYPE_PARTICLE) {
		context_raw->painted = 0; // Always paint particles
	}

	if (context_raw->painted == 0) {
		brush_output_node_parse_inputs(self);
	}

	if (context_raw->painted <= 1) {
		context_raw->pdirty = 1;
		context_raw->rdirty = 2;
		sculpt_push_undo    = true;
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
// 			default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Radius"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(1.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Scale"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(1.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Angle"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(0.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Opacity"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(1.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Hardness"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(1.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Stencil"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(1.0),
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			display: 0
// 		}
// 	],
// 	outputs: [],
// 	buttons: [
// 		{
// 			name: _tr("Directional"),
// 			type: "BOOL",
// 			output: 0,
// 			default_value: f32_array_create_x(0),
//			data: NULL,
//			min: 0.0,
//			max: 1.0,
//			precision: 100,
//			height: 0
// 		}
// 	],
//	width: 0,
//	flags: 0
// };
