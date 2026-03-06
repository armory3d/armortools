void map_range_node_init() {
	any_array_push(nodes_material_utilities, map_range_node_def);
	any_map_set(parser_material_node_values, "MAPRANGE", map_range_node_value);
}

string_t *map_range_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *val       = parser_material_parse_value_input(node->inputs->buffer[0], false);
	string_t *fmin      = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *fmax      = parser_material_parse_value_input(node->inputs->buffer[2], false);
	string_t *tmin      = parser_material_parse_value_input(node->inputs->buffer[3], false);
	string_t *tmax      = parser_material_parse_value_input(node->inputs->buffer[4], false);
	bool      use_clamp = node->buttons->buffer[0]->default_value->buffer[0] > 0;
	string_t *a         = string_join(
        string_join(string_join(string_join(string_join(string_join(string_join(string_join("((", tmin), " - "), tmax), ") / ("), fmin), " - "), fmax), "))");
	string_t *out_val = string_join(
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
