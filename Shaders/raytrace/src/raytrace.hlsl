
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

static uint seed;
static const float PI = 3.1415926535f;

void create_basis(float3 normal, out float3 tangent, out float3 binormal) {
	tangent = abs(normal.x) > abs(normal.y) ?
		normalize(float3(0., normal.z, -normal.y)) :
		normalize(float3(-normal.z, 0., normal.x));
	binormal = cross(normal, tangent);
}

uint wang_hash(uint seed) {
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

void compute_seed(uint index, uint iteration, uint depth) {
	seed = uint(wang_hash((1 << 31) | (depth << 22) | iteration) ^ wang_hash(index));
}

float rand() {
	static const float png_01_convert = (1.0f / 4294967296.0f);
	seed ^= uint(seed << 13);
	seed ^= uint(seed >> 17);
	seed ^= uint(seed << 5);
	return float(seed * png_01_convert);
}

float schlick(float cosine, float ri) {
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow(saturate(1 - cosine), 5);
}

float schlick_weight(float cosTheta) {
	float m = clamp(1. - cosTheta, 0., 1.);
	float m2 = m * m;
	return m2 * m2 * m;
}

float2 calculate_concentric_sample_disk(float u, float v) {
	// Maps a (u,v) in [0, 1)^2 to a 2D unit disk centered at (0,0). Based on PBRT.
	float2 u_offset = 2.0f * float2(u, v) - float2(1, 1);
	if (u_offset.x == 0 && u_offset.y == 0) {
		return float2(0.0f, 0.0f);
	}
	float theta, r;
	if (abs(u_offset.x) > abs(u_offset.y)) {
		r = u_offset.x;
		theta = PI / 4 * (u_offset.y / u_offset.x);
	}
	else {
		r = u_offset.y;
		theta = (PI / 2) - (PI / 4 * (u_offset.x / u_offset.y));
	}
	return r * float2(cos(theta), sin(theta));
}

void generate_camera_ray(float2 screen_pos, out float3 ray_origin, out float3 ray_dir) {
	screen_pos.y = -screen_pos.y;
	float4 world = mul(float4(screen_pos, 0, 1), constant_buffer.inv_vp);
	world.xyz /= world.w;
	ray_origin = constant_buffer.eye.xyz;
	ray_dir = normalize(world.xyz - ray_origin);

	// Depth of Field
	// float lens_rad = 0.005f;
	// float focal_dist = 0.4f;
	// float3 plens = float3(lens_rad * calculate_concentric_sample_disk(rand(), rand()), 0.0f);
	// float ft = focal_dist / abs(ray_dir.z);
	// float3 pfocus = ray_dir * ft;
	// ray_origin += plens;
	// ray_dir = normalize(pfocus - plens);
}

[shader("raygeneration")]
void raygeneration() {
	uint2 sample_point = DispatchRaysIndex().xy;
	uint2 sample_dim = DispatchRaysDimensions().xy;
	uint id = sample_point.x + sample_dim.x * sample_point.y;
	const int depth = 5;
	compute_seed(id, constant_buffer.eye.w, depth);

	float2 xy = DispatchRaysIndex().xy + 0.5f;
	xy.x += rand(); // AA
	xy.y += rand();
	float2 screen_pos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	RayPayload payload;
	payload.color = float4(1, 1, 1, -1);
	generate_camera_ray(screen_pos, payload.ray_origin, payload.ray_dir);

	RayDesc ray;
	ray.TMin = 0.01;
	ray.TMax = 10.0;
	ray.Origin = payload.ray_origin;
	ray.Direction = payload.ray_dir;
	
	for (int i = 0; i < depth; ++i) {
		TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
		compute_seed(id, constant_buffer.eye.w, i);

		if (payload.color.a != 0) break;
		ray.Origin = payload.ray_origin;
		ray.Direction = payload.ray_dir;
	}

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);
	if (constant_buffer.eye.w == 1.0) color = float3(0, 0, 0);
	float a = 1.0 / constant_buffer.eye.w;
	float b = 1.0 - a;

	color = color * b + payload.color.xyz * a;
	render_target[DispatchRaysIndex().xy] = float4(color.xyz, 0.0f);
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

float3 cos_weighted_random_hemisphere_direction(float3 n) {
	float2 r = float2(rand(), rand());
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
	uint2 sample_point = DispatchRaysIndex().xy;
	uint2 sample_dim = DispatchRaysDimensions().xy;
	uint id = sample_point.x + sample_dim.x * sample_point.y;
	compute_seed(id, constant_buffer.eye.w, 0);

	float3 hit_position = hit_world_position();

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

	float emittance = 0.0;
	float3 texpaint0 = mytexture0.Load(uint3(tex_coord * 2048, 0)).rgb;
	float3 texpaint1 = mytexture1.Load(uint3(tex_coord * 2048, 0)).rgb;
	float3 texpaint2 = mytexture2.Load(uint3(tex_coord * 2048, 0)).rgb;
	if (hit_position.z > 0.99) {
		emittance = 1.0;
		texpaint0 = float3(5, 5, 5); // temp
	}
	float3 color = payload.color.rgb * texpaint0.rgb;
	
	if (emittance == 0.0f) {
		if (texpaint2.b >= 0.99) {
			payload.ray_dir = lerp(reflect(WorldRayDirection(), n), cos_weighted_random_hemisphere_direction(n), texpaint2.g);
		}
		// else if (texpaint2.matid == glass) {
		// 	float indexOfRefraction = 1.0f;
		// 	// adjust eta & normal according to direction of ray (inside or outside mat)
		// 	bool inside = dot(WorldRayDirection(), n) > 0.f;
		// 	float3 tempNormal = n * (inside ? -1.0f : 1.0f);
		// 	float eta = inside ? indexOfRefraction : (1.0f / indexOfRefraction);
		// 	// normal refraction
		// 	float3 newDir = refract(WorldRayDirection(), tempNormal, eta);
		// 	// internal total reflection
		// 	if (length(newDir) < 0.01f) {
		// 		color *= 0;
		// 		newDir = reflect(WorldRayDirection(), n);
		// 	}
		// 	// use schlick's approx
		// 	float schlick_0 = pow((inside ? indexOfRefraction - 1.0f : 1.0f - indexOfRefraction) /
		// 		(1.0f + indexOfRefraction), 2.0f);
		// 	float schlick_coef = schlick_0 +
		// 		(1 - schlick_0) * pow(1 - max(0.0f, dot(WorldRayDirection(), n)), 5);
		// 	// based on coef, pick either a refraction or reflection
		// 	newDir = schlick_coef < rand() ? reflect(WorldRayDirection(), n) : newDir;
		// 	payload.ray_dir = newDir;
		// }
		else {
			// TEMP
			float fresnel = schlick(dot(n, payload.ray_dir), 1.35) * (1.0 - texpaint2.g);
			if (rand() > fresnel) {
				payload.ray_dir = cos_weighted_random_hemisphere_direction(n);

				// float3 wo = -payload.ray_dir;
				// payload.ray_dir = cos_weighted_random_hemisphere_direction(n);
				// float3 tangent = float3(0, 0, 0);
				// float3 binormal = float3(0, 0, 0);
				// create_basis(n, tangent, binormal);
				// float3 wi = payload.ray_dir.x * tangent + payload.ray_dir.y * binormal + payload.ray_dir.z * n;
				// float dotNL = dot(n, wo);
				// float dotNV = dot(n, wi);
				// float3 H = normalize(wo + wi);
				// float dotLH = dot(wo, H);
				// float FL = schlick_weight(dotNL);
				// float FV = schlick_weight(dotNV);
				// float Fss90 = dotLH * dotLH * texpaint2.g;
				// float Fss = lerp(1.0, Fss90, FL) * lerp(1.0, Fss90, FV);
				// float ss = 1.25 * (Fss * (1.0 / (dotNL + dotNV) - 0.5) + 0.5);
				// color = (1 / PI) * ss * color;
				// if (dotNL < 0.0 || dotNV < 0.0) color = float3(0, 0, 0);
			}
			else {
				payload.ray_dir = reflect(WorldRayDirection(), n);
			}
		}
		payload.ray_origin = hit_position + payload.ray_dir * 0.0001f;
	}

	payload.color = float4(color.xyz, emittance);
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
	float2 tex_coord = equirect(normalize(payload.ray_dir));
	float3 texenv = mytexture_env.Load(uint3(tex_coord.x * 1024, tex_coord.y * 512, 0)).rgb * 3;
	payload.color = float4(payload.color.rgb * texenv.rgb, -1);
}
