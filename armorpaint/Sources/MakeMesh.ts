
class MakeMesh {

	static layerPassCount = 1;

	static run = (data: TMaterial, layerPass = 0): NodeShaderContextRaw => {
		let context_id = layerPass == 0 ? "mesh" : "mesh" + layerPass;
		let con_mesh = NodeShaderContext.create(data, {
			name: context_id,
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
			let numLayers = 0;
			for (let l of Project.layers) {
				if (!SlotLayer.isVisible(l) || !l.paintHeight || !SlotLayer.isLayer(l)) continue;
				if (numLayers > 16) break;
				numLayers++;
				textureCount++;
				NodeShader.add_uniform(vert, 'sampler2D texpaint_pack_vert' + l.id, '_texpaint_pack_vert' + l.id);
				NodeShader.write(vert, 'height += textureLod(texpaint_pack_vert' + l.id + ', tex, 0.0).a;');
				let masks = SlotLayer.getMasks(l);
				if (masks != null) {
					for (let m of masks) {
						if (!SlotLayer.isVisible(m)) continue;
						textureCount++;
						NodeShader.add_uniform(vert, 'sampler2D texpaint_vert' + m.id, '_texpaint_vert' + m.id);
						NodeShader.write(vert, 'height *= textureLod(texpaint_vert' + m.id + ', tex, 0.0).r;');
					}
				}
			}
			NodeShader.write(vert, `wposition += wnormal * vec3(height, height, height) * vec3(${displaceStrength}, ${displaceStrength}, ${displaceStrength});`);
		}

		NodeShader.write(vert, 'gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		NodeShader.write(vert, 'texCoord = tex;');
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

		if (Context.raw.tool == WorkspaceTool.ToolColorId) {
			textureCount++;
			NodeShader.add_uniform(frag, 'sampler2D texcolorid', '_texcolorid');
			NodeShader.write(frag, 'fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
			NodeShader.write(frag, 'vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			NodeShader.write(frag, 'fragColor[1] = vec4(idcol.rgb, 1.0);'); // occ
		}
		else {
			NodeShader.add_function(frag, ShaderFunctions.str_octahedronWrap);
			NodeShader.add_function(frag, ShaderFunctions.str_cotangentFrame);
			if (layerPass > 0) {
				NodeShader.add_uniform(frag, 'sampler2D gbuffer0');
				NodeShader.add_uniform(frag, 'sampler2D gbuffer1');
				NodeShader.add_uniform(frag, 'sampler2D gbuffer2');
				NodeShader.write(frag, 'vec2 fragcoord = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
				///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				NodeShader.write(frag, 'fragcoord.y = 1.0 - fragcoord.y;');
				///end
				NodeShader.write(frag, 'vec4 gbuffer0_sample = textureLod(gbuffer0, fragcoord, 0.0);');
				NodeShader.write(frag, 'vec4 gbuffer1_sample = textureLod(gbuffer1, fragcoord, 0.0);');
				NodeShader.write(frag, 'vec4 gbuffer2_sample = textureLod(gbuffer2, fragcoord, 0.0);');
				NodeShader.write(frag, 'vec3 basecol = gbuffer0_sample.rgb;');
				NodeShader.write(frag, 'float roughness = gbuffer2_sample.g;');
				NodeShader.write(frag, 'float metallic = gbuffer2_sample.b;');
				NodeShader.write(frag, 'float occlusion = gbuffer2_sample.r;');
				NodeShader.write(frag, 'float opacity = 1.0;//gbuffer0_sample.a;');
				NodeShader.write(frag, 'float matid = gbuffer1_sample.a;');
				NodeShader.write(frag, 'vec3 ntex = gbuffer1_sample.rgb;');
				NodeShader.write(frag, 'float height = gbuffer2_sample.a;');
			}
			else {
				NodeShader.write(frag, 'vec3 basecol = vec3(0.0, 0.0, 0.0);');
				NodeShader.write(frag, 'float roughness = 0.0;');
				NodeShader.write(frag, 'float metallic = 0.0;');
				NodeShader.write(frag, 'float occlusion = 1.0;');
				NodeShader.write(frag, 'float opacity = 1.0;');
				NodeShader.write(frag, 'float matid = 0.0;');
				NodeShader.write(frag, 'vec3 ntex = vec3(0.5, 0.5, 1.0);');
				NodeShader.write(frag, 'float height = 0.0;');
			}
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

			if (Context.raw.drawWireframe) {
				textureCount++;
				NodeShader.add_uniform(frag, 'sampler2D texuvmap', '_texuvmap');
			}

			if (Context.raw.viewportMode == ViewportMode.ViewMask && SlotLayer.getMasks(Context.raw.layer) != null) {
				for (let m of SlotLayer.getMasks(Context.raw.layer)) {
					if (!SlotLayer.isVisible(m)) continue;
					textureCount++;
					NodeShader.add_uniform(frag, 'sampler2D texpaint_view_mask' + m.id, '_texpaint' + Project.layers.indexOf(m));
				}
			}

			if (Context.raw.viewportMode == ViewportMode.ViewLit && Context.raw.renderMode == RenderMode.RenderForward) {
				textureCount += 4;
				NodeShader.add_uniform(frag, 'sampler2D senvmapBrdf', "$brdf.k");
				NodeShader.add_uniform(frag, 'sampler2D senvmapRadiance', '_envmap_radiance');
				NodeShader.add_uniform(frag, 'sampler2D sltcMat', '_ltcMat');
				NodeShader.add_uniform(frag, 'sampler2D sltcMag', '_ltcMag');
			}

			// Get layers for this pass
			MakeMesh.layerPassCount = 1;
			let layers: SlotLayerRaw[] = [];
			let startCount = textureCount;
			let isMaterialTool = Context.raw.tool == WorkspaceTool.ToolMaterial;
			for (let l of Project.layers) {
				if (isMaterialTool && l != Context.raw.layer) continue;
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
					NodeShader.add_uniform(frag, 'int uid', '_uid');
					if (SlotLayer.getObjectMask(l) > Project.paintObjects.length) { // Atlas
						let visibles = Project.getAtlasObjects(SlotLayer.getObjectMask(l));
						NodeShader.write(frag, 'if (');
						for (let i = 0; i < visibles.length; ++i) {
							if (i > 0) NodeShader.write(frag, ' || ');
							NodeShader.write(frag, `${visibles[i].base.uid} == uid`);
						}
						NodeShader.write(frag, ') {');
					}
					else { // Object mask
						let uid = Project.paintObjects[SlotLayer.getObjectMask(l) - 1].base.uid;
						NodeShader.write(frag, `if (${uid} == uid) {`);
					}
				}

				NodeShader.add_shared_sampler(frag, 'sampler2D texpaint' + l.id);
				NodeShader.write(frag, 'texpaint_sample = textureLodShared(texpaint' + l.id + ', texCoord, 0.0);');
				NodeShader.write(frag, 'texpaint_opac = texpaint_sample.a;');
				// ///if (krom_direct3d12 || krom_vulkan)
				// if (Context.raw.viewportMode == ViewLit) {
				// 	NodeShader.write(frag, 'if (texpaint_opac < 0.1) discard;');
				// }
				// ///end

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
						NodeShader.write(frag, `float ${texpaint_mask} = 0.0;`);
						for (let m of masks) {
							if (!SlotLayer.isVisible(m)) continue;
							NodeShader.add_shared_sampler(frag, 'sampler2D texpaint' + m.id);
							NodeShader.write(frag, '{'); // Group mask is sampled across multiple layers
							NodeShader.write(frag, 'float texpaint_mask_sample' + m.id + ' = textureLodShared(texpaint' + m.id + ', texCoord, 0.0).r;');
							NodeShader.write(frag, `${texpaint_mask} = ` + MakeMaterial.blendModeMask(frag, m.blending, `${texpaint_mask}`, 'texpaint_mask_sample' + m.id, 'float(' + SlotLayer.getOpacity(m) + ')') + ';');
							NodeShader.write(frag, '}');
						}
						NodeShader.write(frag, `texpaint_opac *= clamp(${texpaint_mask}, 0.0, 1.0);`);
					}
				}

				if (SlotLayer.getOpacity(l) < 1) {
					NodeShader.write(frag, `texpaint_opac *= ${SlotLayer.getOpacity(l)};`);
				}

				if (l.paintBase) {
					if (l == Project.layers[0]) {
						NodeShader.write(frag, 'basecol = texpaint_sample.rgb * texpaint_opac;');
					}
					else {
						NodeShader.write(frag, 'basecol = ' + MakeMaterial.blendMode(frag, l.blending, 'basecol', 'texpaint_sample.rgb', 'texpaint_opac') + ';');
					}
				}

				if (l.paintNor || MakeMaterial.emisUsed) {
					NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_nor' + l.id);
					NodeShader.write(frag, 'texpaint_nor_sample = textureLodShared(texpaint_nor' + l.id + ', texCoord, 0.0);');

					if (MakeMaterial.emisUsed) {
						NodeShader.write(frag, 'if (texpaint_opac > 0.0) matid = texpaint_nor_sample.a;');
					}

					if (l.paintNor) {
						if (l.paintNorBlend) {
							// Whiteout blend
							NodeShader.write(frag, '{');
							NodeShader.write(frag, 'vec3 n1 = ntex * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
							NodeShader.write(frag, 'vec3 n2 = mix(vec3(0.5, 0.5, 1.0), texpaint_nor_sample.rgb, texpaint_opac) * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
							NodeShader.write(frag, 'ntex = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
							NodeShader.write(frag, '}');
						}
						else {
							NodeShader.write(frag, 'ntex = mix(ntex, texpaint_nor_sample.rgb, texpaint_opac);');
						}
					}
				}

				if (l.paintOcc || l.paintRough || l.paintMet || (l.paintHeight && MakeMaterial.heightUsed)) {
					NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_pack' + l.id);
					NodeShader.write(frag, 'texpaint_pack_sample = textureLodShared(texpaint_pack' + l.id + ', texCoord, 0.0);');

					if (l.paintOcc) {
						NodeShader.write(frag, 'occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);');
					}
					if (l.paintRough) {
						NodeShader.write(frag, 'roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);');
					}
					if (l.paintMet) {
						NodeShader.write(frag, 'metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);');
					}
					if (l.paintHeight && MakeMaterial.heightUsed) {
						let assign = l.paintHeightBlend ? "+=" : "=";
						NodeShader.write(frag, `height ${assign} texpaint_pack_sample.a * texpaint_opac;`);
						NodeShader.write(frag, '{');
						NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
						NodeShader.write(frag, 'float tex_step = 1.0 / texpaintSize.x;');
						NodeShader.write(frag, `height0 ${assign} textureLodShared(texpaint_pack` + l.id + ', vec2(texCoord.x - tex_step, texCoord.y), 0.0).a * texpaint_opac;');
						NodeShader.write(frag, `height1 ${assign} textureLodShared(texpaint_pack` + l.id + ', vec2(texCoord.x + tex_step, texCoord.y), 0.0).a * texpaint_opac;');
						NodeShader.write(frag, `height2 ${assign} textureLodShared(texpaint_pack` + l.id + ', vec2(texCoord.x, texCoord.y - tex_step), 0.0).a * texpaint_opac;');
						NodeShader.write(frag, `height3 ${assign} textureLodShared(texpaint_pack` + l.id + ', vec2(texCoord.x, texCoord.y + tex_step), 0.0).a * texpaint_opac;');
						NodeShader.write(frag, '}');
					}
				}

				if (SlotLayer.getObjectMask(l) > 0) {
					NodeShader.write(frag, '}');
				}
			}

			if (lastPass && Context.raw.drawTexels) {
				NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
				NodeShader.write(frag, 'vec2 texel0 = texCoord * texpaintSize * 0.01;');
				NodeShader.write(frag, 'vec2 texel1 = texCoord * texpaintSize * 0.1;');
				NodeShader.write(frag, 'vec2 texel2 = texCoord * texpaintSize;');
				NodeShader.write(frag, 'basecol *= max(float(mod(int(texel0.x), 2.0) == mod(int(texel0.y), 2.0)), 0.9);');
				NodeShader.write(frag, 'basecol *= max(float(mod(int(texel1.x), 2.0) == mod(int(texel1.y), 2.0)), 0.9);');
				NodeShader.write(frag, 'basecol *= max(float(mod(int(texel2.x), 2.0) == mod(int(texel2.y), 2.0)), 0.9);');
			}

			if (lastPass && Context.raw.drawWireframe) {
				NodeShader.write(frag, 'basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
			}

			if (MakeMaterial.heightUsed) {
				NodeShader.write(frag, 'if (height > 0.0) {');
				// NodeShader.write(frag, 'float height_dx = dFdx(height * 16.0);');
				// NodeShader.write(frag, 'float height_dy = dFdy(height * 16.0);');
				NodeShader.write(frag, 'float height_dx = height0 - height1;');
				NodeShader.write(frag, 'float height_dy = height2 - height3;');
				// Whiteout blend
				NodeShader.write(frag, 'vec3 n1 = ntex * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
				NodeShader.write(frag, 'vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));');
				NodeShader.write(frag, 'ntex = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
				NodeShader.write(frag, '}');
			}

			if (!lastPass) {
				NodeShader.write(frag, 'fragColor[0] = vec4(basecol, opacity);');
				NodeShader.write(frag, 'fragColor[1] = vec4(ntex, matid);');
				NodeShader.write(frag, 'fragColor[2] = vec4(occlusion, roughness, metallic, height);');
				ParserMaterial.finalize(con_mesh);
				con_mesh.data.shader_from_source = true;
				con_mesh.data.vertex_shader = NodeShader.get(vert);
				con_mesh.data.fragment_shader = NodeShader.get(frag);
				return con_mesh;
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
					// NodeShader.write(frag, 'float dotNL = max(dot(n, normalize(lightPos - wposition)), 0.0);');
					// NodeShader.write(frag, 'vec3 direct = albedo * dotNL;');
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
					if (MakeMaterial.emisUsed) NodeShader.write(frag, 'if (int(matid * 255.0) % 3 == 1) basecol *= 10.0;'); // Boost for bloom
					NodeShader.write(frag, 'fragColor[1] = vec4(basecol, occlusion);');
				}
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewBaseColor && Context.raw.layer.paintBase) {
				NodeShader.write(frag, 'fragColor[1] = vec4(basecol, 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewNormalMap && Context.raw.layer.paintNor) {
				NodeShader.write(frag, 'fragColor[1] = vec4(ntex.rgb, 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewOcclusion && Context.raw.layer.paintOcc) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewRoughness && Context.raw.layer.paintRough) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewMetallic && Context.raw.layer.paintMet) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewOpacity && Context.raw.layer.paintOpac) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewHeight && Context.raw.layer.paintHeight) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(height, height, height), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewEmission) {
				NodeShader.write(frag, 'float emis = int(matid * 255.0) % 3 == 1 ? 1.0 : 0.0;');
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(emis, emis, emis), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewSubsurface) {
				NodeShader.write(frag, 'float subs = int(matid * 255.0) % 3 == 2 ? 1.0 : 0.0;');
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(subs, subs, subs), 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewTexCoord) {
				NodeShader.write(frag, 'fragColor[1] = vec4(texCoord, 0.0, 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewObjectNormal) {
				frag.nAttr = true;
				NodeShader.write(frag, 'fragColor[1] = vec4(nAttr, 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewMaterialID) {
				NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_nor' + Context.raw.layer.id);
				NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
				NodeShader.write(frag, 'float sample_matid = texelFetch(texpaint_nor' + Context.raw.layer.id + ', ivec2(texCoord * texpaintSize), 0).a + 1.0 / 255.0;');
				NodeShader.write(frag, 'float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'fragColor[1] = vec4(matid_r, matid_g, matid_b, 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewObjectID) {
				NodeShader.add_uniform(frag, 'float objectId', '_objectId');
				NodeShader.write(frag, 'float obid = objectId + 1.0 / 255.0;');
				NodeShader.write(frag, 'float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'fragColor[1] = vec4(id_r, id_g, id_b, 1.0);');
			}
			else if (Context.raw.viewportMode == ViewportMode.ViewMask && (SlotLayer.getMasks(Context.raw.layer) != null || SlotLayer.isMask(Context.raw.layer))) {
				if (SlotLayer.isMask(Context.raw.layer)) {
					NodeShader.write(frag, 'float mask_view = textureLodShared(texpaint' + Context.raw.layer.id + ', texCoord, 0.0).r;');
				}
				else {
					NodeShader.write(frag, 'float mask_view = 0.0;');
					for (let m of SlotLayer.getMasks(Context.raw.layer)) {
						if (!SlotLayer.isVisible(m)) continue;
						NodeShader.write(frag, 'float mask_sample' + m.id + ' = textureLodShared(texpaint_view_mask' + m.id + ', texCoord, 0.0).r;');
						NodeShader.write(frag, 'mask_view = ' + MakeMaterial.blendModeMask(frag, m.blending, 'mask_view', 'mask_sample' + m.id, 'float(' + SlotLayer.getOpacity(m) + ')') + ';');
					}
				}
				NodeShader.write(frag, 'fragColor[1] = vec4(mask_view, mask_view, mask_view, 1.0);');
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
		}

		NodeShader.write(frag, 'vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		NodeShader.write(frag, 'vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		NodeShader.write(frag, 'fragColor[2] = vec4(posa - posb, texCoord.xy);');

		ParserMaterial.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = NodeShader.get(vert);
		con_mesh.data.fragment_shader = NodeShader.get(frag);
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
