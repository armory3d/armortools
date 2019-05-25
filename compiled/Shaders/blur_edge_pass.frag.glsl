// Exclusive to SSAO for now
#version 450

#include "compiled.inc"
#include "std/gbuffer.glsl"

uniform sampler2D tex;
uniform sampler2D gbuffer0;

uniform vec2 dirInv; // texStep

in vec2 texCoord;
out float fragColor;

// const float blurWeights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
const float blurWeights[10] = float[] (0.132572, 0.125472, 0.106373, 0.08078, 0.05495, 0.033482, 0.018275, 0.008934, 0.003912, 0.001535);
const float discardThreshold = 0.95;

void main() {
	vec3 nor = getNor(textureLod(gbuffer0, texCoord, 0.0).rg);
	
	fragColor = textureLod(tex, texCoord, 0.0).r * blurWeights[0];
	float weight = blurWeights[0];

	for (int i = 1; i < 8; ++i) {
		float posadd = i;// + 0.5;

		vec3 nor2 = getNor(textureLod(gbuffer0, texCoord + i * dirInv, 0.0).rg);
		float influenceFactor = step(discardThreshold, dot(nor2, nor));
		float col = textureLod(tex, texCoord + posadd * dirInv, 0.0).r;
		float w = blurWeights[i] * influenceFactor;
		fragColor += col * w;
		weight += w;
		
		nor2 = getNor(textureLod(gbuffer0, texCoord - i * dirInv, 0.0).rg);
		influenceFactor = step(discardThreshold, dot(nor2, nor));
		col = textureLod(tex, texCoord - posadd * dirInv, 0.0).r;
		w = blurWeights[i] * influenceFactor;
		fragColor += col * w;
		weight += w;
	}

	fragColor = fragColor / weight;
}