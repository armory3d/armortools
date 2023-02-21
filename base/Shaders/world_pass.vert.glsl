#version 450

uniform mat4 SMVP;

in vec3 pos;
in vec3 nor;

out vec3 normal;

void main() {
	normal = nor;
	gl_Position = SMVP * vec4(pos, 1.0);
}
