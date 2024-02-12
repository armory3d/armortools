
class MakeBrush {

	static run = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {

		NodeShader.write(frag, 'float dist = 0.0;');

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

			NodeShader.add_uniform(frag, 'mat4 W', '_world_matrix');

			NodeShader.write_attrib(frag, 'vec3 wposition = mul(texelFetch(texpaint_undo, ivec2(texCoord.x * textureSize(texpaint_undo, 0).x, texCoord.y * textureSize(texpaint_undo, 0).y), 0), W).xyz;');

			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;');
			///else
			NodeShader.write(frag, 'float depthlast = textureLod(gbufferD, vec2(inplast.x, 1.0 - inplast.y), 0.0).r;');
			///end

			NodeShader.write(frag, 'vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
			NodeShader.write(frag, 'winplast = mul(winplast, invVP);');
			NodeShader.write(frag, 'winplast.xyz /= winplast.w;');

			NodeShader.write(frag, 'dist = distance(wposition, winp.xyz);');
		}

		NodeShader.write(frag, 'if (dist > brushRadius) discard;');
	}
}
