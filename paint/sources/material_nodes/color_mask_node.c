
#include "../global.h"

void color_mask_node_init() {
	any_array_push(nodes_material_utilities, color_mask_node_def);
	any_map_set(parser_material_node_values, "COLMASK", color_mask_node_value);
}

char *color_mask_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	char *input_color = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *mask_color  = parser_material_parse_vector_input(node->inputs->buffer[1]);
	char *radius      = parser_material_parse_value_input(node->inputs->buffer[2], false);
	char *fuzziness   = parser_material_parse_value_input(node->inputs->buffer[3], false);
	return string("clamp(1.0 - (distance(%s, %s) - %s) / max(%s, %s), 0.0, 1.0)", input_color, mask_color, radius, fuzziness,
	              f32_to_string(parser_material_eps));
}
