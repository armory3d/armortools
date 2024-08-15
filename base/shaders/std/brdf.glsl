
#ifndef _BRDF_GLSL_
#define _BRDF_GLSL_

vec3 surface_albedo(const vec3 base_color, const float metalness) {
	return mix(base_color, vec3(0.0), metalness);
}

vec3 surface_f0(const vec3 base_color, const float metalness) {
	return mix(vec3(0.04), base_color, metalness);
}

float mip_from_roughness(const float roughness, const float num_mipmaps) {
	// First mipmap level = roughness 0, last = roughness = 1
	return roughness * num_mipmaps;
}

#endif
