
#include "rand.hlsl"
#include "attrib.hlsl"
#include "math.hlsl"

struct Vertex {
	float3 position;
	float3 normal;
	float2 tex;
};

struct RayGenConstantBuffer {
	float4 eye; // xyz, frame
	float4x4 inv_vp;
};

struct RayPayload {
	float4 color; // rgb, frame
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

static uint seed;
static const int DIFFUSE_RAYS = 64;

[shader("raygeneration")]
void raygeneration() {
	float3 accum = float3(0, 0, 0);

	// AA
	float2 xy = DispatchRaysIndex().xy + 0.5f;
	xy.x += rand(DispatchRaysIndex().x, DispatchRaysIndex().y, constant_buffer.eye.w, seed, 0, mytexture_sobol, mytexture_scramble, mytexture_rank);
	seed += 1;
	xy.y += rand(DispatchRaysIndex().x, DispatchRaysIndex().y, constant_buffer.eye.w, seed, 0, mytexture_sobol, mytexture_scramble, mytexture_rank);

	float2 screen_pos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	RayDesc ray;
	ray.TMin = 0.01;
	ray.TMax = 10.0;
	generate_camera_ray(screen_pos, ray.Origin, ray.Direction, constant_buffer.eye.xyz, constant_buffer.inv_vp);

	RayPayload payload;
	payload.color = float4(1, 1, 1, 0);

	TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
	// if (payload.color.a < 0) {
		// if (i == 0) {
			// return;
			// payload.color.rgb = float3(0.05, 0.05, 0.05);
		// }
	// }

	ray.Origin = payload.ray_origin;
	// ray.Direction = payload.ray_dir;

	accum += payload.color.rgb;

	if (payload.color.a >= 0) {
		// DIFFUSE_RAYS
		int rays = 64;//(int)(payload.color.a * DIFFUSE_RAYS);
		float3 accum2 = float3(0, 0, 0);
		float3 norr = payload.ray_dir;
		for (int k = 0; k < rays; ++k) {
			ray.Direction = cos_weighted_hemisphere_direction(norr, k, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
			payload.color = float4(1, 1, 1, (k + 1));
			TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
			accum2 += payload.color.rgb;
		}
		accum += accum2 / rays;
	}

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);
	// float a = 1.0 / constant_buffer.eye.w;
	// float b = 1.0 - a;
	// color = color * b + (accum.xyz) * a;
	// render_target[DispatchRaysIndex().xy] = float4(color.xyz, 0.0f);

	if (constant_buffer.eye.w == 0) {
		color = accum.xyz;
	}

	render_target[DispatchRaysIndex().xy] = float4(lerp(color.xyz, accum.xyz, 1.0 / 4.0), 0.0f);
}

[shader("closesthit")]
void closesthit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
	const uint triangleIndexStride = 12; // 3 * 4
	uint base_index = PrimitiveIndex() * triangleIndexStride;
	uint3 indices_sample = indices.Load3(base_index);

	float3 vertex_normals[3] = { 
		float3(vertices[indices_sample[0]].normal),
		float3(vertices[indices_sample[1]].normal),
		float3(vertices[indices_sample[2]].normal)
	};
	float3 n = normalize(hit_attribute(vertex_normals, attr));

	float2 vertex_uvs[3] = { 
		float2(vertices[indices_sample[0]].tex),
		float2(vertices[indices_sample[1]].tex),
		float2(vertices[indices_sample[2]].tex)
	};
	float2 tex_coord = hit_attribute2d(vertex_uvs, attr);

	float3 texpaint0 = mytexture0.Load(uint3(tex_coord * 2048, 0)).rgb;
	float3 texpaint1 = mytexture1.Load(uint3(tex_coord * 2048, 0)).rgb;
	float3 texpaint2 = mytexture2.Load(uint3(tex_coord * 2048, 0)).rgb;
	float3 color = payload.color.rgb * texpaint0.rgb;

	payload.ray_origin = hit_world_position() + n * 0.0001f;
	payload.color.xyz = color.xyz;

	// DIFFUSE_RAYS
	payload.ray_dir = n;
	payload.color.a = texpaint2.g; // roughness
}

[shader("miss")]
void miss(inout RayPayload payload) {
	float2 tex_coord = equirect(WorldRayDirection());
	float3 texenv = mytexture_env.Load(uint3(tex_coord.x * 1024, tex_coord.y * 512, 0)).rgb * 3;
	payload.color = float4(payload.color.rgb * texenv.rgb, -1);
}
