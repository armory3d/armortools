
#include "iron_quat.h"
#include <math.h>

#define MATH_PI 3.14159265358979323846

quat_t quat_create(float x, float y, float z, float w) {
	quat_t q;
	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;
	return q;
}

quat_t quat_from_axis_angle(vec4_t axis, float angle) {
	float s = sinf(angle * 0.5);
    quat_t q;
	q.x = axis.x * s;
	q.y = axis.y * s;
	q.z = axis.z * s;
	q.w = cosf(angle * 0.5);
	return quat_norm(q);
}

quat_t quat_from_mat(mat4_t m) {
	m = mat4_to_rot(m);
	return quat_from_rot_mat(m);
}

quat_t quat_from_rot_mat(mat4_t m) {
	// Assumes the upper 3x3 is a pure rotation matrix
	float m11 = m.m[0];
	float m12 = m.m[4];
	float m13 = m.m[8];
	float m21 = m.m[1];
	float m22 = m.m[5];
	float m23 = m.m[9];
	float m31 = m.m[2];
	float m32 = m.m[6];
	float m33 = m.m[10];
	float tr = m11 + m22 + m33;
	float s = 0.0;
    quat_t q;

	if (tr > 0) {
		s = 0.5 / sqrtf(tr + 1.0);
		q.w = 0.25 / s;
		q.x = (m32 - m23) * s;
		q.y = (m13 - m31) * s;
		q.z = (m21 - m12) * s;
	}
	else if (m11 > m22 && m11 > m33) {
		s = 2.0 * sqrtf(1.0 + m11 - m22 - m33);
		q.w = (m32 - m23) / s;
		q.x = 0.25 * s;
		q.y = (m12 + m21) / s;
		q.z = (m13 + m31) / s;
	}
	else if (m22 > m33) {
		s = 2.0 * sqrtf(1.0 + m22 - m11 - m33);
		q.w = (m13 - m31) / s;
		q.x = (m12 + m21) / s;
		q.y = 0.25 * s;
		q.z = (m23 + m32) / s;
	}
	else {
		s = 2.0 * sqrtf(1.0 + m33 - m11 - m22);
		q.w = (m21 - m12) / s;
		q.x = (m13 + m31) / s;
		q.y = (m23 + m32) / s;
		q.z = 0.25 * s;
	}
	return q;
}

quat_t quat_mult(quat_t a, quat_t b) {
    quat_t q;
	q.x = a.x * b.w + a.w * b.x + a.y * b.z - a.z * b.y;
	q.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	q.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	q.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	return q;
}

quat_t quat_norm(quat_t q) {
	float l = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	if (l == 0.0) {
		q.x = 0;
		q.y = 0;
		q.z = 0;
		q.w = 0;
	}
	else {
		l = 1.0 / l;
		q.x *= l;
		q.y *= l;
		q.z *= l;
		q.w *= l;
	}
	return q;
}

quat_t quat_clone(quat_t q) {
	return q;
}

vec4_t quat_get_euler(quat_t q) {
	float a = -2 * (q.x * q.z - q.w * q.y);
	float b =  q.w *  q.w + q.x * q.x - q.y * q.y - q.z * q.z;
	float c =  2 * (q.x * q.y + q.w * q.z);
	float d = -2 * (q.y * q.z - q.w * q.x);
	float e =  q.w *  q.w - q.x * q.x + q.y * q.y - q.z * q.z;
	return vec4_create(atan2f(d, e), atan2f(a, b), asinf(c), 1.0);
}

quat_t quat_from_euler(float x, float y, float z) {
	float f = x / 2;
	float c1 = cosf(f);
	float s1 = sinf(f);
	f = y / 2;
	float c2 = cosf(f);
	float s2 = sinf(f);
	f = z / 2;
	float c3 = cosf(f);
	float s3 = sinf(f);
	// YZX
    quat_t q;
	q.x = s1 * c2 * c3 + c1 * s2 * s3;
	q.y = c1 * s2 * c3 + s1 * c2 * s3;
	q.z = c1 * c2 * s3 - s1 * s2 * c3;
	q.w = c1 * c2 * c3 - s1 * s2 * s3;
	return q;
}

quat_t quat_lerp(quat_t from, quat_t to, float s) {
	float dot = quat_dot(from, to);
	if (dot < 0.0) {
		from.x = -from.x;
		from.y = -from.y;
		from.z = -from.z;
		from.w = -from.w;
	}
	from.x += (to.x - from.x) * s;
	from.y += (to.y - from.y) * s;
	from.z += (to.z - from.z) * s;
	from.w += (to.w - from.w) * s;
	return quat_norm(from);
}

float quat_dot(quat_t a, quat_t b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

quat_t quat_from_to(vec4_t v0, vec4_t v1) {
	// Rotation formed by direction vectors
	// v0 and v1 should be normalized first
	float dot = vec4_dot(v0, v1);
	if (dot < -0.999999) {
		vec4_t a = vec4_cross(vec4_x_axis(), v0);
		if (vec4_len(a) < 0.000001) {
			a = vec4_cross(vec4_y_axis(), v0);
		}
		a = vec4_norm(a);
		return quat_from_axis_angle(a, MATH_PI);
	}
	else if (dot > 0.999999) {
		return quat_create(0, 0, 0, 1);
	}
	else {
		vec4_t a = vec4_cross(v0, v1);
		quat_t q = quat_create(a.x, a.y, a.z, 1.0 + dot);
		return quat_norm(q);
	}
}
