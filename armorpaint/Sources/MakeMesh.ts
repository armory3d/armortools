
class MakeMesh {

	static layer_pass_count = 1;

	static run = (data: material_t, layerPass = 0): NodeShaderContextRaw => {
		let context_id: string = layerPass == 0 ? "mesh" : "mesh" + layerPass;
		let con_mesh: NodeShaderContextRaw = NodeShaderContext.create(data, {
			name: context_id,
			depth_write: layerPass == 0 ? true : false,
			compare_mode: layerPass == 0 ? "less" : "equal",
			cull_mode: (context_raw.cull_backfaces || layerPass > 0) ? "clockwise" : "none",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
			color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
			depth_attachment: "DEPTH32"
		});

		let vert: NodeShaderRaw = NodeShaderContext.make_vert(con_mesh);
		let frag: NodeShaderRaw = NodeShaderContext.make_frag(con_mesh);
		frag.ins = vert.outs;

		NodeShader.add_out(vert, 'vec2 texCoord');
		frag.wvpposition = true;
		NodeShader.add_out(vert, 'vec4 prevwvpposition');
		NodeShader.add_uniform(vert, 'mat4 VP', '_view_proj_matrix');
		NodeShader.add_uniform(vert, 'mat4 prevWVP', '_prev_world_view_proj_matrix');
		vert.wposition = true;

		let texture_count: i32 = 0;
		let displace_strength: f32 = MakeMaterial.get_displace_strength();
		if (MakeMaterial.height_used && displace_strength > 0.0) {
			vert.n = true;
			NodeShader.write(vert, 'float height = 0.0;');
			let num_layers: i32 = 0;
			for (let l of project_layers) {
				if (!SlotLayer.is_visible(l) || !l.paint_height || !SlotLayer.is_layer(l)) continue;
				if (num_layers > 16) break;
				num_layers++;
				texture_count++;
				NodeShader.add_uniform(vert, 'sampler2D texpaint_pack_vert' + l.id, '_texpaint_pack_vert' + l.id);
				NodeShader.write(vert, 'height += textureLod(texpaint_pack_vert' + l.id + ', tex, 0.0).a;');
				let masks: SlotLayerRaw[] = SlotLayer.get_masks(l);
				if (masks != null) {
					for (let m of masks) {
						if (!SlotLayer.is_visible(m)) continue;
						texture_count++;
						NodeShader.add_uniform(vert, 'sampler2D texpaint_vert' + m.id, '_texpaint_vert' + m.id);
						NodeShader.write(vert, 'height *= textureLod(texpaint_vert' + m.id + ', tex, 0.0).r;');
					}
				}
			}
			NodeShader.write(vert, `wposition += wnormal * vec3(height, height, height) * vec3(${displace_strength}, ${displace_strength}, ${displace_strength});`);
		}

		NodeShader.write(vert, 'gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		NodeShader.write(vert, 'texCoord = tex;');
		if (MakeMaterial.height_used && displace_strength > 0) {
			NodeShader.add_uniform(vert, 'mat4 invW', '_inv_world_matrix');
			NodeShader.write(vert, 'prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			NodeShader.write(vert, 'prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		NodeShader.add_out(frag, 'vec4 fragColor[3]');
		frag.n = true;
		NodeShader.add_function(frag, str_pack_float_int16);

		if (context_raw.tool == workspace_tool_t.COLORID) {
			texture_count++;
			NodeShader.add_uniform(frag, 'sampler2D texcolorid', '_texcolorid');
			NodeShader.write(frag, 'fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
			NodeShader.write(frag, 'vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			NodeShader.write(frag, 'fragColor[1] = vec4(idcol.rgb, 1.0);'); // occ
		}
		else {
			NodeShader.add_function(frag, str_octahedron_wrap);
			NodeShader.add_function(frag, str_cotangent_frame);
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

			if (MakeMaterial.height_used) {
				NodeShader.write(frag, 'float height0 = 0.0;');
				NodeShader.write(frag, 'float height1 = 0.0;');
				NodeShader.write(frag, 'float height2 = 0.0;');
				NodeShader.write(frag, 'float height3 = 0.0;');
			}

			if (context_raw.draw_wireframe) {
				texture_count++;
				NodeShader.add_uniform(frag, 'sampler2D texuvmap', '_texuvmap');
			}

			if (context_raw.viewport_mode == viewport_mode_t.MASK && SlotLayer.get_masks(context_raw.layer) != null) {
				for (let m of SlotLayer.get_masks(context_raw.layer)) {
					if (!SlotLayer.is_visible(m)) continue;
					texture_count++;
					NodeShader.add_uniform(frag, 'sampler2D texpaint_view_mask' + m.id, '_texpaint' + project_layers.indexOf(m));
				}
			}

			if (context_raw.viewport_mode == viewport_mode_t.LIT && context_raw.render_mode == render_mode_t.FORWARD) {
				texture_count += 4;
				NodeShader.add_uniform(frag, 'sampler2D senvmapBrdf', "$brdf.k");
				NodeShader.add_uniform(frag, 'sampler2D senvmapRadiance', '_envmap_radiance');
				NodeShader.add_uniform(frag, 'sampler2D sltcMat', '_ltcMat');
				NodeShader.add_uniform(frag, 'sampler2D sltcMag', '_ltcMag');
			}

			// Get layers for this pass
			MakeMesh.layer_pass_count = 1;
			let layers: SlotLayerRaw[] = [];
			let start_count: i32 = texture_count;
			let is_material_tool: bool = context_raw.tool == workspace_tool_t.MATERIAL;
			for (let l of project_layers) {
				if (is_material_tool && l != context_raw.layer) continue;
				if (!SlotLayer.is_layer(l) || !SlotLayer.is_visible(l)) continue;

				let count: i32 = 3;
				let masks: SlotLayerRaw[] = SlotLayer.get_masks(l);
				if (masks != null) count += masks.length;
				texture_count += count;
				if (texture_count >= MakeMesh.get_max_textures()) {
					texture_count = start_count + count + 3; // gbuffer0_copy, gbuffer1_copy, gbuffer2_copy
					MakeMesh.layer_pass_count++;
				}
				if (layerPass == MakeMesh.layer_pass_count - 1) {
					layers.push(l);
				}
			}

			let last_pass: bool = layerPass == MakeMesh.layer_pass_count - 1;

			for (let l of layers) {
				if (SlotLayer.get_object_mask(l) > 0) {
					NodeShader.add_uniform(frag, 'int uid', '_uid');
					if (SlotLayer.get_object_mask(l) > project_paint_objects.length) { // Atlas
						let visibles: mesh_object_t[] = project_get_atlas_objects(SlotLayer.get_object_mask(l));
						NodeShader.write(frag, 'if (');
						for (let i: i32 = 0; i < visibles.length; ++i) {
							if (i > 0) NodeShader.write(frag, ' || ');
							NodeShader.write(frag, `${visibles[i].base.uid} == uid`);
						}
						NodeShader.write(frag, ') {');
					}
					else { // Object mask
						let uid: i32 = project_paint_objects[SlotLayer.get_object_mask(l) - 1].base.uid;
						NodeShader.write(frag, `if (${uid} == uid) {`);
					}
				}

				NodeShader.add_shared_sampler(frag, 'sampler2D texpaint' + l.id);
				NodeShader.write(frag, 'texpaint_sample = textureLodShared(texpaint' + l.id + ', texCoord, 0.0);');
				NodeShader.write(frag, 'texpaint_opac = texpaint_sample.a;');
				// ///if (krom_direct3d12 || krom_vulkan)
				// if (raw.viewportMode == ViewLit) {
				// 	NodeShader.write(frag, 'if (texpaint_opac < 0.1) discard;');
				// }
				// ///end

				let masks: SlotLayerRaw[] = SlotLayer.get_masks(l);
				if (masks != null) {
					let has_visible: bool = false;
					for (let m of masks) {
						if (SlotLayer.is_visible(m)) {
							has_visible = true;
							break;
						}
					}
					if (has_visible) {
						let texpaint_mask: string = 'texpaint_mask' + l.id;
						NodeShader.write(frag, `float ${texpaint_mask} = 0.0;`);
						for (let m of masks) {
							if (!SlotLayer.is_visible(m)) continue;
							NodeShader.add_shared_sampler(frag, 'sampler2D texpaint' + m.id);
							NodeShader.write(frag, '{'); // Group mask is sampled across multiple layers
							NodeShader.write(frag, 'float texpaint_mask_sample' + m.id + ' = textureLodShared(texpaint' + m.id + ', texCoord, 0.0).r;');
							NodeShader.write(frag, `${texpaint_mask} = ` + MakeMaterial.blend_mode_mask(frag, m.blending, `${texpaint_mask}`, 'texpaint_mask_sample' + m.id, 'float(' + SlotLayer.get_opacity(m) + ')') + ';');
							NodeShader.write(frag, '}');
						}
						NodeShader.write(frag, `texpaint_opac *= clamp(${texpaint_mask}, 0.0, 1.0);`);
					}
				}

				if (SlotLayer.get_opacity(l) < 1) {
					NodeShader.write(frag, `texpaint_opac *= ${SlotLayer.get_opacity(l)};`);
				}

				if (l.paint_base) {
					if (l == project_layers[0]) {
						NodeShader.write(frag, 'basecol = texpaint_sample.rgb * texpaint_opac;');
					}
					else {
						NodeShader.write(frag, 'basecol = ' + MakeMaterial.blend_mode(frag, l.blending, 'basecol', 'texpaint_sample.rgb', 'texpaint_opac') + ';');
					}
				}

				if (l.paint_nor || MakeMaterial.emis_used) {
					NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_nor' + l.id);
					NodeShader.write(frag, 'texpaint_nor_sample = textureLodShared(texpaint_nor' + l.id + ', texCoord, 0.0);');

					if (MakeMaterial.emis_used) {
						NodeShader.write(frag, 'if (texpaint_opac > 0.0) matid = texpaint_nor_sample.a;');
					}

					if (l.paint_nor) {
						if (l.paint_nor_blend) {
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

				if (l.paint_occ || l.paint_rough || l.paint_met || (l.paint_height && MakeMaterial.height_used)) {
					NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_pack' + l.id);
					NodeShader.write(frag, 'texpaint_pack_sample = textureLodShared(texpaint_pack' + l.id + ', texCoord, 0.0);');

					if (l.paint_occ) {
						NodeShader.write(frag, 'occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);');
					}
					if (l.paint_rough) {
						NodeShader.write(frag, 'roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);');
					}
					if (l.paint_met) {
						NodeShader.write(frag, 'metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);');
					}
					if (l.paint_height && MakeMaterial.height_used) {
						let assign: string = l.paint_height_blend ? "+=" : "=";
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

				if (SlotLayer.get_object_mask(l) > 0) {
					NodeShader.write(frag, '}');
				}
			}

			if (last_pass && context_raw.draw_texels) {
				NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
				NodeShader.write(frag, 'vec2 texel0 = texCoord * texpaintSize * 0.01;');
				NodeShader.write(frag, 'vec2 texel1 = texCoord * texpaintSize * 0.1;');
				NodeShader.write(frag, 'vec2 texel2 = texCoord * texpaintSize;');
				NodeShader.write(frag, 'basecol *= max(float(mod(int(texel0.x), 2.0) == mod(int(texel0.y), 2.0)), 0.9);');
				NodeShader.write(frag, 'basecol *= max(float(mod(int(texel1.x), 2.0) == mod(int(texel1.y), 2.0)), 0.9);');
				NodeShader.write(frag, 'basecol *= max(float(mod(int(texel2.x), 2.0) == mod(int(texel2.y), 2.0)), 0.9);');
			}

			if (last_pass && context_raw.draw_wireframe) {
				NodeShader.write(frag, 'basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
			}

			if (MakeMaterial.height_used) {
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

			if (!last_pass) {
				NodeShader.write(frag, 'fragColor[0] = vec4(basecol, opacity);');
				NodeShader.write(frag, 'fragColor[1] = vec4(ntex, matid);');
				NodeShader.write(frag, 'fragColor[2] = vec4(occlusion, roughness, metallic, height);');
				ParserMaterial.finalize(con_mesh);
				con_mesh.data.shader_from_source = true;
				con_mesh.data.vertex_shader = NodeShader.get(vert);
				con_mesh.data.fragment_shader = NodeShader.get(frag);
				return con_mesh;
			}

			frag.vvec = true;
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			///else
			NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			///end
			NodeShader.write(frag, 'n = ntex * 2.0 - 1.0;');
			NodeShader.write(frag, 'n.y = -n.y;');
			NodeShader.write(frag, 'n = normalize(mul(n, TBN));');

			if (context_raw.viewport_mode == viewport_mode_t.LIT || context_raw.viewport_mode == viewport_mode_t.PATH_TRACE) {
				NodeShader.write(frag, 'basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');

				if (context_raw.viewport_shader != null) {
					let color: string = context_raw.viewport_shader(frag);
					NodeShader.write(frag, `fragColor[1] = vec4(${color}, 1.0);`);
				}
				else if (context_raw.render_mode == render_mode_t.FORWARD && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
					frag.wposition = true;
					NodeShader.write(frag, 'vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);');
					NodeShader.write(frag, 'vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);');
					frag.vvec = true;
					NodeShader.write(frag, 'float dotNV = max(0.0, dot(n, vVec));');
					NodeShader.write(frag, 'vec2 envBRDF = texelFetch(senvmapBrdf, ivec2(vec2(roughness, 1.0 - dotNV) * 256.0), 0).xy;');
					NodeShader.add_uniform(frag, 'int envmapNumMipmaps', '_envmap_num_mipmaps');
					NodeShader.add_uniform(frag, 'vec4 envmapData', '_envmapData'); // angle, sin(angle), cos(angle), strength
					NodeShader.write(frag, 'vec3 wreflect = reflect(-vVec, n);');
					NodeShader.write(frag, 'float envlod = roughness * float(envmapNumMipmaps);');
					NodeShader.add_function(frag, str_envmap_equirect);
					NodeShader.write(frag, 'vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(wreflect, envmapData.x), envlod).rgb;');
					NodeShader.add_uniform(frag, 'vec3 lightArea0', '_light_area0');
					NodeShader.add_uniform(frag, 'vec3 lightArea1', '_light_area1');
					NodeShader.add_uniform(frag, 'vec3 lightArea2', '_light_area2');
					NodeShader.add_uniform(frag, 'vec3 lightArea3', '_light_area3');
					NodeShader.add_function(frag, str_ltc_evaluate);
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
					NodeShader.add_function(frag, str_sh_irradiance());
					NodeShader.write(frag, 'vec3 indirect = albedo * (shIrradiance(vec3(n.x * envmapData.z - n.y * envmapData.y, n.x * envmapData.y + n.y * envmapData.z, n.z), shirr) / 3.14159265);');
					NodeShader.write(frag, 'indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;');
					NodeShader.write(frag, 'indirect *= envmapData.w * occlusion;');
					NodeShader.write(frag, 'fragColor[1] = vec4(direct + indirect, 1.0);');
				}
				else { // Deferred, Pathtraced
					if (MakeMaterial.emis_used) NodeShader.write(frag, 'if (int(matid * 255.0) % 3 == 1) basecol *= 10.0;'); // Boost for bloom
					NodeShader.write(frag, 'fragColor[1] = vec4(basecol, occlusion);');
				}
			}
			else if (context_raw.viewport_mode == viewport_mode_t.BASE_COLOR && context_raw.layer.paint_base) {
				NodeShader.write(frag, 'fragColor[1] = vec4(basecol, 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.NORMAL_MAP && context_raw.layer.paint_nor) {
				NodeShader.write(frag, 'fragColor[1] = vec4(ntex.rgb, 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.OCCLUSION && context_raw.layer.paint_occ) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.ROUGHNESS && context_raw.layer.paint_rough) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.METALLIC && context_raw.layer.paint_met) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.OPACITY && context_raw.layer.paint_opac) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.HEIGHT && context_raw.layer.paint_height) {
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(height, height, height), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.EMISSION) {
				NodeShader.write(frag, 'float emis = int(matid * 255.0) % 3 == 1 ? 1.0 : 0.0;');
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(emis, emis, emis), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.SUBSURFACE) {
				NodeShader.write(frag, 'float subs = int(matid * 255.0) % 3 == 2 ? 1.0 : 0.0;');
				NodeShader.write(frag, 'fragColor[1] = vec4(vec3(subs, subs, subs), 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.TEXCOORD) {
				NodeShader.write(frag, 'fragColor[1] = vec4(texCoord, 0.0, 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_NORMAL) {
				frag.nattr = true;
				NodeShader.write(frag, 'fragColor[1] = vec4(nAttr, 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.MATERIAL_ID) {
				NodeShader.add_shared_sampler(frag, 'sampler2D texpaint_nor' + context_raw.layer.id);
				NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
				NodeShader.write(frag, 'float sample_matid = texelFetch(texpaint_nor' + context_raw.layer.id + ', ivec2(texCoord * texpaintSize), 0).a + 1.0 / 255.0;');
				NodeShader.write(frag, 'float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'fragColor[1] = vec4(matid_r, matid_g, matid_b, 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.OBJECT_ID) {
				NodeShader.add_uniform(frag, 'float objectId', '_objectId');
				NodeShader.write(frag, 'float obid = objectId + 1.0 / 255.0;');
				NodeShader.write(frag, 'float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				NodeShader.write(frag, 'fragColor[1] = vec4(id_r, id_g, id_b, 1.0);');
			}
			else if (context_raw.viewport_mode == viewport_mode_t.MASK && (SlotLayer.get_masks(context_raw.layer) != null || SlotLayer.is_mask(context_raw.layer))) {
				if (SlotLayer.is_mask(context_raw.layer)) {
					NodeShader.write(frag, 'float mask_view = textureLodShared(texpaint' + context_raw.layer.id + ', texCoord, 0.0).r;');
				}
				else {
					NodeShader.write(frag, 'float mask_view = 0.0;');
					for (let m of SlotLayer.get_masks(context_raw.layer)) {
						if (!SlotLayer.is_visible(m)) continue;
						NodeShader.write(frag, 'float mask_sample' + m.id + ' = textureLodShared(texpaint_view_mask' + m.id + ', texCoord, 0.0).r;');
						NodeShader.write(frag, 'mask_view = ' + MakeMaterial.blend_mode_mask(frag, m.blending, 'mask_view', 'mask_sample' + m.id, 'float(' + SlotLayer.get_opacity(m) + ')') + ';');
					}
				}
				NodeShader.write(frag, 'fragColor[1] = vec4(mask_view, mask_view, mask_view, 1.0);');
			}
			else {
				NodeShader.write(frag, 'fragColor[1] = vec4(1.0, 0.0, 1.0, 1.0);'); // Pink
			}

			if (context_raw.viewport_mode != viewport_mode_t.LIT && context_raw.viewport_mode != viewport_mode_t.PATH_TRACE) {
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

	static get_max_textures = (): i32 => {
		///if krom_direct3d11
		return 128 - 66;
		///else
		return 16 - 3; // G4onG5/G4.c.h MAX_TEXTURES
		///end
	}
}
