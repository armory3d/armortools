
#include "../global.h"

char                        *str_tex_magic = "\
fun tex_magic(p: float3): float3 { \
	var a: float = 1.0 - (sin(p.x) + sin(p.y)); \
	var b: float = 1.0 - sin(p.x - p.y); \
	var c: float = 1.0 - sin(p.x + p.y); \
	return float3(a, b, c); \
} \
fun tex_magic_f(p: float3): float { \
	var c: float3 = tex_magic(p); \
	return (c.x + c.y + c.z) / 3.0; \
} \
";

void magic_texture_node_init() {

	magic_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Magic Texture"),
	                              .type   = "TEX_MAGIC",
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
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Distortion"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
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
	gc_root(magic_texture_node_def);

	any_array_push(nodes_material_texture, magic_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_MAGIC", magic_texture_node_vector);
	any_map_set(parser_material_node_values, "TEX_MAGIC", magic_texture_node_value);
}

char *magic_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_magic);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	return string("tex_magic(%s * %s * 4.0)", co, scale);
}

char *magic_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_magic);
	char *co    = parser_material_get_coord(node);
	char *scale = parser_material_parse_value_input(node->inputs->buffer[1], false);
	return string("tex_magic_f(%s * %s * 4.0)", co, scale);
}
