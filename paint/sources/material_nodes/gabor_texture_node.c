
#include "../global.h"

void gabor_texture_node_init() {
	any_array_push(nodes_material_texture, gabor_texture_node_def);
	any_map_set(parser_material_node_values, "TEX_GABOR", gabor_texture_node_value);
}

char *gabor_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_gabor);
	char *co          = parser_material_get_coord(node);
	char *scale       = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *frequency   = parser_material_parse_value_input(node->inputs->buffer[2], false);
	char *anisotropy  = parser_material_parse_value_input(node->inputs->buffer[3], false);
	char *orientation = parser_material_parse_vector_input(node->inputs->buffer[4]);
	char *res         = string("tex_gabor(%s, %s, %s, %s, %s)", co, scale, frequency, anisotropy, orientation);
	if (socket == node->outputs->buffer[0]) { // Value
		return string("%s.x", res);
	}
	else if (socket == node->outputs->buffer[1]) { // Phase
		return string("%s.y", res);
	}
	else { // Intensity
		return string("%s.z", res);
	}
}
