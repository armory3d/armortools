#version 450

uniform sampler2D tex;
uniform vec2 screen_size_inv;
uniform int current_mip_level;

in vec2 tex_coord;
out vec4 frag_color;

const float bloom_knee = 0.5;
const float bloom_threshold = 0.8;
const float epsilon = 6.2e-5;

vec3 downsample_dual_filter(const sampler2D tex, const vec2 tex_coord, const vec2 texel_size) {
	vec3 delta = texel_size.xyx * vec3(0.5, 0.5, -0.5);

	vec3 result;
	result  = textureLod(tex, tex_coord,            0.0).rgb * 4.0;
	result += textureLod(tex, tex_coord - delta.xy, 0.0).rgb;
	result += textureLod(tex, tex_coord - delta.zy, 0.0).rgb;
	result += textureLod(tex, tex_coord + delta.zy, 0.0).rgb;
	result += textureLod(tex, tex_coord + delta.xy, 0.0).rgb;

	return result * (1.0 / 8.0);
}

void main() {
	frag_color.rgb = downsample_dual_filter(tex, tex_coord, screen_size_inv);

	if (current_mip_level == 0) {
		float brightness = max(frag_color.r, max(frag_color.g, frag_color.b));

		float softening_curve = brightness - bloom_threshold + bloom_knee;
		softening_curve = clamp(softening_curve, 0.0, 2.0 * bloom_knee);
		softening_curve = softening_curve * softening_curve / (4 * bloom_knee + epsilon);

		float contribution_factor = max(softening_curve, brightness - bloom_threshold);

		contribution_factor /= max(epsilon, brightness);

		frag_color.rgb *= contribution_factor;
	}

	frag_color.a = 1.0;
}
