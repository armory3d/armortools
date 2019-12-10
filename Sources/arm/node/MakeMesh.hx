package arm.node;

import arm.ui.UITrait;
import arm.node.MaterialShader;
import arm.Tool;

class MakeMesh {

	public static function run(data: MaterialShaderData): MaterialShaderContext {
		var context_id = "mesh";
		var con_mesh: MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: "less",
			cull_mode: UITrait.inst.cullBackfaces ? "clockwise" : "none",
			vertex_elements: [{name: "pos", data: "short4norm"},{name: "nor", data: "short2norm"},{name: "tex", data: "short2norm"}] });

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
				if (!l.visible) continue;
				if (numLayers > 16) break;
				numLayers++;
				vert.add_uniform('sampler2D texpaint_pack_vert' + l.id, '_texpaint_pack_vert' + l.id);
				vert.write('height += textureLod(texpaint_pack_vert' + l.id + ', tex, 0.0).a;');
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
		frag.add_function(MaterialFunctions.str_packFloat2);

		if (Context.tool == ToolColorId) {
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));'); // met/rough
			frag.write('vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
		}
		else {
			frag.add_function(MaterialFunctions.str_octahedronWrap);

			frag.write('vec3 basecol;');
			frag.write('float roughness;');
			frag.write('float metallic;');
			frag.write('float occlusion;');
			frag.write('float opacity;');
			frag.write('float specular;');
			frag.write('float matid = 0.0;');

			frag.vVec = true;
			frag.add_function(MaterialFunctions.str_cotangentFrame);
			#if (kha_direct3d11 || kha_direct3d12)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			if (Project.layers[0].visible) {

				if (Context.layer.paintBase) {
					frag.add_shared_sampler('sampler2D texpaint');
					frag.write('basecol = textureLodShared(texpaint, texCoord, 0.0).rgb;');
				}
				else {
					frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				}

				if (Context.layer.paintNor || MaterialBuilder.emisUsed) {
					frag.add_shared_sampler('sampler2D texpaint_nor');
					frag.write('vec4 texpaint_nor_sample = textureLodShared(texpaint_nor, texCoord, 0.0);');

					if (MaterialBuilder.emisUsed) {
						frag.write('matid = texpaint_nor_sample.a;');
					}

					if (Context.layer.paintNor) {
						frag.write('vec3 ntex = texpaint_nor_sample.rgb;');
						frag.write('n = ntex * 2.0 - 1.0;');
						frag.write('n.y = -n.y;');
						frag.write('n = normalize(mul(n, TBN));');
					}
				}

				if (MaterialBuilder.heightUsed ||
					Context.layer.paintOcc ||
					Context.layer.paintRough ||
					Context.layer.paintMet) {
					frag.add_shared_sampler('sampler2D texpaint_pack');
					frag.write('vec4 pack = textureLodShared(texpaint_pack, texCoord, 0.0);');
				}

				// Height
				if (MaterialBuilder.heightUsed) {
					var ds = MaterialBuilder.getDisplaceStrength() * 5;
					if (ds < 0.1) ds = 0.1;
					else if (ds > 2.0) ds = 2.0;
					frag.wposition = true;
					frag.write('vec3 dpdx = dFdx(wposition);');
					frag.write('vec3 dpdy = dFdy(wposition);');
					frag.write('float dhdx = dFdx(pack.a * $ds);');
					frag.write('float dhdy = dFdy(pack.a * $ds);');
					frag.write('vec3 cross_x = cross(n, dpdx);');
					frag.write('vec3 cross_y = cross(dpdy, n);');
					frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
					frag.write('n = normalize(n - ngrad);');

					// frag.add_uniform('float texpaintSize', '_texpaintSize');
					// frag.write('float tex_step = 1.0 / texpaintSize;');
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

				if (Context.layer.paintOcc) {
					frag.write('occlusion = pack.r;');
				}
				else {
					frag.write('occlusion = 1.0;');
				}
				if (Context.layer.paintRough) {
					frag.write('roughness = pack.g;');
				}
				else {
					frag.write('roughness = 1.0;');
				}
				if (Context.layer.paintMet) {
					frag.write('metallic = pack.b;');
				}
				else {
					frag.write('metallic = 0.0;');
				}

				var l = Project.layers[0];
				if (l.maskOpacity < 1) {
					frag.write('basecol *= ${l.maskOpacity};');
					frag.write('occlusion *= ${l.maskOpacity};');
					frag.write('roughness *= ${l.maskOpacity};');
					frag.write('metallic *= ${l.maskOpacity};');
				}
			}
			else {
				frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 1.0;');
				frag.write('metallic = 0.0;');
				frag.write('specular = 1.0;');
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
					if (l.visible) {
						count++;
						if (count >= maxLayers) break;
					}
				}
				for (i in start...len) {
					var l = Project.layers[i];
					if (!l.visible) continue;
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

					if (Context.layer.paintBase) {
						frag.write('basecol = ' + MaterialBuilder.blendMode(frag, l.blending, 'basecol', 'col_tex0.rgb', 'factor0') + ';');
					}

					if (MaterialBuilder.emisUsed || Context.layer.paintNor) {

						frag.add_shared_sampler('sampler2D texpaint_nor' + id);
						frag.write('col_nor0 = textureLodShared(texpaint_nor' + id + ', texCoord, 0.0);');

						if (MaterialBuilder.emisUsed) {
							frag.write('matid = col_nor0.a;');
						}

						if (Context.layer.paintNor) {
							frag.write('n0 = col_nor0.rgb * 2.0 - 1.0;');
							frag.write('n0.y = -n0.y;');
							frag.write('n0 = normalize(mul(n0, TBN));');
							frag.write('n = normalize(mix(n, n0, factor0));');
						}
					}

					if (Context.layer.paintOcc || Context.layer.paintRough || Context.layer.paintMet) {
						frag.add_shared_sampler('sampler2D texpaint_pack' + id);
						frag.write('col_pack0 = textureLodShared(texpaint_pack' + id + ', texCoord, 0.0);');

						if (Context.layer.paintOcc) {
							frag.write('occlusion = mix(occlusion, col_pack0.r, factor0);');
						}
						if (Context.layer.paintRough) {
							frag.write('roughness = mix(roughness, col_pack0.g, factor0);');
						}
						if (Context.layer.paintMet) {
							frag.write('metallic = mix(metallic, col_pack0.b, factor0);');
						}
					}

					if (l.objectMask > 0) {
						frag.write('}');
					}
				}
			}

			if (UITrait.inst.drawTexels) {
				frag.add_uniform('float texpaintSize', '_texpaintSize');
				frag.write('vec2 texel = texCoord * texpaintSize;');
				frag.write('basecol *= max(float(mod(int(texel.x), 2.0) == mod(int(texel.y), 2.0)), 0.9);');
			}

			if (UITrait.inst.drawWireframe) {
				// GL_NV_fragment_shader_barycentric
				// VK_AMD_shader_explicit_vertex_parameter
				frag.add_uniform('sampler2D texuvmap', '_texuvmap');
				frag.write('basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
				// frag.write('if (basecol == vec3(0,0,0)) discard;');
			}

			frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
			frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
			frag.write('basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(matid)));');

			var deferred = UITrait.inst.viewportMode == ViewRender || UITrait.inst.viewportMode == ViewPathTrace;
			if (deferred) {
				if (MaterialBuilder.emisUsed) frag.write('if (matid == 1.0) basecol *= 10.0;'); // Boost for bloom
				frag.write('fragColor[1] = vec4(basecol, packFloat2(occlusion, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == ViewBaseColor) {
				frag.write('fragColor[1] = vec4(basecol, 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewNormalMap) {
				frag.write('fragColor[1] = vec4(ntex.rgb, 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewOcclusion) {
				frag.write('fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewRoughness) {
				frag.write('fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewMetallic) {
				frag.write('fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewTexCoord) {
				frag.write('fragColor[1] = vec4(texCoord, 0.0, 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewObjectNormal) {
				frag.nAttr = true;
				frag.write('fragColor[1] = vec4(nAttr, 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewMaterialID) {
				frag.write('float sample_matid = textureLodShared(texpaint_nor, texCoord, 0.0).a + 1.0 / 255.0;');
				frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[1] = vec4(matid_r, matid_g, matid_b, 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewObjectID) {
				frag.add_uniform('float objectId', '_objectId');
				frag.write('float obid = objectId + 1.0 / 255.0;');
				frag.write('float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[1] = vec4(id_r, id_g, id_b, 1.0);');
			}
			else if (UITrait.inst.viewportMode == ViewMask) {
				frag.write('float sample_mask = 1.0;');
				if (Context.layer.texpaint_mask != null) {
					frag.add_uniform('sampler2D texpaint_mask_view', '_texpaint_mask');
					frag.write('sample_mask = textureLod(texpaint_mask_view, texCoord, 0.0).r;');
				}
				frag.write('fragColor[1] = vec4(sample_mask, sample_mask, sample_mask, 1.0);');
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
		#if (kha_direct3d11 || kha_direct3d12)
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
