void layer_node_init() {
	any_array_push(nodes_material_input, layer_node_def);
	any_map_set(parser_material_node_vectors, "LAYER", layer_node_vector);
	any_map_set(parser_material_node_values, "LAYER", layer_node_value);
}

char *layer_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	i32 l = node->buttons->buffer[0]->default_value->buffer[0];
	if (socket == node->outputs->buffer[0]) { // Base
		node_shader_add_texture(parser_material_kong, string_join("texpaint", i32_to_string(l)), string_join("_texpaint", i32_to_string(l)));
		return string_join(string_join("sample(texpaint", i32_to_string(l)), ", sampler_linear, tex_coord).rgb");
	}
	else { // Normal
		node_shader_add_texture(parser_material_kong, string_join("texpaint_nor", i32_to_string(l)), string_join("_texpaint_nor", i32_to_string(l)));
		return string_join(string_join("sample(texpaint_nor", i32_to_string(l)), ", sampler_linear, tex_coord).rgb");
	}
}

char *layer_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	i32 l = node->buttons->buffer[0]->default_value->buffer[0];
	if (socket == node->outputs->buffer[1]) { // Opac
		node_shader_add_texture(parser_material_kong, string_join("texpaint", i32_to_string(l)), string_join("_texpaint", i32_to_string(l)));
		return string_join(string_join("sample(texpaint", i32_to_string(l)), ", sampler_linear, tex_coord).a");
	}
	else if (socket == node->outputs->buffer[2]) { // Occ
		node_shader_add_texture(parser_material_kong, string_join("texpaint_pack", i32_to_string(l)), string_join("_texpaint_pack", i32_to_string(l)));
		return string_join(string_join("sample(texpaint_pack", i32_to_string(l)), ", sampler_linear, tex_coord).r");
	}
	else if (socket == node->outputs->buffer[3]) { // Rough
		node_shader_add_texture(parser_material_kong, string_join("texpaint_pack", i32_to_string(l)), string_join("_texpaint_pack", i32_to_string(l)));
		return string_join(string_join("sample(texpaint_pack", i32_to_string(l)), ", sampler_linear, tex_coord).g");
	}
	else if (socket == node->outputs->buffer[4]) { // Metal
		node_shader_add_texture(parser_material_kong, string_join("texpaint_pack", i32_to_string(l)), string_join("_texpaint_pack", i32_to_string(l)));
		return string_join(string_join("sample(texpaint_pack", i32_to_string(l)), ", sampler_linear, tex_coord).b");
	}
	else if (socket == node->outputs->buffer[6]) {
		return "0.0";
	} // Emission
	else if (socket == node->outputs->buffer[7]) { // Height
		node_shader_add_texture(parser_material_kong, string_join("texpaint_pack", i32_to_string(l)), string_join("_texpaint_pack", i32_to_string(l)));
		return string_join(string_join("sample(texpaint_pack", i32_to_string(l)), ", sampler_linear, tex_coord).a");
	}
	else {
		return "0.0";
	} // Subsurface
}
