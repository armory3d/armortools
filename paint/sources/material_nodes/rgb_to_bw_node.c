void rgb_to_bw_node_init() {
	any_array_push(nodes_material_utilities, rgb_to_bw_node_def);
	any_map_set(parser_material_node_values, "RGBTOBW", rgb_to_bw_node_value);
}

char *rgb_to_bw_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *col = parser_material_parse_vector_input(node->inputs->buffer[0]);
	return string_join(string_join(string_join(string_join(string_join(string_join("(((", col), ".r * 0.3 + "), col), ".g * 0.59 + "), col),
	                   ".b * 0.11) / 3.0) * 2.5)");
}
