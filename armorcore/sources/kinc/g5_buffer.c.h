#include "g5_buffer.h"
#include <kinc/g5.h>

extern bool waitAfterNextDraw;

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, kinc_g4_usage_t usage) {
	buffer->impl._multiple = usage == KINC_G4_USAGE_STATIC ? 1 : 2;
	buffer->impl._currentIndex = 0;
	buffer->impl.myCount = count;
	for (int i = 0; i < buffer->impl._multiple; ++i) {
		kinc_g5_vertex_buffer_init(&buffer->impl._buffer[i], count, structure, usage == KINC_G4_USAGE_STATIC);
	}
}

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	for (int i = 0; i < buffer->impl._multiple; ++i) {
		kinc_g5_vertex_buffer_destroy(&buffer->impl._buffer[i]);
	}
}

static void kinc_internal_prepare_lock(kinc_g4_vertex_buffer_t *buffer) {
	++buffer->impl._currentIndex;
	if (buffer->impl._currentIndex >= buffer->impl._multiple - 1) {
		waitAfterNextDraw = true;
	}
	if (buffer->impl._currentIndex >= buffer->impl._multiple) {
		buffer->impl._currentIndex = 0;
	}
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_internal_prepare_lock(buffer);
	return kinc_g5_vertex_buffer_lock_all(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	kinc_internal_prepare_lock(buffer);
	return kinc_g5_vertex_buffer_lock(&buffer->impl._buffer[buffer->impl._currentIndex], start, count);
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_unlock_all(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g5_vertex_buffer_unlock(&buffer->impl._buffer[buffer->impl._currentIndex], count);
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_stride(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer_t *buffer, int offset, int value) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value;
}

void kinc_g5_constant_buffer_set_int2(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
}

void kinc_g5_constant_buffer_set_int3(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
}

void kinc_g5_constant_buffer_set_int4(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
	ints[3] = value4;
}

void kinc_g5_constant_buffer_set_ints(kinc_g5_constant_buffer_t *buffer, int offset, int *values, int count) {
	int *ints = (int *)(&buffer->data[offset]);
	for (int i = 0; i < count; ++i) {
		ints[i] = values[i];
	}
}

void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer_t *buffer, int offset, float value) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value;
}

void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
}

void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4) {
	float *floats = (float *)(&buffer->data[offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer_t *buffer, int offset, float *values, int count) {
	float *floats = (float *)(&buffer->data[offset]);
	for (int i = 0; i < count; ++i) {
		floats[i] = values[i];
	}
}

void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer_t *buffer, int offset, bool value) {
	int *ints = (int *)(&buffer->data[offset]);
	ints[0] = value ? 1 : 0;
}

static void kinc_internal_set_matrix3(uint8_t *constants, int offset, kinc_matrix3x3_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = kinc_matrix3x3_get(value, x, y);
		}
	}
}

void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value) {
	if (kinc_g5_transpose_mat) {
		kinc_matrix3x3_t m = *value;
		kinc_matrix3x3_transpose(&m);
		kinc_internal_set_matrix3(buffer->data, offset, &m);
	}
	else {
		kinc_internal_set_matrix3(buffer->data, offset, value);
	}
}

static void kinc_internal_set_matrix4(uint8_t *constants, int offset, kinc_matrix4x4_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = kinc_matrix4x4_get(value, x, y);
		}
	}
}

void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value) {
	if (kinc_g5_transpose_mat) {
		kinc_matrix4x4_t m = *value;
		kinc_matrix4x4_transpose(&m);
		kinc_internal_set_matrix4(buffer->data, offset, &m);
	}
	else {
		kinc_internal_set_matrix4(buffer->data, offset, value);
	}
}
