#version 450

in vec4 fragment_color;
out vec4 frag_color;

void main() {
	frag_color = fragment_color;
}
