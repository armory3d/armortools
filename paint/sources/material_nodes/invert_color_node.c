
#include "../global.h"

void invert_color_node_init() {
	any_array_push(nodes_material_color, invert_color_node_def);
	any_map_set(parser_material_node_vectors, "INVERT_COLOR", invert_color_node_vector);
}

char *invert_color_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *fac     = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *out_col = parser_material_parse_vector_input(node->inputs->buffer[1]);
	return string("lerp3(%s, float3(1.0, 1.0, 1.0) - (%s), %s)", out_col, out_col, fac);
}
