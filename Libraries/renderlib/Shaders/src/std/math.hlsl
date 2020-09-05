
#ifndef _MATH_HLSL_
#define _MATH_HLSL_

#include "rand.hlsl"

void create_basis(float3 normal, out float3 tangent, out float3 binormal) {
	// tangent = abs(normal.x) > abs(normal.y) ?
	// 	normalize(float3(0.0, normal.z, -normal.y)) :
	// 	normalize(float3(-normal.z, 0.0, normal.x));
	// binormal = cross(normal, tangent);
	float3 v1 = cross(normal, float3(0.0, 0.0, 1.0));
	float3 v2 = cross(normal, float3(0.0, 1.0, 0.0));
	tangent = length(v1) > length(v2) ? v1 : v2;
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
}

float3 surfaceAlbedo(const float3 baseColor, const float metalness) {
	return lerp(baseColor, float3(0.0, 0.0, 0.0), metalness);
}

float3 surfaceSpecular(const float3 baseColor, const float metalness) {
	return lerp(float3(0.04, 0.04, 0.04), baseColor, metalness);
}

// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
float3 envBRDFApprox(float3 specular, float roughness, float dotNV) {
	const float4 c0 = float4(-1, -0.0275, -0.572, 0.022);
	const float4 c1 = float4(1, 0.0425, 1.04, -0.04);
	float4 r = roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * dotNV)) * r.x + r.y;
	float2 ab = float2(-1.04, 1.04) * a004 + r.zw;
	return specular * ab.x + ab.y;
}

#endif
