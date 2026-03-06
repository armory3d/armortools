void camera_texture_node_init() {
	any_array_push(nodes_material_texture, camera_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_CAMERA", camera_texture_node_vector);
}

string_t *camera_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t *tex_name = string_join("texcamera_", parser_material_node_name(node, null));
	node_shader_add_texture(parser_material_kong, string_join("", tex_name), "_camera_texture");
	string_t *store = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_res: float3 = sample("), tex_name),
	                                                        ", sampler_linear, tex_coord).rgb;"));
	return string_join(store, "_res");
}
