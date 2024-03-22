
let make_mesh_layer_pass_count: i32 = 1;

function make_mesh_run(data: material_t, layer_pass: i32 = 0): NodeShaderContextRaw {
	let con_mesh = node_shader_context_create(data, {
		name: "mesh",
		depth_write: layer_pass == 0 ? true : false,
		compare_mode: layer_pass == 0 ? "less" : "equal",
		cull_mode: (context_raw.cull_backfaces || layer_pass > 0) ? "clockwise" : "none",
		vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
		color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
		depth_attachment: "DEPTH32"
	});

	let vert = node_shader_context_make_vert(con_mesh);
	let frag = node_shader_context_make_frag(con_mesh);
	frag.ins = vert.outs;

	node_shader_add_out(vert, "vec2 texCoord");
	frag.wvpposition = true;
	node_shader_add_out(vert, "vec4 prevwvpposition");
	node_shader_add_uniform(vert, "mat4 VP", "_view_proj_matrix");
	node_shader_add_uniform(vert, "mat4 prevWVP", "_prev_world_view_proj_matrix");
	vert.wposition = true;

	let texture_count = 0;
	let displace_strength = make_material_get_displace_strength();
	if (make_material_height_used && displace_strength > 0.0) {
		vert.n = true;
		node_shader_write(vert, "float height = 0.0;");
		let num_layers = 1;
		node_shader_write(vert, "wposition += wnormal * vec3(height, height, height) * vec3(" + displace_strength +", " + displace_strength + ", " + displace_strength + ");");
	}

	node_shader_write(vert, "gl_Position = mul(vec4(wposition.xyz, 1.0), VP);");
	let brush_scale = context_raw.brush_scale;
	node_shader_add_uniform(vert, "float texScale", "_tex_unpack");
	node_shader_write(vert, "texCoord = tex * " + brush_scale + " * texScale;");
	if (make_material_height_used && displace_strength > 0) {
		node_shader_add_uniform(vert, "mat4 invW", "_inv_world_matrix");
		node_shader_write(vert, "prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);");
	}
	else {
		node_shader_write(vert, "prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);");
	}

	node_shader_add_out(frag, "vec4 fragColor[3]");
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
		node_shader_add_uniform(frag, "sampler2D senvmapBrdf", "$brdf.k");
		node_shader_add_uniform(frag, "sampler2D senvmapRadiance", "_envmap_radiance");
		node_shader_add_uniform(frag, "sampler2D sltcMat", "_ltcMat");
		node_shader_add_uniform(frag, "sampler2D sltcMag", "_ltcMag");
	}

	node_shader_add_shared_sampler(frag, "sampler2D texpaint");
	node_shader_write(frag, "texpaint_sample = textureLodShared(texpaint" + ", texCoord, 0.0);");
	node_shader_write(frag, "texpaint_opac = texpaint_sample.a;");

	node_shader_write(frag, "basecol = texpaint_sample.rgb * texpaint_opac;");

	node_shader_add_shared_sampler(frag, "sampler2D texpaint_nor");
	node_shader_write(frag, "texpaint_nor_sample = textureLodShared(texpaint_nor" + ", texCoord, 0.0);");

	node_shader_write(frag, "ntex = mix(ntex, texpaint_nor_sample.rgb, texpaint_opac);");

	node_shader_add_shared_sampler(frag, "sampler2D texpaint_pack");
	node_shader_write(frag, "texpaint_pack_sample = textureLodShared(texpaint_pack" + ", texCoord, 0.0);");

	node_shader_write(frag, "occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);");

	node_shader_write(frag, "roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);");

	node_shader_write(frag, "metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);");

	node_shader_write(frag, "height = texpaint_pack_sample.a * texpaint_opac;");

	// if (l.paintHeight && heightUsed) {
	// 	let assign = l.paintHeightBlend ? "+=" : "=";
	// 	node_shader_write(frag, "height " + assign + " texpaint_pack_sample.a * texpaint_opac;");
	// 	node_shader_write(frag, "{");
	// 	node_shader_add_uniform(frag, "vec2 texpaintSize", "_texpaintSize");
	// 	node_shader_write(frag, "float tex_step = 1.0 / texpaintSize.x;");
	// 	node_shader_write(frag, "height0 " + assign + " textureLodShared(texpaint_pack" + ", vec2(texCoord.x - tex_step, texCoord.y), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "height1 " + assign + " textureLodShared(texpaint_pack" + ", vec2(texCoord.x + tex_step, texCoord.y), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "height2 " + assign + " textureLodShared(texpaint_pack" + ", vec2(texCoord.x, texCoord.y - tex_step), 0.0).a * texpaint_opac;");
	// 	node_shader_write(frag, "height3 " + assign + " textureLodShared(texpaint_pack" + ", vec2(texCoord.x, texCoord.y + tex_step), 0.0).a * texpaint_opac;");
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
	///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
	node_shader_write(frag, "mat3 TBN = cotangentFrame(n, vVec, texCoord);");
	///else
	node_shader_write(frag, "mat3 TBN = cotangentFrame(n, -vVec, texCoord);");
	///end
	node_shader_write(frag, "n = ntex * 2.0 - 1.0;");
	node_shader_write(frag, "n.y = -n.y;");
	node_shader_write(frag, "n = normalize(mul(n, TBN));");

	if (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {

		node_shader_write(frag, "basecol = pow(basecol, vec3(2.2, 2.2, 2.2));");

		if (context_raw.viewport_shader != null) {
			let color = context_raw.viewport_shader(frag);
			node_shader_write(frag, "fragColor[1] = vec4(" + color + ", 1.0);");
		}
		else if (context_raw.render_mode == render_mode_t.FORWARD && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
			frag.wposition = true;
			node_shader_write(frag, "vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);");
			node_shader_write(frag, "vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);");
			frag.vvec = true;
			node_shader_write(frag, "float dotNV = max(0.0, dot(n, vVec));");
			node_shader_write(frag, "vec2 envBRDF = texelFetch(senvmapBrdf, ivec2(vec2(roughness, 1.0 - dotNV) * 256.0), 0).xy;");
			node_shader_add_uniform(frag, "int envmapNumMipmaps", "_envmap_num_mipmaps");
			node_shader_add_uniform(frag, "vec4 envmapData", "_envmapData"); // angle, sin(angle), cos(angle), strength
			node_shader_write(frag, "vec3 wreflect = reflect(-vVec, n);");
			node_shader_write(frag, "float envlod = roughness * float(envmapNumMipmaps);");
			node_shader_add_function(frag, str_envmap_equirect);
			node_shader_write(frag, "vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(wreflect, envmapData.x), envlod).rgb;");
			node_shader_add_uniform(frag, "vec3 lightArea0", "_light_area0");
			node_shader_add_uniform(frag, "vec3 lightArea1", "_light_area1");
			node_shader_add_uniform(frag, "vec3 lightArea2", "_light_area2");
			node_shader_add_uniform(frag, "vec3 lightArea3", "_light_area3");
			node_shader_add_function(frag, str_ltc_evaluate);
			node_shader_add_uniform(frag, "vec3 lightPos", "_point_pos");
			node_shader_add_uniform(frag, "vec3 lightColor", "_point_color");
			node_shader_write(frag, "float ldist = distance(wposition, lightPos);");
			node_shader_write(frag, "const float LUT_SIZE = 64.0;");
			node_shader_write(frag, "const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;");
			node_shader_write(frag, "const float LUT_BIAS = 0.5 / LUT_SIZE;");
			node_shader_write(frag, "float theta = acos(dotNV);");
			node_shader_write(frag, "vec2 tuv = vec2(roughness, theta / (0.5 * 3.14159265));");
			node_shader_write(frag, "tuv = tuv * LUT_SCALE + LUT_BIAS;");
			node_shader_write(frag, "vec4 t = textureLod(sltcMat, tuv, 0.0);");
			node_shader_write(frag, "mat3 minv = mat3(vec3(1.0, 0.0, t.y), vec3(0.0, t.z, 0.0), vec3(t.w, 0.0, t.x));");
			node_shader_write(frag, "float ltcspec = ltcEvaluate(n, vVec, dotNV, wposition, minv, lightArea0, lightArea1, lightArea2, lightArea3);");
			node_shader_write(frag, "ltcspec *= textureLod(sltcMag, tuv, 0.0).a;");
			node_shader_write(frag, "mat3 mident = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);");
			node_shader_write(frag, "float ltcdiff = ltcEvaluate(n, vVec, dotNV, wposition, mident, lightArea0, lightArea1, lightArea2, lightArea3);");
			node_shader_write(frag, "vec3 direct = albedo * ltcdiff + ltcspec * 0.05;");
			node_shader_write(frag, "direct *= lightColor * (1.0 / (ldist * ldist));");

			node_shader_add_uniform(frag, "vec4 shirr[7]", "_envmap_irradiance");
			node_shader_add_function(frag, str_sh_irradiance());
			node_shader_write(frag, "vec3 indirect = albedo * (shIrradiance(vec3(n.x * envmapData.z - n.y * envmapData.y, n.x * envmapData.y + n.y * envmapData.z, n.z), shirr) / 3.14159265);");
			node_shader_write(frag, "indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;");
			node_shader_write(frag, "indirect *= envmapData.w * occlusion;");
			node_shader_write(frag, "fragColor[1] = vec4(direct + indirect, 1.0);");
		}
		else { // Deferred, Pathtraced
			node_shader_write(frag, "fragColor[1] = vec4(basecol, occlusion);");
		}
	}
	else if (context_raw.viewport_mode == viewport_mode_t.BASE_COLOR) {
		node_shader_write(frag, "fragColor[1] = vec4(basecol, 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.NORMAL_MAP) {
		node_shader_write(frag, "fragColor[1] = vec4(ntex.rgb, 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.OCCLUSION) {
		node_shader_write(frag, "fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.ROUGHNESS) {
		node_shader_write(frag, "fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.METALLIC) {
		node_shader_write(frag, "fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.OPACITY) {
		node_shader_write(frag, "fragColor[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);");
	}
	else if (context_raw.viewport_mode == viewport_mode_t.HEIGHT) {
		node_shader_write(frag, "fragColor[1] = vec4(vec3(height, height, height), 1.0);");
	}
	else {
		node_shader_write(frag, "fragColor[1] = vec4(1.0, 0.0, 1.0, 1.0);"); // Pink
	}

	if (context_raw.viewport_mode != viewport_mode_t.LIT && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
		node_shader_write(frag, "fragColor[1].rgb = pow(fragColor[1].rgb, vec3(2.2, 2.2, 2.2));");
	}

	node_shader_write(frag, "n /= (abs(n.x) + abs(n.y) + abs(n.z));");
	node_shader_write(frag, "n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);");
	node_shader_write(frag, "fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(int(matid * 255.0) % 3)));");

	node_shader_write(frag, "vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;");
	node_shader_write(frag, "vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;");
	node_shader_write(frag, "fragColor[2] = vec4(posa - posb, texCoord.xy);");

	parser_material_finalize(con_mesh);
	con_mesh.data.shader_from_source = true;
	con_mesh.data.vertex_shader = node_shader_get(vert);
	con_mesh.data.fragment_shader = node_shader_get(frag);
	return con_mesh;
}
