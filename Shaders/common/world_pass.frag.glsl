#version 450

#include "../std/math.glsl"

uniform sampler2D envmap;
uniform vec4 envmapDataWorld; // angle, sin(angle), cos(angle), strength

in vec3 normal;
out vec4 fragColor;

void main() {
	vec4 envmapDataLocal = envmapDataWorld; // TODO: SPIRV workaround
	vec3 n = normalize(normal);
	fragColor.rgb = texture(envmap, envMapEquirect(n, envmapDataLocal.x)).rgb * envmapDataLocal.w;
	fragColor.a = 0.0; // Mark as non-opaque
}
