void script_node_init() {
	any_array_push(nodes_material_input, script_node_def);
	any_map_set(parser_material_node_values, "SCRIPT_CPU", script_node_value);
}

string_t *script_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (parser_material_script_links == null) {
		gc_unroot(parser_material_script_links);
		parser_material_script_links = any_map_create();
		gc_root(parser_material_script_links);
	}
	buffer_t *script = node->buttons->buffer[0]->default_value;
	string_t *str    = sys_buffer_to_string(script);
	string_t *link   = parser_material_node_name(node, null);
	any_map_set(parser_material_script_links, link, str);
	node_shader_add_constant(parser_material_kong, string_join(string_join("", link), ": float"), string_join("_", link));
	return string_join("constants.", link);
}
