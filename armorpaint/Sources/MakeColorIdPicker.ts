
class MakeColorIdPicker {

	static run = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		// Mangle vertices to form full screen triangle
		NodeShader.write(vert, 'gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

		NodeShader.add_uniform(frag, 'sampler2D gbuffer2');
		NodeShader.add_uniform(frag, 'vec2 gbufferSize', '_gbufferSize');
		NodeShader.add_uniform(frag, 'vec4 inp', '_inputBrush');

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
		///else
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
		///end

		if (Context.raw.tool == WorkspaceTool.ToolColorId) {
			NodeShader.add_out(frag, 'vec4 fragColor');
			NodeShader.add_uniform(frag, 'sampler2D texcolorid', '_texcolorid');
			NodeShader.write(frag, 'vec3 idcol = textureLod(texcolorid, texCoordInp, 0.0).rgb;');
			NodeShader.write(frag, 'fragColor = vec4(idcol, 1.0);');
		}
		else if (Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial) {
			if (Context.raw.pickPosNorTex) {
				NodeShader.add_out(frag, 'vec4 fragColor[2]');
				NodeShader.add_uniform(frag, 'sampler2D gbufferD');
				NodeShader.add_uniform(frag, 'mat4 invVP', '_inv_view_proj_matrix');
				NodeShader.add_function(frag, ShaderFunctions.str_get_pos_from_depth);
				NodeShader.add_function(frag, ShaderFunctions.str_get_nor_from_depth);
				NodeShader.write(frag, 'fragColor[0] = vec4(get_pos_from_depth(vec2(inp.x, 1.0 - inp.y), invVP, texturePass(gbufferD)), texCoordInp.x);');
				NodeShader.write(frag, 'fragColor[1] = vec4(get_nor_from_depth(fragColor[0].rgb, vec2(inp.x, 1.0 - inp.y), invVP, vec2(1.0, 1.0) / gbufferSize, texturePass(gbufferD)), texCoordInp.y);');
			}
			else {
				NodeShader.add_out(frag, 'vec4 fragColor[4]');
				NodeShader.add_uniform(frag, 'sampler2D texpaint');
				NodeShader.add_uniform(frag, 'sampler2D texpaint_nor');
				NodeShader.add_uniform(frag, 'sampler2D texpaint_pack');
				NodeShader.write(frag, 'fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
				NodeShader.write(frag, 'fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
				NodeShader.write(frag, 'fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
				NodeShader.write(frag, 'fragColor[3].rg = texCoordInp.xy;');
			}
		}
	}
}
