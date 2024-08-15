#version 450

uniform mat4 invVP;
uniform vec3 eye;

in vec2 pos;

out vec2 tex_coord;
out vec3 view_ray;

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	tex_coord = pos.xy * madd + madd;
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	tex_coord.y = 1.0 - tex_coord.y;
	#endif

	gl_Position = vec4(pos.xy, 0.0, 1.0);

	// NDC (at the back of cube)
	vec4 v = vec4(pos.x, pos.y, 1.0, 1.0);
	v = vec4(invVP * v);
	v.xyz /= v.w;
	view_ray = v.xyz - eye;
}
