#version 450

uniform mat4 VP;

in vec3 pos;
in vec3 col;
out vec3 color;

void main() {
	color = col;
	gl_Position = mul(vec4(pos, 1.0), VP);
}
