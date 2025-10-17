
#ifdef _FULL
#define _EMISSION
#define _SUBSURFACE
#define _TRANSLUCENCY
#define _TRANSPARENCY
#define _ROULETTE
// #define _FRESNEL
#endif
// #define _RENDER

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
	float4 params; // envstr, envangle, uvscale, empty
};

struct RayPayload {
	float4 color; // rgb, frame
	float3 ray_origin;
	float3 ray_dir;
};

RWTexture2D<half4> render_target : register(u0);
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
#ifdef _TRANSLUCENCY
static const int DEPTH = 6;
#else
static const int DEPTH = 3; // Opaque hits
#endif
static uint seed = 0;
#ifdef _TRANSPARENCY
static const int DEPTH_TRANSPARENT = 16; // Transparent hits
#endif
#ifdef _ROULETTE
static const int rr_start = 2;
static const float rr_probability = 0.5; // Map to albedo
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
		ray.TMax = 100.0;
		generate_camera_ray(screen_pos, ray.Origin, ray.Direction, constant_buffer.eye.xyz, constant_buffer.inv_vp);

		RayPayload payload;
		payload.color = float4(1, 1, 1, j);

		#ifdef _TRANSPARENCY
		int transparent_hits = 0;
		#endif

		for (int i = 0; i < DEPTH; ++i) {

			#ifdef _ROULETTE
			float rr_factor = 1.0;
			if (i >= rr_start) {
				float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
				if (f <= rr_probability) {
					break;
				}
				rr_factor = 1.0 / (1.0 - rr_probability);
			}
			#endif

			// #ifdef _SUBSURFACE
			// TraceRay(scene, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
			// #else
			TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
			// #endif

			#ifdef _EMISSION
			if (payload.color.a == -3) {
				accum += payload.color.rgb;
				break;
			}
			#endif

			#ifdef _TRANSPARENCY
			if (payload.color.a == -2 && transparent_hits < DEPTH_TRANSPARENT) {
				payload.color.a = j;
				transparent_hits++;
				i--;
			}
			#endif

			// Miss
			if (payload.color.a < 0) {
				if (i == 0 && constant_buffer.params.x < 0) { // No envmap
					payload.color.rgb = float3(0.032, 0.032, 0.032);
				}

				accum += clamp(payload.color.rgb, 0.0, 8.0);
				break;
			}

			#ifdef _ROULETTE
			payload.color.rgb *= rr_factor;
			#endif

			ray.Origin = payload.ray_origin;
			ray.Direction = payload.ray_dir;
		}
	}

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);
	accum = accum / SAMPLES;

	#ifdef _RENDER

	float a = 1.0 / (constant_buffer.eye.w + 1);
	float b = 1.0 - a;
	color = color * b + accum * a;
	render_target[DispatchRaysIndex().xy] = half4(color, 1.0f);

	#else

	if (constant_buffer.eye.w == 0) {
		color = accum;
	}
	render_target[DispatchRaysIndex().xy] = half4(lerp(color, accum, 1.0 / 4.0), 1.0f);

	#endif
}

