#version 450
#extension GL_ARB_shader_image_load_store : enable

const int voxelgi_resolution = 256;

uniform layout(r8) writeonly image3D voxels;

in vec3 voxposition;

void main() {
	if (abs(voxposition.z) > 1.0 || abs(voxposition.x) > 1.0 || abs(voxposition.y) > 1.0) return;
	imageStore(voxels, ivec3(voxelgi_resolution * (voxposition * 0.5 + 0.5)), vec4(1.0));
}
