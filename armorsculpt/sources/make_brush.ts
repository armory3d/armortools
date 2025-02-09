
function make_brush_run(vert: node_shader_t, frag: node_shader_t) {

	node_shader_write(frag, "float dist = 0.0;");

	if (config_raw.brush_3d) {
		node_shader_write(frag, "float depth = textureLod(gbufferD, inp.xy, 0.0).r;");

		node_shader_add_uniform(frag, "mat4 invVP", "_inv_view_proj_matrix");
		node_shader_write(frag, "vec4 winp = vec4(vec2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write(frag, "winp = mul(winp, invVP);");
		node_shader_write(frag, "winp.xyz /= winp.w;");

		node_shader_add_uniform(frag, "mat4 W", "_world_matrix");

		node_shader_write_attrib(frag, "vec3 wposition = (mul(texelFetch(texpaint_undo, ivec2(tex_coord.x * textureSize(texpaint_undo, 0).x, tex_coord.y * textureSize(texpaint_undo, 0).y), 0), W)).xyz;");

		node_shader_write(frag, "float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;");

		node_shader_write(frag, "vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);");
		node_shader_write(frag, "winplast = mul(winplast, invVP);");
		node_shader_write(frag, "winplast.xyz /= winplast.w;");

		node_shader_write(frag, "dist = distance(wposition, winp.xyz);");
	}

	node_shader_write(frag, "if (dist > brush_radius) discard;");
}
