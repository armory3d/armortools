package arm.node;

import arm.ui.UISidebar;
import arm.shader.MaterialParser;
import arm.shader.NodeShader;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;
import arm.shader.ShaderFunctions;
import arm.Enums;

class MakeMesh {

	public static function run(data: NodeShaderData, layerPass = 0): NodeShaderContext {
		var con_mesh: NodeShaderContext = data.add_context({
			name: "mesh",
			depth_write: layerPass == 0 ? true : false,
			compare_mode: layerPass == 0 ? "less" : "equal",
			cull_mode: (Context.cullBackfaces || layerPass > 0) ? "clockwise" : "none",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
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
		var displaceStrength = MakeMaterial.getDisplaceStrength();
		if (MakeMaterial.heightUsed && displaceStrength > 0.0) {
			vert.n = true;
			vert.write('float height = 0.0;');
			var numLayers = 1;
			vert.write('wposition += wnormal * vec3(height, height, height) * vec3($displaceStrength, $displaceStrength, $displaceStrength);');
		}

		vert.write('gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		var brushScale = Context.brushScale;
		vert.write('texCoord = tex * $brushScale;');
		if (MakeMaterial.heightUsed && displaceStrength > 0) {
			vert.add_uniform('mat4 invW', '_inverseWorldMatrix');
			vert.write('prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			vert.write('prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;
		frag.add_function(ShaderFunctions.str_packFloatInt16);
		frag.add_function(ShaderFunctions.str_octahedronWrap);
		frag.add_function(ShaderFunctions.str_cotangentFrame);

		frag.write('vec3 basecol = vec3(0.0, 0.0, 0.0);');
		frag.write('float roughness = 0.0;');
		frag.write('float metallic = 0.0;');
		frag.write('float occlusion = 1.0;');
		frag.write('float opacity = 1.0;');
		frag.write('float matid = 0.0;');
		frag.write('vec3 ntex = vec3(0.5, 0.5, 1.0);');
		frag.write('float height = 0.0;');

		frag.write('vec4 texpaint_sample = vec4(0.0, 0.0, 0.0, 1.0);');
		frag.write('vec4 texpaint_nor_sample;');
		frag.write('vec4 texpaint_pack_sample;');
		frag.write('float texpaint_opac;');

		if (MakeMaterial.heightUsed) {
			frag.write('float height0 = 0.0;');
			frag.write('float height1 = 0.0;');
			frag.write('float height2 = 0.0;');
			frag.write('float height3 = 0.0;');
		}

		if (Context.viewportMode == ViewLit && Context.renderMode == RenderForward) {
			frag.add_uniform('sampler2D senvmapBrdf', "$brdf.k");
			frag.add_uniform('sampler2D senvmapRadiance', '_envmapRadiance');
			frag.add_uniform('sampler2D sltcMat', '_ltcMat');
			frag.add_uniform('sampler2D sltcMag', '_ltcMag');
		}

		frag.add_shared_sampler('sampler2D texpaint');
		frag.write('texpaint_sample = textureLodShared(texpaint' + ', texCoord, 0.0);');
		frag.write('texpaint_opac = texpaint_sample.a;');

		frag.write('basecol = texpaint_sample.rgb * texpaint_opac;');

		frag.add_shared_sampler('sampler2D texpaint_nor');
		frag.write('texpaint_nor_sample = textureLodShared(texpaint_nor' + ', texCoord, 0.0);');

		frag.write('ntex = mix(ntex, texpaint_nor_sample.rgb, texpaint_opac);');

		frag.add_shared_sampler('sampler2D texpaint_pack');
		frag.write('texpaint_pack_sample = textureLodShared(texpaint_pack' + ', texCoord, 0.0);');

		frag.write('occlusion = mix(occlusion, texpaint_pack_sample.r, texpaint_opac);');

		frag.write('roughness = mix(roughness, texpaint_pack_sample.g, texpaint_opac);');

		frag.write('metallic = mix(metallic, texpaint_pack_sample.b, texpaint_opac);');

		frag.write('height = texpaint_pack_sample.a * texpaint_opac;');

		// if (l.paintHeight && MakeMaterial.heightUsed) {
		// 	var assign = l.paintHeightBlend ? "+=" : "=";
		// 	frag.write('height $assign texpaint_pack_sample.a * texpaint_opac;');
		// 	frag.write('{');
		// 	frag.add_uniform('vec2 texpaintSize', '_texpaintSize');
		// 	frag.write('float tex_step = 1.0 / texpaintSize.x;');
		// 	frag.write('height0 $assign textureLodShared(texpaint_pack' + ', vec2(texCoord.x - tex_step, texCoord.y), 0.0).a * texpaint_opac;');
		// 	frag.write('height1 $assign textureLodShared(texpaint_pack' + ', vec2(texCoord.x + tex_step, texCoord.y), 0.0).a * texpaint_opac;');
		// 	frag.write('height2 $assign textureLodShared(texpaint_pack' + ', vec2(texCoord.x, texCoord.y - tex_step), 0.0).a * texpaint_opac;');
		// 	frag.write('height3 $assign textureLodShared(texpaint_pack' + ', vec2(texCoord.x, texCoord.y + tex_step), 0.0).a * texpaint_opac;');
		// 	frag.write('}');
		// }

		if (MakeMaterial.heightUsed) {
			frag.write('if (height > 0.0) {');
			frag.write('float height_dx = height0 - height1;');
			frag.write('float height_dy = height2 - height3;');
			// Whiteout blend
			frag.write('vec3 n1 = ntex * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.write('vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));');
			frag.write('ntex = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			frag.write('}');
		}

		frag.vVec = true;
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
		#else
		frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
		#end
		frag.write('n = ntex * 2.0 - 1.0;');
		frag.write('n.y = -n.y;');
		frag.write('n = normalize(mul(n, TBN));');

		if (Context.viewportMode == ViewLit || Context.viewportMode == ViewPathTrace) {

			frag.write('basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');

			if (Context.viewportShader != null) {
				var color = Context.viewportShader(frag);
				frag.write('fragColor[1] = vec4($color, 1.0);');
			}
			else if (Context.renderMode == RenderForward) {
				frag.wposition = true;
				frag.write('vec3 albedo = mix(basecol, vec3(0.0, 0.0, 0.0), metallic);');
				frag.write('vec3 f0 = mix(vec3(0.04, 0.04, 0.04), basecol, metallic);');
				frag.vVec = true;
				frag.write('float dotNV = max(dot(n, vVec), 0.0);');
				frag.write('vec2 envBRDF = texelFetch(senvmapBrdf, ivec2(vec2(roughness, 1.0 - dotNV) * 256.0), 0).xy;');
				frag.add_uniform('int envmapNumMipmaps', '_envmapNumMipmaps');
				frag.add_uniform('vec4 envmapData', '_envmapData'); // angle, sin(angle), cos(angle), strength
				frag.write('vec3 wreflect = reflect(-vVec, n);');
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
				frag.write('float ldist = distance(wposition, lightPos);');
				frag.write('const float LUT_SIZE = 64.0;');
				frag.write('const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;');
				frag.write('const float LUT_BIAS = 0.5 / LUT_SIZE;');
				frag.write('float theta = acos(dotNV);');
				frag.write('vec2 tuv = vec2(roughness, theta / (0.5 * 3.14159265));');
				frag.write('tuv = tuv * LUT_SCALE + LUT_BIAS;');
				frag.write('vec4 t = textureLod(sltcMat, tuv, 0.0);');
				frag.write('mat3 minv = mat3(vec3(1.0, 0.0, t.y), vec3(0.0, t.z, 0.0), vec3(t.w, 0.0, t.x));');
				frag.write('float ltcspec = ltcEvaluate(n, vVec, dotNV, wposition, minv, lightArea0, lightArea1, lightArea2, lightArea3);');
				frag.write('ltcspec *= textureLod(sltcMag, tuv, 0.0).a;');
				frag.write('mat3 mident = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);');
				frag.write('float ltcdiff = ltcEvaluate(n, vVec, dotNV, wposition, mident, lightArea0, lightArea1, lightArea2, lightArea3);');
				frag.write('vec3 direct = albedo * ltcdiff + ltcspec * 0.05;');
				frag.write('direct *= lightColor * (1.0 / (ldist * ldist));');

				frag.add_uniform('vec4 shirr[7]', '_envmapIrradiance');
				frag.add_function(ShaderFunctions.str_shIrradiance);
				frag.write('vec3 indirect = albedo * (shIrradiance(vec3(n.x * envmapDataLocal.z - n.y * envmapDataLocal.y, n.x * envmapDataLocal.y + n.y * envmapDataLocal.z, n.z), shirr) / 3.14159265);');
				frag.write('indirect += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;');
				frag.write('indirect *= envmapDataLocal.w * occlusion;');
				frag.write('fragColor[1] = vec4(direct + indirect, 1.0);');
			}
			else { // Deferred, Pathtraced
				frag.write('fragColor[1] = vec4(basecol, occlusion);');
			}
		}
		else if (Context.viewportMode == ViewBaseColor) {
			frag.write('fragColor[1] = vec4(basecol, 1.0);');
		}
		else if (Context.viewportMode == ViewNormalMap) {
			frag.write('fragColor[1] = vec4(ntex.rgb, 1.0);');
		}
		else if (Context.viewportMode == ViewOcclusion) {
			frag.write('fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), 1.0);');
		}
		else if (Context.viewportMode == ViewRoughness) {
			frag.write('fragColor[1] = vec4(vec3(roughness, roughness, roughness), 1.0);');
		}
		else if (Context.viewportMode == ViewMetallic) {
			frag.write('fragColor[1] = vec4(vec3(metallic, metallic, metallic), 1.0);');
		}
		else if (Context.viewportMode == ViewOpacity) {
			frag.write('fragColor[1] = vec4(vec3(texpaint_sample.a, texpaint_sample.a, texpaint_sample.a), 1.0);');
		}
		else if (Context.viewportMode == ViewHeight) {
			frag.write('fragColor[1] = vec4(vec3(height, height, height), 1.0);');
		}
		else {
			frag.write('fragColor[1] = vec4(1.0, 0.0, 1.0, 1.0);'); // Pink
		}

		frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		frag.write('fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(int(matid * 255.0) % 3)));');

		frag.write('vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		frag.write('vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		frag.write('fragColor[2] = vec4(posa - posb, texCoord.xy);');

		MaterialParser.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();
		return con_mesh;
	}
}
