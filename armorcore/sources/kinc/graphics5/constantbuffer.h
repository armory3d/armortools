#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/constantbuffer.h>

#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

/*! \file constantbuffer.h
    \brief Provides support for managing buffers of constant-data for shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_constant_buffer {
	uint8_t *data;
	ConstantBuffer5Impl impl;
} kinc_g5_constant_buffer_t;

/// <summary>
/// Initializes a constant-buffer.
/// </summary>
/// <param name="buffer">The buffer to initialize</param>
/// <param name="size">The size of the constant-data in the buffer in bytes</param>
void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size);

/// <summary>
/// Destroys a buffer.
/// </summary>
/// <param name="buffer">The buffer to destroy</param>
void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer);

/// <summary>
/// Locks all of a constant-buffer to modify its contents.
/// </summary>
/// <param name="buffer">The buffer to lock</param>
/// <returns>The contents of the buffer</returns>
void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer);

/// <summary>
/// Locks part of a constant-buffer to modify its contents.
/// </summary>
/// <param name="buffer">The buffer to lock</param>
/// <param name="start">The offset of where to start the lock in bytes</param>
/// <param name="count">The number of bytes to lock</param>
/// <returns>The contents of the buffer, starting at start</returns>
void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count);

/// <summary>
/// Unlocks a constant-buffer so the changed contents can be used.
/// </summary>
/// <param name="buffer">The buffer to unlock</param>
void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer);

/// <summary>
/// Figures out the size of the constant-data in the buffer.
/// </summary>
/// <param name="buffer">The buffer to figure out the size for</param>
/// <returns>Returns the size of the constant-data in the buffer in bytes</returns>
int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer);

/// <summary>
/// Assigns a bool at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value">The value to write into the buffer</param>
void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer_t *buffer, int offset, bool value);

/// <summary>
/// Assigns an integer at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value">The value to assign to the constant/uniform</param>
void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer_t *buffer, int offset, int value);

/// <summary>
/// Assigns two integers at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value1">The first value to write into the buffer</param>
/// <param name="value2">The second value to write into the buffer</param>
void kinc_g5_constant_buffer_set_int2(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2);

/// <summary>
/// Assigns three integers at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value1">The first value to write into the buffer</param>
/// <param name="value2">The second value to write into the buffer</param>
/// <param name="value3">The third value to write into the buffer/param>
void kinc_g5_constant_buffer_set_int3(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3);

/// <summary>
/// Assigns four integers at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value1">The first value to write into the buffer</param>
/// <param name="value2">The second value to write into the buffer</param>
/// <param name="value3">The third value to write into the buffer/param>
/// <param name="value4">The fourth value to write into the buffer</param>
void kinc_g5_constant_buffer_set_int4(kinc_g5_constant_buffer_t *buffer, int offset, int value1, int value2, int value3, int value4);

/// <summary>
/// Assigns a bunch of integers at an offset in a constant-buffer.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value">The values to write into the buffer</param>
/// <param name="value">The number of values to write into the buffer</param>
void kinc_g5_constant_buffer_set_ints(kinc_g5_constant_buffer_t *buffer, int offset, int *values, int count);

/// <summary>
/// Assigns a float at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value">The value to write into the buffer</param>
void kinc_g5_constant_buffer_set_float(kinc_g5_constant_buffer_t *buffer, int offset, float value);

/// <summary>
/// Assigns two floats at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value1">The first value to write into the buffer</param>
/// <param name="value2">The second value to write into the buffer</param>
void kinc_g5_constant_buffer_set_float2(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2);

/// <summary>
/// Assigns three floats at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value1">The first value to write into the buffer</param>
/// <param name="value2">The second value to write into the buffer</param>
/// <param name="value3">The third value to write into the buffer/param>
void kinc_g5_constant_buffer_set_float3(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3);

/// <summary>
/// Assigns four floats at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value1">The first value to write into the buffer</param>
/// <param name="value2">The second value to write into the buffer</param>
/// <param name="value3">The third value to write into the buffer/param>
/// <param name="value4">The fourth value to write into the buffer</param>
void kinc_g5_constant_buffer_set_float4(kinc_g5_constant_buffer_t *buffer, int offset, float value1, float value2, float value3, float value4);

/// <summary>
/// Assigns a bunch of floats at an offset in a constant-buffer.
/// </summary>
/// <param name="location">The location of the constant/uniform to assign the values to</param>
/// <param name="value">The values to write into the buffer</param>
/// <param name="value">The number of values to write into the buffer</param>
void kinc_g5_constant_buffer_set_floats(kinc_g5_constant_buffer_t *buffer, int offset, float *values, int count);

/// <summary>
/// Assigns a 3x3-matrix at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value">The value to write into the buffer</param>
void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix3x3_t *value);

/// <summary>
/// Assigns a 4x4-matrix at an offset in a constant-buffer.
/// </summary>
/// <param name="offset">The offset at which to write the data</param>
/// <param name="value">The value to write into the buffer</param>
void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer_t *buffer, int offset, kinc_matrix4x4_t *value);

extern bool kinc_g5_transposeMat3;
extern bool kinc_g5_transposeMat4;

#ifdef __cplusplus
}
#endif
