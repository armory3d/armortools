
#include "../global.h"

void mix_color_node_init() {
	any_array_push(nodes_material_color, mix_color_node_def);
	any_map_set(parser_material_node_vectors, "MIX_RGB", mix_color_node_vector);
}

char *mix_color_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	char *fac     = parser_material_parse_value_input(node->inputs->buffer[0], false);
	char *fac_var = string_join(parser_material_node_name(node, NULL), "_fac");
	parser_material_write(parser_material_kong, string_join(string_join(string_join(string_join("var ", fac_var), ": float = "), fac), ";"));
	char         *col1  = parser_material_parse_vector_input(node->inputs->buffer[1]);
	char         *col2  = parser_material_parse_vector_input(node->inputs->buffer[2]);
	ui_node_button_t *but   = node->buttons->buffer[0]; // blend_type
	char         *blend = to_upper_case(u8_array_string_at(but->data, but->default_value->buffer[0]));
	blend                   = string_copy(string_replace_all(blend, " ", "_"));
	bool      use_clamp     = node->buttons->buffer[1]->default_value->buffer[0] > 0;
	char *out_col       = "";
	if (string_equals(blend, "MIX")) {
		out_col = string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", "), col2), ", "), fac_var), ")");
	}
	else if (string_equals(blend, "DARKEN")) {
		out_col = string_join(string_join(string_join(string_join(string_join(string_join("min3(", col1), ", "), col2), " * "), fac_var), ")");
	}
	else if (string_equals(blend, "MULTIPLY")) {
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", "), col1), " * "), col2), ", "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "BURN")) {
		out_col = string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1),
		                                                                                              ", float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - "),
		                                                                                  col1),
		                                                                      ") / "),
		                                                          col2),
		                                              ", "),
		                                  fac_var),
		                      ")");
	}
	else if (string_equals(blend, "LIGHTEN")) {
		out_col = string_join(string_join(string_join(string_join(string_join(string_join("max3(", col1), ", "), col2), " * "), fac_var), ")");
	}
	else if (string_equals(blend, "SCREEN")) {
		char *v3 = parser_material_to_vec3(string_join("1.0 - ", fac_var));
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("(float3(1.0, 1.0, 1.0) - (", v3), " + "), fac_var),
		                                                                " * (float3(1.0, 1.0, 1.0) - "),
		                                                    col2),
		                                        ")) * (float3(1.0, 1.0, 1.0) - "),
		                            col1),
		                "))");
	}
	else if (string_equals(blend, "DODGE")) {
		out_col = string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", "), col1),
		                                                                      " / (float3(1.0, 1.0, 1.0) - "),
		                                                          col2),
		                                              "), "),
		                                  fac_var),
		                      ")");
	}
	else if (string_equals(blend, "ADD")) {
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", "), col1), " + "), col2), ", "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "OVERLAY")) {
		// out_col = "lerp3(" + col1 + ", float3( \
		// 	" + col1 + ".r < 0.5 ? 2.0 * " + col1 + ".r * " + col2 + ".r : 1.0 - 2.0 * (1.0 - " + col1 + ".r) * (1.0 - " + col2 + ".r), \
		// 	" + col1 + ".g < 0.5 ? 2.0 * " + col1 + ".g * " + col2 + ".g : 1.0 - 2.0 * (1.0 - " + col1 + ".g) * (1.0 - " + col2 + ".g), \
		// 	" + col1 + ".b < 0.5 ? 2.0 * " + col1 + ".b * " + col2 + ".b : 1.0 - 2.0 * (1.0 - " + col1 + ".b) * (1.0 - " + col2 + ".b) \
		// ), " + fac_var + ")";
		char *res_r = string_join(parser_material_node_name(node, NULL), "_res_r");
		char *res_g = string_join(parser_material_node_name(node, NULL), "_res_g");
		char *res_b = string_join(parser_material_node_name(node, NULL), "_res_b");
		parser_material_write(parser_material_kong, string_join(string_join("var ", res_r), ": float;"));
		parser_material_write(parser_material_kong, string_join(string_join("var ", res_g), ": float;"));
		parser_material_write(parser_material_kong, string_join(string_join("var ", res_b), ": float;"));
		parser_material_write(
		    parser_material_kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", col1),
		                                                                                                                        ".r < 0.5) { "),
		                                                                                                            res_r),
		                                                                                                " = 2.0 * "),
		                                                                                    col1),
		                                                                        ".r * "),
		                                                            col2),
		                                                ".r; } else { "),
		                                    res_r),
		                        " = 1.0 - 2.0 * (1.0 - "),
		                    col1),
		                ".r) * (1.0 - "),
		            col2),
		        ".r); }"));
		parser_material_write(
		    parser_material_kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", col1),
		                                                                                                                        ".g < 0.5) { "),
		                                                                                                            res_g),
		                                                                                                " = 2.0 * "),
		                                                                                    col1),
		                                                                        ".g * "),
		                                                            col2),
		                                                ".g; } else { "),
		                                    res_g),
		                        " = 1.0 - 2.0 * (1.0 - "),
		                    col1),
		                ".g) * (1.0 - "),
		            col2),
		        ".g); }"));
		parser_material_write(
		    parser_material_kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", col1),
		                                                                                                                        ".b < 0.5) { "),
		                                                                                                            res_b),
		                                                                                                " = 2.0 * "),
		                                                                                    col1),
		                                                                        ".b * "),
		                                                            col2),
		                                                ".b; } else { "),
		                                    res_b),
		                        " = 1.0 - 2.0 * (1.0 - "),
		                    col1),
		                ".b) * (1.0 - "),
		            col2),
		        ".b); }"));
		out_col = string_join(
		    string_join(
		        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", float3("), res_r), ", "),
		                                                        res_g),
		                                            ", "),
		                                res_b),
		                    "), "),
		        fac_var),
		    ")");
	}
	else if (string_equals(blend, "SOFT_LIGHT")) {
		out_col = string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(
		                            string_join(
		                                string_join(
		                                    string_join(
		                                        string_join(
		                                            string_join(
		                                                string_join(string_join(string_join(string_join(string_join(string_join("((1.0 - ", fac_var), ") * "),
		                                                                                                col1),
		                                                                                    " + "),
		                                                                        fac_var),
		                                                            " * ((float3(1.0, 1.0, 1.0) - "),
		                                                col1),
		                                            ") * "),
		                                        col2),
		                                    " * "),
		                                col1),
		                            " + "),
		                        col1),
		                    " * (float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - "),
		                col2),
		            ") * (float3(1.0, 1.0, 1.0) - "),
		        col1),
		    "))))");
	}
	else if (string_equals(blend, "LINEAR_LIGHT")) {
		out_col = string_join(string_join(string_join(string_join(string_join(string_join("(", col1), " + "), fac_var), " * (float3(2.0, 2.0, 2.0) * ("), col2),
		                      " - float3(0.5, 0.5, 0.5))))");
	}
	else if (string_equals(blend, "DIFFERENCE")) {
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", abs3("), col1), " - "), col2),
		                                        "), "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "SUBTRACT")) {
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1), ", "), col1), " - "), col2), ", "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "DIVIDE")) {
		f32 eps = 0.000001;
		col2    = string_join(
            string_join(string_join(string_join(string_join(string_join(string_join(string_join("max3(", col2), ", float3("), f32_to_string(eps)), ", "),
		                                           f32_to_string(eps)),
		                               ", "),
		                   f32_to_string(eps)),
            "))");
		char *v3 = string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(
		                            string_join(
		                                string_join(
		                                    string_join(
		                                        string_join(
		                                            string_join(string_join(string_join(string_join(string_join(string_join("(float3(1.0, 1.0, 1.0) - float3(",
		                                                                                                                    fac_var),
		                                                                                                        ", "),
		                                                                                            fac_var),
		                                                                                ", "),
		                                                                    fac_var),
		                                                        ")) * "),
		                                            col1),
		                                        " + float3("),
		                                    fac_var),
		                                ", "),
		                            fac_var),
		                        ", "),
		                    fac_var),
		                ") * "),
		            col1),
		        " / "),
		    col2);
		out_col = string_join(string_join("(", v3), ")");
	}
	else if (string_equals(blend, "HUE")) {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1),
		                                                                                                                ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                    col2),
		                                                                                        ").r, rgb_to_hsv("),
		                                                                            col1),
		                                                                ").g, rgb_to_hsv("),
		                                                    col1),
		                                        ").b)), "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "SATURATION")) {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1),
		                                                                                                                ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                    col1),
		                                                                                        ").r, rgb_to_hsv("),
		                                                                            col2),
		                                                                ").g, rgb_to_hsv("),
		                                                    col1),
		                                        ").b)), "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "COLOR")) {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1),
		                                                                                                                ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                    col2),
		                                                                                        ").r, rgb_to_hsv("),
		                                                                            col2),
		                                                                ").g, rgb_to_hsv("),
		                                                    col1),
		                                        ").b)), "),
		                            fac_var),
		                ")");
	}
	else if (string_equals(blend, "VALUE")) {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", col1),
		                                                                                                                ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                    col1),
		                                                                                        ").r, rgb_to_hsv("),
		                                                                            col1),
		                                                                ").g, rgb_to_hsv("),
		                                                    col2),
		                                        ").b)), "),
		                            fac_var),
		                ")");
	}
	if (use_clamp) {
		return string_join(string_join("clamp3(", out_col), ", float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0))");
	}
	else {
		return out_col;
	}
}
