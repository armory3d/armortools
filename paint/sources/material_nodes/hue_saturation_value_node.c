
#include "../global.h"

void hue_saturation_value_node_init() {
	any_array_push(nodes_material_color, hue_saturation_value_node_def);
	any_map_set(parser_material_node_vectors, "HUE_SAT", hue_saturation_value_node_vector);
}

char *hue_saturation_value_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_hue_sat);
	char *hue = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *sat = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *val = parser_material_parse_value_input(node->inputs->buffer[2], false);
	char *fac = parser_material_parse_value_input(node->inputs->buffer[3], false);
	char *col = parser_material_parse_vector_input(node->inputs->buffer[4]);
	return string("hue_sat(%s, float4(%s - 0.5, %s, %s, 1.0 - %s))", col, hue, sat, val, fac);
}
