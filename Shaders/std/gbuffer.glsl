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

vec3 getPosNoEye(const vec3 eyeLook, const vec3 viewRay, const float depth, const vec2 cameraProj) {
	// eyeLook, viewRay should be normalized
	float linearDepth = cameraProj.y / ((depth * 0.5 + 0.5) - cameraProj.x);
	float viewZDist = dot(eyeLook, viewRay);
	vec3 wposition = viewRay * (linearDepth / viewZDist);
	return wposition;
}

#if defined(HLSL) || defined(METAL) || defined(SPIRV)
vec3 getPos2(const mat4 invVP, const float depth, vec2 coord) {
	coord.y = 1.0 - coord.y;
#else
vec3 getPos2(const mat4 invVP, const float depth, const vec2 coord) {
#endif
	vec4 pos = vec4(coord * 2.0 - 1.0, depth, 1.0);
	pos = invVP * pos;
	pos.xyz /= pos.w;
	return pos.xyz;
}

#if defined(HLSL) || defined(METAL) || defined(SPIRV)
vec3 getPosView2(const mat4 invP, const float depth, vec2 coord) {
	coord.y = 1.0 - coord.y;
#else
vec3 getPosView2(const mat4 invP, const float depth, const vec2 coord) {
#endif
	vec4 pos = vec4(coord * 2.0 - 1.0, depth, 1.0);
	pos = invP * pos;
	pos.xyz /= pos.w;
	return pos.xyz;
}

#if defined(HLSL) || defined(METAL) || defined(SPIRV)
vec3 getPos2NoEye(const vec3 eye, const mat4 invVP, const float depth, vec2 coord) {
	coord.y = 1.0 - coord.y;
#else
vec3 getPos2NoEye(const vec3 eye, const mat4 invVP, const float depth, const vec2 coord) {
#endif
	vec4 pos = vec4(coord * 2.0 - 1.0, depth, 1.0);
	pos = invVP * pos;
	pos.xyz /= pos.w;
	return pos.xyz - eye;
}

vec4 encodeRGBM(const vec3 rgb) {
	const float maxRange = 6.0;
	float maxRGB = max(rgb.x, max(rgb.g, rgb.b));
	float m = maxRGB / maxRange;
	m = ceil(m * 255.0) / 255.0;
	return vec4(rgb / (m * maxRange), m);
}

vec3 decodeRGBM(const vec4 rgbm) {
	const float maxRange = 6.0;
    return rgbm.rgb * rgbm.a * maxRange;
}

uint encNor(vec3 n) {
	ivec3 nor = ivec3(n * 255.0f);
	uvec3 norSigns;
	norSigns.x = (nor.x >> 5) & 0x04000000;
	norSigns.y = (nor.y >> 14) & 0x00020000;
	norSigns.z = (nor.z >> 23) & 0x00000100;
	nor = abs(nor);
	uint val = norSigns.x | (nor.x << 18) | norSigns.y | (nor.y << 9) | norSigns.z | nor.z;
	return val;
}

vec3 decNor(uint val) {
	uvec3 nor;
	nor.x = (val >> 18) & 0x000000ff;
	nor.y = (val >> 9) & 0x000000ff;
	nor.z = val & 0x000000ff;
	uvec3 norSigns;
	norSigns.x = (val >> 25) & 0x00000002;
	norSigns.y = (val >> 16) & 0x00000002;
	norSigns.z = (val >> 7) & 0x00000002;
	norSigns = 1 - norSigns;
	vec3 normal = vec3(nor) / 255.0f;
	normal *= norSigns;
	return normal;
}

// GBuffer helper - Sebastien Lagarde
// https://seblagarde.wordpress.com/2018/09/02/gbuffer-helper-packing-integer-and-float-together/
float packFloatInt8(const float f, const uint i) {
	// Constant optimize by compiler
	const int numBitTarget = 8;
	const int numBitI = 4;
	const float prec = float(1 << numBitTarget);
	const float maxi = float(1 << numBitI);
	const float precMinusOne = prec - 1.0;
	const float t1 = ((prec / maxi) - 1.0) / precMinusOne;
	const float t2 = (prec / maxi) / precMinusOne;
	// Code
	return t1 * f + t2 * float(i);
}

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

void unpackFloatInt8(const float val, out float f, out uint i) {
	// Constant optimize by compiler
	const int numBitTarget = 8;
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
