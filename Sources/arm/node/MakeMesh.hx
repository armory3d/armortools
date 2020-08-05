package arm.node;

import arm.ui.UISidebar;
import arm.node.MaterialShader;
import arm.Enums;

class MakeMesh {

	public static function run(data: MaterialShaderData): MaterialShaderContext {
		var context_id = "mesh";
		var con_mesh: MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: "less",
			cull_mode: Context.cullBackfaces ? "clockwise" : "none",
			vertex_elements: [{name: "pos", data: "short4norm"},{name: "nor", data: "short2norm"},{name: "tex", data: "short2norm"}],
			color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
			depth_attachment: "DEPTH32"
		});

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();
		frag.ins = vert.outs;

		vert.add_out('vec2 texCoord');
		frag.wvpposition = true;
		vert.add_out('vec4 prevwvpposition');
		vert.add_uniform('mat4 VP', '_viewProjectionMatrix');
		vert.add_uniform('mat4 prevWVP', '_prevWorldViewProjectionMatrix');
		vert.wposition = true;

		// Height
		// TODO: can cause TAA issues
		if (MaterialBuilder.heightUsed) {
			var displaceStrength = MaterialBuilder.getDisplaceStrength();
			vert.n = true;
			vert.write('float height = 0.0;');
			var numLayers = 0;
			for (l in Project.layers) {
				if (!l.isVisible() || !l.paintHeight || l.getChildren() != null) continue;
				if (numLayers > 16) break;
				numLayers++;
				vert.add_uniform('sampler2D texpaint_pack_vert' + l.id, '_texpaint_pack_vert' + l.id);
				vert.write('height += textureLod(texpaint_pack_vert' + l.id + ', tex, 0.0).a;');
				if (l.texpaint_mask != null) {
					vert.add_uniform('sampler2D texpaint_mask_vert' + l.id, '_texpaint_mask_vert' + l.id);
					vert.write('height *= textureLod(texpaint_mask_vert' + l.id + ', tex, 0.0).r;');
				}
			}
			vert.write('wposition += wnormal * vec3(height, height, height) * vec3($displaceStrength, $displaceStrength, $displaceStrength);');
		}
		//

		vert.write('gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		vert.write('texCoord = tex;');
		if (MaterialBuilder.heightUsed) {
			vert.add_uniform('mat4 invW', '_inverseWorldMatrix');
			vert.write('prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			vert.write('prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;

		frag.add_function(MaterialFunctions.str_packFloatInt16);

		if (Context.tool == ToolColorId) {
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));'); // met/rough
			frag.write('vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, 1.0);'); // occ
		}
		else {
			frag.add_function(MaterialFunctions.str_octahedronWrap);

			frag.write('vec3 basecol;');
			frag.write('float roughness;');
			frag.write('float metallic;');
			frag.write('float occlusion;');
			frag.write('float opacity;');
			frag.write('float matid = 0.0;');

			frag.vVec = true;
			frag.add_function(MaterialFunctions.str_cotangentFrame);
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			if (Project.layers[0].isVisible()) {
				var l = Project.layers[0];

				if (l.objectMask > 0) {
					var uid = Project.paintObjects[l.objectMask - 1].uid;
					frag.add_uniform('int objectId', '_uid');
					frag.write('if ($uid == objectId) {');
				}

				if (l.paintBase) {
					frag.add_shared_sampler('sampler2D texpaint');
					frag.write('vec4 texpaint_sample = textureLodShared(texpaint, texCoord, 0.0);');
					#if (kha_direct3d12 || kha_vulkan)
					if (Context.viewportMode == ViewLit) {
						frag.write('if (texpaint_sample.a < 0.1) discard;');
					}
					#end
				}
				else {
					frag.write('vec4 texpaint_sample = vec4(0.0, 0.0, 0.0, 1.0);');
				}
				frag.write('basecol = texpaint_sample.rgb * texpaint_sample.a;');

				if (l.texpaint_mask != null) {
					frag.add_shared_sampler('sampler2D texpaint_mask');
					frag.write('float maskTexture = textureLodShared(texpaint_mask, texCoord, 0.0).r;');
				}

				if (l.paintNor || MaterialBuilder.emisUsed) {
					frag.add_shared_sampler('sampler2D texpaint_nor');
					frag.write('vec4 texpaint_nor_sample = textureLodShared(texpaint_nor, texCoord, 0.0);');

					if (MaterialBuilder.emisUsed) {
						frag.write('matid = texpaint_nor_sample.a;');
					}

					if (l.paintNor) {
						frag.write('vec3 ntex = texpaint_nor_sample.rgb;');
						frag.write('n = ntex * 2.0 - 1.0;');
						frag.write('n.y = -n.y;');
						if (l.texpaint_mask != null) {
							frag.write('n.xy *= maskTexture;');
						}
						frag.write('n = normalize(mul(n, TBN));');
					}
				}

				if ((l.paintHeight && MaterialBuilder.heightUsed) ||
					l.paintOcc ||
					l.paintRough ||
					l.paintMet) {
					frag.add_shared_sampler('sampler2D texpaint_pack');
					frag.write('vec4 pack = textureLodShared(texpaint_pack, texCoord, 0.0);');
				}

				// Height
				if (l.paintHeight && MaterialBuilder.heightUsed) {
					var ds = MaterialBuilder.getDisplaceStrength() * 5;
					if (ds < 0.1) ds = 0.1;
					else if (ds > 2.0) ds = 2.0;
					frag.wposition = true;
					frag.write('vec3 dpdx = dFdx(wposition);');
					frag.write('vec3 dpdy = dFdy(wposition);');
					frag.write('float height_sample = pack.a;');
					if (l.texpaint_mask != null) {
						frag.write('height_sample *= maskTexture;');
					}
					frag.write('float dhdx = dFdx(height_sample * $ds);');
					frag.write('float dhdy = dFdy(height_sample * $ds);');
					frag.write('vec3 cross_x = cross(n, dpdx);');
					frag.write('vec3 cross_y = cross(dpdy, n);');
					frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
					frag.write('n = normalize(n - ngrad);');

					// frag.add_uniform('vec2 texpaintSize', '_texpaintSize');
					// frag.write('float tex_step = 1.0 / texpaintSize.x;');
					// frag.wposition = true;
					// frag.write('float pack_a = textureLodShared(texpaint_pack, vec2(texCoord.x + tex_step, texCoord.y), 0.0).a;');
					// frag.write('float pack_b = textureLodShared(texpaint_pack, vec2(texCoord.x - tex_step, texCoord.y), 0.0).a;');
					// frag.write('float pack_c = textureLodShared(texpaint_pack, vec2(texCoord.x, texCoord.y + tex_step), 0.0).a;');
					// frag.write('float pack_d = textureLodShared(texpaint_pack, vec2(texCoord.x, texCoord.y - tex_step), 0.0).a;');
					// frag.write('vec3 dpdx = dFdx(wposition);');
					// frag.write('vec3 dpdy = dFdy(wposition);');
					// frag.write('float dhdx = pack_a - pack_b;');
					// frag.write('float dhdy = pack_c - pack_d;');
					// frag.write('vec3 cross_x = cross(n, dpdx);');
					// frag.write('vec3 cross_y = cross(dpdy, n);');
					// frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
					// frag.write('n = normalize(n - ngrad);');
				}
				//

				if (l.paintOcc) {
					frag.write('occlusion = pack.r;');
				}
				else {
					frag.write('occlusion = 1.0;');
				}
				if (l.paintRough) {
					frag.write('roughness = pack.g;');
				}
				else {
					frag.write('roughness = 1.0;');
				}
				if (l.paintMet) {
					frag.write('metallic = pack.b;');
				}
				else {
					frag.write('metallic = 0.0;');
				}

				if (l.texpaint_mask != null) {
					frag.write('basecol *= maskTexture;');
					frag.write('occlusion *= maskTexture;');
					frag.write('roughness *= maskTexture;');
					frag.write('metallic *= maskTexture;');
				}

				if (l.maskOpacity < 1) {
					frag.write('basecol *= ${l.maskOpacity};');
					frag.write('occlusion *= ${l.maskOpacity};');
					frag.write('roughness *= ${l.maskOpacity};');
					frag.write('metallic *= ${l.maskOpacity};');
				}

				if (l.objectMask > 0) {
					frag.write('}');
					frag.write('else {');
					frag.write('basecol = vec3(0.0, 0.0, 0.0);');
					frag.write('occlusion = 1.0;');
					frag.write('roughness = 0.0;');
					frag.write('metallic = 0.0;');
					frag.write('}');
				}
			}
			else {
				frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 0.0;');
				frag.write('metallic = 0.0;');
			}

			if (Project.layers.length > 1) {
				frag.write('float factor0;');
				frag.write('vec4 col_tex0;');
				frag.write('vec4 col_nor0;');
				frag.write('vec4 col_pack0;');
				frag.write('vec3 n0;');
				var len = Project.layers.length;
				var start = len - 1;
				var maxLayers = getMaxVisibleLayers();
				var count = 1;
				for (i in 1...len) {
					if (start == 1) break;
					start--;
					var l = Project.layers[len - i];
					if (l.isVisible() && l.getChildren() == null) {
						count++;
						if (count >= maxLayers) break;
					}
				}
				for (i in start...len) {
					var l = Project.layers[i];
					if (!l.isVisible() || l.getChildren() != null) continue;
					var id = l.id;

					if (l.objectMask > 0) {
						var uid = Project.paintObjects[l.objectMask - 1].uid;
						frag.add_uniform('int objectId', '_uid');
						frag.write('if ($uid == objectId) {');
					}

					frag.add_shared_sampler('sampler2D texpaint' + id);
					frag.write('col_tex0 = textureLodShared(texpaint' + id + ', texCoord, 0.0);');
					frag.write('factor0 = col_tex0.a;');

					if (l.texpaint_mask != null) {
						frag.add_shared_sampler('sampler2D texpaint_mask' + id);
						frag.write('factor0 *= textureLodShared(texpaint_mask' + id + ', texCoord, 0.0).r;');
					}
					if (l.maskOpacity < 1) {
						frag.write('factor0 *= ${l.maskOpacity};');
					}

					if (l.paintBase) {
						frag.write('basecol = ' + MaterialBuilder.blendMode(frag, l.blending, 'basecol', 'col_tex0.rgb', 'factor0') + ';');
					}

					if (MaterialBuilder.emisUsed || l.paintNor) {

						frag.add_shared_sampler('sampler2D texpaint_nor' + id);
						frag.write('col_nor0 = textureLodShared(texpaint_nor' + id + ', texCoord, 0.0);');

						if (MaterialBuilder.emisUsed) {
							frag.write('matid = col_nor0.a;');
						}

						if (l.paintNor) {
							frag.write('n0 = col_nor0.rgb * 2.0 - 1.0;');
							frag.write('n0.y = -n0.y;');
							frag.write('n0 = normalize(mul(n0, TBN));');
							frag.write('n = normalize(mix(n, n0, factor0));');
						}
					}

					if (l.paintOcc || l.paintRough || l.paintMet) {
						frag.add_shared_sampler('sampler2D texpaint_pack' + id);
						frag.write('col_pack0 = textureLodShared(texpaint_pack' + id + ', texCoord, 0.0);');

						if (l.paintOcc) {
							frag.write('occlusion = mix(occlusion, col_pack0.r, factor0);');
						}
						if (l.paintRough) {
							frag.write('roughness = mix(roughness, col_pack0.g, factor0);');
						}
						if (l.paintMet) {
							frag.write('metallic = mix(metallic, col_pack0.b, factor0);');
						}
					}

					if (l.objectMask > 0) {
						frag.write('}');
					}
				}
			}

			if (Context.drawTexels) {
				frag.add_uniform('vec2 texpaintSize', '_texpaintSize');
				frag.write('vec2 texel = texCoord * texpaintSize;');
				frag.write('basecol *= max(float(mod(int(texel.x), 2.0) == mod(int(texel.y), 2.0)), 0.9);');
			}

			if (Context.drawWireframe) {
				// GL_NV_fragment_shader_barycentric
				// VK_AMD_shader_explicit_vertex_parameter
				frag.add_uniform('sampler2D texuvmap', '_texuvmap');
				frag.write('basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
				// frag.write('if (basecol == vec3(0,0,0)) discard;');
			}

			if (Context.renderMode == RenderForward) {
				frag.write('vec3 wn = n;');
			}

			frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
			frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
			frag.write('basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(matid)));');

			if (Context.viewportMode == ViewLit || Context.viewportMode == ViewPathTrace) {
				if (Context.renderMode == RenderForward) {
					frag.wposition = true;
					frag.write('vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);');
					frag.write('vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);');
					frag.write('float dotNV = max(dot(wn, vVec), 0.0);');
					frag.add_uniform('sampler2D senvmapBrdf', "$brdf.k");
					frag.write('vec2 envBRDF = texture(senvmapBrdf, vec2(roughness, 1.0 - dotNV)).xy;');
					frag.add_uniform('sampler2D senvmapRadiance', '_envmapRadiance');
					frag.add_uniform('int envmapNumMipmaps', '_envmapNumMipmaps');
					frag.add_uniform('vec4 envmapData', '_envmapData'); // angle, sin(angle), cos(angle), strength
					frag.write('vec3 wreflect = reflect(-vVec, wn);');
					frag.write('float envlod = roughness * float(envmapNumMipmaps);');
					frag.add_function(MaterialFunctions.str_envMapEquirect);
					frag.write('vec4 envmapDataLocal = envmapData;'); // TODO: spirv workaround
					frag.write('vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(wreflect, envmapDataLocal.x), envlod).rgb;');

					frag.add_uniform('vec3 lightArea0', '_lightArea0');
					frag.add_uniform('vec3 lightArea1', '_lightArea1');
					frag.add_uniform('vec3 lightArea2', '_lightArea2');
					frag.add_uniform('vec3 lightArea3', '_lightArea3');
					frag.add_uniform('sampler2D sltcMat', '_ltcMat');
					frag.add_uniform('sampler2D sltcMag', '_ltcMag');
					frag.add_function(MaterialFunctions.str_ltcEvaluate);
					frag.add_uniform('vec3 lightPos', '_pointPosition');
					frag.add_uniform('vec3 lightColor', '_pointColor');
					// frag.write('float dotNL = max(dot(wn, normalize(lightPos - wposition)), 0.0);');
					// frag.write('vec3 direct = albedo * dotNL;');
					frag.write('float ldist = distance(wposition, lightPos);');
					frag.write('const float LUT_SIZE = 64.0;');
					frag.write('const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;');
					frag.write('const float LUT_BIAS = 0.5 / LUT_SIZE;');
					frag.write('float theta = acos(dotNV);');
					frag.write('vec2 tuv = vec2(roughness, theta / (0.5 * 3.14159265));');
					frag.write('tuv = tuv * LUT_SCALE + LUT_BIAS;');
					frag.write('vec4 t = textureLod(sltcMat, tuv, 0.0);');
					frag.write('mat3 minv = mat3(vec3(1.0, 0.0, t.y), vec3(0.0, t.z, 0.0), vec3(t.w, 0.0, t.x));');
					frag.write('float ltcspec = ltcEvaluate(wn, vVec, dotNV, wposition, minv, lightArea0, lightArea1, lightArea2, lightArea3);');
					frag.write('ltcspec *= textureLod(sltcMag, tuv, 0.0).a;');
					frag.write('mat3 mident = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);');
					frag.write('float ltcdiff = ltcEvaluate(wn, vVec, dotNV, wposition, mident, lightArea0, lightArea1, lightArea2, lightArea3);');
					frag.write('vec3 direct = albedo * ltcdiff + ltcspec * 0.05;');
					frag.write('direct *= lightColor * (1.0 / (ldist * ldist));');

					frag.add_uniform('vec4 shirr[7]', '_envmapIrradiance');
					frag.add_function(MaterialFunctions.str_shIrradiance);
					frag.write('vec3 indirect = albedo * (shIrradiance(vec3(wn.x * envmapDataLocal.z - wn.y * envmapDataLocal.y, wn.x * envmapDataLocal.y + wn.y * envmapDataLocal.z, wn.z), shirr) / 3.14159265);');
					frag.write('indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;');
					frag.write('indirect *= envmapDataLocal.w * occlusion;');
					frag.write('fragColor[1] = vec4(direct + indirect, 1.0);');
				}
				else { // Deferred, Pathtraced
					if (MaterialBuilder.emisUsed) frag.write('if (matid == 1.0) basecol *= 10.0;'); // Boost for bloom
					frag.write('fragColor[1] = vec4(basecol, occlusion);');
				}
			}
			else if (Context.viewportMode == ViewBaseColor && Context.layer.paintBase) {
				frag.write('fragColor[1] = vec4(basecol, 1.0);');
			}
			else if (Context.viewportMode == ViewNormalMap && Context.layer.paintNor) {
				frag.write('fragColor[1] = vec4(ntex.rgb, 1.0);');
			}
			else if (Context.viewportMode == ViewOcclusion && Context.layer.paintOcc) {
				frag.write('fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);');
			}
			else if (Context.viewportMode == ViewRoughness && Context.layer.paintRough) {
				frag.write('fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);');
			}
			else if (Context.viewportMode == ViewMetallic && Context.layer.paintMet) {
				frag.write('fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);');
			}
			else if (Context.viewportMode == ViewOpacity && Context.layer.paintOpac) {
				frag.write('fragColor[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);');
			}
			else if (Context.viewportMode == ViewTexCoord) {
				frag.write('fragColor[1] = vec4(texCoord, 0.0, 1.0);');
			}
			else if (Context.viewportMode == ViewObjectNormal) {
				frag.nAttr = true;
				frag.write('fragColor[1] = vec4(nAttr, 1.0);');
			}
			else if (Context.viewportMode == ViewMaterialID) {
				frag.write('float sample_matid = textureLodShared(texpaint_nor, texCoord, 0.0).a + 1.0 / 255.0;');
				frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[1] = vec4(matid_r, matid_g, matid_b, 1.0);');
			}
			else if (Context.viewportMode == ViewObjectID) {
				frag.add_uniform('float objectId', '_objectId');
				frag.write('float obid = objectId + 1.0 / 255.0;');
				frag.write('float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[1] = vec4(id_r, id_g, id_b, 1.0);');
			}
			else if (Context.viewportMode == ViewMask && Context.layer.texpaint_mask != null) {
				frag.add_uniform('sampler2D texpaint_mask_view', '_texpaint_mask');
				frag.write('float sample_mask = textureLod(texpaint_mask_view, texCoord, 0.0).r;');
				frag.write('fragColor[1] = vec4(sample_mask, sample_mask, sample_mask, 1.0);');
			}
			else {
				frag.write('fragColor[1] = vec4(1.0, 0.0, 1.0, 1.0);'); // Pink
			}
		}

		frag.write('vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		frag.write('vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		frag.write('fragColor[2] = vec4(posa - posb, texCoord.xy);');

		Material.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();
		return con_mesh;
	}

	static function getMaxVisibleLayers(): Int {
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal)
		// 128 texture slots available
		// 4 textures per layer (3 + 1 mask)
		// 32 layers - base + 31 on top
		return 31;
		#else
		// 32 texture slots available
		return 4; // base + 4 on top
		#end
	}
}
