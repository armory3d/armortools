#version 450

uniform sampler2D tex;
uniform sampler2D gbufferD;
uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform mat4 P;
uniform mat3 V3;
uniform vec2 camera_proj;

#include "std/math.glsl"
#include "std/gbuffer.glsl"

in vec3 view_ray;
in vec2 tex_coord;
out vec4 frag_color;

const float ssr_ray_step = 0.04;
const float ssr_min_ray_step = 0.05;
const float ssr_search_dist = 5.0;
const float ssr_falloff_exp = 5.0;
const float ssr_jitter = 0.6;
const int num_binary_search_steps = 7;
const int max_steps = 18;

vec3 hit_coord;
float depth;

vec2 get_projected_coord(const vec3 hit) {
	vec4 projected_coord = mul(vec4(hit, 1.0), P);
	projected_coord.xy /= projected_coord.w;
	projected_coord.xy = projected_coord.xy * 0.5 + 0.5;
#ifdef GLSL
#else
	projected_coord.y = 1.0 - projected_coord.y;
#endif
	return projected_coord.xy;
}

float get_delta_depth(const vec3 hit) {
	depth = textureLod(gbufferD, get_projected_coord(hit), 0.0).r * 2.0 - 1.0;
	vec3 view_pos = get_pos_view(view_ray, depth, camera_proj);
	return view_pos.z - hit.z;
}

vec4 binary_search(vec3 dir) {
	float ddepth;
	vec3 start = hit_coord;
	for (int i = 0; i < num_binary_search_steps; i++) {
		dir *= 0.5;
		hit_coord -= dir;
		ddepth = get_delta_depth(hit_coord);
		if (ddepth < 0.0) hit_coord += dir;
	}
	// Ugly discard of hits too far away
	if (abs(ddepth) > ssr_search_dist / 500) return vec4(0.0, 0.0, 0.0, 0.0);
	return vec4(get_projected_coord(hit_coord), 0.0, 1.0);
}

vec4 ray_cast(vec3 dir) {
	dir *= ssr_ray_step;
	for (int i = 0; i < max_steps; i++) {
		hit_coord += dir;
		if (get_delta_depth(hit_coord) > 0.0) return binary_search(dir);
	}
	return vec4(0.0, 0.0, 0.0, 0.0);
}

void main() {
	vec4 g0 = textureLod(gbuffer0, tex_coord, 0.0);
	float roughness = g0.b;
	if (roughness == 1.0) {
		frag_color.rgb = vec3(0.0, 0.0, 0.0);
		return;
	}

	float d = textureLod(gbufferD, tex_coord, 0.0).r * 2.0 - 1.0;
	if (d == 1.0) {
		frag_color.rgb = vec3(0.0, 0.0, 0.0);
		return;
	}

	vec2 enc = g0.rg;
	vec3 n;
	n.z = 1.0 - abs(enc.x) - abs(enc.y);
	n.xy = n.z >= 0.0 ? enc.xy : octahedron_wrap(enc.xy);
	n = normalize(n);

	vec3 view_normal = mul(n, V3);
	vec3 view_pos = get_pos_view(view_ray, d, camera_proj);
	vec3 reflected = normalize(reflect(view_pos, view_normal));
	hit_coord = view_pos;

	vec3 dir = reflected * (1.0 - rand(tex_coord) * ssr_jitter * roughness) * 2.0;
	// * max(ssr_min_ray_step, -view_pos.z)
	vec4 coords = ray_cast(dir);

	vec2 delta_coords = abs(vec2(0.5, 0.5) - coords.xy);
	float screen_edge_factor = clamp(1.0 - (delta_coords.x + delta_coords.y), 0.0, 1.0);

	float reflectivity = 1.0 - roughness;
	float intensity = pow(reflectivity, ssr_falloff_exp) *
		screen_edge_factor *
		clamp(-reflected.z, 0.0, 1.0) *
		clamp((ssr_search_dist - length(view_pos - hit_coord)) *
		(1.0 / ssr_search_dist), 0.0, 1.0) *
		coords.w;

	intensity = clamp(intensity, 0.0, 1.0);
	vec3 reflcol = textureLod(tex, coords.xy, 0.0).rgb;
	reflcol = clamp(reflcol, 0.0, 1.0);
	frag_color.rgb = reflcol * intensity * 0.5;
}
