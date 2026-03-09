
#include "../global.h"

void picker_node_init() {
	any_array_push(nodes_material_input, picker_node_def);
	any_map_set(parser_material_node_vectors, "PICKER", picker_node_vector);
	any_map_set(parser_material_node_values, "PICKER", picker_node_value);
}

char *picker_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[0]) { // Base
		node_shader_add_constant(parser_material_kong, "picker_base: float3", "_picker_base");
		return "constants.picker_base";
	}
	else { // Normal
		node_shader_add_constant(parser_material_kong, "picker_normal: float3", "_picker_normal");
		return "constants.picker_normal";
	}
}

char *picker_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[1]) {
		node_shader_add_constant(parser_material_kong, "picker_opacity: float", "_picker_opacity");
		return "constants.picker_opacity";
	}
	else if (socket == node->outputs->buffer[2]) {
		node_shader_add_constant(parser_material_kong, "picker_occlusion: float", "_picker_occlusion");
		return "constants.picker_occlusion";
	}
	else if (socket == node->outputs->buffer[3]) {
		node_shader_add_constant(parser_material_kong, "picker_roughness: float", "_picker_roughness");
		return "constants.picker_roughness";
	}
	else if (socket == node->outputs->buffer[4]) {
		node_shader_add_constant(parser_material_kong, "picker_metallic: float", "_picker_metallic");
		return "constants.picker_metallic";
	}
	else if (socket == node->outputs->buffer[6]) {
		return "0.0";
	} // Emission
	else if (socket == node->outputs->buffer[7]) {
		node_shader_add_constant(parser_material_kong, "picker_height: float", "_picker_height");
		return "constants.picker_height";
	}
	else {
		return "0.0";
	} // Subsurface
}
