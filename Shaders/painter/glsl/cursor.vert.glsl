#version 330
uniform mat4 VP;
uniform mat4 invVP;
uniform vec2 mouse;
uniform vec2 step;
uniform float radius;
uniform sampler2D gbufferD;
uniform sampler2D gbuffer0;
in vec4 pos;
in vec2 nor;
in vec2 tex;
out vec2 texCoord;
vec2 octahedronWrap(const vec2 v) { return (1.0 - abs(v.yx)) * (vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0)); }
mat3 rotAxis(vec3 axis, float a) {
	float c = cos(a);
	vec3 as = axis * sin(a);
	mat3 p = mat3(axis.x * axis, axis.y * axis, axis.z * axis);
	mat3 q = mat3(c, -as.z, as.y, as.z, c, -as.x, -as.y, as.x, c);
	return p * (1.0 - c) + q;
}
vec3 getNormal(vec2 uv) {
	vec2 g0 = textureLod(gbuffer0, mouse, 0.0).rg;
	vec3 n;
	n.z = 1.0 - abs(g0.x) - abs(g0.y);
	n.xy = n.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);
	return n;
}
void main() {
	texCoord = tex;
	float depth = textureLod(gbufferD, mouse, 0.0).r;
	vec4 wpos = vec4(mouse * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	wpos = invVP * wpos;
	wpos.xyz /= wpos.w;
	vec3 n = normalize(
		getNormal(mouse + vec2(step.x, step.y)) +
		getNormal(mouse + vec2(-step.x, step.y)) +
		getNormal(mouse + vec2(-step.x, -step.y)) +
		getNormal(mouse + vec2(step.x, -step.y)) +
		getNormal(mouse)
	);
	float ax = acos(dot(vec3(1,0,0), vec3(n.x,0,0)));
	float az = acos(dot(vec3(0,0,1), vec3(0,0,n.z)));
	float sy = -sign(n.y);
	wpos.xyz +=
		rotAxis(vec3(1,0,0), -az * sy + 3.14/2) *
		rotAxis(vec3(0,0,1), ax + 3.14/2) *
		(pos.xyz * radius);
	gl_Position = VP * vec4(wpos.xyz, 1.0);
}
