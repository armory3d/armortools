#version 450
#include "../std/conetrace.glsl"
in vec4 pos;
out vec3 voxpositionGeom;
uniform mat4 W;
void main() {
	voxpositionGeom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgiHalfExtents;
}
