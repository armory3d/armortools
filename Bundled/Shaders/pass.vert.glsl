#version 450

#include "compiled.inc"

in vec2 pos;

out vec2 texCoord;

void main() {
	// Scale vertex attribute to 0-1 range
	const vec2 madd = vec2(0.5, 0.5);
	texCoord = pos.xy * madd + madd;
	#ifdef HLSL
	texCoord.y = 1.0 - texCoord.y;
	#endif

	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
