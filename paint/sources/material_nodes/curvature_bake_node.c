
#include "../global.h"

void curvature_bake_node_init() {
	any_array_push(nodes_material_texture, curvature_bake_node_def);
	any_map_set(parser_material_node_values, "BAKE_CURVATURE", curvature_bake_node_value);
}

char *curvature_bake_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (parser_material_bake_passthrough) {
		gc_unroot(parser_material_bake_passthrough_strength);
		parser_material_bake_passthrough_strength = string_copy(parser_material_parse_value_input(node->inputs->buffer[0], false));
		gc_root(parser_material_bake_passthrough_strength);
		gc_unroot(parser_material_bake_passthrough_radius);
		parser_material_bake_passthrough_radius = string_copy(parser_material_parse_value_input(node->inputs->buffer[1], false));
		gc_root(parser_material_bake_passthrough_radius);
		gc_unroot(parser_material_bake_passthrough_offset);
		parser_material_bake_passthrough_offset = string_copy(parser_material_parse_value_input(node->inputs->buffer[2], false));
		gc_root(parser_material_bake_passthrough_offset);
		return "0.0";
	}
	char *tex_name = string("texbake_%s", parser_material_node_name(node, NULL));
	node_shader_add_texture(parser_material_kong, tex_name, string("_%s", tex_name));
	char *store = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string("var %s_res: float = sample(%s, sampler_linear, tex_coord).r;", store, tex_name));
	return string("%s_res", store);
}
