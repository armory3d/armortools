#pragma once

#include <kinc/global.h>
#include "usage.h"
#include <kinc/backend/graphics4/indexbuffer.h>

/*! \file indexbuffer.h
    \brief Provides functions for setting up and using index-buffers.
*/

typedef struct kinc_g4_index_buffer {
	kinc_g4_index_buffer_impl_t impl;
} kinc_g4_index_buffer_t;

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_usage_t usage);
void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer);
void *kinc_g4_index_buffer_lock_all(kinc_g4_index_buffer_t *buffer);
void *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer, int start, int count);
void kinc_g4_index_buffer_unlock_all(kinc_g4_index_buffer_t *buffer);
void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer, int count);
int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer);

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer);
