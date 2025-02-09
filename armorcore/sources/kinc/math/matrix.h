#pragma once

#include "vector.h"

/*! \file matrix.h
    \brief Provides basic matrix types and a few functions which create transformation-matrices. This only provides functionality which is needed elsewhere in
   Kinc - if you need more, look up how transformation-matrices work and add some functions to your own project. Alternatively the Kore/C++-API also provides a
   more complete matrix-API.
*/

typedef union kinc_matrix3x3 {
	float m[3 * 3];
	struct {
		float m00;
		float m01;
		float m02;
		float m10;
		float m11;
		float m12;
		float m20;
		float m21;
		float m22;
	};
} kinc_matrix3x3_t;

float kinc_matrix3x3_get(kinc_matrix3x3_t *matrix, int x, int y);
void kinc_matrix3x3_set(kinc_matrix3x3_t *matrix, int x, int y, float value);
void kinc_matrix3x3_transpose(kinc_matrix3x3_t *matrix);
kinc_matrix3x3_t kinc_matrix3x3_identity(void);
kinc_matrix3x3_t kinc_matrix3x3_rotation_x(float alpha);
kinc_matrix3x3_t kinc_matrix3x3_rotation_y(float alpha);
kinc_matrix3x3_t kinc_matrix3x3_rotation_z(float alpha);
kinc_matrix3x3_t kinc_matrix3x3_translation(float x, float y);
kinc_matrix3x3_t kinc_matrix3x3_multiply(kinc_matrix3x3_t *a, kinc_matrix3x3_t *b);
kinc_vector3_t kinc_matrix3x3_multiply_vector(kinc_matrix3x3_t *a, kinc_vector3_t b);

typedef union kinc_matrix4x4 {
	float m[4 * 4];
	struct {
		float m00;
		float m01;
		float m02;
		float m03;
		float m10;
		float m11;
		float m12;
		float m13;
		float m20;
		float m21;
		float m22;
		float m23;
		float m30;
		float m31;
		float m32;
		float m33;
	};
} kinc_matrix4x4_t;

float kinc_matrix4x4_get(kinc_matrix4x4_t *matrix, int x, int y);
void kinc_matrix4x4_set(kinc_matrix4x4_t *matrix, int x, int y, float value);
void kinc_matrix4x4_transpose(kinc_matrix4x4_t *matrix);
kinc_matrix4x4_t kinc_matrix4x4_multiply(kinc_matrix4x4_t *a, kinc_matrix4x4_t *b);

#ifdef KINC_IMPLEMENTATION_MATH
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#ifdef KINC_IMPLEMENTATION_MATH
#undef KINC_IMPLEMENTATION
#endif
#include <kinc/math/core.h>
#include <kinc/math/matrix.h>
#ifdef KINC_IMPLEMENTATION_MATH
#define KINC_IMPLEMENTATION
#endif

#include <string.h>

float kinc_matrix3x3_get(kinc_matrix3x3_t *matrix, int x, int y) {
	return matrix->m[x * 3 + y];
}

void kinc_matrix3x3_set(kinc_matrix3x3_t *matrix, int x, int y, float value) {
	matrix->m[x * 3 + y] = value;
}

void kinc_matrix3x3_transpose(kinc_matrix3x3_t *matrix) {
	kinc_matrix3x3_t transposed;
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			kinc_matrix3x3_set(&transposed, x, y, kinc_matrix3x3_get(matrix, y, x));
		}
	}
	memcpy(matrix->m, transposed.m, sizeof(transposed.m));
}

kinc_matrix3x3_t kinc_matrix3x3_identity(void) {
	kinc_matrix3x3_t m;
	memset(m.m, 0, sizeof(m.m));
	for (unsigned x = 0; x < 3; ++x) {
		kinc_matrix3x3_set(&m, x, x, 1.0f);
	}
	return m;
}

