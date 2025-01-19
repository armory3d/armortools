#version 450

uniform mat4 VP;
in vec3 pos;

void main() {
	gl_Position = mul(vec4(pos, 1.0), VP);
}
