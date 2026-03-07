void mapping_node_init() {
	any_array_push(nodes_material_utilities, mapping_node_def);
	any_map_set(parser_material_node_vectors, "MAPPING", mapping_node_vector);
}

char *mapping_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *out              = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *node_translation = parser_material_parse_vector_input(node->inputs->buffer[1]);
	char *node_rotation    = parser_material_parse_vector_input(node->inputs->buffer[2]);
	char *node_scale       = parser_material_parse_vector_input(node->inputs->buffer[3]);
	if (!string_equals(node_scale, "float3(1, 1, 1)")) {
		out = string_join(string_join(string_join(string_join("(", out), " * "), node_scale), ")");
	}
	if (!string_equals(node_rotation, "float3(0, 0, 0)")) {
		// ZYX rotation, Z axis for now..
		char *a = string_join(node_rotation, ".z * (3.1415926535 / 180)");
		// x * cos(theta) - y * sin(theta)
		// x * sin(theta) + y * cos(theta)
		out = string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(
		                            string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("float3(", out),
		                                                                                                                            ".x * cos("),
		                                                                                                                a),
		                                                                                                    ") - "),
		                                                                                        out),
		                                                                            ".y * sin("),
		                                                                a),
		                                                    "), "),
		                                        out),
		                            ".x * sin("),
		                        a),
		                    ") + "),
		                out),
		            ".y * cos("),
		        a),
		    "), 0.0)");
	}
	// if node.rotation[1] != 0.0:
	//     a = node.rotation[1]
	//     out = "float3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
	// if node.rotation[0] != 0.0:
	//     a = node.rotation[0]
	//     out = "float3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
	if (!string_equals(node_translation, "float3(0, 0, 0)")) {
		out = string_join(string_join(string_join(string_join("(", out), " + "), node_translation), ")");
	}
	// if node.use_min:
	// out = "max({0}, float3({1}, {2}, {3}))".format(out, node.min[0], node.min[1])
	// if node.use_max:
	// out = "min({0}, float3({1}, {2}, {3}))".format(out, node.max[0], node.max[1])
	return out;
}
