void rgb_node_init() {
	any_array_push(nodes_material_input, rgb_node_def);
	any_map_set(parser_material_node_vectors, "RGB", rgb_node_vector);
}

string_t *rgb_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	return parser_material_vec3(socket->default_value);
}
