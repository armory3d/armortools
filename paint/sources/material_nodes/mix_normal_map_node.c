
#include "../global.h"

void mix_normal_map_node_init() {
	any_array_push(nodes_material_utilities, mix_normal_map_node_def);
	any_map_set(parser_material_node_vectors, "MIX_NORMAL_MAP", mix_normal_map_node_vector);
}

char *mix_normal_map_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char         *nm1   = parser_material_parse_vector_input(node->inputs->buffer[0]);
	char         *nm2   = parser_material_parse_vector_input(node->inputs->buffer[1]);
	ui_node_button_t *but   = node->buttons->buffer[0];
	char         *blend = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0])); // blend_type
	blend                   = string_copy(string_replace_all(blend, " ", "_"));
	char *store         = parser_material_store_var_name(node);

	// The blending algorithms are based on the paper "Blending in Detail" by Colin Barré-Brisebois and Stephen Hill 2012
	// https://blog.selfshadow.com/publications/blending-in-detail/
	if (string_equals(blend, "PARTIAL_DERIVATIVE")) { // partial derivate blending
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_n1: float3 = "), nm1), " * 2.0 - 1.0;"));
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_n2: float3 = "), nm2), " * 2.0 - 1.0;"));
		return string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(string_join(string_join(string_join(string_join(string_join(string_join("0.5 * normalize(float3(", store), "_n1.xy * "),
		                                                                                store),
		                                                                    "_n2.z + "),
		                                                        store),
		                                            "_n2.xy * "),
		                                store),
		                    "_n1.z, "),
		                store),
		            "_n1.z * "),
		        store),
		    "_n2.z)) + 0.5");
	}
	else if (string_equals(blend, "WHITEOUT")) { // whiteout blending
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_n1: float3 = "), nm1), " * 2.0 - 1.0;"));
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_n2: float3 = "), nm2), " * 2.0 - 1.0;"));
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("0.5 * normalize(float3(", store), "_n1.xy + "), store),
		                                                    "_n2.xy, "),
		                                        store),
		                            "_n1.z * "),
		                store),
		    "_n2.z)) + 0.5");
	}
	else { // REORIENTED - reoriented normal mapping
		parser_material_write(parser_material_kong,
		                      string_join(string_join(string_join(string_join("var ", store), "_n1: float3 = "), nm1), " * 2.0 - float3(1.0, 1.0, 0.0);"));
		parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", store), "_n2: float3 = "), nm2),
		                                                        " * float3(-2.0, -2.0, 2.0) - float3(-1.0, -1.0, 1.0);"));
		return string_join(
		    string_join(
		        string_join(
		            string_join(string_join(string_join(string_join(string_join(string_join(string_join("0.5 * normalize(", store), "_n1 * dot("), store),
		                                                            "_n1, "),
		                                                store),
		                                    "_n2) - "),
		                        store),
		            "_n2 * "),
		        store),
		    "_n1.z) + 0.5");
	}
}
