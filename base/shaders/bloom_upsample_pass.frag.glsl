#version 450

uniform sampler2D tex;
uniform vec2 screen_size_inv;
uniform int current_mip_level;
uniform float sample_scale;

in vec2 tex_coord;
out vec4 frag_color;

const float bloom_strength = 0.02;

vec3 upsample_dual_filter(const sampler2D tex, const vec2 tex_coord, const vec2 texel_size) {
	vec2 delta = texel_size * sample_scale;

	vec3 result;
	result  = textureLod(tex, tex_coord + vec2(-delta.x * 2.0, 0.0), 0.0).rgb;
	result += textureLod(tex, tex_coord + vec2(-delta.x, delta.y),   0.0).rgb * 2.0;
	result += textureLod(tex, tex_coord + vec2(0.0, delta.y * 2.0),  0.0).rgb;
	result += textureLod(tex, tex_coord + delta,                     0.0).rgb * 2.0;
	result += textureLod(tex, tex_coord + vec2(delta.x * 2.0, 0.0),  0.0).rgb;
	result += textureLod(tex, tex_coord + vec2(delta.x, -delta.y),   0.0).rgb * 2.0;
	result += textureLod(tex, tex_coord + vec2(0.0, -delta.y * 2.0), 0.0).rgb;
	result += textureLod(tex, tex_coord - delta,                     0.0).rgb * 2.0;

	return result * (1.0 / 12.0);
}

void main() {
	frag_color.rgb = upsample_dual_filter(tex, tex_coord, screen_size_inv);

	if (current_mip_level == 0) {
		frag_color.rgb *= bloom_strength;
	}

	frag_color.a = 1.0;
}
