#version 330
in vec2 pos;
out vec2 texCoord;
void main() {
	gl_Position = vec4(pos.xy, 0.0, 1.0);
	const vec2 madd = vec2(0.5, 0.5);
	texCoord = pos.xy * madd + madd;
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	texCoord.y = 1.0 - texCoord.y;
	#endif
}
