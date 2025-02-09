
function strings_arm_file_expected(): string {
	return tr("Error: .arm file expected");
}

function strings_unknown_asset_format(): string {
	return tr("Error: Unknown asset format");
}

function strings_could_not_locate_texture(): string {
	return tr("Error: Could not locate texture");
}

function strings_failed_to_read_mesh_data(): string {
	return tr("Error: Failed to read mesh data");
}

function strings_check_internet_connection(): string {
	return tr("Error: Check internet connection to access the cloud");
}

function strings_asset_already_imported(): string {
	return tr("Info: Asset already imported");
}

function strings_graphics_api(): string {
	///if arm_direct3d12
	return "Direct3D12";
	///elseif arm_metal
	return "Metal";
	///else
	return "Vulkan";
	///end
}

let str_tex_checker: string = " \
vec3 tex_checker(const vec3 co, const vec3 col1, const vec3 col2, const float scale) { \
	/* Prevent precision issues on unit coordinates */ \
	vec3 p = (co + 0.000001 * 0.999999) * scale; \
	float xi = abs(floor(p.x)); \
	float yi = abs(floor(p.y)); \
	float zi = abs(floor(p.z)); \
	bool check = ((mod(xi, 2.0) == mod(yi, 2.0)) == bool(mod(zi, 2.0))); \
	return check ? col1 : col2; \
} \
float tex_checker_f(const vec3 co, const float scale) { \
	vec3 p = (co + 0.000001 * 0.999999) * scale; \
	float xi = abs(floor(p.x)); \
	float yi = abs(floor(p.y)); \
	float zi = abs(floor(p.z)); \
	return float((mod(xi, 2.0) == mod(yi, 2.0)) == bool(mod(zi, 2.0))); \
} \
";

	// Created by inigo quilez - iq/2013
	// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
let str_tex_voronoi: string = " \
vec4 tex_voronoi(const vec3 x, textureArg(snoise256)) { \
	vec3 p = floor(x); \
	vec3 f = fract(x); \
	float id = 0.0; \
	float res = 100.0; \
	for (int k = -1; k <= 1; k++) \
	for (int j = -1; j <= 1; j++) \
	for (int i = -1; i <= 1; i++) { \
		vec3 b = vec3(float(i), float(j), float(k)); \
		vec3 pb = p + b; \
		vec3 r = vec3(b) - f + texture(snoise256, (pb.xy + vec2(3.0, 1.0) * pb.z + 0.5) / 256.0).xyz; \
		float d = dot(r, r); \
		if (d < res) { \
			id = dot(p + b, vec3(1.0, 57.0, 113.0)); \
			res = d; \
		} \
	} \
	vec3 col = 0.5 + 0.5 * cos(id * 0.35 + vec3(0.0, 1.0, 2.0)); \
	return vec4(col, sqrt(res)); \
} \
";

	// By Morgan McGuire @morgan3d, http://graphicscodex.com Reuse permitted under the BSD license.
	// https://www.shadertoy.com/view/4dS3Wd
let str_tex_noise: string = " \
float hash(float n) { return fract(sin(n) * 1e4); } \
float tex_noise_f(vec3 x) { \
    const vec3 step = vec3(110, 241, 171); \
    vec3 i = floor(x); \
    vec3 f = fract(x); \
    float n = dot(i, step); \
    vec3 u = f * f * (3.0 - 2.0 * f); \
    return mix(mix(mix(hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x), \
                   mix(hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y), \
               mix(mix(hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x), \
                   mix(hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z); \
} \
float tex_noise(vec3 p) { \
	p *= 1.25; \
	float f = 0.5 * tex_noise_f(p); p *= 2.01; \
	f += 0.25 * tex_noise_f(p); p *= 2.02; \
	f += 0.125 * tex_noise_f(p); p *= 2.03; \
	f += 0.0625 * tex_noise_f(p); \
	return 1.0 - f; \
} \
";

	// Based on noise created by Nikita Miropolskiy, nikat/2013
	// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
