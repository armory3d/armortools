
function make_particle_mask(kong: node_shader_t) {
	node_shader_add_out(kong, "wpos: float4");
	node_shader_add_constant(kong, "W: float4x4", "_world_matrix");
	node_shader_write_attrib_vert(kong, "output.wpos = constants.W * float4(input.pos.xyz, 1.0);");
	node_shader_add_constant(kong, "particle_hit: float3", "_particle_hit");
	node_shader_add_constant(kong, "particle_hit_last: float3", "_particle_hit_last");

	node_shader_write_frag(kong, "var pa: float3 = input.wpos.xyz - constants.particle_hit;");
	node_shader_write_frag(kong, "var ba: float3 = constants.particle_hit_last - constants.particle_hit;");
	node_shader_write_frag(kong, "var h: float = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
	node_shader_write_frag(kong, "dist = length(pa - ba * h) * 10.0;");

	node_shader_write_frag(kong, "if (dist > 1.0) { discard; }");
	node_shader_write_frag(kong, "var str: float = clamp(pow(1.0 / dist * constants.brush_hardness * 0.2, 4.0), 0.0, 1.0) * opacity;");
	node_shader_write_frag(kong, "if (constants.particle_hit.x == 0.0 && constants.particle_hit.y == 0.0 && constants.particle_hit.z == 0.0) { str = 0.0; }");
	node_shader_write_frag(kong, "if (str == 0.0) { discard; }");
}
