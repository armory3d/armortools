#version 450

uniform sampler2D tex;
uniform vec2 screenSizeInv;
uniform int currentMipLevel;
uniform float sampleScale;

in vec2 texCoord;
out vec4 fragColor;

const float bloomStrength = 0.02;

vec3 upsample_dual_filter(const sampler2D tex, const vec2 texCoord, const vec2 texelSize) {
	vec2 delta = texelSize * sampleScale;

	vec3 result;
	result  = textureLod(tex, texCoord + vec2(-delta.x * 2.0, 0.0), 0.0).rgb;
	result += textureLod(tex, texCoord + vec2(-delta.x, delta.y),   0.0).rgb * 2.0;
	result += textureLod(tex, texCoord + vec2(0.0, delta.y * 2.0),  0.0).rgb;
	result += textureLod(tex, texCoord + delta,                     0.0).rgb * 2.0;
	result += textureLod(tex, texCoord + vec2(delta.x * 2.0, 0.0),  0.0).rgb;
	result += textureLod(tex, texCoord + vec2(delta.x, -delta.y),   0.0).rgb * 2.0;
	result += textureLod(tex, texCoord + vec2(0.0, -delta.y * 2.0), 0.0).rgb;
	result += textureLod(tex, texCoord - delta,                     0.0).rgb * 2.0;

	return result * (1.0 / 12.0);
}

void main() {
	fragColor.rgb = upsample_dual_filter(tex, texCoord, screenSizeInv);

	if (currentMipLevel == 0) {
		fragColor.rgb *= bloomStrength;
	}

	fragColor.a = 1.0;
}
