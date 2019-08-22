#version 450
#include "../std/gbuffer.glsl"
in vec2 texCoord;
in vec3 wnormal;
in vec4 wvpposition;
in vec4 prevwvpposition;
out vec4 fragColor[3];
uniform sampler2D ImageTexture;
void main() {
	vec3 n = normalize(wnormal);
	vec3 basecol;
	float roughness;
	float metallic;
	float occlusion;
	float specular;
	float emission;
	float Mix_fac = 0.0;
	vec3 RGB_Color_res = vec3(0.2176000028848648, 0.2176000028848648, 0.2176000028848648);
	vec4 ImageTexture_store = texture(ImageTexture, texCoord.xy);
	vec3 ImageTexture_Color_res = ImageTexture_store.rgb;
	vec3 Mix_Color_res = mix(RGB_Color_res, ImageTexture_Color_res, Mix_fac);
	basecol = Mix_Color_res;
	roughness = 0.4000000059604645;
	metallic = 0.0;
	occlusion = 1.0;
	specular = 1.0;
	emission = 0.0;
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);
	uint matid = 0;
	if (emission > 0) { basecol *= emission; matid = 1; }
	fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, matid));
	fragColor[1] = vec4(basecol, packFloat2(occlusion, specular));
	vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;
	vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;
	fragColor[2].rg = vec2(posa - posb);
}
