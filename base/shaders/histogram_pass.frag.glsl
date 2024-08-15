#version 450

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	const float auto_exposure_speed = 1.0;
	frag_color.a = 0.01 * auto_exposure_speed;
	frag_color.rgb = textureLod(tex, vec2(0.5, 0.5), 0.0).rgb +
					textureLod(tex, vec2(0.2, 0.2), 0.0).rgb +
					textureLod(tex, vec2(0.8, 0.2), 0.0).rgb +
					textureLod(tex, vec2(0.2, 0.8), 0.0).rgb +
					textureLod(tex, vec2(0.8, 0.8), 0.0).rgb;
	frag_color.rgb /= 5.0;
}
