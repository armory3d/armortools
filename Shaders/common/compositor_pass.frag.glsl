#version 450

#define _CToneFilmic
#define _CVignette
#define _CGrainStatic
// #define _AutoExposure

#include "../std/tonemap.glsl"

uniform sampler2D tex;

#ifdef _AutoExposure
uniform sampler2D histogram;
#endif

#ifdef _CVignette
uniform float vignetteStrength;
#endif

in vec2 texCoord;
out vec4 fragColor;

void main() {
	fragColor = textureLod(tex, texCoord, 0.0);

#ifdef _CGrainStatic
	float x = (texCoord.x + 4.0) * (texCoord.y + 4.0) * 10.0;
	fragColor.rgb += vec3(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * 0.09;
#endif

#ifdef _CVignette
	fragColor.rgb *= (1.0 - vignetteStrength) + vignetteStrength * pow(16.0 * texCoord.x * texCoord.y * (1.0 - texCoord.x) * (1.0 - texCoord.y), 0.2);
#endif

#ifdef _AutoExposure
	const float autoExposureStrength = 1.0;
	float expo = 2.0 - clamp(length(textureLod(histogram, vec2(0.5, 0.5), 0).rgb), 0.0, 1.0);
	fragColor.rgb *= pow(expo, autoExposureStrength * 2.0);
#endif

#ifdef _CToneFilmic
	fragColor.rgb = tonemapFilmic(fragColor.rgb); // With gamma
#endif
}
