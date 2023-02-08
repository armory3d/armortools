
#ifndef _RAND_HLSL_
#define _RAND_HLSL_

// A Low-Discrepancy Sampler that Distributes Monte Carlo Errors as a Blue Noise in Screen Space
// Eric Heitz, Laurent Belcour, Victor Ostromoukhov, David Coeurjolly and Jean-Claude Iehl
// https://eheitzresearch.wordpress.com/762-2/
float rand(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension, int frame, Texture2D<float4> sobol, Texture2D<float4> scramble, Texture2D<float4> rank) {
	// wrap arguments
	pixel_i += frame * 9;
	pixel_j += frame * 11;
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	// xor index based on optimized ranking
	int i = sampleDimension + (pixel_i + pixel_j*128)*8;
	int rankedSampleIndex = sampleIndex ^ int(rank.Load(uint3(i % 128, uint(i / 128), 0)).r * 255);

	// fetch value in sequence
	i = sampleDimension + rankedSampleIndex*256;
	int value = int(sobol.Load(uint3(i % 256, uint(i / 256), 0)).r * 255);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	i = (sampleDimension%8) + (pixel_i + pixel_j*128)*8;
	value = value ^ int(scramble.Load(uint3(i % 128, uint(i / 128), 0)).r * 255);

	// convert to float and return
	float v = (0.5f+value)/256.0f;
	return v;
}

#endif
