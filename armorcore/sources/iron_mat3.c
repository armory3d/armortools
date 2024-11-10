#include "iron_mat3.h"

#include <math.h>
#include <kinc/math/core.h>

mat3_t mat3_create(float _00, float _10, float _20,
				   float _01, float _11, float _21,
				   float _02, float _12, float _22) {
	mat3_t m;
	m.m[0] = _00;
	m.m[1] = _01;
	m.m[2] = _02;
	m.m[3] = _10;
	m.m[4] = _11;
	m.m[5] = _12;
	m.m[6] = _20;
	m.m[7] = _21;
	m.m[8] = _22;
	return m;
}

mat3_t mat3_identity() {
	return mat3_create(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
}

mat3_t mat3_translation(float x, float y) {
	return mat3_create(
		1, 0, x,
		0, 1, y,
		0, 0, 1
	);
}

mat3_t mat3_rotation(float alpha) {
	return mat3_create(
		cosf(alpha), -sinf(alpha), 0,
		sinf(alpha), cosf(alpha), 0,
		0, 0, 1
	);
}

mat3_t mat3_set_from4(mat4_t m4) {
	mat3_t m;
    m.m[0] = m4.m[0];
	m.m[1] = m4.m[1];
	m.m[2] = m4.m[2];
	m.m[3] = m4.m[4];
	m.m[4] = m4.m[5];
	m.m[5] = m4.m[6];
	m.m[6] = m4.m[8];
	m.m[7] = m4.m[9];
	m.m[8] = m4.m[10];
	return m;
}

mat3_t mat3_multmat(mat3_t a, mat3_t b) {
	return mat3_create(
		a.m[0] * b.m[0] + a.m[3] * b.m[1] + a.m[6] * b.m[2],
		a.m[0] * b.m[3] + a.m[3] * b.m[4] + a.m[6] * b.m[5],
		a.m[0] * b.m[6] + a.m[3] * b.m[7] + a.m[6] * b.m[8],
		a.m[1] * b.m[0] + a.m[4] * b.m[1] + a.m[7] * b.m[2],
		a.m[1] * b.m[3] + a.m[4] * b.m[4] + a.m[7] * b.m[5],
		a.m[1] * b.m[6] + a.m[4] * b.m[7] + a.m[7] * b.m[8],
		a.m[2] * b.m[0] + a.m[5] * b.m[1] + a.m[8] * b.m[2],
		a.m[2] * b.m[3] + a.m[5] * b.m[4] + a.m[8] * b.m[5],
		a.m[2] * b.m[6] + a.m[5] * b.m[7] + a.m[8] * b.m[8]
	);
}

mat3_t mat3_nan() {
	mat3_t m;
	m.m[0] = NAN;
	return m;
}

bool mat3_isnan(mat3_t m) {
	return isnan(m.m[0]);
}
