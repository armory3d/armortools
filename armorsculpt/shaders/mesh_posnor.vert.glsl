#version 450

uniform mat3 N;
uniform mat4 WVP;
uniform mat4 prevWVP;

in vec4 pos;
in vec2 nor;
out vec3 wnormal;
out vec4 wvpposition;
out vec4 prevwvpposition;

void main() {
	vec4 spos = vec4(pos.xyz, 1.0);
	wnormal = normalize(mul(vec3(nor.xy, pos.w), N));
	gl_Position = mul(spos, WVP);
	wvpposition = gl_Position;
	prevwvpposition = mul(spos, prevWVP);
}
