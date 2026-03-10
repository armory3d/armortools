
#include "../global.h"

void vector_math2_node_init() {
	any_array_push(nodes_material_utilities, vector_math2_node_def);
	any_map_set(parser_material_node_vectors, "VECT_MATH", vector_math2_node_vector);
	any_map_set(parser_material_node_values, "VECT_MATH", vector_math2_node_value);
}

char *vector_math2_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char             *vec1 = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char             *vec2 = parser_material_parse_vector_input(node->inputs->buffer[1]);
	ui_node_button_t *but  = node->buttons->buffer[0]; // operation;
	char             *op   = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                     = string_copy(string_replace_all(op, " ", "_"));
	if (string_equals(op, "ADD")) {
		return string("(%s + %s)", vec1, vec2);
	}
	else if (string_equals(op, "SUBTRACT")) {
		return string("(%s - %s)", vec1, vec2);
	}
	else if (string_equals(op, "AVERAGE")) {
		return string("((%s + %s) / 2.0)", vec1, vec2);
	}
	else if (string_equals(op, "DOT_PRODUCT")) {
		return parser_material_to_vec3(string("dot(%s, %s)", vec1, vec2));
	}
	else if (string_equals(op, "LENGTH")) {
		return parser_material_to_vec3(string("length(%s)", vec1));
	}
	else if (string_equals(op, "DISTANCE")) {
		return parser_material_to_vec3(string("distance(%s, %s)", vec1, vec2));
	}
	else if (string_equals(op, "CROSS_PRODUCT")) {
		return string("cross(%s, %s)", vec1, vec2);
	}
	else if (string_equals(op, "NORMALIZE")) {
		return string("normalize(%s)", vec1);
	}
	else if (string_equals(op, "MULTIPLY")) {
		return string("(%s * %s)", vec1, vec2);
	}
	else if (string_equals(op, "DIVIDE")) {
		char *store = string("%s_vec2", parser_material_store_var_name(node));
		parser_material_write(parser_material_kong, string("var %s: float3 = %s;", store, vec2));
		parser_material_write(parser_material_kong, string("if (%s.x == 0.0) { %s.x = 0.000001; }", store, store));
		parser_material_write(parser_material_kong, string("if (%s.y == 0.0) { %s.y = 0.000001; }", store, store));
		parser_material_write(parser_material_kong, string("if (%s.z == 0.0) { %s.z = 0.000001; }", store, store));
		return string("(%s / %s)", vec1, vec2);
	}
	else if (string_equals(op, "PROJECT")) {
		return string("(dot(%s, %s) / dot(%s, %s) * %s)", vec1, vec2, vec2, vec2, vec2);
	}
	else if (string_equals(op, "REFLECT")) {
		return string("reflect(%s, normalize(%s))", vec1, vec2);
	}
	else if (string_equals(op, "SCALE")) {
		return string("(%s.x * %s)", vec2, vec1);
	}
	else if (string_equals(op, "ABSOLUTE")) {
		return string("abs3(%s)", vec1);
	}
	else if (string_equals(op, "MINIMUM")) {
		return string("min3(%s, %s)", vec1, vec2);
	}
	else if (string_equals(op, "MAXIMUM")) {
		return string("max3(%s, %s)", vec1, vec2);
	}
	else if (string_equals(op, "FLOOR")) {
		return string("floor3(%s)", vec1);
	}
	else if (string_equals(op, "CEIL")) {
		return string("ceil3(%s)", vec1);
	}
	else if (string_equals(op, "FRACTION")) {
		return string("frac3(%s)", vec1);
	}
	else if (string_equals(op, "MODULO")) {
		return string("(%s %% %s)", vec1, vec2);
	}
	else if (string_equals(op, "SNAP")) {
		return string("(floor3(%s / %s) * %s)", vec1, vec2, vec2);
	}
	else if (string_equals(op, "SINE")) {
		return string("float3(sin(%s.x), sin(%s.y), sin(%s.z))", vec1, vec1, vec1);
	}
	else if (string_equals(op, "COSINE")) {
		return string("float3(cos(%s.x), cos(%s.y), cos(%s.z))", vec1, vec1, vec1);
	}
	else { // TANGENT
		return string("float3(tan(%s.x), tan(%s.y), tan(%s.z))", vec1, vec1, vec1);
	}
}

char *vector_math2_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char             *vec1 = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char             *vec2 = parser_material_parse_vector_input(node->inputs->buffer[1]);
	ui_node_button_t *but  = node->buttons->buffer[0]; // operation;
	char             *op   = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                     = string_copy(string_replace_all(op, " ", "_"));
	if (string_equals(op, "DOT_PRODUCT")) {
		return string("dot(%s, %s)", vec1, vec2);
	}
	else if (string_equals(op, "LENGTH")) {
		return string("length(%s)", vec1);
	}
	else if (string_equals(op, "DISTANCE")) {
		return string("distance(%s, %s)", vec1, vec2);
	}
	else {
		return "0.0";
	}
}
