#version 450

in vec2 tex_coord;

out vec4 frag_color;

uniform sampler2D my_texture;

void main() {
	frag_color = texture(my_texture, tex_coord);
}
