#version 450

uniform mat4 VP;

in vec3 pos;
out vec3 dummy;

void main() {
	gl_Position = mul(vec4(pos, 1.0), VP);
}
