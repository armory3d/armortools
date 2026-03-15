
#include "../global.h"

void color_ramp_node_init() {
	any_array_push(nodes_material_utilities, color_ramp_node_def);
	any_map_set(parser_material_node_vectors, "VALTORGB", color_ramp_node_vector);
	any_map_set(ui_nodes_custom_buttons, "nodes_material_color_ramp_button", nodes_material_color_ramp_button);
}

char *color_ramp_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char        *fac    = parser_material_parse_value_input(node->inputs->buffer[0], false);
	i32          data0  = node->buttons->buffer[0]->data->buffer[0];
	char        *interp = data0 == 0 ? "LINEAR" : "CONSTANT";
	f32_array_t *elems  = node->buttons->buffer[0]->default_value;
	i32          len    = elems->length / (float)5;
	if (len == 1) {
		return parser_material_vec3(elems);
	}
	// Write cols array
	char *cols_var = string("%s_cols", parser_material_node_name(node, NULL));
	parser_material_write(parser_material_kong, string("var %s: float3[%s];", cols_var, i32_to_string(len))); // TODO: Make const
	for (i32 i = 0; i < len; ++i) {
		f32_array_t *tmp = f32_array_create_from_raw((f32[]){}, 0);
		f32_array_push(tmp, elems->buffer[i * 5]);
		f32_array_push(tmp, elems->buffer[i * 5 + 1]);
		f32_array_push(tmp, elems->buffer[i * 5 + 2]);
		parser_material_write(parser_material_kong, string("%s[%s] = %s;", cols_var, i32_to_string(i), parser_material_vec3(tmp)));
	}
	// Get index
	char *fac_var = string("%s_fac", parser_material_node_name(node, NULL));
	parser_material_write(parser_material_kong, string("var %s: float = %s;", fac_var, fac));
	char *index = "0";
	for (i32 i = 1; i < len; ++i) {
		f32 e = elems->buffer[i * 5 + 4];
		index = string("%s + (%s > %s ? 1 : 0)", index, fac_var, f32_to_string(e));
	}
	// Write index
	char *index_var = string("%s_i", parser_material_node_name(node, NULL));
	parser_material_write(parser_material_kong, string("var %s: int = %s;", index_var, index));
	if (string_equals(interp, "CONSTANT")) {
		return string("%s[%s]", cols_var, index_var);
	}
	else { // Linear
		// Write facs array
		char *facs_var = string("%s_facs", parser_material_node_name(node, NULL));
		parser_material_write(parser_material_kong, string("var %s: float[%s];", facs_var, i32_to_string(len))); // TODO: Make const
		for (i32 i = 0; i < len; ++i) {
			f32 e = elems->buffer[i * 5 + 4];
			parser_material_write(parser_material_kong, string("%s[%s] = %s;", facs_var, i32_to_string(i), f32_to_string(e)));
		}
		// Mix color
		// float f = (pos - start) * (1.0 / (finish - start))
		// TODO: index_var + 1 out of bounds
		return string("lerp3(%s[%s], %s[%s + 1], (%s - %s[%s]) * (1.0 / (%s[%s + 1] - %s[%s])) ))", cols_var, index_var, cols_var, index_var, fac_var, facs_var,
		              index_var, facs_var, index_var, facs_var, index_var);
	}
}

