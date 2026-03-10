
#include "../global.h"

void normal_map_node_init() {
	any_array_push(nodes_material_utilities, normal_map_node_def);
	any_map_set(parser_material_node_vectors, "NORMAL_MAP", normal_map_node_vector);
}

char *normal_map_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *strength = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *norm     = parser_material_parse_vector_input(node->inputs->buffer[1]);

	char *store = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string("var %s_texn: float3 = %s * 2.0 - 1.0;", store, norm));
	parser_material_write(parser_material_kong, string("%s_texn.xy = %s * %s_texn.xy;", store, strength, store));
	parser_material_write(parser_material_kong, string("%s_texn = normalize(%s_texn);", store, store));

	return string("(0.5 * %s_texn + 0.5)", store);
}
