#include <kinc/threads/semaphore.h>

void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max) {}

void kinc_semaphore_destroy(kinc_semaphore_t *semaphore) {}

void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count) {}

void kinc_semaphore_acquire(kinc_semaphore_t *semaphore) {}

bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double seconds) {
	return false;
}