void nodes_material_color_ramp_button(i32 node_id) {
	ui_nodes_t       *nodes   = ui_nodes_get_nodes();
	ui_node_t        *node    = ui_get_node(ui_nodes_get_canvas(true)->nodes, node_id);
	ui_node_button_t *but     = node->buttons->buffer[0];
	ui_handle_t      *nhandle = ui_nest(ui_handle(__ID__), node->id);
	f32               nx      = ui->_x;
	f32               ny      = ui->_y;
	f32_array_t      *vals    = but->default_value; // [r, g, b, a, pos, r, g, b, a, pos, ..]
	f32               sw      = ui->_w / (float)UI_NODES_SCALE();
	// Preview
	for (i32 i = 0; i < vals->length / (float)5; ++i) {
		f32 pos = vals->buffer[i * 5 + 4];
		i32 col = color_from_floats(vals->buffer[i * 5 + 0], vals->buffer[i * 5 + 1], vals->buffer[i * 5 + 2], 1.0);
		ui_fill(pos * sw, 0, (1.0 - pos) * sw, UI_LINE_H() - 2 * UI_NODES_SCALE(), col);
	}
	ui->_y += UI_LINE_H();
	// Edit
	ui_handle_t *ihandle = ui_nest(ui_nest(nhandle, 0), 2);
	f32_array_t *row     = f32_array_create_from_raw(
        (f32[]){
            1 / (float)4,
            1 / (float)4,
            2 / (float)4,
        },
        3);
	ui_row(row);
	if (ui_button("+", UI_ALIGN_CENTER, "")) {
		// TODO:
		// array_push(vals, vals[vals.length - 5]); // r
		// array_push(vals, vals[vals.length - 5]); // g
		// array_push(vals, vals[vals.length - 5]); // b
		// array_push(vals, vals[vals.length - 5]); // a
		// array_push(vals, 1.0); // pos
		// ihandle.f += 1;
	}
	if (ui_button("-", UI_ALIGN_CENTER, "") && vals->length > 5) {
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		array_pop(vals);
		ihandle->f -= 1;
	}
	ui_handle_t *h = ui_nest(ui_nest(nhandle, 0), 1);
	if (h->init) {
		h->i = but->data->buffer[0];
	}
	string_t_array_t *interpolate_combo = any_array_create_from_raw(
	    (void *[]){
	        tr("Linear"),
	        tr("Constant"),
	    },
	    2);
	but->data->buffer[0] = ui_combo(h, interpolate_combo, tr("Interpolate"), false, UI_ALIGN_LEFT, true);
	ui_row2();
	i32 i = math_floor(ui_slider(ihandle, "Index", 0, (vals->length / (float)5) - 1, false, 1, true, UI_ALIGN_LEFT, true));
	if (i >= (vals->length * 5) || i < 0) {
		ihandle->f = i = (vals->length / (float)5) - 1; // Stay in bounds
	}
	ui_nest(ui_nest(nhandle, 0), 3)->f = vals->buffer[i * 5 + 4];
	vals->buffer[i * 5 + 4]            = ui_slider(ui_nest(ui_nest(nhandle, 0), 3), "Pos", 0, 1, true, 100, true, UI_ALIGN_LEFT, true);
	if (vals->buffer[i * 5 + 4] > 1.0) {
		vals->buffer[i * 5 + 4] = 1.0; // Stay in bounds
	}
	else if (vals->buffer[i * 5 + 4] < 0.0) {
		vals->buffer[i * 5 + 4] = 0.0;
	}
	ui_handle_t *chandle = ui_nest(ui_nest(nhandle, 0), 4);
	chandle->color       = color_from_floats(vals->buffer[i * 5 + 0], vals->buffer[i * 5 + 1], vals->buffer[i * 5 + 2], 1.0);
	if (ui_text("", UI_ALIGN_RIGHT, chandle->color) == UI_STATE_STARTED) {
		f32 rx                = nx + ui->_w - ui_p(37);
		f32 ry                = ny - ui_p(5);
		nodes->_input_started = ui->input_started = false;
		ui_nodes_rgba_popup(chandle, vals->buffer + i * 5, math_floor(rx), math_floor(ry + UI_ELEMENT_H()));
	}
	vals->buffer[i * 5 + 0] = color_get_rb(chandle->color) / (float)255;
	vals->buffer[i * 5 + 1] = color_get_gb(chandle->color) / (float)255;
	vals->buffer[i * 5 + 2] = color_get_bb(chandle->color) / (float)255;
}
