void fresnel_node_init() {
	any_array_push(nodes_material_input, fresnel_node_def);
	any_map_set(parser_material_node_values, "FRESNEL", fresnel_node_value);
}

char *fresnel_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *ior                    = parser_material_parse_value_input(node->inputs->buffer[0], false);
	parser_material_kong->frag_dotnv = true;
	return string_join(string_join("pow(1.0 - dotnv, 7.25 / ", ior), ")");
}
