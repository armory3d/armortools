void normal_map_node_init() {
	any_array_push(nodes_material_utilities, normal_map_node_def);
	any_map_set(parser_material_node_vectors, "NORMAL_MAP", normal_map_node_vector);
}

char *normal_map_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *strength = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *norm     = parser_material_parse_vector_input(node->inputs->buffer[1]);

	char *store    = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_texn: float3 = "), norm), " * 2.0 - 1.0;"));
	parser_material_write(parser_material_kong,
	                      string_join(string_join(string_join(string_join(string_join(string_join("", store), "_texn.xy = "), strength), " * "), store),
	                                  "_texn.xy;"));
	parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("", store), "_texn = normalize("), store), "_texn);"));

	return string_join(string_join("(0.5 * ", store), "_texn + 0.5)");
}
