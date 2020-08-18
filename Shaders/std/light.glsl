#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

#include "../std/brdf.glsl"
#include "../std/math.glsl"
#ifdef _VoxelAOvar
#include "../std/conetrace.glsl"
#endif
#include "../std/ltc.glsl"

uniform vec3 lightArea0;
uniform vec3 lightArea1;
uniform vec3 lightArea2;
uniform vec3 lightArea3;
uniform sampler2D sltcMat;
uniform sampler2D sltcMag;

vec3 sampleLight(const vec3 p, const vec3 n, const vec3 v, const float dotNV, const vec3 lp, const vec3 lightCol,
	const vec3 albedo, const float rough, const vec3 f0
	#ifdef _VoxelAOvar
	#ifdef _VoxelShadow
		, sampler3D voxels, vec3 voxpos
	#endif
	#endif
		, float occ
	) {
	vec3 ld = lp - p;
	vec3 l = normalize(ld);
	vec3 h = normalize(v + l);
	float dotNH = dot(n, h);
	float dotVH = dot(v, h);
	float dotNL = dot(n, l);

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

	direct *= dotNL + 2.0 * occ * occ - 1.0;

	#ifdef _VoxelAOvar
	#ifdef _VoxelShadow
	direct *= 1.0 - traceShadow(voxels, voxpos, l);
	#endif
	#endif

	return direct;
}

#endif
