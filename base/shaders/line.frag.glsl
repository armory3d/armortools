#version 450

uniform vec3 color;

in vec3 dummy;
out vec4 frag_color[2];

void main() {
	frag_color[0] = vec4(1.0, 1.0, 0.0, 1.0);
	frag_color[1] = vec4(color, 1.0);
}