let str_tex_musgrave: string = " \
vec3 random3(const vec3 c) { \
	float j = 4096.0 * sin(dot(c, vec3(17.0, 59.4, 15.0))); \
	vec3 r; \
	r.z = fract(512.0 * j); \
	j *= 0.125; \
	r.x = fract(512.0 * j); \
	j *= 0.125; \
	r.y = fract(512.0 * j); \
	return r - 0.5; \
} \
float tex_musgrave_f(const vec3 p) { \
	const float F3 = 0.3333333; \
	const float G3 = 0.1666667; \
	vec3 s = floor(p + dot(p, vec3(F3, F3, F3))); \
	vec3 x = p - s + dot(s, vec3(G3, G3, G3)); \
	vec3 e = step(vec3(0.0, 0.0, 0.0), x - x.yzx); \
	vec3 i1 = e * (1.0 - e.zxy); \
	vec3 i2 = 1.0 - e.zxy * (1.0 - e); \
	vec3 x1 = x - i1 + G3; \
	vec3 x2 = x - i2 + 2.0 * G3; \
	vec3 x3 = x - 1.0 + 3.0 * G3; \
	vec4 w, d; \
	w.x = dot(x, x); \
	w.y = dot(x1, x1); \
	w.z = dot(x2, x2); \
	w.w = dot(x3, x3); \
	w = max(0.6 - w, 0.0); \
	d.x = dot(random3(s), x); \
	d.y = dot(random3(s + i1), x1); \
	d.z = dot(random3(s + i2), x2); \
	d.w = dot(random3(s + 1.0), x3); \
	w *= w; \
	w *= w; \
	d *= w; \
	return clamp(dot(d, vec4(52.0, 52.0, 52.0, 52.0)), 0.0, 1.0); \
} \
";

let str_hue_sat: string = " \
vec3 hsv_to_rgb(const vec3 c) { \
	const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0); \
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www); \
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y); \
} \
vec3 rgb_to_hsv(const vec3 c) { \
	const vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0); \
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g)); \
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r)); \
	float d = q.x - min(q.w, q.y); \
	float e = 1.0e-10; \
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x); \
} \
vec3 hue_sat(const vec3 col, const vec4 shift) { \
	vec3 hsv = rgb_to_hsv(col); \
	hsv.x += shift.x; \
	hsv.y *= shift.y; \
	hsv.z *= shift.z; \
	return mix(hsv_to_rgb(hsv), col, shift.w); \
} \
";

// https://twitter.com/Donzanoid/status/903424376707657730
let str_wavelength_to_rgb: string = " \
vec3 wavelength_to_rgb(const float t) { \
	vec3 r = t * 2.1 - vec3(1.8, 1.14, 0.3); \
	return 1.0 - r * r; \
} \
";

let str_tex_magic: string = " \
vec3 tex_magic(const vec3 p) { \
	float a = 1.0 - (sin(p.x) + sin(p.y)); \
	float b = 1.0 - sin(p.x - p.y); \
	float c = 1.0 - sin(p.x + p.y); \
	return vec3(a, b, c); \
} \
float tex_magic_f(const vec3 p) { \
	vec3 c = tex_magic(p); \
	return (c.x + c.y + c.z) / 3.0; \
} \
";

let str_tex_brick: string = " \
float tex_brick_noise(int n) { \
	int nn; \
	n = (n >> 13) ^ n; \
	nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff; \
	return 0.5f * float(nn) / 1073741824.0; \
} \
vec3 tex_brick(vec3 p, const vec3 c1, const vec3 c2, const vec3 c3) { \
	vec3 brick_size = vec3(0.9, 0.49, 0.49); \
	vec3 mortar_size = vec3(0.05, 0.1, 0.1); \
	p /= brick_size / 2; \
	if (fract(p.y * 0.5) > 0.5) p.x += 0.5; \
	float col = floor(p.x / (brick_size.x + (mortar_size.x * 2.0))); \
	float row = p.y; \
	p = fract(p); \
	vec3 b = step(p, 1.0 - mortar_size); \
	float tint = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 0xFFFF)), 0.0), 1.0); \
	return mix(c3, mix(c1, c2, tint), b.x * b.y * b.z); \
} \
float tex_brick_f(vec3 p) { \
	p /= vec3(0.9, 0.49, 0.49) / 2; \
	if (fract(p.y * 0.5) > 0.5) p.x += 0.5; \
	p = fract(p); \
	vec3 b = step(p, vec3(0.95, 0.9, 0.9)); \
	return mix(1.0, 0.0, b.x * b.y * b.z); \
} \
";

