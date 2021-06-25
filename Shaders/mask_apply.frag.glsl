#version 330
uniform sampler2D tex0;
uniform sampler2D texa;
in vec2 texCoord;
out vec4 FragColor;
void main() {
	vec4 col0 = textureLod(tex0, texCoord, 0);
	float mask = textureLod(texa, texCoord, 0).r;
	FragColor = vec4(col0.rgb, col0.a * mask);
}
