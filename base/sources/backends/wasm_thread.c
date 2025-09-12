#include <iron_thread.h>

void iron_thread_init(iron_thread_t *t, void (*thread)(void *param), void *param) {}

void iron_thread_wait_and_destroy(iron_thread_t *thread) {}

bool iron_thread_try_to_destroy(iron_thread_t *thread) {
	return false;
}

void iron_threads_init() {}

void iron_threads_quit() {}

void iron_mutex_init(iron_mutex_t *mutex) {}

void iron_mutex_destroy(iron_mutex_t *mutex) {}

bool iron_mutex_try_to_lock(iron_mutex_t *mutex) {
	return false;
}

void iron_mutex_lock(iron_mutex_t *mutex) {}

void iron_mutex_unlock(iron_mutex_t *mutex) {}
