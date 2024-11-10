#pragma once

#include <kinc/global.h>

#include "vertexstructure.h"

#include <kinc/backend/graphics5/vertexbuffer.h>

/*! \file vertexbuffer.h
    \brief Provides functions for setting up and using vertex-buffers.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_vertex_buffer {
	VertexBuffer5Impl impl;
} kinc_g5_vertex_buffer_t;

/// <summary>
/// Allocate and initialize a vertex-buffer.
/// </summary>
/// <param name="buffer">The buffer to initialize</param>
/// <param name="count">The number of vertices in the buffer</param>
/// <param name="structure">The structure of the buffer</param>
/// <param name="gpu_memory">If true, the vertex-buffer will reside in gpu-memory which will make it slower to update but faster to use</param>
/// <param name="instance_data_step_rate">The step-rate for instanced-rendering - use 0 if instanced-rendering will not be used with this buffer</param>
void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpu_memory,
                                          int instance_data_step_rate);

/// <summary>
/// Destroys a vertex-buffer.
/// </summary>
/// <param name="buffer">The buffer to destroy</param>
void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer);

/// <summary>
/// Locks all of a vertex-buffer to modify its contents.
/// </summary>
/// <param name="buffer">The buffer to lock</param>
/// <returns>The contents of the buffer</returns>
float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer);

/// <summary>
/// Locks part of a vertex-buffer to modify its contents.
/// </summary>
/// <param name="buffer">The buffer to lock</param>
/// <param name="start">The index of the first vertex to lock</param>
/// <param name="count">The number of vertices to lock</param>
/// <returns>The contents of the buffer, starting at start</returns>
float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count);

/// <summary>
/// Unlock all of a vertex-buffer so the changed contents can be used.
/// </summary>
/// <param name="buffer">The buffer to unlock</param>
void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer);

/// <summary>
/// Unlocks part of a vertex-buffer so the changed contents can be used.
/// </summary>
/// <param name="buffer">The buffer to unlock</param>
/// <param name="count">The number of vertices to unlock, starting from the start-vertex from the previous lock-call</param>
void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count);

/// <summary>
/// Returns the number of vertices in a buffer.
/// </summary>
/// <param name="buffer">The buffer to figure out the number of vertices for</param>
/// <returns>The number of vertices</returns>
int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer);

/// <summary>
/// Returns the stride aka the size of one vertex of the buffer in bytes.
/// </summary>
/// <param name="buffer">The buffer to figure out the stride for</param>
/// <returns>The stride of the buffer in bytes</returns>
int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer);

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer, int offset);

#ifdef __cplusplus
}
#endif
