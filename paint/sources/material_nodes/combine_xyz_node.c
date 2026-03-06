void combine_xyz_node_init() {
	any_array_push(nodes_material_utilities, combine_xyz_node_def);
	any_map_set(parser_material_node_vectors, "COMBXYZ", combine_xyz_node_vector);
}

string_t *combine_xyz_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *x = parser_material_parse_value_input(node->inputs->buffer[0], false);
	string_t *y = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *z = parser_material_parse_value_input(node->inputs->buffer[2], false);
	return string_join(string_join(string_join(string_join(string_join(string_join("float3(", x), ", "), y), ", "), z), ")");
}
