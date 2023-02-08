#version 450
in vec4 pos;
in vec2 nor;
in vec4 col;
out vec3 vcolor;
out vec3 wnormal;
uniform mat3 N;
uniform mat4 WVP;
void main() {
	vec4 spos = vec4(pos.xyz, 1.0);
	vcolor = col.rgb;
	wnormal = normalize(N * vec3(nor.xy, pos.w));
	gl_Position = WVP * spos;
}
