void quantize_node_init() {
	any_array_push(nodes_material_color, quantize_node_def);
	any_map_set(parser_material_node_vectors, "QUANTIZE", quantize_node_vector);
}

char *quantize_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *strength = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *col      = parser_material_parse_vector_input(node->inputs->buffer[1]);
	return string_join(string_join(string_join(string_join(string_join(string_join("(floor3(100.0 * ", strength), " * "), col), ") / (100.0 * "), strength),
	                   "))");
}
