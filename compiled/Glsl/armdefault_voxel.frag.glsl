#version 450
#extension GL_ARB_shader_image_load_store : enable
#include "../Shaders/compiled.inc"
#include "../Shaders/std/math.glsl"
#include "../Shaders/std/imageatomic.glsl"
in vec3 voxposition;
uniform layout(r8) writeonly image3D voxels;
void main() {
	if (abs(voxposition.z) > 1.0 || abs(voxposition.x) > 1 || abs(voxposition.y) > 1) return;
	imageStore(voxels, ivec3(voxelgiResolution * (voxposition * 0.5 + 0.5)), vec4(1.0));
}
