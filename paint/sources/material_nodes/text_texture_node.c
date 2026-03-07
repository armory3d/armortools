void text_texture_node_init() {
	any_array_push(nodes_material_texture, text_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_TEXT", text_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_TEXT", text_texture_node_value);
}

void _parser_material_cache_tex_text_node(char *file, char *text) {
	if (any_map_get(data_cached_images, file) == NULL) {
		sys_notify_on_next_frame(&_parser_material_cache_tex_text_node_205902, text);
	}
}

void _parser_material_cache_tex_text_node_205902(char *text) {
	char      *_text_tool_text  = context_raw->text_tool_text;
	gpu_texture_t *_text_tool_image = context_raw->text_tool_image;
	context_raw->text_tool_text     = string_copy(text);
	context_raw->text_tool_image    = NULL;
	util_render_make_text_preview();
	char *file = string_join("tex_text_", text);
	// TODO: remove old cache
	any_map_set(data_cached_images, file, context_raw->text_tool_image);
	context_raw->text_tool_text  = string_copy(_text_tool_text);
	context_raw->text_tool_image = _text_tool_image;
}

char *text_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *tex_name    = parser_material_node_name(node, NULL);
	buffer_t *text_buffer = node->buttons->buffer[0]->default_value;
	char *text        = sys_buffer_to_string(text_buffer);
	char *file        = string_join("tex_text_", text);
	_parser_material_cache_tex_text_node(file, text);
	bind_tex_t *tex      = parser_material_make_bind_tex(tex_name, file);
	char   *texstore = parser_material_texture_store(node, tex, tex_name, COLOR_SPACE_AUTO);
	return string_join(texstore, ".rrr");
}

char *text_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *tex_name    = parser_material_node_name(node, NULL);
	buffer_t *text_buffer = node->buttons->buffer[0]->default_value;
	char *text        = sys_buffer_to_string(text_buffer);
	char *file        = string_join("tex_text_", text);
	_parser_material_cache_tex_text_node(file, text);
	bind_tex_t *tex      = parser_material_make_bind_tex(tex_name, file);
	char   *texstore = parser_material_texture_store(node, tex, tex_name, COLOR_SPACE_AUTO);
	return string_join(texstore, ".r");
}
