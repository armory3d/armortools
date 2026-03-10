
#include "../global.h"

void brick_texture_node_init() {
	any_array_push(nodes_material_texture, brick_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_BRICK", brick_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_BRICK", brick_texture_node_value);
}

char *brick_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_brick);
	char *co    = parser_material_get_coord(node);
	char *col1  = parser_material_parse_vector_input(node->inputs->buffer[1]);
	char *col2  = parser_material_parse_vector_input(node->inputs->buffer[2]);
	char *col3  = parser_material_parse_vector_input(node->inputs->buffer[3]);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[4], false);
	return string("tex_brick(%s * %s, %s, %s, %s)", co, scale, col1, col2, col3);
}

char *brick_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_brick);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[4], false);
	return string("tex_brick_f(%s * %s)", co, scale);
}
