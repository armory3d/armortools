void object_info_node_init() {
	any_array_push(nodes_material_input, object_info_node_def);
	any_map_set(parser_material_node_vectors, "OBJECT_INFO", object_info_node_vector);
	any_map_set(parser_material_node_values, "OBJECT_INFO", object_info_node_value);
}

string_t *object_info_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[0]) { // Location
		parser_material_kong->frag_wposition = true;
		return "input.wposition";
	}
	else { // Color
		return "float3(0.0, 0.0, 0.0)";
	}
}

string_t *object_info_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[2]) { // Alpha
		return "0.0";
	}
	else if (socket == node->outputs->buffer[3]) { // Object Index
		node_shader_add_constant(parser_material_kong, "object_info_index: float", "_object_info_index");
		return "constants.object_info_index";
	}
	else if (socket == node->outputs->buffer[4]) { // Material Index
		node_shader_add_constant(parser_material_kong, "object_info_material_index: float", "_object_info_material_index");
		return "constants.object_info_material_index";
	}
	else { // Random
		node_shader_add_constant(parser_material_kong, "object_info_random: float", "_object_info_random");
		return "constants.object_info_random";
	}
}
