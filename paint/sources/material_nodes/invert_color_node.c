void invert_color_node_init() {
	any_array_push(nodes_material_color, invert_color_node_def);
	any_map_set(parser_material_node_vectors, "INVERT_COLOR", invert_color_node_vector);
}

string_t *invert_color_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *fac     = parser_material_parse_value_input(node->inputs->buffer[0], false);
	string_t *out_col = parser_material_parse_vector_input(node->inputs->buffer[1]);
	return string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", out_col), ", float3(1.0, 1.0, 1.0) - ("), out_col), "), "), fac),
	                   ")");
}
