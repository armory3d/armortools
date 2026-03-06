void color_mask_node_init() {
	any_array_push(nodes_material_utilities, color_mask_node_def);
	any_map_set(parser_material_node_values, "COLMASK", color_mask_node_value);
}

string_t *color_mask_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *input_color = parser_material_parse_vector_input(node->inputs->buffer[0]);
	string_t *mask_color  = parser_material_parse_vector_input(node->inputs->buffer[1]);
	string_t *radius      = parser_material_parse_value_input(node->inputs->buffer[2], false);
	string_t *fuzziness   = parser_material_parse_value_input(node->inputs->buffer[3], false);
	return string_join(
	    string_join(
	        string_join(
	            string_join(string_join(string_join(string_join(string_join(string_join(string_join("clamp(1.0 - (distance(", input_color), ", "), mask_color),
	                                                            ") - "),
	                                                radius),
	                                    ") / max("),
	                        fuzziness),
	            ", "),
	        f32_to_string(parser_material_eps)),
	    "), 0.0, 1.0)");
}
