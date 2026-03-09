
#include "../global.h"

void attribute_node_init() {
	any_array_push(nodes_material_input, attribute_node_def);
	any_map_set(parser_material_node_vectors, "ATTRIBUTE", attribute_node_vector);
	any_map_set(parser_material_node_values, "ATTRIBUTE", attribute_node_value);
}

char *attribute_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[0]) { // Color
		if (parser_material_kong->context->allow_vcols) {
			node_shader_context_add_elem(parser_material_kong->context, "col", "short4norm"); // Vcols only for now
			return "input.vcolor";
		}
		else {
			return ("float3(0.0, 0.0, 0.0)");
		}
	}
	else {                                                                                // Vector
		node_shader_context_add_elem(parser_material_kong->context, "tex", "short2norm"); // UVMaps only for now
		return "float3(tex_coord.x, tex_coord.y, 0.0)";
	}
}

char *attribute_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_constant(parser_material_kong, "time: float", "_time");
	return "constants.time";
}
