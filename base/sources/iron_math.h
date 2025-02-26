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

void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha);

void kinc_random_init(int64_t seed);
int64_t kinc_random_get(void);
int64_t kinc_random_get_max(int64_t max);
int64_t kinc_random_get_in(int64_t min, int64_t max);
