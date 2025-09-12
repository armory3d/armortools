
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

let str_tex_checker: string = "\
fun tex_checker(co: float3, col1: float3, col2: float3, scale: float): float3 { \
	/* Prevent precision issues on unit coordinates */ \
	var p: float3 = (co + 0.000001 * 0.999999) * scale; \
	var xi: float = abs(floor(p.x)); \
	var yi: float = abs(floor(p.y)); \
	var zi: float = abs(floor(p.z)); \
	/* var check: bool = (xi % 2.0 == yi % 2.0) == zi % 2.0;*/ \
	var checka: int = 0; \
	var checkb: int = 0; \
	if (xi % 2.0 == yi % 2.0) { checka = 1; } \
	if (zi % 2.0 != 0.0) { checkb = 1; } \
	if (checka == checkb) { return col1; } return col2; \
} \
fun tex_checker_f(co: float3, scale: float): float { \
	var p: float3 = (co + 0.000001 * 0.999999) * scale; \
	var xi: float = abs(floor(p.x)); \
	var yi: float = abs(floor(p.y)); \
	var zi: float = abs(floor(p.z)); \
	/*return float((xi % 2.0 == yi % 2.0) == zi % 2.0);*/ \
	var checka: int = 0; \
	var checkb: int = 0; \
	if (xi % 2.0 == yi % 2.0) { checka = 1; } \
	if (zi % 2.0 != 0.0) { checkb = 1; } \
	if (checka == checkb) { return 1.0; } return 0.0; \
	\
} \
";

	// Created by inigo quilez - iq/2013
	// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
let str_tex_voronoi: string = "\
fun tex_voronoi(x: float3): float4 { \
	var p: float3 = floor3(x); \
	var f: float3 = frac3(x); \
	var id: float = 0.0; \
	var res: float = 100.0; \
	for (var k: int = 0; k <= 2; k += 1) \
	for (var j: int = 0; j <= 2; j += 1) \
	for (var i: int = 0; i <= 2; i += 1) { \
		var b: float3 = float3(float(i - 1), float(j - 1), float(k - 1)); \
		var pb: float3 = p + b; \
		var snoise_sample: float3 = sample(snoise256, sampler_linear, (pb.xy + float2(3.0, 1.0) * pb.z + 0.5) / 256.0).xyz; \
		var r: float3 = b - f + snoise_sample; \
		var d: float = dot(r, r); \
		if (d < res) { \
			id = dot(p + b, float3(1.0, 57.0, 113.0)); \
			res = d; \
		} \
	} \
	/*var col: float3 = 0.5 + 0.5 * cos(id * 0.35 + float3(0.0, 1.0, 2.0));*/ \
	var col: float3; \
	col.x = 0.5 + 0.5 * cos(id * 0.35 + 0.0); \
	col.y = 0.5 + 0.5 * cos(id * 0.35 + 1.0); \
	col.z = 0.5 + 0.5 * cos(id * 0.35 + 2.0); \
	return float4(col, sqrt(res)); \
} \
";

	// By Morgan McGuire @morgan3d, http://graphicscodex.com Reuse permitted under the BSD license.
	// https://www.shadertoy.com/view/4dS3Wd
let str_tex_noise: string = "\
fun hash(n: float): float { return frac(sin(n) * 10000.0); } \
fun tex_noise_f(x: float3): float { \
    var step: float3 = float3(110.0, 241.0, 171.0); \
    var i: float3 = floor3(x); \
    var f: float3 = frac3(x); \
    var n: float = dot(i, step); \
    var u: float3 = f * f * (3.0 - 2.0 * f); \
    return lerp(lerp(lerp(hash(n + dot(step, float3(0.0, 0.0, 0.0))), hash(n + dot(step, float3(1.0, 0.0, 0.0))), u.x), \
                     lerp(hash(n + dot(step, float3(0.0, 1.0, 0.0))), hash(n + dot(step, float3(1.0, 1.0, 0.0))), u.x), u.y), \
                lerp(lerp(hash(n + dot(step, float3(0.0, 0.0, 1.0))), hash(n + dot(step, float3(1.0, 0.0, 1.0))), u.x), \
                     lerp(hash(n + dot(step, float3(0.0, 1.0, 1.0))), hash(n + dot(step, float3(1.0, 1.0, 1.0))), u.x), u.y), u.z); \
} \
fun tex_noise(p: float3): float { \
	p = p * 1.25; \
	var f: float = 0.5 * tex_noise_f(p); p = p * 2.01; \
	f += 0.25 * tex_noise_f(p); p = p * 2.02; \
	f += 0.125 * tex_noise_f(p); p = p * 2.03; \
	f += 0.0625 * tex_noise_f(p); \
	return 1.0 - f; \
} \
";

	// Based on noise created by Nikita Miropolskiy, nikat/2013
	// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
