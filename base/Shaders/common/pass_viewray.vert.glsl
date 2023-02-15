#version 450

uniform mat4 invVP;
uniform vec3 eye;

in vec2 pos;

out vec2 texCoord;
out vec3 viewRay;

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	texCoord = pos.xy * madd + madd;
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	texCoord.y = 1.0 - texCoord.y;
	#endif

	gl_Position = vec4(pos.xy, 0.0, 1.0);

	// NDC (at the back of cube)
	vec4 v = vec4(pos.x, pos.y, 1.0, 1.0);
	v = vec4(invVP * v);
	v.xyz /= v.w;
	viewRay = v.xyz - eye;
}
