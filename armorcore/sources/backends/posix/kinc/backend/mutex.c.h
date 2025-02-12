#include <kinc/threads/mutex.h>
#include <assert.h>

void kinc_mutex_init(kinc_mutex_t *mutex) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex->impl.mutex, &attr);
}

void kinc_mutex_destroy(kinc_mutex_t *mutex) {
	pthread_mutex_destroy(&mutex->impl.mutex);
}

bool kinc_mutex_try_to_lock(kinc_mutex_t *mutex) {
	return pthread_mutex_trylock(&mutex->impl.mutex) == 0;
}

void kinc_mutex_lock(kinc_mutex_t *mutex) {
	pthread_mutex_lock(&mutex->impl.mutex);
}

void kinc_mutex_unlock(kinc_mutex_t *mutex) {
	pthread_mutex_unlock(&mutex->impl.mutex);
}

bool kinc_uber_mutex_init(kinc_uber_mutex_t *mutex, const char *name) {
	return false;
}

void kinc_uber_mutex_destroy(kinc_uber_mutex_t *mutex) {}

void kinc_uber_mutex_lock(kinc_uber_mutex_t *mutex) {
	assert(false);
}

void kinc_uber_mutex_unlock(kinc_uber_mutex_t *mutex) {
	assert(false);
}
