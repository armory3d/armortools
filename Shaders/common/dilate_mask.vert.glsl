#version 450

in vec4 pos;
in vec2 nor;
in vec2 tex;
// uniform float texUnpack;
void main() {
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	vec2 texCoord = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);
	#else
	vec2 texCoord = tex * 2.0 - 1.0;
	#endif
	gl_Position = vec4(texCoord, 0.0, 1.0);
	#ifdef HLSL
	float keep = pos.x + nor.x;
	#endif
}
