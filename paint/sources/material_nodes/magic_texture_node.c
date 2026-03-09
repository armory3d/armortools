
#include "../global.h"

void magic_texture_node_init() {
	any_array_push(nodes_material_texture, magic_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_MAGIC", magic_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_MAGIC", magic_texture_node_value);
}

char *magic_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_magic);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *res   = string_join(string_join(string_join(string_join("tex_magic(", co), " * "), scale), " * 4.0)");
	return res;
}

char *magic_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_magic);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *res   = string_join(string_join(string_join(string_join("tex_magic_f(", co), " * "), scale), " * 4.0)");
	return res;
}
