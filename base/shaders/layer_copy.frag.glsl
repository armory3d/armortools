#version 450

uniform sampler2D tex;

in vec2 tex_coord;
in vec4 color;
out vec4 frag_color;

void main() {
	frag_color = textureLod(tex, tex_coord, 0.0).rgba * color;
}
