#version 450

in vec3 vcolor;
out vec4 frag_color;

void main() {
	frag_color = vec4(vcolor, 1.0);
	frag_color.rgb = pow(frag_color.rgb, vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}
