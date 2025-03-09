#pragma once

#include <stdbool.h>
#include <iron_global.h>
#include BACKEND_THREAD_H

typedef struct iron_thread {
	iron_thread_impl_t impl;
} iron_thread_t;

void iron_threads_init(void);
void iron_threads_quit(void);
void iron_thread_init(iron_thread_t *thread, void (*func)(void *param), void *param);
void iron_thread_wait_and_destroy(iron_thread_t *thread);
bool iron_thread_try_to_destroy(iron_thread_t *thread);
void iron_thread_set_name(const char *name);
void iron_thread_sleep(int milliseconds);

typedef struct iron_mutex {
	iron_mutex_impl_t impl;
} iron_mutex_t;

void iron_mutex_init(iron_mutex_t *mutex);
void iron_mutex_destroy(iron_mutex_t *mutex);
void iron_mutex_lock(iron_mutex_t *mutex);
bool iron_mutex_try_to_lock(iron_mutex_t *mutex);
void iron_mutex_unlock(iron_mutex_t *mutex);