let str_tex_musgrave: string = "\
fun random3(c: float3): float3 { \
	var j: float = 4096.0 * sin(dot(c, float3(17.0, 59.4, 15.0))); \
	var r: float3; \
	r.z = frac(512.0 * j); \
	j *= 0.125; \
	r.x = frac(512.0 * j); \
	j *= 0.125; \
	r.y = frac(512.0 * j); \
	return r - 0.5; \
} \
fun tex_musgrave_f(p: float3): float { \
	var F3: float = 0.3333333; \
	var G3: float = 0.1666667; \
	var s: float3 = floor3(p + dot(p, float3(F3, F3, F3))); \
	var x: float3 = p - s + dot(s, float3(G3, G3, G3)); \
	var e: float3 = step3(float3(0.0, 0.0, 0.0), x - x.yzx); \
	var i1: float3 = e * (1.0 - e.zxy); \
	var i2: float3 = 1.0 - e.zxy * (1.0 - e); \
	var x1: float3 = x - i1 + G3; \
	var x2: float3 = x - i2 + 2.0 * G3; \
	var x3: float3 = x - 1.0 + 3.0 * G3; \
	var w: float4; \
	var d: float4; \
	w.x = dot(x, x); \
	w.y = dot(x1, x1); \
	w.z = dot(x2, x2); \
	w.w = dot(x3, x3); \
	w = max4(float4(0.6, 0.6, 0.6, 0.6) - w, float4(0.0, 0.0, 0.0, 0.0)); \
	d.x = dot(random3(s), x); \
	d.y = dot(random3(s + i1), x1); \
	d.z = dot(random3(s + i2), x2); \
	d.w = dot(random3(s + 1.0), x3); \
	w *= w; \
	w *= w; \
	d *= w; \
	return clamp(dot(d, float4(52.0, 52.0, 52.0, 52.0)), 0.0, 1.0); \
} \
";

let str_hue_sat: string = "\
fun hsv_to_rgb(c: float3): float3 { \
	var K: float4 = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0); \
	var p: float3 = abs3(frac3(c.xxx + K.xyz) * 6.0 - K.www); \
	return lerp3(K.xxx, clamp3(p - K.xxx, float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0)), c.y) * c.z; \
} \
fun rgb_to_hsv(c: float3): float3 { \
	var K: float4 = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0); \
	var p: float4 = lerp4(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g)); \
	var q: float4 = lerp4(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r)); \
	var d: float = q.x - min(q.w, q.y); \
	var e: float = 0.0000000001; \
	return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x); \
} \
fun hue_sat(col: float3, shift: float4): float3 { \
	var hsv: float3 = rgb_to_hsv(col); \
	hsv.x += shift.x; \
	hsv.y *= shift.y; \
	hsv.z *= shift.z; \
	return lerp3(hsv_to_rgb(hsv), col, shift.w); \
} \
";

// https://x.com/Donzanoid/status/903424376707657730
let str_wavelength_to_rgb: string = "\
fun wavelength_to_rgb(t: float): float3 { \
	var r: float3 = t * 2.1 - float3(1.8, 1.14, 0.3); \
	return 1.0 - r * r; \
} \
";

let str_tex_magic: string = "\
fun tex_magic(p: float3): float3 { \
	var a: float = 1.0 - (sin(p.x) + sin(p.y)); \
	var b: float = 1.0 - sin(p.x - p.y); \
	var c: float = 1.0 - sin(p.x + p.y); \
	return float3(a, b, c); \
} \
fun tex_magic_f(p: float3): float { \
	var c: float3 = tex_magic(p); \
	return (c.x + c.y + c.z) / 3.0; \
} \
";

