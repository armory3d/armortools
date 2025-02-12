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

#include "std/gbuffer.glsl"
#include "std/brdf.glsl"
#include "std/math.glsl"
#include "std/shirr.glsl"

in vec2 tex_coord;
in vec3 view_ray;
out vec4 frag_color;

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

	// Envmap
	vec3 envl = sh_irradiance(vec3(n.x * envmap_data.z - n.y * envmap_data.y, n.x * envmap_data.y + n.y * envmap_data.z, n.z), shirr);
	envl /= PI;

	vec3 reflection_world = reflect(-v, n);
	float lod = mip_from_roughness(roughness, float(envmap_num_mipmaps));
	vec3 prefiltered_color = textureLod(senvmap_radiance, envmap_equirect(reflection_world, envmap_data.x), lod).rgb;

	envl.rgb *= albedo;
	// Indirect specular
	vec2 env_brdf = texelFetch(senvmap_brdf, ivec2(vec2(roughness, 1.0 - dotnv) * 256.0), 0).xy;
	envl.rgb += prefiltered_color * (f0 * env_brdf.x + env_brdf.y) * 1.5;
	envl.rgb *= envmap_data.w * occ;
	frag_color.rgb = envl;
	frag_color.rgb *= textureLod(ssaotex, tex_coord, 0.0).r;

	if (matid == uint(1)) { // Emission
		frag_color.rgb += g1.rgb; // materialid
		albedo = vec3(0.0, 0.0, 0.0);
	}

	frag_color.a = 1.0; // Mark as opaque
}
