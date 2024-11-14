
const float voxelgi_resolution = 256.0;
const vec3 voxelgi_half_extents = vec3(1.0, 1.0, 1.0);
const float voxelgi_occ = 1.0;
const float voxelgi_step = 1.0;
const float voxelgi_range = 2.0;
const float MAX_DISTANCE = 1.73205080757 * voxelgi_range;
const float VOXEL_SIZE = (2.0 / voxelgi_resolution) * voxelgi_step;

vec3 tangent(const vec3 n) {
	vec3 t1 = cross(n, vec3(0.0, 0.0, 1.0));
	vec3 t2 = cross(n, vec3(0.0, 1.0, 0.0));
	if (length(t1) > length(t2)) return normalize(t1);
	else return normalize(t2);
}

float trace_cone_ao(sampler3D voxels, const vec3 origin, vec3 dir, const float aperture, const float max_dist) {
	dir = normalize(dir);
	float sample_col = 0.0;
	float dist = 1.5 * VOXEL_SIZE * cone_offset;
	float diam = dist * aperture;
	vec3 sample_pos;
	while (sample_col < 1.0 && dist < max_dist) {
		sample_pos = dir * dist + origin;
		float mip = max(log2(diam * voxelgi_resolution), 0.0);
		float mip_sample = textureLod(voxels, sample_pos * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), mip).r;
		sample_col += (1 - sample_col) * mip_sample;
		dist += max(diam / 2.0, VOXEL_SIZE);
		diam = dist * aperture;
	}
	return sample_col;
}

float trace_shadow(sampler3D voxels, const vec3 origin, const vec3 dir) {
	return trace_cone_ao(voxels, origin, dir, 0.14 * cone_aperture, 2.5 * voxelgi_range);
}

float trace_ao(const vec3 origin, const vec3 normal, sampler3D voxels) {
	const float angle_mix = 0.5;
	const float aperture = 0.55785173935;
	vec3 o1 = normalize(tangent(normal));
	vec3 o2 = normalize(cross(o1, normal));
	vec3 c1 = vec3(0.5, 0.5, 0.5) * (o1 + o2);
	vec3 c2 = vec3(0.5, 0.5, 0.5) * (o1 - o2);

#ifdef HLSL
	const float factor = voxelgi_occ * 0.93;
#else
	const float factor = voxelgi_occ * 0.90;
#endif

	float col = trace_cone_ao(voxels, origin, normal, aperture, MAX_DISTANCE);
	col += trace_cone_ao(voxels, origin, mix(normal, o1, angle_mix), aperture, MAX_DISTANCE);
	col += trace_cone_ao(voxels, origin, mix(normal, o2, angle_mix), aperture, MAX_DISTANCE);
	col += trace_cone_ao(voxels, origin, mix(normal, -c1, angle_mix), aperture, MAX_DISTANCE);
	col += trace_cone_ao(voxels, origin, mix(normal, -c2, angle_mix), aperture, MAX_DISTANCE);
	return (col / 5.0) * factor;
}
