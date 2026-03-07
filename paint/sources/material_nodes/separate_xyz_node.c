void separate_xyz_node_init() {
	any_array_push(nodes_material_utilities, separate_xyz_node_def);
	any_map_set(parser_material_node_values, "SEPXYZ", separate_xyz_node_value);
}

char *separate_xyz_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *vec = parser_material_parse_vector_input(node->inputs->buffer[0]);
	if (socket == node->outputs->buffer[0]) {
		return string_join(vec, ".x");
	}
	else if (socket == node->outputs->buffer[1]) {
		return string_join(vec, ".y");
	}
	else {
		return string_join(vec, ".z");
	}
}
