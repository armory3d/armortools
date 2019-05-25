#version 450

#include "compiled.inc"

// #ifdef _CPos
	// uniform mat4 invVP;
	// uniform vec3 eye;
// #endif

in vec2 pos;

out vec2 texCoord;
// #ifdef _CPos
	// out vec3 viewRay;
// #endif

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	texCoord = pos.xy * madd + madd;
	#ifdef HLSL
	texCoord.y = 1.0 - texCoord.y;
	#endif

	gl_Position = vec4(pos.xy, 0.0, 1.0);

// #ifdef _CPos
	// NDC (at the back of cube)
	// vec4 v = vec4(pos.xy, 1.0, 1.0);	
	// v = vec4(invVP * v);
	// v.xyz /= v.w;
	// viewRay = v.xyz - eye;
// #endif
}
