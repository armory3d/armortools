#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

#include "../std/brdf.glsl"
#include "../std/math.glsl"
#ifdef _ShadowMap
#include "../std/shadows.glsl"
#endif
#ifdef _VoxelAOvar
#include "../std/conetrace.glsl"
#endif
#ifdef _LTC
#include "../std/ltc.glsl"
#endif
#ifdef _LightIES
#include "../std/ies.glsl"
#endif

#ifdef _ShadowMap
#ifdef _SinglePoint
	#ifdef _Spot
	uniform sampler2DShadow shadowMapSpot[1];
	uniform mat4 LWVPSpot0;
	#else
	uniform samplerCubeShadow shadowMapPoint[1];
	uniform vec2 lightProj;
	#endif
#endif
#ifdef _Clusters
	uniform samplerCubeShadow shadowMapPoint[4];
	uniform vec2 lightProj;
	#ifdef _Spot
	uniform sampler2DShadow shadowMapSpot[4];
	uniform mat4 LWVPSpot0;
	uniform mat4 LWVPSpot1;
	uniform mat4 LWVPSpot2;
	uniform mat4 LWVPSpot3;
	#endif
#endif
#endif

#ifdef _LTC
uniform vec3 lightArea0;
uniform vec3 lightArea1;
uniform vec3 lightArea2;
uniform vec3 lightArea3;
uniform sampler2D sltcMat;
uniform sampler2D sltcMag;
#ifdef _ShadowMap
	#ifndef _Spot
	#ifdef _SinglePoint
	uniform sampler2DShadow shadowMapSpot[1];
	uniform mat4 LWVPSpot0;
	#endif
	#ifdef _Clusters
	uniform sampler2DShadow shadowMapSpot[4];
	uniform mat4 LWVPSpot0;
	uniform mat4 LWVPSpot1;
	uniform mat4 LWVPSpot2;
	uniform mat4 LWVPSpot3;
	#endif
	#endif
#endif
#endif

vec3 sampleLight(const vec3 p, const vec3 n, const vec3 v, const float dotNV, const vec3 lp, const vec3 lightCol,
	const vec3 albedo, const float rough, const float spec, const vec3 f0
	#ifdef _ShadowMap
		, int index, float bias
	#endif
	#ifdef _Spot
		, bool isSpot, float spotA, float spotB, vec3 spotDir
	#endif
	#ifdef _VoxelAOvar
	#ifdef _VoxelShadow
		, sampler3D voxels, vec3 voxpos
	#endif
	#endif
	#ifdef _MicroShadowing
		, float occ
	#endif
	) {
	vec3 ld = lp - p;
	vec3 l = normalize(ld);
	vec3 h = normalize(v + l);
	float dotNH = dot(n, h);
	float dotVH = dot(v, h);
	float dotNL = dot(n, l);

	#ifdef _LTC
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
	vec3 direct = albedo * ltcdiff + ltcspec * spec * 0.05;
	#else
	vec3 direct = lambertDiffuseBRDF(albedo, dotNL) +
				  specularBRDF(f0, rough, dotNL, dotNH, dotNV, dotVH) * spec;
	#endif
	direct *= attenuate(distance(p, lp));
	direct *= lightCol;

	#ifdef _MicroShadowing
	direct *= dotNL + 2.0 * occ * occ - 1.0;
	#endif

	#ifdef _VoxelAOvar
	#ifdef _VoxelShadow
	direct *= 1.0 - traceShadow(voxels, voxpos, l);
	#endif
	#endif

	#ifdef _LTC
	#ifdef _ShadowMap
		#ifdef _SinglePoint
		vec4 lPos = LWVPSpot0 * vec4(p + n * bias * 10, 1.0);
		direct *= shadowTest(shadowMapSpot[0], lPos.xyz / lPos.w, bias);
		#endif
		#ifdef _Clusters
		if (index == 0) {
			vec4 lPos = LWVPSpot0 * vec4(p + n * bias * 10, 1.0);
			direct *= shadowTest(shadowMapSpot[0], lPos.xyz / lPos.w, bias);
		}
		else if (index == 1) {
			vec4 lPos = LWVPSpot1 * vec4(p + n * bias * 10, 1.0);
			direct *= shadowTest(shadowMapSpot[1], lPos.xyz / lPos.w, bias);
		}
		else if (index == 2) {
			vec4 lPos = LWVPSpot2 * vec4(p + n * bias * 10, 1.0);
			direct *= shadowTest(shadowMapSpot[2], lPos.xyz / lPos.w, bias);
		}
		else if (index == 3) {
			vec4 lPos = LWVPSpot3 * vec4(p + n * bias * 10, 1.0);
			direct *= shadowTest(shadowMapSpot[3], lPos.xyz / lPos.w, bias);
		}
		#endif
	#endif
	return direct;
	#endif

	#ifdef _Spot
	if (isSpot) {
		float spotEffect = dot(spotDir, l); // lightDir
		// x - cutoff, y - cutoff - exponent
		if (spotEffect < spotA) {
			direct *= smoothstep(spotB, spotA, spotEffect);
		}
		#ifdef _ShadowMap
			#ifdef _SinglePoint
			vec4 lPos = LWVPSpot0 * vec4(p + n * bias * 10, 1.0);
			direct *= shadowTest(shadowMapSpot[0], lPos.xyz / lPos.w, bias);
			#endif
			#ifdef _Clusters
			if (index == 0) {
				vec4 lPos = LWVPSpot0 * vec4(p + n * bias * 10, 1.0);
				direct *= shadowTest(shadowMapSpot[0], lPos.xyz / lPos.w, bias);
			}
			else if (index == 1) {
				vec4 lPos = LWVPSpot1 * vec4(p + n * bias * 10, 1.0);
				direct *= shadowTest(shadowMapSpot[1], lPos.xyz / lPos.w, bias);
			}
			else if (index == 2) {
				vec4 lPos = LWVPSpot2 * vec4(p + n * bias * 10, 1.0);
				direct *= shadowTest(shadowMapSpot[2], lPos.xyz / lPos.w, bias);
			}
			else if (index == 3) {
				vec4 lPos = LWVPSpot3 * vec4(p + n * bias * 10, 1.0);
				direct *= shadowTest(shadowMapSpot[3], lPos.xyz / lPos.w, bias);
			}
			#endif
		#endif
		return direct;
	}
	#endif

	#ifdef _LightIES
	direct *= iesAttenuation(-l);
	#endif

	#ifdef _ShadowMap
		#ifdef _SinglePoint
		#ifndef _Spot
		direct *= PCFCube(shadowMapPoint[0], ld, -l, bias, lightProj, n);
		#endif
		#endif
		#ifdef _Clusters
		if (index == 0) direct *= PCFCube(shadowMapPoint[0], ld, -l, bias, lightProj, n);
		else if (index == 1) direct *= PCFCube(shadowMapPoint[1], ld, -l, bias, lightProj, n);
		else if (index == 2) direct *= PCFCube(shadowMapPoint[2], ld, -l, bias, lightProj, n);
		else if (index == 3) direct *= PCFCube(shadowMapPoint[3], ld, -l, bias, lightProj, n);
		#endif
	#endif

	return direct;
}

#endif
