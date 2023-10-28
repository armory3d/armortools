package arm.shader;

import iron.data.SceneFormat;
import zui.Nodes;
import arm.shader.MaterialParser;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;

class MakePaint {

	public static function run(data: NodeShaderData, matcon: TMaterialContext): NodeShaderContext {
		var con_paint:NodeShaderContext = data.add_context({
			name: "paint",
			depth_write: false,
			compare_mode: "always", // TODO: align texcoords winding order
			// cull_mode: "counter_clockwise",
			cull_mode: "none",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
			color_attachments:
				Context.raw.tool == ToolPicker ? ["RGBA32", "RGBA32", "RGBA32", "RGBA32"] :
					["RGBA32", "RGBA32", "RGBA32", "R8"]
		});

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];
		con_paint.allow_vcols = Context.raw.paintObject.data.geom.cols != null;

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();
		frag.ins = vert.outs;

		if (Context.raw.tool == ToolPicker) {
			// Mangle vertices to form full screen triangle
			vert.write('gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('vec4 inp', '_inputBrush');

			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
			#else
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
			#end

			frag.add_out('vec4 fragColor[4]');
			frag.add_uniform('sampler2D texpaint');
			frag.add_uniform('sampler2D texpaint_nor');
			frag.add_uniform('sampler2D texpaint_pack');
			frag.write('fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
			frag.write('fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
			frag.write('fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
			frag.write('fragColor[3].rg = texCoordInp.xy;');
			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);');
		// vert.write('vec2 tpos = vec2(frac(tex.x * texScale) * 2.0 - 1.0, (1.0 - frac(tex.y * texScale)) * 2.0 - 1.0);'); // 3D View
		#else
		vert.write('vec2 tpos = vec2(tex.xy * 2.0 - 1.0);');
		#end

		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');

		vert.add_out('vec4 ndc');
		vert.write_attrib('ndc = mul(vec4(pos.xyz, 1.0), WVP);');

		frag.write_attrib('vec3 sp = vec3((ndc.xyz / ndc.w) * 0.5 + 0.5);');
		frag.write_attrib('sp.y = 1.0 - sp.y;');
		frag.write_attrib('sp.z -= 0.0001;'); // small bias

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');
		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D gbufferD');

		frag.add_out('vec4 fragColor[4]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushHardness', '_brushHardness');

		if (Context.raw.tool == ToolEraser ||
			Context.raw.tool == ToolClone  ||
			Context.raw.tool == ToolBlur   ||
			Context.raw.tool == ToolSmudge) {

			frag.write('float dist = 0.0;');

			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('float depth = textureLod(gbufferD, inp.xy, 0.0).r;');
			#else
			frag.write('float depth = textureLod(gbufferD, vec2(inp.x, 1.0 - inp.y), 0.0).r;');
			#end

			frag.add_uniform('mat4 invVP', '_inverseViewProjectionMatrix');
			frag.write('vec4 winp = vec4(vec2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);');
			frag.write('winp = mul(winp, invVP);');
			frag.write('winp.xyz /= winp.w;');
			frag.wposition = true;

			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;');
			#else
			frag.write('float depthlast = textureLod(gbufferD, vec2(inplast.x, 1.0 - inplast.y), 0.0).r;');
			#end

			frag.write('vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
			frag.write('winplast = mul(winplast, invVP);');
			frag.write('winplast.xyz /= winplast.w;');

			frag.write('vec3 pa = wposition - winp.xyz;');
			frag.write('vec3 ba = winplast.xyz - winp.xyz;');

			// Capsule
			frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
			frag.write('dist = length(pa - ba * h);');

			frag.write('if (dist > brushRadius) discard;');
		}

		// vert.add_uniform('float brushScale', '_brushScale');
		// vert.add_uniform('float texScale', '_texUnpack');
		// vert.add_out('vec2 texCoord');
		// vert.write('texCoord = tex * brushScale * texScale;');

		if (Context.raw.tool == ToolClone || Context.raw.tool == ToolBlur || Context.raw.tool == ToolSmudge) {
			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');

			if (Context.raw.tool == ToolClone) {
				// frag.add_uniform('vec2 cloneDelta', '_cloneDelta');
				// #if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				// frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.xy + cloneDelta) * gbufferSize), 0).ba;');
				// #else
				// frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.x + cloneDelta.x) * gbufferSize.x, (1.0 - (sp.y + cloneDelta.y)) * gbufferSize.y), 0).ba;');
				// #end

				// frag.write('vec3 texpaint_pack_sample = textureLod(texpaint_pack_undo, texCoordInp, 0.0).rgb;');
				// var base = 'textureLod(texpaint_undo, texCoordInp, 0.0).rgb';
				// var rough = 'texpaint_pack_sample.g';
				// var met = 'texpaint_pack_sample.b';
				// var occ = 'texpaint_pack_sample.r';
				// var nortan = 'textureLod(texpaint_nor_undo, texCoordInp, 0.0).rgb';
				// var height = '0.0';
				// var opac = '1.0';
				// frag.write('vec3 basecol = $base;');
				// frag.write('float roughness = $rough;');
				// frag.write('float metallic = $met;');
				// frag.write('float occlusion = $occ;');
				// frag.write('vec3 nortan = $nortan;');
				// frag.write('float height = $height;');
				// frag.write('float mat_opacity = $opac;');
				// frag.write('float opacity = mat_opacity * brushOpacity;');
			}
			else { // Blur
				// #if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				// frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, sp.y * gbufferSize.y), 0).ba;');
				// #else
				// frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, (1.0 - sp.y) * gbufferSize.y), 0).ba;');
				// #end

				// frag.write('vec3 basecol = vec3(0.0, 0.0, 0.0);');
				// frag.write('float roughness = 0.0;');
				// frag.write('float metallic = 0.0;');
				// frag.write('float occlusion = 0.0;');
				// frag.write('vec3 nortan = vec3(0.0, 0.0, 0.0);');
				// frag.write('float height = 0.0;');
				// frag.write('float mat_opacity = 1.0;');
				// frag.write('float opacity = 0.0;');

				// frag.add_uniform('vec2 texpaintSize', '_texpaintSize');
				// frag.write('float blur_step = 1.0 / texpaintSize.x;');
				// if (Context.raw.blurDirectional) {
				// 	#if (kha_direct3d11 || kha_direct3d12 || kha_metal)
				// 	frag.write('const float blur_weight[7] = {1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0};');
				// 	#else
				// 	frag.write('const float blur_weight[7] = float[](1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0);');
				// 	#end
				// 	frag.add_uniform('vec3 brushDirection', '_brushDirection');
				// 	frag.write('vec2 blur_direction = brushDirection.yx;');
				// 	frag.write('for (int i = 0; i < 7; ++i) {');
				// 	#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				// 	frag.write('vec2 texCoordInp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbufferSize.x, (sp.y + blur_direction.y * blur_step * float(i)) * gbufferSize.y), 0).ba;');
				// 	#else
				// 	frag.write('vec2 texCoordInp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbufferSize.x, (1.0 - (sp.y + blur_direction.y * blur_step * float(i))) * gbufferSize.y), 0).ba;');
				// 	#end
				// 	frag.write('vec4 texpaint_sample = texture(texpaint_undo, texCoordInp2);');
				// 	frag.write('opacity += texpaint_sample.a * blur_weight[i];');
				// 	frag.write('basecol += texpaint_sample.rgb * blur_weight[i];');
				// 	frag.write('vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp2) * blur_weight[i];');
				// 	frag.write('roughness += texpaint_pack_sample.g;');
				// 	frag.write('metallic += texpaint_pack_sample.b;');
				// 	frag.write('occlusion += texpaint_pack_sample.r;');
				// 	frag.write('height += texpaint_pack_sample.a;');
				// 	frag.write('nortan += texture(texpaint_nor_undo, texCoordInp2).rgb * blur_weight[i];');
				// 	frag.write('}');
				// }
				// else {
				// 	#if (kha_direct3d11 || kha_direct3d12 || kha_metal)
				// 	frag.write('const float blur_weight[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};');
				// 	#else
				// 	frag.write('const float blur_weight[15] = float[](0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0);');
				// 	#end
				// 	// X
				// 	frag.write('for (int i = -7; i <= 7; ++i) {');
				// 	frag.write('vec4 texpaint_sample = texture(texpaint_undo, texCoordInp + vec2(blur_step * float(i), 0.0));');
				// 	frag.write('opacity += texpaint_sample.a * blur_weight[i + 7];');
				// 	frag.write('basecol += texpaint_sample.rgb * blur_weight[i + 7];');
				// 	frag.write('vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(blur_step * float(i), 0.0)) * blur_weight[i + 7];');
				// 	frag.write('roughness += texpaint_pack_sample.g;');
				// 	frag.write('metallic += texpaint_pack_sample.b;');
				// 	frag.write('occlusion += texpaint_pack_sample.r;');
				// 	frag.write('height += texpaint_pack_sample.a;');
				// 	frag.write('nortan += texture(texpaint_nor_undo, texCoordInp + vec2(blur_step * float(i), 0.0)).rgb * blur_weight[i + 7];');
				// 	frag.write('}');
				// 	// Y
				// 	frag.write('for (int j = -7; j <= 7; ++j) {');
				// 	frag.write('vec4 texpaint_sample = texture(texpaint_undo, texCoordInp + vec2(0.0, blur_step * float(j)));');
				// 	frag.write('opacity += texpaint_sample.a * blur_weight[j + 7];');
				// 	frag.write('basecol += texpaint_sample.rgb * blur_weight[j + 7];');
				// 	frag.write('vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(0.0, blur_step * float(j))) * blur_weight[j + 7];');
				// 	frag.write('roughness += texpaint_pack_sample.g;');
				// 	frag.write('metallic += texpaint_pack_sample.b;');
				// 	frag.write('occlusion += texpaint_pack_sample.r;');
				// 	frag.write('height += texpaint_pack_sample.a;');
				// 	frag.write('nortan += texture(texpaint_nor_undo, texCoordInp + vec2(0.0, blur_step * float(j))).rgb * blur_weight[j + 7];');
				// 	frag.write('}');
				// }
				// frag.write('opacity *= brushOpacity;');
			}
		}

		frag.write('float opacity = 1.0;');
		frag.write('if (opacity == 0.0) discard;');

		frag.write('float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');

		// Manual blending to preserve memory
		frag.wvpposition = true;
		frag.write('vec2 sample_tc = vec2(wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		frag.write('sample_tc.y = 1.0 - sample_tc.y;');
		#end
		frag.add_uniform('sampler2D paintmask');
		frag.write('float sample_mask = textureLod(paintmask, sample_tc, 0.0).r;');
		frag.write('str = max(str, sample_mask);');

		frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
		frag.write('vec4 sample_undo = textureLod(texpaint_undo, sample_tc, 0.0);');

		if (Context.raw.tool == ToolEraser) {
			// frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, vec3(0.0, 0.0, 0.0), str), sample_undo.a - str);');
			frag.write('fragColor[0] = vec4(0.0, 0.0, 0.0, 0.0);');
			frag.write('fragColor[1] = vec4(0.5, 0.5, 1.0, 0.0);');
			frag.write('fragColor[2] = vec4(1.0, 0.0, 0.0, 0.0);');
		}

		frag.write('fragColor[3] = vec4(str, 0.0, 0.0, 1.0);');

		MaterialParser.finalize(con_paint);
		MaterialParser.sample_keep_aspect = false;
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}
}
