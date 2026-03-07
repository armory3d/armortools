void math2_node_init() {
	any_array_push(nodes_material_utilities, math2_node_def);
	any_map_set(parser_material_node_values, "MATH", math2_node_value);
}

char *math2_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char         *val1 = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char         *val2 = parser_material_parse_value_input(node->inputs->buffer[1], false);
	ui_node_button_t *but  = node->buttons->buffer[0]; // operation
	char         *op   = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                     = string_copy(string_replace_all(op, " ", "_"));
	bool      use_clamp    = node->buttons->buffer[1]->default_value->buffer[0] > 0;
	char *out_val      = "";
	if (string_equals(op, "ADD")) {
		out_val = string_join(string_join(string_join(string_join("(", val1), " + "), val2), ")");
	}
	else if (string_equals(op, "SUBTRACT")) {
		out_val = string_join(string_join(string_join(string_join("(", val1), " - "), val2), ")");
	}
	else if (string_equals(op, "MULTIPLY")) {
		out_val = string_join(string_join(string_join(string_join("(", val1), " * "), val2), ")");
	}
	else if (string_equals(op, "DIVIDE")) {
		char *store = string_join(parser_material_store_var_name(node), "_divide");
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), ": float = "), val2), ";"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join(string_join(string_join("if (", store), " == 0.0) { "), store), " = "),
		                                              f32_to_string(parser_material_eps)),
		                                  "; }"));
		out_val = string_join(string_join(string_join(string_join("(", val1), " / "), store), ")");
	}
	else if (string_equals(op, "POWER")) {
		out_val = string_join(string_join(string_join(string_join("pow(", val1), ", "), val2), ")");
	}
	else if (string_equals(op, "LOGARITHM")) {
		out_val = string_join(string_join("log(", val1), ")");
	}
	else if (string_equals(op, "SQUARE_ROOT")) {
		out_val = string_join(string_join("sqrt(", val1), ")");
	}
	else if (string_equals(op, "INVERSE_SQUARE_ROOT")) {
		out_val = string_join(string_join("rsqrt(", val1), ")");
	}
	else if (string_equals(op, "EXPONENT")) {
		out_val = string_join(string_join("exp(", val1), ")");
	}
	else if (string_equals(op, "ABSOLUTE")) {
		out_val = string_join(string_join("abs(", val1), ")");
	}
	else if (string_equals(op, "MINIMUM")) {
		out_val = string_join(string_join(string_join(string_join("min(", val1), ", "), val2), ")");
	}
	else if (string_equals(op, "MAXIMUM")) {
		out_val = string_join(string_join(string_join(string_join("max(", val1), ", "), val2), ")");
	}
	else if (string_equals(op, "LESS_THAN")) {
		// out_val = "float(" + val1 + " < " + val2 + ")";
		char *store = string_join(parser_material_store_var_name(node), "_lessthan");
		parser_material_write(parser_material_kong, string_join(string_join("var ", store), ": float = 0.0;"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join(string_join(string_join("if (", val1), " < "), val2), ") { "), store),
		                                  " = 1.0; }"));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "GREATER_THAN")) {
		// out_val = "float(" + val1 + " > " + val2 + ")";
		char *store = string_join(parser_material_store_var_name(node), "_greaterthan");
		parser_material_write(parser_material_kong, string_join(string_join("var ", store), ": float = 0.0;"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join(string_join(string_join("if (", val1), " > "), val2), ") { "), store),
		                                  " = 1.0; }"));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "SIGN")) {
		out_val = string_join(string_join("sign(", val1), ")");
	}
	else if (string_equals(op, "ROUND")) {
		out_val = string_join(string_join("floor(", val1), " + 0.5)");
	}
	else if (string_equals(op, "FLOOR")) {
		out_val = string_join(string_join("floor(", val1), ")");
	}
	else if (string_equals(op, "CEIL")) {
		out_val = string_join(string_join("ceil(", val1), ")");
	}
	else if (string_equals(op, "SNAP")) {
		out_val = string_join(string_join(string_join(string_join(string_join(string_join("(floor(", val1), " / "), val2), ") * "), val2), ")");
	}
	else if (string_equals(op, "TRUNCATE")) {
		out_val = string_join(string_join("trunc(", val1), ")");
	}
	else if (string_equals(op, "FRACTION")) {
		out_val = string_join(string_join("frac(", val1), ")");
	}
	else if (string_equals(op, "MODULO")) {
		out_val = string_join(string_join(string_join(string_join("(", val1), " % "), val2), ")");
	}
	else if (string_equals(op, "PING-PONG")) {
		char *store = string_join(parser_material_store_var_name(node), "_pingpong");
		parser_material_write(parser_material_kong, string_join(string_join("var ", store), ": float = 0.0;"));
		parser_material_write(
		    parser_material_kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", val2),
		                                                                                                                        " != 0.0) { "),
		                                                                                                            store),
		                                                                                                " = abs(frac(("),
		                                                                                    val1),
		                                                                        " - "),
		                                                            val2),
		                                                ") / ("),
		                                    val2),
		                        " * 2.0)) * "),
		                    val2),
		                " * 2.0 - "),
		            val2),
		        "); }"));
		out_val = string_copy(store);
	}
	else if (string_equals(op, "SINE")) {
		out_val = string_join(string_join("sin(", val1), ")");
	}
	else if (string_equals(op, "COSINE")) {
		out_val = string_join(string_join("cos(", val1), ")");
	}
	else if (string_equals(op, "TANGENT")) {
		out_val = string_join(string_join("tan(", val1), ")");
	}
	else if (string_equals(op, "ARCSINE")) {
		out_val = string_join(string_join("asin(", val1), ")");
	}
	else if (string_equals(op, "ARCCOSINE")) {
		out_val = string_join(string_join("acos(", val1), ")");
	}
	else if (string_equals(op, "ARCTANGENT")) {
		out_val = string_join(string_join("atan(", val1), ")");
	}
	else if (string_equals(op, "ARCTAN2")) {
		out_val = string_join(string_join(string_join(string_join("atan2(", val1), ", "), val2), ")");
	}
	else if (string_equals(op, "HYPERBOLIC_SINE")) {
		out_val = string_join(string_join("sinh(", val1), ")");
	}
	else if (string_equals(op, "HYPERBOLIC_COSINE")) {
		out_val = string_join(string_join("cosh(", val1), ")");
	}
	else if (string_equals(op, "HYPERBOLIC_TANGENT")) {
		out_val = string_join(string_join("tanh(", val1), ")");
	}
	else if (string_equals(op, "TO_RADIANS")) {
		out_val = string_join(string_join("radians(", val1), ")");
	}
	else if (string_equals(op, "TO_DEGREES")) {
		out_val = string_join(string_join("degrees(", val1), ")");
	}
	if (use_clamp) {
		return string_join(string_join("clamp(", out_val), ", 0.0, 1.0)");
	}
	else {
		return out_val;
	}
}
