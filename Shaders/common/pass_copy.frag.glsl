#version 450

uniform sampler2D tex;

in vec2 texCoord;
out vec4 fragColor;

void main() {
	fragColor = textureLod(tex, texCoord, 0.0);
}
