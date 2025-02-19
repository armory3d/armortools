#pragma once

#include <stdbool.h>
#include <kinc/global.h>
#include <kinc/matrix.h>
#include <kinc/vector.h>
#include <kinc/backend/graphics5/g5_buffer.h>
#include <kinc/g5.h>

typedef struct kinc_g5_vertex_buffer {
	VertexBuffer5Impl impl;
} kinc_g5_vertex_buffer_t;

typedef struct {
	int myCount;
	kinc_g5_vertex_buffer_t _buffer[2];
	int _currentIndex;
	int _multiple;
} kinc_g4_vertex_buffer_impl_t;

typedef struct kinc_g4_vertex_buffer {
	kinc_g4_vertex_buffer_impl_t impl;
} kinc_g4_vertex_buffer_t;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpu_memory);
void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer);
float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer);
float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count);
void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer);
void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count);
int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer);
int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer);

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer);

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, kinc_g4_usage_t usage);
void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count);
void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count);
int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer);
int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer);

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

extern bool kinc_g5_transpose_mat;

typedef struct kinc_g5_index_buffer {
	IndexBuffer5Impl impl;
} kinc_g5_index_buffer_t;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpu_memory);
void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer);
void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer);
void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count);
void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer);
void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count);
int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer);
