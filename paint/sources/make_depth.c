
#include "global.h"

node_shader_context_t *make_depth_run(material_t *data, material_context_t *matcon) {
	shader_context_t      *props     = GC_ALLOC_INIT(shader_context_t, {.name            = "depth",
	                                                                    .depth_write     = true,
	                                                                    .compare_mode    = "less",
	                                                                    .cull_mode       = g_context->cull_backfaces ? "clockwise" : "none",
	                                                                    .vertex_elements = any_array_create_from_raw(
                                                                   (void *[]){
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "pos", .data = "short4norm"}),
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "nor", .data = "short2norm"}),
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "tex", .data = "short2norm"}),
                                                                   },
                                                                   3),
	                                                                    .color_attachments = any_array_create_from_raw(
                                                                   (void *[]){
                                                                       "RGBA64",
                                                                   },
                                                                   1),
	                                                                    .depth_attachment = "D32"});
	node_shader_context_t *con_depth = node_shader_context_create(data, props);

	node_shader_t *kong  = node_shader_context_make_kong(con_depth);
	kong->frag_wposition = true;

	node_shader_add_constant(kong, "VP: float4x4", "_view_proj_matrix");

	// f32 displace_strength = make_material_get_displace_strength();
	// if (make_material_height_used && displace_strength > 0.0) {
	// 	kong->vert_n = true;
	// 	node_shader_write_vert(kong, "var height: float = 0.0;");
	// 	i32 num_layers = 0;
	// 	for (i32 i = 0; i < project_layers->length; ++i) {
	// 		slot_layer_t *l = project_layers->buffer[i];
	// 		if (!slot_layer_is_visible(l) || !l->paint_height || !slot_layer_is_layer(l)) {
	// 			continue;
	// 		}
	// 		if (num_layers > 16) {
	// 			break;
	// 		}
	// 		num_layers++;
	// 		node_shader_add_texture(kong, string("texpaint_pack_vert%s", i32_to_string(l->id)), string("_texpaint_pack_vert%s", i32_to_string(l->id)));
	// 		node_shader_write_vert(kong, string("height += sample_lod(texpaint_pack_vert%s, sampler_linear, input.tex, 0.0).a;", i32_to_string(l->id)));
	// 		slot_layer_t_array_t *masks = slot_layer_get_masks(l, true);
	// 		if (masks != NULL) {
	// 			for (i32 i = 0; i < masks->length; ++i) {
	// 				slot_layer_t *m = masks->buffer[i];
	// 				if (!slot_layer_is_visible(m)) {
	// 					continue;
	// 				}
	// 				node_shader_add_texture(kong, string("texpaint_vert%s", i32_to_string(m->id)), string("_texpaint_vert%s", i32_to_string(m->id)));
	// 				node_shader_write_vert(kong, string("height *= sample_lod(texpaint_vert%s, sampler_linear, input.tex, 0.0).r;", i32_to_string(m->id)));
	// 			}
	// 		}
	// 	}
	// 	node_shader_write_vert(kong, string("output.wposition += wnormal * float3(height, height, height) * float3(%s, %s, %s);",
	// 	                                    f32_to_string(displace_strength), f32_to_string(displace_strength), f32_to_string(displace_strength)));
	// }

	node_shader_write_vert(kong, "output.pos = constants.VP * float4(output.wposition.xyz, 1.0);");

	con_depth->data->color_writes_red = u8_array_create_from_raw(
	    (u8[]){
	        false,
	    },
	    1);

	con_depth->data->color_writes_green = u8_array_create_from_raw(
	    (u8[]){
	        false,
	    },
	    1);

	con_depth->data->color_writes_blue = u8_array_create_from_raw(
	    (u8[]){
	        false,
	    },
	    1);

	con_depth->data->color_writes_alpha = u8_array_create_from_raw(
	    (u8[]){
	        false,
	    },
	    1);

	parser_material_finalize(con_depth);

	con_depth->data->shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), &con_depth->data->vertex_shader, &con_depth->data->fragment_shader,
	                             &con_depth->data->_->vertex_shader_size, &con_depth->data->_->fragment_shader_size);
	return con_depth;
}
