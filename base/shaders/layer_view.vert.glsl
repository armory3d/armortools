#version 450

uniform mat4 projectionMatrix;

in vec3 pos;
in vec2 tex;
in vec4 col;
out vec2 texCoord;
out vec4 color;

void main() {
	gl_Position = projectionMatrix * vec4(pos, 1.0);
	texCoord = tex;
	color = col;
}
