#version 450

const float waterLevel = 0.0;
const float waterDisplace = 1.0;
const float waterSpeed = 1.0;
const float waterFreq = 1.0;
const vec3 waterColor = vec3(1.0, 1.0, 1.0);
const float waterDensity = 1.0;
const float waterRefract = 1.0;
const float waterReflect = 1.0;

#include "../std/gbuffer.glsl"
#include "../std/math.glsl"

uniform sampler2D gbufferD;
uniform sampler2D tex;
uniform sampler2D sbase;
uniform sampler2D sdetail;
uniform sampler2D sfoam;
uniform sampler2D senvmapRadiance;

uniform float time;
uniform vec3 eye;
uniform vec3 eyeLook;
uniform vec2 cameraProj;
uniform vec3 ld;
uniform float envmapStrength;

in vec2 texCoord;
in vec3 viewRay;
out vec4 fragColor;

void main() {

	float gdepth = textureLod(gbufferD, texCoord, 0.0).r * 2.0 - 1.0;
	if (gdepth == 1.0) {
		fragColor = vec4(0.0);
		return;
	}

	// Eye below water
	if (eye.z < waterLevel) {
		fragColor = vec4(0.0);
		return;
	}

	// Displace surface
	vec3 vray = normalize(viewRay);
	vec3 p = getPos(eye, eyeLook, vray, gdepth, cameraProj);
	float speed = time * 2.0 * waterSpeed;
	p.z += sin(p.x * 10.0 / waterDisplace + speed) * cos(p.y * 10.0 / waterDisplace + speed) / 50.0 * waterDisplace;

	// Above water
	if (p.z > waterLevel) {
		fragColor = vec4(0.0);
		return;
	}

	// Hit plane to determine uvs
	vec3 v = normalize(eye - p.xyz);
	float t = -(dot(eye, vec3(0.0, 0.0, 1.0)) - waterLevel) / dot(v, vec3(0.0, 0.0, 1.0));
	vec3 hit = eye + t * v;
	hit.xy *= waterFreq;
	hit.z += waterLevel;

	// Sample normal maps
	vec2 tcnor0 = hit.xy / 3.0;
	vec3 n0 = textureLod(sdetail, tcnor0 + vec2(speed / 60.0, speed / 120.0), 0.0).rgb;

	vec2 tcnor1 = hit.xy / 6.0 + n0.xy / 20.0;
	vec3 n1 = textureLod(sbase, tcnor1 + vec2(speed / 40.0, speed / 80.0), 0.0).rgb;
	vec3 n2 = normalize(((n1 + n0) / 2.0) * 2.0 - 1.0);

	float ddepth = textureLod(gbufferD, texCoord + (n2.xy * n2.z) / 40.0, 0.0).r * 2.0 - 1.0;
	vec3 p2 = getPos(eye, eyeLook, vray, ddepth, cameraProj);
	vec2 tc = p2.z > waterLevel ? texCoord : texCoord + (n2.xy * n2.z) / 30.0 * waterRefract;

	// Light
	float fresnel = 1.0 - max(dot(n2, v), 0.0);
	fresnel = pow(fresnel, 30.0) * 0.45;
	vec3 r = reflect(-v, n2);
	vec3 reflected =  textureLod(senvmapRadiance, envMapEquirect(r, 0.0), 0).rgb;
	vec3 refracted = textureLod(tex, tc, 0.0).rgb;
	fragColor.rgb = mix(refracted, reflected, fresnel * waterReflect);
	fragColor.rgb *= waterColor;
	fragColor.rgb += clamp(pow(max(dot(r, ld), 0.0), 200.0) * (200.0 + 8.0) / (PI * 8.0), 0.0, 2.0);
	fragColor.rgb *= 1.0 - (clamp(-(p.z - waterLevel) * waterDensity, 0.0, 0.9));
	fragColor.a = clamp(abs(p.z - waterLevel) * 5.0, 0.0, 1.0);

	// Foam
	float fd = abs(p.z - waterLevel);
	if (fd < 0.1) {
		// Based on foam by Owen Deery
		// http://fire-face.com/personal/water
		vec3 foamMask0 = textureLod(sfoam, tcnor0 * 10, 0.0).rgb;
		vec3 foamMask1 = textureLod(sfoam, tcnor1 * 11, 0.0).rgb;
		vec3 foam = vec3(1.0) - foamMask0.rrr - foamMask1.bbb;
		float fac = 1.0 - (fd * (1.0 / 0.1));
		fragColor.rgb = mix(fragColor.rgb, clamp(foam, 0.0, 1.0), clamp(fac, 0.0, 1.0));
	}
}
