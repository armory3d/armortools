
function make_colorid_picker_run(kong: node_shader_t) {
	// Mangle vertices to form full screen triangle
	node_shader_write_vert(kong, "output.pos = float4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);");

	node_shader_add_uniform(kong, "gbuffer2: tex2d");
	node_shader_add_uniform(kong, "gbuffer_size: float2", "_gbuffer_size");
	node_shader_add_uniform(kong, "inp: float4", "_input_brush");

	node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[int2(inp.x * gbuffer_size.x, inp.y * gbuffer_size.y)].ba;");

	if (context_raw.tool == workspace_tool_t.COLORID) {
		kong.frag_out = "float4";
		node_shader_add_uniform(kong, "texcolorid: tex2d", "_texcolorid");
		node_shader_write_frag(kong, "var idcol: float3 = sample_lod(texcolorid, tex_coord_inp, 0.0).rgb;");
		node_shader_write_frag(kong, "output = float4(idcol, 1.0);");
	}
	else if (context_raw.tool == workspace_tool_t.PICKER || context_raw.tool == workspace_tool_t.MATERIAL) {
		if (context_raw.pick_pos_nor_tex) {
			kong.frag_out = "float4[2]";
			node_shader_add_uniform(kong, "gbufferD: tex2d");
			node_shader_add_uniform(kong, "invVP: float4x4", "_inv_view_proj_matrix");
			node_shader_add_function(kong, str_get_pos_nor_from_depth);
			node_shader_write_frag(kong, "output[0].rgba = float4(get_pos_from_depth(float2(inp.x, 1.0 - inp.y), invVP, texturePass(gbufferD)), tex_coord_inp.x);");
			node_shader_write_frag(kong, "output[1].rgba = float4(get_nor_from_depth(output[0].rgb, float2(inp.x, 1.0 - inp.y), invVP, float2(1.0, 1.0) / gbuffer_size, texturePass(gbufferD)), tex_coord_inp.y);");
		}
		else {
			kong.frag_out = "float4[4]";
			node_shader_add_uniform(kong, "texpaint: tex2d");
			node_shader_add_uniform(kong, "texpaint_nor: tex2d");
			node_shader_add_uniform(kong, "texpaint_pack: tex2d");
			node_shader_write_frag(kong, "output[0].rgba = sample_lod(texpaint, tex_coord_inp, 0.0);");
			node_shader_write_frag(kong, "output[1].rgba = sample_lod(texpaint_nor, tex_coord_inp, 0.0);");
			node_shader_write_frag(kong, "output[2].rgba = sample_lod(texpaint_pack, tex_coord_inp, 0.0);");
			node_shader_write_frag(kong, "output[3].rg = tex_coord_inp.xy;");
		}
	}
}
