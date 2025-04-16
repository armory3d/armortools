
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
		let num_layers: i32 = 1;
		let displace_3: string = displace_strength + ", " + displace_strength + ", " + displace_strength;
		node_shader_write_vert(kong, "\output.wposition += wnormal * float3(height, height, height) * float3(" + displace_3 + ");");
	}

	node_shader_write_vert(kong, "output.pos = constants.VP * float4(\output.wposition.xyz, 1.0);");
	let brush_scale: f32 = context_raw.brush_scale;
	node_shader_add_constant(kong, "tex_scale: float", "_tex_unpack");
	node_shader_write_vert(kong, "output.tex_coord = input.tex * " + brush_scale + " * constants.tex_scale;");

	kong.frag_out = "float4[3]";
	kong.frag_n = true;
	node_shader_add_function(kong, str_pack_float_int16);
	node_shader_add_function(kong, str_octahedron_wrap);
	node_shader_add_function(kong, str_cotangent_frame);

	node_shader_write_frag(kong, "var basecol: float3 = float3(0.0, 0.0, 0.0);");
	node_shader_write_frag(kong, "var roughness: float = 0.0;");
	node_shader_write_frag(kong, "var metallic: float = 0.0;");
	node_shader_write_frag(kong, "var occlusion: float = 1.0;");
	node_shader_write_frag(kong, "var opacity: float = 1.0;");
	node_shader_write_frag(kong, "var matid: float = 0.0;");
	node_shader_write_frag(kong, "var ntex: float3 = float3(0.5, 0.5, 1.0);");
	node_shader_write_frag(kong, "var height: float = 0.0;");

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

	if (context_raw.viewport_mode == viewport_mode_t.LIT && context_raw.render_mode == render_mode_t.FORWARD) {
		node_shader_add_texture(kong, "senvmap_brdf", "$brdf.k");
		node_shader_add_texture(kong, "senvmap_radiance", "_envmap_radiance");
	}

	node_shader_add_shared_sampler(kong, "texpaint");
	node_shader_write_frag(kong, "texpaint_sample = sample_lod(texpaint, shared_sampler, input.tex_coord, 0.0);");
	node_shader_write_frag(kong, "texpaint_opac = texpaint_sample.a;");

	node_shader_write_frag(kong, "basecol = texpaint_sample.rgb * texpaint_opac;");
	node_shader_add_shared_sampler(kong, "texpaint_nor");
	node_shader_write_frag(kong, "texpaint_nor_sample = sample_lod(texpaint_nor, shared_sampler, input.tex_coord, 0.0);");
	node_shader_write_frag(kong, "ntex = lerp3(ntex, texpaint_nor_sample.rgb, texpaint_opac);");
	node_shader_add_shared_sampler(kong, "texpaint_pack");
	node_shader_write_frag(kong, "texpaint_pack_sample = sample_lod(texpaint_pack, shared_sampler, input.tex_coord, 0.0);");
	node_shader_write_frag(kong, "occlusion = lerp(occlusion, texpaint_pack_sample.r, texpaint_opac);");
	node_shader_write_frag(kong, "roughness = lerp(roughness, texpaint_pack_sample.g, texpaint_opac);");
	node_shader_write_frag(kong, "metallic = lerp(metallic, texpaint_pack_sample.b, texpaint_opac);");
	node_shader_write_frag(kong, "height = texpaint_pack_sample.a * texpaint_opac;");

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

	kong.frag_vvec = true;
	node_shader_write_frag(kong, "var TBN: float3x3 = cotangent_frame(n, vvec, input.tex_coord);");
	node_shader_write_frag(kong, "n = ntex * 2.0 - 1.0;");
	node_shader_write_frag(kong, "n.y = -n.y;");
	node_shader_write_frag(kong, "n = normalize(TBN * n);");

	if (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {

		node_shader_write_frag(kong, "basecol = pow3(basecol, float3(2.2, 2.2, 2.2));");

		if (context_raw.viewport_shader != null) {
			node_shader_write_frag(kong, "output_color: float3;");
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
			node_shader_add_function(frag, str_envmap_equirect);
			node_shader_write_frag(kong, "var prefiltered_color: float3 = sample_lod(senvmap_radiance, senvmap_radiance_sampler, envmap_equirect(wreflect, constants.envmap_data.x), envlod).rgb;");
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
			node_shader_write_frag(kong, "output[1] = float4(basecol, occlusion);");
		}
	}
	else if (context_raw.viewport_mode == viewport_mode_t.BASE_COLOR) {
		node_shader_write_frag(kong, "output[1] = float4(basecol, 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.NORMAL_MAP) {
		node_shader_write_frag(kong, "output[1] = float4(ntex.rgb, 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.OCCLUSION) {
		node_shader_write_frag(kong, "output[1] = float4(float3(occlusion, occlusion, occlusion), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.ROUGHNESS) {
		node_shader_write_frag(kong, "output[1] = float4(float3(roughness, roughness, roughness), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.METALLIC) {
		node_shader_write_frag(kong, "output[1] = float4(float3(metallic, metallic, metallic), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.OPACITY) {
		node_shader_write_frag(kong, "output[1] = float4(float3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.HEIGHT) {
		node_shader_write_frag(kong, "output[1] = float4(float3(height, height, height), 1.0);");
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
	node_shader_write_frag(kong, "output[0] = float4(n.xy, roughness, pack_f32_i16(metallic, uint(int(matid * 255.0) % 3)));");
	node_shader_write_frag(kong, "output[2] = float4(0.0, 0.0, input.tex_coord.xy);");

	parser_material_finalize(con_mesh);
	con_mesh.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader));
	return con_mesh;
}
