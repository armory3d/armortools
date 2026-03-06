void replace_color_node_init() {
	any_array_push(nodes_material_color, replace_color_node_def);
	any_map_set(parser_material_node_vectors, "REPLACECOL", replace_color_node_vector);
}

string_t *replace_color_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *input_color = parser_material_parse_vector_input(node->inputs->buffer[0]);
	string_t *old_color   = parser_material_parse_vector_input(node->inputs->buffer[1]);
	string_t *new_color   = parser_material_parse_vector_input(node->inputs->buffer[2]);
	string_t *radius      = parser_material_parse_value_input(node->inputs->buffer[3], false);
	string_t *fuzziness   = parser_material_parse_value_input(node->inputs->buffer[4], false);
	return string_join(
	    string_join(
	        string_join(
	            string_join(
	                string_join(
	                    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", new_color), ", "),
	                                                                                                        input_color),
	                                                                                            ", clamp((distance("),
	                                                                                old_color),
	                                                                    ", "),
	                                                        input_color),
	                                            ") - "),
	                                radius),
	                    ") / max("),
	                fuzziness),
	            ", "),
	        f32_to_string(parser_material_eps)),
	    "), 0.0, 1.0))");
}
