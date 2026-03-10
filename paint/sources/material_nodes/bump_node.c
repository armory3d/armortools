
#include "../global.h"

void bump_node_init() {
	any_array_push(nodes_material_utilities, bump_node_def);
	any_map_set(parser_material_node_vectors, "BUMP", bump_node_vector);
}

char *bump_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *strength = parser_material_parse_value_input(node->inputs->buffer[0], false);
	// char *distance = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *height          = parser_material_parse_value_input(node->inputs->buffer[2], false);
	char *nor             = parser_material_parse_vector_input(node->inputs->buffer[3]);
	char *sample_bump_res = string("%s_bump", parser_material_store_var_name(node));
	parser_material_write(parser_material_kong, string("var %s_x: float = ddx(float(%s)) * (%s) * 16.0;", sample_bump_res, height, strength));
	parser_material_write(parser_material_kong, string("var %s_y: float = ddy(float(%s)) * (%s) * 16.0;", sample_bump_res, height, strength));
	return string("(normalize(float3(%s_x, %s_y, 1.0) + %s) * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5))", sample_bump_res, sample_bump_res, nor);
}
