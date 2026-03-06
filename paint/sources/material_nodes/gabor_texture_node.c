void gabor_texture_node_init() {
	any_array_push(nodes_material_texture, gabor_texture_node_def);
	any_map_set(parser_material_node_values, "TEX_GABOR", gabor_texture_node_value);
}

string_t *gabor_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_gabor);
	string_t *co          = parser_material_get_coord(node);
	string_t *scale       = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *frequency   = parser_material_parse_value_input(node->inputs->buffer[2], false);
	string_t *anisotropy  = parser_material_parse_value_input(node->inputs->buffer[3], false);
	string_t *orientation = parser_material_parse_vector_input(node->inputs->buffer[4]);
	string_t *res         = string_join(
        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("tex_gabor(", co), ", "), scale), ", "),
	                                                                        frequency),
	                                                            ", "),
	                                                anisotropy),
	                                    ", "),
	                        orientation),
        ")");
	if (socket == node->outputs->buffer[0]) { // Value
		return string_join(res, ".x");
	}
	else if (socket == node->outputs->buffer[1]) { // Phase
		return string_join(res, ".y");
	}
	else { // Intensity
		return string_join(res, ".z");
	}
}
