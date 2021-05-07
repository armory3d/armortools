
#ifndef _MATH_GLSL_
#define _MATH_GLSL_

const float PI = 3.1415926535;
const float PI2 = PI * 2.0;

vec2 envMapEquirect(const vec3 normal, const float angle) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float phi = acos(normal.z);
	float theta = atan(-normal.y, normal.x) + PI + angle;
	return vec2(theta / PI2, phi / PI);
}

float rand(const vec2 co) { // Unreliable
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float attenuate(const float dist) {
	return 1.0 / (dist * dist);
}

#endif
