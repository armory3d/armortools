
class MakeMesh {

	static layerPassCount = 1;

	static run = (data: TMaterial, layerPass = 0): NodeShaderContextRaw => {
		let con_mesh = NodeShaderContext.create(data, {
			name: "mesh",
			depth_write: layerPass == 0 ? true : false,
			compare_mode: layerPass == 0 ? "less" : "equal",
			cull_mode: (Context.raw.cullBackfaces || layerPass > 0) ? "clockwise" : "none",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
			color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
			depth_attachment: "DEPTH32"
		});

		let vert = NodeShaderContext.make_vert(con_mesh);
		let frag = NodeShaderContext.make_frag(con_mesh);
		frag.ins = vert.outs;

		NodeShader.add_out(vert, 'vec2 texCoord');
		frag.wvpposition = true;
		NodeShader.add_out(vert, 'vec4 prevwvpposition');
		NodeShader.add_uniform(vert, 'mat4 VP', '_view_proj_matrix');
		NodeShader.add_uniform(vert, 'mat4 prevWVP', '_prev_world_view_proj_matrix');
		vert.wposition = true;

		let textureCount = 0;
		let displaceStrength = MakeMaterial.getDisplaceStrength();
		if (MakeMaterial.heightUsed && displaceStrength > 0.0) {
			vert.n = true;
			NodeShader.write(vert, 'float height = 0.0;');
			let numLayers = 1;
			NodeShader.write(vert, `wposition += wnormal * vec3(height, height, height) * vec3(${displaceStrength}, ${displaceStrength}, ${displaceStrength});`);
		}

		NodeShader.write(vert, 'gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		let brushScale = Context.raw.brushScale;
		NodeShader.add_uniform(vert, 'float texScale', '_tex_unpack');
		NodeShader.write(vert, `texCoord = tex * ${brushScale} * texScale;`);
		if (MakeMaterial.heightUsed && displaceStrength > 0) {
			NodeShader.add_uniform(vert, 'mat4 invW', '_inv_world_matrix');
			NodeShader.write(vert, 'prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			NodeShader.write(vert, 'prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		NodeShader.add_out(frag, 'vec4 fragColor[3]');
		frag.n = true;
		NodeShader.add_function(frag, ShaderFunctions.str_packFloatInt16);
		NodeShader.add_function(frag, ShaderFunctions.str_octahedronWrap);
		NodeShader.add_function(frag, ShaderFunctions.str_cotangentFrame);

		NodeShader.write(frag, 'vec3 basecol = vec3(0.0, 0.0, 0.0);');
		NodeShader.write(frag, 'float roughness = 0.0;');
		NodeShader.write(frag, 'float metallic = 0.0;');
		NodeShader.write(frag, 'float occlusion = 1.0;');
		NodeShader.write(frag, 'float opacity = 1.0;');
		NodeShader.write(frag, 'float matid = 0.0;');
		NodeShader.write(frag, 'vec3 ntex = vec3(0.5, 0.5, 1.0);');
		NodeShader.write(frag, 'float height = 0.0;');

		NodeShader.write(frag, 'vec4 texpaint_sample = vec4(0.0, 0.0, 0.0, 1.0);');
		NodeShader.write(frag, 'vec4 texpaint_nor_sample;');
		NodeShader.write(frag, 'vec4 texpaint_pack_sample;');
		NodeShader.write(frag, 'float texpaint_opac;');

		if (MakeMaterial.heightUsed) {
			NodeShader.write(frag, 'float height0 = 0.0;');
			NodeShader.write(frag, 'float height1 = 0.0;');
			NodeShader.write(frag, 'float height2 = 0.0;');
			NodeShader.write(frag, 'float height3 = 0.0;');
		}

		if (Context.raw.viewportMode == ViewportMode.ViewLit && Context.raw.renderMode == RenderMode.RenderForward) {
			NodeShader.add_uniform(frag, 'sampler2D senvmapBrdf', "$brdf.k");
			NodeShader.add_uniform(frag, 'sampler2D senvmapRadiance', '_envmap_radiance');
			NodeShader.add_uniform(frag, 'sampler2D sltcMat', '_ltcMat');
			NodeShader.add_uniform(frag, 'sampler2D sltcMag', '_ltcMag');
		}

		NodeShader.add_shared_sampler(frag, 'sampler2D texpaint');
		NodeShader.write(frag, 'texpaint_sample = textureLodShared(texpaint' + ', texCoord, 0.0);');
		NodeShader.write(frag, 'texpaint_opac = texpaint_sample.a;');

		NodeShader.write(frag, 'basecol = texpaint_sample.rgb * texpaint_opac;');

		NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_nor');
		NodeShader.write(frag, 'texpaint_nor_sample = textureLodShared(texpaint_nor' + ', texCoord, 0.0);');

		NodeShader.write(frag, 'ntex = mix(ntex, texpaint_nor_sample.rgb, texpaint_opac);');

		NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_pack');
		NodeShader.write(frag, 'texpaint_pack_sample = textureLodShared(texpaint_pack' + ', texCoord, 0.0);');

		NodeShader.write(frag, 'occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);');

		NodeShader.write(frag, 'roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);');

		NodeShader.write(frag, 'metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);');

		NodeShader.write(frag, 'height = texpaint_pack_sample.a * texpaint_opac;');

		// if (l.paintHeight && MakeMaterial.heightUsed) {
		// 	let assign = l.paintHeightBlend ? "+=" : "=";
		// 	NodeShader.write(frag, `height ${assign} texpaint_pack_sample.a * texpaint_opac;`);
		// 	NodeShader.write(frag, '{');
		// 	NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
		// 	NodeShader.write(frag, 'float tex_step = 1.0 / texpaintSize.x;');
		// 	NodeShader.write(frag, `height0 ${assign} textureLodShared(texpaint_pack` + ', vec2(texCoord.x - tex_step, texCoord.y), 0.0).a * texpaint_opac;');
		// 	NodeShader.write(frag, `height1 ${assign} textureLodShared(texpaint_pack` + ', vec2(texCoord.x + tex_step, texCoord.y), 0.0).a * texpaint_opac;');
		// 	NodeShader.write(frag, `height2 ${assign} textureLodShared(texpaint_pack` + ', vec2(texCoord.x, texCoord.y - tex_step), 0.0).a * texpaint_opac;');
		// 	NodeShader.write(frag, `height3 ${assign} textureLodShared(texpaint_pack` + ', vec2(texCoord.x, texCoord.y + tex_step), 0.0).a * texpaint_opac;');
		// 	NodeShader.write(frag, '}');
		// }

		if (MakeMaterial.heightUsed) {
			NodeShader.write(frag, 'if (height > 0.0) {');
			NodeShader.write(frag, 'float height_dx = height0 - height1;');
			NodeShader.write(frag, 'float height_dy = height2 - height3;');
			// Whiteout blend
			NodeShader.write(frag, 'vec3 n1 = ntex * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			NodeShader.write(frag, 'vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));');
			NodeShader.write(frag, 'ntex = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			NodeShader.write(frag, '}');
		}

		frag.vVec = true;
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, vVec, texCoord);');
		///else
		NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
		///end
		NodeShader.write(frag, 'n = ntex * 2.0 - 1.0;');
		NodeShader.write(frag, 'n.y = -n.y;');
		NodeShader.write(frag, 'n = normalize(mul(n, TBN));');

		if (Context.raw.viewportMode == ViewportMode.ViewLit || Context.raw.viewportMode == ViewportMode.ViewPathTrace) {

			NodeShader.write(frag, 'basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');

			if (Context.raw.viewportShader != null) {
				let color = Context.raw.viewportShader(frag);
				NodeShader.write(frag, `fragColor[1] = vec4(${color}, 1.0);`);
			}
			else if (Context.raw.renderMode == RenderMode.RenderForward && Context.raw.viewportMode != ViewportMode.ViewPathTrace) {
				frag.wposition = true;
				NodeShader.write(frag, 'vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);');
				NodeShader.write(frag, 'vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);');
				frag.vVec = true;
				NodeShader.write(frag, 'float dotNV = max(0.0, dot(n, vVec));');
				NodeShader.write(frag, 'vec2 envBRDF = texelFetch(senvmapBrdf, ivec2(vec2(roughness, 1.0 - dotNV) * 256.0), 0).xy;');
				NodeShader.add_uniform(frag, 'int envmapNumMipmaps', '_envmap_num_mipmaps');
				NodeShader.add_uniform(frag, 'vec4 envmapData', '_envmapData'); // angle, sin(angle), cos(angle), strength
				NodeShader.write(frag, 'vec3 wreflect = reflect(-vVec, n);');
				NodeShader.write(frag, 'float envlod = roughness * float(envmapNumMipmaps);');
				NodeShader.add_function(frag, ShaderFunctions.str_envMapEquirect);
				NodeShader.write(frag, 'vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(wreflect, envmapData.x), envlod).rgb;');
				NodeShader.add_uniform(frag, 'vec3 lightArea0', '_light_area0');
				NodeShader.add_uniform(frag, 'vec3 lightArea1', '_light_area1');
				NodeShader.add_uniform(frag, 'vec3 lightArea2', '_light_area2');
				NodeShader.add_uniform(frag, 'vec3 lightArea3', '_light_area3');
				NodeShader.add_function(frag, ShaderFunctions.str_ltcEvaluate);
				NodeShader.add_uniform(frag, 'vec3 lightPos', '_point_pos');
				NodeShader.add_uniform(frag, 'vec3 lightColor', '_point_color');
				NodeShader.write(frag, 'float ldist = distance(wposition, lightPos);');
				NodeShader.write(frag, 'const float LUT_SIZE = 64.0;');
				NodeShader.write(frag, 'const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;');
				NodeShader.write(frag, 'const float LUT_BIAS = 0.5 / LUT_SIZE;');
				NodeShader.write(frag, 'float theta = acos(dotNV);');
				NodeShader.write(frag, 'vec2 tuv = vec2(roughness, theta / (0.5 * 3.14159265));');
				NodeShader.write(frag, 'tuv = tuv * LUT_SCALE + LUT_BIAS;');
				NodeShader.write(frag, 'vec4 t = textureLod(sltcMat, tuv, 0.0);');
				NodeShader.write(frag, 'mat3 minv = mat3(vec3(1.0, 0.0, t.y), vec3(0.0, t.z, 0.0), vec3(t.w, 0.0, t.x));');
				NodeShader.write(frag, 'float ltcspec = ltcEvaluate(n, vVec, dotNV, wposition, minv, lightArea0, lightArea1, lightArea2, lightArea3);');
				NodeShader.write(frag, 'ltcspec *= textureLod(sltcMag, tuv, 0.0).a;');
				NodeShader.write(frag, 'mat3 mident = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);');
				NodeShader.write(frag, 'float ltcdiff = ltcEvaluate(n, vVec, dotNV, wposition, mident, lightArea0, lightArea1, lightArea2, lightArea3);');
				NodeShader.write(frag, 'vec3 direct = albedo * ltcdiff + ltcspec * 0.05;');
				NodeShader.write(frag, 'direct *= lightColor * (1.0 / (ldist * ldist));');

				NodeShader.add_uniform(frag, 'vec4 shirr[7]', '_envmap_irradiance');
				NodeShader.add_function(frag, ShaderFunctions.str_shIrradiance);
				NodeShader.write(frag, 'vec3 indirect = albedo * (shIrradiance(vec3(n.x * envmapData.z - n.y * envmapData.y, n.x * envmapData.y + n.y * envmapData.z, n.z), shirr) / 3.14159265);');
				NodeShader.write(frag, 'indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;');
				NodeShader.write(frag, 'indirect *= envmapData.w * occlusion;');
				NodeShader.write(frag, 'fragColor[1] = vec4(direct + indirect, 1.0);');
			}
			else { // Deferred, Pathtraced
				NodeShader.write(frag, 'fragColor[1] = vec4(basecol, occlusion);');
			}
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewBaseColor) {
			NodeShader.write(frag, 'fragColor[1] = vec4(basecol, 1.0);');
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewNormalMap) {
			NodeShader.write(frag, 'fragColor[1] = vec4(ntex.rgb, 1.0);');
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewOcclusion) {
			NodeShader.write(frag, 'fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);');
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewRoughness) {
			NodeShader.write(frag, 'fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);');
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewMetallic) {
			NodeShader.write(frag, 'fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);');
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewOpacity) {
			NodeShader.write(frag, 'fragColor[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);');
		}
		else if (Context.raw.viewportMode == ViewportMode.ViewHeight) {
			NodeShader.write(frag, 'fragColor[1] = vec4(vec3(height, height, height), 1.0);');
		}
		else {
			NodeShader.write(frag, 'fragColor[1] = vec4(1.0, 0.0, 1.0, 1.0);'); // Pink
		}

		if (Context.raw.viewportMode != ViewportMode.ViewLit && Context.raw.viewportMode != ViewportMode.ViewPathTrace) {
			NodeShader.write(frag, 'fragColor[1].rgb = pow(fragColor[1].rgb, vec3(2.2, 2.2, 2.2));');
		}

		NodeShader.write(frag, 'n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		NodeShader.write(frag, 'n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		NodeShader.write(frag, 'fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(int(matid * 255.0) % 3)));');

		NodeShader.write(frag, 'vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		NodeShader.write(frag, 'vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		NodeShader.write(frag, 'fragColor[2] = vec4(posa - posb, texCoord.xy);');

		ParserMaterial.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = NodeShader.get(vert);
		con_mesh.data.fragment_shader = NodeShader.get(frag);
		return con_mesh;
	}
}
