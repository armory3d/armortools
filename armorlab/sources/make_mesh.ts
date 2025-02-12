
let make_mesh_layer_pass_count: i32 = 1;

function make_mesh_run(data: material_t, layer_pass: i32 = 0): node_shader_context_t {

	let depth_write: bool = layer_pass == 0 ? true : false;
	let compare_mode: string = layer_pass == 0 ? "less" : "equal";
	let cull_mode: string = (context_raw.cull_backfaces || layer_pass > 0) ? "clockwise" : "none";

	let props: shader_context_t = {
		name: "mesh",
		depth_write: depth_write,
		compare_mode: compare_mode,
		cull_mode: cull_mode,
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
	let displace_strength: f32 = make_material_get_displace_strength();
	if (make_material_height_used && displace_strength > 0.0) {
		vert.n = true;
		node_shader_write(vert, "float height = 0.0;");
		let num_layers: i32 = 1;
		let displace_3: string = displace_strength + ", " + displace_strength + ", " + displace_strength;
		node_shader_write(vert, "wposition += wnormal * vec3(height, height, height) * vec3(" + displace_3 + ");");
	}

	node_shader_write(vert, "gl_Position = mul(vec4(wposition.xyz, 1.0), VP);");
	let brush_scale: f32 = context_raw.brush_scale;
	node_shader_add_uniform(vert, "float tex_scale", "_tex_unpack");
	node_shader_write(vert, "tex_coord = tex * " + brush_scale + " * tex_scale;");
	if (make_material_height_used && displace_strength > 0) {
		node_shader_add_uniform(vert, "mat4 invW", "_inv_world_matrix");
		node_shader_write(vert, "prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);");
	}
	else {
		node_shader_write(vert, "prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);");
	}

	node_shader_add_out(frag, "vec4 frag_color[3]");
	frag.n = true;
	node_shader_add_function(frag, str_pack_float_int16);
	node_shader_add_function(frag, str_octahedron_wrap);
	node_shader_add_function(frag, str_cotangent_frame);

	node_shader_write(frag, "vec3 basecol = vec3(0.0, 0.0, 0.0);");
	node_shader_write(frag, "float roughness = 0.0;");
	node_shader_write(frag, "float metallic = 0.0;");
	node_shader_write(frag, "float occlusion = 1.0;");
	node_shader_write(frag, "float opacity = 1.0;");
	node_shader_write(frag, "float matid = 0.0;");
	node_shader_write(frag, "vec3 ntex = vec3(0.5, 0.5, 1.0);");
	node_shader_write(frag, "float height = 0.0;");

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

	if (context_raw.viewport_mode == viewport_mode_t.LIT && context_raw.render_mode == render_mode_t.FORWARD) {
		node_shader_add_uniform(frag, "sampler2D senvmap_brdf", "$brdf.k");
		node_shader_add_uniform(frag, "sampler2D senvmap_radiance", "_envmap_radiance");
	}

	node_shader_add_shared_sampler(frag, "sampler2D texpaint");
	node_shader_write(frag, "texpaint_sample = textureLodShared(texpaint" + ", tex_coord, 0.0);");
	node_shader_write(frag, "texpaint_opac = texpaint_sample.a;");

	node_shader_write(frag, "basecol = texpaint_sample.rgb * texpaint_opac;");

	node_shader_add_shared_sampler(frag, "sampler2D texpaint_nor");
	node_shader_write(frag, "texpaint_nor_sample = textureLodShared(texpaint_nor" + ", tex_coord, 0.0);");

	node_shader_write(frag, "ntex = mix(ntex, texpaint_nor_sample.rgb, texpaint_opac);");

	node_shader_add_shared_sampler(frag, "sampler2D texpaint_pack");
	node_shader_write(frag, "texpaint_pack_sample = textureLodShared(texpaint_pack" + ", tex_coord, 0.0);");

	node_shader_write(frag, "occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);");

	node_shader_write(frag, "roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);");

	node_shader_write(frag, "metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);");

	node_shader_write(frag, "height = texpaint_pack_sample.a * texpaint_opac;");

	// if (l.paint_height && height_used) {
	// 	let assign: string = l.paint_height_blend ? "+=" : "=";
	// 	node_shader_write(frag, "height " + assign + " texpaint_pack_sample.a * texpaint_opac;");
	// 	node_shader_write(frag, "{");
	// 	node_shader_add_uniform(frag, "vec2 texpaint_size", "_texpaint_size");
	// 	node_shader_write(frag, "float tex_step = 1.0 / texpaint_size.x;");
	// 	node_shader_write(frag, "height0 " + assign + " textureLodShared(texpaint_pack" + ", vec2(tex_coord.x - tex_step, tex_coord.y), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "height1 " + assign + " textureLodShared(texpaint_pack" + ", vec2(tex_coord.x + tex_step, tex_coord.y), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "height2 " + assign + " textureLodShared(texpaint_pack" + ", vec2(tex_coord.x, tex_coord.y - tex_step), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "height3 " + assign + " textureLodShared(texpaint_pack" + ", vec2(tex_coord.x, tex_coord.y + tex_step), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "}");
	// }

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

	frag.vvec = true;
	node_shader_write(frag, "mat3 TBN = cotangent_frame(n, vvec, tex_coord);");
	node_shader_write(frag, "n = ntex * 2.0 - 1.0;");
	node_shader_write(frag, "n.y = -n.y;");
	node_shader_write(frag, "n = normalize(mul(n, TBN));");

	if (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {

		node_shader_write(frag, "basecol = pow(basecol, vec3(2.2, 2.2, 2.2));");

		if (context_raw.viewport_shader != null) {
			node_shader_write(frag, "vec3 output_color;");
			js_call_ptr(context_raw.viewport_shader, frag);
			node_shader_write(frag, "frag_color[1] = vec4(output_color, 1.0);");
		}
		else if (context_raw.render_mode == render_mode_t.FORWARD && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
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
			node_shader_write(frag, "frag_color[1] = vec4(basecol, occlusion);");
		}
	}
	else if (context_raw.viewport_mode == viewport_mode_t.BASE_COLOR) {
		node_shader_write(frag, "frag_color[1] = vec4(basecol, 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.NORMAL_MAP) {
		node_shader_write(frag, "frag_color[1] = vec4(ntex.rgb, 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.OCCLUSION) {
		node_shader_write(frag, "frag_color[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.ROUGHNESS) {
		node_shader_write(frag, "frag_color[1] = vec4(vec3(roughness, roughness, roughness), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.METALLIC) {
		node_shader_write(frag, "frag_color[1] = vec4(vec3(metallic, metallic, metallic), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.OPACITY) {
		node_shader_write(frag, "frag_color[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.HEIGHT) {
		node_shader_write(frag, "frag_color[1] = vec4(vec3(height, height, height), 1.0);");
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

	node_shader_write(frag, "vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;");
	node_shader_write(frag, "vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;");
	node_shader_write(frag, "frag_color[2] = vec4(posa - posb, tex_coord.xy);");

	parser_material_finalize(con_mesh);
	con_mesh.data.shader_from_source = true;
	con_mesh.data.vertex_shader = node_shader_get(vert);
	con_mesh.data.fragment_shader = node_shader_get(frag);
	return con_mesh;
}
