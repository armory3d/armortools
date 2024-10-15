#version 450

const vec3 voxelgi_half_extents = vec3(1, 1, 1);

uniform mat4 W;

in vec4 pos;
out vec3 voxposition_geom;

void main() {
	voxposition_geom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgi_half_extents;
}
