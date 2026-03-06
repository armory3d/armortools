void brick_texture_node_init() {
	any_array_push(nodes_material_texture, brick_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_BRICK", brick_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_BRICK", brick_texture_node_value);
}

string_t *brick_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_brick);
	string_t *co    = parser_material_get_coord(node);
	string_t *col1  = parser_material_parse_vector_input(node->inputs->buffer[1]);
	string_t *col2  = parser_material_parse_vector_input(node->inputs->buffer[2]);
	string_t *col3  = parser_material_parse_vector_input(node->inputs->buffer[3]);
	string_t *scale = parser_material_parse_value_input(node->inputs->buffer[4], false);
	string_t *res   = string_join(
        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("tex_brick(", co), " * "), scale), ", "),
	                                                                  col1),
	                                                      ", "),
	                                          col2),
	                              ", "),
	                  col3),
        ")");
	return res;
}

string_t *brick_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_brick);
	string_t *co    = parser_material_get_coord(node);
	string_t *scale = parser_material_parse_value_input(node->inputs->buffer[4], false);
	string_t *res   = string_join(string_join(string_join(string_join("tex_brick_f(", co), " * "), scale), ")");
	return res;
}
