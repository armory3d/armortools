#version 450

uniform sampler2D tex0;
uniform sampler2D texa;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	vec4 col = textureLod(tex0, tex_coord, 0.0);
	float mask = clamp(textureLod(texa, tex_coord, 0.0).r + 0.5, 0.0, 1.0);
	frag_color = col * mask;
}
