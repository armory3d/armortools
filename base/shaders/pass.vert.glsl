#version 450

in vec2 pos;
out vec2 tex_coord;

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	tex_coord = pos.xy * madd + madd;

#ifdef GLSL
#else
	tex_coord.y = 1.0 - tex_coord.y;
#endif

	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
