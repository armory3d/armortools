#version 450

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

in vec2 tex_coord;
out vec4 frag_color[3];

void main() {
	frag_color[0] = textureLod(tex0, tex_coord, 0.0);
	frag_color[1] = textureLod(tex1, tex_coord, 0.0);
	frag_color[2] = textureLod(tex2, tex_coord, 0.0);
}