let str_tex_wave: string = " \
float tex_wave_f(const vec3 p) { \
	return 1.0 - sin((p.x + p.y) * 10.0); \
} \
";

let str_brightcontrast: string = " \
vec3 brightcontrast(const vec3 col, const float bright, const float contr) { \
	float a = 1.0 + contr; \
	float b = bright - contr * 0.5; \
	return max(a * col + b, 0.0); \
} \
";

let str_cotangent_frame: string = " \
mat3 cotangent_frame(const vec3 n, const vec3 p, const vec2 duv1, const vec2 duv2) { \
	vec3 dp1 = dFdx(p); \
	vec3 dp2 = dFdy(p); \
	vec3 dp2perp = cross(dp2, n); \
	vec3 dp1perp = cross(n, dp1); \
	vec3 t = dp2perp * duv1.x + dp1perp * duv2.x; \
	vec3 b = dp2perp * duv1.y + dp1perp * duv2.y; \
	float invmax = inversesqrt(max(dot(t, t), dot(b, b))); \
	return mat3(t * invmax, b * invmax, n); \
} \
mat3 cotangent_frame(const vec3 n, const vec3 p, const vec2 tex_coord) { \
	return cotangent_frame(n, p, dFdx(tex_coord), dFdy(tex_coord)); \
} \
";

let str_octahedron_wrap: string = " \
vec2 octahedron_wrap(const vec2 v) { \
	return (1.0 - abs(v.yx)) * (vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0)); \
} \
";

let str_pack_float_int16: string = " \
float pack_f32_i16(const float f, const uint i) { \
	const float prec = float(1 << 16); \
	const float maxi = float(1 << 4); \
	const float prec_minus_one = prec - 1.0; \
	const float t1 = ((prec / maxi) - 1.0) / prec_minus_one; \
	const float t2 = (prec / maxi) / prec_minus_one; \
	return t1 * f + t2 * float(i); \
} \
";

///if arm_skin
let str_get_skinning_dual_quat: string = " \
void get_skinning_dual_quat(const ivec4 bone, vec4 weight, out vec4 A, inout vec4 B) { \
	ivec4 bonei = bone * 2; \
	mat4 mat_a = mat4( \
		skin_bones[bonei.x], \
		skin_bones[bonei.y], \
		skin_bones[bonei.z], \
		skin_bones[bonei.w]); \
	mat4 mat_b = mat4( \
		skin_bones[bonei.x + 1], \
		skin_bones[bonei.y + 1], \
		skin_bones[bonei.z + 1], \
		skin_bones[bonei.w + 1]); \
	weight.xyz *= sign(mul(mat_a, mat_a[3])).xyz; \
	A = mul(weight, mat_a); \
	B = mul(weight, mat_b); \
	float inv_norm_a = 1.0 / length(A); \
	A *= inv_norm_a; \
	B *= inv_norm_a; \
} \
";
///end

let str_create_basis: string = " \
void create_basis(vec3 normal, out vec3 tangent, out vec3 binormal) { \
	tangent = normalize(camera_right - normal * dot(camera_right, normal)); \
	binormal = cross(tangent, normal); \
} \
";

