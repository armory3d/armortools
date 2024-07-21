
#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

#include "std/brdf.glsl"
#include "std/math.glsl"
#include "std/ltc.glsl"
#ifdef _Voxel
#include "std/conetrace.glsl"
#endif

uniform vec3 lightArea0;
uniform vec3 lightArea1;
uniform vec3 lightArea2;
uniform vec3 lightArea3;
uniform sampler2D sltcMat;
uniform sampler2D sltcMag;

vec3 sampleLight(const vec3 p, const vec3 n, const vec3 v, const float dotNV, const vec3 lp, const vec3 lightCol,
	const vec3 albedo, const float rough, const vec3 f0, const float occ
#ifdef _Voxel
	, sampler3D voxels, vec3 voxpos
#endif
	) {
	vec3 ld = lp - p;
	vec3 l = normalize(ld);
	float dotNL = max(0.0, dot(n, l));

	float theta = acos(dotNV);
	vec2 tuv = vec2(rough, theta / (0.5 * PI));
	tuv = tuv * LUT_SCALE + LUT_BIAS;
	vec4 t = textureLod(sltcMat, tuv, 0.0);
	mat3 invM = mat3(
		vec3(1.0, 0.0, t.y),
		vec3(0.0, t.z, 0.0),
		vec3(t.w, 0.0, t.x));
	float ltcspec = ltcEvaluate(n, v, dotNV, p, invM, lightArea0, lightArea1, lightArea2, lightArea3);
	ltcspec *= textureLod(sltcMag, tuv, 0.0).a;
	float ltcdiff = ltcEvaluate(n, v, dotNV, p, mat3(1.0), lightArea0, lightArea1, lightArea2, lightArea3);
	vec3 direct = albedo * ltcdiff + ltcspec * 0.05;

	direct *= attenuate(distance(p, lp));
	direct *= lightCol;
	direct *= clamp(dotNL + 2.0 * occ * occ - 1.0, 0.0, 1.0); // Micro shadowing

	#ifdef _Voxel
	direct *= 1.0 - traceShadow(voxels, voxpos, l);
	#endif

	return direct;
}

#endif
