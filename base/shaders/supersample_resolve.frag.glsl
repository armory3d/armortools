#version 450

uniform sampler2D tex;
uniform vec2 screen_size_inv;

in vec2 tex_coord;
out vec4 frag_color;

vec4 texture_ss(sampler2D tex, vec2 tc, vec2 tex_step) {
	vec4 col = texture(tex, tc);
	col += texture(tex, tc + vec2(1.5, 0.0) * tex_step);
	col += texture(tex, tc + vec2(-1.5, 0.0) * tex_step);
	col += texture(tex, tc + vec2(0.0, 1.5) * tex_step);
	col += texture(tex, tc + vec2(0.0, -1.5) * tex_step);
	col += texture(tex, tc + vec2(1.5, 1.5) * tex_step);
	col += texture(tex, tc + vec2(-1.5, -1.5) * tex_step);
	col += texture(tex, tc + vec2(1.5, -1.5) * tex_step);
	col += texture(tex, tc + vec2(-1.5, 1.5) * tex_step);
	return col / 9.0;
}

void main() {
	// 4X resolve
	frag_color = texture_ss(tex, tex_coord, screen_size_inv / 4.0);
}
