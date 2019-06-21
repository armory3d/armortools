#version 450

in vec3 color;
out vec4 fragColor[2];

void main() {
	fragColor[0] = vec4(1.0, 1.0, 0.0, 1.0);
	fragColor[1] = vec4(color, 1.0);
}
