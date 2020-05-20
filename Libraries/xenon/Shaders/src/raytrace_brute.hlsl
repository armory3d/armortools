
#include "std/rand.hlsl"
#include "std/attrib.hlsl"
#include "std/math.hlsl"

struct Vertex {
	uint posxy;
	uint poszw;
	uint nor;
	uint tex;
};

struct RayGenConstantBuffer {
	float4 eye; // xyz, frame
	float4x4 inv_vp;
	float4 params; // envstr, envangle
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

static const int SAMPLES = 64;
static const int DEPTH = 3; // Opaque hits
static uint seed = 0;
#ifdef _TRANSPARENCY
static const int DEPTH_TRANSPARENT = 16; // Transparent hits
#endif
#ifdef _ROULETTE
static const int rrStart = 2;
static const float rrProbability = 0.5; // Map to albedo
#endif

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
		ray.TMin = 0.0001;
		ray.TMax = 10.0;
		generate_camera_ray(screen_pos, ray.Origin, ray.Direction, constant_buffer.eye.xyz, constant_buffer.inv_vp);

		RayPayload payload;
		payload.color = float4(1, 1, 1, j);

		#ifdef _TRANSPARENCY
		int transparentHits = 0;
		#endif

		for (int i = 0; i < DEPTH; ++i) {

			#ifdef _ROULETTE
			float rrFactor = 1.0;
			if (i >= rrStart) {
				float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
				if (f <= rrProbability) {
					break;
				}
				rrFactor = 1.0 / (1.0 - rrProbability);
			}
			#endif

			#ifdef _SUBSURFACE
			TraceRay(scene, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
			#else
			TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
			#endif

			// Miss
			if (payload.color.a < 0) {
				#ifdef _TRANSPARENCY
				if (payload.color.a == -2 && transparentHits < DEPTH_TRANSPARENT) {
					payload.color.a = j;
					transparentHits++;
					i--;
				}
				// else {
				#endif

				if (i == 0 && constant_buffer.params.x < 0) { // No envmap
					payload.color.rgb = float3(0.05, 0.05, 0.05);
				}

				accum += payload.color.rgb;
				break;
			}

			#ifdef _ROULETTE
			payload.color.rgb *= rrFactor;
			#endif

			ray.Origin = payload.ray_origin;
			ray.Direction = payload.ray_dir;
		}
	}

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);

	#ifdef _RENDER
	float a = 1.0 / (constant_buffer.eye.w + 1);
	float b = 1.0 - a;
	color = color * b + (accum.xyz / SAMPLES) * a;
	render_target[DispatchRaysIndex().xy] = float4(color.xyz, 0.0f);
	#else
	if (constant_buffer.eye.w == 0) {
		color = accum.xyz / SAMPLES;
	}
	render_target[DispatchRaysIndex().xy] = float4(lerp(color.xyz, accum.xyz / SAMPLES, 1.0 / 4.0), 0.0f);
	#endif
}

[shader("closesthit")]
void closesthit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
	const uint triangleIndexStride = 12; // 3 * 4
	uint base_index = PrimitiveIndex() * triangleIndexStride;
	uint3 indices_sample = indices.Load3(base_index);

	float2 vertex_uvs[3] = {
		S16toF32(vertices[indices_sample[0]].tex),
		S16toF32(vertices[indices_sample[1]].tex),
		S16toF32(vertices[indices_sample[2]].tex)
	};
	float2 tex_coord = hit_attribute2d(vertex_uvs, attr);

	uint2 size;
	mytexture0.GetDimensions(size.x, size.y);
	float4 texpaint0 = mytexture0.Load(uint3(tex_coord * size, 0));

	#ifdef _TRANSPARENCY
	if (texpaint0.a <= 0.1) {
		payload.ray_dir = WorldRayDirection();
		payload.ray_origin = hit_world_position() + payload.ray_dir * 0.0001f;
		payload.color.a = -2;
		return;
	}
	#endif

	float3 vertex_normals[3] = {
		float3(S16toF32(vertices[indices_sample[0]].nor), S16toF32(vertices[indices_sample[0]].poszw).y),
		float3(S16toF32(vertices[indices_sample[1]].nor), S16toF32(vertices[indices_sample[1]].poszw).y),
		float3(S16toF32(vertices[indices_sample[2]].nor), S16toF32(vertices[indices_sample[2]].poszw).y)
	};
	float3 n = normalize(hit_attribute(vertex_normals, attr));

	float4 texpaint1 = mytexture1.Load(uint3(tex_coord * size, 0));
	float4 texpaint2 = mytexture2.Load(uint3(tex_coord * size, 0));
	float3 color = payload.color.rgb * pow(texpaint0.rgb, float3(2.2, 2.2, 2.2));

	float3 tangent = float3(0, 0, 0);
	float3 binormal = float3(0, 0, 0);
	create_basis(n, tangent, binormal);

	texpaint1.rgb = normalize(texpaint1.rgb * 2.0 - 1.0);
	texpaint1.g = -texpaint1.g;
	n = mul(texpaint1.rgb, float3x3(tangent, binormal, n));

	float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
	if (f > 0.5) {
		payload.ray_dir = lerp(reflect(WorldRayDirection(), n), cos_weighted_hemisphere_direction(n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank), pow(texpaint2.g, 1.2));
	}
	else {
		payload.ray_dir = cos_weighted_hemisphere_direction(n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
		color = color * (1.0 - texpaint2.b);
	}

	// float dotNL = dot(n, -WorldRayDirection());
	// float3 wi = payload.ray_dir.x * tangent + payload.ray_dir.y * binormal + payload.ray_dir.z * n;
	// float dotNV = dot(n, wi);

	payload.ray_origin = hit_world_position() + payload.ray_dir * 0.0001f;
	payload.color.xyz = color.xyz;

	#ifdef _EMISSION
	if (texpaint1.a == 1.0) { // matid
		payload.color.xyz *= 100.0f;
		payload.color.a = -1.0;
	}
	#endif

	#ifdef _SUBSURFACE
	if (texpaint1.a == (254.0f / 255.0f)) {
		payload.ray_origin += WorldRayDirection() * f;
	}
	#endif
}

[shader("miss")]
void miss(inout RayPayload payload) {

	#ifdef _EMISSION
	if (payload.color.a == -1.0) {
		return;
	}
	#endif

	float2 tex_coord = equirect(WorldRayDirection(), constant_buffer.params.y);
	uint2 size;
	mytexture_env.GetDimensions(size.x, size.y);
	float3 texenv = mytexture_env.Load(uint3(tex_coord * size, 0)).rgb * abs(constant_buffer.params.x);
	payload.color = float4(payload.color.rgb * texenv.rgb, -1);
}
