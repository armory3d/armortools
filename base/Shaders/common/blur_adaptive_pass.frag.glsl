// Exclusive to SSR for now
#version 450

#include "../std/gbuffer.glsl"

uniform sampler2D tex;
uniform sampler2D gbuffer0; // Roughness

uniform vec2 dirInv;

in vec2 texCoord;
out vec4 fragColor;

void main() {
	float roughness = textureLod(gbuffer0, texCoord, 0.0).b;
	// if (roughness == 0.0) { // Always blur for now, non blured output can produce noise
		// fragColor.rgb = textureLod(tex, texCoord).rgb;
		// return;
	// }
	if (roughness >= 0.8) { // No reflections
		fragColor.rgb = textureLod(tex, texCoord, 0.0).rgb;
		return;
	}

	fragColor.rgb = textureLod(tex, texCoord + dirInv * 2.5, 0.0).rgb;
	fragColor.rgb += textureLod(tex, texCoord + dirInv * 1.5, 0.0).rgb;
	fragColor.rgb += textureLod(tex, texCoord, 0.0).rgb;
	fragColor.rgb += textureLod(tex, texCoord - dirInv * 1.5, 0.0).rgb;
	fragColor.rgb += textureLod(tex, texCoord - dirInv * 2.5, 0.0).rgb;
	fragColor.rgb /= vec3(5.0);
}
