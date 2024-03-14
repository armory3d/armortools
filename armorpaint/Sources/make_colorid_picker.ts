
function make_colorid_picker_run(vert: NodeShaderRaw, frag: NodeShaderRaw) {
	// Mangle vertices to form full screen triangle
	node_shader_write(vert, 'gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

	node_shader_add_uniform(frag, 'sampler2D gbuffer2');
	node_shader_add_uniform(frag, 'vec2 gbufferSize', '_gbufferSize');
	node_shader_add_uniform(frag, 'vec4 inp', '_inputBrush');

	///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
	node_shader_write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
	///else
	node_shader_write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
	///end

	if (context_raw.tool == workspace_tool_t.COLORID) {
		node_shader_add_out(frag, 'vec4 fragColor');
		node_shader_add_uniform(frag, 'sampler2D texcolorid', '_texcolorid');
		node_shader_write(frag, 'vec3 idcol = textureLod(texcolorid, texCoordInp, 0.0).rgb;');
		node_shader_write(frag, 'fragColor = vec4(idcol, 1.0);');
	}
	else if (context_raw.tool == workspace_tool_t.PICKER || context_raw.tool == workspace_tool_t.MATERIAL) {
		if (context_raw.pick_pos_nor_tex) {
			node_shader_add_out(frag, 'vec4 fragColor[2]');
			node_shader_add_uniform(frag, 'sampler2D gbufferD');
			node_shader_add_uniform(frag, 'mat4 invVP', '_inv_view_proj_matrix');
			node_shader_add_function(frag, str_get_pos_from_depth);
			node_shader_add_function(frag, str_get_nor_from_depth);
			node_shader_write(frag, 'fragColor[0] = vec4(get_pos_from_depth(vec2(inp.x, 1.0 - inp.y), invVP, texturePass(gbufferD)), texCoordInp.x);');
			node_shader_write(frag, 'fragColor[1] = vec4(get_nor_from_depth(fragColor[0].rgb, vec2(inp.x, 1.0 - inp.y), invVP, vec2(1.0, 1.0) / gbufferSize, texturePass(gbufferD)), texCoordInp.y);');
		}
		else {
			node_shader_add_out(frag, 'vec4 fragColor[4]');
			node_shader_add_uniform(frag, 'sampler2D texpaint');
			node_shader_add_uniform(frag, 'sampler2D texpaint_nor');
			node_shader_add_uniform(frag, 'sampler2D texpaint_pack');
			node_shader_write(frag, 'fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
			node_shader_write(frag, 'fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
			node_shader_write(frag, 'fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
			node_shader_write(frag, 'fragColor[3].rg = texCoordInp.xy;');
		}
	}
}
