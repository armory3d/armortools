
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

static uint seed = 0;
static const float PI = 3.1415926535f;
static const int SAMPLES = 64;
static const int DEPTH = 3; // 3 - 5

static const int rrStart = 2;
static const float rrProbability = 0.5; // map to albedo

// A Low-Discrepancy Sampler that Distributes Monte Carlo Errors as a Blue Noise in Screen Space
// Eric Heitz, Laurent Belcour, Victor Ostromoukhov, David Coeurjolly and Jean-Claude Iehl
// https://eheitzresearch.wordpress.com/762-2/
float rand(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension) {
	// wrap arguments
	pixel_i += constant_buffer.eye.w * 9;
	pixel_j += constant_buffer.eye.w * 11;
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	// xor index based on optimized ranking
	int i = sampleDimension + (pixel_i + pixel_j*128)*8;
	int rankedSampleIndex = sampleIndex ^ int(mytexture_rank.Load(uint3(i % 128, uint(i / 128), 0)).r * 255);

	// fetch value in sequence
	i = sampleDimension + rankedSampleIndex*256;
	int value = int(mytexture_sobol.Load(uint3(i % 256, uint(i / 256), 0)).r * 255);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	i = (sampleDimension%8) + (pixel_i + pixel_j*128)*8;
	value = value ^ int(mytexture_scramble.Load(uint3(i % 128, uint(i / 128), 0)).r * 255);

	// convert to float and return
	float v = (0.5f+value)/256.0f;
	return v;
}

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

void generate_camera_ray(float2 screen_pos, out float3 ray_origin, out float3 ray_dir) {
	screen_pos.y = -screen_pos.y;
	float4 world = mul(float4(screen_pos, 0, 1), constant_buffer.inv_vp);
	world.xyz /= world.w;
	ray_origin = constant_buffer.eye.xyz;
	ray_dir = normalize(world.xyz - ray_origin);
}

[shader("raygeneration")]
void raygeneration() {
	float3 accum = float3(0, 0, 0);
	for (int j = 0; j < SAMPLES; ++j) {
		// AA
		float2 xy = DispatchRaysIndex().xy + 0.5f;
		xy.x += rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed);
		seed += 1;
		xy.y += rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed);

		float2 screen_pos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

		RayDesc ray;
		ray.TMin = 0.01;
		ray.TMax = 10.0;
		generate_camera_ray(screen_pos, ray.Origin, ray.Direction);

		RayPayload payload;
		payload.color = float4(1, 1, 1, j);

		for (int i = 0; i < DEPTH; ++i) {

			float rrFactor = 1.0;
			// if (i >= rrStart) {
			// 	float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, j, seed);
			// 	if (f <= rrProbability) {
			// 		break;
			// 	}
			// 	rrFactor = 1.0 / (1.0 - rrProbability);
			// }

			TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
			if (payload.color.a < 0) {
				if (i == 0) {
					// return;
					payload.color.rgb = float3(0.05, 0.05, 0.05);
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

float3 hit_world_position() {
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float3 hit_attribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr) {
	return vertexAttribute[0] +
		attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float2 hit_attribute2d(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr) {
	return vertexAttribute[0] +
		attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float3 cos_weighted_hemisphere_direction(float3 n, uint sample) {
	float2 r = float2(
		rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed),
		rand(DispatchRaysIndex().x, DispatchRaysIndex().y, sample, seed + 1)
	);
	seed += 1;
	float3 uu = normalize(cross(n, float3(0.0, 1.0, 1.0)));
	float3 vv = cross(uu, n);
	float ra = sqrt(r.y);
	float rx = ra * cos(6.2831 * r.x);
	float ry = ra * sin(6.2831 * r.x);
	float rz = sqrt(1.0 - r.y);
	float3 rr = float3(rx * uu + ry * vv + rz * n);
	return normalize(rr);
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
		payload.ray_dir = lerp(reflect(WorldRayDirection(), n), cos_weighted_hemisphere_direction(n, payload.color.a), texpaint2.g);
	}
	else {
		float fresnel = schlick_weight(dot(n, WorldRayDirection())) * (1.0 - texpaint2.g);
		float f = rand(DispatchRaysIndex().x, DispatchRaysIndex().y, payload.color.a, seed);
		if (f > fresnel) {
			float3 wo = -WorldRayDirection();
			payload.ray_dir = cos_weighted_hemisphere_direction(n, payload.color.a);
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

float2 equirect(float3 normal) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float phi = acos(normal.z);
	float theta = atan2(-normal.y, normal.x) + PI;
	return float2(theta / PI2, phi / PI);
}

[shader("miss")]
void miss(inout RayPayload payload) {
	float2 tex_coord = equirect(WorldRayDirection());
	float3 texenv = mytexture_env.Load(uint3(tex_coord.x * 1024, tex_coord.y * 512, 0)).rgb * 3;
	payload.color = float4(payload.color.rgb * texenv.rgb, -1);
}
