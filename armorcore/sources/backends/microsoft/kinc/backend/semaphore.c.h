#include <kinc/threads/semaphore.h>

void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max) {
	semaphore->impl.handle = CreateSemaphoreA(NULL, current, max, NULL);
}

void kinc_semaphore_destroy(kinc_semaphore_t *semaphore) {
	CloseHandle(semaphore->impl.handle);
	semaphore->impl.handle = NULL;
}

void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count) {
	ReleaseSemaphore(semaphore->impl.handle, count, NULL);
}

void kinc_semaphore_acquire(kinc_semaphore_t *semaphore) {
	WaitForSingleObject(semaphore->impl.handle, INFINITE);
}

bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double seconds) {
	return WaitForSingleObject(semaphore->impl.handle, (DWORD)(seconds * 1000)) == WAIT_OBJECT_0;
}