let str_tex_brick: string = "\
fun tex_brick_noise(n: int): float { \
	var nn: int; \
	n = (n >> 13) ^ n; \
	/*nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;*/ \
	nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 2147483647; \
	return 0.5 * float(nn) / 1073741824.0; \
} \
fun tex_brick(p: float3, c1: float3, c2: float3, c3: float3): float3 { \
	var brick_size: float3 = float3(0.9, 0.49, 0.49); \
	var mortar_size: float3 = float3(0.05, 0.1, 0.1); \
	p /= brick_size / 2.0; \
	if (frac(p.y * 0.5) > 0.5) { p.x += 0.5; } \
	var col: float = floor(p.x / (brick_size.x + (mortar_size.x * 2.0))); \
	var row: float = p.y; \
	p = frac3(p); \
	var b: float3 = step3(p, 1.0 - mortar_size); \
	/*var tint: float = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 0xffff)), 0.0), 1.0);*/ \
	var tint: float = min(max(tex_brick_noise((int(col) << 16) + (int(row) & 65535)), 0.0), 1.0); \
	return lerp3(c3, lerp3(c1, c2, tint), b.x * b.y * b.z); \
} \
fun tex_brick_f(p: float3): float { \
	p /= float3(0.9, 0.49, 0.49) / 2.0; \
	if (frac(p.y * 0.5) > 0.5) { p.x += 0.5; } \
	p = frac3(p); \
	var b: float3 = step3(p, float3(0.95, 0.9, 0.9)); \
	return lerp(1.0, 0.0, b.x * b.y * b.z); \
} \
";

let str_tex_wave: string = "\
fun tex_wave_f(p: float3): float { \
	return 1.0 - sin((p.x + p.y) * 10.0); \
} \
";

let str_brightcontrast: string = "\
fun brightcontrast(col: float3, bright: float, contr: float): float3 { \
	var a: float = 1.0 + contr; \
	var b: float = bright - contr * 0.5; \
	return max3(a * col + b, float3(0.0, 0.0, 0.0)); \
} \
";

let str_cotangent_frame: string = "\
fun cotangent_frame(n: float3, p: float3, tex_coord: float2): float3x3 { \
	var duv1: float2 = ddx2(tex_coord); \
	var duv2: float2 = ddy2(tex_coord); \
	var dp1: float3 = ddx3(p); \
	var dp2: float3 = ddy3(p); \
	var dp2perp: float3 = cross(dp2, n); \
	var dp1perp: float3 = cross(n, dp1); \
	var t: float3 = dp2perp * duv1.x + dp1perp * duv2.x; \
	var b: float3 = dp2perp * duv1.y + dp1perp * duv2.y; \
	var invmax: float = rsqrt(max(dot(t, t), dot(b, b))); \
	return float3x3(t * invmax, b * invmax, n); \
} \
";

let str_octahedron_wrap: string = "\
fun octahedron_wrap(v: float2): float2 { \
	var a: float2; \
	if (v.x >= 0.0) { a.x = 1.0; } else { a.x = -1.0; } \
	if (v.y >= 0.0) { a.y = 1.0; } else { a.y = -1.0; } \
	var r: float2; \
	r.x = abs(v.y); \
	r.y = abs(v.x); \
	r.x = 1.0 - r.x; \
	r.y = 1.0 - r.y; \
	return r * a; \
} \
";

// let str_octahedron_wrap: string = "\
// fun octahedron_wrap(v: float2): float2 { \
// 	return (1.0 - abs(v.yx)) * (float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0)); \
// } \
// ";

let str_pack_float_int16: string = "\
fun pack_f32_i16(f: float, i: uint): float { \
	return 0.062504762 * f + 0.062519999 * float(i); \
} \
";

// let str_pack_float_int16: string = "\
// fun pack_f32_i16(f: float, i: uint): float { \
// 	var prec: float = float(1 << 16); \
// 	var maxi: float = float(1 << 4); \
// 	var prec_minus_one: float = prec - 1.0; \
// 	var t1: float = ((prec / maxi) - 1.0) / prec_minus_one; \
// 	var t2: float = (prec / maxi) / prec_minus_one; \
// 	return t1 * f + t2 * float(i); \
// } \
// ";

let str_create_basis: string = "\
fun create_basis(normal: float3, out tangent: float3, out binormal: float3) { \
	tangent = normalize(camera_right - normal * dot(camera_right, normal)); \
	binormal = cross(tangent, normal); \
} \
";

