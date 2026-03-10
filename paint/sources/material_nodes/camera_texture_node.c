
#include "../global.h"

void camera_texture_node_init() {
	any_array_push(nodes_material_texture, camera_texture_node_def);
	any_map_set(parser_material_node_vectors, "TEX_CAMERA", camera_texture_node_vector);
}

char *camera_texture_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *tex_name = string("texcamera_%s", parser_material_node_name(node, NULL));
	node_shader_add_texture(parser_material_kong, tex_name, "_camera_texture");
	char *store = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, string("var %s_res: float3 = sample(%s, sampler_linear, tex_coord).rgb;", store, tex_name));
	return string("%s_res", store);
}
