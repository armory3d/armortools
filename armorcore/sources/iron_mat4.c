#include "iron_mat4.h"

#include <math.h>
#include "iron_gc.h"

mat4_t mat4_create(float _00, float _10, float _20, float _30,
				   float _01, float _11, float _21, float _31,
				   float _02, float _12, float _22, float _32,
				   float _03, float _13, float _23, float _33) {
	mat4_t m;
	m.m[0] = _00;
	m.m[1] = _01;
	m.m[2] = _02;
	m.m[3] = _03;
	m.m[4] = _10;
	m.m[5] = _11;
	m.m[6] = _12;
	m.m[7] = _13;
	m.m[8] = _20;
	m.m[9] = _21;
	m.m[10] = _22;
	m.m[11] = _23;
	m.m[12] = _30;
	m.m[13] = _31;
	m.m[14] = _32;
	m.m[15] = _33;
	return m;
}

mat4_t mat4_identity() {
	return mat4_create(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
}

mat4_t mat4_from_f32_array(f32_array_t *a, int offset) {
	return mat4_create(
		a->buffer[0 + offset], a->buffer[1 + offset], a->buffer[2 + offset], a->buffer[3 + offset],
		a->buffer[4 + offset], a->buffer[5 + offset], a->buffer[6 + offset], a->buffer[7 + offset],
		a->buffer[8 + offset], a->buffer[9 + offset], a->buffer[10 + offset], a->buffer[11 + offset],
		a->buffer[12 + offset], a->buffer[13 + offset], a->buffer[14 + offset], a->buffer[15 + offset]
	);
}

mat4_t mat4_persp(float fov_y, float aspect, float zn, float zf) {
	float uh = 1.0 / tanf(fov_y / 2);
	float uw = uh / aspect;
	return mat4_create(
		uw, 0, 0, 0,
		0, uh, 0, 0,
		0, 0, (zf + zn) / (zn - zf), 2 * zf * zn / (zn - zf),
		0, 0, -1, 0
	);
}

mat4_t mat4_ortho(float left, float right, float bottom, float top, float znear, float zfar) {
	float rl = right - left;
	float tb = top - bottom;
	float fn = zfar - znear;
	float tx = -(right + left) / (rl);
	float ty = -(top + bottom) / (tb);
	float tz = -(zfar + znear) / (fn);
	return mat4_create(
		2 / rl,	0,		0,		 tx,
		0,		2 / tb,	0,		 ty,
		0,		0,		-2 / fn, tz,
		0,		0,		0,		 1
	);
}

mat4_t mat4_rot_z(float alpha) {
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	return mat4_create(
		ca, -sa, 0, 0,
		sa,  ca, 0, 0,
		0,   0, 1, 0,
		0,   0, 0, 1
	);
}

mat4_t mat4_compose(vec4_t loc, quat_t rot, vec4_t scl) {
	mat4_t m = mat4_from_quat(rot);
	m = mat4_scale(m, scl);
	m = mat4_set_loc(m, loc);
	return m;
}

mat4_decomposed_t *mat4_decompose(mat4_t m) {
	vec4_t loc;
	quat_t rot;
	vec4_t scl;
	loc.x = m.m[12];
	loc.y = m.m[13];
	loc.z = m.m[14];
	scl.x = vec4_len(vec4_create(m.m[0], m.m[1], m.m[2], 1.0));
	scl.y = vec4_len(vec4_create(m.m[4], m.m[5], m.m[6], 1.0));
	scl.z = vec4_len(vec4_create(m.m[8], m.m[9], m.m[10], 1.0));
	if (mat4_determinant(m) < 0.0) {
		scl.x = -scl.x;
	}
	float invs = 1.0 / scl.x; // Scale the rotation part
	m.m[0] *= invs;
	m.m[1] *= invs;
	m.m[2] *= invs;
	invs = 1.0 / scl.y;
	m.m[4] *= invs;
	m.m[5] *= invs;
	m.m[6] *= invs;
	invs = 1.0 / scl.z;
	m.m[8] *= invs;
	m.m[9] *= invs;
	m.m[10] *= invs;
	rot = quat_from_rot_mat(m);

	mat4_decomposed_t *dec = gc_alloc(sizeof(mat4_decomposed_t));
	dec->loc = loc;
	dec->rot = rot;
	dec->scl = scl;
	return dec;
}

mat4_t mat4_set_loc(mat4_t m, vec4_t v) {
	m.m[12] = v.x;
	m.m[13] = v.y;
	m.m[14] = v.z;
	return m;
}

mat4_t mat4_from_quat(quat_t q) {
	float x2 = q.x + q.x;
	float y2 = q.y + q.y;
	float z2 = q.z + q.z;
	float xx = q.x * x2;
	float xy = q.x * y2;
	float xz = q.x * z2;
	float yy = q.y * y2;
	float yz = q.y * z2;
	float zz = q.z * z2;
	float wx = q.w * x2;
	float wy = q.w * y2;
	float wz = q.w * z2;

	mat4_t m;
	m.m[0] = 1.0 - (yy + zz);
	m.m[4] = xy - wz;
	m.m[8] = xz + wy;

	m.m[1] = xy + wz;
	m.m[5] = 1.0 - (xx + zz);
	m.m[9] = yz - wx;

	m.m[2] = xz - wy;
	m.m[6] = yz + wx;
	m.m[10] = 1.0 - (xx + yy);

	m.m[3] = 0.0;
	m.m[7] = 0.0;
	m.m[11] = 0.0;
	m.m[12] = 0.0;
	m.m[13] = 0.0;
	m.m[14] = 0.0;
	m.m[15] = 1.0;

	return m;
}

mat4_t mat4_init_translate(float x, float y, float z) {
	mat4_t m;
	m.m[0] = 1.0;
	m.m[1] = 0.0;
	m.m[2] = 0.0;
	m.m[3] = 0.0;
	m.m[4] = 0.0;
	m.m[5] = 1.0;
	m.m[6] = 0.0;
	m.m[7] = 0.0;
	m.m[8] = 0.0;
	m.m[9] = 0.0;
	m.m[10] = 1.0;
	m.m[11] = 0.0;
	m.m[12] = x;
	m.m[13] = y;
	m.m[14] = z;
	m.m[15] = 1.0;
	return m;
}

mat4_t mat4_translate(mat4_t m, float x, float y, float z) {
	m.m[0] += x * m.m[3];
	m.m[1] += y * m.m[3];
	m.m[2] += z * m.m[3];
	m.m[4] += x * m.m[7];
	m.m[5] += y * m.m[7];
	m.m[6] += z * m.m[7];
	m.m[8] += x * m.m[11];
	m.m[9] += y * m.m[11];
	m.m[10] += z * m.m[11];
	m.m[12] += x * m.m[15];
	m.m[13] += y * m.m[15];
	m.m[14] += z * m.m[15];
	return m;
}

mat4_t mat4_scale(mat4_t m, vec4_t v) {
	m.m[0] *= v.x;
	m.m[1] *= v.x;
	m.m[2] *= v.x;
	m.m[3] *= v.x;
	m.m[4] *= v.y;
	m.m[5] *= v.y;
	m.m[6] *= v.y;
	m.m[7] *= v.y;
	m.m[8] *= v.z;
	m.m[9] *= v.z;
	m.m[10] *= v.z;
	m.m[11] *= v.z;
	return m;
}

mat4_t mat4_mult_mat3x4(mat4_t a, mat4_t b) {
	float a00 = a.m[0];
	float a01 = a.m[1];
	float a02 = a.m[2];
	float a03 = a.m[3];
	float a10 = a.m[4];
	float a11 = a.m[5];
	float a12 = a.m[6];
	float a13 = a.m[7];
	float a20 = a.m[8];
	float a21 = a.m[9];
	float a22 = a.m[10];
	float a23 = a.m[11];
	float a30 = a.m[12];
	float a31 = a.m[13];
	float a32 = a.m[14];
	float a33 = a.m[15];

	float b0 = b.m[0];
	float b1 = b.m[4];
	float b2 = b.m[8];
	float b3 = b.m[12];

	mat4_t m;
	m.m[0] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	m.m[4] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	m.m[8] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	m.m[12] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[1];
	b1 = b.m[5];
	b2 = b.m[9];
	b3 = b.m[13];
	m.m[1] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	m.m[5] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	m.m[9] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	m.m[13] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[2];
	b1 = b.m[6];
	b2 = b.m[10];
	b3 = b.m[14];
	m.m[2] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	m.m[6] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	m.m[10] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	m.m[14] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	m.m[3] = 0;
	m.m[7] = 0;
	m.m[11] = 0;
	m.m[15] = 1;
	return m;
}

mat4_t mat4_mult_mat(mat4_t a, mat4_t b) {
	float a00 = a.m[0];
	float a01 = a.m[1];
	float a02 = a.m[2];
	float a03 = a.m[3];
	float a10 = a.m[4];
	float a11 = a.m[5];
	float a12 = a.m[6];
	float a13 = a.m[7];
	float a20 = a.m[8];
	float a21 = a.m[9];
	float a22 = a.m[10];
	float a23 = a.m[11];
	float a30 = a.m[12];
	float a31 = a.m[13];
	float a32 = a.m[14];
	float a33 = a.m[15];

	float b0 = b.m[0];
	float b1 = b.m[4];
	float b2 = b.m[8];
	float b3 = b.m[12];
	a.m[0] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	a.m[4] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	a.m[8] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	a.m[12] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[1];
	b1 = b.m[5];
	b2 = b.m[9];
	b3 = b.m[13];
	a.m[1] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	a.m[5] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	a.m[9] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	a.m[13] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[2];
	b1 = b.m[6];
	b2 = b.m[10];
	b3 = b.m[14];
	a.m[2] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	a.m[6] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	a.m[10] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	a.m[14] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[3];
	b1 = b.m[7];
	b2 = b.m[11];
	b3 = b.m[15];
	a.m[3] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	a.m[7] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	a.m[11] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	a.m[15] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	return a;
}

mat4_t mat4_inv(mat4_t a) {
	float a00 = a.m[0];
	float a01 = a.m[1];
	float a02 = a.m[2];
	float a03 = a.m[3];
	float a10 = a.m[4];
	float a11 = a.m[5];
	float a12 = a.m[6];
	float a13 = a.m[7];
	float a20 = a.m[8];
	float a21 = a.m[9];
	float a22 = a.m[10];
	float a23 = a.m[11];
	float a30 = a.m[12];
	float a31 = a.m[13];
	float a32 = a.m[14];
	float a33 = a.m[15];
	float b00 = a00 * a11 - a01 * a10;
	float b01 = a00 * a12 - a02 * a10;
	float b02 = a00 * a13 - a03 * a10;
	float b03 = a01 * a12 - a02 * a11;
	float b04 = a01 * a13 - a03 * a11;
	float b05 = a02 * a13 - a03 * a12;
	float b06 = a20 * a31 - a21 * a30;
	float b07 = a20 * a32 - a22 * a30;
	float b08 = a20 * a33 - a23 * a30;
	float b09 = a21 * a32 - a22 * a31;
	float b10 = a21 * a33 - a23 * a31;
	float b11 = a22 * a33 - a23 * a32;

	float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if (det == 0.0) {
		return mat4_identity();
	}
	det = 1.0 / det;

	mat4_t m;
	m.m[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
	m.m[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
	m.m[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
	m.m[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
	m.m[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
	m.m[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
	m.m[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
	m.m[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
	m.m[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
	m.m[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
	m.m[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
	m.m[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
	m.m[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
	m.m[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
	m.m[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
	m.m[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;

	return m;
}

mat4_t mat4_transpose(mat4_t m) {
	float f = m.m[1];
	m.m[1] = m.m[4];
	m.m[4] = f;

	f = m.m[2];
	m.m[2] = m.m[8];
	m.m[8] = f;

	f = m.m[3];
	m.m[3] = m.m[12];
	m.m[12] = f;

	f = m.m[6];
	m.m[6] = m.m[9];
	m.m[9] = f;

	f = m.m[7];
	m.m[7] = m.m[13];
	m.m[13] = f;

	f = m.m[11];
	m.m[11] = m.m[14];
	m.m[14] = f;

	return m;
}

mat4_t mat4_transpose3x3(mat4_t m) {
	float f = m.m[1];
	m.m[1] = m.m[4];
	m.m[4] = f;

	f = m.m[2];
	m.m[2] = m.m[8];
	m.m[8] = f;

	f = m.m[6];
	m.m[6] = m.m[9];
	m.m[9] = f;

	return m;
}

mat4_t mat4_clone(mat4_t m) {
	return mat4_create(
		m.m[0], m.m[4], m.m[8], m.m[12],
		m.m[1], m.m[5], m.m[9], m.m[13],
		m.m[2], m.m[6], m.m[10], m.m[14],
		m.m[3], m.m[7], m.m[11], m.m[15]
	);
}

vec4_t mat4_get_loc(mat4_t m) {
	return vec4_create(m.m[12], m.m[13], m.m[14], m.m[15]);
}

vec4_t mat4_get_scale(mat4_t m) {
	return vec4_create(
		sqrtf(m.m[0] * m.m[0] + m.m[4] * m.m[4] + m.m[8] * m.m[8]),
		sqrtf(m.m[1] * m.m[1] + m.m[5] * m.m[5] + m.m[9] * m.m[9]),
		sqrtf(m.m[2] * m.m[2] + m.m[6] * m.m[6] + m.m[10] * m.m[10]),
		1.0
	);
}

mat4_t mat4_mult(mat4_t m, float s) {
	m.m[0] *= s;
	m.m[4] *= s;
	m.m[8] *= s;
	m.m[12] *= s;
	m.m[1] *= s;
	m.m[5] *= s;
	m.m[9] *= s;
	m.m[13] *= s;
	m.m[2] *= s;
	m.m[6] *= s;
	m.m[10] *= s;
	m.m[14] *= s;
	m.m[3] *= s;
	m.m[7] *= s;
	m.m[11] *= s;
	m.m[15] *= s;
	return m;
}

mat4_t mat4_to_rot(mat4_t m) {
	float scale = 1.0 / vec4_len(vec4_create(m.m[0], m.m[1], m.m[2], 1.0));
	m.m[0] = m.m[0] * scale;
	m.m[1] = m.m[1] * scale;
	m.m[2] = m.m[2] * scale;
	scale = 1.0 / vec4_len(vec4_create(m.m[4], m.m[5], m.m[6], 1.0));
	m.m[4] = m.m[4] * scale;
	m.m[5] = m.m[5] * scale;
	m.m[6] = m.m[6] * scale;
	scale = 1.0 / vec4_len(vec4_create(m.m[8], m.m[9], m.m[10], 1.0));
	m.m[8] = m.m[8] * scale;
	m.m[9] = m.m[9] * scale;
	m.m[10] = m.m[10] * scale;
	m.m[3] = 0.0;
	m.m[7] = 0.0;
	m.m[11] = 0.0;
	m.m[12] = 0.0;
	m.m[13] = 0.0;
	m.m[14] = 0.0;
	m.m[15] = 1.0;
	return m;
}

vec4_t mat4_right(mat4_t m) {
	return vec4_norm(vec4_create(m.m[0], m.m[1], m.m[2], 1.0));
}

vec4_t mat4_look(mat4_t m) {
	return vec4_norm(vec4_create(m.m[4], m.m[5], m.m[6], 1.0));
}

vec4_t mat4_up(mat4_t m) {
	return vec4_norm(vec4_create(m.m[8], m.m[9], m.m[10], 1.0));
}

f32_array_t *mat4_to_f32_array(mat4_t m) {
	f32_array_t *a = f32_array_create(16);
	a->buffer[0] = m.m[0];
	a->buffer[1] = m.m[4];
	a->buffer[2] = m.m[8];
	a->buffer[3] = m.m[12];
	a->buffer[4] = m.m[1];
	a->buffer[5] = m.m[5];
	a->buffer[6] = m.m[9];
	a->buffer[7] = m.m[13];
	a->buffer[8] = m.m[2];
	a->buffer[9] = m.m[6];
	a->buffer[10] = m.m[10];
	a->buffer[11] = m.m[14];
	a->buffer[12] = m.m[3];
	a->buffer[13] = m.m[7];
	a->buffer[14] = m.m[11];
	a->buffer[15] = m.m[15];
	return a;
}

float mat4_cofactor(float m0, float m1, float m2,
					float m3, float m4, float m5,
					float m6, float m7, float m8) {
	return m0 * (m4 * m8 - m5 * m7) - m1 * (m3 * m8 - m5 * m6) + m2 * (m3 * m7 - m4 * m6);
}

float mat4_determinant(mat4_t m) {
	float c00 = mat4_cofactor(m.m[5], m.m[9], m.m[13], m.m[6], m.m[10], m.m[14], m.m[7], m.m[11], m.m[15]);
	float c01 = mat4_cofactor(m.m[4], m.m[8], m.m[12], m.m[6], m.m[10], m.m[14], m.m[7], m.m[11], m.m[15]);
	float c02 = mat4_cofactor(m.m[4], m.m[8], m.m[12], m.m[5], m.m[9], m.m[13], m.m[7], m.m[11], m.m[15]);
	float c03 = mat4_cofactor(m.m[4], m.m[8], m.m[12], m.m[5], m.m[9], m.m[13], m.m[6], m.m[10], m.m[14]);
	return m.m[0] * c00 - m.m[1] * c01 + m.m[2] * c02 - m.m[3] * c03;
}

mat4_t mat4_nan() {
	mat4_t m;
	m.m[0] = NAN;
	return m;
}

bool mat4_isnan(mat4_t m) {
	return isnan(m.m[0]);
}
