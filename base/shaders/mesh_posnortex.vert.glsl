#version 450

uniform mat3 N;
uniform mat4 WVP;
uniform float tex_unpack;
uniform mat4 prevWVP;

in vec4 pos;
in vec2 nor;
in vec2 tex;
out vec2 tex_coord;
out vec3 wnormal;
out vec4 wvpposition;
out vec4 prevwvpposition;

void main() {
	vec4 spos = vec4(pos.xyz, 1.0);
	tex_coord = tex * tex_unpack;
	wnormal = normalize(N * vec3(nor.xy, pos.w));
	gl_Position = WVP * spos;
	wvpposition = gl_Position;
	prevwvpposition = prevWVP * spos;
}
