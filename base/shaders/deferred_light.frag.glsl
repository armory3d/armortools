#version 450

uniform sampler2D gbufferD;
uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform vec4 envmap_data; // angle, sin(angle), cos(angle), strength
uniform vec4 shirr[7];
uniform sampler2D senvmap_brdf;
uniform sampler2D senvmap_radiance;
#ifdef SPIRV
uniform float envmap_num_mipmaps;
#else
uniform int envmap_num_mipmaps;
#endif
uniform sampler2D ssaotex;
uniform vec2 camera_proj;
uniform vec3 eye;
uniform vec3 eye_look;
uniform vec3 light_area0;
uniform vec3 light_area1;
uniform vec3 light_area2;
uniform vec3 light_area3;
uniform sampler2D sltc_mat;
uniform sampler2D sltc_mag;

#include "std/gbuffer.glsl"
#include "std/brdf.glsl"
#include "std/math.glsl"
#include "std/shirr.glsl"

in vec2 tex_coord;
in vec3 view_ray;
out vec4 frag_color;

const float LUT_SIZE = 64.0;
const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const float LUT_BIAS = 0.5 / LUT_SIZE;

float integrate_edge(vec3 v1, vec3 v2) {
	float cos_theta = dot(v1, v2);
	float theta = acos(cos_theta);
	float res = cross(v1, v2).z * ((theta > 0.001) ? theta / sin(theta) : 1.0);
	return res;
}

float ltc_evaluate(vec3 N, vec3 V, float dotnv, vec3 P, mat3 Minv, vec3 points0, vec3 points1, vec3 points2, vec3 points3) {
	// Construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N * dotnv);
	T2 = cross(N, T1);

	// Rotate area light in (T1, T2, R) basis
	Minv = mul(transpose(mat3(T1, T2, N)), Minv);

	// Polygon (allocate 5 vertices for clipping)
	vec3 L0 = mul((points0 - P), Minv);
	vec3 L1 = mul((points1 - P), Minv);
	vec3 L2 = mul((points2 - P), Minv);
	vec3 L3 = mul((points3 - P), Minv);
	vec3 L4 = vec3(0.0, 0.0, 0.0);

	int n = 0;
	// Detect clipping config
	int config = 0;
	if (L0.z > 0.0) config += 1;
	if (L1.z > 0.0) config += 2;
	if (L2.z > 0.0) config += 4;
	if (L3.z > 0.0) config += 8;

	// Clip
	if (config == 0) {
		// Clip all
	}
	else if (config == 1) { // V1 clip V2 V3 V4
		n = 3;
		L1 = -L1.z * L0 + L0.z * L1;
		L2 = -L3.z * L0 + L0.z * L3;
	}
	else if (config == 2) { // V2 clip V1 V3 V4
		n = 3;
		L0 = -L0.z * L1 + L1.z * L0;
		L2 = -L2.z * L1 + L1.z * L2;
	}
	else if (config == 3) { // V1 V2 clip V3 V4
		n = 4;
		L2 = -L2.z * L1 + L1.z * L2;
		L3 = -L3.z * L0 + L0.z * L3;
	}
	else if (config == 4) { // V3 clip V1 V2 V4
		n = 3;
		L0 = -L3.z * L2 + L2.z * L3;
		L1 = -L1.z * L2 + L2.z * L1;
	}
	else if (config == 5) { // V1 V3 clip V2 V4) impossible
		n = 0;
	}
	else if (config == 6) { // V2 V3 clip V1 V4
		n = 4;
		L0 = -L0.z * L1 + L1.z * L0;
		L3 = -L3.z * L2 + L2.z * L3;
	}
	else if (config == 7) { // V1 V2 V3 clip V4
		n = 5;
		L4 = -L3.z * L0 + L0.z * L3;
		L3 = -L3.z * L2 + L2.z * L3;
	}
	else if (config == 8) { // V4 clip V1 V2 V3
		n = 3;
		L0 = -L0.z * L3 + L3.z * L0;
		L1 = -L2.z * L3 + L3.z * L2;
		L2 =  L3;
	}
	else if (config == 9) { // V1 V4 clip V2 V3
		n = 4;
		L1 = -L1.z * L0 + L0.z * L1;
		L2 = -L2.z * L3 + L3.z * L2;
	}
	else if (config == 10) { // V2 V4 clip V1 V3) impossible
		n = 0;
	}
	else if (config == 11) { // V1 V2 V4 clip V3
		n = 5;
		L4 = L3;
		L3 = -L2.z * L3 + L3.z * L2;
		L2 = -L2.z * L1 + L1.z * L2;
	}
	else if (config == 12) { // V3 V4 clip V1 V2
		n = 4;
		L1 = -L1.z * L2 + L2.z * L1;
		L0 = -L0.z * L3 + L3.z * L0;
	}
	else if (config == 13) { // V1 V3 V4 clip V2
		n = 5;
		L4 = L3;
		L3 = L2;
		L2 = -L1.z * L2 + L2.z * L1;
		L1 = -L1.z * L0 + L0.z * L1;
	}
	else if (config == 14) { // V2 V3 V4 clip V1
		n = 5;
		L4 = -L0.z * L3 + L3.z * L0;
		L0 = -L0.z * L1 + L1.z * L0;
	}
	else if (config == 15) { // V1 V2 V3 V4
		n = 4;
	}

	if (n == 0) return 0.0;
	if (n == 3) L3 = L0;
	if (n == 4) L4 = L0;

	// Project onto sphere
	L0 = normalize(L0);
	L1 = normalize(L1);
	L2 = normalize(L2);
	L3 = normalize(L3);
	L4 = normalize(L4);

	// Integrate
	float sum = 0.0;

	sum += integrate_edge(L0, L1);
	sum += integrate_edge(L1, L2);
	sum += integrate_edge(L2, L3);

	if (n >= 4) sum += integrate_edge(L3, L4);
	if (n == 5) sum += integrate_edge(L4, L0);

	return max(0.0, -sum);
}

