
#ifndef _BRDF_GLSL_
#define _BRDF_GLSL_

vec3 surfaceAlbedo(const vec3 baseColor, const float metalness) {
	return mix(baseColor, vec3(0.0), metalness);
}

vec3 surfaceF0(const vec3 baseColor, const float metalness) {
	return mix(vec3(0.04), baseColor, metalness);
}

float getMipFromRoughness(const float roughness, const float numMipmaps) {
	// First mipmap level = roughness 0, last = roughness = 1
	return roughness * numMipmaps;
}

#endif
