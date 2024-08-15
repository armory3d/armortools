
#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

#include "std/brdf.glsl"
#include "std/math.glsl"
#include "std/ltc.glsl"
#ifdef _Voxel
#include "std/conetrace.glsl"
#endif

uniform vec3 light_area0;
uniform vec3 light_area1;
uniform vec3 light_area2;
uniform vec3 light_area3;
uniform sampler2D sltc_mat;
uniform sampler2D sltc_mag;

vec3 sample_light(const vec3 p, const vec3 n, const vec3 v, const float dotnv, const vec3 lp, const vec3 light_col,
	const vec3 albedo, const float rough, const vec3 f0, const float occ
#ifdef _Voxel
	, sampler3D voxels, vec3 voxpos
#endif
	) {
	vec3 ld = lp - p;
	vec3 l = normalize(ld);
	float dotnl = max(0.0, dot(n, l));

	float theta = acos(dotnv);
	vec2 tuv = vec2(rough, theta / (0.5 * PI));
	tuv = tuv * LUT_SCALE + LUT_BIAS;
	vec4 t = textureLod(sltc_mat, tuv, 0.0);
	mat3 inv = mat3(
		vec3(1.0, 0.0, t.y),
		vec3(0.0, t.z, 0.0),
		vec3(t.w, 0.0, t.x));
	float ltcspec = ltc_evaluate(n, v, dotnv, p, inv, light_area0, light_area1, light_area2, light_area3);
	ltcspec *= textureLod(sltc_mag, tuv, 0.0).a;
	float ltcdiff = ltc_evaluate(n, v, dotnv, p, mat3(1.0), light_area0, light_area1, light_area2, light_area3);
	vec3 direct = albedo * ltcdiff + ltcspec * 0.05;

	direct *= attenuate(distance(p, lp));
	direct *= light_col;
	direct *= clamp(dotnl + 2.0 * occ * occ - 1.0, 0.0, 1.0); // Micro shadowing

	#ifdef _Voxel
	direct *= 1.0 - trace_shadow(voxels, voxpos, l);
	#endif

	return direct;
}

#endif
