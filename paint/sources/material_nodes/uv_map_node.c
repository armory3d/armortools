void uv_map_node_init() {
	any_array_push(nodes_material_input, uv_map_node_def);
	any_map_set(parser_material_node_vectors, "UVMAP", uv_map_node_vector);
}

string_t *uv_map_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_context_add_elem(parser_material_kong->context, "tex", "short2norm");
	i32 uv_map = node->buttons->buffer[0]->default_value->buffer[0];
	if (uv_map == 1 && mesh_data_get_vertex_array(context_raw->paint_object->data, "tex1") != null) {
		node_shader_context_add_elem(parser_material_kong->context, "tex1", "short2norm");
		node_shader_add_out(parser_material_kong, "tex_coord1: float2");
		node_shader_write_vert(parser_material_kong, "output.tex_coord1 = input.tex1;");
		return "float3(input.tex_coord1.x, input.tex_coord1.y, 0.0)";
	}
	else {
		return "float3(tex_coord.x, tex_coord.y, 0.0)";
	}
}
