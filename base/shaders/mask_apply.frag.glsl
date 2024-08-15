#version 450

uniform sampler2D tex0;
uniform sampler2D texa;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	vec4 col0 = textureLod(tex0, tex_coord, 0);
	float mask = textureLod(texa, tex_coord, 0).r;
	frag_color = vec4(col0.rgb, col0.a * mask);
}
