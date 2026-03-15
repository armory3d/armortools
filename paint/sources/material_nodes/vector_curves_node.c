
#include "../global.h"

void vector_curves_node_init() {
	any_array_push(nodes_material_utilities, vector_curves_node_def);
	any_map_set(parser_material_node_vectors, "CURVE_VEC", vector_curves_node_vector);
	any_map_set(ui_nodes_custom_buttons, "nodes_material_vector_curves_button", nodes_material_vector_curves_button);
}

char *parser_material_vector_curve(char *name, char *fac, f32 *points, i32 num) {
	// Write Ys array
	char *ys_var = string("%s_ys", name);
	parser_material_write(parser_material_kong, string("var %s: float[%s];", ys_var, i32_to_string(num))); // TODO: Make const
	for (i32 i = 0; i < num; ++i) {
		f32 p = points[i * 2 + 1];
		parser_material_write(parser_material_kong, string("%s[%s] = %s;", ys_var, i32_to_string(i), f32_to_string(p)));
	}
	// Get index
	char *fac_var = string("%s_fac", name);
	parser_material_write(parser_material_kong, string("var %s: float = %s;", fac_var, fac));
	char *index = "0";
	for (i32 i = 1; i < num; ++i) {
		f32 p = points[i * 2 + 0];
		index = string("%s + (%s > %s ? 1 : 0)", index, fac_var, f32_to_string(p));
	}
	// Write index
	char *index_var = string("%s_i", name);
	parser_material_write(parser_material_kong, string("var %s: int = %s;", index_var, index));
	// Linear
	// Write Xs array
	char *facs_var = string("%s_xs", name);
	parser_material_write(parser_material_kong, string("var %s: float[%s];", facs_var, i32_to_string(num))); // TODO: Make const
	for (i32 i = 0; i < num; ++i) {
		f32 p = points[i * 2 + 0];
		parser_material_write(parser_material_kong, string("%s[%s] = %s;", facs_var, i32_to_string(i), f32_to_string(p)));
	}
	// Map vector
	return "0.0"; ////
	              // return "lerp(" +
	              // 	ys_var + "[" + index_var + "], " + ys_var + "[" + index_var + " + 1], (" + fac_var + " - " +
	              // 	facs_var + "[" + index_var + "]) * (1.0 / (" + facs_var + "[" + index_var + " + 1] - " + facs_var + "[" + index_var + "])))";
}

char *vector_curves_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char        *fac    = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char        *vec    = parser_material_parse_vector_input(node->inputs->buffer[1]);
	f32_array_t *curves = node->buttons->buffer[0]->default_value;
	if (curves->buffer[96] == 0.0) {
		curves->buffer[96] = 1.0;
		curves->buffer[97] = 1.0;
		curves->buffer[98] = 1.0;
	}
	char *name = parser_material_node_name(node, NULL);
	char *vc0  = parser_material_vector_curve(string("%s0", name), string("%s.x", vec), curves->buffer + 32 * 0, curves->buffer[96]);
	char *vc1  = parser_material_vector_curve(string("%s1", name), string("%s.y", vec), curves->buffer + 32 * 1, curves->buffer[97]);
	char *vc2  = parser_material_vector_curve(string("%s2", name), string("%s.z", vec), curves->buffer + 32 * 2, curves->buffer[98]);
	// mapping.curves[0].points[0].handle_type // bezier curve
	return string("(float3(%s, %s, %s) * %s)", vc0, vc1, vc2, fac);
}

void nodes_material_vector_curves_button(i32 node_id) {
	ui_node_t        *node    = ui_get_node(ui_nodes_get_canvas(true)->nodes, node_id);
	ui_node_button_t *but     = node->buttons->buffer[0];
	ui_handle_t      *nhandle = ui_nest(ui_handle(__ID__), node->id);
	ui_row3();
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 0, "X", "");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 1, "Y", "");
	ui_radio(ui_nest(ui_nest(nhandle, 0), 1), 2, "Z", "");
	// Preview
	i32          axis = ui_nest(ui_nest(nhandle, 0), 1)->i;
	f32_array_t *val  = but->default_value;
	ui->_y += UI_LINE_H() * 5;
	i32 num = val->buffer[96 + axis];
	if (num == 0) {
		// Init
		val->buffer[96 + 0] = 1;
		val->buffer[96 + 1] = 1;
		val->buffer[96 + 2] = 1;
	}
	// Edit
	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        1 / 5.0,
	        1 / 5.0,
	        3 / 5.0,
	    },
	    3);
	ui_row(row);
	if (ui_button("+", UI_ALIGN_CENTER, "")) {
		// TODO:
		// val[axis * 32 + num * 2 + 0] = 0.0;
		// val[axis * 32 + num * 2 + 1] = 0.0;
		// num++;
		// val[96 + axis] = num;
	}
	if (ui_button("-", UI_ALIGN_CENTER, "")) {
		if (num > 1) {
			num--;
			val->buffer[96 + axis] = num;
		}
	}
	ui_handle_t *ihandle = ui_nest(ui_nest(ui_nest(nhandle, 0), 2), axis);
	if (ihandle->init) {
		ihandle->i = 0;
	}
	i32 i = math_floor(ui_slider(ihandle, "Index", 0, num - 1, false, 1, true, UI_ALIGN_LEFT, true));
	if (i >= num || i < 0) {
		ihandle->f = i = num - 1; // Stay in bounds
	}
	ui_row2();
	ui_nest(ui_nest(nhandle, 0), 3)->f = val->buffer[axis * 32 + i * 2 + 0];
	ui_nest(ui_nest(nhandle, 0), 4)->f = val->buffer[axis * 32 + i * 2 + 1];
	ui_handle_t *h1                    = ui_nest(ui_nest(nhandle, 0), 3);
	if (h1->init) {
		h1->f = 0.0;
	}
	ui_handle_t *h2 = ui_nest(ui_nest(nhandle, 0), 4);
	if (h2->init) {
		h2->f = 0.0;
	}
	val->buffer[axis * 32 + i * 2 + 0] = ui_slider(h1, "X", -1, 1, true, 100, true, UI_ALIGN_LEFT, true);
	val->buffer[axis * 32 + i * 2 + 1] = ui_slider(h2, "Y", -1, 1, true, 100, true, UI_ALIGN_LEFT, true);
}
