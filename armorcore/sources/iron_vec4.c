#include "iron_vec4.h"
#include "iron_quat.h"

#include <math.h>
#include <kinc/math/core.h>

vec4_t vec4_create(float x, float y, float z, float w) {
	vec4_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

vec4_t vec4_cross(vec4_t a, vec4_t b) {
	vec4_t v;
	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;
	return v;
}

vec4_t vec4_add(vec4_t a, vec4_t b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

vec4_t vec4_fadd(vec4_t a, float x, float y, float z, float w) {
	a.x += x;
	a.y += y;
	a.z += z;
	a.w += w;
	return a;
}

vec4_t vec4_norm(vec4_t a) {
	float n = vec4_len(a);
	if (n > 0.0) {
		float inv_n = 1.0f / n;
		a.x *= inv_n;
		a.y *= inv_n;
		a.z *= inv_n;
	}
	return a;
}

vec4_t vec4_mult(vec4_t a, float f) {
	a.x *= f;
	a.y *= f;
	a.z *= f;
	a.w *= f;
	return a;
}

float vec4_dot(vec4_t a, vec4_t b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec4_t vec4_clone(vec4_t v) {
	return v;
}

vec4_t vec4_lerp(vec4_t from, vec4_t to, float s) {
	vec4_t v;
	v.x = from.x + (to.x - from.x) * s;
	v.y = from.y + (to.y - from.y) * s;
	v.z = from.z + (to.z - from.z) * s;
	return v;
}

vec4_t vec4_apply_proj(vec4_t a, mat4_t m) {
	vec4_t v;
	float d = 1.0 / (m.m[3] * a.x + m.m[7] * a.y + m.m[11] * a.z + m.m[15]); // Perspective divide
	v.x = (m.m[0] * a.x + m.m[4] * a.y + m.m[8] * a.z + m.m[12]) * d;
	v.y = (m.m[1] * a.x + m.m[5] * a.y + m.m[9] * a.z + m.m[13]) * d;
	v.z = (m.m[2] * a.x + m.m[6] * a.y + m.m[10] * a.z + m.m[14]) * d;
	return v;
}

vec4_t vec4_apply_mat(vec4_t a, mat4_t m) {
	vec4_t v;
	v.x = m.m[0] * a.x + m.m[4] * a.y + m.m[8] * a.z + m.m[12];
	v.y = m.m[1] * a.x + m.m[5] * a.y + m.m[9] * a.z + m.m[13];
	v.z = m.m[2] * a.x + m.m[6] * a.y + m.m[10] * a.z + m.m[14];
	return v;
}

vec4_t vec4_apply_mat4(vec4_t a, mat4_t m) {
	vec4_t v;
	v.x = m.m[0] * a.x + m.m[4] * a.y + m.m[8] * a.z + m.m[12] * a.w;
	v.y = m.m[1] * a.x + m.m[5] * a.y + m.m[9] * a.z + m.m[13] * a.w;
	v.z = m.m[2] * a.x + m.m[6] * a.y + m.m[10] * a.z + m.m[14] * a.w;
	v.w = m.m[3] * a.x + m.m[7] * a.y + m.m[11] * a.z + m.m[15] * a.w;
	return v;
}

vec4_t vec4_apply_axis_angle(vec4_t a, vec4_t axis, float angle) {
	quat_t q = quat_from_axis_angle(axis, angle);
	return vec4_apply_quat(a, q);
}

vec4_t vec4_apply_quat(vec4_t a, quat_t q) {
	vec4_t v;
	float ix = q.w * a.x + q.y * a.z - q.z * a.y;
	float iy = q.w * a.y + q.z * a.x - q.x * a.z;
	float iz = q.w * a.z + q.x * a.y - q.y * a.x;
	float iw = -q.x * a.x - q.y * a.y - q.z * a.z;
	v.x = ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y;
	v.y = iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z;
	v.z = iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x;
	return v;
}

bool vec4_equals(vec4_t a, vec4_t b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool vec4_almost_equals(vec4_t a, vec4_t b, float prec) {
	return fabs(a.x - b.x) < prec && fabs(a.y - b.y) < prec && fabs(a.z - b.z) < prec;
}

float vec4_len(vec4_t a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

vec4_t vec4_sub(vec4_t a, vec4_t b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

vec4_t vec4_exp(vec4_t a) {
	a.x = expf(a.x);
	a.y = expf(a.y);
	a.z = expf(a.z);
	return a;
}

float vec4_dist(vec4_t v1, vec4_t v2) {
	return vec4_fdist(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
}

float vec4_fdist(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z) {
	float vx = v1x - v2x;
	float vy = v1y - v2y;
	float vz = v1z - v2z;
	return sqrtf(vx * vx + vy * vy + vz * vz);
}

vec4_t vec4_reflect(vec4_t a, vec4_t n) {
	float d = 2 * vec4_dot(a, n);
	a.x -= d * n.x;
	a.y -= d * n.y;
	a.z -= d * n.z;
	return a;
}

vec4_t vec4_clamp(vec4_t a, float min, float max) {
	float l = vec4_len(a);
	if (l < min) {
		return vec4_mult(vec4_norm(a), min);
	}
	else if (l > max) {
		return vec4_mult(vec4_norm(a), max);
	}
	return a;
}

vec4_t vec4_x_axis() {
	return vec4_create(1.0, 0.0, 0.0, 1.0);
}

vec4_t vec4_y_axis() {
	return vec4_create(0.0, 1.0, 0.0, 1.0);
}

vec4_t vec4_z_axis() {
	return vec4_create(0.0, 0.0, 1.0, 1.0);
}

vec4_t vec4_nan() {
	vec4_t v;
	v.x = NAN;
	return v;
}

bool vec4_isnan(vec4_t v) {
	return isnan(v.x);
}
