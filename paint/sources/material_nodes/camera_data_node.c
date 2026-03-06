void camera_data_node_init() {
	any_array_push(nodes_material_input, camera_data_node_def);
	any_map_set(parser_material_node_vectors, "CAMERA", camera_data_node_vector);
	any_map_set(parser_material_node_values, "CAMERA", camera_data_node_value);
}

string_t *camera_data_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	parser_material_kong->frag_vvec_cam = true;
	return "vvec_cam";
}

string_t *camera_data_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[1]) { // View Z Depth
		node_shader_add_constant(parser_material_kong, "camera_proj: float2", "_camera_plane_proj");
		parser_material_kong->frag_wvpposition = true;
		return "(constants.camera_proj.y / ((input.wvpposition.z / input.wvpposition.w) - constants.camera_proj.x))";
	}
	else { // View Distance
		node_shader_add_constant(parser_material_kong, "eye: float3", "_camera_pos");
		parser_material_kong->frag_wposition = true;
		return "distance(constants.eye, input.wposition)";
	}
}
