
#include "../global.h"

void layer_node_init() {
	any_array_push(nodes_material_input, layer_node_def);
	any_map_set(parser_material_node_vectors, "LAYER", layer_node_vector);
	any_map_set(parser_material_node_values, "LAYER", layer_node_value);
}

char *layer_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	i32   l  = node->buttons->buffer[0]->default_value->buffer[0];
	char *ls = i32_to_string(l);
	if (socket == node->outputs->buffer[0]) { // Base
		node_shader_add_texture(parser_material_kong, string("texpaint%s", ls), string("_texpaint%s", ls));
		return string("sample(texpaint%s, sampler_linear, tex_coord).rgb", ls);
	}
	else { // Normal
		node_shader_add_texture(parser_material_kong, string("texpaint_nor%s", ls), string("_texpaint_nor%s", ls));
		return string("sample(texpaint_nor%s, sampler_linear, tex_coord).rgb", ls);
	}
}

char *layer_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	i32   l  = node->buttons->buffer[0]->default_value->buffer[0];
	char *ls = i32_to_string(l);
	if (socket == node->outputs->buffer[1]) { // Opac
		node_shader_add_texture(parser_material_kong, string("texpaint%s", ls), string("_texpaint%s", ls));
		return string("sample(texpaint%s, sampler_linear, tex_coord).a", ls);
	}
	else if (socket == node->outputs->buffer[2]) { // Occ
		node_shader_add_texture(parser_material_kong, string("texpaint_pack%s", ls), string("_texpaint_pack%s", ls));
		return string("sample(texpaint_pack%s, sampler_linear, tex_coord).r", ls);
	}
	else if (socket == node->outputs->buffer[3]) { // Rough
		node_shader_add_texture(parser_material_kong, string("texpaint_pack%s", ls), string("_texpaint_pack%s", ls));
		return string("sample(texpaint_pack%s, sampler_linear, tex_coord).g", ls);
	}
	else if (socket == node->outputs->buffer[4]) { // Metal
		node_shader_add_texture(parser_material_kong, string("texpaint_pack%s", ls), string("_texpaint_pack%s", ls));
		return string("sample(texpaint_pack%s, sampler_linear, tex_coord).b", ls);
	}
	else if (socket == node->outputs->buffer[6]) {
		return "0.0";
	} // Emission
	else if (socket == node->outputs->buffer[7]) { // Height
		node_shader_add_texture(parser_material_kong, string("texpaint_pack%s", ls), string("_texpaint_pack%s", ls));
		return string("sample(texpaint_pack%s, sampler_linear, tex_coord).a", ls);
	}
	else {
		return "0.0";
	} // Subsurface
}