function str_sh_irradiance(): string {
///if arm_metal
	return "vec3 sh_irradiance(const vec3 nor, constant vec4 shirr[7]) { \
	const float c1 = 0.429043; \
	const float c2 = 0.511664; \
	const float c3 = 0.743125; \
	const float c4 = 0.886227; \
	const float c5 = 0.247708; \
	vec3 cl00 = vec3(shirr[0].x, shirr[0].y, shirr[0].z); \
	vec3 cl1m1 = vec3(shirr[0].w, shirr[1].x, shirr[1].y); \
	vec3 cl10 = vec3(shirr[1].z, shirr[1].w, shirr[2].x); \
	vec3 cl11 = vec3(shirr[2].y, shirr[2].z, shirr[2].w); \
	vec3 cl2m2 = vec3(shirr[3].x, shirr[3].y, shirr[3].z); \
	vec3 cl2m1 = vec3(shirr[3].w, shirr[4].x, shirr[4].y); \
	vec3 cl20 = vec3(shirr[4].z, shirr[4].w, shirr[5].x); \
	vec3 cl21 = vec3(shirr[5].y, shirr[5].z, shirr[5].w); \
	vec3 cl22 = vec3(shirr[6].x, shirr[6].y, shirr[6].z); \
	return ( \
		c1 * cl22 * (nor.y * nor.y - (-nor.z) * (-nor.z)) + \
		c3 * cl20 * nor.x * nor.x + \
		c4 * cl00 - \
		c5 * cl20 + \
		2.0 * c1 * cl2m2 * nor.y * (-nor.z) + \
		2.0 * c1 * cl21  * nor.y * nor.x + \
		2.0 * c1 * cl2m1 * (-nor.z) * nor.x + \
		2.0 * c2 * cl11  * nor.y + \
		2.0 * c2 * cl1m1 * (-nor.z) + \
		2.0 * c2 * cl10  * nor.x \
	); \
} \
";
///else
	return "vec3 sh_irradiance(const vec3 nor, const vec4 shirr[7]) { \
	const float c1 = 0.429043; \
	const float c2 = 0.511664; \
	const float c3 = 0.743125; \
	const float c4 = 0.886227; \
	const float c5 = 0.247708; \
	vec3 cl00 = vec3(shirr[0].x, shirr[0].y, shirr[0].z); \
	vec3 cl1m1 = vec3(shirr[0].w, shirr[1].x, shirr[1].y); \
	vec3 cl10 = vec3(shirr[1].z, shirr[1].w, shirr[2].x); \
	vec3 cl11 = vec3(shirr[2].y, shirr[2].z, shirr[2].w); \
	vec3 cl2m2 = vec3(shirr[3].x, shirr[3].y, shirr[3].z); \
	vec3 cl2m1 = vec3(shirr[3].w, shirr[4].x, shirr[4].y); \
	vec3 cl20 = vec3(shirr[4].z, shirr[4].w, shirr[5].x); \
	vec3 cl21 = vec3(shirr[5].y, shirr[5].z, shirr[5].w); \
	vec3 cl22 = vec3(shirr[6].x, shirr[6].y, shirr[6].z); \
	return ( \
		c1 * cl22 * (nor.y * nor.y - (-nor.z) * (-nor.z)) + \
		c3 * cl20 * nor.x * nor.x + \
		c4 * cl00 - \
		c5 * cl20 + \
		2.0 * c1 * cl2m2 * nor.y * (-nor.z) + \
		2.0 * c1 * cl21  * nor.y * nor.x + \
		2.0 * c1 * cl2m1 * (-nor.z) * nor.x + \
		2.0 * c2 * cl11  * nor.y + \
		2.0 * c2 * cl1m1 * (-nor.z) + \
		2.0 * c2 * cl10  * nor.x \
	); \
} \
";
///end
}

let str_envmap_equirect: string = " \
vec2 envmap_equirect(const vec3 normal, const float angle) { \
	const float PI = 3.1415926535; \
	const float PI2 = PI * 2.0; \
	float phi = acos(normal.z); \
	float theta = atan2(-normal.y, normal.x) + PI + angle; \
	return vec2(theta / PI2, phi / PI); \
} \
";

