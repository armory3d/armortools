
#include "../global.h"

void layer_mask_node_init() {
	any_array_push(nodes_material_input, layer_mask_node_def);
	any_map_set(parser_material_node_values, "LAYER_MASK", layer_mask_node_value);
}

char *layer_mask_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	i32 l = node->buttons->buffer[0]->default_value->buffer[0];
	node_shader_add_texture(parser_material_kong, string_join("texpaint", i32_to_string(l)), string_join("_texpaint", i32_to_string(l)));
	return string_join(string_join("sample(texpaint", i32_to_string(l)), ", sampler_linear, tex_coord).r");
}
