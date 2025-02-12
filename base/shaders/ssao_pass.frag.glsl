#version 450

uniform sampler2D gbufferD;
uniform sampler2D gbuffer0;
uniform mat4 P;
uniform mat3 V3;
uniform vec2 camera_proj;

#include "std/math.glsl"
#include "std/gbuffer.glsl"

in vec3 view_ray;
in vec2 tex_coord;
out float frag_color;

const int max_steps = 32;
const float ray_step = 0.001;

vec3 hit_coord;
vec2 coord;
float depth;
vec3 vpos;

vec2 get_projected_coord(vec3 hit_coord) {
	vec4 projected_coord = mul(vec4(hit_coord, 1.0), P);
	projected_coord.xy /= projected_coord.w;
	projected_coord.xy = projected_coord.xy * 0.5 + 0.5;
#ifdef GLSL
#else
	projected_coord.y = 1.0 - projected_coord.y;
#endif
	return projected_coord.xy;
}

float get_delta_depth(vec3 hit_coord) {
	coord = get_projected_coord(hit_coord);
	depth = textureLod(gbufferD, coord, 0.0).r * 2.0 - 1.0;
	vec3 p = get_pos_view(view_ray, depth, camera_proj);
	return p.z - hit_coord.z;
}

void ray_cast(vec3 dir) {
	hit_coord = vpos;
	dir *= ray_step;
	float dist = 0.15;
	for (int i = 0; i < max_steps; i++) {
		hit_coord += dir;
		float delta = get_delta_depth(hit_coord);
		if (delta > 0.0 && delta < 0.2) {
			dist = distance(vpos, hit_coord);
			break;
		}
	}
	frag_color += dist;
}

vec3 tangent(const vec3 n) {
	vec3 t1 = cross(n, vec3(0, 0, 1));
	vec3 t2 = cross(n, vec3(0, 1, 0));
	if (length(t1) > length(t2)) return normalize(t1);
	else return normalize(t2);
}

void main() {
	frag_color = 0;
	vec4 g0 = textureLod(gbuffer0, tex_coord, 0.0);
	float d = textureLod(gbufferD, tex_coord, 0.0).r * 2.0 - 1.0;

	vec2 enc = g0.rg;
	vec3 n;
	n.z = 1.0 - abs(enc.x) - abs(enc.y);
	n.xy = n.z >= 0.0 ? enc.xy : octahedron_wrap(enc.xy);
	n = normalize(mul(n, V3));

	vpos = get_pos_view(view_ray, d, camera_proj);

	ray_cast(n);
	vec3 o1 = normalize(tangent(n));
	vec3 o2 = (cross(o1, n));
	vec3 c1 = 0.5f * (o1 + o2);
	vec3 c2 = 0.5f * (o1 - o2);
	ray_cast(mix(n, o1, 0.5));
	ray_cast(mix(n, o2, 0.5));
	ray_cast(mix(n, -c1, 0.5));
	ray_cast(mix(n, -c2, 0.5));
}