// Linearly Transformed Cosines
// https://eheitzresearch.wordpress.com/415-2/
let str_ltc_evaluate: string = " \
float integrate_edge(vec3 v1, vec3 v2) { \
	float cos_theta = dot(v1, v2); \
	float theta = acos(cos_theta); \
	float res = cross(v1, v2).z * ((theta > 0.001) ? theta / sin(theta) : 1.0); \
	return res; \
} \
float ltc_evaluate(vec3 N, vec3 V, float dotnv, vec3 P, mat3 Minv, vec3 points0, vec3 points1, vec3 points2, vec3 points3) { \
	vec3 T1 = normalize(V - N * dotnv); \
	vec3 T2 = cross(N, T1); \
	Minv = mul(transpose(mat3(T1, T2, N)), Minv); \
	vec3 L0 = mul((points0 - P), Minv); \
	vec3 L1 = mul((points1 - P), Minv); \
	vec3 L2 = mul((points2 - P), Minv); \
	vec3 L3 = mul((points3 - P), Minv); \
	vec3 L4 = vec3(0.0, 0.0, 0.0); \
	int n = 0; \
	int config = 0; \
	if (L0.z > 0.0) config += 1; \
	if (L1.z > 0.0) config += 2; \
	if (L2.z > 0.0) config += 4; \
	if (L3.z > 0.0) config += 8; \
	if (config == 0) {} \
	else if (config == 1) { \
		n = 3; \
		L1 = -L1.z * L0 + L0.z * L1; \
		L2 = -L3.z * L0 + L0.z * L3; \
	} \
	else if (config == 2) { \
		n = 3; \
		L0 = -L0.z * L1 + L1.z * L0; \
		L2 = -L2.z * L1 + L1.z * L2; \
	} \
	else if (config == 3) { \
		n = 4; \
		L2 = -L2.z * L1 + L1.z * L2; \
		L3 = -L3.z * L0 + L0.z * L3; \
	} \
	else if (config == 4) { \
		n = 3; \
		L0 = -L3.z * L2 + L2.z * L3; \
		L1 = -L1.z * L2 + L2.z * L1; \
	} \
	else if (config == 5) { n = 0; } \
	else if (config == 6) { \
		n = 4; \
		L0 = -L0.z * L1 + L1.z * L0; \
		L3 = -L3.z * L2 + L2.z * L3; \
	} \
	else if (config == 7) { \
		n = 5; \
		L4 = -L3.z * L0 + L0.z * L3; \
		L3 = -L3.z * L2 + L2.z * L3; \
	} \
	else if (config == 8) { \
		n = 3; \
		L0 = -L0.z * L3 + L3.z * L0; \
		L1 = -L2.z * L3 + L3.z * L2; \
		L2 =  L3; \
	} \
	else if (config == 9) { \
		n = 4; \
		L1 = -L1.z * L0 + L0.z * L1; \
		L2 = -L2.z * L3 + L3.z * L2; \
	} \
	else if (config == 10) { n = 0; } \
	else if (config == 11) { \
		n = 5; \
		L4 = L3; \
		L3 = -L2.z * L3 + L3.z * L2; \
		L2 = -L2.z * L1 + L1.z * L2; \
	} \
	else if (config == 12) { \
		n = 4; \
		L1 = -L1.z * L2 + L2.z * L1; \
		L0 = -L0.z * L3 + L3.z * L0; \
	} \
	else if (config == 13) { \
		n = 5; \
		L4 = L3; \
		L3 = L2; \
		L2 = -L1.z * L2 + L2.z * L1; \
		L1 = -L1.z * L0 + L0.z * L1; \
	} \
	else if (config == 14) { \
		n = 5; \
		L4 = -L0.z * L3 + L3.z * L0; \
		L0 = -L0.z * L1 + L1.z * L0; \
	} \
	else if (config == 15) { n = 4; } \
	if (n == 0) return 0.0; \
	if (n == 3) L3 = L0; \
	if (n == 4) L4 = L0; \
	L0 = normalize(L0); \
	L1 = normalize(L1); \
	L2 = normalize(L2); \
	L3 = normalize(L3); \
	L4 = normalize(L4); \
	float sum = 0.0; \
	sum += integrate_edge(L0, L1); \
	sum += integrate_edge(L1, L2); \
	sum += integrate_edge(L2, L3); \
	if (n >= 4) sum += integrate_edge(L3, L4); \
	if (n == 5) sum += integrate_edge(L4, L0); \
	return max(0.0, -sum); \
} \
";

let str_get_pos_nor_from_depth: string = " \
vec3 get_pos_from_depth(vec2 uv, mat4 invVP, textureArg(gbufferD)) { \n\
#ifdef GLSL \n\
	float depth = textureLod(gbufferD, uv, 0.0).r; \n\
#else \n\
	float depth = textureLod(gbufferD, vec2(uv.x, 1.0 - uv.y), 0.0).r; \n\
#endif \n\
	vec4 wpos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0); \
	wpos = mul(wpos, invVP); \
	return wpos.xyz / wpos.w; \
} \
vec3 get_nor_from_depth(vec3 p0, vec2 uv, mat4 invVP, vec2 tex_step, textureArg(gbufferD)) { \
	vec3 p1 = get_pos_from_depth(uv + vec2(tex_step.x * 4.0, 0.0), invVP, texturePass(gbufferD)); \
	vec3 p2 = get_pos_from_depth(uv + vec2(0.0, tex_step.y * 4.0), invVP, texturePass(gbufferD)); \
	return normalize(cross(p2 - p0, p1 - p0)); \
} \
";
