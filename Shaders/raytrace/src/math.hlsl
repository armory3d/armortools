
#ifndef _MATH_HLSL_
#define _MATH_HLSL_

#include "rand.hlsl"

void create_basis(float3 normal, out float3 tangent, out float3 binormal) {
	tangent = abs(normal.x) > abs(normal.y) ?
		normalize(float3(0., normal.z, -normal.y)) :
		normalize(float3(-normal.z, 0., normal.x));
	binormal = cross(normal, tangent);
}

float schlick_weight(float cosTheta) {
	float m = saturate(1. - cosTheta);
	float m2 = m * m;
	return m2 * m2 * m;
}

void generate_camera_ray(float2 screen_pos, out float3 ray_origin, out float3 ray_dir, float3 eye, float4x4 inv_vp) {
	screen_pos.y = -screen_pos.y;
	float4 world = mul(float4(screen_pos, 0, 1), inv_vp);
	world.xyz /= world.w;
	ray_origin = eye;
	ray_dir = normalize(world.xyz - ray_origin);
}

float2 equirect(float3 normal) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float phi = acos(normal.z);
	float theta = atan2(-normal.y, normal.x) + PI;
	return float2(theta / PI2, phi / PI);
}

float3 cos_weighted_hemisphere_direction(float3 n, uint sample, uint seed, int frame, Texture2D<float4> sobol, Texture2D<float4> scramble, Texture2D<float4> rank) {
	float2 r = float2(
		rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed, frame, sobol, scramble, rank),
		rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed + 1, frame, sobol, scramble, rank)
	);
	float3 uu = normalize(cross(n, float3(0.0, 1.0, 1.0)));
	float3 vv = cross(uu, n);
	float ra = sqrt(r.y);
	float rx = ra * cos(6.2831 * r.x);
	float ry = ra * sin(6.2831 * r.x);
	float rz = sqrt(1.0 - r.y);
	float3 rr = float3(rx * uu + ry * vv + rz * n);
	return normalize(rr);
}

#endif
