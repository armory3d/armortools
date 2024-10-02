#version 450

#include "std/gbuffer.glsl"

uniform sampler2D tex;
uniform sampler2D gbuffer0;
uniform vec2 dir_inv;

in vec2 tex_coord;
out float frag_color;

const float blur_weights[10] = { 0.132572, 0.125472, 0.106373, 0.08078, 0.05495, 0.033482, 0.018275, 0.008934, 0.003912, 0.001535 };
const float discard_threshold = 0.95;

void main() {
	vec3 nor = get_nor(textureLod(gbuffer0, tex_coord, 0.0).rg);

	frag_color = textureLod(tex, tex_coord, 0.0).r * blur_weights[0];
	float weight = blur_weights[0];

	for (int i = 1; i < 8; ++i) {
		float posadd = i;

		vec3 nor2 = get_nor(textureLod(gbuffer0, tex_coord + i * dir_inv, 0.0).rg);
		float influence_factor = step(discard_threshold, dot(nor2, nor));
		float col = textureLod(tex, tex_coord + posadd * dir_inv, 0.0).r;
		float w = blur_weights[i] * influence_factor;
		frag_color += col * w;
		weight += w;

		nor2 = get_nor(textureLod(gbuffer0, tex_coord - i * dir_inv, 0.0).rg);
		influence_factor = step(discard_threshold, dot(nor2, nor));
		col = textureLod(tex, tex_coord - posadd * dir_inv, 0.0).r;
		w = blur_weights[i] * influence_factor;
		frag_color += col * w;
		weight += w;
	}

	frag_color = frag_color / weight;
}
