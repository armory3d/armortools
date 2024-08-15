#version 450

uniform sampler2D tex;
uniform int channel;

in vec2 tex_coord;
in vec4 color;
out vec4 frag_color;

void main() {
	if (channel == 1) {
		frag_color = textureLod(tex, tex_coord, 0).rrra * color;
	}
	else if (channel == 2) {
		frag_color = textureLod(tex, tex_coord, 0).ggga * color;
	}
	else if (channel == 3) {
		frag_color = textureLod(tex, tex_coord, 0).bbba * color;
	}
	else if (channel == 4) {
		frag_color = textureLod(tex, tex_coord, 0).aaaa * color;
	}
	else if (channel == 5) {
		frag_color = textureLod(tex, tex_coord, 0).rgba * color;
	}
	else {
		vec4 tex_sample = textureLod(tex, tex_coord, 0).rgba;
		tex_sample.rgb *= tex_sample.a;
		frag_color = tex_sample * color;
	}
}
