
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
		depth_attachment: "DEPTH32"
	};

	let con_mesh: node_shader_context_t = node_shader_context_create(data, props);

	let vert: node_shader_t = node_shader_context_make_vert(con_mesh);
	let frag: node_shader_t = node_shader_context_make_frag(con_mesh);
	frag.ins = vert.outs;

	node_shader_add_out(vert, "vec2 tex_coord");
	frag.wvpposition = true;
	node_shader_add_out(vert, "vec4 prevwvpposition");
	node_shader_add_uniform(vert, "mat4 VP", "_view_proj_matrix");
	node_shader_add_uniform(vert, "mat4 prevWVP", "_prev_world_view_proj_matrix");
	vert.wposition = true;

	let texture_count: i32 = 0;

	node_shader_add_uniform(vert, "mat4 WVP", "_world_view_proj_matrix");
	let lid: i32 = project_layers[0].id;
	node_shader_add_uniform(vert, "sampler2D texpaint_vert", "_texpaint_vert" + lid);
	node_shader_write(vert, "vec3 meshpos = texelFetch(texpaint_vert, ivec2(gl_VertexID % textureSize(texpaint_vert, 0).x, gl_VertexID / textureSize(texpaint_vert, 0).y), 0).xyz;");
	// + pos.xyz * 0.000001
	node_shader_write(vert, "gl_Position = mul(vec4(meshpos.xyz, 1.0), WVP);");

	node_shader_write(vert, "tex_coord = vec2(0.0, 0.0);");

	node_shader_write(vert, "prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);");

	node_shader_add_out(frag, "vec4 frag_color[3]");

	node_shader_add_uniform(vert, "mat3 N", "_normal_matrix");
	node_shader_add_out(vert, "vec3 wnormal");

	node_shader_write_attrib(vert, "int base_vertex0 = gl_VertexID - (gl_VertexID % 3);");
	node_shader_write_attrib(vert, "int base_vertex1 = base_vertex0 + 1;");
	node_shader_write_attrib(vert, "int base_vertex2 = base_vertex0 + 2;");
	node_shader_write_attrib(vert, "vec3 meshpos0 = texelFetch(texpaint_vert, ivec2(base_vertex0 % textureSize(texpaint_vert, 0).x, base_vertex0 / textureSize(texpaint_vert, 0).y), 0).xyz;");
	node_shader_write_attrib(vert, "vec3 meshpos1 = texelFetch(texpaint_vert, ivec2(base_vertex1 % textureSize(texpaint_vert, 0).x, base_vertex1 / textureSize(texpaint_vert, 0).y), 0).xyz;");
	node_shader_write_attrib(vert, "vec3 meshpos2 = texelFetch(texpaint_vert, ivec2(base_vertex2 % textureSize(texpaint_vert, 0).x, base_vertex2 / textureSize(texpaint_vert, 0).y), 0).xyz;");
	node_shader_write_attrib(vert, "vec3 meshnor = normalize(cross(meshpos2 - meshpos1, meshpos0 - meshpos1));");
	node_shader_write_attrib(vert, "wnormal = mul(meshnor, N);");
	node_shader_write_attrib(frag, "vec3 n = normalize(wnormal);");

	node_shader_add_function(frag, str_pack_float_int16);
	node_shader_add_function(frag, str_octahedron_wrap);
	node_shader_add_function(frag, str_cotangent_frame);
	if (layer_pass > 0) {
		node_shader_add_uniform(frag, "sampler2D gbuffer0");
		node_shader_add_uniform(frag, "sampler2D gbuffer1");
		node_shader_add_uniform(frag, "sampler2D gbuffer2");
		node_shader_write(frag, "vec2 fragcoord = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;");
		node_shader_write(frag, "fragcoord.y = 1.0 - fragcoord.y;");
		node_shader_write(frag, "vec4 gbuffer0_sample = textureLod(gbuffer0, fragcoord, 0.0);");
		node_shader_write(frag, "vec4 gbuffer1_sample = textureLod(gbuffer1, fragcoord, 0.0);");
		node_shader_write(frag, "vec4 gbuffer2_sample = textureLod(gbuffer2, fragcoord, 0.0);");
		node_shader_write(frag, "vec3 basecol = gbuffer0_sample.rgb;");
		node_shader_write(frag, "float roughness = gbuffer2_sample.g;");
		node_shader_write(frag, "float metallic = gbuffer2_sample.b;");
		node_shader_write(frag, "float occlusion = gbuffer2_sample.r;");
		node_shader_write(frag, "float opacity = 1.0;//gbuffer0_sample.a;");
		node_shader_write(frag, "float matid = gbuffer1_sample.a;");
		node_shader_write(frag, "vec3 ntex = gbuffer1_sample.rgb;");
		node_shader_write(frag, "float height = gbuffer2_sample.a;");
	}
	else {
		node_shader_write(frag, "vec3 basecol = vec3(0.0, 0.0, 0.0);");
		node_shader_write(frag, "float roughness = 0.3;");
		node_shader_write(frag, "float metallic = 0.0;");
		node_shader_write(frag, "float occlusion = 1.0;");
		node_shader_write(frag, "float opacity = 1.0;");
		node_shader_write(frag, "float matid = 0.0;");
		node_shader_write(frag, "vec3 ntex = vec3(0.5, 0.5, 1.0);");
		node_shader_write(frag, "float height = 0.0;");
	}
	node_shader_write(frag, "vec4 texpaint_sample = vec4(0.0, 0.0, 0.0, 1.0);");
	node_shader_write(frag, "vec4 texpaint_nor_sample;");
	node_shader_write(frag, "vec4 texpaint_pack_sample;");
	node_shader_write(frag, "float texpaint_opac;");

	if (make_material_height_used) {
		node_shader_write(frag, "float height0 = 0.0;");
		node_shader_write(frag, "float height1 = 0.0;");
		node_shader_write(frag, "float height2 = 0.0;");
		node_shader_write(frag, "float height3 = 0.0;");
	}

	if (context_raw.draw_wireframe) {
		texture_count++;
		node_shader_add_uniform(frag, "sampler2D texuvmap", "_texuvmap");
	}

	if (context_raw.viewport_mode == viewport_mode_t.LIT && context_raw.render_mode == render_mode_t.FORWARD) {
		texture_count += 4;
		node_shader_add_uniform(frag, "sampler2D senvmap_brdf", "$brdf.k");
		node_shader_add_uniform(frag, "sampler2D senvmap_radiance", "_envmap_radiance");
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
			node_shader_add_uniform(frag, "int uid", "_uid");
			if (slot_layer_get_object_mask(l) > project_paint_objects.length) { // Atlas
				let visibles: mesh_object_t[] = project_get_atlas_objects(slot_layer_get_object_mask(l));
				node_shader_write(frag, "if (");
				for (let i: i32 = 0; i < visibles.length; ++i) {
					if (i > 0) {
						node_shader_write(frag, " || ");
					}
					let uid: i32 = visibles[i].base.uid;
					node_shader_write(frag, uid + " == uid");
				}
				node_shader_write(frag, ") {");
			}
			else { // Object mask
				let uid: i32 = project_paint_objects[slot_layer_get_object_mask(l) - 1].base.uid;
				node_shader_write(frag, "if (" + uid + " == uid) {");
			}
		}

		node_shader_add_shared_sampler(frag, "sampler2D texpaint" + l.id);
		node_shader_write(frag, "texpaint_sample = vec4(0.8, 0.8, 0.8, 1.0);");
		node_shader_write(frag, "texpaint_opac = texpaint_sample.a;");

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
				node_shader_write(frag, "float " + texpaint_mask + " = 0.0;");
				for (let i: i32 = 0; i < masks.length; ++i) {
					let m: slot_layer_t = masks[i];
					if (!slot_layer_is_visible(m)) continue;
					node_shader_add_shared_sampler(frag, "sampler2D texpaint" + m.id);
					node_shader_write(frag, "{"); // Group mask is sampled across multiple layers
					node_shader_write(frag, "float texpaint_mask_sample" + m.id + " = textureLodShared(texpaint" + m.id + ", tex_coord, 0.0).r;");
					node_shader_write(frag, "}");
				}
				node_shader_write(frag, "texpaint_opac *= clamp(" + texpaint_mask + ", 0.0, 1.0);");
			}
		}

		if (slot_layer_get_opacity(l) < 1) {
			let opac: f32 = slot_layer_get_opacity(l);
			node_shader_write(frag, "texpaint_opac *= " + opac + ";");
		}

		if (l == project_layers[0]) {
			node_shader_write(frag, "basecol = vec3(0.8, 0.8, 0.8);// texpaint_sample.rgb * texpaint_opac;");
		}

		if (slot_layer_get_object_mask(l) > 0) {
			node_shader_write(frag, "}");
		}

		if (last_pass && context_raw.draw_texels) {
			node_shader_add_uniform(frag, "vec2 texpaint_size", "_texpaint_size");
			node_shader_write(frag, "vec2 texel0 = tex_coord * texpaint_size * 0.01;");
			node_shader_write(frag, "vec2 texel1 = tex_coord * texpaint_size * 0.1;");
			node_shader_write(frag, "vec2 texel2 = tex_coord * texpaint_size;");
			node_shader_write(frag, "basecol *= max(float(mod(int(texel0.x), 2.0) == mod(int(texel0.y), 2.0)), 0.9);");
			node_shader_write(frag, "basecol *= max(float(mod(int(texel1.x), 2.0) == mod(int(texel1.y), 2.0)), 0.9);");
			node_shader_write(frag, "basecol *= max(float(mod(int(texel2.x), 2.0) == mod(int(texel2.y), 2.0)), 0.9);");
		}

		if (last_pass && context_raw.draw_wireframe) {
			node_shader_write(frag, "basecol *= 1.0 - textureLod(texuvmap, tex_coord, 0.0).r;");
		}

		if (make_material_height_used) {
			node_shader_write(frag, "if (height > 0.0) {");
			node_shader_write(frag, "float height_dx = height0 - height1;");
			node_shader_write(frag, "float height_dy = height2 - height3;");
			// Whiteout blend
			node_shader_write(frag, "vec3 n1 = ntex * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);");
			node_shader_write(frag, "vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));");
			node_shader_write(frag, "ntex = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);");
			node_shader_write(frag, "}");
		}

		if (!last_pass) {
			node_shader_write(frag, "frag_color[0] = vec4(basecol, opacity);");
			node_shader_write(frag, "frag_color[1] = vec4(ntex, matid);");
			node_shader_write(frag, "frag_color[2] = vec4(occlusion, roughness, metallic, height);");
			parser_material_finalize(con_mesh);
			con_mesh.data.shader_from_source = true;
			con_mesh.data.vertex_shader = node_shader_get(vert);
			con_mesh.data.fragment_shader = node_shader_get(frag);
			return con_mesh;
		}

		frag.vvec = true;
		node_shader_write(frag, "mat3 TBN = cotangent_frame(n, vvec, tex_coord);");
		node_shader_write(frag, "n = ntex * 2.0 - 1.0;");
		node_shader_write(frag, "n.y = -n.y;");
		node_shader_write(frag, "n = normalize(mul(n, TBN));");

		if (context_raw.viewport_mode == viewport_mode_t.LIT) {

			node_shader_write(frag, "basecol = pow(basecol, vec3(2.2, 2.2, 2.2));");

			if (context_raw.viewport_shader != null) {
				node_shader_write(frag, "vec3 output_color;");
				js_call_ptr(context_raw.viewport_shader, frag);
				node_shader_write(frag, "frag_color[1] = vec4(output_color, 1.0);");
			}
			else if (context_raw.render_mode == render_mode_t.FORWARD) {
				frag.wposition = true;
				node_shader_write(frag, "vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);");
				node_shader_write(frag, "vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);");
				frag.vvec = true;
				node_shader_write(frag, "float dotnv = max(0.0, dot(n, vvec));");
				node_shader_write(frag, "vec2 env_brdf = texelFetch(senvmap_brdf, ivec2(vec2(roughness, 1.0 - dotnv) * 256.0), 0).xy;");
				node_shader_add_uniform(frag, "int envmap_num_mipmaps", "_envmap_num_mipmaps");
				node_shader_add_uniform(frag, "vec4 envmap_data", "_envmap_data"); // angle, sin(angle), cos(angle), strength
				node_shader_write(frag, "vec3 wreflect = reflect(-vvec, n);");
				node_shader_write(frag, "float envlod = roughness * float(envmap_num_mipmaps);");
				node_shader_add_function(frag, str_envmap_equirect);
				node_shader_write(frag, "vec3 prefiltered_color = textureLod(senvmap_radiance, envmap_equirect(wreflect, envmap_data.x), envlod).rgb;");
				node_shader_add_uniform(frag, "vec4 shirr[7]", "_envmap_irradiance");
				node_shader_add_function(frag, str_sh_irradiance());
				node_shader_write(frag, "vec3 indirect = albedo * (sh_irradiance(vec3(n.x * envmap_data.z - n.y * envmap_data.y, n.x * envmap_data.y + n.y * envmap_data.z, n.z), shirr) / 3.14159265);");
				node_shader_write(frag, "indirect += prefiltered_color * (f0 * env_brdf.x + env_brdf.y) * 1.5;");
				node_shader_write(frag, "indirect *= envmap_data.w * occlusion;");
				node_shader_write(frag, "frag_color[1] = vec4(indirect, 1.0);");
			}
			else { // Deferred, Pathtraced
				if (make_material_emis_used) {
					node_shader_write(frag, "if (int(matid * 255.0) % 3 == 1) basecol *= 10.0;"); // Boost for bloom
				}
				node_shader_write(frag, "frag_color[1] = vec4(basecol, occlusion);");
			}
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_NORMAL) {
			frag.nattr = true;
			node_shader_write(frag, "frag_color[1] = vec4(nAttr, 1.0);");
		}
		else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_ID) {
			node_shader_add_uniform(frag, "float object_id", "_object_id");
			node_shader_write(frag, "float obid = object_id + 1.0 / 255.0;");
			node_shader_write(frag, "float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write(frag, "float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write(frag, "float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);");
			node_shader_write(frag, "frag_color[1] = vec4(id_r, id_g, id_b, 1.0);");
		}
		else {
			node_shader_write(frag, "frag_color[1] = vec4(1.0, 0.0, 1.0, 1.0);"); // Pink
		}

		if (context_raw.viewport_mode != viewport_mode_t.LIT && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
			node_shader_write(frag, "frag_color[1].rgb = pow(frag_color[1].rgb, vec3(2.2, 2.2, 2.2));");
		}

		node_shader_write(frag, "n /= (abs(n.x) + abs(n.y) + abs(n.z));");
		node_shader_write(frag, "n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);");
		node_shader_write(frag, "frag_color[0] = vec4(n.xy, roughness, pack_f32_i16(metallic, uint(int(matid * 255.0) % 3)));");
	}

	node_shader_write(frag, "vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;");
	node_shader_write(frag, "vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;");
	node_shader_write(frag, "frag_color[2] = vec4(posa - posb, tex_coord.xy);");

	parser_material_finalize(con_mesh);
	con_mesh.data.shader_from_source = true;
	con_mesh.data.vertex_shader = node_shader_get(vert);
	con_mesh.data.fragment_shader = node_shader_get(frag);
	return con_mesh;
}

function make_mesh_get_max_textures(): i32 {
	return 16 - 3; // G4onG5/G4.c.h MAX_TEXTURES
}
