#include "constantbuffer.h"

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

static void set_matrix3(uint8_t *constants, int offset, kinc_matrix3x3_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = kinc_matrix3x3_get(value, x, y);
		}
	}
}

void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value) {
	if (kinc_g5_transposeMat) {
		kinc_matrix3x3_t m = *value;
		kinc_matrix3x3_transpose(&m);
		set_matrix3(buffer->data, offset, &m);
	}
	else {
		set_matrix3(buffer->data, offset, value);
	}
}

static void set_matrix4(uint8_t *constants, int offset, kinc_matrix4x4_t *value) {
	float *floats = (float *)(&constants[offset]);
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = kinc_matrix4x4_get(value, x, y);
		}
	}
}

void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value) {
	if (kinc_g5_transposeMat) {
		kinc_matrix4x4_t m = *value;
		kinc_matrix4x4_transpose(&m);
		set_matrix4(buffer->data, offset, &m);
	}
	else {
		set_matrix4(buffer->data, offset, value);
	}
}
