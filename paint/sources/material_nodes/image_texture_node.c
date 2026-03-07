void image_texture_node_init() {
	any_array_push(nodes_material_texture, image_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_IMAGE", image_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_IMAGE", image_texture_node_value);
}

char *parser_material_texture_store(ui_node_t *node, bind_tex_t *tex, char *tex_name, i32 color_space) {
	any_array_push(parser_material_matcon->bind_textures, tex);
	node_shader_context_add_elem(parser_material_kong->context, "tex", "short2norm");
	node_shader_add_texture(parser_material_kong, string_join("", tex_name), NULL);
	char *uv_name = "";
	if (string_equals(node->type, "TEX_IMAGE") && parser_material_get_input_link(node->inputs->buffer[0]) != NULL) {
		uv_name = string_copy(parser_material_parse_vector_input(node->inputs->buffer[0]));
	}
	else {
		uv_name = string_copy(parser_material_tex_coord);
	}
	char *tex_store = parser_material_store_var_name(node);
	if (parser_material_sample_keep_aspect) {
		node_shader_add_constant(parser_material_kong, string_join(tex_name, "_size: float2"), string_join(string_join("_size(", tex_name), ")"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join("var ", tex_store), "_size: float2 = constants."), tex_name), "_size;"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join("var ", tex_store), "_ax: float = "), tex_store), "_size.x / "), tex_store),
		                "_size.y;"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join("var ", tex_store), "_ay: float = "), tex_store), "_size.y / "), tex_store),
		                "_size.x;"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("var ", tex_store),
		                                                                                                                "_uv: float2 = (("),
		                                                                                                    uv_name),
		                                                                                        ".xy / float("),
		                                                                            parser_material_sample_uv_scale),
		                                                                ") - float2(0.5, 0.5)) * float2(max("),
		                                                    tex_store),
		                                        "_ay, 1.0), max("),
		                            tex_store),
		                "_ax, 1.0))) + float2(0.5, 0.5);"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", tex_store), "_uv.x < 0.0 || "), tex_store),
		                                                                "_uv.y < 0.0 || "),
		                                                    tex_store),
		                                        "_uv.x > 1.0 || "),
		                            tex_store),
		                "_uv.y > 1.0) { discard; }"));
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join(string_join(tex_store, "_uv = "), tex_store), "_uv * float("),
		                                              parser_material_sample_uv_scale),
		                                  ");"));
		uv_name = string_join(tex_store, "_uv");
	}
	if (parser_material_triplanar) {
		parser_material_write(parser_material_kong, string_join(string_join("var ", tex_store), ": float4 = float4(0.0, 0.0, 0.0, 0.0);"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join("if (tex_coord_blend.x > 0.0) {", tex_store), " += sample("), tex_name),
		                                        ", sampler_linear, "),
		                            uv_name),
		                ".xy) * tex_coord_blend.x; }"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join("if (tex_coord_blend.y > 0.0) {", tex_store), " += sample("), tex_name),
		                                        ", sampler_linear, "),
		                            uv_name),
		                "1.xy) * tex_coord_blend.y; }"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join("if (tex_coord_blend.z > 0.0) {", tex_store), " += sample("), tex_name),
		                                        ", sampler_linear, "),
		                            uv_name),
		                "2.xy) * tex_coord_blend.z; }"));
	}
	else {
		if (parser_material_is_frag) {
			any_map_set(parser_material_texture_map, tex_store,
			            string_join(string_join(string_join(string_join("sample(", tex_name), ", sampler_linear, "), uv_name), ".xy)"));
			parser_material_write(parser_material_kong,
			                      string_join(string_join(string_join(string_join(string_join(string_join("var ", tex_store), ": float4 = sample("), tex_name),
			                                                          ", sampler_linear, "),
			                                              uv_name),
			                                  ".xy);"));
		}
		else {
			any_map_set(parser_material_texture_map, tex_store,
			            string_join(string_join(string_join(string_join("sample_lod(", tex_name), ", sampler_linear, "), uv_name), ".xy, 0.0)"));
			parser_material_write(parser_material_kong,
			                      string_join(string_join(string_join(string_join(string_join(string_join("var ", tex_store), ": float4 = sample_lod("),
			                                                                      tex_name),
			                                                          ", sampler_linear, "),
			                                              uv_name),
			                                  ".xy, 0.0);"));
		}
		if (!ends_with(tex->file, ".jpg")) { // Pre-mult alpha
			parser_material_write(parser_material_kong,
			                      string_join(string_join(string_join(string_join(string_join(tex_store, ".rgb = "), tex_store), ".rgb * "), tex_store),
			                                  ".a;"));
		}
	}
	if (parser_material_transform_color_space) {
		// Base color socket auto-converts from sRGB to linear
		if (color_space == COLOR_SPACE_LINEAR && parser_material_parsing_basecolor) { // Linear to sRGB
			parser_material_write(parser_material_kong,
			                      string_join(string_join(string_join(tex_store, ".rgb = pow3("), tex_store), ".rgb, float3(2.2, 2.2, 2.2));"));
		}
		else if (color_space == COLOR_SPACE_SRGB && !parser_material_parsing_basecolor) { // sRGB to linear
			parser_material_write(parser_material_kong, string_join(string_join(string_join(tex_store, ".rgb = pow3("), tex_store),
			                                                        ".rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));"));
		}
		else if (color_space == COLOR_SPACE_DIRECTX_NORMAL_MAP) { // DirectX normal map to OpenGL normal map
			parser_material_write(parser_material_kong, string_join(string_join(string_join(tex_store, ".y = 1.0 - "), tex_store), ".y;"));
		}
	}
	return tex_store;
}

