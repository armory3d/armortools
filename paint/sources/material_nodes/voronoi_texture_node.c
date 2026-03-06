void voronoi_texture_node_init() {
	any_array_push(nodes_material_texture, voronoi_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_VORONOI", voronoi_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_VORONOI", voronoi_texture_node_value);
}

string_t *voronoi_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_voronoi);
	node_shader_add_texture(parser_material_kong, "snoise256", "$noise256.k");
	string_t         *co       = parser_material_get_coord(node);
	string_t         *scale    = parser_material_parse_value_input(node->inputs->buffer[1], false);
	ui_node_button_t *but      = node->buttons->buffer[0]; // coloring;
	string_t         *coloring = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	coloring                   = string_copy(string_replace_all(coloring, " ", "_"));
	string_t *res              = "";
	if (string_equals(coloring, "INTENSITY")) {
		string_t *voronoi = string_join(string_join(string_join(string_join("tex_voronoi(", co), " * "), scale), ").a");
		res               = string_copy(parser_material_to_vec3(voronoi));
	}
	else { // Cells
		res = string_join(string_join(string_join(string_join("tex_voronoi(", co), " * "), scale), ").rgb");
	}
	return res;
}

string_t *voronoi_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_voronoi);
	node_shader_add_texture(parser_material_kong, "snoise256", "$noise256.k");
	string_t         *co       = parser_material_get_coord(node);
	string_t         *scale    = parser_material_parse_value_input(node->inputs->buffer[1], false);
	ui_node_button_t *but      = node->buttons->buffer[0]; // coloring
	string_t         *coloring = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	coloring                   = string_copy(string_replace_all(coloring, " ", "_"));
	string_t *res              = "";
	if (string_equals(coloring, "INTENSITY")) {
		res = string_join(string_join(string_join(string_join("tex_voronoi(", co), " * "), scale), ").a");
	}
	else { // Cells
		res = string_join(string_join(string_join(string_join("tex_voronoi(", co), " * "), scale), ").r");
	}
	return res;
}
