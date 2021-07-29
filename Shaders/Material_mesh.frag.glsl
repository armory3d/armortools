#version 450
#include "../Libraries/armorbase/Shaders/std/gbuffer.glsl"
in vec2 texCoord;
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
	float Mix_fac = 0.0;
	vec3 RGB_Color_res = vec3(0.2176000028848648, 0.2176000028848648, 0.2176000028848648);
	vec3 Mix_Color_res = RGB_Color_res + texCoord.x * 0.0000001; // keep texCoord
	basecol = Mix_Color_res;
	roughness = 0.4000000059604645;
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
