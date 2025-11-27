
#ifndef _MATH_HLSL_
#define _MATH_HLSL_

#include "rand.hlsl"

void create_basis(float3 normal, out float3 tangent, out float3 binormal) {
	float3 v = cross(normal, float3(0.0, 0.0, 1.0));
	if (dot(v, v) > 0.0001) {
		tangent = normalize(v);
	}
	else {
		v = cross(normal, float3(0.0, 1.0, 0.0));
		tangent = normalize(v);
	}
	binormal = cross(tangent, normal);
}

void generate_camera_ray(float2 screen_pos, out float3 ray_origin, out float3 ray_dir, float3 eye, float4x4 inv_vp) {
	screen_pos.y = -screen_pos.y;
	float4 world = mul(float4(screen_pos, 0, 1), inv_vp);
	world.xyz /= world.w;
	ray_origin = eye;
	ray_dir = normalize(world.xyz - ray_origin);
}

float2 equirect(float3 normal, float angle) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float phi = acos(normal.z);
	float theta = atan2(-normal.y, normal.x) + PI + angle;
	return float2(theta / PI2, phi / PI);
}

float3 cos_weighted_hemisphere_direction(float3 n, uint sample, uint seed, int frame, Texture2D<float4> sobol, Texture2D<float4> scramble, Texture2D<float4> rank) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float f0 = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed, frame, sobol, scramble, rank);
	float f1 = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed + 1, frame, sobol, scramble, rank);
	float z = f0 * 2.0f - 1.0f;
	float a = f1 * PI2;
	float r = sqrt(1.0f - z * z);
	float x = r * cos(a);
	float y = r * sin(a);
	return normalize(n + float3(x, y, z));

	// float xi1 = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed, frame, sobol, scramble, rank);
	// float xi2 = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed + 1, frame, sobol, scramble, rank);
	// float3 tangent, binormal;
	// create_basis(n, tangent, binormal);
	// float sin_theta = sqrt(xi1);
	// float cos_theta = sqrt(1.0 - xi1);
	// float phi = xi2 * PI2;
	// float3 dir = cos(phi) * sin_theta * tangent + sin(phi) * sin_theta * binormal + cos_theta * n;
	// return dir;
}

float3 surface_albedo(const float3 base_color, const float metalness) {
	return lerp(base_color, float3(0.0, 0.0, 0.0), metalness);
}

float3 surface_specular(const float3 base_color, const float metalness) {
	return lerp(float3(0.04, 0.04, 0.04), base_color, metalness);
}

float fresnel(float3 normal, float3 incident) {
	return lerp(0.5, 1.0, pow(1.0 + dot(normal, incident), 5.0));
}

#endif