kinc_matrix3x3_t kinc_matrix3x3_rotation_x(float alpha) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	kinc_matrix3x3_set(&m, 1, 1, ca);
	kinc_matrix3x3_set(&m, 2, 1, -sa);
	kinc_matrix3x3_set(&m, 1, 2, sa);
	kinc_matrix3x3_set(&m, 2, 2, ca);
	return m;
}

kinc_matrix3x3_t kinc_matrix3x3_rotation_y(float alpha) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	kinc_matrix3x3_set(&m, 0, 0, ca);
	kinc_matrix3x3_set(&m, 2, 0, sa);
	kinc_matrix3x3_set(&m, 0, 2, -sa);
	kinc_matrix3x3_set(&m, 2, 2, ca);
	return m;
}

kinc_matrix3x3_t kinc_matrix3x3_rotation_z(float alpha) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	kinc_matrix3x3_set(&m, 0, 0, ca);
	kinc_matrix3x3_set(&m, 1, 0, -sa);
	kinc_matrix3x3_set(&m, 0, 1, sa);
	kinc_matrix3x3_set(&m, 1, 1, ca);
	return m;
}

kinc_matrix3x3_t kinc_matrix3x3_translation(float x, float y) {
	kinc_matrix3x3_t m = kinc_matrix3x3_identity();
	kinc_matrix3x3_set(&m, 2, 0, x);
	kinc_matrix3x3_set(&m, 2, 1, y);
	return m;
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#endif

kinc_matrix3x3_t kinc_matrix3x3_multiply(kinc_matrix3x3_t *a, kinc_matrix3x3_t *b) {
	kinc_matrix3x3_t result;
	for (unsigned x = 0; x < 3; ++x) {
		for (unsigned y = 0; y < 3; ++y) {
			float t = kinc_matrix3x3_get(a, 0, y) * kinc_matrix3x3_get(b, x, 0);
			for (unsigned i = 1; i < 3; ++i) {
				t += kinc_matrix3x3_get(a, i, y) * kinc_matrix3x3_get(b, x, i);
			}
			kinc_matrix3x3_set(&result, x, y, t);
		}
	}
	return result;
}

static float vector3_get(kinc_vector3_t vec, int index) {
	float *values = (float *)&vec;
	return values[index];
}

static void vector3_set(kinc_vector3_t *vec, int index, float value) {
	float *values = (float *)vec;
	values[index] = value;
}

kinc_vector3_t kinc_matrix3x3_multiply_vector(kinc_matrix3x3_t *a, kinc_vector3_t b) {
	kinc_vector3_t product;
	for (unsigned y = 0; y < 3; ++y) {
		float t = 0;
		for (unsigned x = 0; x < 3; ++x) {
			t += kinc_matrix3x3_get(a, x, y) * vector3_get(b, x);
		}
		vector3_set(&product, y, t);
	}
	return product;
}

float kinc_matrix4x4_get(kinc_matrix4x4_t *matrix, int x, int y) {
	return matrix->m[x * 4 + y];
}

void kinc_matrix4x4_set(kinc_matrix4x4_t *matrix, int x, int y, float value) {
	matrix->m[x * 4 + y] = value;
}

void kinc_matrix4x4_transpose(kinc_matrix4x4_t *matrix) {
	kinc_matrix4x4_t transposed;
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			kinc_matrix4x4_set(&transposed, x, y, kinc_matrix4x4_get(matrix, y, x));
		}
	}
	memcpy(matrix->m, transposed.m, sizeof(transposed.m));
}

kinc_matrix4x4_t kinc_matrix4x4_multiply(kinc_matrix4x4_t *a, kinc_matrix4x4_t *b) {
	kinc_matrix4x4_t result;
	for (unsigned x = 0; x < 4; ++x)
		for (unsigned y = 0; y < 4; ++y) {
			float t = kinc_matrix4x4_get(a, 0, y) * kinc_matrix4x4_get(b, x, 0);
			for (unsigned i = 1; i < 4; ++i) {
				t += kinc_matrix4x4_get(a, i, y) * kinc_matrix4x4_get(b, x, i);
			}
			kinc_matrix4x4_set(&result, x, y, t);
		}
	return result;
}

#endif
