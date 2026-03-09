
#include "../global.h"

void normal_node_init() {
	any_array_push(nodes_material_utilities, normal_node_def);
	any_map_set(parser_material_node_vectors, "NORMAL", normal_node_vector);
	any_map_set(parser_material_node_values, "NORMAL", normal_node_value);
}

char *normal_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	return parser_material_vec3(node->outputs->buffer[0]->default_value);
}

char *normal_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *nor    = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *norout = parser_material_vec3(node->outputs->buffer[0]->default_value);
	return string_join(string_join(string_join(string_join("dot(", norout), ", "), nor), ")");
}
