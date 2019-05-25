#ifndef _COMPILED_GLSL_
#define _COMPILED_GLSL_
#define _EnvTex
#define _EnvStr
#define _Irr
#define _EnvStr
#define _MicroShadowing
#define _Deferred
#define _VoxelCones5
#define _VoxelShadow
#define _LTC
#define _SinglePoint
#define _SMAA
#define _Veloc
#define _SSAO
#define _gbuffer2
#define _CGrainStatic
#define _Emission
#define _Brdf
#define _Irr
#define _Rad
#define _CToneFilmic
#define _CVignette
const float PI = 3.1415926535;
const float PI2 = PI * 2.0;
const vec2 shadowmapSize = vec2(0, 0);
const float shadowmapCubePcfSize = 0.001;
const int shadowmapCascades = 1;
const float ssaoRadius = 1.0;
const float ssaoStrength = 1.0;
const float ssaoScale = 20.0;
const int ssgiMaxSteps = 8;
const float ssgiRayStep = 0.005 * 2.0;
const float ssgiStrength = 1.0;
const float bloomThreshold = 1.5;
const float bloomStrength = 3.5;
const float bloomRadius = 3.0;
const float ssrRayStep = 0.04;
const float ssrMinRayStep = 0.05;
const float ssrSearchDist = 5.0;
const float ssrFalloffExp = 5.0;
const float ssrJitter = 0.6;
const float compoVignetteStrength = 0.4;
const ivec3 voxelgiResolution = ivec3(256, 256, 256);
const vec3 voxelgiHalfExtents = vec3(1, 1, 1);
const float voxelgiOcc = 1.0;
const float voxelgiStep = 1.0;
const float voxelgiRange = 2.0;
const float voxelgiOffset = 1.5;
const float voxelgiAperture = 1.2;
const int skinMaxBones = 50;

#endif // _COMPILED_GLSL_
