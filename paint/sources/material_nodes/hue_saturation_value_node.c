void hue_saturation_value_node_init() {
	any_array_push(nodes_material_color, hue_saturation_value_node_def);
	any_map_set(parser_material_node_vectors, "HUE_SAT", hue_saturation_value_node_vector);
}

string_t *hue_saturation_value_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_hue_sat);
	string_t *hue = parser_material_parse_value_input(node->inputs->buffer[0], false);
	string_t *sat = parser_material_parse_value_input(node->inputs->buffer[1], false);
	string_t *val = parser_material_parse_value_input(node->inputs->buffer[2], false);
	string_t *fac = parser_material_parse_value_input(node->inputs->buffer[3], false);
	string_t *col = parser_material_parse_vector_input(node->inputs->buffer[4]);
	return string_join(
	    string_join(
	        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("hue_sat(", col), ", float4("), hue), " - 0.5, "),
	                                                        sat),
	                                            ", "),
	                                val),
	                    ", 1.0 - "),
	        fac),
	    "))");
}
