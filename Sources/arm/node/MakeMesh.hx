package arm.node;

import arm.ui.UISidebar;
import arm.shader.MaterialParser;
import arm.shader.NodeShader;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;
import arm.shader.ShaderFunctions;
import arm.data.LayerSlot;
import arm.Enums;

class MakeMesh {

	public static var layerPassCount = 1;

	public static function run(data: NodeShaderData, layerPass = 0): NodeShaderContext {
		var context_id = layerPass == 0 ? "mesh" : "mesh" + layerPass;
		var con_mesh: NodeShaderContext = data.add_context({
			name: context_id,
			depth_write: layerPass == 0 ? true : false,
			compare_mode: layerPass == 0 ? "less" : "equal",
			cull_mode: (Context.cullBackfaces || layerPass > 0) ? "clockwise" : "none",
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

		var textureCount = 0;
		if (MakeMaterial.heightUsed) {
			var displaceStrength = MakeMaterial.getDisplaceStrength();
			if (displaceStrength > 0.0) {
				vert.n = true;
				vert.write('float height = 0.0;');
				var numLayers = 0;
				for (l in Project.layers) {
					if (!l.isVisible() || !l.paintHeight || l.getChildren() != null) continue;
					if (numLayers > 16) break;
					numLayers++;
					textureCount++;
					vert.add_uniform('sampler2D texpaint_pack_vert' + l.id, '_texpaint_pack_vert' + l.id);
					vert.write('height += textureLod(texpaint_pack_vert' + l.id + ', tex, 0.0).a;');
					if (l.texpaint_mask != null) {
						textureCount++;
						vert.add_uniform('sampler2D texpaint_mask_vert' + l.id, '_texpaint_mask_vert' + l.id);
						vert.write('height *= textureLod(texpaint_mask_vert' + l.id + ', tex, 0.0).r;');
					}
				}
				vert.write('wposition += wnormal * vec3(height, height, height) * vec3($displaceStrength, $displaceStrength, $displaceStrength);');
			}
		}

		vert.write('gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		vert.write('texCoord = tex;');
		if (MakeMaterial.heightUsed) {
			vert.add_uniform('mat4 invW', '_inverseWorldMatrix');
			vert.write('prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			vert.write('prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;
		frag.add_function(ShaderFunctions.str_packFloatInt16);

		if (Context.tool == ToolColorId) {
			textureCount++;
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));'); // met/rough
			frag.write('vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, 1.0);'); // occ
		}
		else {
			frag.add_function(ShaderFunctions.str_octahedronWrap);
			frag.add_function(ShaderFunctions.str_cotangentFrame);
			if (layerPass > 0) {
				frag.add_uniform('sampler2D gbuffer0');
				frag.add_uniform('sampler2D gbuffer1');
				frag.add_uniform('sampler2D gbuffer2');
				frag.write('vec2 fragcoord = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
				#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				frag.write('fragcoord.y = 1.0 - fragcoord.y;');
				#end
				frag.write('vec4 gbuffer0_sample = textureLod(gbuffer0, fragcoord, 0.0);');
				frag.write('vec4 gbuffer1_sample = textureLod(gbuffer1, fragcoord, 0.0);');
				frag.write('vec4 gbuffer2_sample = textureLod(gbuffer2, fragcoord, 0.0);');
				frag.write('vec3 basecol = gbuffer0_sample.rgb;');
				frag.write('float roughness = gbuffer2_sample.g;');
				frag.write('float metallic = gbuffer2_sample.b;');
				frag.write('float occlusion = gbuffer2_sample.r;');
				frag.write('float opacity = 1.0;//gbuffer0_sample.a;');
				frag.write('float matid = gbuffer1_sample.a;');
				frag.write('vec3 ntex = gbuffer1_sample.rgb;');
				frag.write('float height = gbuffer2_sample.a;');
			}
			else {
				frag.write('vec3 basecol = vec3(0.0, 0.0, 0.0);');
				frag.write('float roughness = 0.0;');
				frag.write('float metallic = 0.0;');
				frag.write('float occlusion = 1.0;');
				frag.write('float opacity = 1.0;');
				frag.write('float matid = 0.0;');
				frag.write('vec3 ntex = vec3(0.5, 0.5, 1.0);');
				frag.write('float height = 0.0;');
			}
			frag.write('vec3 n0;');
			frag.write('vec4 texpaint_sample = vec4(0.0, 0.0, 0.0, 1.0);');
			frag.write('vec4 texpaint_nor_sample;');
			frag.write('vec4 texpaint_pack_sample;');
			frag.write('float texpaint_opac;');
			frag.vVec = true;
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			if (Context.drawWireframe) {
				textureCount++;
				frag.add_uniform('sampler2D texuvmap', '_texuvmap');
			}

			if (Context.viewportMode == ViewMask && Context.layer.texpaint_mask != null) {
				textureCount++;
				frag.add_uniform('sampler2D texpaint_mask_view', '_texpaint_mask');
			}

			if (Context.viewportMode == ViewLit && Context.renderMode == RenderForward) {
				textureCount += 4;
				frag.add_uniform('sampler2D senvmapBrdf', "$brdf.k");
				frag.add_uniform('sampler2D senvmapRadiance', '_envmapRadiance');
				frag.add_uniform('sampler2D sltcMat', '_ltcMat');
				frag.add_uniform('sampler2D sltcMag', '_ltcMag');
			}

			// Get layers for this pass
			layerPassCount = 1;
			var layers: Array<LayerSlot> = [];
			var startCount = textureCount;
			for (l in Project.layers) {
				if (l.isVisible() && l.getChildren() == null) {
					var count = l.texpaint_mask != null ? 4 : 3;
					textureCount += count;
					if (textureCount >= getMaxTextures()) {
						textureCount = startCount + count + 3; // gbuffer0_copy, gbuffer1_copy, gbuffer2_copy
						layerPassCount++;
					}
					if (layerPass == layerPassCount - 1) {
						layers.push(l);
					}
				}
			}

			for (l in layers) {
				if (l.objectMask > 0) {
					var uid = Project.paintObjects[l.objectMask - 1].uid;
					frag.add_uniform('int uid', '_uid');
					frag.write('if ($uid == uid) {');
				}

				frag.add_shared_sampler('sampler2D texpaint' + l.id);
				frag.write('texpaint_sample = textureLodShared(texpaint' + l.id + ', texCoord, 0.0);');
				frag.write('texpaint_opac = texpaint_sample.a;');
				// #if (kha_direct3d12 || kha_vulkan)
				// if (Context.viewportMode == ViewLit) {
				// 	frag.write('if (texpaint_opac < 0.1) discard;');
				// }
				// #end

				if (l.texpaint_mask != null) {
					frag.add_shared_sampler('sampler2D texpaint_mask' + l.id);
					frag.write('texpaint_opac *= textureLodShared(texpaint_mask' + l.id + ', texCoord, 0.0).r;');
				}

				if (l.maskOpacity < 1) {
					frag.write('texpaint_opac *= ${l.maskOpacity};');
				}

				if (l.paintBase) {
					if (l == Project.layers[0]) {
						frag.write('basecol = texpaint_sample.rgb * texpaint_opac;');
					}
					else {
						frag.write('basecol = ' + MakeMaterial.blendMode(frag, l.blending, 'basecol', 'texpaint_sample.rgb', 'texpaint_opac') + ';');
					}
				}

				if (l.paintNor || MakeMaterial.emisUsed) {
					frag.add_shared_sampler('sampler2D texpaint_nor' + l.id);
					frag.write('texpaint_nor_sample = textureLodShared(texpaint_nor' + l.id + ', texCoord, 0.0);');

					if (MakeMaterial.emisUsed) {
						frag.write('matid = texpaint_nor_sample.a;');
					}

					if (l.paintNor) {
						frag.write('ntex = texpaint_nor_sample.rgb;');
						frag.write('n0 = ntex * 2.0 - 1.0;');
						frag.write('n0.y = -n0.y;');
						frag.write('n0 = normalize(mul(n0, TBN));');
						frag.write('n = normalize(mix(n, n0, texpaint_opac));');
					}
				}

				if (l.paintOcc || l.paintRough || l.paintMet || (l.paintHeight && MakeMaterial.heightUsed)) {
					frag.add_shared_sampler('sampler2D texpaint_pack' + l.id);
					frag.write('texpaint_pack_sample = textureLodShared(texpaint_pack' + l.id + ', texCoord, 0.0);');

					if (l.paintOcc) {
						frag.write('occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);');
					}
					if (l.paintRough) {
						frag.write('roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);');
					}
					if (l.paintMet) {
						frag.write('metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);');
					}
					if (l.paintHeight && MakeMaterial.heightUsed) {
						var ds = MakeMaterial.getDisplaceStrength() * 5;
						if (ds < 0.1) ds = 0.1;
						else if (ds > 2.0) ds = 2.0;
						frag.wposition = true;
						frag.write('{');
						frag.write('vec3 dpdx = dFdx(wposition);');
						frag.write('vec3 dpdy = dFdy(wposition);');
						frag.write('height = texpaint_pack_sample.a * texpaint_opac;');
						frag.write('float dhdx = dFdx(height * $ds);');
						frag.write('float dhdy = dFdy(height * $ds);');
						frag.write('vec3 cross_x = cross(n, dpdx);');
						frag.write('vec3 cross_y = cross(dpdy, n);');
						frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
						frag.write('n = normalize(n - ngrad);');
						frag.write('}');

						// frag.add_uniform('vec2 texpaintSize', '_texpaintSize');
						// frag.write('float tex_step = 1.0 / texpaintSize.x;');
						// frag.wposition = true;
						// frag.write('{');
						// frag.write('float pack_a = textureLodShared(texpaint_pack' + l.id + ', vec2(texCoord.x + tex_step, texCoord.y), 0.0).a;');
						// frag.write('float pack_b = textureLodShared(texpaint_pack' + l.id + ', vec2(texCoord.x - tex_step, texCoord.y), 0.0).a;');
						// frag.write('float pack_c = textureLodShared(texpaint_pack' + l.id + ', vec2(texCoord.x, texCoord.y + tex_step), 0.0).a;');
						// frag.write('float pack_d = textureLodShared(texpaint_pack' + l.id + ', vec2(texCoord.x, texCoord.y - tex_step), 0.0).a;');
						// frag.write('vec3 dpdx = dFdx(wposition);');
						// frag.write('vec3 dpdy = dFdy(wposition);');
						// frag.write('float dhdx = pack_a - pack_b;');
						// frag.write('float dhdy = pack_c - pack_d;');
						// frag.write('vec3 cross_x = cross(n, dpdx);');
						// frag.write('vec3 cross_y = cross(dpdy, n);');
						// frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
						// frag.write('n = normalize(n - ngrad);');
						// frag.write('}');
					}
				}

				if (l.objectMask > 0) {
					frag.write('}');
				}
			}

			var lastPass = layerPass == layerPassCount - 1;

			if (lastPass && Context.drawTexels) {
				frag.add_uniform('vec2 texpaintSize', '_texpaintSize');
				frag.write('vec2 texel = texCoord * texpaintSize;');
				frag.write('basecol *= max(float(mod(int(texel.x), 2.0) == mod(int(texel.y), 2.0)), 0.9);');
			}

			if (lastPass && Context.drawWireframe) {
				frag.write('basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
			}

			if (lastPass && Context.renderMode == RenderForward) {
				frag.write('vec3 wn = n;');
			}

			if (!lastPass) {
				frag.write('fragColor[0] = vec4(basecol, opacity);');
				frag.write('fragColor[1] = vec4(ntex, matid);');
				frag.write('fragColor[2] = vec4(occlusion, roughness, metallic, height);');
				MaterialParser.finalize(con_mesh);
				con_mesh.data.shader_from_source = true;
				con_mesh.data.vertex_shader = vert.get();
				con_mesh.data.fragment_shader = frag.get();
				return con_mesh;
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
					frag.write('vec2 envBRDF = texture(senvmapBrdf, vec2(roughness, 1.0 - dotNV)).xy;');
					frag.add_uniform('int envmapNumMipmaps', '_envmapNumMipmaps');
					frag.add_uniform('vec4 envmapData', '_envmapData'); // angle, sin(angle), cos(angle), strength
					frag.write('vec3 wreflect = reflect(-vVec, wn);');
					frag.write('float envlod = roughness * float(envmapNumMipmaps);');
					frag.add_function(ShaderFunctions.str_envMapEquirect);
					frag.write('vec4 envmapDataLocal = envmapData;'); // TODO: spirv workaround
					frag.write('vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(wreflect, envmapDataLocal.x), envlod).rgb;');
					frag.add_uniform('vec3 lightArea0', '_lightArea0');
					frag.add_uniform('vec3 lightArea1', '_lightArea1');
					frag.add_uniform('vec3 lightArea2', '_lightArea2');
					frag.add_uniform('vec3 lightArea3', '_lightArea3');
					frag.add_function(ShaderFunctions.str_ltcEvaluate);
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
					frag.add_function(ShaderFunctions.str_shIrradiance);
					frag.write('vec3 indirect = albedo * (shIrradiance(vec3(wn.x * envmapDataLocal.z - wn.y * envmapDataLocal.y, wn.x * envmapDataLocal.y + wn.y * envmapDataLocal.z, wn.z), shirr) / 3.14159265);');
					frag.write('indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;');
					frag.write('indirect *= envmapDataLocal.w * occlusion;');
					frag.write('fragColor[1] = vec4(direct + indirect, 1.0);');
				}
				else { // Deferred, Pathtraced
					if (MakeMaterial.emisUsed) frag.write('if (matid == 1.0) basecol *= 10.0;'); // Boost for bloom
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
				frag.add_shared_sampler('sampler2D texpaint_nor' + Context.layer.id);
				frag.write('float sample_matid = textureLodShared(texpaint_nor' + Context.layer.id + ', texCoord, 0.0).a + 1.0 / 255.0;');
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

		MaterialParser.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();
		return con_mesh;
	}

	static inline function getMaxTextures(): Int {
		#if (kha_direct3d11 || kha_metal)
		return 128 - 66;
		#elseif (kha_direct3d12 || kha_vulkan)
		// CommandList5Impl->textureCount = 16;
		return 16 - 3;
		#else
		return 16 - 3;
		#end
	}
}