let str_sh_irradiance: string = "\
fun sh_irradiance(nor: float3): float3 { \
	var c1: float = 0.429043; \
	var c2: float = 0.511664; \
	var c3: float = 0.743125; \
	var c4: float = 0.886227; \
	var c5: float = 0.247708; \
	var cl00: float3 = float3(constants.shirr0.x, constants.shirr0.y, constants.shirr0.z); \
	var cl1m1: float3 = float3(constants.shirr0.w, constants.shirr1.x, constants.shirr1.y); \
	var cl10: float3 = float3(constants.shirr1.z, constants.shirr1.w, constants.shirr2.x); \
	var cl11: float3 = float3(constants.shirr2.y, constants.shirr2.z, constants.shirr2.w); \
	var cl2m2: float3 = float3(constants.shirr3.x, constants.shirr3.y, constants.shirr3.z); \
	var cl2m1: float3 = float3(constants.shirr3.w, constants.shirr4.x, constants.shirr4.y); \
	var cl20: float3 = float3(constants.shirr4.z, constants.shirr4.w, constants.shirr5.x); \
	var cl21: float3 = float3(constants.shirr5.y, constants.shirr5.z, constants.shirr5.w); \
	var cl22: float3 = float3(constants.shirr6.x, constants.shirr6.y, constants.shirr6.z); \
	return ( \
		cl22 * c1 * (nor.y * nor.y - (-nor.z) * (-nor.z)) + \
		cl20 * c3 * nor.x * nor.x + \
		cl00 * c4 - \
		cl20 * c5 + \
		cl2m2 * 2.0 * c1 * nor.y * (-nor.z) + \
		cl21  * 2.0 * c1 * nor.y * nor.x + \
		cl2m1 * 2.0 * c1 * (-nor.z) * nor.x + \
		cl11  * 2.0 * c2 * nor.y + \
		cl1m1 * 2.0 * c2 * (-nor.z) + \
		cl10  * 2.0 * c2 * nor.x \
	); \
} \
";

let str_envmap_equirect: string = "\
fun envmap_equirect(normal: float3, angle: float): float2 { \
	var PI: float = 3.1415926535; \
	var PI2: float = PI * 2.0; \
	var phi: float = acos(normal.z); \
	var theta: float = atan2(-normal.y, normal.x) + PI + angle; \
	return float2(theta / PI2, phi / PI); \
} \
";

let str_envmap_sample: string = "\
fun envmap_sample(lod: float, coord: float2): float3 { \
	if (lod == 0.0) { \
		return sample_lod(senvmap_radiance, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 1.0) { \
		return sample_lod(senvmap_radiance0, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 2.0) { \
		return sample_lod(senvmap_radiance1, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 3.0) { \
		return sample_lod(senvmap_radiance2, sampler_linear, coord, 0.0).rgb; \
	} \
	if (lod == 4.0) { \
		return sample_lod(senvmap_radiance3, sampler_linear, coord, 0.0).rgb; \
	} \
	return sample_lod(senvmap_radiance4, sampler_linear, coord, 0.0).rgb; \
} \
";

let str_get_pos_nor_from_depth: string = "\
fun get_pos_from_depth(uv: float2, invVP: float4x4): float3 { \
	var depth: float = sample_lod(gbufferD, sampler_linear, float2(uv.x, 1.0 - uv.y), 0.0).r; \
	var wpos: float4 = float4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0); \
	wpos = invVP * wpos; \
	return wpos.xyz / wpos.w; \
} \
fun get_nor_from_depth(p0: float3, uv: float2, invVP: float4x4, tex_step: float2): float3 { \
	var p1: float3 = get_pos_from_depth(uv + float2(tex_step.x * 4.0, 0.0), invVP); \
	var p2: float3 = get_pos_from_depth(uv + float2(0.0, tex_step.y * 4.0), invVP); \
	return normalize(cross(p2 - p0, p1 - p0)); \
} \
";

////
let str_get_smudge_tool_weight: string = "\
fun get_smudge_tool_weight(i: int): float { \
	if (i == 0) { return 1.0 / 28.0; } \
	if (i == 1) { return 2.0 / 28.0; } \
	if (i == 2) { return 3.0 / 28.0; } \
	if (i == 3) { return 4.0 / 28.0; } \
	if (i == 4) { return 5.0 / 28.0; } \
	if (i == 5) { return 6.0 / 28.0; } \
	return 7.0 / 28.0; \
} \
";

let str_get_blur_tool_weight: string = "\
fun get_blur_tool_weight(i: int): float { \
	if (i == 0) { return 0.034619 / 2.0; } \
	if (i == 1) { return 0.044859 / 2.0; } \
	if (i == 2) { return 0.055857 / 2.0; } \
	if (i == 3) { return 0.066833 / 2.0; } \
	if (i == 4) { return 0.076841 / 2.0; } \
	if (i == 5) { return 0.084894 / 2.0; } \
	if (i == 6) { return 0.090126 / 2.0; } \
	if (i == 7) { return 0.09194 / 2.0; } \
	if (i == 8) { return 0.090126 / 2.0; } \
	if (i == 9) { return 0.084894 / 2.0; } \
	if (i == 10) { return 0.076841 / 2.0; } \
	if (i == 11) { return 0.066833 / 2.0; } \
	if (i == 12) { return 0.055857 / 2.0; } \
	if (i == 13) { return 0.044859 / 2.0; } \
	return 0.034619 / 2.0; \
} \
";
////
