#version 450

uniform vec3 tint;
#ifdef SPIRV
uniform sampler2D gbufferD; // vulkan unit align
#endif
uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	vec4 col = texture(tex, tex_coord);
	frag_color = vec4((col.rgb / col.a) * tint, col.a);
}
