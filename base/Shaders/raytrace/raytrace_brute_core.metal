
using namespace metal;
using namespace raytracing;

struct Uniforms {
	float4 viewport;
};

kernel void raytracingKernel(
	uint2 tid [[thread_position_in_grid]],
	constant Uniforms &uniforms [[buffer(0)]],
	texture2d<float, access::write> render_target [[texture(0)]],
	instance_acceleration_structure accel [[buffer(1)]]
) {
	if (tid.x < 1280 && tid.y < 720) {
		float2 uv = (float2)tid / float2(1280, 720);
		uv = uv * 2.0f - 1.0f;

		ray ray;
		ray.origin = float3(uv.x, uv.y, 0);
		ray.direction = float3(0, 0, 1);
		ray.min_distance = 0.1f;
		ray.max_distance = 100;

		intersector<triangle_data, instancing> i;
		i.assume_geometry_type(geometry_type::triangle);
		i.force_opacity(forced_opacity::opaque);
		i.accept_any_intersection(true);
		//i.assume_identity_transforms(true);
		//i.set_triangle_cull_mode(triangle_cull_mode::none);

		typename intersector<triangle_data, instancing>::result_type intersection;
		intersection = i.intersect(ray, accel);
		if (intersection.type == intersection_type::none) {
			render_target.write(float4(0.0, 0.0, 0.0, 1.0f), tid);
		}
		else {
			float3 barycentrics = float3(1 - intersection.triangle_barycentric_coord.x - intersection.triangle_barycentric_coord.y, intersection.triangle_barycentric_coord.x, intersection.triangle_barycentric_coord.y);
			render_target.write(float4(barycentrics, 1.0f), tid);
		}
	}
}
