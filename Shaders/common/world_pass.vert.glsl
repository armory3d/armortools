#version 450

in vec3 pos;
in vec3 nor;

out vec3 normal;

uniform mat4 SMVP;

void main() {
	normal = nor;
	gl_Position = SMVP * vec4(pos, 1.0);
}
