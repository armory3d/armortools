
#include "iron_math.h"
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

float iron_cot(float x) {
	return cosf(x) / sinf(x);
}

float iron_round(float value) {
	return floorf(value + 0.5f);
}

float iron_abs(float value) {
	return value < 0 ? -value : value;
}

float iron_min(float a, float b) {
	return a > b ? b : a;
}

float iron_max(float a, float b) {
	return a > b ? a : b;
}

int iron_mini(int a, int b) {
	return a > b ? b : a;
}

int iron_maxi(int a, int b) {
	return a > b ? a : b;
}

float iron_clamp(float value, float minValue, float maxValue) {
	return iron_max(minValue, iron_min(maxValue, value));
}

float iron_matrix3x3_get(iron_matrix3x3_t *matrix, int x, int y) {
	return matrix->m[x * 3 + y];
}

void iron_matrix3x3_set(iron_matrix3x3_t *matrix, int x, int y, float value) {
	matrix->m[x * 3 + y] = value;
}

void iron_matrix3x3_transpose(iron_matrix3x3_t *matrix) {
	iron_matrix3x3_t transposed;
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			iron_matrix3x3_set(&transposed, x, y, iron_matrix3x3_get(matrix, y, x));
		}
	}
	memcpy(matrix->m, transposed.m, sizeof(transposed.m));
}

iron_matrix3x3_t iron_matrix3x3_identity(void) {
	iron_matrix3x3_t m;
	memset(m.m, 0, sizeof(m.m));
	for (unsigned x = 0; x < 3; ++x) {
		iron_matrix3x3_set(&m, x, x, 1.0f);
	}
	return m;
}

iron_matrix3x3_t iron_matrix3x3_rotation_x(float alpha) {
	iron_matrix3x3_t m = iron_matrix3x3_identity();
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	iron_matrix3x3_set(&m, 1, 1, ca);
	iron_matrix3x3_set(&m, 2, 1, -sa);
	iron_matrix3x3_set(&m, 1, 2, sa);
	iron_matrix3x3_set(&m, 2, 2, ca);
	return m;
}

iron_matrix3x3_t iron_matrix3x3_rotation_y(float alpha) {
	iron_matrix3x3_t m = iron_matrix3x3_identity();
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	iron_matrix3x3_set(&m, 0, 0, ca);
	iron_matrix3x3_set(&m, 2, 0, sa);
	iron_matrix3x3_set(&m, 0, 2, -sa);
	iron_matrix3x3_set(&m, 2, 2, ca);
	return m;
}

iron_matrix3x3_t iron_matrix3x3_rotation_z(float alpha) {
	iron_matrix3x3_t m = iron_matrix3x3_identity();
	float ca = cosf(alpha);
	float sa = sinf(alpha);
	iron_matrix3x3_set(&m, 0, 0, ca);
	iron_matrix3x3_set(&m, 1, 0, -sa);
	iron_matrix3x3_set(&m, 0, 1, sa);
	iron_matrix3x3_set(&m, 1, 1, ca);
	return m;
}

iron_matrix3x3_t iron_matrix3x3_translation(float x, float y) {
	iron_matrix3x3_t m = iron_matrix3x3_identity();
	iron_matrix3x3_set(&m, 2, 0, x);
	iron_matrix3x3_set(&m, 2, 1, y);
	return m;
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#endif

iron_matrix3x3_t iron_matrix3x3_multiply(iron_matrix3x3_t *a, iron_matrix3x3_t *b) {
	iron_matrix3x3_t result;
	for (unsigned x = 0; x < 3; ++x) {
		for (unsigned y = 0; y < 3; ++y) {
			float t = iron_matrix3x3_get(a, 0, y) * iron_matrix3x3_get(b, x, 0);
			for (unsigned i = 1; i < 3; ++i) {
				t += iron_matrix3x3_get(a, i, y) * iron_matrix3x3_get(b, x, i);
			}
			iron_matrix3x3_set(&result, x, y, t);
		}
	}
	return result;
}

static float vector3_get(iron_vector3_t vec, int index) {
	float *values = (float *)&vec;
	return values[index];
}

static void vector3_set(iron_vector3_t *vec, int index, float value) {
	float *values = (float *)vec;
	values[index] = value;
}

iron_vector3_t iron_matrix3x3_multiply_vector(iron_matrix3x3_t *a, iron_vector3_t b) {
	iron_vector3_t product;
	for (unsigned y = 0; y < 3; ++y) {
		float t = 0;
		for (unsigned x = 0; x < 3; ++x) {
			t += iron_matrix3x3_get(a, x, y) * vector3_get(b, x);
		}
		vector3_set(&product, y, t);
	}
	return product;
}

float iron_matrix4x4_get(iron_matrix4x4_t *matrix, int x, int y) {
	return matrix->m[x * 4 + y];
}

void iron_matrix4x4_set(iron_matrix4x4_t *matrix, int x, int y, float value) {
	matrix->m[x * 4 + y] = value;
}

void iron_matrix4x4_transpose(iron_matrix4x4_t *matrix) {
	iron_matrix4x4_t transposed;
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			iron_matrix4x4_set(&transposed, x, y, iron_matrix4x4_get(matrix, y, x));
		}
	}
	memcpy(matrix->m, transposed.m, sizeof(transposed.m));
}

iron_matrix4x4_t iron_matrix4x4_multiply(iron_matrix4x4_t *a, iron_matrix4x4_t *b) {
	iron_matrix4x4_t result;
	for (unsigned x = 0; x < 4; ++x)
		for (unsigned y = 0; y < 4; ++y) {
			float t = iron_matrix4x4_get(a, 0, y) * iron_matrix4x4_get(b, x, 0);
			for (unsigned i = 1; i < 4; ++i) {
				t += iron_matrix4x4_get(a, i, y) * iron_matrix4x4_get(b, x, i);
			}
			iron_matrix4x4_set(&result, x, y, t);
		}
	return result;
}

void iron_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha) {
	*alpha = ((color & 0xff000000) >> 24) / 255.0f;
	*red = ((color & 0x00ff0000) >> 16) / 255.0f;
	*green = ((color & 0x0000ff00) >> 8) / 255.0f;
	*blue = (color & 0x000000ff) / 255.0f;
}

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

void iron_random_init(int64_t seed) {
	s[0] = (uint64_t)seed;
	s[1] = 2;
	s[2] = 3;
	s[3] = 4;
	s[1] = next();
	s[2] = next();
	s[3] = next();
}

int64_t iron_random_get(void) {
	return (int64_t)next();
}

int64_t iron_random_get_max(int64_t max) {
	return iron_random_get() % (max + 1);
}

int64_t iron_random_get_in(int64_t min, int64_t max) {
	int64_t value = iron_random_get();
	return (value < -LLONG_MAX ? LLONG_MAX : llabs(value)) % (max + 1 - min) + min;
}

uint32_t iron_hash_djb2(unsigned char *str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = hash * 33 ^ c;
	}
	return hash;
}
