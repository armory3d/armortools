void layer_weight_node_init() {
	any_array_push(nodes_material_input, layer_weight_node_def);
	any_map_set(parser_material_node_values, "LAYER_WEIGHT", layer_weight_node_value);
}

string_t *layer_weight_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *blend = parser_material_parse_value_input(node->inputs->buffer[0], false);
	if (socket == node->outputs->buffer[0]) { // Fresnel
		parser_material_kong->frag_dotnv = true;
		return string_join(string_join("clamp(pow(1.0 - dotnv, (1.0 - ", blend), ") * 10.0), 0.0, 1.0)");
	}
	else { // Facing
		parser_material_kong->frag_dotnv = true;
		return string_join(string_join("((1.0 - dotnv) * ", blend), ")");
	}
}
