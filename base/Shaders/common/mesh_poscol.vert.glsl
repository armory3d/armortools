#version 450

uniform mat4 WVP;

in vec4 pos;
in vec4 col;
out vec3 vcolor;

void main() {
	vec4 spos = vec4(pos.xyz, 1.0);
	vcolor = col.rgb;
	gl_Position = WVP * spos;
}
