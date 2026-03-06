void bump_node_init() {
	any_array_push(nodes_material_utilities, bump_node_def);
	any_map_set(parser_material_node_vectors, "BUMP", bump_node_vector);
}

string_t *bump_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *strength = parser_material_parse_value_input(node->inputs->buffer[0], false);
	// string_t *distance = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *height          = parser_material_parse_value_input(node->inputs->buffer[2], false);
	string_t *nor             = parser_material_parse_vector_input(node->inputs->buffer[3]);
	string_t *sample_bump_res = string_join(parser_material_store_var_name(node), "_bump");
	parser_material_write(parser_material_kong,
	                      string_join(string_join(string_join(string_join(string_join(string_join("var ", sample_bump_res), "_x: float = ddx(float("), height),
	                                                          ")) * ("),
	                                              strength),
	                                  ") * 16.0;"));
	parser_material_write(parser_material_kong,
	                      string_join(string_join(string_join(string_join(string_join(string_join("var ", sample_bump_res), "_y: float = ddy(float("), height),
	                                                          ")) * ("),
	                                              strength),
	                                  ") * 16.0;"));
	return string_join(
	    string_join(string_join(string_join(string_join(string_join("(normalize(float3(", sample_bump_res), "_x, "), sample_bump_res), "_y, 1.0) + "), nor),
	    ") * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5))");
}
