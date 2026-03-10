
#include "../global.h"

void brightness_contrast_node_init() {
	any_array_push(nodes_material_color, brightness_contrast_node_def);
	any_map_set(parser_material_node_vectors, "BRIGHTCONTRAST", brightness_contrast_node_vector);
}

char *brightness_contrast_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *out_col = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *bright  = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *contr   = parser_material_parse_value_input(node->inputs->buffer[2], false);
	node_shader_add_function(parser_material_kong, str_brightcontrast);
	return string("brightcontrast(%s, %s, %s)", out_col, bright, contr);
}
