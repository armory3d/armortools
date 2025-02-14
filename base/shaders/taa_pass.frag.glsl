#version 450

uniform sampler2D tex;
uniform sampler2D tex2;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	vec4 current = textureLod(tex, tex_coord, 0.0);
	vec4 previous = textureLod(tex2, tex_coord, 0.0);
	frag_color = vec4(mix(current.rgb, previous.rgb, 0.5), 1.0);
}
