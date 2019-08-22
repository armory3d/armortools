#version 450
in vec3 vcolor;
in vec3 wnormal;
out vec4 fragColor;
void main() {
	vec3 n = normalize(wnormal);
	vec3 basecol;
	float roughness;
	float metallic;
	float occlusion;
	float specular;
	float emission;
	vec3 Attribute_Color_res = vcolor;
	basecol = Attribute_Color_res;
	roughness = 0.0;
	metallic = 0.0;
	occlusion = 1.0;
	specular = 0.0;
	emission = 0.0;
	fragColor = vec4(basecol, 1.0);
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
}
