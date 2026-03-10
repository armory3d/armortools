
#include "../global.h"

void gradient_texture_node_init() {
	any_array_push(nodes_material_texture, gradient_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_GRADIENT", gradient_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_GRADIENT", gradient_texture_node_value);
}

char *parser_material_get_gradient(char *grad, char *co) {
	if (string_equals(grad, "LINEAR")) {
		return string("%s.x", co);
	}
	else if (string_equals(grad, "QUADRATIC")) {
		return "0.0";
	}
	else if (string_equals(grad, "EASING")) {
		return "0.0";
	}
	else if (string_equals(grad, "DIAGONAL")) {
		return string("(%s.x + %s.y) * 0.5", co, co);
	}
	else if (string_equals(grad, "RADIAL")) {
		return string("atan2(%s.x, %s.y) / (3.141592 * 2.0) + 0.5", co, co);
	}
	else if (string_equals(grad, "QUADRATIC_SPHERE")) {
		return "0.0";
	}
	else { // "SPHERICAL"
		return string("max(1.0 - sqrt(%s.x * %s.x + %s.y * %s.y + %s.z * %s.z), 0.0)", co, co, co, co, co, co);
	}
}

char *gradient_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char             *co   = parser_material_get_coord(node);
	ui_node_button_t *but  = node->buttons->buffer[0]; // gradient_type;
	char             *grad = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	grad                   = string_copy(string_replace_all(grad, " ", "_"));
	char *f                = parser_material_get_gradient(grad, co);
	return parser_material_to_vec3(string("clamp(%s, 0.0, 1.0)", f));
}

char *gradient_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char             *co   = parser_material_get_coord(node);
	ui_node_button_t *but  = node->buttons->buffer[0]; // gradient_type;
	char             *grad = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	grad                   = string_copy(string_replace_all(grad, " ", "_"));
	char *f                = parser_material_get_gradient(grad, co);
	return string("(clamp(%s, 0.0, 1.0))", f);
}
