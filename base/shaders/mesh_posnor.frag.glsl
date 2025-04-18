#version 450

#include "../../base/shaders/std/gbuffer.glsl"

in vec3 wnormal;
in vec4 wvpposition;
in vec4 prevwvpposition;
out vec4 frag_color[3];

void main() {
	vec3 n = normalize(wnormal);
	vec3 basecol = vec3(0.8, 0.8, 0.8);
	float roughness = 0.25;
	float metallic = 0.0;
	float occlusion = 1.0;
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);
	uint matid = uint(0);
	frag_color[0] = vec4(n.xy, roughness, pack_f32_i16(metallic, matid));
	frag_color[1] = vec4(basecol, occlusion);
	vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;
	vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;
	frag_color[2].rg = vec2(posa - posb);
}
