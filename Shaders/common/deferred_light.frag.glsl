#version 450

#define _LTC
#define _VoxelShadow
#define _MicroShadowing
#define _SinglePoint
#define _SSAO
#define _Emission
#define _Rad
#define _Irr
#define _Brdf
#define _EnvTex

#include "../std/gbuffer.glsl"
#include "../std/light.glsl"
#include "../std/shirr.glsl"
#ifdef _VoxelAOvar
#include "../std/conetrace.glsl"
#endif

uniform sampler2D gbufferD;
uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;

#ifdef _VoxelAOvar
uniform sampler3D voxels;
#endif

uniform vec4 envmapData; // angle, sin(angle), cos(angle), strength
uniform vec4 shirr[7];
uniform sampler2D senvmapBrdf;
uniform sampler2D senvmapRadiance;
#ifdef SPIRV
uniform float envmapNumMipmaps;
#else
uniform int envmapNumMipmaps;
#endif
uniform sampler2D ssaotex;

uniform vec2 cameraProj;
uniform vec3 eye;
uniform vec3 eyeLook;

uniform vec3 pointPos;
uniform vec3 pointCol;

in vec2 texCoord;
in vec3 viewRay;
out vec4 fragColor;

void main() {
	vec4 g0 = textureLod(gbuffer0, texCoord, 0.0); // Normal.xy, roughness, metallic/matid

	vec3 n;
	n.z = 1.0 - abs(g0.x) - abs(g0.y);
	n.xy = n.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);
	n = normalize(n);

	float roughness = g0.b;
	float metallic;
	uint matid;
	unpackFloatInt16(g0.a, metallic, matid);
	vec4 g1 = textureLod(gbuffer1, texCoord, 0.0); // Basecolor.rgb, occ
	float occ = g1.a;
	vec3 albedo = surfaceAlbedo(g1.rgb, metallic); // g1.rgb - basecolor
	vec3 f0 = surfaceF0(g1.rgb, metallic);

	float depth = textureLod(gbufferD, texCoord, 0.0).r * 2.0 - 1.0;
	vec3 p = getPos(eye, eyeLook, normalize(viewRay), depth, cameraProj);
	vec3 v = normalize(eye - p);
	float dotNV = max(dot(n, v), 0.0);

	occ = mix(1.0, occ, dotNV); // AO Fresnel

	vec2 envBRDF = textureLod(senvmapBrdf, vec2(roughness, 1.0 - dotNV), 0.0).xy;

	// Envmap
	vec4 envmapDataLocal = envmapData; // TODO: SPIRV workaround
	vec3 envl = shIrradiance(vec3(n.x * envmapDataLocal.z - n.y * envmapDataLocal.y, n.x * envmapDataLocal.y + n.y * envmapDataLocal.z, n.z), shirr);
	envl /= PI;

	vec3 reflectionWorld = reflect(-v, n);
	float lod = getMipFromRoughness(roughness, envmapNumMipmaps);
	vec3 prefilteredColor = textureLod(senvmapRadiance, envMapEquirect(reflectionWorld, envmapDataLocal.x), lod).rgb;

	envl.rgb *= albedo;

	// Indirect specular
	envl.rgb += prefilteredColor * (f0 * envBRDF.x + envBRDF.y) * 1.5;

	envl.rgb *= envmapDataLocal.w * occ;

#ifdef _VoxelAOvar
	vec3 voxpos = p / voxelgiHalfExtents;
	envl.rgb *= 1.0 - traceAO(voxpos, n, voxels);
#endif

	fragColor.rgb = envl;
	fragColor.rgb *= textureLod(ssaotex, texCoord, 0.0).r;

	if (g0.a == 1.0) { // Emission
		fragColor.rgb += g1.rgb; // materialid
		albedo = vec3(0.0);
	}

	fragColor.rgb += sampleLight(p, n, v, dotNV, pointPos, pointCol, albedo, roughness, f0
		#ifdef _VoxelAOvar
		#ifdef _VoxelShadow
		, voxels, voxpos
		#endif
		#endif
		, occ);
	fragColor.a = 1.0; // Mark as opaque
}
