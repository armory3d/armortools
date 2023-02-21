#version 450

#include "std/math.glsl"
#include "std/gbuffer.glsl"

uniform sampler2D gbufferD;
uniform sampler2D gbuffer0; // Normal
uniform mat4 P;
uniform mat3 V3;
uniform vec2 cameraProj;

in vec3 viewRay;
in vec2 texCoord;
out float fragColor;

const int maxSteps = 8;
const float rayStep = 0.01;
const float angleMix = 0.5;
const float strength = 3.6;

vec3 hitCoord;
vec2 coord;
float depth;
vec3 vpos;

vec2 getProjectedCoord(vec3 hitCoord) {
	vec4 projectedCoord = P * vec4(hitCoord, 1.0);
	projectedCoord.xy /= projectedCoord.w;
	projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	projectedCoord.y = 1.0 - projectedCoord.y;
	#endif
	return projectedCoord.xy;
}

float getDeltaDepth(vec3 hitCoord) {
	coord = getProjectedCoord(hitCoord);
	depth = textureLod(gbufferD, coord, 0.0).r * 2.0 - 1.0;
	vec3 p = getPosView(viewRay, depth, cameraProj);
	return p.z - hitCoord.z;
}

void rayCast(vec3 dir) {
	hitCoord = vpos;
	dir *= rayStep * 2;
	float dist = 0.15;
	for (int i = 0; i < maxSteps; i++) {
		hitCoord += dir;
		float delta = getDeltaDepth(hitCoord);
		if (delta > 0.0 && delta < 0.2) {
			dist = distance(vpos, hitCoord);
			break;
		}
	}
	fragColor += dist;
}

vec3 tangent(const vec3 n) {
	vec3 t1 = cross(n, vec3(0, 0, 1));
	vec3 t2 = cross(n, vec3(0, 1, 0));
	if (length(t1) > length(t2)) return normalize(t1);
	else return normalize(t2);
}

void main() {
	fragColor = 0;
	vec4 g0 = textureLod(gbuffer0, texCoord, 0.0);
	float d = textureLod(gbufferD, texCoord, 0.0).r * 2.0 - 1.0;

	vec2 enc = g0.rg;
	vec3 n;
	n.z = 1.0 - abs(enc.x) - abs(enc.y);
	n.xy = n.z >= 0.0 ? enc.xy : octahedronWrap(enc.xy);
	n = normalize(V3 * n);

	vpos = getPosView(viewRay, d, cameraProj);

	rayCast(n);
	vec3 o1 = normalize(tangent(n));
	vec3 o2 = (cross(o1, n));
	vec3 c1 = 0.5f * (o1 + o2);
	vec3 c2 = 0.5f * (o1 - o2);
	rayCast(mix(n, o1, angleMix));
	rayCast(mix(n, o2, angleMix));
	rayCast(mix(n, -c1, angleMix));
	rayCast(mix(n, -c2, angleMix));
}
