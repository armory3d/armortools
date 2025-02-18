#pragma once

#include <kinc/global.h>
#include <kinc/backend/mutex.h>
#include <stdbool.h>

/*! \file mutex.h
    \brief Provides mutexes which are used to synchronize threads and uber-mutexes which are used to synchronize processes.
*/

typedef struct kinc_mutex {
	kinc_mutex_impl_t impl;
} kinc_mutex_t;

void kinc_mutex_init(kinc_mutex_t *mutex);
void kinc_mutex_destroy(kinc_mutex_t *mutex);
void kinc_mutex_lock(kinc_mutex_t *mutex);
bool kinc_mutex_try_to_lock(kinc_mutex_t *mutex);
void kinc_mutex_unlock(kinc_mutex_t *mutex);

typedef struct kinc_uber_mutex {
	kinc_uber_mutex_impl_t impl;
} kinc_uber_mutex_t;

bool kinc_uber_mutex_init(kinc_uber_mutex_t *mutex, const char *name);
void kinc_uber_mutex_destroy(kinc_uber_mutex_t *mutex);
void kinc_uber_mutex_lock(kinc_uber_mutex_t *mutex);
void kinc_uber_mutex_unlock(kinc_uber_mutex_t *mutex);
