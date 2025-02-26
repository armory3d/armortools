#include <iron_thread.h>

void kinc_thread_init(kinc_thread_t *t, void (*thread)(void *param), void *param) {}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {}

bool kinc_thread_try_to_destroy(kinc_thread_t *thread) {
	return false;
}

void kinc_threads_init() {}

void kinc_threads_quit() {}

void kinc_thread_sleep(int milliseconds) {}

void kinc_mutex_init(kinc_mutex_t *mutex) {}

void kinc_mutex_destroy(kinc_mutex_t *mutex) {}

bool kinc_mutex_try_to_lock(kinc_mutex_t *mutex) {
	return false;
}

void kinc_mutex_lock(kinc_mutex_t *mutex) {}

void kinc_mutex_unlock(kinc_mutex_t *mutex) {}
