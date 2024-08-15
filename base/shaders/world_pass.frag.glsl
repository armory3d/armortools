#version 450

#include "std/math.glsl"

uniform sampler2D envmap;
uniform vec4 envmap_data_world; // angle, sin(angle), cos(angle), strength

in vec3 normal;
out vec4 frag_color;

void main() {
	vec3 n = normalize(normal);
	frag_color.rgb = texture(envmap, envmap_equirect(-n, envmap_data_world.x)).rgb * envmap_data_world.w;
	frag_color.a = 0.0; // Mark as non-opaque
}
