#version 450

#include "../std/conetrace.glsl"

uniform mat4 W;

in vec4 pos;
out vec3 voxpositionGeom;

void main() {
	voxpositionGeom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgiHalfExtents;
}
