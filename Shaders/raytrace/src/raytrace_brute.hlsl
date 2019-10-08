
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
	float4 params; // envstr, envshow
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

static uint seed = 0;
static const int SAMPLES = 64;
static const int DEPTH = 3; // 3 - 5

static const int rrStart = 2;
static const float rrProbability = 0.5; // map to albedo

[shader("raygeneration")]
void raygeneration() {
	float3 accum = float3(0, 0, 0);
	for (int j = 0; j < SAMPLES; ++j) {
		// AA
		float2 xy = DispatchRaysIndex().xy + 0.5f;
		xy.x += rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
		seed += 1;
		xy.y += rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);

		float2 screen_pos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

		RayDesc ray;
		ray.TMin = 0.01;
		ray.TMax = 10.0;
		generate_camera_ray(screen_pos, ray.Origin, ray.Direction, constant_buffer.eye.xyz, constant_buffer.inv_vp);

		RayPayload payload;
		payload.color = float4(1, 1, 1, j);

		for (int i = 0; i < DEPTH; ++i) {

			float rrFactor = 1.0;
			// if (i >= rrStart) {
			// 	float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
			// 	if (f <= rrProbability) {
			// 		break;
			// 	}
			// 	rrFactor = 1.0 / (1.0 - rrProbability);
			// }

			TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
			if (payload.color.a < 0) {
				if (i == 0) {
					// return;
					if (constant_buffer.params.y == 0) {
						payload.color.rgb = float3(0.05, 0.05, 0.05);
					}
				}
				break;
			}

			payload.color.rgb *= rrFactor;

			ray.Origin = payload.ray_origin;
			ray.Direction = payload.ray_dir;
		}

		accum += payload.color.rgb;
	}

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);
	// float a = 1.0 / constant_buffer.eye.w;
	// float b = 1.0 - a;
	// color = color * b + (accum.xyz / SAMPLES) * a;
	// render_target[DispatchRaysIndex().xy] = float4(color.xyz, 0.0f);

	if (constant_buffer.eye.w == 0) {
		color = accum.xyz / SAMPLES;
	}

	render_target[DispatchRaysIndex().xy] = float4(lerp(color.xyz, accum.xyz / SAMPLES, 1.0 / 4.0), 0.0f);
	// render_target[DispatchRaysIndex().xy] = float4(accum.xyz / SAMPLES, 0.0f);
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

	if (texpaint2.b >= 0.99) {
		payload.ray_dir = lerp(reflect(WorldRayDirection(), n), cos_weighted_hemisphere_direction(n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank), texpaint2.g);
	}
	else {
		float fresnel = schlick_weight(dot(n, WorldRayDirection())) * (1.0 - texpaint2.g);
		float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
		if (f > fresnel) {
			float3 wo = -WorldRayDirection();
			payload.ray_dir = cos_weighted_hemisphere_direction(n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
			float3 tangent = float3(0, 0, 0);
			float3 binormal = float3(0, 0, 0);
			create_basis(n, tangent, binormal);
			float3 wi = payload.ray_dir.x * tangent + payload.ray_dir.y * binormal + payload.ray_dir.z * n;
			float dotNL = dot(n, wo);
			float dotNV = dot(n, wi);
			if (dotNL < 0.0 || dotNV < 0.0) color = float3(0, 0, 0);
		}
		else {
			payload.ray_dir = reflect(WorldRayDirection(), n);
		}
	}
	payload.ray_origin = hit_world_position() + payload.ray_dir * 0.0001f;
	payload.color.xyz = color.xyz;
}

[shader("miss")]
void miss(inout RayPayload payload) {
	float2 tex_coord = equirect(WorldRayDirection());
	float3 texenv = mytexture_env.Load(uint3(tex_coord.x * 4096, tex_coord.y * 2048, 0)).rgb * constant_buffer.params.x;
	payload.color = float4(payload.color.rgb * texenv.rgb, -1);
}
