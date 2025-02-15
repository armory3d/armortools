#pragma once

#include <kinc/global.h>
#include "vertexstructure.h"
#include <kinc/backend/graphics5/vertexbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>
#include "graphics.h"
#include <stdbool.h>

/*! \file vertexbuffer.h
    \brief Provides functions for setting up and using vertex-buffers.
*/

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

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage);
void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count);
void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count);
int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer);
int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer);
