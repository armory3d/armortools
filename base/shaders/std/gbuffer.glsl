
vec2 octahedron_wrap(const vec2 v) {
	return (1.0 - abs(v.yx)) * (vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0));
}

vec3 get_nor(const vec2 enc) {
	vec3 n;
	n.z = 1.0 - abs(enc.x) - abs(enc.y);
	n.xy = n.z >= 0.0 ? enc.xy : octahedron_wrap(enc.xy);
	n = normalize(n);
	return n;
}

vec3 get_pos_view(const vec3 view_ray, const float depth, const vec2 camera_proj) {
	float linear_depth = camera_proj.y / (camera_proj.x - depth);
	// float linear_depth = camera_proj.y / ((depth * 0.5 + 0.5) - camera_proj.x);
	return view_ray * linear_depth;
}

vec3 get_pos(const vec3 eye, const vec3 eye_look, const vec3 view_ray, const float depth, const vec2 camera_proj) {
	// eye_look, view_ray should be normalized
	float linear_depth = camera_proj.y / ((depth * 0.5 + 0.5) - camera_proj.x);
	float view_z_dist = dot(eye_look, view_ray);
	vec3 wposition = eye + view_ray * (linear_depth / view_z_dist);
	return wposition;
}

// GBuffer helper - Sebastien Lagarde
// https://seblagarde.wordpress.com/2018/09/02/gbuffer-helper-packing-integer-and-float-together/
float pack_f32_i16(const float f, const uint i) {
	// Constant optimize by compiler
	const int num_bit_target = 16;
	const int num_bit_i = 4;
	const float prec = float(1 << num_bit_target);
	const float maxi = float(1 << num_bit_i);
	const float prec_minus_one = prec - 1.0;
	const float t1 = ((prec / maxi) - 1.0) / prec_minus_one;
	const float t2 = (prec / maxi) / prec_minus_one;
	// Code
	return t1 * f + t2 * float(i);
}

void unpack_f32_i16(const float val, OUT(float, f), OUT(uint, i)) {
	// Constant optimize by compiler
	const int num_bit_target = 16;
	const int num_bit_i = 4;
	const float prec = float(1 << num_bit_target);
	const float maxi = float(1 << num_bit_i);
	const float prec_minus_one = prec - 1.0;
	const float t1 = ((prec / maxi) - 1.0) / prec_minus_one;
	const float t2 = (prec / maxi) / prec_minus_one;
	// Code
	// extract integer part
	// + rcp(prec_minus_one) to deal with precision issue
	i = uint((val / t2) + (1.0 / prec_minus_one));
	// Now that we have i, solve formula in pack_f32_i16 for f
	//f = (val - t2 * float(i)) / t1 => convert in mads form
	f = clamp((-t2 * float(i) + val) / t1, 0.0, 1.0); // Saturate in case of precision issue
}
