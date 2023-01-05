// Turn picked color id into mask

#version 330

uniform sampler2D texpaint_colorid; // 1x1 picked color
uniform sampler2D texcolorid;
in vec2 texCoord;
out vec4 FragColor;

void main() {
	vec3 colorid_c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;
	vec3 colorid_c2 = textureLod(texcolorid, texCoord, 0).rgb;
	if (colorid_c1 != colorid_c2) discard;
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
