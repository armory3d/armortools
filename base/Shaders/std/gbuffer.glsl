
#ifndef _GBUFFER_GLSL_
#define _GBUFFER_GLSL_

vec2 octahedronWrap(const vec2 v) {
	return (1.0 - abs(v.yx)) * (vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0));
}

vec3 getNor(const vec2 enc) {
	vec3 n;
	n.z = 1.0 - abs(enc.x) - abs(enc.y);
	n.xy = n.z >= 0.0 ? enc.xy : octahedronWrap(enc.xy);
	n = normalize(n);
	return n;
}

vec3 getPosView(const vec3 viewRay, const float depth, const vec2 cameraProj) {
	float linearDepth = cameraProj.y / (cameraProj.x - depth);
	// float linearDepth = cameraProj.y / ((depth * 0.5 + 0.5) - cameraProj.x);
	return viewRay * linearDepth;
}

vec3 getPos(const vec3 eye, const vec3 eyeLook, const vec3 viewRay, const float depth, const vec2 cameraProj) {
	// eyeLook, viewRay should be normalized
	float linearDepth = cameraProj.y / ((depth * 0.5 + 0.5) - cameraProj.x);
	float viewZDist = dot(eyeLook, viewRay);
	vec3 wposition = eye + viewRay * (linearDepth / viewZDist);
	return wposition;
}

// GBuffer helper - Sebastien Lagarde
// https://seblagarde.wordpress.com/2018/09/02/gbuffer-helper-packing-integer-and-float-together/
float packFloatInt16(const float f, const uint i) {
	// Constant optimize by compiler
	const int numBitTarget = 16;
	const int numBitI = 4;
	const float prec = float(1 << numBitTarget);
	const float maxi = float(1 << numBitI);
	const float precMinusOne = prec - 1.0;
	const float t1 = ((prec / maxi) - 1.0) / precMinusOne;
	const float t2 = (prec / maxi) / precMinusOne;
	// Code
	return t1 * f + t2 * float(i);
}

void unpackFloatInt16(const float val, out float f, out uint i) {
	// Constant optimize by compiler
	const int numBitTarget = 16;
	const int numBitI = 4;
	const float prec = float(1 << numBitTarget);
	const float maxi = float(1 << numBitI);
	const float precMinusOne = prec - 1.0;
	const float t1 = ((prec / maxi) - 1.0) / precMinusOne;
	const float t2 = (prec / maxi) / precMinusOne;
	// Code
	// extract integer part
	// + rcp(precMinusOne) to deal with precision issue
	i = int((val / t2) + (1.0 / precMinusOne));
	// Now that we have i, solve formula in packFloatInt for f
	//f = (val - t2 * float(i)) / t1 => convert in mads form
	f = clamp((-t2 * float(i) + val) / t1, 0.0, 1.0); // Saturate in case of precision issue
}

#endif
