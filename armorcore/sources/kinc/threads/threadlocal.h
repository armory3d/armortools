#pragma once

#include <kinc/global.h>
#include <kinc/backend/threadlocal.h>

/*! \file threadlocal.h
    \brief Provides storage-slots for thread-specific data.
*/

typedef struct kinc_thread_local {
	kinc_thread_local_impl_t impl;
} kinc_thread_local_t;

void kinc_thread_local_init(kinc_thread_local_t *local);
void kinc_thread_local_destroy(kinc_thread_local_t *local);
void *kinc_thread_local_get(kinc_thread_local_t *local);
void kinc_thread_local_set(kinc_thread_local_t *local, void *data);
