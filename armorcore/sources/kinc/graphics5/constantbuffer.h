#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics5/constantbuffer.h>
#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

/*! \file constantbuffer.h
    \brief Provides support for managing buffers of constant-data for shaders.
*/

typedef struct kinc_g5_constant_buffer {
	uint8_t *data;
	ConstantBuffer5Impl impl;
} kinc_g5_constant_buffer_t;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size);
void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer);
void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer);
void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count);
void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer);
int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer);
void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer_t *buffer, int offset, bool value);
void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer_t *buffer, int offset, int value);
void kinc_g5_constant_buffer_set_int2(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2);
void kinc_g5_constant_buffer_set_int3(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3);
void kinc_g5_constant_buffer_set_int4(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4);
void kinc_g5_constant_buffer_set_ints(kinc_g5_constant_buffer_t *buffer, int offset, int *values, int count);
void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer_t *buffer, int offset, float value);
void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2);
void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3);
void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4);
void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer_t *buffer, int offset, float *values, int count);
void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value);
void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value);

extern bool kinc_g5_transposeMat;
