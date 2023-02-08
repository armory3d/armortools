
#include "std/rand.hlsl"
#include "std/math.hlsl"

struct Vertex {
	uint posxy;
	uint poszw;
	uint nor;
	uint tex;
};

struct RayGenConstantBuffer {
	float4 v0; // frame, strength, radius, offset
	float4 v1;
	float4 v2;
	float4 v3;
	float4 v4;
};

struct RayPayload {
	float4 color;
	float3 ray_origin;
	float3 ray_dir;
};

RWTexture2D<float4> render_target : register(u0);
RaytracingAccelerationStructure scene : register(t0);
ByteAddressBuffer indices : register(t1);
StructuredBuffer<Vertex> vertices : register(t2);
ConstantBuffer<RayGenConstantBuffer> constant_buffer : register(b0);

Texture2D<float4> mytexture0 : register(t3);
Texture2D<float4> mytexture1 : register(t4);
Texture2D<float4> mytexture2 : register(t5);
Texture2D<float4> mytexture_env : register(t6);
Texture2D<float4> mytexture_sobol : register(t7);
Texture2D<float4> mytexture_scramble : register(t8);
Texture2D<float4> mytexture_rank : register(t9);

static const int SAMPLES = 64;
static uint seed = 0;

[shader("raygeneration")]
void raygeneration() {
	float2 xy = DispatchRaysIndex().xy + 0.5f;
	float4 tex0 = mytexture0.Load(uint3(xy, 0));
	if (tex0.a == 0.0) {
		render_target[DispatchRaysIndex().xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}
	float3 pos = tex0.rgb;
	float3 nor = mytexture1.Load(uint3(xy, 0)).rgb;

	RayPayload payload;

	RayDesc ray;
	ray.TMin = constant_buffer.v0.w * 0.01;
	ray.TMax = constant_buffer.v0.z * 10.0;
	ray.Origin = pos;
	payload.ray_origin = ray.Origin;

	float3 accum = float3(0, 0, 0);

	for (int i = 0; i < SAMPLES; ++i) {
		ray.Direction = cos_weighted_hemisphere_direction(-nor, i, seed, constant_buffer.v0.x, mytexture_sobol, mytexture_scramble, mytexture_rank);
		seed += 1;
		TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
		accum += payload.color.rgb;
	}

	accum /= SAMPLES;

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);
	if (constant_buffer.v0.x == 0) {
		color = accum.xyz;
	}
	else {
		float a = 1.0 / constant_buffer.v0.x;
		float b = 1.0 - a;
		color = color * b + accum.xyz * a;
	}
	render_target[DispatchRaysIndex().xy] = float4(color.xyz, 1.0f);
}

[shader("closesthit")]
void closesthit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
	float dist = RayTCurrent() * 2.0;
	payload.color = float4(dist, dist, dist, 1);
}

[shader("miss")]
void miss(inout RayPayload payload) {
	payload.color = float4(0, 0, 0, 1);
}
