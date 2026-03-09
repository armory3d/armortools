
#include "../global.h"

void blur_node_init() {
	any_array_push(nodes_material_color, blur_node_def);
	any_map_set(parser_material_node_vectors, "BLUR", blur_node_vector);
}

char *blur_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (parser_material_blur_passthrough) {
		return parser_material_parse_vector_input(node->inputs->buffer[0]);
	}
	char *strength = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *steps    = string_join(string_join("(", strength), " * 10.0 + 1.0)");
	char *tex_name = string_join("texblur_", parser_material_node_name(node, NULL));
	node_shader_add_texture(parser_material_kong, string_join("", tex_name), string_join("_", tex_name));
	node_shader_add_constant(parser_material_kong, string_join(string_join("", tex_name), "_size: float2"), string_join(string_join("_size(_", tex_name), ")"));
	char *store = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string_join(string_join("var ", store), "_res: float3 = float3(0.0, 0.0, 0.0);"));
	parser_material_write(parser_material_kong, string_join(string_join("for (var i: int = 0; i <= int(", steps), " * 2.0); i += 1) {"));
	parser_material_write(parser_material_kong, string_join(string_join("for (var j: int = 0; j <= int(", steps), " * 2.0); j += 1) {"));
	parser_material_write(
	    parser_material_kong,
	    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(store, "_res += sample("), tex_name),
	                                                                                        ", sampler_linear, tex_coord + float2(float(i) - "),
	                                                                            steps),
	                                                                ", float(j) - "),
	                                                    steps),
	                                        ") / constants."),
	                            tex_name),
	                "_size).rgb;"));
	parser_material_write(parser_material_kong, "}");
	parser_material_write(parser_material_kong, "}");
	parser_material_write(parser_material_kong,
	                      string_join(string_join(string_join(string_join(string_join(string_join(string_join(store, "_res = "), store), "_res / (("), steps),
	                                                          " * 2.0 + 1.0) * ("),
	                                              steps),
	                                  " * 2.0 + 1.0));"));
	return string_join(store, "_res");
}
