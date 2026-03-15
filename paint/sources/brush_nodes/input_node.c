
#include "../global.h"

input_node_t *input_node_create(ui_node_t *raw, f32_array_t *args) {
	float_node_t *n = GC_ALLOC_INIT(float_node_t, {0});
	n->base         = logic_node_create(n);
	n->base->get    = input_node_get;

	if (!input_node_registered) {
		input_node_registered = true;
		sys_notify_on_update(input_node_update, n);
	}

	return n;
}

void input_node_update(float_node_t *self) {
	if (context_raw->split_view) {
		context_raw->view_index = mouse_view_x() > base_w() / 2.0 ? 1 : 0;
	}

	bool decal_mask = context_is_decal_mask_paint();
	char *ruler_paint = string("%s+%s", any_map_get(config_keymap, "brush_ruler"), any_map_get(config_keymap, "action_paint"));
	bool lazy_paint = context_raw->brush_lazy_radius > 0 && (operator_shortcut(any_map_get(config_keymap, "action_paint"), SHORTCUT_TYPE_DOWN) ||
	                                                         operator_shortcut(ruler_paint, SHORTCUT_TYPE_DOWN) || decal_mask);

	f32 paint_x = mouse_view_x() / (float)sys_w();
	f32 paint_y = mouse_view_y() / (float)sys_h();

	if (mouse_started("left")) {
		input_node_start_x = mouse_view_x() / (float)sys_w();
		input_node_start_y = mouse_view_y() / (float)sys_h();
	}

	if (pen_down("tip")) {
		paint_x = pen_view_x() / (float)sys_w();
		paint_y = pen_view_y() / (float)sys_h();
	}
	if (pen_started("tip")) {
		input_node_start_x = pen_view_x() / (float)sys_w();
		input_node_start_y = pen_view_y() / (float)sys_h();
	}

	if (operator_shortcut(ruler_paint, SHORTCUT_TYPE_DOWN)) {
		if (input_node_lock_x) {
			paint_x = input_node_start_x;
		}
		if (input_node_lock_y) {
			paint_y = input_node_start_y;
		}
	}

	if (context_raw->brush_lazy_radius > 0) {
		context_raw->brush_lazy_x = paint_x;
		context_raw->brush_lazy_y = paint_y;
	}
	if (!lazy_paint) {
		input_node_coords.x = paint_x;
		input_node_coords.y = paint_y;
	}

	if (context_raw->split_view) {
		context_raw->view_index = -1;
	}

	if (input_node_lock_begin) {
		f32 dx = math_abs(input_node_lock_start_x - mouse_view_x());
		f32 dy = math_abs(input_node_lock_start_y - mouse_view_y());
		if (dx > 1 || dy > 1) {
			input_node_lock_begin = false;
			if (dx > dy) {
				input_node_lock_y = true;
			}
			else {
				input_node_lock_x = true;
			}
		}
	}

	if (keyboard_started(any_map_get(config_keymap, "brush_ruler"))) {
		input_node_lock_start_x = mouse_view_x();
		input_node_lock_start_y = mouse_view_y();
		input_node_lock_begin   = true;
	}
	else if (keyboard_released(any_map_get(config_keymap, "brush_ruler"))) {
		input_node_lock_x = input_node_lock_y = input_node_lock_begin = false;
	}

	if (context_raw->brush_lazy_radius > 0) {
		vec4_t v1 = vec4_create(context_raw->brush_lazy_x * sys_w(), context_raw->brush_lazy_y * sys_h(), 0.0, 1.0);
		vec4_t v2 = vec4_create(input_node_coords.x * sys_w(), input_node_coords.y * sys_h(), 0.0, 1.0);
		f32    d  = vec4_dist(v1, v2);
		f32    r  = context_raw->brush_lazy_radius * 85;
		if (d > r) {
			vec4_t v3           = vec4_create(0.0, 0.0, 0.0, 1.0);
			v3                  = vec4_sub(v2, v1);
			v3                  = vec4_norm(v3);
			v3                  = vec4_mult(v3, 1.0 - context_raw->brush_lazy_step);
			v3                  = vec4_mult(v3, r);
			v2                  = vec4_add(v1, v3);
			input_node_coords.x = v2.x / (float)sys_w();
			input_node_coords.y = v2.y / (float)sys_h();
			// Parse brush inputs once on next draw
			context_raw->painted = -1;
		}
		context_raw->last_paint_x = -1;
		context_raw->last_paint_y = -1;
	}

	context_raw->parse_brush_inputs(context_raw->brush_output_node_inst);
}

logic_node_value_t *input_node_get(input_node_t *self, i32 from) {
	context_raw->brush_lazy_radius = logic_node_input_get(self->base->inputs->buffer[0])->_f32;
	context_raw->brush_lazy_step   = logic_node_input_get(self->base->inputs->buffer[1])->_f32;
	logic_node_value_t *v          = GC_ALLOC_INIT(logic_node_value_t, {._vec4 = input_node_coords});
	return v;
}
