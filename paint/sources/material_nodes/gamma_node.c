
#include "../global.h"

void gamma_node_init() {
	any_array_push(nodes_material_color, gamma_node_def);
	any_map_set(parser_material_node_vectors, "GAMMA", gamma_node_vector);
}

char *gamma_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *out_col = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *gamma   = parser_material_parse_value_input(node->inputs->buffer[1], false);
	return string_join(string_join(string_join(string_join("pow3(", out_col), ", "), parser_material_to_vec3(gamma)), ")");
}
