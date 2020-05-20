#version 450

#define _EnvStr
#define _EnvSky
#define _EnvClouds

const float cloudsLower = 1.0;
const float cloudsUpper = 1.0;
const vec2 cloudsWind = vec2(1.0, 0.0);
const float cloudsPrecipitation = 1.0;
const float cloudsSecondary = 1.0;
const int cloudsSteps = 24;

#ifdef _EnvTex
#include "../std/math.glsl"
#else
const float PI = 3.1415926535;
#endif

#ifdef _EnvCol
	uniform vec3 backgroundCol;
#endif
#ifdef _EnvSky
	uniform vec3 A;
	uniform vec3 B;
	uniform vec3 C;
	uniform vec3 D;
	uniform vec3 E;
	uniform vec3 F;
	uniform vec3 G;
	uniform vec3 H;
	uniform vec3 I;
	uniform vec3 Z;
	uniform vec3 hosekSunDirection;
#endif
#ifdef _EnvClouds
	uniform sampler3D scloudsBase;
	uniform sampler3D scloudsDetail;
	uniform sampler2D scloudsMap;
	uniform float time;
#endif
#ifdef _EnvTex
	uniform sampler2D envmap;
#endif
#ifdef _EnvImg // Static background
	uniform vec2 screenSize;
	uniform sampler2D envmap;
#endif

#ifdef _EnvStr
uniform float envmapStrength;
#endif

in vec3 normal;
out vec4 fragColor;

#ifdef _EnvSky
vec3 hosekWilkie(float cos_theta, float gamma, float cos_gamma) {
	vec3 chi = (1 + cos_gamma * cos_gamma) / pow(1 + H * H - 2 * cos_gamma * H, vec3(1.5));
	return (1 + A * exp(B / (cos_theta + 0.01))) * (C + D * exp(E * gamma) + F * (cos_gamma * cos_gamma) + G * chi + I * sqrt(cos_theta));
}
#endif

#ifdef _EnvClouds
// GPU PRO 7 - Real-time Volumetric Cloudscapes
// https://www.guerrilla-games.com/read/the-real-time-volumetric-cloudscapes-of-horizon-zero-dawn
// https://github.com/sebh/TileableVolumeNoise
float remap(float old_val, float old_min, float old_max, float new_min, float new_max) {
	return new_min + (((old_val - old_min) / (old_max - old_min)) * (new_max - new_min));
}

float getDensityHeightGradientForPoint(float height, float cloud_type) {
	const vec4 stratusGrad = vec4(0.02f, 0.05f, 0.09f, 0.11f);
	const vec4 stratocumulusGrad = vec4(0.02f, 0.2f, 0.48f, 0.625f);
	const vec4 cumulusGrad = vec4(0.01f, 0.0625f, 0.78f, 1.0f);
	float stratus = 1.0f - clamp(cloud_type * 2.0f, 0, 1);
	float stratocumulus = 1.0f - abs(cloud_type - 0.5f) * 2.0f;
	float cumulus = clamp(cloud_type - 0.5f, 0, 1) * 2.0f;
	vec4 cloudGradient = stratusGrad * stratus + stratocumulusGrad * stratocumulus + cumulusGrad * cumulus;
	return smoothstep(cloudGradient.x, cloudGradient.y, height) - smoothstep(cloudGradient.z, cloudGradient.w, height);
}

float sampleCloudDensity(vec3 p) {
	float cloud_base = textureLod(scloudsBase, p, 0).r * 40; // Base noise
	vec3 weather_data = textureLod(scloudsMap, p.xy, 0).rgb; // Weather map
	cloud_base *= getDensityHeightGradientForPoint(p.z, weather_data.b); // Cloud type
	cloud_base = remap(cloud_base, weather_data.r, 1.0, 0.0, 1.0); // Coverage
	cloud_base *= weather_data.r;
	float cloud_detail = textureLod(scloudsDetail, p, 0).r * 2; // Detail noise
	float cloud_detail_mod = mix(cloud_detail, 1.0 - cloud_detail, clamp(p.z * 10.0, 0, 1));
	cloud_base = remap(cloud_base, cloud_detail_mod * 0.2, 1.0, 0.0, 1.0);
	return cloud_base;
}

float cloudRadiance(vec3 p, vec3 dir){
	#ifdef _EnvSky
	vec3 sun_dir = hosekSunDirection;
	#else
	vec3 sun_dir = vec3(0, 0, -1);
	#endif
	const int steps = 8;
	float step_size = 0.5 / float(steps);
	float d = 0.0;
	p += sun_dir * step_size;
	for(int i = 0; i < steps; ++i) {
		d += sampleCloudDensity(p + sun_dir * float(i) * step_size);
	}
	return 1.0 - d;
}

vec3 traceClouds(vec3 sky, vec3 dir) {
	const float step_size = 0.5 / float(cloudsSteps);
	float T = 1.0;
	float C = 0.0;
	vec2 uv = dir.xy / dir.z * 0.4 * cloudsLower + cloudsWind * time * 0.02;

	for (int i = 0; i < cloudsSteps; ++i) {
		float h = float(i) / float(cloudsSteps);
		vec3 p = vec3(uv * 0.04, h);
		float d = sampleCloudDensity(p);

		if (d > 0) {
			// float radiance = cloudRadiance(p, dir);
			C += T * exp(h) * d * step_size * 0.6 * cloudsPrecipitation;
			T *= exp(-d * step_size);
			if (T < 0.01) break;
		}
		uv += (dir.xy / dir.z) * step_size * cloudsUpper;
	}

	return vec3(C) + sky * T;
}
#endif // _EnvClouds

void main() {

#ifdef _EnvCol
	fragColor.rgb = backgroundCol;
#ifdef _EnvTransp
	return;
#endif
#ifdef _EnvClouds
	vec3 n = normalize(normal);
#endif
#endif

#ifndef _EnvSky // Prevent case when sky radiance is enabled
#ifdef _EnvTex
	vec3 n = normalize(normal);
	fragColor.rgb = texture(envmap, envMapEquirect(n, 0.0)).rgb * envmapStrength;
	#ifdef _EnvLDR
	fragColor.rgb = pow(fragColor.rgb, vec3(2.2));
	#endif
#endif
#endif

#ifdef _EnvImg // Static background
	// Will have to get rid of gl_FragCoord, pass tc from VS
	vec2 texco = gl_FragCoord.xy / screenSize;
	fragColor.rgb = texture(envmap, vec2(texco.x, 1.0 - texco.y)).rgb * envmapStrength;
#endif

#ifdef _EnvSky
	vec3 n = normalize(normal);
	float cos_theta = clamp(n.z, 0.0, 1.0);
	float cos_gamma = dot(n, hosekSunDirection);
	float gamma_val = acos(cos_gamma);

	fragColor.rgb = Z * hosekWilkie(cos_theta, gamma_val, cos_gamma) * envmapStrength;
#endif

#ifdef _EnvClouds
	if (n.z > 0.0) fragColor.rgb = mix(fragColor.rgb, traceClouds(fragColor.rgb, n), clamp(n.z * 5.0, 0, 1));
#endif

#ifdef _LDR
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
#endif

	fragColor.a = 0.0; // Mark as non-opaque
}
