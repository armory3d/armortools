// Geometric Skinning with Approximate Dual Quaternion Blending, Kavan
// Based on https://github.com/tcoppex/aer-engine/blob/master/demos/aura/data/shaders/Skinning.glsl
uniform vec4 skinBones[skinMaxBones * 2];

void getSkinningDualQuat(const ivec4 bone, vec4 weight, out vec4 A, inout vec4 B) {
	// Retrieve the real and dual part of the dual-quaternions
	ivec4 bonei = bone * 2;
	mat4 matA = mat4(
		skinBones[bonei.x],
		skinBones[bonei.y],
		skinBones[bonei.z],
		skinBones[bonei.w]);
	mat4 matB = mat4(
		skinBones[bonei.x + 1],
		skinBones[bonei.y + 1],
		skinBones[bonei.z + 1],
		skinBones[bonei.w + 1]);
	// Handles antipodality by sticking joints in the same neighbourhood
	// weight.xyz *= sign(matA[3] * mat3x4(matA)).xyz;
	weight.xyz *= sign(matA[3] * matA).xyz;
	// Apply weights
	A = matA * weight; // Real part
	B = matB * weight; // Dual part
	// Normalize
	float invNormA = 1.0 / length(A);
	A *= invNormA;
	B *= invNormA;
}
