#version 450

in vec3 pos;
in vec3 col;

uniform mat4 VP;
out vec3 color;

void main() {
	color = col;
	gl_Position = VP * vec4(pos, 1.0);
}
