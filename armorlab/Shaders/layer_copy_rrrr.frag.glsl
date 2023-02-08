#version 330
uniform sampler2D tex;
in vec2 texCoord;
out vec4 FragColor;
void main() {
	FragColor = textureLod(tex, texCoord, 0).rrrr;
}
