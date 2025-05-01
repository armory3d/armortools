
let make_mesh_layer_pass_count: i32 = 1;

function make_mesh_run(data: material_t, layer_pass: i32 = 0): node_shader_context_t {
	let context_id: string = layer_pass == 0 ? "mesh" : "mesh" + layer_pass;
	let props: shader_context_t = {
		name: context_id,
		depth_write: layer_pass == 0 ? true : false,
		compare_mode: layer_pass == 0 ? "less" : "equal",
		cull_mode: (context_raw.cull_backfaces || layer_pass > 0) ? "clockwise" : "none",
		vertex_elements: [
			{
				name: "pos",
				data: "short4norm"
			},
			{
				name: "nor",
				data: "short2norm"
			},
			{
				name: "tex",
				data: "short2norm"
			}
		],
		color_attachments: [
			"RGBA64",
			"RGBA64",
			"RGBA64"
		],
		depth_attachment: "DEPTH32"
	};
	let con_mesh: node_shader_context_t = node_shader_context_create(data, props);

	let kong: node_shader_t = node_shader_context_make_kong(con_mesh);

	node_shader_add_out(kong, "tex_coord: float2");
	kong.frag_wvpposition = true;
	node_shader_add_constant(kong, "VP: float4x4", "_view_proj_matrix");
	kong.frag_wposition = true;

	let texture_count: i32 = 0;
	let displace_strength: f32 = make_material_get_displace_strength();
	if (make_material_height_used && displace_strength > 0.0) {
		kong.vert_n = true;
		node_shader_write_vert(kong, "var height: float = 0.0;");
		let num_layers: i32 = 0;
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (!slot_layer_is_visible(l) || !l.paint_height || !slot_layer_is_layer(l)) {
				continue;
			}
			if (num_layers > 16) {
				break;
			}
			num_layers++;
			texture_count++;
			node_shader_add_texture(kong, "texpaint_pack_vert" + l.id, "_texpaint_pack_vert" + l.id);
			node_shader_write_vert(kong, "height += sample_lod(texpaint_pack_vert" + l.id + ", sampler_linear, input.tex, 0.0).a;");
			let masks: slot_layer_t[] = slot_layer_get_masks(l);
			if (masks != null) {
				for (let i: i32 = 0; i < masks.length; ++i) {
					let m: slot_layer_t = masks[i];
					if (!slot_layer_is_visible(m)) {
						continue;
					}
					texture_count++;
					node_shader_add_texture(kong, "texpaint_vert" + m.id, "_texpaint_vert" + m.id);
					node_shader_write_vert(kong, "height *= sample_lod(texpaint_vert" + m.id + ", sampler_linear, input.tex, 0.0).r;");
				}
			}
		}
		node_shader_write_vert(kong, "output.wposition += wnormal * float3(height, height, height) * float3(" + displace_strength + ", " + displace_strength + ", " + displace_strength + ");");
	}

	node_shader_write_vert(kong, "output.pos = constants.VP * float4(output.wposition.xyz, 1.0);");
	node_shader_write_vert(kong, "output.tex_coord = input.tex;");

	kong.frag_out = "float4[3]";
	kong.frag_n = true;
	node_shader_add_function(kong, str_pack_float_int16);

	if (context_raw.tool == workspace_tool_t.COLORID) {
		texture_count++;
		node_shader_add_texture(kong, "texcolorid", "_texcolorid");
		node_shader_write_frag(kong, "output[0] = float4(n.xy, 1.0, pack_f32_i16(0.0, uint(0)));");
		node_shader_write_frag(kong, "var idcol: float3 = pow3(sample_lod(texcolorid, sampler_linear, input.tex_coord, 0.0).rgb, float3(2.2, 2.2, 2.2));");
		node_shader_write_frag(kong, "output[1] = float4(idcol.rgb, 1.0);"); // occ
	}
	else {
		node_shader_add_function(kong, str_octahedron_wrap);
		node_shader_add_function(kong, str_cotangent_frame);
		if (layer_pass > 0) {
			node_shader_add_texture(kong, "gbuffer0");
			node_shader_add_texture(kong, "gbuffer1");
			node_shader_add_texture(kong, "gbuffer2");
			node_shader_write_frag(kong, "var fragcoord: float2 = (input.wvpposition.xy / input.wvpposition.w) * 0.5 + 0.5;");
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

		if (context_raw.draw_wireframe) {
			texture_count++;
			node_shader_add_texture(kong, "texuvmap", "_texuvmap");
		}

		if (context_raw.viewport_mode == viewport_mode_t.MASK && slot_layer_get_masks(context_raw.layer) != null) {
			for (let i: i32 = 0; i < slot_layer_get_masks(context_raw.layer).length; ++i) {
				let m: slot_layer_t = slot_layer_get_masks(context_raw.layer)[i];
				if (!slot_layer_is_visible(m)) {
					continue;
				}
				texture_count++;
				let index: i32 = array_index_of(project_layers, m);
				node_shader_add_texture(kong, "texpaint_view_mask" + m.id, "_texpaint" + index);
			}
		}

		if (context_raw.viewport_mode == viewport_mode_t.LIT && context_raw.render_mode == render_mode_t.FORWARD) {
			texture_count += 2;
			node_shader_add_texture(kong, "senvmap_brdf", "$brdf.k");
			node_shader_add_texture(kong, "senvmap_radiance", "_envmap_radiance");
		}

		// Get layers for this pass
		make_mesh_layer_pass_count = 1;
		let layers: slot_layer_t[] = [];
		let start_count: i32 = texture_count;
		let is_material_tool: bool = context_raw.tool == workspace_tool_t.MATERIAL;
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (is_material_tool && l != context_raw.layer) {
				continue;
			}
			if (!slot_layer_is_layer(l) || !slot_layer_is_visible(l)) {
				continue;
			}

			let count: i32 = 3;
			let masks: slot_layer_t[] = slot_layer_get_masks(l);
			if (masks != null) {
				count += masks.length;
			}
			texture_count += count;
			if (texture_count >= make_mesh_get_max_textures()) {
				texture_count = start_count + count + 3; // gbuffer0_copy, gbuffer1_copy, gbuffer2_copy
				make_mesh_layer_pass_count++;
			}
			if (layer_pass == make_mesh_layer_pass_count - 1) {
				array_push(layers, l);
			}
		}

		let last_pass: bool = layer_pass == make_mesh_layer_pass_count - 1;

		for (let i: i32 = 0; i < layers.length; ++i) {
			let l: slot_layer_t = layers[i];
			if (slot_layer_get_object_mask(l) > 0) {
				node_shader_add_constant(kong, "uid: int", "_uid");
				if (slot_layer_get_object_mask(l) > project_paint_objects.length) { // Atlas
					let visibles: mesh_object_t[] = project_get_atlas_objects(slot_layer_get_object_mask(l));
					node_shader_write_frag(kong, "if (");
					for (let i: i32 = 0; i < visibles.length; ++i) {
						if (i > 0) {
							node_shader_write_frag(kong, " || ");
						}
						let uid: i32 = visibles[i].base.uid;
						node_shader_write_frag(kong, uid + " == constants.uid");
					}
					node_shader_write_frag(kong, ") {");
				}
				else { // Object mask
					let uid: i32 = project_paint_objects[slot_layer_get_object_mask(l) - 1].base.uid;
					node_shader_write_frag(kong, "if (" + uid + " == constants.uid) {");
				}
			}

			node_shader_add_texture(kong, "texpaint" + l.id);
			node_shader_write_frag(kong, "texpaint_sample = sample_lod(texpaint" + l.id + ", sampler_linear, input.tex_coord, 0.0);");
			node_shader_write_frag(kong, "texpaint_opac = texpaint_sample.a;");
			// ///if (arm_direct3d12 || arm_vulkan)
			// if (raw.viewport_mode == viewport_mode_t.LIT) {
			// 	write_frag(kong, "if (texpaint_opac < 0.1) { discard; }");
			// }
			// ///end

			let masks: slot_layer_t[] = slot_layer_get_masks(l);
			if (masks != null) {
				let has_visible: bool = false;
				for (let i: i32 = 0; i < masks.length; ++i) {
					let m: slot_layer_t = masks[i];
					if (slot_layer_is_visible(m)) {
						has_visible = true;
						break;
					}
				}
				if (has_visible) {
					let texpaint_mask: string = "texpaint_mask" + l.id;
					node_shader_write_frag(kong, "var " + texpaint_mask + ": float = 0.0;");
					for (let i: i32 = 0; i < masks.length; ++i) {
						let m: slot_layer_t = masks[i];
						if (!slot_layer_is_visible(m)) {
							continue;
						}
						node_shader_add_texture(kong, "texpaint" + m.id);
						node_shader_write_frag(kong, "{"); // Group mask is sampled across multiple layers
						node_shader_write_frag(kong, "var texpaint_mask_sample" + m.id + ": float = sample_lod(texpaint" + m.id + ", sampler_linear, input.tex_coord, 0.0).r;");
						let opac: f32 = slot_layer_get_opacity(m);
						let mask: string = make_material_blend_mode_mask(m.blending, texpaint_mask, "texpaint_mask_sample" + m.id, "float(" + opac + ")");
						node_shader_write_frag(kong, texpaint_mask + " = " + mask + ";");
						node_shader_write_frag(kong, "}");
					}
					node_shader_write_frag(kong, "texpaint_opac *= clamp(" + texpaint_mask + ", 0.0, 1.0);");
				}
			}

			if (slot_layer_get_opacity(l) < 1) {
				let opac: f32 = slot_layer_get_opacity(l);
				node_shader_write_frag(kong, "texpaint_opac *= " + opac + ";");
			}

			if (l.paint_base) {
				if (l == project_layers[0]) {
					node_shader_write_frag(kong, "basecol = texpaint_sample.rgb * texpaint_opac;");
				}
				else {
					node_shader_write_frag(kong, "basecol = " + make_material_blend_mode(kong, l.blending, "basecol", "texpaint_sample.rgb", "texpaint_opac") + ";");
				}
			}

			if (l.paint_nor || make_material_emis_used) {
				node_shader_add_texture(kong, "texpaint_nor" + l.id);
				node_shader_write_frag(kong, "texpaint_nor_sample = sample_lod(texpaint_nor" + l.id + ", sampler_linear, input.tex_coord, 0.0);");

				if (make_material_emis_used) {
					node_shader_write_frag(kong, "if (texpaint_opac > 0.0) { matid = texpaint_nor_sample.a; }");
				}

				if (l.paint_nor) {
					if (l.paint_nor_blend) {
						// Whiteout blend
						node_shader_write_frag(kong, "{");
						node_shader_write_frag(kong, "var n1: float3 = ntex * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
						node_shader_write_frag(kong, "var n2: float3 = lerp3(float3(0.5, 0.5, 1.0), texpaint_nor_sample.rgb, texpaint_opac) * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
						node_shader_write_frag(kong, "ntex = normalize(float3(n1.xy + n2.xy, n1.z * n2.z)) * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5);");
						node_shader_write_frag(kong, "}");
					}
					else {
						node_shader_write_frag(kong, "ntex = lerp3(ntex, texpaint_nor_sample.rgb, texpaint_opac);");
					}
				}
			}

			if (l.paint_occ || l.paint_rough || l.paint_met || (l.paint_height && make_material_height_used)) {
				node_shader_add_texture(kong, "texpaint_pack" + l.id);
				node_shader_write_frag(kong, "texpaint_pack_sample = sample_lod(texpaint_pack" + l.id + ", sampler_linear, input.tex_coord, 0.0);");

				if (l.paint_occ) {
					node_shader_write_frag(kong, "occlusion = lerp(occlusion, texpaint_pack_sample.r, texpaint_opac);");
				}
				if (l.paint_rough) {
					node_shader_write_frag(kong, "roughness = lerp(roughness, texpaint_pack_sample.g, texpaint_opac);");
				}
				if (l.paint_met) {
					node_shader_write_frag(kong, "metallic = lerp(metallic, texpaint_pack_sample.b, texpaint_opac);");
				}
				if (l.paint_height && make_material_height_used) {
					let assign: string = l.paint_height_blend ? "+=" : "=";
					node_shader_write_frag(kong, "height " + assign + " texpaint_pack_sample.a * texpaint_opac;");
					node_shader_write_frag(kong, "{");
					node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
					node_shader_write_frag(kong, "var tex_step: float = 1.0 / constants.texpaint_size.x;");
					node_shader_write_frag(kong, "height0 " + assign + " sample_lod(texpaint_pack" + l.id + ", sampler_linear, float2(input.tex_coord.x - tex_step, input.tex_coord.y), 0.0).a * texpaint_opac;");
					node_shader_write_frag(kong, "height1 " + assign + " sample_lod(texpaint_pack" + l.id + ", sampler_linear, float2(input.tex_coord.x + tex_step, input.tex_coord.y), 0.0).a * texpaint_opac;");
					node_shader_write_frag(kong, "height2 " + assign + " sample_lod(texpaint_pack" + l.id + ", sampler_linear, float2(input.tex_coord.x, input.tex_coord.y - tex_step), 0.0).a * texpaint_opac;");
					node_shader_write_frag(kong, "height3 " + assign + " sample_lod(texpaint_pack" + l.id + ", sampler_linear, float2(input.tex_coord.x, input.tex_coord.y + tex_step), 0.0).a * texpaint_opac;");
					node_shader_write_frag(kong, "}");
				}
			}

			if (slot_layer_get_object_mask(l) > 0) {
				node_shader_write_frag(kong, "}");
			}
		}

		if (last_pass && context_raw.draw_texels) {
			node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
			node_shader_write_frag(kong, "var texel0: float2 = input.tex_coord * constants.texpaint_size * 0.01;");
			node_shader_write_frag(kong, "var texel1: float2 = input.tex_coord * constants.texpaint_size * 0.1;");
			node_shader_write_frag(kong, "var texel2: float2 = input.tex_coord * constants.texpaint_size;");
			node_shader_write_frag(kong, "basecol *= max(float((int(texel0.x) % 2.0) == (int(texel0.y) % 2.0)), 0.9);");
			node_shader_write_frag(kong, "basecol *= max(float((int(texel1.x) % 2.0) == (int(texel1.y) % 2.0)), 0.9);");
			node_shader_write_frag(kong, "basecol *= max(float((int(texel2.x) % 2.0) == (int(texel2.y) % 2.0)), 0.9);");
		}

		if (last_pass && context_raw.draw_wireframe) {
			node_shader_write_frag(kong, "var wireframe: float = sample_lod(texuvmap, sampler_linear, input.tex_coord, 0.0).a;");
			node_shader_write_frag(kong, "basecol *= 1.0 - wireframe * 0.25;");
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
			con_mesh.data.shader_from_source = true;
			gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader));
			return con_mesh;
		}

		kong.frag_vvec = true;
		node_shader_write_frag(kong, "var TBN: float3x3 = cotangent_frame(n, vvec, input.tex_coord);");
		node_shader_write_frag(kong, "n = ntex * 2.0 - 1.0;");
		node_shader_write_frag(kong, "n.y = -n.y;");
		node_shader_write_frag(kong, "n = normalize(TBN * n);");

		if (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {
			node_shader_write_frag(kong, "basecol = pow3(basecol, float3(2.2, 2.2, 2.2));");

			if (context_raw.viewport_shader != null) {
				node_shader_write_frag(kong, "var output_color: float3;");
				js_call_ptr(context_raw.viewport_shader, kong);
				node_shader_write_frag(kong, "output[1] = float4(output_color, 1.0);");
			}
			else if (context_raw.render_mode == render_mode_t.FORWARD && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
				kong.frag_wposition = true;
				node_shader_write_frag(kong, "var albedo: float3 = lerp3(basecol, float3(0.0, 0.0, 0.0), metallic);");
				node_shader_write_frag(kong, "var f0: float3 = lerp3(float3(0.04, 0.04, 0.04), basecol, metallic);");
				kong.frag_vvec = true;
				node_shader_write_frag(kong, "var dotnv: float = max(0.0, dot(n, vvec));");
				// node_shader_write_frag(kong, "var env_brdf: float2 = senvmap_brdf[uint2(float2(roughness, 1.0 - dotnv) * 256.0)].xy;");
				node_shader_write_frag(kong, "var env_brdf: float4 = senvmap_brdf[uint2(float2(roughness, 1.0 - dotnv) * 256.0)];");
				node_shader_add_constant(kong, "envmap_num_mipmaps: int", "_envmap_num_mipmaps");
				node_shader_add_constant(kong, "envmap_data: float4", "_envmap_data"); // angle, sin(angle), cos(angle), strength
				node_shader_write_frag(kong, "var wreflect: float3 = reflect(-vvec, n);");
				node_shader_write_frag(kong, "var envlod: float = roughness * float(constants.envmap_num_mipmaps);");
				node_shader_add_function(kong, str_envmap_equirect);
				node_shader_write_frag(kong, "var prefiltered_color: float3 = sample_lod(senvmap_radiance, sampler_linear, envmap_equirect(wreflect, constants.envmap_data.x), envlod).rgb;");
				// node_shader_add_constant(kong, "shirr: float4[7]", "_envmap_irradiance");
				node_shader_add_constant(kong, "shirr0: float4", "_envmap_irradiance0");
				node_shader_add_constant(kong, "shirr1: float4", "_envmap_irradiance1");
				node_shader_add_constant(kong, "shirr2: float4", "_envmap_irradiance2");
				node_shader_add_constant(kong, "shirr3: float4", "_envmap_irradiance3");
				node_shader_add_constant(kong, "shirr4: float4", "_envmap_irradiance4");
				node_shader_add_constant(kong, "shirr5: float4", "_envmap_irradiance5");
				node_shader_add_constant(kong, "shirr6: float4", "_envmap_irradiance6");
				node_shader_add_function(kong, str_sh_irradiance);
				node_shader_write_frag(kong, "var indirect: float3 = albedo * (sh_irradiance(float3(n.x * constants.envmap_data.z - n.y * constants.envmap_data.y, n.x * constants.envmap_data.y + n.y * constants.envmap_data.z, n.z)) / 3.14159265);");
				node_shader_write_frag(kong, "indirect += prefiltered_color * (f0 * env_brdf.x + env_brdf.y) * 1.5;");
				node_shader_write_frag(kong, "indirect *= constants.envmap_data.w * occlusion;");
				node_shader_write_frag(kong, "output[1] = float4(indirect, 1.0);");
			}
			else { // Deferred, Pathtraced
				if (make_material_emis_used) {
					node_shader_write_frag(kong, "if (int(matid * 255.0) % float(3) == 1) { basecol *= 10.0; }"); // Boost for bloom
				}
				node_shader_write_frag(kong, "output[1] = float4(basecol, occlusion);");
			}
		}
		else if (context_raw.viewport_mode == viewport_mode_t.BASE_COLOR && context_raw.layer.paint_base) {
			node_shader_write_frag(kong, "output[1] = float4(basecol, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.NORMAL_MAP && context_raw.layer.paint_nor) {
			node_shader_write_frag(kong, "output[1] = float4(ntex.rgb, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OCCLUSION && context_raw.layer.paint_occ) {
			node_shader_write_frag(kong, "output[1] = float4(float3(occlusion, occlusion, occlusion), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.ROUGHNESS && context_raw.layer.paint_rough) {
			node_shader_write_frag(kong, "output[1] = float4(float3(roughness, roughness, roughness), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.METALLIC && context_raw.layer.paint_met) {
			node_shader_write_frag(kong, "output[1] = float4(float3(metallic, metallic, metallic), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OPACITY && context_raw.layer.paint_opac) {
			node_shader_write_frag(kong, "output[1] = float4(float3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.HEIGHT && context_raw.layer.paint_height) {
			node_shader_write_frag(kong, "output[1] = float4(float3(height, height, height), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.EMISSION) {
			node_shader_write_frag(kong, "float emis = int(matid * 255.0) % float(3) == 1 ? 1.0 : 0.0;");
			node_shader_write_frag(kong, "output[1] = float4(float3(emis, emis, emis), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.SUBSURFACE) {
			node_shader_write_frag(kong, "float subs = int(matid * 255.0) % float(3) == 2 ? 1.0 : 0.0;");
			node_shader_write_frag(kong, "output[1] = float4(float3(subs, subs, subs), 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.TEXCOORD) {
			node_shader_write_frag(kong, "output[1] = float4(input.tex_coord, 0.0, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_NORMAL) {
			kong.frag_nattr = true;
			node_shader_write_frag(kong, "output[1] = float4(input.nattr, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.MATERIAL_ID) {
			let id: i32 = context_raw.layer.id;
			node_shader_add_texture(kong, "texpaint_nor" + id);
			node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
			node_shader_write_frag(kong, "var sample_matid: float = texpaint_nor" + id + "[uint2(input.tex_coord * constants.texpaint_size)].a + 1.0 / 255.0;");
			node_shader_write_frag(kong, "var matid_r: float = frac(sin(dot(float2(sample_matid, sample_matid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var matid_g: float = frac(sin(dot(float2(sample_matid * 20.0, sample_matid), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var matid_b: float = frac(sin(dot(float2(sample_matid, sample_matid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "output[1] = float4(matid_r, matid_g, matid_b, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_ID) {
			node_shader_add_constant(kong, "object_id: float", "_object_id");
			node_shader_write_frag(kong, "var obid: float = constants.object_id + 1.0 / 255.0;");
			node_shader_write_frag(kong, "var id_r: float = frac(sin(dot(float2(obid, obid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var id_g: float = frac(sin(dot(float2(obid * 20.0, obid), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var id_b: float = frac(sin(dot(float2(obid, obid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "output[1] = float4(id_r, id_g, id_b, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.MASK && (slot_layer_get_masks(context_raw.layer) != null || slot_layer_is_mask(context_raw.layer))) {
			if (slot_layer_is_mask(context_raw.layer)) {
				let id: i32 = context_raw.layer.id;
				node_shader_write_frag(kong, "var mask_view: float = sample_lod(texpaint" + id + ", sampler_linear, input.tex_coord, 0.0).r;");
			}
			else {
				node_shader_write_frag(kong, "var mask_view: float = 0.0;");
				for (let i: i32 = 0; i < slot_layer_get_masks(context_raw.layer).length; ++i) {
					let m: slot_layer_t = slot_layer_get_masks(context_raw.layer)[i];
					if (!slot_layer_is_visible(m)) {
						continue;
					}
					node_shader_write_frag(kong, "var mask_sample" + m.id + ": float = sample_lod(texpaint_view_mask" + m.id + ", sampler_linear, input.tex_coord, 0.0).r;");
					let opac: f32 = slot_layer_get_opacity(m);
					let mask: string = make_material_blend_mode_mask(m.blending, "mask_view", "mask_sample" + m.id, "float(" + opac + ")");
					node_shader_write_frag(kong, "mask_view = " + mask + ";");
				}
			}
			node_shader_write_frag(kong, "output[1] = float4(mask_view, mask_view, mask_view, 1.0);");
		}
		else {
			node_shader_write_frag(kong, "output[1] = float4(1.0, 0.0, 1.0, 1.0);"); // Pink
		}

		if (context_raw.viewport_mode != viewport_mode_t.LIT && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
			node_shader_write_frag(kong, "output[1].rgb = pow3(output[1].rgb, float3(2.2, 2.2, 2.2));"); ////
		}

		node_shader_write_frag(kong, "n /= (abs(n.x) + abs(n.y) + abs(n.z));");
		// node_shader_write_frag(kong, "n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);");
		node_shader_write_frag(kong, "if (n.z < 0.0) { n.xy = octahedron_wrap(n.xy); }");
		node_shader_write_frag(kong, "output[0] = float4(n.xy, roughness, pack_f32_i16(metallic, uint(int(matid * 255.0) % float(3))));");
	}

	node_shader_write_frag(kong, "output[2] = float4(0.0, 0.0, input.tex_coord.xy);");

	parser_material_finalize(con_mesh);
	con_mesh.data.shader_from_source = true;

	let test: string = node_shader_get(kong);

	// iron_file_save_bytes("c:/users/lubos/desktop/test.kong", sys_string_to_buffer(test), 0);

	gpu_create_shaders_from_kong(test, ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader));

	return con_mesh;
}

function make_mesh_get_max_textures(): i32 {
	return 16 - 3; // G4onG5/G4.c.h MAX_TEXTURES
}