vec3 sample_light(const vec3 p, const vec3 n, const vec3 v, const float dotnv,
	const vec3 albedo, const float rough, const vec3 f0, const float occ
	) {
	float theta = acos(dotnv);
	vec2 tuv = vec2(rough, theta / (0.5 * PI));
	tuv = tuv * LUT_SCALE + LUT_BIAS;
	vec4 t = textureLod(sltc_mat, tuv, 0.0);
	mat3 inv = mat3(
		vec3(1.0, 0.0, t.y),
		vec3(0.0, t.z, 0.0),
		vec3(t.w, 0.0, t.x));
	float ltcspec = ltc_evaluate(n, v, dotnv, p, inv, light_area0, light_area1, light_area2, light_area3);
	ltcspec *= textureLod(sltc_mag, tuv, 0.0).a;

	mat3 m1 = mat3(
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0));
	float ltcdiff = ltc_evaluate(n, v, dotnv, p, m1, light_area0, light_area1, light_area2, light_area3);
	vec3 direct = albedo * ltcdiff + ltcspec * 0.05;
	direct *= vec3(1000.0, 1000.0, 1000.0);
	return direct;
}

void main() {
	vec4 g0 = textureLod(gbuffer0, tex_coord, 0.0); // Normal.xy, roughness, metallic/matid

	vec3 n;
	n.z = 1.0 - abs(g0.x) - abs(g0.y);
	n.xy = n.z >= 0.0 ? g0.xy : octahedron_wrap(g0.xy);
	n = normalize(n);

	float roughness = g0.b;
	float metallic;
	uint matid;
	unpack_f32_i16(g0.a, metallic, matid);
	vec4 g1 = textureLod(gbuffer1, tex_coord, 0.0); // Basecolor.rgb, occ
	float occ = g1.a;
	vec3 albedo = surface_albedo(g1.rgb, metallic); // g1.rgb - basecolor
	vec3 f0 = surface_f0(g1.rgb, metallic);

	float depth = textureLod(gbufferD, tex_coord, 0.0).r * 2.0 - 1.0;
	vec3 p = get_pos(eye, eye_look, normalize(view_ray), depth, camera_proj);
	vec3 v = normalize(eye - p);
	float dotnv = max(0.0, dot(n, v));

	occ = mix(1.0, occ, dotnv); // AO Fresnel

	vec2 env_brdf = texelFetch(senvmap_brdf, ivec2(vec2(roughness, 1.0 - dotnv) * 256.0), 0).xy;

	// Envmap
	vec3 envl = sh_irradiance(vec3(n.x * envmap_data.z - n.y * envmap_data.y, n.x * envmap_data.y + n.y * envmap_data.z, n.z), shirr);
	envl /= PI;

	vec3 reflection_world = reflect(-v, n);
	float lod = mip_from_roughness(roughness, float(envmap_num_mipmaps));
	vec3 prefiltered_color = textureLod(senvmap_radiance, envmap_equirect(reflection_world, envmap_data.x), lod).rgb;

	envl.rgb *= albedo;

	// Indirect specular
	envl.rgb += prefiltered_color * (f0 * env_brdf.x + env_brdf.y) * 1.5;

	envl.rgb *= envmap_data.w * occ;

	frag_color.rgb = envl;
	frag_color.rgb *= textureLod(ssaotex, tex_coord, 0.0).r;

	if (matid == uint(1)) { // Emission
		frag_color.rgb += g1.rgb; // materialid
		albedo = vec3(0.0, 0.0, 0.0);
	}

	frag_color.rgb += sample_light(p, n, v, dotnv, albedo, roughness, f0, occ);
	frag_color.a = 1.0; // Mark as opaque
}
