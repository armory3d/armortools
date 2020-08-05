package arm.node;

import arm.ui.UISidebar;
import arm.node.MaterialShader;
import arm.Enums;

class MakeBrush {

	public static function run(vert: MaterialShader, frag: MaterialShader) {

		if (Context.tool == ToolDecal || Context.tool == ToolText || Context.tool == ToolParticle) {
			frag.write('float dist = 0.0;');
		}
		else if (Config.raw.brush_3d) {
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

			if (Context.brushAngleReject || Context.xray) {
				frag.add_function(MaterialFunctions.str_octahedronWrap);
				frag.add_uniform('sampler2D gbuffer0');
				#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				frag.write('vec2 g0 = textureLod(gbuffer0, inp.xy, 0.0).rg;');
				#else
				frag.write('vec2 g0 = textureLod(gbuffer0, vec2(inp.x, 1.0 - inp.y), 0.0).rg;');
				#end
				frag.write('vec3 wn;');
				frag.write('wn.z = 1.0 - abs(g0.x) - abs(g0.y);');
				frag.write('wn.xy = wn.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);');
				frag.write('wn = normalize(wn);');
				frag.write('float planeDist = dot(wn, winp.xyz - wposition);');

				if (Context.brushAngleReject && !Context.xray) {
					frag.write('if (planeDist < -0.01) discard;');
					frag.n = true;
					var angle = Context.brushAngleRejectDot;
					frag.write('if (dot(wn, n) < $angle) discard;');
				}
			}

			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;');
			#else
			frag.write('float depthlast = textureLod(gbufferD, vec2(inplast.x, 1.0 - inplast.y), 0.0).r;');
			#end

			frag.write('vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
			frag.write('winplast = mul(winplast, invVP);');
			frag.write('winplast.xyz /= winplast.w;');

			frag.write('vec3 pa = wposition - winp.xyz;');
			if (Context.xray) {
				frag.write('pa += wn * vec3(planeDist, planeDist, planeDist);');
			}
			frag.write('vec3 ba = winplast.xyz - winp.xyz;');

			if (Context.brushLazyRadius > 0 && Context.brushLazyStep > 0) {
				// Sphere
				frag.write('float dist = distance(wposition, winp.xyz);');
			}
			else {
				// Capsule
				frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
				frag.write('float dist = length(pa - ba * h);');
			}

			frag.write('if (dist > brushRadius) discard;');
		}
		else { // !brush3d
			frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
			frag.write('binp.x *= aspectRatio;');
			frag.write('binp = binp * 0.5 + 0.5;');

			frag.write('vec2 binplast = inplast.xy * 2.0 - 1.0;');
			frag.write('binplast.x *= aspectRatio;');
			frag.write('binplast = binplast * 0.5 + 0.5;');

			frag.write('vec2 pa = bsp.xy - binp.xy;');
			frag.write('vec2 ba = binplast.xy - binp.xy;');
			frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
			frag.write('float dist = length(pa - ba * h);');

			frag.write('if (dist > brushRadius) discard;');
		}
	}
}
