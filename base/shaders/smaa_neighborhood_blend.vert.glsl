#version 450

uniform vec2 screen_size_inv;

in vec2 pos;

out vec2 tex_coord;
out vec4 offset;

#if defined(HLSL) || defined(METAL) || defined(SPIRV)
#define V_DIR(v) -(v)
#else
#define V_DIR(v) v
#endif

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	tex_coord = pos.xy * madd + madd;
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	tex_coord.y = 1.0 - tex_coord.y;
	#endif

	// Neighborhood Blending Vertex Shader
	offset = screen_size_inv.xyxy * vec4(1.0, 0.0, 0.0, V_DIR(1.0)) + tex_coord.xyxy;
	gl_Position = vec4(pos.xy, 0.0, 1.0);
}