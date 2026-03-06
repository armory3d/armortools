void vector_math2_node_init() {
	any_array_push(nodes_material_utilities, vector_math2_node_def);
	any_map_set(parser_material_node_vectors, "VECT_MATH", vector_math2_node_vector);
	any_map_set(parser_material_node_values, "VECT_MATH", vector_math2_node_value);
}

string_t *vector_math2_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t         *vec1 = parser_material_parse_vector_input(node->inputs->buffer[0]);
	string_t         *vec2 = parser_material_parse_vector_input(node->inputs->buffer[1]);
	ui_node_button_t *but  = node->buttons->buffer[0]; // operation;
	string_t         *op   = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                     = string_copy(string_replace_all(op, " ", "_"));
	if (string_equals(op, "ADD")) {
		return string_join(string_join(string_join(string_join("(", vec1), " + "), vec2), ")");
	}
	else if (string_equals(op, "SUBTRACT")) {
		return string_join(string_join(string_join(string_join("(", vec1), " - "), vec2), ")");
	}
	else if (string_equals(op, "AVERAGE")) {
		return string_join(string_join(string_join(string_join("((", vec1), " + "), vec2), ") / 2.0)");
	}
	else if (string_equals(op, "DOT_PRODUCT")) {
		return parser_material_to_vec3(string_join(string_join(string_join(string_join("dot(", vec1), ", "), vec2), ")"));
	}
	else if (string_equals(op, "LENGTH")) {
		return parser_material_to_vec3(string_join(string_join("length(", vec1), ")"));
	}
	else if (string_equals(op, "DISTANCE")) {
		return parser_material_to_vec3(string_join(string_join(string_join(string_join("distance(", vec1), ", "), vec2), ")"));
	}
	else if (string_equals(op, "CROSS_PRODUCT")) {
		return string_join(string_join(string_join(string_join("cross(", vec1), ", "), vec2), ")");
	}
	else if (string_equals(op, "NORMALIZE")) {
		return string_join(string_join("normalize(", vec1), ")");
	}
	else if (string_equals(op, "MULTIPLY")) {
		return string_join(string_join(string_join(string_join("(", vec1), " * "), vec2), ")");
	}
	else if (string_equals(op, "DIVIDE")) {
		string_t *store = string_join(parser_material_store_var_name(node), "_vec2");
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), ": float3 = "), vec2), ";"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join("if (", store), ".x == 0.0) { "), store), ".x = 0.000001; }"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join("if (", store), ".y == 0.0) { "), store), ".y = 0.000001; }"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join("if (", store), ".z == 0.0) { "), store), ".z = 0.000001; }"));
		return string_join(string_join(string_join(string_join("(", vec1), " / "), vec2), ")");
	}
	else if (string_equals(op, "PROJECT")) {
		return string_join(
		    string_join(
		        string_join(
		            string_join(string_join(string_join(string_join(string_join(string_join(string_join("(dot(", vec1), ", "), vec2), ") / dot("), vec2), ", "),
		                        vec2),
		            ") * "),
		        vec2),
		    ")");
	}
	else if (string_equals(op, "REFLECT")) {
		return string_join(string_join(string_join(string_join("reflect(", vec1), ", normalize("), vec2), "))");
	}
	else if (string_equals(op, "SCALE")) {
		return string_join(string_join(string_join(string_join("(", vec2), ".x * "), vec1), ")");
	}
	else if (string_equals(op, "ABSOLUTE")) {
		return string_join(string_join("abs3(", vec1), ")");
	}
	else if (string_equals(op, "MINIMUM")) {
		return string_join(string_join(string_join(string_join("min3(", vec1), ", "), vec2), ")");
	}
	else if (string_equals(op, "MAXIMUM")) {
		return string_join(string_join(string_join(string_join("max3(", vec1), ", "), vec2), ")");
	}
	else if (string_equals(op, "FLOOR")) {
		return string_join(string_join("floor3(", vec1), ")");
	}
	else if (string_equals(op, "CEIL")) {
		return string_join(string_join("ceil3(", vec1), ")");
	}
	else if (string_equals(op, "FRACTION")) {
		return string_join(string_join("frac3(", vec1), ")");
	}
	else if (string_equals(op, "MODULO")) {
		return string_join(string_join(string_join(string_join("(", vec1), " % "), vec2), ")");
	}
	else if (string_equals(op, "SNAP")) {
		return string_join(string_join(string_join(string_join(string_join(string_join("(floor3(", vec1), " / "), vec2), ") * "), vec2), ")");
	}
	else if (string_equals(op, "SINE")) {
		return string_join(string_join(string_join(string_join(string_join(string_join("float3(sin(", vec1), ".x), sin("), vec1), ".y), sin("), vec1), ".z))");
	}
	else if (string_equals(op, "COSINE")) {
		return string_join(string_join(string_join(string_join(string_join(string_join("float3(cos(", vec1), ".x), cos("), vec1), ".y), cos("), vec1), ".z))");
	}
	else { // TANGENT
		return string_join(string_join(string_join(string_join(string_join(string_join("float3(tan(", vec1), ".x), tan("), vec1), ".y), tan("), vec1), ".z))");
	}
}

string_t *vector_math2_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t         *vec1 = parser_material_parse_vector_input(node->inputs->buffer[0]);
	string_t         *vec2 = parser_material_parse_vector_input(node->inputs->buffer[1]);
	ui_node_button_t *but  = node->buttons->buffer[0]; // operation;
	string_t         *op   = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                     = string_copy(string_replace_all(op, " ", "_"));
	if (string_equals(op, "DOT_PRODUCT")) {
		return string_join(string_join(string_join(string_join("dot(", vec1), ", "), vec2), ")");
	}
	else if (string_equals(op, "LENGTH")) {
		return string_join(string_join("length(", vec1), ")");
	}
	else if (string_equals(op, "DISTANCE")) {
		return string_join(string_join(string_join(string_join("distance(", vec1), ", "), vec2), ")");
	}
	else {
		return "0.0";
	}
}
