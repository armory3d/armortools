
#include "../global.h"

void warp_node_init() {
	any_array_push(nodes_material_color, warp_node_def);
	any_map_set(parser_material_node_vectors, "DIRECT_WARP", warp_node_vector);
}

char *warp_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (parser_material_warp_passthrough) {
		return parser_material_parse_vector_input(node->inputs->buffer[0]);
	}
	char *angle    = parser_material_parse_value_input(node->inputs->buffer[1], true);
	char *mask     = parser_material_parse_value_input(node->inputs->buffer[2], true);
	char *tex_name = string("texwarp_%s", parser_material_node_name(node, NULL));
	node_shader_add_texture(parser_material_kong, tex_name, string("_%s", tex_name));
	char *store = parser_material_store_var_name(node);
	f32   pi    = math_pi();
	parser_material_write(parser_material_kong, string("var %s_rad: float = %s * (%s / 180.0);", store, angle, f32_to_string(pi)));
	parser_material_write(parser_material_kong, string("var %s_x: float = cos(%s_rad);", store, store));
	parser_material_write(parser_material_kong, string("var %s_y: float = sin(%s_rad);", store, store));
	return string("sample(%s, sampler_linear, tex_coord + float2(%s_x, %s_y) * %s).rgb", tex_name, store, store, mask);
}
