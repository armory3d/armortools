void wireframe_node_init() {
	any_array_push(nodes_material_input, wireframe_node_def);
	any_map_set(parser_material_node_values, "WIREFRAME", wireframe_node_value);
}

char *wireframe_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_texture(parser_material_kong, "texuvmap", "_texuvmap");
	// let use_pixel_size: bool = node.buttons[0].default_value[0] > 0.0;
	// let pixel_size: f32 = parse_value_input(node.inputs[0]);
	return "sample_lod(texuvmap, sampler_linear, tex_coord, 0.0).r";
}
