#version 450

// Turn picked color id into mask

uniform sampler2D texpaint_colorid; // 1x1 picked color
uniform sampler2D texcolorid;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	vec3 colorid_c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;
	vec3 colorid_c2 = textureLod(texcolorid, tex_coord, 0.0).rgb;
	if (colorid_c1.x != colorid_c2.x || colorid_c1.y != colorid_c2.y || colorid_c1.z != colorid_c2.z) discard;
	frag_color = vec4(1.0, 1.0, 1.0, 1.0);
}
