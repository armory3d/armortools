void gradient_texture_node_init() {
	any_array_push(nodes_material_texture, gradient_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_GRADIENT", gradient_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_GRADIENT", gradient_texture_node_value);
}

string_t *parser_material_get_gradient(string_t *grad, string_t *co) {
	if (string_equals(grad, "LINEAR")) {
		return string_join(co, ".x");
	}
	else if (string_equals(grad, "QUADRATIC")) {
		return "0.0";
	}
	else if (string_equals(grad, "EASING")) {
		return "0.0";
	}
	else if (string_equals(grad, "DIAGONAL")) {
		return string_join(string_join(string_join(string_join("(", co), ".x + "), co), ".y) * 0.5");
	}
	else if (string_equals(grad, "RADIAL")) {
		return string_join(string_join(string_join(string_join("atan2(", co), ".x, "), co), ".y) / (3.141592 * 2.0) + 0.5");
	}
	else if (string_equals(grad, "QUADRATIC_SPHERE")) {
		return "0.0";
	}
	else { // "SPHERICAL"
		return string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("max(1.0 - sqrt(", co), ".x * "), co),
		                                                                            ".x + "),
		                                                                co),
		                                                    ".y * "),
		                                        co),
		                            ".y + "),
		                co),
		            ".z * "),
		        co),
		    ".z), 0.0)");
	}
}

string_t *gradient_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	string_t         *co   = parser_material_get_coord(node);
	ui_node_button_t *but  = node->buttons->buffer[0]; // gradient_type;
	string_t         *grad = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	grad                   = string_copy(string_replace_all(grad, " ", "_"));
	string_t *f            = parser_material_get_gradient(grad, co);
	string_t *res          = parser_material_to_vec3(string_join(string_join("clamp(", f), ", 0.0, 1.0)"));
	return res;
}

string_t *gradient_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	string_t         *co   = parser_material_get_coord(node);
	ui_node_button_t *but  = node->buttons->buffer[0]; // gradient_type;
	string_t         *grad = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	grad                   = string_copy(string_replace_all(grad, " ", "_"));
	string_t *f            = parser_material_get_gradient(grad, co);
	string_t *res          = string_join(string_join("(clamp(", f), ", 0.0, 1.0))");
	return res;
}
