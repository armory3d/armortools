
#include "../global.h"

char                        *str_tex_brick = "\
fun tex_brick_noise(n: int): float { \
	var nn: int; \
	n = (n >> 13) ^ n; \
	/*nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;*/ \
	nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 2147483647; \
	return 0.5 * float(nn) / 1073741824.0; \
} \
fun tex_brick(p: float3, c1: float3, c2: float3, c3: float3): float3 { \
	var brick_size: float3 = float3(0.9, 0.49, 0.49); \
	var mortar_size: float3 = float3(0.05, 0.1, 0.1); \
	p /= brick_size / 2.0; \
	if (frac(p.y * 0.5) > 0.5) { p.x += 0.5; } \
	var col: float = floor(p.x / (brick_size.x + (mortar_size.x * 2.0))); \
	var row: float = p.y; \
	p = frac3(p); \
	var b: float3 = step3(p, 1.0 - mortar_size); \
	/*var tint: float = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 0xffff)), 0.0), 1.0);*/ \
	var tint: float = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 65535)), 0.0), 1.0); \
	return lerp3(c3, lerp3(c1, c2, tint), b.x * b.y * b.z); \
} \
fun tex_brick_f(p: float3): float { \
	p /= float3(0.9, 0.49, 0.49) / 2.0; \
	if (frac(p.y * 0.5) > 0.5) { p.x += 0.5; } \
	p = frac3(p); \
	var b: float3 = step3(p, float3(0.95, 0.9, 0.9)); \
	return lerp(1.0, 0.0, b.x * b.y * b.z); \
} \
";

void brick_texture_node_init() {

	brick_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Brick Texture"),
	                              .type   = "TEX_BRICK",
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
	                                                                       .name          = _tr("Color 1"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.8, 0.8, 0.8),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color 2"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyz(0.2, 0.2, 0.2),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Mortar"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
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
	                                  5),
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
	gc_root(brick_texture_node_def);

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