bind_tex_t *parser_material_make_texture(ui_node_t *image_node, char *tex_name) {
	i32 i = image_node->buttons->buffer[0]->default_value->buffer[0];
	if (i > 9000) { // 9999 - Texture deleted, use pink now
		return NULL;
	}
	char *filepath = parser_material_enum_data(base_enum_texts(image_node->type)->buffer[i]);
	if (string_equals(filepath, "") || string_index_of(filepath, ".") == -1) {
		return NULL;
	}
	return parser_material_make_bind_tex(tex_name, filepath);
}

char *image_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	// Already fetched
	if (char_ptr_array_index_of(parser_material_parsed, parser_material_res_var_name(node, node->outputs->buffer[1])) >= 0) { // TODO: node.outputs[0]
		char *varname = parser_material_store_var_name(node);
		return string_join(varname, ".rgb");
	}
	char   *tex_name = parser_material_node_name(node, NULL);
	bind_tex_t *tex      = parser_material_make_texture(node, tex_name);
	if (tex != NULL) {
		i32       color_space = node->buttons->buffer[1]->default_value->buffer[0];
		char *texstore    = parser_material_texture_store(node, tex, tex_name, color_space);
		return string_join(texstore, ".rgb");
	}
	else {
		char *tex_store = parser_material_store_var_name(node); // Pink color for missing texture
		parser_material_write(parser_material_kong, string_join(string_join("var ", tex_store), ": float4 = float4(1.0, 0.0, 1.0, 1.0);"));
		return string_join(tex_store, ".rgb");
	}
}

char *image_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	// Already fetched
	if (char_ptr_array_index_of(parser_material_parsed, parser_material_res_var_name(node, node->outputs->buffer[0])) >= 0) { // TODO: node.outputs[1]
		char *varname = parser_material_store_var_name(node);
		return string_join(varname, ".a");
	}
	char   *tex_name = parser_material_node_name(node, NULL);
	bind_tex_t *tex      = parser_material_make_texture(node, tex_name);
	if (tex != NULL) {
		i32       color_space = node->buttons->buffer[1]->default_value->buffer[0];
		char *texstore    = parser_material_texture_store(node, tex, tex_name, color_space);
		return string_join(texstore, ".a");
	}
	return "0.0";
}
