void geometry_node_init() {
	any_array_push(nodes_material_input, geometry_node_def);
	any_map_set(parser_material_node_vectors, "NEW_GEOMETRY", geometry_node_vector);
	any_map_set(parser_material_node_values, "NEW_GEOMETRY", geometry_node_value);
}

string_t *geometry_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[0]) { // Position
		parser_material_kong->frag_wposition = true;
		return "input.wposition";
	}
	else if (socket == node->outputs->buffer[1]) { // Normal
		parser_material_kong->frag_n = true;
		return "n";
	}
	else if (socket == node->outputs->buffer[2]) { // Tangent
		parser_material_kong->frag_wtangent = true;
		return "input.wtangent";
	}
	else if (socket == node->outputs->buffer[3]) { // True Normal
		parser_material_kong->frag_n = true;
		return "n";
	}
	else if (socket == node->outputs->buffer[4]) { // Incoming
		parser_material_kong->frag_vvec = true;
		return "vvec";
	}
	else { // Parametric
		parser_material_kong->frag_mposition = true;
		return "input.mposition";
	}
}

string_t *geometry_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[6]) { // Backfacing
		return "0.0";                         // SV_IsFrontFace
		                                      // return "(1.0 - float(gl_FrontFacing))";
	}
	else if (socket == node->outputs->buffer[7]) { // Pointiness
		f32       strength           = 1.0;
		f32       radius             = 1.0;
		f32       offset             = 0.0;
		string_t *store              = parser_material_store_var_name(node);
		parser_material_kong->frag_n = true;
		parser_material_write(parser_material_kong, string_join(string_join("var ", store), "_dx: float3 = ddx3(n);"));
		parser_material_write(parser_material_kong, string_join(string_join("var ", store), "_dy: float3 = ddy3(n);"));
		parser_material_write(
		    parser_material_kong,
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("var ", store),
		                                                                                                                "_curvature: float = max(dot("),
		                                                                                                    store),
		                                                                                        "_dx, "),
		                                                                            store),
		                                                                "_dx), dot("),
		                                                    store),
		                                        "_dy, "),
		                            store),
		                "_dy));"));
		parser_material_write(
		    parser_material_kong,
		    string_join(
		        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(store, "_curvature = clamp(pow("), store),
		                                                                                "_curvature, (1.0 / "),
		                                                                    f32_to_string(radius)),
		                                                        ") * 0.25) * "),
		                                            f32_to_string(strength)),
		                                " * 2.0 + "),
		                    f32_to_string(offset)),
		        " / 10.0, 0.0, 1.0);"));
		return string_join(store, "_curvature");
	}
	else { // Random Per Island
		return "0.0";
	}
}
