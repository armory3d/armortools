#version 330
in vec3 pos;
in vec2 tex;
in vec4 col;
uniform mat4 projectionMatrix;
out vec2 texCoord;
out vec4 color;
void main() {
	gl_Position = projectionMatrix * vec4(pos, 1.0);
	texCoord = tex;
	color = col;
}
