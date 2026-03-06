void value_node_init() {
	any_array_push(nodes_material_input, value_node_def);
	any_map_set(parser_material_node_values, "VALUE", value_node_value);
}

string_t *value_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	return parser_material_vec1(node->outputs->buffer[0]->default_value->buffer[0]);
}
