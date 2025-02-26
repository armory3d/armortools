#pragma once

#include <iron_global.h>
#include <stdint.h>

#define KINC_PI 3.141592654
#define KINC_TAU 6.283185307

float kinc_cot(float x);
float kinc_round(float value);
float kinc_abs(float value);
float kinc_min(float a, float b);
float kinc_max(float a, float b);
int kinc_mini(int a, int b);
int kinc_maxi(int a, int b);
float kinc_clamp(float value, float minValue, float maxValue);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif
#ifdef KINC_IMPLEMENTATION
#include <math.h>

float kinc_cot(float x) {
	return cosf(x) / sinf(x);
}

float kinc_round(float value) {
	return floorf(value + 0.5f);
}

float kinc_abs(float value) {
	return value < 0 ? -value : value;
}

float kinc_min(float a, float b) {
	return a > b ? b : a;
}

float kinc_max(float a, float b) {
	return a > b ? a : b;
}

int kinc_mini(int a, int b) {
	return a > b ? b : a;
}

int kinc_maxi(int a, int b) {
	return a > b ? a : b;
}

float kinc_clamp(float value, float minValue, float maxValue) {
	return kinc_max(minValue, kinc_min(maxValue, value));
}
#endif

typedef struct kinc_vector2 {
	float x;
	float y;
} kinc_vector2_t;

typedef struct kinc_vector3 {
	float x;
	float y;
	float z;
} kinc_vector3_t;

typedef struct kinc_vector4 {
	float x;
	float y;
	float z;
	float w;
} kinc_vector4_t;

typedef struct kinc_quaternion {
	float x;
	float y;
	float z;
	float w;
} kinc_quaternion_t;

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

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif
#ifdef KINC_IMPLEMENTATION
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

void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif
#ifdef KINC_IMPLEMENTATION

void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha) {
	*alpha = ((color & 0xff000000) >> 24) / 255.0f;
	*red = ((color & 0x00ff0000) >> 16) / 255.0f;
	*green = ((color & 0x0000ff00) >> 8) / 255.0f;
	*blue = (color & 0x000000ff) / 255.0f;
}
#endif

void kinc_random_init(int64_t seed);
int64_t kinc_random_get(void);
int64_t kinc_random_get_max(int64_t max);
int64_t kinc_random_get_in(int64_t min, int64_t max);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif
#ifdef KINC_IMPLEMENTATION
#include <limits.h>
#include <stdlib.h>

// xoshiro256** 1.0
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t s[4] = {1, 2, 3, 4};

uint64_t next(void) {
	const uint64_t result = rotl(s[1] * 5, 7) * 9;

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}

void kinc_random_init(int64_t seed) {
	s[0] = (uint64_t)seed;
	s[1] = 2;
	s[2] = 3;
	s[3] = 4;
	s[1] = next();
	s[2] = next();
	s[3] = next();
}

int64_t kinc_random_get(void) {
	return (int64_t)next();
}

int64_t kinc_random_get_max(int64_t max) {
	return kinc_random_get() % (max + 1);
}

int64_t kinc_random_get_in(int64_t min, int64_t max) {
	int64_t value = kinc_random_get();
	return (value < -LLONG_MAX ? LLONG_MAX : llabs(value)) % (max + 1 - min) + min;
}
#endif
