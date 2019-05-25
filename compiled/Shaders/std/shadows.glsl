#ifndef _SHADOWS_GLSL_
#define _SHADOWS_GLSL_

#include "compiled.inc"

#ifdef _CSM
uniform vec4 casData[shadowmapCascades * 4 + 4];
#endif

#ifdef _SMSizeUniform
uniform vec2 smSizeUniform;
#endif

float PCF(sampler2DShadow shadowMap, const vec2 uv, const float compare, const vec2 smSize) {
	float result = texture(shadowMap, vec3(uv + (vec2(-1.0, -1.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv + (vec2(-1.0, 0.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv + (vec2(-1.0, 1.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv + (vec2(0.0, -1.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv, compare));
	result += texture(shadowMap, vec3(uv + (vec2(0.0, 1.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv + (vec2(1.0, -1.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv + (vec2(1.0, 0.0) / smSize), compare));
	result += texture(shadowMap, vec3(uv + (vec2(1.0, 1.0) / smSize), compare));
	return result / 9.0;
}

float lpToDepth(vec3 lp, const vec2 lightProj) {
	lp = abs(lp);
	float zcomp = max(lp.x, max(lp.y, lp.z));
	zcomp = lightProj.x - lightProj.y / zcomp;
	return zcomp * 0.5 + 0.5;
}

float PCFCube(samplerCubeShadow shadowMapCube, const vec3 lp, vec3 ml, const float bias, const vec2 lightProj, const vec3 n) {
	const float s = shadowmapCubePcfSize; // TODO: incorrect...
	float compare = lpToDepth(lp, lightProj) - bias * 1.5;
	ml = ml + n * bias * 20;
	#ifdef HLSL
	ml.y = -ml.y;
	#endif
	float result = texture(shadowMapCube, vec4(ml, compare));
	result += texture(shadowMapCube, vec4(ml + vec3(s, s, s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(-s, s, s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(s, -s, s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(s, s, -s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(-s, -s, s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(s, -s, -s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(-s, s, -s), compare));
	result += texture(shadowMapCube, vec4(ml + vec3(-s, -s, -s), compare));
	return result / 9.0;
}

float shadowTest(sampler2DShadow shadowMap, const vec3 lPos, const float shadowsBias) {
	#ifdef _SMSizeUniform
	vec2 smSize = smSizeUniform;
	#else
	const vec2 smSize = shadowmapSize;
	#endif
	if (lPos.x < 0.0 || lPos.y < 0.0 || lPos.x > 1.0 || lPos.y > 1.0) return 1.0;
	return PCF(shadowMap, lPos.xy, lPos.z - shadowsBias, smSize);
}

#ifdef _CSM
mat4 getCascadeMat(const float d, out int casi, out int casIndex) {
	const int c = shadowmapCascades;

	// Get cascade index
	// TODO: use bounding box slice selection instead of sphere
	const vec4 ci = vec4(float(c > 0), float(c > 1), float(c > 2), float(c > 3));
	// int ci;
	// if (d < casData[c * 4].x) ci = 0;
	// else if (d < casData[c * 4].y) ci = 1 * 4;
	// else if (d < casData[c * 4].z) ci = 2 * 4;
	// else ci = 3 * 4;
	// Splits
	vec4 comp = vec4(
		float(d > casData[c * 4].x),
		float(d > casData[c * 4].y),
		float(d > casData[c * 4].z),
		float(d > casData[c * 4].w));
	casi = int(min(dot(ci, comp), c));

	// Get cascade mat
	casIndex = casi * 4;

	return mat4(
		casData[casIndex    ],
		casData[casIndex + 1],
		casData[casIndex + 2],
		casData[casIndex + 3]);

	// if (casIndex == 0) return mat4(casData[0], casData[1], casData[2], casData[3]);
	// ..
}

float shadowTestCascade(sampler2DShadow shadowMap, const vec3 eye, const vec3 p, const float shadowsBias) {
	#ifdef _SMSizeUniform
	vec2 smSize = smSizeUniform * vec2(shadowmapCascades, 1.0);
	#else
	const vec2 smSize = shadowmapSize * vec2(shadowmapCascades, 1.0);
	#endif
	const int c = shadowmapCascades;
	float d = distance(eye, p);

	int casi;
	int casIndex;
	mat4 LWVP = getCascadeMat(d, casi, casIndex);
	
	vec4 lPos = LWVP * vec4(p, 1.0);
	lPos.xyz /= lPos.w;

	float visibility = 1.0;
	if (lPos.w > 0.0) visibility = PCF(shadowMap, lPos.xy, lPos.z - shadowsBias, smSize);

	// Blend cascade
	// https://github.com/TheRealMJP/Shadows
	const float blendThres = 0.15;
	float nextSplit = casData[c * 4][casi];
	float splitSize = casi == 0 ? nextSplit : nextSplit - casData[c * 4][casi - 1];
	float splitDist = (nextSplit - d) / splitSize;
	if (splitDist <= blendThres && casi != c - 1) {
		int casIndex2 = casIndex + 4;
		mat4 LWVP2 = mat4(
			casData[casIndex2    ],
			casData[casIndex2 + 1],
			casData[casIndex2 + 2],
			casData[casIndex2 + 3]);

		vec4 lPos2 = LWVP2 * vec4(p, 1.0);
		lPos2.xyz /= lPos2.w;
		float visibility2 = 1.0;
		if (lPos2.w > 0.0) visibility2 = PCF(shadowMap, lPos2.xy, lPos2.z - shadowsBias, smSize);

		float lerpAmt = smoothstep(0.0, blendThres, splitDist);
		return mix(visibility2, visibility, lerpAmt);
	}
	return visibility;

	// Visualize cascades
	// if (ci == 0) albedo.rgb = vec3(1.0, 0.0, 0.0);
	// if (ci == 4) albedo.rgb = vec3(0.0, 1.0, 0.0);
	// if (ci == 8) albedo.rgb = vec3(0.0, 0.0, 1.0);
	// if (ci == 12) albedo.rgb = vec3(1.0, 1.0, 0.0);
}
#endif

#endif
