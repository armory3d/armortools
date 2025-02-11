
function make_particle_mask(vert: node_shader_t, frag: node_shader_t) {
	///if arm_physics
	node_shader_add_out(vert, "vec4 wpos");
	node_shader_add_uniform(vert, "mat4 W", "_world_matrix");
	node_shader_write_attrib(vert, "wpos = mul(vec4(pos.xyz, 1.0), W);");
	node_shader_add_uniform(frag, "vec3 particle_hit", "_particle_hit");
	node_shader_add_uniform(frag, "vec3 particle_hit_last", "_particle_hit_last");

	node_shader_write(frag, "vec3 pa = wpos.xyz - particle_hit;");
	node_shader_write(frag, "vec3 ba = particle_hit_last - particle_hit;");
	node_shader_write(frag, "float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
	node_shader_write(frag, "dist = length(pa - ba * h) * 10.0;");
	// write(frag, "dist = distance(particle_hit, wpos.xyz) * 10.0;");

	node_shader_write(frag, "if (dist > 1.0) discard;");
	node_shader_write(frag, "float str = clamp(pow(1.0 / dist * brush_hardness * 0.2, 4.0), 0.0, 1.0) * opacity;");
	node_shader_write(frag, "if (particle_hit.x == 0.0 && particle_hit.y == 0.0 && particle_hit.z == 0.0) str = 0.0;");
	node_shader_write(frag, "if (str == 0.0) discard;");
	///else
	node_shader_write(frag, "float str = 0.0;");
	///end
}
