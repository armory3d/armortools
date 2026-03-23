
#include "global.h"

node_shader_context_t *make_mesh_run(material_t *data, i32 layer_pass) {
	char                  *context_id = layer_pass == 0 ? "mesh" : string("mesh%s", i32_to_string(layer_pass));
	shader_context_t      *props      = GC_ALLOC_INIT(shader_context_t, {.name            = context_id,
	                                                                     .depth_write     = layer_pass == 0 ? true : false,
	                                                                     .compare_mode    = layer_pass == 0 ? "less" : "equal",
	                                                                     .cull_mode       = (context_raw->cull_backfaces || layer_pass > 0) ? "clockwise" : "none",
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
                                                                       "RGBA64",
                                                                       "RGBA64",
                                                                   },
                                                                   3),
	                                                                     .depth_attachment = "D32"});
	node_shader_context_t *con_mesh   = node_shader_context_create(data, props);

	if (mesh_data_get_vertex_array(context_raw->paint_object->data, "col") != NULL) {
		node_shader_context_add_elem(con_mesh, "col", "short4norm");
	}

	if (mesh_data_get_vertex_array(context_raw->paint_object->data, "tex1") != NULL) {
		node_shader_context_add_elem(con_mesh, "tex1", "short2norm");
	}

	node_shader_t *kong = node_shader_context_make_kong(con_mesh);

	node_shader_add_out(kong, "tex_coord: float2");
	kong->frag_wvpposition = true;
	node_shader_add_constant(kong, "VP: float4x4", "_view_proj_matrix");
	kong->frag_wposition = true;

	i32 texture_count     = 0;
	f32 displace_strength = make_material_get_displace_strength();
	if (make_material_height_used && displace_strength > 0.0) {
		kong->vert_n = true;
		node_shader_write_vert(kong, "var height: float = 0.0;");
		i32 num_layers = 0;
		for (i32 i = 0; i < project_layers->length; ++i) {
			slot_layer_t *l = project_layers->buffer[i];
			if (!slot_layer_is_visible(l) || !l->paint_height || !slot_layer_is_layer(l)) {
				continue;
			}
			if (num_layers > 16) {
				break;
			}
			num_layers++;
			texture_count++;
			node_shader_add_texture(kong, string("texpaint_pack_vert%s", i32_to_string(l->id)), string("_texpaint_pack_vert%s", i32_to_string(l->id)));
			node_shader_write_vert(kong, string("height += sample_lod(texpaint_pack_vert%s, sampler_linear, input.tex, 0.0).a;", i32_to_string(l->id)));
			slot_layer_t_array_t *masks = slot_layer_get_masks(l, true);
			if (masks != NULL) {
				for (i32 i = 0; i < masks->length; ++i) {
					slot_layer_t *m = masks->buffer[i];
					if (!slot_layer_is_visible(m)) {
						continue;
					}
					texture_count++;
					node_shader_add_texture(kong, string("texpaint_vert%s", i32_to_string(m->id)), string("_texpaint_vert%s", i32_to_string(m->id)));
					node_shader_write_vert(kong, string("height *= sample_lod(texpaint_vert%s, sampler_linear, input.tex, 0.0).r;", i32_to_string(m->id)));
				}
			}
		}
		node_shader_write_vert(kong, string("output.wposition += wnormal * float3(height, height, height) * float3(%s, %s, %s);",
		                                    f32_to_string(displace_strength), f32_to_string(displace_strength), f32_to_string(displace_strength)));
	}

	node_shader_write_vert(kong, "output.pos = constants.VP * float4(output.wposition.xyz, 1.0);");
	node_shader_write_vert(kong, "output.tex_coord = input.tex;");
	node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");

	if (mesh_data_get_vertex_array(context_raw->paint_object->data, "tex1") != NULL) {
		node_shader_add_out(kong, "tex_coord1: float2");
		node_shader_write_vert(kong, "output.tex_coord1 = input.tex1;");
		node_shader_write_attrib_frag(kong, "var tex_coord1: float2 = input.tex_coord1;");
	}

	kong->frag_out = "float4[3]";
	kong->frag_n   = true;
	node_shader_add_function(kong, str_pack_float_int16);

	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		if (l->texpaint_sculpt != NULL) {
			sculpt_make_mesh_run(kong, l);
		}
	}
	if (context_raw->tool == TOOL_TYPE_COLORID) {
		texture_count++;
		node_shader_add_texture(kong, "texcolorid", "_texcolorid");
		node_shader_write_frag(kong, "output[0] = float4(n.xy, 1.0, pack_f32_i16(0.0, uint(0)));");
		node_shader_write_frag(kong, "var idcol: float3 = pow3(sample_lod(texcolorid, sampler_linear, tex_coord, 0.0).rgb, float3(2.2, 2.2, 2.2));");
		node_shader_write_frag(kong, "output[1] = float4(idcol.rgb, 1.0);"); // occ
	}
	else {
		node_shader_add_function(kong, str_octahedron_wrap);
		node_shader_add_function(kong, str_cotangent_frame);
		if (layer_pass > 0) {
			node_shader_add_texture(kong, "gbuffer0", NULL);
			node_shader_add_texture(kong, "gbuffer1", NULL);
			node_shader_add_texture(kong, "gbuffer2", NULL);
			node_shader_write_frag(
			    kong, "var fragcoord: float2 = float2(input.wvpposition.x / input.wvpposition.w, input.wvpposition.y / input.wvpposition.w) * 0.5 + 0.5;");
			node_shader_write_frag(kong, "fragcoord.y = 1.0 - fragcoord.y;");
			node_shader_write_frag(kong, "var gbuffer0_sample: float4 = sample_lod(gbuffer0, sampler_linear, fragcoord, 0.0);");
			node_shader_write_frag(kong, "var gbuffer1_sample: float4 = sample_lod(gbuffer1, sampler_linear, fragcoord, 0.0);");
			node_shader_write_frag(kong, "var gbuffer2_sample: float4 = sample_lod(gbuffer2, sampler_linear, fragcoord, 0.0);");
			node_shader_write_frag(kong, "var basecol: float3 = gbuffer0_sample.rgb;");
			node_shader_write_frag(kong, "var roughness: float = gbuffer2_sample.g;");
			node_shader_write_frag(kong, "var metallic: float = gbuffer2_sample.b;");
			node_shader_write_frag(kong, "var occlusion: float = gbuffer2_sample.r;");
			node_shader_write_frag(kong, "var opacity: float = 1.0;//gbuffer0_sample.a;");
			node_shader_write_frag(kong, "var matid: float = gbuffer1_sample.a;");
			node_shader_write_frag(kong, "var ntex: float3 = gbuffer1_sample.rgb;");
			node_shader_write_frag(kong, "var height: float = gbuffer2_sample.a;");
		}
		else {
			node_shader_write_frag(kong, "var basecol: float3 = float3(0.0, 0.0, 0.0);");
			node_shader_write_frag(kong, "var roughness: float = 0.0;");
			node_shader_write_frag(kong, "var metallic: float = 0.0;");
			node_shader_write_frag(kong, "var occlusion: float = 1.0;");
			node_shader_write_frag(kong, "var opacity: float = 1.0;");
			node_shader_write_frag(kong, "var matid: float = 0.0;");
			node_shader_write_frag(kong, "var ntex: float3 = float3(0.5, 0.5, 1.0);");
			node_shader_write_frag(kong, "var height: float = 0.0;");
		}
		node_shader_write_frag(kong, "var texpaint_sample: float4 = float4(0.0, 0.0, 0.0, 1.0);");
		node_shader_write_frag(kong, "var texpaint_nor_sample: float4;");
		node_shader_write_frag(kong, "var texpaint_pack_sample: float4;");
		node_shader_write_frag(kong, "var texpaint_opac: float;");

		if (make_material_height_used) {
			node_shader_write_frag(kong, "var height0: float = 0.0;");
			node_shader_write_frag(kong, "var height1: float = 0.0;");
			node_shader_write_frag(kong, "var height2: float = 0.0;");
			node_shader_write_frag(kong, "var height3: float = 0.0;");
		}

		if (context_raw->draw_wireframe) {
			texture_count++;
			node_shader_add_texture(kong, "texuvmap", "_texuvmap");
		}

		if (context_raw->viewport_mode == VIEWPORT_MODE_MASK && slot_layer_get_masks(context_raw->layer, true) != NULL) {
			for (i32 i = 0; i < slot_layer_get_masks(context_raw->layer, true)->length; ++i) {
				slot_layer_t *m = slot_layer_get_masks(context_raw->layer, true)->buffer[i];
				if (!slot_layer_is_visible(m)) {
					continue;
				}
				texture_count++;
				i32 index = array_index_of(project_layers, m);
				node_shader_add_texture(kong, string("texpaint_view_mask%s", i32_to_string(m->id)), string("_texpaint%s", i32_to_string(index)));
			}
		}

		if (context_raw->viewport_mode == VIEWPORT_MODE_LIT && config_raw->render_mode == RENDER_MODE_FORWARD) {
			texture_count += 6;
			node_shader_add_texture(kong, "senvmap_radiance", "_envmap_radiance");
			node_shader_add_texture(kong, "senvmap_radiance0", "_envmap_radiance0");
			node_shader_add_texture(kong, "senvmap_radiance1", "_envmap_radiance1");
			node_shader_add_texture(kong, "senvmap_radiance2", "_envmap_radiance2");
			node_shader_add_texture(kong, "senvmap_radiance3", "_envmap_radiance3");
			node_shader_add_texture(kong, "senvmap_radiance4", "_envmap_radiance4");
		}

		// Get layers for this pass
		make_mesh_layer_pass_count             = 1;
		slot_layer_t_array_t *layers           = any_array_create_from_raw((void *[]){}, 0);
		i32                   start_count      = texture_count;
		bool                  is_material_tool = context_raw->tool == TOOL_TYPE_MATERIAL;
		for (i32 i = 0; i < project_layers->length; ++i) {
			slot_layer_t *l = project_layers->buffer[i];
			if (is_material_tool && l != context_raw->layer) {
				continue;
			}
			if (!slot_layer_is_layer(l) || !slot_layer_is_visible(l)) {
				continue;
			}

			i32                   count = 3;
			slot_layer_t_array_t *masks = slot_layer_get_masks(l, true);
			if (masks != NULL) {
				count += masks->length;
			}
			texture_count += count;
			if (texture_count >= GPU_MAX_TEXTURES - 3) {
				texture_count = start_count + count + 3; // gbuffer0_copy, gbuffer1_copy, gbuffer2_copy
				make_mesh_layer_pass_count++;
			}
			if (layer_pass == make_mesh_layer_pass_count - 1) {
				any_array_push(layers, l);
			}
		}

		bool last_pass = layer_pass == make_mesh_layer_pass_count - 1;

		for (i32 i = 0; i < layers->length; ++i) {
			slot_layer_t *l = layers->buffer[i];
			if (slot_layer_get_object_mask(l) > 0) {
				node_shader_add_constant(kong, "uid: int", "_uid");
				if (slot_layer_get_object_mask(l) > project_paint_objects->length) { // Atlas
					mesh_object_t_array_t *visibles = project_get_atlas_objects(slot_layer_get_object_mask(l));
					node_shader_write_frag(kong, "if (");
					for (i32 i = 0; i < visibles->length; ++i) {
						if (i > 0) {
							node_shader_write_frag(kong, " || ");
						}
						i32 uid = visibles->buffer[i]->base->uid;
						node_shader_write_frag(kong, string("%s == constants.uid", i32_to_string(uid)));
					}
					node_shader_write_frag(kong, ") {");
				}
				else { // Object mask
					i32 uid = project_paint_objects->buffer[slot_layer_get_object_mask(l) - 1]->base->uid;
					node_shader_write_frag(kong, string("if (%s == constants.uid) {", i32_to_string(uid)));
				}
			}

			char *tex_coord = l->uv_map == 1 ? "tex_coord1" : "tex_coord";
			node_shader_add_texture(kong, string("texpaint%s", i32_to_string(l->id)), NULL);
			node_shader_write_frag(kong, string("texpaint_sample = sample_lod(texpaint%s, sampler_linear, %s, 0.0);", i32_to_string(l->id), tex_coord));
			node_shader_write_frag(kong, "texpaint_opac = texpaint_sample.a;");

			// if (context_raw.viewport_mode == viewport_mode_t.LIT && make_material_opac_used) {
			// 	kong.frag_wvpposition = true;
			// 	node_shader_add_function(kong, str_dither_bayer);
			// 	node_shader_add_constant(kong, "gbuffer_size: float2", "_gbuffer_size");
			// 	node_shader_write_frag(kong, "var fragcoord1: float2 = float2(input.wvpposition.x / input.wvpposition.w, input.wvpposition.y /
			// input.wvpposition.w) * 0.5 + 0.5;"); 	node_shader_write_frag(kong, "var dither: float = dither_bayer(fragcoord1 * constants.gbuffer_size);");
			// 	node_shader_write_frag(kong, "if (texpaint_opac < dither) { discard; }");
			// }

			slot_layer_t_array_t *masks = slot_layer_get_masks(l, true);
			if (masks != NULL) {
				bool has_visible = false;
				for (i32 i = 0; i < masks->length; ++i) {
					slot_layer_t *m = masks->buffer[i];
					if (slot_layer_is_visible(m)) {
						has_visible = true;
						break;
					}
				}
				if (has_visible) {
					char *texpaint_mask = string("texpaint_mask%s", i32_to_string(l->id));
					node_shader_write_frag(kong, string("var %s: float = 0.0;", texpaint_mask));
					for (i32 i = 0; i < masks->length; ++i) {
						slot_layer_t *m = masks->buffer[i];
						if (!slot_layer_is_visible(m)) {
							continue;
						}
						node_shader_add_texture(kong, string("texpaint%s", i32_to_string(m->id)), NULL);
						node_shader_write_frag(kong, "{"); // Group mask is sampled across multiple layers
						node_shader_write_frag(kong, string("var texpaint_mask_sample%s: float = sample_lod(texpaint%s, sampler_linear, %s, 0.0).r;",
						                                    i32_to_string(m->id), i32_to_string(m->id), tex_coord));

						f32   opac = slot_layer_get_opacity(m);
						char *mask = make_material_blend_mode_mask(kong, m->blending, texpaint_mask, string("texpaint_mask_sample%s", i32_to_string(m->id)),
						                                           string("float(%s)", f32_to_string(opac)));
						node_shader_write_frag(kong, string("%s = %s;", texpaint_mask, mask));
						node_shader_write_frag(kong, "}");
					}
					node_shader_write_frag(kong, string("texpaint_opac *= clamp(%s, 0.0, 1.0);", texpaint_mask));
				}
			}

			if (slot_layer_get_opacity(l) < 1) {
				f32 opac = slot_layer_get_opacity(l);
				node_shader_write_frag(kong, string("texpaint_opac *= float(%s);", f32_to_string(opac)));
			}

			if (l->paint_base) {
				if (l == project_layers->buffer[0]) {
					node_shader_write_frag(kong, "basecol = texpaint_sample.rgb * texpaint_opac;");
				}
				else {
					node_shader_write_frag(kong, string("basecol = %s;",
					                                    make_material_blend_mode(kong, l->blending, "basecol", "texpaint_sample.rgb", "texpaint_opac")));
				}
			}

			if (l->paint_nor || make_material_emis_used) {
				node_shader_add_texture(kong, string("texpaint_nor%s", i32_to_string(l->id)), NULL);
				node_shader_write_frag(kong,
				                       string("texpaint_nor_sample = sample_lod(texpaint_nor%s, sampler_linear, %s, 0.0);", i32_to_string(l->id), tex_coord));

				if (make_material_emis_used) {
					node_shader_write_frag(kong, "if (texpaint_opac > 0.0) {");
					node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
					node_shader_write_frag(kong, string("	var texpaint_nor_raw: float4 = texpaint_nor%s[uint2(uint(%s.x * constants.texpaint_size.x), "
					                                    "uint(%s.y * constants.texpaint_size.y))];",
					                                    i32_to_string(l->id), tex_coord, tex_coord));
					node_shader_write_frag(kong, "	matid = texpaint_nor_raw.a;");
					node_shader_write_frag(kong, "}");
				}

				if (l->paint_nor) {
					if (l->paint_nor_blend) {
						// Whiteout blend
						node_shader_write_frag(kong, "{");
						node_shader_write_frag(kong, "var n1: float3 = ntex * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
						node_shader_write_frag(kong, "var n2: float3 = lerp3(float3(0.5, 0.5, 1.0), texpaint_nor_sample.rgb, texpaint_opac) * float3(2.0, 2.0, "
						                             "2.0) - float3(1.0, 1.0, 1.0);");
						node_shader_write_frag(kong, "ntex = normalize(float3(n1.xy + n2.xy, n1.z * n2.z)) * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5);");
						node_shader_write_frag(kong, "}");
					}
					else {
						node_shader_write_frag(kong, "ntex = lerp3(ntex, texpaint_nor_sample.rgb, texpaint_opac);");
					}
				}
			}

			if (l->paint_occ || l->paint_rough || l->paint_met || (l->paint_height && make_material_height_used)) {
				node_shader_add_texture(kong, string("texpaint_pack%s", i32_to_string(l->id)), NULL);
				node_shader_write_frag(kong,
				                       string("texpaint_pack_sample = sample_lod(texpaint_pack%s, sampler_linear, %s, 0.0);", i32_to_string(l->id), tex_coord));

				if (l->paint_occ) {
					node_shader_write_frag(kong, "occlusion = lerp(occlusion, texpaint_pack_sample.r, texpaint_opac);");
				}
				if (l->paint_rough) {
					node_shader_write_frag(kong, "roughness = lerp(roughness, texpaint_pack_sample.g, texpaint_opac);");
				}
				if (l->paint_met) {
					node_shader_write_frag(kong, "metallic = lerp(metallic, texpaint_pack_sample.b, texpaint_opac);");
				}
				if (l->paint_height && make_material_height_used) {
					char *assign = l->paint_height_blend ? "+=" : "=";
					node_shader_write_frag(kong, string("height %s texpaint_pack_sample.a * texpaint_opac;", assign));
					node_shader_write_frag(kong, "{");
					node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
					node_shader_write_frag(kong, "var tex_step: float = 1.0 / constants.texpaint_size.x;");
					node_shader_write_frag(
					    kong, string("height0 %s sample_lod(texpaint_pack%s, sampler_linear, float2(%s.x - tex_step, %s.y), 0.0).a * texpaint_opac;", assign,
					                 i32_to_string(l->id), tex_coord, tex_coord));
					node_shader_write_frag(
					    kong, string("height1 %s sample_lod(texpaint_pack%s, sampler_linear, float2(%s.x + tex_step, %s.y), 0.0).a * texpaint_opac;", assign,
					                 i32_to_string(l->id), tex_coord, tex_coord));
					node_shader_write_frag(
					    kong, string("height2 %s sample_lod(texpaint_pack%s, sampler_linear, float2(%s.x, %s.y - tex_step), 0.0).a * texpaint_opac;", assign,
					                 i32_to_string(l->id), tex_coord, tex_coord));
					node_shader_write_frag(
					    kong, string("height3 %s sample_lod(texpaint_pack%s, sampler_linear, float2(%s.x, %s.y + tex_step), 0.0).a * texpaint_opac;", assign,
					                 i32_to_string(l->id), tex_coord, tex_coord));
					node_shader_write_frag(kong, "}");
				}
			}

			if (slot_layer_get_object_mask(l) > 0) {
				node_shader_write_frag(kong, "}");
			}
		}

		if (last_pass && context_raw->draw_texels) {
			node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
			node_shader_write_frag(kong, "var texel0: float2 = tex_coord * constants.texpaint_size * 0.01;");
			node_shader_write_frag(kong, "var texel1: float2 = tex_coord * constants.texpaint_size * 0.1;");
			node_shader_write_frag(kong, "var texel2: float2 = tex_coord * constants.texpaint_size;");
			// node_shader_write_frag(kong, "basecol = basecol * max(float((int(texel0.x) % 2.0) == (int(texel0.y) % 2.0)), 0.9);");
			// node_shader_write_frag(kong, "basecol = basecol * max(float((int(texel1.x) % 2.0) == (int(texel1.y) % 2.0)), 0.9);");
			// node_shader_write_frag(kong, "basecol = basecol * max(float((int(texel2.x) % 2.0) == (int(texel2.y) % 2.0)), 0.9);");
			node_shader_write_frag(kong, "var texel0xmod: float = float(int(texel0.x)) % 2.0;");
			node_shader_write_frag(kong, "var texel0ymod: float = float(int(texel0.y)) % 2.0;");
			node_shader_write_frag(kong, "var texel1xmod: float = float(int(texel1.x)) % 2.0;");
			node_shader_write_frag(kong, "var texel1ymod: float = float(int(texel1.y)) % 2.0;");
			node_shader_write_frag(kong, "var texel2xmod: float = float(int(texel2.x)) % 2.0;");
			node_shader_write_frag(kong, "var texel2ymod: float = float(int(texel2.y)) % 2.0;");
			node_shader_write_frag(kong, "if (texel0xmod == texel0ymod) { basecol = basecol * 0.9; }");
			node_shader_write_frag(kong, "if (texel1xmod == texel1ymod) { basecol = basecol * 0.9; }");
			node_shader_write_frag(kong, "if (texel2xmod == texel2ymod) { basecol = basecol * 0.9; }");
		}

		if (last_pass && context_raw->draw_wireframe) {
			node_shader_write_frag(kong, "var wireframe: float = sample_lod(texuvmap, sampler_linear, tex_coord, 0.0).a;");
			node_shader_write_frag(kong, "basecol = basecol * (1.0 - wireframe * 0.25);");
			node_shader_write_frag(kong, "roughness = max(roughness, wireframe);");
		}

		if (make_material_height_used) {
			node_shader_write_frag(kong, "if (height > 0.0) {");
			node_shader_write_frag(kong, "var height_dx: float = height0 - height1;");
			node_shader_write_frag(kong, "var height_dy: float = height2 - height3;");
			// Whiteout blend
			node_shader_write_frag(kong, "var n1: float3 = ntex * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
			node_shader_write_frag(kong, "var n2: float3 = normalize(float3(height_dx * 16.0, height_dy * 16.0, 1.0));");
			node_shader_write_frag(kong, "ntex = normalize(float3(n1.xy + n2.xy, n1.z * n2.z)) * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5);");
			node_shader_write_frag(kong, "}");
		}

		if (!last_pass) {
			node_shader_write_frag(kong, "output[0] = float4(basecol, opacity);");
			node_shader_write_frag(kong, "output[1] = float4(ntex, matid);");
			node_shader_write_frag(kong, "output[2] = float4(occlusion, roughness, metallic, height);");
			parser_material_finalize(con_mesh);
			con_mesh->data->shader_from_source = true;
			gpu_create_shaders_from_kong(node_shader_get(kong), &con_mesh->data->vertex_shader, &con_mesh->data->fragment_shader,
			                             &con_mesh->data->_->vertex_shader_size, &con_mesh->data->_->fragment_shader_size);
			return con_mesh;
		}
		kong->frag_vvec = true;
		node_shader_write_frag(kong, "var TBN: float3x3 = cotangent_frame(n, vvec, tex_coord);");
		node_shader_write_frag(kong, "n = ntex * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
		node_shader_write_frag(kong, "n.y = -n.y;");
		node_shader_write_frag(kong, "n = normalize(TBN * n);");

		if (context_raw->viewport_mode == VIEWPORT_MODE_LIT || context_raw->viewport_mode == VIEWPORT_MODE_PATH_TRACE) {
			node_shader_write_frag(kong, "basecol = pow3(basecol, float3(2.2, 2.2, 2.2));");
			node_shader_write_frag(kong, "basecol = max3(basecol, float3(0.0, 0.0, 0.0));");

			if (context_raw->viewport_shader != NULL) {
				node_shader_write_frag(kong, "var output_color: float3;");
				minic_val_t args[1] = {minic_val_ptr(kong)};
				minic_call_fn(context_raw->viewport_shader, args, 1);
				node_shader_write_frag(kong, "output[1] = float4(output_color, 1.0);");
			}
			else if (config_raw->render_mode == RENDER_MODE_FORWARD && context_raw->viewport_mode != VIEWPORT_MODE_PATH_TRACE) {
				node_shader_write_frag(kong, "var albedo: float3 = lerp3(basecol, float3(0.0, 0.0, 0.0), metallic);");
				node_shader_write_frag(kong, "var f0: float3 = lerp3(float3(0.04, 0.04, 0.04), basecol, metallic);");
				kong->frag_vvec = true;
				node_shader_write_frag(kong, "var dotnv: float = max(0.0, dot(n, vvec));");
				// node_shader_add_constant(kong, "envmap_num_mipmaps: int", "_envmap_num_mipmaps");
				node_shader_add_constant(kong, "envmap_data: float4", "_envmap_data"); // angle, sin(angle), cos(angle), strength
				node_shader_write_frag(kong, "var wreflect: float3 = reflect(-vvec, n);");

				node_shader_add_function(kong, str_envmap_equirect);
				// node_shader_write_frag(kong, "var envlod: float = roughness * float(constants.envmap_num_mipmaps);");
				// node_shader_write_frag(kong, "var prefiltered_color: float3 = sample_lod(senvmap_radiance, sampler_linear, envmap_equirect(wreflect,
				// constants.envmap_data.x), envlod).rgb;");

				node_shader_add_function(kong, str_envmap_sample);
				node_shader_write_frag(kong, "var envlod: float = roughness * 5.0;");
				node_shader_write_frag(kong, "var lod0: float = floor(envlod);");
				node_shader_write_frag(kong, "var lod1: float = ceil(envlod);");
				node_shader_write_frag(kong, "var lodf: float = envlod - lod0;");
				node_shader_write_frag(kong, "var envmap_coord: float2 = envmap_equirect(wreflect, constants.envmap_data.x);");
				node_shader_write_frag(kong, "var lodc0: float3 = envmap_sample(lod0, envmap_coord);");
				node_shader_write_frag(kong, "var lodc1: float3 = envmap_sample(lod1, envmap_coord);");
				node_shader_write_frag(kong, "var prefiltered_color: float3 = lerp3(lodc0, lodc1, lodf);");
				// node_shader_add_constant(kong, "shirr: float4[7]", "_envmap_irradiance");
				node_shader_add_constant(kong, "shirr0: float4", "_envmap_irradiance0");
				node_shader_add_constant(kong, "shirr1: float4", "_envmap_irradiance1");
				node_shader_add_constant(kong, "shirr2: float4", "_envmap_irradiance2");
				node_shader_add_constant(kong, "shirr3: float4", "_envmap_irradiance3");
				node_shader_add_constant(kong, "shirr4: float4", "_envmap_irradiance4");
				node_shader_add_constant(kong, "shirr5: float4", "_envmap_irradiance5");
				node_shader_add_constant(kong, "shirr6: float4", "_envmap_irradiance6");
				node_shader_add_function(kong, str_sh_irradiance);
				node_shader_write_frag(kong, "var indirect: float3 = albedo * (sh_irradiance(float3(n.x * constants.envmap_data.z - n.y * "
				                             "constants.envmap_data.y, n.x * constants.envmap_data.y + n.y * constants.envmap_data.z, n.z)) / 3.14159265);");
				node_shader_add_function(kong, str_env_brdf_approx);
				node_shader_write_frag(kong, "indirect = indirect + prefiltered_color * env_brdf_approx(f0, roughness, dotnv) * 0.5;");
				node_shader_write_frag(kong, "indirect = indirect * constants.envmap_data.w * occlusion;");
				node_shader_write_frag(kong, "indirect = max3(indirect, float3(0.0, 0.0, 0.0));");
				node_shader_write_frag(kong, "output[1] = float4(indirect, 1.0);");
			}
			else {
				if (make_material_emis_used) {
					node_shader_write_frag(kong, "if (float(int(matid * 255.0)) % float(3) == 1.0) { basecol = basecol * 10.0; }");
				}
				node_shader_write_frag(kong, "output[1] = float4(basecol, occlusion);");
			}
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_BASE_COLOR && context_raw->layer->paint_base) {
			node_shader_write_frag(kong, "output[1] = float4(basecol, 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_NORMAL_MAP && context_raw->layer->paint_nor) {
			node_shader_write_frag(kong, "output[1] = float4(ntex.rgb, 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_OCCLUSION && context_raw->layer->paint_occ) {
			node_shader_write_frag(kong, "output[1] = float4(float3(occlusion, occlusion, occlusion), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_ROUGHNESS && context_raw->layer->paint_rough) {
			node_shader_write_frag(kong, "output[1] = float4(float3(roughness, roughness, roughness), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_METALLIC && context_raw->layer->paint_met) {
			node_shader_write_frag(kong, "output[1] = float4(float3(metallic, metallic, metallic), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_OPACITY && context_raw->layer->paint_opac) {
			node_shader_write_frag(kong, "output[1] = float4(float3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_HEIGHT && context_raw->layer->paint_height) {
			node_shader_write_frag(kong, "output[1] = float4(float3(height, height, height), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_EMISSION) {
			node_shader_write_frag(kong, "var matid_mod: float = float(int(matid * 255.0)) % float(3);");
			node_shader_write_frag(kong, "var emis: float = 0.0; if (matid_mod == 1.0) { emis = 1.0; }");
			node_shader_write_frag(kong, "output[1] = float4(float3(emis, emis, emis), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_SUBSURFACE) {
			node_shader_write_frag(kong, "var matid_mod: float = float(int(matid * 255.0)) % float(3);");
			node_shader_write_frag(kong, "var subs: float = 0.0; if (matid_mod == 2.0) { subs = 1.0; }");
			node_shader_write_frag(kong, "output[1] = float4(float3(subs, subs, subs), 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_TEXCOORD) {
			node_shader_write_frag(kong, "output[1] = float4(tex_coord, 0.0, 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_OBJECT_NORMAL) {
			kong->frag_nattr = true;
			node_shader_write_frag(kong, "output[1] = float4(input.nattr, 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_MATERIAL_ID) {
			i32 id = context_raw->layer->id;
			node_shader_add_texture(kong, string("texpaint_nor%s", i32_to_string(id)), NULL);
			node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
			node_shader_write_frag(kong, "var sample_matid_coord: float2 = tex_coord * constants.texpaint_size;");
			node_shader_write_frag(kong, string("var sample_matid4: float4 = texpaint_nor%s[uint2(uint(sample_matid_coord.x), uint(sample_matid_coord.y))];",
			                                    i32_to_string(id)));
			node_shader_write_frag(kong, "var sample_matid: float = sample_matid4.a + 1.0 / 255.0;");
			node_shader_write_frag(kong,
			                       "var matid_r: float = frac(sin(dot(float2(sample_matid, sample_matid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong,
			                       "var matid_g: float = frac(sin(dot(float2(sample_matid * 20.0, sample_matid), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong,
			                       "var matid_b: float = frac(sin(dot(float2(sample_matid, sample_matid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "output[1] = float4(matid_r, matid_g, matid_b, 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_OBJECT_ID) {
			node_shader_add_constant(kong, "object_id: float", "_object_id");
			node_shader_write_frag(kong, "var obid: float = constants.object_id + 1.0 / 255.0;");
			node_shader_write_frag(kong, "var id_r: float = frac(sin(dot(float2(obid, obid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var id_g: float = frac(sin(dot(float2(obid * 20.0, obid), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var id_b: float = frac(sin(dot(float2(obid, obid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "output[1] = float4(id_r, id_g, id_b, 1.0);");
		}
		else if (context_raw->viewport_mode == VIEWPORT_MODE_MASK &&
		         (slot_layer_get_masks(context_raw->layer, true) != NULL || slot_layer_is_mask(context_raw->layer))) {
			if (slot_layer_is_mask(context_raw->layer)) {
				i32 id = context_raw->layer->id;
				node_shader_write_frag(kong, string("var mask_view: float = sample_lod(texpaint%s, sampler_linear, tex_coord, 0.0).r;", i32_to_string(id)));
			}
			else {
				node_shader_write_frag(kong, "var mask_view: float = 0.0;");
				for (i32 i = 0; i < slot_layer_get_masks(context_raw->layer, true)->length; ++i) {
					slot_layer_t *m = slot_layer_get_masks(context_raw->layer, true)->buffer[i];
					if (!slot_layer_is_visible(m)) {
						continue;
					}
					node_shader_write_frag(kong, string("var mask_sample%s: float = sample_lod(texpaint_view_mask%s, sampler_linear, tex_coord, 0.0).r;",
					                                    i32_to_string(m->id), i32_to_string(m->id)));
					f32   opac = slot_layer_get_opacity(m);
					char *mask = make_material_blend_mode_mask(kong, m->blending, "mask_view", string("mask_sample%s", i32_to_string(m->id)),
					                                           string("float(%s)", f32_to_string(opac)));
					node_shader_write_frag(kong, string("mask_view = %s;", mask));
				}
			}
			node_shader_write_frag(kong, "output[1] = float4(mask_view, mask_view, mask_view, 1.0);");
		}
		else {
			node_shader_write_frag(kong, "output[1] = float4(1.0, 0.0, 1.0, 1.0);");
		}
		node_shader_write_frag(kong, "n = n / (abs(n.x) + abs(n.y) + abs(n.z));");
		// node_shader_write_frag(kong, "n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);");
		node_shader_write_frag(kong, "if (n.z < 0.0) { n.xy = octahedron_wrap(n.xy); }");
		node_shader_write_frag(kong, "output[0] = float4(n.xy, roughness, pack_f32_i16(metallic, uint(float(int(matid * 255.0)) % float(3))));");
	}

	node_shader_write_frag(kong, "output[2] = float4(0.0, 0.0, tex_coord.xy);");

	parser_material_finalize(con_mesh);

	con_mesh->data->shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), &con_mesh->data->vertex_shader, &con_mesh->data->fragment_shader,
	                             &con_mesh->data->_->vertex_shader_size, &con_mesh->data->_->fragment_shader_size);
	return con_mesh;
}
