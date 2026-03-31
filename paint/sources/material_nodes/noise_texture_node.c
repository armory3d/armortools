
#include "../global.h"

void noise_texture_node_init() {

	noise_texture_node_def = GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                                                   .name   = _tr("Noise Texture"),
	                                                   .type   = "TEX_NOISE",
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
	                                                   .buttons = any_array_create_from_raw((void *[]){}, 0),
	                                                   .width   = 0,
	                                                   .flags   = 0});
	gc_root(noise_texture_node_def);

	any_array_push(nodes_material_texture, noise_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_NOISE", noise_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_NOISE", noise_texture_node_value);
}

char *noise_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_noise);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	return string("float3(tex_noise(%s * %s), tex_noise(%s * %s + 0.33), tex_noise(%s * %s + 0.66))", co, scale, co, scale, co, scale);
}

char *noise_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_noise);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	return string("tex_noise(%s * %s)", co, scale);
}
