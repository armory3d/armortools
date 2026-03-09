
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
	char *tex_name = string_join("texwarp_", parser_material_node_name(node, NULL));
	node_shader_add_texture(parser_material_kong, string_join("", tex_name), string_join("_", tex_name));
	char *store = parser_material_store_var_name(node);
	f32       pi    = math_pi();
	parser_material_write(parser_material_kong,
	                      string_join(string_join(string_join(string_join(string_join(string_join("var ", store), "_rad: float = "), angle), " * ("),
	                                              f32_to_string(pi)),
	                                  " / 180.0);"));
	parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_x: float = cos("), store), "_rad);"));
	parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_y: float = sin("), store), "_rad);"));
	return string_join(
	    string_join(string_join(string_join(string_join(string_join(string_join(string_join("sample(", tex_name), ", sampler_linear, tex_coord + float2("),
	                                                                store),
	                                                    "_x, "),
	                                        store),
	                            "_y) * "),
	                mask),
	    ").rgb");
}
