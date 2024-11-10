#pragma once

#include <kinc/math/matrix.h>
#include "iron_mat4.h"

mat3_t mat3_create(float _00, float _10, float _20,
				   float _01, float _11, float _21,
				   float _02, float _12, float _22);
mat3_t mat3_identity();
mat3_t mat3_translation(float x, float y);
mat3_t mat3_rotation(float alpha);
mat3_t mat3_set_from4(mat4_t m4);
mat3_t mat3_multmat(mat3_t a, mat3_t b);
mat3_t mat3_nan();
bool mat3_isnan(mat3_t m);
