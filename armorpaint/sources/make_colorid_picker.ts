
function make_colorid_picker_run(vert: node_shader_t, frag: node_shader_t) {
	// Mangle vertices to form full screen triangle
	node_shader_write(vert, "gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);");

	node_shader_add_uniform(frag, "sampler2D gbuffer2");
	node_shader_add_uniform(frag, "vec2 gbuffer_size", "_gbuffer_size");
	node_shader_add_uniform(frag, "vec4 inp", "_input_brush");

	node_shader_write(frag, "vec2 tex_coord_inp = texelFetch(gbuffer2, ivec2(inp.x * gbuffer_size.x, inp.y * gbuffer_size.y), 0).ba;");

	if (context_raw.tool == workspace_tool_t.COLORID) {
		node_shader_add_out(frag, "vec4 frag_color");
		node_shader_add_uniform(frag, "sampler2D texcolorid", "_texcolorid");
		node_shader_write(frag, "vec3 idcol = textureLod(texcolorid, tex_coord_inp, 0.0).rgb;");
		node_shader_write(frag, "frag_color = vec4(idcol, 1.0);");
	}
	else if (context_raw.tool == workspace_tool_t.PICKER || context_raw.tool == workspace_tool_t.MATERIAL) {
		if (context_raw.pick_pos_nor_tex) {
			node_shader_add_out(frag, "vec4 frag_color[2]");
			node_shader_add_uniform(frag, "sampler2D gbufferD");
			node_shader_add_uniform(frag, "mat4 invVP", "_inv_view_proj_matrix");
			node_shader_add_function(frag, str_get_pos_nor_from_depth);
			node_shader_write(frag, "frag_color[0] = vec4(get_pos_from_depth(vec2(inp.x, 1.0 - inp.y), invVP, texturePass(gbufferD)), tex_coord_inp.x);");
			node_shader_write(frag, "frag_color[1] = vec4(get_nor_from_depth(frag_color[0].rgb, vec2(inp.x, 1.0 - inp.y), invVP, vec2(1.0, 1.0) / gbuffer_size, texturePass(gbufferD)), tex_coord_inp.y);");
		}
		else {
			node_shader_add_out(frag, "vec4 frag_color[4]");
			node_shader_add_uniform(frag, "sampler2D texpaint");
			node_shader_add_uniform(frag, "sampler2D texpaint_nor");
			node_shader_add_uniform(frag, "sampler2D texpaint_pack");
			node_shader_write(frag, "frag_color[0] = textureLod(texpaint, tex_coord_inp, 0.0);");
			node_shader_write(frag, "frag_color[1] = textureLod(texpaint_nor, tex_coord_inp, 0.0);");
			node_shader_write(frag, "frag_color[2] = textureLod(texpaint_pack, tex_coord_inp, 0.0);");
			node_shader_write(frag, "frag_color[3].rg = tex_coord_inp.xy;");
		}
	}
}
