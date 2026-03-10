
#include "../global.h"

void replace_color_node_init() {
	any_array_push(nodes_material_color, replace_color_node_def);
	any_map_set(parser_material_node_vectors, "REPLACECOL", replace_color_node_vector);
}

char *replace_color_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *input_color = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *old_color   = parser_material_parse_vector_input(node->inputs->buffer[1]);
	char *new_color   = parser_material_parse_vector_input(node->inputs->buffer[2]);
	char *radius      = parser_material_parse_value_input(node->inputs->buffer[3], false);
	char *fuzziness   = parser_material_parse_value_input(node->inputs->buffer[4], false);
	return string("lerp3(%s, %s, clamp((distance(%s, %s) - %s) / max(%s, %s), 0.0, 1.0))", new_color, input_color, old_color, input_color, radius, fuzziness,
	              f32_to_string(parser_material_eps));
}
