
let make_mesh_layer_pass_count: i32 = 1;

function make_mesh_run(data: material_t, layer_pass: i32 = 0): node_shader_context_t {
	let context_id: string = layer_pass == 0 ? "mesh" : "mesh" + layer_pass;
	let depth_write: bool = layer_pass == 0 ? true : false;
	let compare_mode: string = layer_pass == 0 ? "less" : "equal";
	let cull_mode: string = (context_raw.cull_backfaces || layer_pass > 0) ? "clockwise" : "none";

	let props: shader_context_t = {
		name: context_id,
		depth_write: depth_write,
		compare_mode: compare_mode,
		cull_mode: cull_mode,
		vertex_elements: [
			{
				name: "pos",
				data: "short4norm"
			}
		],
		color_attachments: [
			"RGBA64",
			"RGBA64",
			"RGBA64"
		],
		depth_attachment: "D32"
	};

	let con_mesh: node_shader_context_t = node_shader_context_create(data, props);

	let kong: node_shader_t = node_shader_context_make_kong(con_mesh);

	node_shader_add_out(kong, "tex_coord: float2");
	kong.frag_wvpposition = true;
	node_shader_add_constant(kong, "VP: float4x4", "_view_proj_matrix");
	kong.frag_wposition = true;

	let texture_count: i32 = 0;

	node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");
	let lid: i32 = project_layers[0].id;
	node_shader_add_texture(kong, "texpaint_vert", "_texpaint_vert" + lid);
	node_shader_add_constant(kong, "texpaint_vert_size: float2", "_size(texpaint_vert" + lid + ")");
	node_shader_write_vert(kong, "var meshpos: float3 = sample_lod(texpaint_vert, sampler_linear, uint2(vertex_id() % constants.texpaint_vert_size.x, vertex_id() / constants.texpaint_vert_size.y), 0).xyz;");
	// + input.pos.xyz * 0.000001
	node_shader_write_vert(kong, "output.pos = constants.WVP * float4(meshpos.xyz, 1.0);");

	node_shader_write_vert(kong, "output.tex_coord = float2(0.0, 0.0);");

	kong.frag_out = "float4[3]";

	node_shader_add_constant(kong, "N: float3x3", "_normal_matrix");
	node_shader_add_out(kong, "wnormal: float3");

	node_shader_write_attrib_vert(kong, "var base_vertex0: int = vertex_id() - (vertex_id() % float(3));");
	node_shader_write_attrib_vert(kong, "var base_vertex1: int = base_vertex0 + 1;");
	node_shader_write_attrib_vert(kong, "var base_vertex2: int = base_vertex0 + 2;");
	node_shader_write_attrib_vert(kong, "var meshpos0: float3 = sample_lod(texpaint_vert, sampler_linear, uint2(base_vertex0 % constants.texpaint_vert_size.x, base_vertex0 / constants.texpaint_vert_size.y), 0).xyz;");
	node_shader_write_attrib_vert(kong, "var meshpos1: float3 = sample_lod(texpaint_vert, sampler_linear, uint2(base_vertex1 % constants.texpaint_vert_size.x, base_vertex1 / constants.texpaint_vert_size.y), 0).xyz;");
	node_shader_write_attrib_vert(kong, "var meshpos2: float3 = sample_lod(texpaint_vert, sampler_linear, uint2(base_vertex2 % constants.texpaint_vert_size.x, base_vertex2 / constants.texpaint_vert_size.y), 0).xyz;");
	node_shader_write_attrib_vert(kong, "var meshnor: float3 = normalize(cross(meshpos2 - meshpos1, meshpos0 - meshpos1));");
	node_shader_write_attrib_vert(kong, "output.wnormal = constants.N * meshnor;");
	node_shader_write_attrib_frag(kong, "var n: float3 = normalize(input.wnormal);");

	node_shader_add_function(kong, str_pack_float_int16);
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
		node_shader_write_frag(kong, "var roughness: float = 0.3;");
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

	if (context_raw.viewport_mode == viewport_mode_t.LIT && context_raw.render_mode == render_mode_t.FORWARD) {
		texture_count += 2;
		node_shader_add_texture(kong, "senvmap_brdf", "$brdf.k");
		node_shader_add_texture(kong, "senvmap_radiance", "_envmap_radiance");
		node_shader_add_texture(kong, "senvmap_radiance0", "_envmap_radiance0");
		node_shader_add_texture(kong, "senvmap_radiance1", "_envmap_radiance1");
		node_shader_add_texture(kong, "senvmap_radiance2", "_envmap_radiance2");
		node_shader_add_texture(kong, "senvmap_radiance3", "_envmap_radiance3");
		node_shader_add_texture(kong, "senvmap_radiance4", "_envmap_radiance4");
	}

	// Get layers for this pass
	make_mesh_layer_pass_count = 1;
	let layers: slot_layer_t[] = [];
	let start_count: i32 = texture_count;
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
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
		node_shader_write_frag(kong, "texpaint_sample = float4(0.8, 0.8, 0.8, 1.0);");
		node_shader_write_frag(kong, "texpaint_opac = texpaint_sample.a;");

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
					if (!slot_layer_is_visible(m)) continue;
					node_shader_add_texture(kong, "texpaint" + m.id);
					node_shader_write_frag(kong, "{"); // Group mask is sampled across multiple layers
					node_shader_write_frag(kong, "var texpaint_mask_sample" + m.id + ": float = sample_lod(texpaint" + m.id + ", sampler_liniear, input.tex_coord, 0.0).r;");
					node_shader_write_frag(kong, "}");
				}
				node_shader_write_frag(kong, "texpaint_opac *= clamp(" + texpaint_mask + ", 0.0, 1.0);");
			}
		}

		if (slot_layer_get_opacity(l) < 1) {
			let opac: f32 = slot_layer_get_opacity(l);
			node_shader_write_frag(kong, "texpaint_opac *= " + opac + ";");
		}

		if (l == project_layers[0]) {
			node_shader_write_frag(kong, "basecol = float3(0.8, 0.8, 0.8);// texpaint_sample.rgb * texpaint_opac;");
		}

		if (slot_layer_get_object_mask(l) > 0) {
			node_shader_write_frag(kong, "}");
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
			node_shader_write_frag(kong, "basecol *= 1.0 - sample_lod(texuvmap, sampler_linear, input.tex_coord, 0.0).r;");
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
			gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader), ADDRESS(con_mesh.data._.vertex_shader_size), ADDRESS(con_mesh.data._.fragment_shader_size));
			return con_mesh;
		}

		kong.frag_vvec = true;
		node_shader_write_frag(kong, "var TBN: float3x3 = cotangent_frame(n, vvec, input.tex_coord);");
		node_shader_write_frag(kong, "n = ntex * 2.0 - 1.0;");
		node_shader_write_frag(kong, "n.y = -n.y;");
		node_shader_write_frag(kong, "n = normalize(TBN * n);");

		if (context_raw.viewport_mode == viewport_mode_t.LIT) {

			node_shader_write_frag(kong, "basecol = pow3(basecol, float3(2.2, 2.2, 2.2));");

			if (context_raw.viewport_shader != null) {
				node_shader_write_frag(kong, "var output_color: float3;");
				js_call_ptr(context_raw.viewport_shader, kong);
				node_shader_write_frag(kong, "output[1] = float4(output_color, 1.0);");
			}
			else if (context_raw.render_mode == render_mode_t.FORWARD) {
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

				node_shader_add_function(kong, str_envmap_equirect);
				// node_shader_write_frag(kong, "var envlod: float = roughness * float(constants.envmap_num_mipmaps);");
				// node_shader_write_frag(kong, "var prefiltered_color: float3 = sample_lod(senvmap_radiance, sampler_linear, envmap_equirect(wreflect, constants.envmap_data.x), envlod).rgb;");

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
		else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_NORMAL) {
			kong.frag_nattr = true;
			node_shader_write_frag(kong, "output[1] = float4(input.nattr, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_ID) {
			node_shader_add_constant(kong, "object_id: float", "_object_id");
			node_shader_write_frag(kong, "var obid: float = constants.object_id + 1.0 / 255.0;");
			node_shader_write_frag(kong, "var id_r: float = frac(sin(dot(float2(obid, obid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var id_g: float = frac(sin(dot(float2(obid * 20.0, obid), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "var id_b: float = frac(sin(dot(float2(obid, obid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write_frag(kong, "output[1] = float4(id_r, id_g, id_b, 1.0);");
		}
		else {
			node_shader_write_frag(kong, "output[1] = float4(1.0, 0.0, 1.0, 1.0);"); // Pink
		}

		if (context_raw.viewport_mode != viewport_mode_t.LIT && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
			node_shader_write_frag(kong, "output[1].rgb = pow3(output[1].rgb, float3(2.2, 2.2, 2.2));");
		}

		node_shader_write_frag(kong, "n /= (abs(n.x) + abs(n.y) + abs(n.z));");
		// node_shader_write_frag(kong, "n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);");
		node_shader_write_frag(kong, "if (n.z < 0.0) { n.xy = octahedron_wrap(n.xy); }");
		node_shader_write_frag(kong, "output[0] = float4(n.xy, roughness, pack_f32_i16(metallic, uint(float(int(matid * 255.0)) % float(3))));");
	}

	node_shader_write_frag(kong, "output[2] = float4(0.0, 0.0, input.tex_coord.xy);");

	parser_material_finalize(con_mesh);
	con_mesh.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader), ADDRESS(con_mesh.data._.vertex_shader_size), ADDRESS(con_mesh.data._.fragment_shader_size));
	return con_mesh;
}

function make_mesh_get_max_textures(): i32 {
	return 16 - 3; // G4onG5/G4.c.h MAX_TEXTURES
}
