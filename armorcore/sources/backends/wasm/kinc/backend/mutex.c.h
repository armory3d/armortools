#include <kinc/threads/mutex.h>

void kinc_mutex_init(kinc_mutex_t *mutex) {}

void kinc_mutex_destroy(kinc_mutex_t *mutex) {}

bool kinc_mutex_try_to_lock(kinc_mutex_t *mutex) {
	return false;
}

void kinc_mutex_lock(kinc_mutex_t *mutex) {}

void kinc_mutex_unlock(kinc_mutex_t *mutex) {}

bool kinc_uber_mutex_init(kinc_uber_mutex_t *mutex, const char *name) {
	return false;
}

void kinc_uber_mutex_destroy(kinc_uber_mutex_t *mutex) {}

void kinc_uber_mutex_lock(kinc_uber_mutex_t *mutex) {}

void kinc_uber_mutex_unlock(kinc_uber_mutex_t *mutex) {}
