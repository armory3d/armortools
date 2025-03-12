#pragma once

#include <iron_global.h>
#include <stdint.h>

#define IRON_PI 3.141592654
#define IRON_TAU 6.283185307

float iron_cot(float x);
float iron_round(float value);
float iron_abs(float value);
float iron_min(float a, float b);
float iron_max(float a, float b);
int iron_mini(int a, int b);
int iron_maxi(int a, int b);
float iron_clamp(float value, float minValue, float maxValue);

typedef struct iron_vector2 {
	float x;
	float y;
} iron_vector2_t;

typedef struct iron_vector3 {
	float x;
	float y;
	float z;
} iron_vector3_t;

typedef struct iron_vector4 {
	float x;
	float y;
	float z;
	float w;
} iron_vector4_t;

typedef struct iron_quaternion {
	float x;
	float y;
	float z;
	float w;
} iron_quaternion_t;

typedef union iron_matrix3x3 {
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
} iron_matrix3x3_t;

float iron_matrix3x3_get(iron_matrix3x3_t *matrix, int x, int y);
void iron_matrix3x3_set(iron_matrix3x3_t *matrix, int x, int y, float value);
void iron_matrix3x3_transpose(iron_matrix3x3_t *matrix);
iron_matrix3x3_t iron_matrix3x3_identity(void);
iron_matrix3x3_t iron_matrix3x3_rotation_x(float alpha);
iron_matrix3x3_t iron_matrix3x3_rotation_y(float alpha);
iron_matrix3x3_t iron_matrix3x3_rotation_z(float alpha);
iron_matrix3x3_t iron_matrix3x3_translation(float x, float y);
iron_matrix3x3_t iron_matrix3x3_multiply(iron_matrix3x3_t *a, iron_matrix3x3_t *b);
iron_vector3_t iron_matrix3x3_multiply_vector(iron_matrix3x3_t *a, iron_vector3_t b);

typedef union iron_matrix4x4 {
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
} iron_matrix4x4_t;

float iron_matrix4x4_get(iron_matrix4x4_t *matrix, int x, int y);
void iron_matrix4x4_set(iron_matrix4x4_t *matrix, int x, int y, float value);
void iron_matrix4x4_transpose(iron_matrix4x4_t *matrix);
iron_matrix4x4_t iron_matrix4x4_multiply(iron_matrix4x4_t *a, iron_matrix4x4_t *b);

void iron_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha);

void iron_random_init(int64_t seed);
int64_t iron_random_get(void);
int64_t iron_random_get_max(int64_t max);
int64_t iron_random_get_in(int64_t min, int64_t max);

uint32_t iron_hash_djb2(unsigned char *str);
