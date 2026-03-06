void material_node_init() {
	any_array_push(nodes_material_input, material_node_def);
	any_map_set(parser_material_node_vectors, "MATERIAL", material_node_vector);
	any_map_set(parser_material_node_values, "MATERIAL", material_node_value);
}

string_t *material_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *result = "float3(0.0, 0.0, 0.0)";
	i32       mi     = node->buttons->buffer[0]->default_value->buffer[0];
	if (mi >= project_materials->length) {
		return result;
	}
	slot_material_t        *m      = project_materials->buffer[mi];
	ui_node_t_array_t      *_nodes = parser_material_nodes;
	ui_node_link_t_array_t *_links = parser_material_links;
	gc_unroot(parser_material_nodes);
	parser_material_nodes = m->canvas->nodes;
	gc_root(parser_material_nodes);
	gc_unroot(parser_material_links);
	parser_material_links = m->canvas->links;
	gc_root(parser_material_links);
	any_array_push(parser_material_parents, node);
	ui_node_t *output_node = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
	if (socket == node->outputs->buffer[0]) { // Base
		result = string_copy(parser_material_parse_vector_input(output_node->inputs->buffer[0]));
	}
	else if (socket == node->outputs->buffer[5]) { // Normal
		result = string_copy(parser_material_parse_vector_input(output_node->inputs->buffer[5]));
	}
	gc_unroot(parser_material_nodes);
	parser_material_nodes = _nodes;
	gc_root(parser_material_nodes);
	gc_unroot(parser_material_links);
	parser_material_links = _links;
	gc_root(parser_material_links);
	array_pop(parser_material_parents);
	return result;
}

string_t *material_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *result = "0.0";
	i32       mi     = node->buttons->buffer[0]->default_value->buffer[0];
	if (mi >= project_materials->length)
		return result;
	slot_material_t        *m      = project_materials->buffer[mi];
	ui_node_t_array_t      *_nodes = parser_material_nodes;
	ui_node_link_t_array_t *_links = parser_material_links;
	gc_unroot(parser_material_nodes);
	parser_material_nodes = m->canvas->nodes;
	gc_root(parser_material_nodes);
	gc_unroot(parser_material_links);
	parser_material_links = m->canvas->links;
	gc_root(parser_material_links);
	any_array_push(parser_material_parents, node);
	ui_node_t *output_node = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
	if (socket == node->outputs->buffer[1]) { // Opac
		result = string_copy(parser_material_parse_value_input(output_node->inputs->buffer[1], false));
	}
	else if (socket == node->outputs->buffer[2]) { // Occ
		result = string_copy(parser_material_parse_value_input(output_node->inputs->buffer[2], false));
	}
	else if (socket == node->outputs->buffer[3]) { // Rough
		result = string_copy(parser_material_parse_value_input(output_node->inputs->buffer[3], false));
	}
	else if (socket == node->outputs->buffer[4]) { // Metal
		result = string_copy(parser_material_parse_value_input(output_node->inputs->buffer[4], false));
	}
	else if (socket == node->outputs->buffer[7]) { // Height
		result = string_copy(parser_material_parse_value_input(output_node->inputs->buffer[7], false));
	}
	gc_unroot(parser_material_nodes);
	parser_material_nodes = _nodes;
	gc_root(parser_material_nodes);
	gc_unroot(parser_material_links);
	parser_material_links = _links;
	gc_root(parser_material_links);
	array_pop(parser_material_parents);
	return result;
}
