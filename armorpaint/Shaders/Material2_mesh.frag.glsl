#version 450
#include "../Libraries/armorbase/Shaders/std/gbuffer.glsl"
in vec3 wnormal;
in vec4 wvpposition;
in vec4 prevwvpposition;
out vec4 fragColor[3];
void main() {
vec3 n = normalize(wnormal);
	vec3 basecol;
	float roughness;
	float metallic;
	float occlusion;
	float emission;
	basecol = vec3(0.800000011920929, 0.800000011920929, 0.800000011920929);
	roughness = 0.0;
	metallic = 0.0;
	occlusion = 1.0;
	emission = 0.0;
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);
	uint matid = 0;
	if (emission > 0) { basecol *= emission; matid = 1; }
	fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, matid));
	fragColor[1] = vec4(basecol, occlusion);
	vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;
	vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;
	fragColor[2].rg = vec2(posa - posb);
}
