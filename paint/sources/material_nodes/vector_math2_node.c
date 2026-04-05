
#include "../global.h"

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
	else if (string_equals(op, "POWER")) {
		return string("float3(pow(%s.x, %s.x), pow(%s.y, %s.y), pow(%s.z, %s.z))", vec1, vec2, vec1, vec2, vec1, vec2);
	}
	else if (string_equals(op, "SIGN")) {
		return string("float3(sign(%s.x), sign(%s.y), sign(%s.z))", vec1, vec1, vec1);
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

void vector_math2_node_init() {

	char *vector_math_operation_data =
	    string("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s", _tr("Add"), _tr("Subtract"),
	           _tr("Multiply"), _tr("Divide"), _tr("Average"), _tr("Cross Product"), _tr("Project"), _tr("Reflect"), _tr("Dot Product"), _tr("Distance"),
	           _tr("Length"), _tr("Scale"), _tr("Normalize"), _tr("Absolute"), _tr("Power"), _tr("Sign"), _tr("Minimum"), _tr("Maximum"), _tr("Floor"),
	           _tr("Ceil"), _tr("Fraction"), _tr("Modulo"), _tr("Snap"), _tr("Sine"), _tr("Cosine"), _tr("Tangent"));
	ui_node_t *vector_math2_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Vector Math"),
	                              .type   = "VECT_MATH",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff62676d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(vector_math_operation_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});

	any_array_push(nodes_material_utilities, vector_math2_node_def);
	any_map_set(parser_material_node_vectors, "VECT_MATH", vector_math2_node_vector);
	any_map_set(parser_material_node_values, "VECT_MATH", vector_math2_node_value);
}
