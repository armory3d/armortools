
function make_brush_run(kong: node_shader_t) {

	node_shader_write_frag(kong, "var dist: float = 0.0;");

	if (config_raw.brush_3d) {
		node_shader_write_frag(kong, "var depth: float = sample_lod(gbufferD, inp.xy, 0.0).r;");

		node_shader_add_constant(kong, "invVP: float4x4", "_inv_view_proj_matrix");
		node_shader_write_frag(kong, "var winp: float4 = float4(float2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winp = constants.invVP * winp;");
		node_shader_write_frag(kong, "winp.xyz /= winp.w;");

		node_shader_add_constant(kong, "W: float4x4", "_world_matrix");

		node_shader_write_attrib_frag(kong, "var wposition: float3 = (constants.W * texpaint_undo[int2(tex_coord.x * textureSize(texpaint_undo, 0).x, tex_coord.y * textureSize(texpaint_undo, 0).y)]).xyz;");

		node_shader_write_frag(kong, "var depthlast: float = sample_lod(gbufferD, inplast.xy, 0.0).r;");

		node_shader_write_frag(kong, "var winplast: float4 = float4(float2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winplast = constants.invVP * winplast;");
		node_shader_write_frag(kong, "winplast.xyz /= winplast.w;");

		node_shader_write_frag(kong, "dist = distance(wposition, winp.xyz);");
	}

	node_shader_write_frag(kong, "if (dist > brush_radius) { discard; }");
}
