
class MakeBrush {

	static run = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {

		NodeShader.write(frag, 'float dist = 0.0;');

		if (Context.raw.tool == WorkspaceTool.ToolParticle) return;

		let fillLayer = Context.raw.layer.fill_layer != null;
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		if (decal && !fillLayer) NodeShader.write(frag, 'if (decalMask.z > 0.0) {');

		if (Config.raw.brush_3d) {
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'float depth = textureLod(gbufferD, inp.xy, 0.0).r;');
			///else
			NodeShader.write(frag, 'float depth = textureLod(gbufferD, vec2(inp.x, 1.0 - inp.y), 0.0).r;');
			///end

			NodeShader.add_uniform(frag, 'mat4 invVP', '_inv_view_proj_matrix');
			NodeShader.write(frag, 'vec4 winp = vec4(vec2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);');
			NodeShader.write(frag, 'winp = mul(winp, invVP);');
			NodeShader.write(frag, 'winp.xyz /= winp.w;');
			frag.wposition = true;

			if (Config.raw.brush_angle_reject || Context.raw.xray) {
				NodeShader.add_function(frag, ShaderFunctions.str_octahedronWrap);
				NodeShader.add_uniform(frag, 'sampler2D gbuffer0');
				///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				NodeShader.write(frag, 'vec2 g0 = textureLod(gbuffer0, inp.xy, 0.0).rg;');
				///else
				NodeShader.write(frag, 'vec2 g0 = textureLod(gbuffer0, vec2(inp.x, 1.0 - inp.y), 0.0).rg;');
				///end
				NodeShader.write(frag, 'vec3 wn;');
				NodeShader.write(frag, 'wn.z = 1.0 - abs(g0.x) - abs(g0.y);');
				NodeShader.write(frag, 'wn.xy = wn.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);');
				NodeShader.write(frag, 'wn = normalize(wn);');
				NodeShader.write(frag, 'float planeDist = dot(wn, winp.xyz - wposition);');

				if (Config.raw.brush_angle_reject && !Context.raw.xray) {
					NodeShader.write(frag, 'if (planeDist < -0.01) discard;');
					frag.n = true;
					let angle = Context.raw.brushAngleRejectDot;
					NodeShader.write(frag, `if (dot(wn, n) < ${angle}) discard;`);
				}
			}

			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;');
			///else
			NodeShader.write(frag, 'float depthlast = textureLod(gbufferD, vec2(inplast.x, 1.0 - inplast.y), 0.0).r;');
			///end

			NodeShader.write(frag, 'vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
			NodeShader.write(frag, 'winplast = mul(winplast, invVP);');
			NodeShader.write(frag, 'winplast.xyz /= winplast.w;');

			NodeShader.write(frag, 'vec3 pa = wposition - winp.xyz;');
			if (Context.raw.xray) {
				NodeShader.write(frag, 'pa += wn * vec3(planeDist, planeDist, planeDist);');
			}
			NodeShader.write(frag, 'vec3 ba = winplast.xyz - winp.xyz;');

			if (Context.raw.brushLazyRadius > 0 && Context.raw.brushLazyStep > 0) {
				// Sphere
				NodeShader.write(frag, 'dist = distance(wposition, winp.xyz);');
			}
			else {
				// Capsule
				NodeShader.write(frag, 'float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
				NodeShader.write(frag, 'dist = length(pa - ba * h);');
			}
		}
		else { // !brush3d
			NodeShader.write(frag, 'vec2 binp = inp.xy * 2.0 - 1.0;');
			NodeShader.write(frag, 'binp.x *= aspectRatio;');
			NodeShader.write(frag, 'binp = binp * 0.5 + 0.5;');

			NodeShader.write(frag, 'vec2 binplast = inplast.xy * 2.0 - 1.0;');
			NodeShader.write(frag, 'binplast.x *= aspectRatio;');
			NodeShader.write(frag, 'binplast = binplast * 0.5 + 0.5;');

			NodeShader.write(frag, 'vec2 pa = bsp.xy - binp.xy;');
			NodeShader.write(frag, 'vec2 ba = binplast.xy - binp.xy;');
			NodeShader.write(frag, 'float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
			NodeShader.write(frag, 'dist = length(pa - ba * h);');
		}

		NodeShader.write(frag, 'if (dist > brushRadius) discard;');

		if (decal && !fillLayer) NodeShader.write(frag, '}');
	}
}
