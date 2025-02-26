#pragma once

#include <stdbool.h>
#include <iron_global.h>
#include <kinc/backend/thread.h>
#include <kinc/backend/mutex.h>
#include <kinc/backend/atomic.h>

typedef struct kinc_thread {
	kinc_thread_impl_t impl;
} kinc_thread_t;

void kinc_threads_init(void);
void kinc_threads_quit(void);
void kinc_thread_init(kinc_thread_t *thread, void (*func)(void *param), void *param);
void kinc_thread_wait_and_destroy(kinc_thread_t *thread);
bool kinc_thread_try_to_destroy(kinc_thread_t *thread);
void kinc_thread_set_name(const char *name);
void kinc_thread_sleep(int milliseconds);

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
