
#include "../global.h"

char *math2_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char             *val1 = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char             *val2 = parser_material_parse_value_input(node->inputs->buffer[1], false);
	ui_node_button_t *but  = node->buttons->buffer[0]; // operation
	char             *op   = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                     = string_copy(string_replace_all(op, " ", "_"));
	bool  use_clamp        = node->buttons->buffer[1]->default_value->buffer[0] > 0;
	char *out_val          = "";
	if (string_equals(op, "ADD")) {
		out_val = string("(%s + %s)", val1, val2);
	}
	else if (string_equals(op, "SUBTRACT")) {
		out_val = string("(%s - %s)", val1, val2);
	}
	else if (string_equals(op, "MULTIPLY")) {
		out_val = string("(%s * %s)", val1, val2);
	}
	else if (string_equals(op, "DIVIDE")) {
		char *store = string("%s_divide", parser_material_store_var_name(node));
		parser_material_write(parser_material_kong, string("var %s: float = %s;", store, val2));
		parser_material_write(parser_material_kong, string("if (%s == 0.0) { %s = %s; }", store, store, f32_to_string(parser_material_eps)));
		out_val = string("(%s / %s)", val1, store);
	}
	else if (string_equals(op, "POWER")) {
		out_val = string("pow(%s, %s)", val1, val2);
	}
	else if (string_equals(op, "LOGARITHM")) {
		out_val = string("log(%s)", val1);
	}
	else if (string_equals(op, "SQUARE_ROOT")) {
		out_val = string("sqrt(%s)", val1);
	}
	else if (string_equals(op, "INVERSE_SQUARE_ROOT")) {
		out_val = string("rsqrt(%s)", val1);
	}
	else if (string_equals(op, "EXPONENT")) {
		out_val = string("exp(%s)", val1);
	}
	else if (string_equals(op, "ABSOLUTE")) {
		out_val = string("abs(%s)", val1);
	}
	else if (string_equals(op, "MINIMUM")) {
		out_val = string("min(%s, %s)", val1, val2);
	}
	else if (string_equals(op, "MAXIMUM")) {
		out_val = string("max(%s, %s)", val1, val2);
	}
	else if (string_equals(op, "LESS_THAN")) {
		// out_val = "float(" + val1 + " < " + val2 + ")";
		char *store = string("%s_lessthan", parser_material_store_var_name(node));
		parser_material_write(parser_material_kong, string("var %s: float = 0.0;", store));
		parser_material_write(parser_material_kong, string("if (%s < %s) { %s = 1.0; }", val1, val2, store));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "GREATER_THAN")) {
		// out_val = "float(" + val1 + " > " + val2 + ")";
		char *store = string("%s_greaterthan", parser_material_store_var_name(node));
		parser_material_write(parser_material_kong, string("var %s: float = 0.0;", store));
		parser_material_write(parser_material_kong, string("if (%s > %s) { %s = 1.0; }", val1, val2, store));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "SIGN")) {
		out_val = string("sign(%s)", val1);
	}
	else if (string_equals(op, "ROUND")) {
		out_val = string("floor(%s + 0.5)", val1);
	}
	else if (string_equals(op, "FLOOR")) {
		out_val = string("floor(%s)", val1);
	}
	else if (string_equals(op, "CEIL")) {
		out_val = string("ceil(%s)", val1);
	}
	else if (string_equals(op, "SNAP")) {
		out_val = string("(floor(%s / %s) * %s)", val1, val2, val2);
	}
	else if (string_equals(op, "TRUNCATE")) {
		out_val = string("trunc(%s)", val1);
	}
	else if (string_equals(op, "FRACTION")) {
		out_val = string("frac(%s)", val1);
	}
	else if (string_equals(op, "TRUNCATED_MODULO")) {
		out_val = string("(%s %% %s)", val1, val2);
	}
	else if (string_equals(op, "FLOORED_MODULO")) {
		char *store = string("%s_flooredmod", parser_material_store_var_name(node));
		parser_material_write(parser_material_kong, string("var %s: float = 0.0;", store));
		parser_material_write(parser_material_kong, string("if (%s != 0.0) { %s = %s - %s * floor(%s / %s); }", val2, store, val1, val2, val1, val2));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "PING-PONG")) {
		char *store = string("%s_pingpong", parser_material_store_var_name(node));
		parser_material_write(parser_material_kong, string("var %s: float = 0.0;", store));
		parser_material_write(parser_material_kong,
		                      string("if (%s != 0.0) { %s = abs(frac((%s - %s) / (%s * 2.0)) * %s * 2.0 - %s); }", val2, store, val1, val2, val2, val2, val2));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "SINE")) {
		out_val = string("sin(%s)", val1);
	}
	else if (string_equals(op, "COSINE")) {
		out_val = string("cos(%s)", val1);
	}
	else if (string_equals(op, "TANGENT")) {
		out_val = string("tan(%s)", val1);
	}
	else if (string_equals(op, "ARCSINE")) {
		out_val = string("asin(%s)", val1);
	}
	else if (string_equals(op, "ARCCOSINE")) {
		out_val = string("acos(%s)", val1);
	}
	else if (string_equals(op, "ARCTANGENT")) {
		out_val = string("atan(%s)", val1);
	}
	else if (string_equals(op, "ARCTAN2")) {
		out_val = string("atan2(%s, %s)", val1, val2);
	}
	else if (string_equals(op, "HYPERBOLIC_SINE")) {
		out_val = string("sinh(%s)", val1);
	}
	else if (string_equals(op, "HYPERBOLIC_COSINE")) {
		out_val = string("cosh(%s)", val1);
	}
	else if (string_equals(op, "HYPERBOLIC_TANGENT")) {
		out_val = string("tanh(%s)", val1);
	}
	else if (string_equals(op, "TO_RADIANS")) {
		out_val = string("radians(%s)", val1);
	}
	else if (string_equals(op, "TO_DEGREES")) {
		out_val = string("degrees(%s)", val1);
	}
	if (use_clamp) {
		return string("clamp(%s, 0.0, 1.0)", out_val);
	}
	else {
		return out_val;
	}
}

void math2_node_init() {

	char *math_operation_data =
	    string("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s",
	           _tr("Add"), _tr("Subtract"), _tr("Multiply"), _tr("Divide"), _tr("Power"), _tr("Logarithm"), _tr("Square Root"), _tr("Inverse Square Root"),
	           _tr("Absolute"), _tr("Exponent"), _tr("Minimum"), _tr("Maximum"), _tr("Less Than"), _tr("Greater Than"), _tr("Sign"), _tr("Round"), _tr("Floor"),
	           _tr("Ceil"), _tr("Truncate"), _tr("Fraction"), _tr("Truncated Modulo"), _tr("Floored Modulo"), _tr("Snap"), _tr("Ping-Pong"), _tr("Sine"),
	           _tr("Cosine"), _tr("Tangent"), _tr("Arcsine"), _tr("Arccosine"), _tr("Arctangent"), _tr("Arctan2"), _tr("Hyperbolic Sine"),
	           _tr("Hyperbolic Cosine"), _tr("Hyperbolic Tangent"), _tr("To Radians"), _tr("To Degrees"));
	ui_node_t *math2_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Math"),
	                              .type   = "MATH",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff62676d,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
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
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("operation"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(math_operation_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Clamp"),
	                                                                       .type          = "BOOL",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  2),
	                              .width = 0,
	                              .flags = 0});

	any_array_push(nodes_material_utilities, math2_node_def);
	any_map_set(parser_material_node_values, "MATH", math2_node_value);
}
