#pragma once

#include <kinc/math/matrix.h>
#include "iron_vec4.h"
#include "iron_quat.h"
#include "iron_array.h"

typedef struct mat4_decomposed {
	vec4_t loc;
	quat_t rot;
	vec4_t scl;
} mat4_decomposed_t;

mat4_t mat4_create(float _00, float _10, float _20, float _30,
				   float _01, float _11, float _21, float _31,
				   float _02, float _12, float _22, float _32,
				   float _03, float _13, float _23, float _33);
mat4_t mat4_identity();
mat4_t mat4_from_f32_array(f32_array_t *a, int offset);
mat4_t mat4_persp(float fov_y, float aspect, float zn, float zf);
mat4_t mat4_ortho(float left, float right, float bottom, float top, float znear, float zfar);
mat4_t mat4_rot_z(float alpha);
mat4_t mat4_compose(vec4_t loc, quat_t rot, vec4_t scl);
mat4_decomposed_t *mat4_decompose(mat4_t m);
mat4_t mat4_set_loc(mat4_t m, vec4_t v);
mat4_t mat4_from_quat(quat_t q);
mat4_t mat4_init_translate(float x, float y, float z);
mat4_t mat4_translate(mat4_t m, float x, float y, float z);
mat4_t mat4_scale(mat4_t m, vec4_t v);
mat4_t mat4_mult_mat3x4(mat4_t a, mat4_t b);
mat4_t mat4_mult_mat(mat4_t a, mat4_t b);
mat4_t mat4_inv(mat4_t a);
mat4_t mat4_transpose(mat4_t m);
mat4_t mat4_transpose3x3(mat4_t m);
mat4_t mat4_clone(mat4_t m);
vec4_t mat4_get_loc(mat4_t m);
vec4_t mat4_get_scale(mat4_t m);
mat4_t mat4_mult(mat4_t m, float s);
mat4_t mat4_to_rot(mat4_t m);
vec4_t mat4_right(mat4_t m);
vec4_t mat4_look(mat4_t m);
vec4_t mat4_up(mat4_t m);
f32_array_t *mat4_to_f32_array(mat4_t m);
float mat4_cofactor(float m0, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8);
float mat4_determinant(mat4_t m);
mat4_t mat4_nan();
bool mat4_isnan(mat4_t m);

#define mat4nan (mat4_nan())