[shader("closesthit")]
void closesthit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
	const uint triangle_index_stride = 12; // 3 * 4
	uint base_index = PrimitiveIndex() * triangle_index_stride;

	#ifdef _FORGE
	base_index += InstanceID(); // Offset to index buffer of this instance
	#endif

	uint3 indices_sample = indices.Load3(base_index);

	float2 vertex_uvs[3] = {
		s16_to_f32(vertices[indices_sample[0]].tex),
		s16_to_f32(vertices[indices_sample[1]].tex),
		s16_to_f32(vertices[indices_sample[2]].tex)
	};
	float2 tex_coord = hit_attribute2d(vertex_uvs, attr) * constant_buffer.params.z;

	uint2 size;
	mytexture0.GetDimensions(size.x, size.y);
	uint3 utex_coord = uint3((tex_coord - uint2(tex_coord)) * size, 0);
	float4 texpaint0 = mytexture0.Load(utex_coord);

	#ifdef _TRANSPARENCY
	if (texpaint0.a <= 0.01) {
		payload.ray_dir = WorldRayDirection();
		payload.ray_origin = hit_world_position() + payload.ray_dir * 0.0001f;
		payload.color.a = -2;
		return;
	}
	#endif

	float3 vertex_normals[3] = {
		float3(s16_to_f32(vertices[indices_sample[0]].nor), s16_to_f32(vertices[indices_sample[0]].poszw).y),
		float3(s16_to_f32(vertices[indices_sample[1]].nor), s16_to_f32(vertices[indices_sample[1]].poszw).y),
		float3(s16_to_f32(vertices[indices_sample[2]].nor), s16_to_f32(vertices[indices_sample[2]].poszw).y)
	};
	float3 n = normalize(hit_attribute(vertex_normals, attr));

	#ifdef _FORGE
	n = mul((float3x3)(WorldToObject4x3()), n);
	n = normalize(n);
	#endif

	mytexture1.GetDimensions(size.x, size.y);
	utex_coord = uint3((tex_coord - uint2(tex_coord)) * size, 0);
	float4 texpaint1 = mytexture1.Load(utex_coord);

	mytexture2.GetDimensions(size.x, size.y);
	utex_coord = uint3((tex_coord - uint2(tex_coord)) * size, 0);
	float4 texpaint2 = mytexture2.Load(utex_coord);

	float3 texcolor = pow(texpaint0.rgb, float3(2.2, 2.2, 2.2));

	float3 tangent = float3(0, 0, 0);
	float3 binormal = float3(0, 0, 0);
	create_basis(n, tangent, binormal);

	texpaint1.rgb = normalize(texpaint1.rgb * 2.0 - 1.0);
	texpaint1.g = -texpaint1.g;
	n = mul(texpaint1.rgb, float3x3(tangent, binormal, n));

	float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
	seed += 1;

	#ifdef _TRANSLUCENCY
	float3 _payload_color = payload.color.xyz;
	float3 diffuse_dir = texpaint0.a < f ?
		WorldRayDirection() : cos_weighted_hemisphere_direction(n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
	#else
	float3 diffuse_dir = cos_weighted_hemisphere_direction(n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
	#endif

	#ifdef _FRESNEL
	float specular_chance = fresnel(n, WorldRayDirection());
	#else
	const float specular_chance = 0.5;
	#endif

	if (f < specular_chance) {
		#ifdef _TRANSLUCENCY
		float3 specular_dir = texpaint0.a < f * 2.0 ? WorldRayDirection() : reflect(WorldRayDirection(), n);
		#else
		float3 specular_dir = reflect(WorldRayDirection(), n);
		#endif
		payload.ray_dir = lerp(specular_dir, diffuse_dir, texpaint2.g * texpaint2.g);

		float3 v = normalize(constant_buffer.eye.xyz - hit_world_position());
		float dotnv = max(dot(n, v), 0.0);
		float3 specular = surface_specular(texcolor, texpaint2.b);
		payload.color.xyz *= env_brdf_approx(specular, texpaint2.g, dotnv);
		#ifdef _FRESNEL
		payload.color.xyz /= specular_chance;
		#endif
	}
	else {
		payload.ray_dir = diffuse_dir;
		payload.color.xyz *= surface_albedo(texcolor, texpaint2.b);
		#ifdef _FRESNEL
		payload.color.xyz /= 1.0 - specular_chance;
		#endif
	}
	#ifdef _FRESNEL
	payload.color.xyz *= 0.5;
	#endif

	#ifdef _TRANSLUCENCY
	payload.color.xyz = lerp(_payload_color, payload.color.xyz, texpaint0.a);
	#endif

	payload.ray_origin = hit_world_position() + payload.ray_dir * 0.0001f;

	#ifdef _EMISSION
	if (int(texpaint1.a * 255.0f) % 3 == 1) { // matid
		payload.color.xyz *= 100.0f;
		payload.color.a = -3.0;
	}
	#endif

	#ifdef _SUBSURFACE
	if (int(texpaint1.a * 255.0f) % 3 == 2) {
		// Thickness
		float d = min(1.0 / min(RayTCurrent() * 2.0, 1.0) / 10.0, 0.5);
		payload.color.xyz += payload.color.xyz * d;
		// Fake scatter
		if (f < 0.5) {
			payload.ray_origin += WorldRayDirection() * f * 0.001;
		}
	}
	#endif
}

[shader("miss")]
void miss(inout RayPayload payload) {

	#ifdef _EMISSION
	if (payload.color.a == -3.0) {
		return;
	}
	#endif

	float2 tex_coord = frac(equirect(WorldRayDirection(), constant_buffer.params.y));
	uint2 size;
	mytexture_env.GetDimensions(size.x, size.y);
	uint2 itex = tex_coord * size;

	#ifdef _FULL
	// Use .Sample() instead..
	itex = clamp(itex, uint2(0, 0), size - uint2(2, 2));
	float2 f = frac(tex_coord * size);
	float3 t00 = mytexture_env.Load(int3(itex, 0)).rgb;
	float3 t10 = mytexture_env.Load(int3(itex + uint2(1, 0), 0)).rgb;
	float3 t01 = mytexture_env.Load(int3(itex + uint2(0, 1), 0)).rgb;
	float3 t11 = mytexture_env.Load(int3(itex + uint2(1, 1), 0)).rgb;
	float3 texenv = lerp(lerp(t00, t10, f.x), lerp(t01, t11, f.x), f.y);
	#else
	float3 texenv = mytexture_env.Load(uint3(tex_coord * size, 0)).rgb;
	#endif

	texenv *= abs(constant_buffer.params.x);

	payload.color = float4(payload.color.rgb * texenv.rgb, -1);
}
