
#include "../global.h"

char *mapping_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *out              = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char *node_translation = parser_material_parse_vector_input(node->inputs->buffer[1]);
	char *node_rotation    = parser_material_parse_vector_input(node->inputs->buffer[2]);
	char *node_scale       = parser_material_parse_vector_input(node->inputs->buffer[3]);
	if (!string_equals(node_scale, "float3(1, 1, 1)")) {
		out = string("(%s * %s)", out, node_scale);
	}
	if (!string_equals(node_rotation, "float3(0, 0, 0)")) {
		// ZYX rotation, Z axis for now..
		char *a = string("%s.z * (3.1415926535 / 180)", node_rotation);
		// x * cos(theta) - y * sin(theta)
		// x * sin(theta) + y * cos(theta)
		out = string("float3(%s.x * cos(%s) - %s.y * sin(%s), %s.x * sin(%s) + %s.y * cos(%s), 0.0)", out, a, out, a, out, a, out, a);
	}
	if (!string_equals(node_translation, "float3(0, 0, 0)")) {
		out = string("(%s + %s)", out, node_translation);
	}
	// if node.rotation[1] != 0.0:
	//     a = node.rotation[1]
	//     out = "float3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
	// if node.rotation[0] != 0.0:
	//     a = node.rotation[0]
	//     out = "float3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
	// if node.use_min:
	// out = "max({0}, float3({1}, {2}, {3}))".format(out, node.min[0], node.min[1])
	// if node.use_max:
	// out = "min({0}, float3({1}, {2}, {3}))".format(out, node.max[0], node.max[1])
	return out;
}

void mapping_node_init() {

	ui_node_t *mapping_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Mapping"),
	                              .type   = "MAPPING",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff522c99,
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
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Location"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Rotation"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 360.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(1.0, 1.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 1}),
	                                  },
	                                  4),
	                              .outputs = any_array_create_from_raw(
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
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});

	any_array_push(nodes_material_utilities, mapping_node_def);
	any_map_set(parser_material_node_vectors, "MAPPING", mapping_node_vector);
}
