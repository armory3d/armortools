
#ifndef _DOF_HLSL_
#define _DOF_HLSL_

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
	float lens_rad = 0.005f;
	float focal_dist = 0.4f;
	float3 plens = float3(lens_rad * calculate_concentric_sample_disk(rand(), rand()), 0.0f);
	float ft = focal_dist / abs(ray_dir.z);
	float3 pfocus = ray_dir * ft;
	ray_origin += plens;
	ray_dir = normalize(pfocus - plens);
}

#endif
