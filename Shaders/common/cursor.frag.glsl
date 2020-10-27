#version 330
uniform vec3 tint;
#ifdef SPIRV
uniform sampler2D gbufferD; // vulkan unit align
#endif
uniform sampler2D tex;
in vec2 texCoord;
out vec4 FragColor;
void main() {
	vec4 col = texture(tex, texCoord);
	FragColor = vec4((col.rgb / col.a) * tint, col.a);
}
