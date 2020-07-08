#version 330
uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;
void main() {
	FragColor = vec4(1.0) - textureLod(tex, texCoord, 0).rgba * color;
}
