#version 450

uniform sampler2D tex;
uniform vec2 screenSizeInv;
uniform int currentMipLevel;

in vec2 texCoord;
out vec4 fragColor;

const float bloomKnee = 0.5;
const float bloomThreshold = 0.8;
const float epsilon = 6.2e-5;

vec3 downsample_dual_filter(const sampler2D tex, const vec2 texCoord, const vec2 texelSize) {
	vec3 delta = texelSize.xyx * vec3(0.5, 0.5, -0.5);

	vec3 result;
	result  = textureLod(tex, texCoord,            0.0).rgb * 4.0;
	result += textureLod(tex, texCoord - delta.xy, 0.0).rgb;
	result += textureLod(tex, texCoord - delta.zy, 0.0).rgb;
	result += textureLod(tex, texCoord + delta.zy, 0.0).rgb;
	result += textureLod(tex, texCoord + delta.xy, 0.0).rgb;

	return result * (1.0 / 8.0);
}

void main() {
	fragColor.rgb = downsample_dual_filter(tex, texCoord, screenSizeInv);

	if (currentMipLevel == 0) {
		float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

		float softeningCurve = brightness - bloomThreshold + bloomKnee;
		softeningCurve = clamp(softeningCurve, 0.0, 2.0 * bloomKnee);
		softeningCurve = softeningCurve * softeningCurve / (4 * bloomKnee + epsilon);

		float contributionFactor = max(softeningCurve, brightness - bloomThreshold);

		contributionFactor /= max(epsilon, brightness);

		fragColor.rgb *= contributionFactor;
	}

	fragColor.a = 1.0;
}
