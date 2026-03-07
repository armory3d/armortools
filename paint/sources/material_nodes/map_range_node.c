void map_range_node_init() {
	any_array_push(nodes_material_utilities, map_range_node_def);
	any_map_set(parser_material_node_values, "MAPRANGE", map_range_node_value);
}

char *map_range_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *val       = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *fmin      = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *fmax      = parser_material_parse_value_input(node->inputs->buffer[2], false);
	char *tmin      = parser_material_parse_value_input(node->inputs->buffer[3], false);
	char *tmax      = parser_material_parse_value_input(node->inputs->buffer[4], false);
	bool      use_clamp = node->buttons->buffer[0]->default_value->buffer[0] > 0;
	char *a         = string_join(
        string_join(string_join(string_join(string_join(string_join(string_join(string_join("((", tmin), " - "), tmax), ") / ("), fmin), " - "), fmax), "))");
	char *out_val = string_join(
	    string_join(
	        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("(", a), " * "), val), " + "), tmin), " - "), a),
	                    " * "),
	        fmin),
	    ")");
	if (use_clamp) {
		return string_join(string_join(string_join(string_join(string_join(string_join("(clamp(", out_val), ", "), tmin), ", "), tmax), "))");
	}
	else {
		return out_val;
	}
}
