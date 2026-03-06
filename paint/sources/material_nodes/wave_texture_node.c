void wave_texture_node_init() {
	any_array_push(nodes_material_texture, wave_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_WAVE", wave_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_WAVE", wave_texture_node_value);
}

string_t *wave_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_wave);
	string_t *co    = parser_material_get_coord(node);
	string_t *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *res   = parser_material_to_vec3(string_join(string_join(string_join(string_join("tex_wave_f(", co), " * "), scale), ")"));
	return res;
}

string_t *wave_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_wave);
	string_t *co    = parser_material_get_coord(node);
	string_t *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *res   = string_join(string_join(string_join(string_join("tex_wave_f(", co), " * "), scale), ")");
	return res;
}
