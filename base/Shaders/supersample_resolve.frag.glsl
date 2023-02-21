#version 450

uniform sampler2D tex;
uniform vec2 screenSizeInv;

in vec2 texCoord;
out vec4 fragColor;

vec4 textureSS(sampler2D tex, vec2 tc, vec2 texStep) {
	vec4 col = texture(tex, tc);
	col += texture(tex, tc + vec2(1.5, 0.0) * texStep);
	col += texture(tex, tc + vec2(-1.5, 0.0) * texStep);
	col += texture(tex, tc + vec2(0.0, 1.5) * texStep);
	col += texture(tex, tc + vec2(0.0, -1.5) * texStep);
	col += texture(tex, tc + vec2(1.5, 1.5) * texStep);
	col += texture(tex, tc + vec2(-1.5, -1.5) * texStep);
	col += texture(tex, tc + vec2(1.5, -1.5) * texStep);
	col += texture(tex, tc + vec2(-1.5, 1.5) * texStep);
	return col / 9.0;
}

void main() {
	// 4X resolve
	fragColor = textureSS(tex, texCoord, screenSizeInv / 4.0);
}
