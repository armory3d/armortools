
#include "../global.h"

void tangent_node_init() {
	any_array_push(nodes_material_input, tangent_node_def);
	any_map_set(parser_material_node_vectors, "TANGENT", tangent_node_vector);
}

char *tangent_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	parser_material_kong->frag_wtangent = true;
	return "input.wtangent";
}
