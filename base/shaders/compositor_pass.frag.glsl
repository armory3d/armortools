#version 450

uniform sampler2D tex;
// uniform sampler2D histogram;
uniform float vignette_strength;
uniform float grain_strength;

in vec2 tex_coord;
out vec4 frag_color;

// Based on Filmic Tonemapping Operators http://filmicgames.com/archives/75
vec3 tonemap_filmic(const vec3 color) {
	vec3 x = max(vec3(0.0, 0.0, 0.0), color - 0.004);
	return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

void main() {
	frag_color = textureLod(tex, tex_coord, 0.0);

	// Static grain
	float x = (tex_coord.x + 4.0) * (tex_coord.y + 4.0) * 10.0;
	float g = mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005;
	frag_color.rgb += vec3(g, g, g) * grain_strength;

	frag_color.rgb *= (1.0 - vignette_strength) + vignette_strength * pow(16.0 * tex_coord.x * tex_coord.y * (1.0 - tex_coord.x) * (1.0 - tex_coord.y), 0.2);

	// Auto exposure
	// const float auto_exposure_strength = 1.0;
	// float expo = 2.0 - clamp(length(textureLod(histogram, vec2(0.5, 0.5), 0.0).rgb), 0.0, 1.0);
	// frag_color.rgb *= pow(expo, auto_exposure_strength * 2.0);

	frag_color.rgb = tonemap_filmic(frag_color.rgb); // With gamma
}
