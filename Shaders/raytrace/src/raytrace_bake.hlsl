
struct Vertex {
	float3 position;
	float3 normal;
	float2 tex;
};

struct RayGenConstantBuffer {
	float4 v0; // frame
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

static uint seed;

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

[shader("raygeneration")]
void raygeneration() {

	uint id = DispatchRaysIndex().x + DispatchRaysDimensions().x * DispatchRaysIndex().y;
	compute_seed(id, constant_buffer.v0.x, 0);

	float2 xy = DispatchRaysIndex().xy + 0.5f;
	float3 pos = mytexture0.Load(uint3(xy, 0)).rgb;
	float3 nor = mytexture1.Load(uint3(xy, 0)).rgb;

	RayPayload payload;
	payload.ray_origin = pos;
	payload.ray_dir = cos_weighted_random_hemisphere_direction(nor);

	RayDesc ray;
	ray.TMin = 0.01;
	ray.TMax = 10.0;
	// ray.TMin = 0.005; // cavity
	// ray.TMax = 0.01;
	ray.Origin = payload.ray_origin;
	ray.Direction = payload.ray_dir;

	TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);

	float3 color = float3(render_target[DispatchRaysIndex().xy].xyz);
	float a = 1.0 / constant_buffer.v0.x;
	float b = 1.0 - a;

	color = color * b + (1.0 - payload.color.r).xxx * a;
	render_target[DispatchRaysIndex().xy] = float4(color.xyz, 1.0f);
}

[shader("closesthit")]
void closesthit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
	payload.color = float4(1, 1, 1, 1);
}

[shader("miss")]
void miss(inout RayPayload payload) {
	payload.color = float4(0, 0, 0, 0);
}
