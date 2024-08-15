#version 450

#include "std/gbuffer.glsl"

uniform sampler2D tex;
uniform sampler2D gbuffer0; // Roughness
uniform vec2 dir_inv;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	float roughness = textureLod(gbuffer0, tex_coord, 0.0).b;
	if (roughness >= 0.8) { // No reflections
		frag_color.rgb = textureLod(tex, tex_coord, 0.0).rgb;
		return;
	}

	frag_color.rgb = textureLod(tex, tex_coord + dir_inv * 2.5, 0.0).rgb;
	frag_color.rgb += textureLod(tex, tex_coord + dir_inv * 1.5, 0.0).rgb;
	frag_color.rgb += textureLod(tex, tex_coord, 0.0).rgb;
	frag_color.rgb += textureLod(tex, tex_coord - dir_inv * 1.5, 0.0).rgb;
	frag_color.rgb += textureLod(tex, tex_coord - dir_inv * 2.5, 0.0).rgb;
	frag_color.rgb /= vec3(5.0);
}
