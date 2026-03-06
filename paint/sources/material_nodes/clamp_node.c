void clamp_node_init() {
	any_array_push(nodes_material_utilities, clamp_node_def);
	any_map_set(parser_material_node_values, "CLAMP", clamp_node_value);
}

string_t *clamp_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t         *val = parser_material_parse_value_input(node->inputs->buffer[0], false);
	string_t         *min = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t         *max = parser_material_parse_value_input(node->inputs->buffer[2], false);
	ui_node_button_t *but = node->buttons->buffer[0]; // operation;
	string_t         *op  = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	op                    = string_copy(string_replace_all(op, " ", "_"));
	if (string_equals(op, "MIN_MAX")) {
		return string_join(string_join(string_join(string_join(string_join(string_join("(clamp(", val), ", "), min), ", "), max), "))");
	}
	else { // RANGE
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("(clamp(", val), ", min("), min), ", "),
		                                                                max),
		                                                    "), max("),
		                                        min),
		                            ", "),
		                max),
		    ")))");
	}
}
