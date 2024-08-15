#version 450

#include "../std/conetrace.glsl"

uniform mat4 W;

in vec4 pos;
out vec3 voxposition_geom;

void main() {
	voxposition_geom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgi_half_extents;
}
