#version 450

uniform sampler2D tex0;
uniform sampler2D texa;

in vec2 texCoord;
out vec4 FragColor;

void main() {
	vec4 col = textureLod(tex0, texCoord, 0);
	float mask = clamp(textureLod(texa, texCoord, 0).r + 0.5, 0.0, 1.0);
	FragColor = col * mask;
}
