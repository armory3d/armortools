#pragma once

#include <kinc/global.h>
#include <kinc/backend/thread.h>

/*! \file thread.h
    \brief Supports the creation and destruction of threads.
*/

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
