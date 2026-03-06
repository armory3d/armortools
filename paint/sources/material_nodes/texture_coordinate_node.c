void texture_coordinate_node_init() {
	any_array_push(nodes_material_input, texture_coordinate_node_def);
	any_map_set(parser_material_node_vectors, "TEX_COORD", texture_coordinate_node_vector);
}

string_t *texture_coordinate_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[0]) { // Generated - bounds
		parser_material_kong->frag_bposition = true;
		return "bposition";
	}
	else if (socket == node->outputs->buffer[1]) { // Normal
		parser_material_kong->frag_n = true;
		return "n";
	}
	else if (socket == node->outputs->buffer[2]) { // UV
		node_shader_context_add_elem(parser_material_kong->context, "tex", "short2norm");
		return "float3(tex_coord.x, tex_coord.y, 0.0)";
	}
	else if (socket == node->outputs->buffer[3]) { // Object
		parser_material_kong->frag_mposition = true;
		return "input.mposition";
	}
	else if (socket == node->outputs->buffer[4]) { // Camera
		parser_material_kong->frag_vposition = true;
		return "input.vposition";
	}
	else if (socket == node->outputs->buffer[5]) { // Window
		parser_material_kong->frag_wvpposition = true;
		return "input.wvpposition.xyz";
	}
	else { // Reflection
		return "float3(0.0, 0.0, 0.0)";
	}
}
