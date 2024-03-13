
class MakeMesh {

	static layerPassCount = 1;

	static run = (data: TMaterial, layerPass = 0): NodeShaderContextRaw => {
		let context_id = layerPass == 0 ? "mesh" : "mesh" + layerPass;
		let con_mesh = NodeShadercontext_create(data, {
			name: context_id,
			depth_write: layerPass == 0 ? true : false,
			compare_mode: layerPass == 0 ? "less" : "equal",
			cull_mode: (context_raw.cullBackfaces || layerPass > 0) ? "clockwise" : "none",
			vertex_elements: [{name: "pos", data: "short4norm"}],
			color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
			depth_attachment: "DEPTH32"
		});

		let vert = NodeShadercontext_make_vert(con_mesh);
		let frag = NodeShadercontext_make_frag(con_mesh);
		frag.ins = vert.outs;

		node_shader_add_out(vert, 'vec2 texCoord');
		frag.wvpposition = true;
		node_shader_add_out(vert, 'vec4 prevwvpposition');
		node_shader_add_uniform(vert, 'mat4 VP', '_view_proj_matrix');
		node_shader_add_uniform(vert, 'mat4 prevWVP', '_prev_world_view_proj_matrix');
		vert.wposition = true;

		let textureCount = 0;

		node_shader_add_uniform(vert, 'mat4 WVP', '_world_view_proj_matrix');
		node_shader_add_uniform(vert, 'sampler2D texpaint_vert', '_texpaint_vert' + project_layers[0].id);
		node_shader_write(vert, 'vec3 meshpos = texelFetch(texpaint_vert, ivec2(gl_VertexID % textureSize(texpaint_vert, 0).x, gl_VertexID / textureSize(texpaint_vert, 0).y), 0).xyz;');
		// + pos.xyz * 0.000001
		node_shader_write(vert, 'gl_Position = mul(vec4(meshpos.xyz, 1.0), WVP);');

		node_shader_write(vert, 'texCoord = vec2(0.0, 0.0);');

		node_shader_write(vert, 'prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');

		node_shader_add_out(frag, 'vec4 fragColor[3]');

		node_shader_add_uniform(vert, "mat3 N", "_normal_matrix");
		node_shader_add_out(vert, "vec3 wnormal");

		node_shader_write_attrib(vert, 'int baseVertex0 = gl_VertexID - (gl_VertexID % 3);');
		node_shader_write_attrib(vert, 'int baseVertex1 = baseVertex0 + 1;');
		node_shader_write_attrib(vert, 'int baseVertex2 = baseVertex0 + 2;');
		node_shader_write_attrib(vert, 'vec3 meshpos0 = texelFetch(texpaint_vert, ivec2(baseVertex0 % textureSize(texpaint_vert, 0).x, baseVertex0 / textureSize(texpaint_vert, 0).y), 0).xyz;');
		node_shader_write_attrib(vert, 'vec3 meshpos1 = texelFetch(texpaint_vert, ivec2(baseVertex1 % textureSize(texpaint_vert, 0).x, baseVertex1 / textureSize(texpaint_vert, 0).y), 0).xyz;');
		node_shader_write_attrib(vert, 'vec3 meshpos2 = texelFetch(texpaint_vert, ivec2(baseVertex2 % textureSize(texpaint_vert, 0).x, baseVertex2 / textureSize(texpaint_vert, 0).y), 0).xyz;');
		node_shader_write_attrib(vert, 'vec3 meshnor = normalize(cross(meshpos2 - meshpos1, meshpos0 - meshpos1));');
		node_shader_write_attrib(vert, 'wnormal = mul(meshnor, N);');
		node_shader_write_attrib(frag, 'vec3 n = normalize(wnormal);');

		node_shader_add_function(frag, str_packFloatInt16);
		node_shader_add_function(frag, str_octahedronWrap);
		node_shader_add_function(frag, str_cotangentFrame);
		if (layerPass > 0) {
			node_shader_add_uniform(frag, 'sampler2D gbuffer0');
			node_shader_add_uniform(frag, 'sampler2D gbuffer1');
			node_shader_add_uniform(frag, 'sampler2D gbuffer2');
			node_shader_write(frag, 'vec2 fragcoord = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			node_shader_write(frag, 'fragcoord.y = 1.0 - fragcoord.y;');
			///end
			node_shader_write(frag, 'vec4 gbuffer0_sample = textureLod(gbuffer0, fragcoord, 0.0);');
			node_shader_write(frag, 'vec4 gbuffer1_sample = textureLod(gbuffer1, fragcoord, 0.0);');
			node_shader_write(frag, 'vec4 gbuffer2_sample = textureLod(gbuffer2, fragcoord, 0.0);');
			node_shader_write(frag, 'vec3 basecol = gbuffer0_sample.rgb;');
			node_shader_write(frag, 'float roughness = gbuffer2_sample.g;');
			node_shader_write(frag, 'float metallic = gbuffer2_sample.b;');
			node_shader_write(frag, 'float occlusion = gbuffer2_sample.r;');
			node_shader_write(frag, 'float opacity = 1.0;//gbuffer0_sample.a;');
			node_shader_write(frag, 'float matid = gbuffer1_sample.a;');
			node_shader_write(frag, 'vec3 ntex = gbuffer1_sample.rgb;');
			node_shader_write(frag, 'float height = gbuffer2_sample.a;');
		}
		else {
			node_shader_write(frag, 'vec3 basecol = vec3(0.0, 0.0, 0.0);');
			node_shader_write(frag, 'float roughness = 0.3;');
			node_shader_write(frag, 'float metallic = 0.0;');
			node_shader_write(frag, 'float occlusion = 1.0;');
			node_shader_write(frag, 'float opacity = 1.0;');
			node_shader_write(frag, 'float matid = 0.0;');
			node_shader_write(frag, 'vec3 ntex = vec3(0.5, 0.5, 1.0);');
			node_shader_write(frag, 'float height = 0.0;');
		}
		node_shader_write(frag, 'vec4 texpaint_sample = vec4(0.0, 0.0, 0.0, 1.0);');
		node_shader_write(frag, 'vec4 texpaint_nor_sample;');
		node_shader_write(frag, 'vec4 texpaint_pack_sample;');
		node_shader_write(frag, 'float texpaint_opac;');

		if (MakeMaterial.heightUsed) {
			node_shader_write(frag, 'float height0 = 0.0;');
			node_shader_write(frag, 'float height1 = 0.0;');
			node_shader_write(frag, 'float height2 = 0.0;');
			node_shader_write(frag, 'float height3 = 0.0;');
		}

		if (context_raw.drawWireframe) {
			textureCount++;
			node_shader_add_uniform(frag, 'sampler2D texuvmap', '_texuvmap');
		}

		if (context_raw.viewportMode == ViewportMode.ViewLit && context_raw.renderMode == RenderMode.RenderForward) {
			textureCount += 4;
			node_shader_add_uniform(frag, 'sampler2D senvmapBrdf', "$brdf.k");
			node_shader_add_uniform(frag, 'sampler2D senvmapRadiance', '_envmap_radiance');
			node_shader_add_uniform(frag, 'sampler2D sltcMat', '_ltcMat');
			node_shader_add_uniform(frag, 'sampler2D sltcMag', '_ltcMag');
		}

		// Get layers for this pass
		MakeMesh.layerPassCount = 1;
		let layers: SlotLayerRaw[] = [];
		let startCount = textureCount;
		for (let l of project_layers) {
			if (!SlotLayer.isLayer(l) || !SlotLayer.isVisible(l)) continue;

			let count = 3;
			let masks = SlotLayer.getMasks(l);
			if (masks != null) count += masks.length;
			textureCount += count;
			if (textureCount >= MakeMesh.getMaxTextures()) {
				textureCount = startCount + count + 3; // gbuffer0_copy, gbuffer1_copy, gbuffer2_copy
				MakeMesh.layerPassCount++;
			}
			if (layerPass == MakeMesh.layerPassCount - 1) {
				layers.push(l);
			}
		}

		let lastPass = layerPass == MakeMesh.layerPassCount - 1;

		for (let l of layers) {
			if (SlotLayer.getObjectMask(l) > 0) {
				node_shader_add_uniform(frag, 'int uid', '_uid');
				if (SlotLayer.getObjectMask(l) > project_paintObjects.length) { // Atlas
					let visibles = project_getAtlasObjects(SlotLayer.getObjectMask(l));
					node_shader_write(frag, 'if (');
					for (let i = 0; i < visibles.length; ++i) {
						if (i > 0) node_shader_write(frag, ' || ');
						node_shader_write(frag, `${visibles[i].base.uid} == uid`);
					}
					node_shader_write(frag, ') {');
				}
				else { // Object mask
					let uid = project_paintObjects[SlotLayer.getObjectMask(l) - 1].base.uid;
					node_shader_write(frag, `if (${uid} == uid) {`);
				}
			}

			node_shader_add_shared_sampler(frag, 'sampler2D texpaint' + l.id);
			node_shader_write(frag, 'texpaint_sample = vec4(0.8, 0.8, 0.8, 1.0);');
			node_shader_write(frag, 'texpaint_opac = texpaint_sample.a;');

			let masks = SlotLayer.getMasks(l);
			if (masks != null) {
				let hasVisible = false;
				for (let m of masks) {
					if (SlotLayer.isVisible(m)) {
						hasVisible = true;
						break;
					}
				}
				if (hasVisible) {
					let texpaint_mask = 'texpaint_mask' + l.id;
					node_shader_write(frag, `float ${texpaint_mask} = 0.0;`);
					for (let m of masks) {
						if (!SlotLayer.isVisible(m)) continue;
						node_shader_add_shared_sampler(frag, 'sampler2D texpaint' + m.id);
						node_shader_write(frag, '{'); // Group mask is sampled across multiple layers
						node_shader_write(frag, 'float texpaint_mask_sample' + m.id + ' = textureLodShared(texpaint' + m.id + ', texCoord, 0.0).r;');
						node_shader_write(frag, '}');
					}
					node_shader_write(frag, `texpaint_opac *= clamp(${texpaint_mask}, 0.0, 1.0);`);
				}
			}

			if (SlotLayer.getOpacity(l) < 1) {
				node_shader_write(frag, `texpaint_opac *= ${SlotLayer.getOpacity(l)};`);
			}

			if (l == project_layers[0]) {
				node_shader_write(frag, 'basecol = vec3(0.8, 0.8, 0.8);// texpaint_sample.rgb * texpaint_opac;');
			}

			if (SlotLayer.getObjectMask(l) > 0) {
				node_shader_write(frag, '}');
			}

			if (lastPass && context_raw.drawTexels) {
				node_shader_add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
				node_shader_write(frag, 'vec2 texel0 = texCoord * texpaintSize * 0.01;');
				node_shader_write(frag, 'vec2 texel1 = texCoord * texpaintSize * 0.1;');
				node_shader_write(frag, 'vec2 texel2 = texCoord * texpaintSize;');
				node_shader_write(frag, 'basecol *= max(float(mod(int(texel0.x), 2.0) == mod(int(texel0.y), 2.0)), 0.9);');
				node_shader_write(frag, 'basecol *= max(float(mod(int(texel1.x), 2.0) == mod(int(texel1.y), 2.0)), 0.9);');
				node_shader_write(frag, 'basecol *= max(float(mod(int(texel2.x), 2.0) == mod(int(texel2.y), 2.0)), 0.9);');
			}

			if (lastPass && context_raw.drawWireframe) {
				node_shader_write(frag, 'basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
			}

			if (MakeMaterial.heightUsed) {
				node_shader_write(frag, 'if (height > 0.0) {');
				node_shader_write(frag, 'float height_dx = height0 - height1;');
				node_shader_write(frag, 'float height_dy = height2 - height3;');
				// Whiteout blend
				node_shader_write(frag, 'vec3 n1 = ntex * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
				node_shader_write(frag, 'vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));');
				node_shader_write(frag, 'ntex = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
				node_shader_write(frag, '}');
			}

			if (!lastPass) {
				node_shader_write(frag, 'fragColor[0] = vec4(basecol, opacity);');
				node_shader_write(frag, 'fragColor[1] = vec4(ntex, matid);');
				node_shader_write(frag, 'fragColor[2] = vec4(occlusion, roughness, metallic, height);');
				parser_material_finalize(con_mesh);
				con_mesh.data.shader_from_source = true;
				con_mesh.data.vertex_shader = node_shader_get(vert);
				con_mesh.data.fragment_shader = node_shader_get(frag);
				return con_mesh;
			}

			frag.vVec = true;
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			node_shader_write(frag, 'mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			///else
			node_shader_write(frag, 'mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			///end
			node_shader_write(frag, 'n = ntex * 2.0 - 1.0;');
			node_shader_write(frag, 'n.y = -n.y;');
			node_shader_write(frag, 'n = normalize(mul(n, TBN));');

			if (context_raw.viewportMode == ViewportMode.ViewLit) {

				node_shader_write(frag, 'basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');

				if (context_raw.viewportShader != null) {
					let color = context_raw.viewportShader(frag);
					node_shader_write(frag, `fragColor[1] = vec4(${color}, 1.0);`);
				}
				else if (context_raw.renderMode == RenderMode.RenderForward) {
					frag.wposition = true;
					node_shader_write(frag, 'vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);');
					node_shader_write(frag, 'vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);');
					frag.vVec = true;
					node_shader_write(frag, 'float dotNV = max(0.0, dot(n, vVec));');
					node_shader_write(frag, 'vec2 envBRDF = texelFetch(senvmapBrdf, ivec2(vec2(roughness, 1.0 - dotNV) * 256.0), 0).xy;');
					node_shader_add_uniform(frag, 'int envmapNumMipmaps', '_envmap_num_mipmaps');
					node_shader_add_uniform(frag, 'vec4 envmapData', '_envmapData'); // angle, sin(angle), cos(angle), strength
					node_shader_write(frag, 'vec3 wreflect = reflect(-vVec, n);');
					node_shader_write(frag, 'float envlod = roughness * float(envmapNumMipmaps);');
					node_shader_add_function(frag, str_envMapEquirect);
					node_shader_write(frag, 'vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(wreflect, envmapData.x), envlod).rgb;');
					node_shader_add_uniform(frag, 'vec3 lightArea0', '_light_area0');
					node_shader_add_uniform(frag, 'vec3 lightArea1', '_light_area1');
					node_shader_add_uniform(frag, 'vec3 lightArea2', '_light_area2');
					node_shader_add_uniform(frag, 'vec3 lightArea3', '_light_area3');
					node_shader_add_function(frag, str_ltcEvaluate);
					node_shader_add_uniform(frag, 'vec3 lightPos', '_point_pos');
					node_shader_add_uniform(frag, 'vec3 lightColor', '_point_color');
					node_shader_write(frag, 'float ldist = distance(wposition, lightPos);');
					node_shader_write(frag, 'const float LUT_SIZE = 64.0;');
					node_shader_write(frag, 'const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;');
					node_shader_write(frag, 'const float LUT_BIAS = 0.5 / LUT_SIZE;');
					node_shader_write(frag, 'float theta = acos(dotNV);');
					node_shader_write(frag, 'vec2 tuv = vec2(roughness, theta / (0.5 * 3.14159265));');
					node_shader_write(frag, 'tuv = tuv * LUT_SCALE + LUT_BIAS;');
					node_shader_write(frag, 'vec4 t = textureLod(sltcMat, tuv, 0.0);');
					node_shader_write(frag, 'mat3 minv = mat3(vec3(1.0, 0.0, t.y), vec3(0.0, t.z, 0.0), vec3(t.w, 0.0, t.x));');
					node_shader_write(frag, 'float ltcspec = ltcEvaluate(n, vVec, dotNV, wposition, minv, lightArea0, lightArea1, lightArea2, lightArea3);');
					node_shader_write(frag, 'ltcspec *= textureLod(sltcMag, tuv, 0.0).a;');
					node_shader_write(frag, 'mat3 mident = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);');
					node_shader_write(frag, 'float ltcdiff = ltcEvaluate(n, vVec, dotNV, wposition, mident, lightArea0, lightArea1, lightArea2, lightArea3);');
					node_shader_write(frag, 'vec3 direct = albedo * ltcdiff + ltcspec * 0.05;');
					node_shader_write(frag, 'direct *= lightColor * (1.0 / (ldist * ldist));');

					node_shader_add_uniform(frag, 'vec4 shirr[7]', '_envmap_irradiance');
					node_shader_add_function(frag, str_shIrradiance);
					node_shader_write(frag, 'vec3 indirect = albedo * (shIrradiance(vec3(n.x * envmapData.z - n.y * envmapData.y, n.x * envmapData.y + n.y * envmapData.z, n.z), shirr) / 3.14159265);');
					node_shader_write(frag, 'indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;');
					node_shader_write(frag, 'indirect *= envmapData.w * occlusion;');
					node_shader_write(frag, 'fragColor[1] = vec4(direct + indirect, 1.0);');
				}
				else { // Deferred, Pathtraced
					if (MakeMaterial.emisUsed) node_shader_write(frag, 'if (int(matid * 255.0) % 3 == 1) basecol *= 10.0;'); // Boost for bloom
					node_shader_write(frag, 'fragColor[1] = vec4(basecol, occlusion);');
				}
			}
			else if (context_raw.viewportMode == ViewportMode.ViewObjectNormal) {
				frag.nAttr = true;
				node_shader_write(frag, 'fragColor[1] = vec4(nAttr, 1.0);');
			}
			else if (context_raw.viewportMode == ViewportMode.ViewObjectID) {
				node_shader_add_uniform(frag, 'float objectId', '_objectId');
				node_shader_write(frag, 'float obid = objectId + 1.0 / 255.0;');
				node_shader_write(frag, 'float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				node_shader_write(frag, 'float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
				node_shader_write(frag, 'float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				node_shader_write(frag, 'fragColor[1] = vec4(id_r, id_g, id_b, 1.0);');
			}
			else {
				node_shader_write(frag, 'fragColor[1] = vec4(1.0, 0.0, 1.0, 1.0);'); // Pink
			}

			if (context_raw.viewportMode != ViewportMode.ViewLit && context_raw.viewportMode != ViewportMode.ViewPathTrace) {
				node_shader_write(frag, 'fragColor[1].rgb = pow(fragColor[1].rgb, vec3(2.2, 2.2, 2.2));');
			}

			node_shader_write(frag, 'n /= (abs(n.x) + abs(n.y) + abs(n.z));');
			node_shader_write(frag, 'n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
			node_shader_write(frag, 'fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(int(matid * 255.0) % 3)));');
		}

		node_shader_write(frag, 'vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		node_shader_write(frag, 'vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		node_shader_write(frag, 'fragColor[2] = vec4(posa - posb, texCoord.xy);');

		parser_material_finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = node_shader_get(vert);
		con_mesh.data.fragment_shader = node_shader_get(frag);
		return con_mesh;
	}

	static getMaxTextures = (): i32 => {
		///if krom_direct3d11
		return 128 - 66;
		///else
		return 16 - 3; // G4onG5/G4.c.h MAX_TEXTURES
		///end
	}
}
