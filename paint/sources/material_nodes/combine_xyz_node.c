
#include "../global.h"

void combine_xyz_node_init() {
	any_array_push(nodes_material_utilities, combine_xyz_node_def);
	any_map_set(parser_material_node_vectors, "COMBXYZ", combine_xyz_node_vector);
}

char *combine_xyz_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *x = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *y = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *z = parser_material_parse_value_input(node->inputs->buffer[2], false);
	return string("float3(%s, %s, %s)", x, y, z);
}
