
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
	char *steps    = string("(%s * 10.0 + 1.0)", strength);
	char *tex_name = string("texblur_%s", parser_material_node_name(node, NULL));
	node_shader_add_texture(parser_material_kong, tex_name, string("_%s", tex_name));
	node_shader_add_constant(parser_material_kong, string("%s_size: float2", tex_name), string("_size(_%s)", tex_name));
	char *store = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string("var %s_res: float3 = float3(0.0, 0.0, 0.0);", store));
	parser_material_write(parser_material_kong, string("for (var i: int = 0; i <= int(%s * 2.0); i += 1) {", steps));
	parser_material_write(parser_material_kong, string("for (var j: int = 0; j <= int(%s * 2.0); j += 1) {", steps));
	parser_material_write(parser_material_kong,
	                      string("%s_res += sample(%s, sampler_linear, tex_coord + float2(float(i) - %s, float(j) - %s) / constants.%s_size).rgb;", store,
	                             tex_name, steps, steps, tex_name));
	parser_material_write(parser_material_kong, "}");
	parser_material_write(parser_material_kong, "}");
	parser_material_write(parser_material_kong, string("%s_res = %s_res / ((%s * 2.0 + 1.0) * (%s * 2.0 + 1.0));", store, store, steps, steps));
	return string("%s_res", store);
}
