
#include "../global.h"

void voronoi_texture_node_init() {

	char *voronoi_coloring_data = string("%s\n%s", _tr("Intensity"), _tr("Cells"));
	voronoi_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Voronoi Texture"),
	                              .type   = "TEX_VORONOI",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Factor"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  2),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("coloring"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = u8_array_create_from_string(voronoi_coloring_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(voronoi_texture_node_def);

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
