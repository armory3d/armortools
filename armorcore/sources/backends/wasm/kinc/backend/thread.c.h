#include <kinc/threads/thread.h>

void kinc_thread_init(kinc_thread_t *t, void (*thread)(void *param), void *param) {}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {}

bool kinc_thread_try_to_destroy(kinc_thread_t *thread) {
	return false;
}

void kinc_threads_init() {}

void kinc_threads_quit() {}

void kinc_thread_sleep(int milliseconds) {}
