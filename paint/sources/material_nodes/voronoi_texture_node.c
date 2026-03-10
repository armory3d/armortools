
#include "../global.h"

void voronoi_texture_node_init() {
	any_array_push(nodes_material_texture, voronoi_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_VORONOI", voronoi_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_VORONOI", voronoi_texture_node_value);
}

char *voronoi_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_voronoi);
	node_shader_add_texture(parser_material_kong, "snoise256", "$noise256.k");
	char             *co       = parser_material_get_coord(node);
	char             *scale    = parser_material_parse_value_input(node->inputs->buffer[1], false);
	ui_node_button_t *but      = node->buttons->buffer[0]; // coloring;
	char             *coloring = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	coloring                   = string_copy(string_replace_all(coloring, " ", "_"));
	char *res                  = "";
	if (string_equals(coloring, "INTENSITY")) {
		res = string_copy(parser_material_to_vec3(string("tex_voronoi(%s * %s).a", co, scale)));
	}
	else { // Cells
		res = string("tex_voronoi(%s * %s).rgb", co, scale);
	}
	return res;
}

char *voronoi_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_voronoi);
	node_shader_add_texture(parser_material_kong, "snoise256", "$noise256.k");
	char             *co       = parser_material_get_coord(node);
	char             *scale    = parser_material_parse_value_input(node->inputs->buffer[1], false);
	ui_node_button_t *but      = node->buttons->buffer[0]; // coloring
	char             *coloring = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	coloring                   = string_copy(string_replace_all(coloring, " ", "_"));
	char *res                  = "";
	if (string_equals(coloring, "INTENSITY")) {
		res = string("tex_voronoi(%s * %s).a", co, scale);
	}
	else { // Cells
		res = string("tex_voronoi(%s * %s).r", co, scale);
	}
	return res;
}
