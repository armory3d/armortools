#version 450

uniform sampler2D tex;

in vec2 tex_coord;
in vec4 color;
out vec4 frag_color;

void main() {
	vec4 texcolor = texture(tex, tex_coord) * color;
	texcolor.rgb = texcolor.rgb * texcolor.a * color.a;
	frag_color = texcolor;
}
