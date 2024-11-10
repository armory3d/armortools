#include "iron_vec2.h"

#include <math.h>
#include <kinc/math/core.h>

vec2_t vec2_create(float x, float y) {
	vec2_t v;
	v.x = x;
	v.y = y;
	return v;
}

float vec2_len(vec2_t v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}

vec2_t vec2_set_len(vec2_t v, float length) {
	float current_length = vec2_len(v);
	if (current_length == 0) {
		return v;
	}
	float mul = length / current_length;
	v.x *= mul;
	v.y *= mul;
	return v;
}

vec2_t vec2_mult(vec2_t v, float f) {
	v.x *= f;
	v.y *= f;
	return v;
}

vec2_t vec2_add(vec2_t a, vec2_t b) {
	a.x += b.x;
	a.y += b.y;
	return a;
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

float vec2_cross(vec2_t a, vec2_t b) {
	return a.x * b.y - a.y * b.x;
}

vec2_t vec2_norm(vec2_t v) {
	float n = vec2_len(v);
	if (n > 0.0) {
		float inv_n = 1.0f / n;
		v.x *= inv_n;
		v.y *= inv_n;
	}
	return v;
}

float vec2_dot(vec2_t a, vec2_t b) {
	return a.x * b.x + a.y * b.y;
}

vec2_t vec2_nan() {
	vec2_t v;
	v.x = NAN;
	return v;
}

bool vec2_isnan(vec2_t v) {
	return isnan(v.x);
}
